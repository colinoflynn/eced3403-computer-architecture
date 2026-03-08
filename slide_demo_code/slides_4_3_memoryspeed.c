#include "stm32f3xx.h"
#include <stdint.h>
#include <string.h>

#define LOOP_COUNT 50000u

/* --------------------------------------------------------------------------
 * Assembly functions from loops.S
 * -------------------------------------------------------------------------- */
extern uint8_t  flash_linear_start;
extern uint8_t  flash_linear_end;
extern uint32_t flash_linear_fn(uint32_t count);
extern uint32_t flash_jumpy_fn(uint32_t count);

/* --------------------------------------------------------------------------
 * Results visible in debugger
 * -------------------------------------------------------------------------- */
volatile uint32_t measured_flash_linear = 0;
volatile uint32_t measured_sram_linear  = 0;
volatile uint32_t measured_flash_jumpy  = 0;

volatile uint32_t expected_flash_linear = 799998u;
volatile uint32_t expected_sram_linear  = 799998u;
volatile uint32_t expected_flash_jumpy  = 799998u;

volatile uint32_t result_flash_linear   = 0;
volatile uint32_t result_sram_linear    = 0;
volatile uint32_t result_flash_jumpy    = 0;

/* Optional: differences between measured and nominal */
volatile int32_t delta_flash_linear     = 0;
volatile int32_t delta_sram_linear      = 0;
volatile int32_t delta_flash_jumpy      = 0;

/* --------------------------------------------------------------------------
 * SRAM copy buffer
 *
 * No linker-script setup needed. We just copy the machine code into RAM.
 * -------------------------------------------------------------------------- */
typedef uint32_t (*bench_fn_t)(uint32_t);

static uint8_t ram_code[128] __attribute__((aligned(4)));


void clock_72mhz(void)
{
    /* 1. Enable HSE (external crystal) */
    RCC->CR |= RCC_CR_HSEON;

    while (!(RCC->CR & RCC_CR_HSERDY));

    /* 2. Configure Flash wait states
       72 MHz requires 2 wait states */
    FLASH->ACR =
          FLASH_ACR_PRFTBE     /* enable prefetch */
        | FLASH_ACR_LATENCY_2;

    /* 3. Configure PLL
       HSE = 8 MHz
       PLL = 8 MHz * 9 = 72 MHz */
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;
    RCC->CFGR |= RCC_CFGR_PLLMUL9;

    /* enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* 4. Configure bus prescalers */
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;   /* AHB  = 72 MHz */
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;  /* APB1 = 36 MHz */
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;  /* APB2 = 72 MHz */

    /* 5. Switch system clock to PLL */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}


/* --------------------------------------------------------------------------
 * DWT cycle counter support
 * -------------------------------------------------------------------------- */
static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t dwt_get_cycles(void)
{
    return DWT->CYCCNT;
}

/* --------------------------------------------------------------------------
 * Copy flash_linear_fn into SRAM and return a callable Thumb function pointer
 * -------------------------------------------------------------------------- */
static bench_fn_t prepare_sram_copy(void)
{
    uintptr_t start = (uintptr_t)&flash_linear_start;
    uintptr_t end   = (uintptr_t)&flash_linear_end;
    size_t len = (size_t)(end - start);

    if (len > sizeof(ram_code)) {
        while (1) {
            __NOP();
        }
    }

    memcpy(ram_code, (const void *)start, len);

    __DSB();
    __ISB();

    /* Thumb function pointer needs bit 0 set */
    return (bench_fn_t)((uintptr_t)ram_code | 1u);
}

/* --------------------------------------------------------------------------
 * Measure a benchmark function in cycles
 * -------------------------------------------------------------------------- */
static uint32_t measure_fn(bench_fn_t fn, uint32_t count, volatile uint32_t *result_out)
{
    uint32_t start;
    uint32_t end;
    uint32_t ret;

    __disable_irq();
    __DSB();
    __ISB();

    start = dwt_get_cycles();
    ret   = fn(count);
    end   = dwt_get_cycles();

    __DSB();
    __ISB();
    __enable_irq();

    *result_out = ret;
    return end - start;
}

/* --------------------------------------------------------------------------
 * Main
 * -------------------------------------------------------------------------- */
int main(void)
{
    SystemInit();
    dwt_init();

    //Typical 72 MHz setup - comment this in/out to see how times change

    clock_72mhz();

    bench_fn_t sram_linear_fn = prepare_sram_copy();

    /* Warm-up runs so first measurement is less weird */
    (void)flash_linear_fn(100);
    (void)flash_jumpy_fn(100);
    (void)sram_linear_fn(100);

    measured_flash_linear = measure_fn(flash_linear_fn, LOOP_COUNT, &result_flash_linear);
    measured_sram_linear  = measure_fn(sram_linear_fn, LOOP_COUNT, &result_sram_linear);
    measured_flash_jumpy  = measure_fn(flash_jumpy_fn, LOOP_COUNT, &result_flash_jumpy);

    delta_flash_linear = (int32_t)measured_flash_linear - (int32_t)expected_flash_linear;
    delta_sram_linear  = (int32_t)measured_sram_linear  - (int32_t)expected_sram_linear;
    delta_flash_jumpy  = (int32_t)measured_flash_jumpy  - (int32_t)expected_flash_jumpy;

    while (1) {
        __NOP();
    }
}
