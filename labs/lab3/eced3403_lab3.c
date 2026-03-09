#include "stm32f3xx.h"
#include <stdint.h>
#include <stdio.h>

#define BUF_WORDS 32

/* Buffers used in experiments */

__attribute__((section(".RAM1"))) volatile uint32_t src_buf[BUF_WORDS];
__attribute__((section(".RAM1"))) volatile uint32_t dst_cpu[BUF_WORDS];
__attribute__((section(".RAM1"))) volatile uint32_t dst_dma[BUF_WORDS];

__attribute__((aligned(256)))
__attribute__((section(".RAM1"))) volatile uint32_t protected_buf[64];

/* Results visible in debugger */

volatile uint32_t cpu_cycles;
volatile uint32_t dma_cycles;

volatile uint32_t dma_test_result = 0;

void mpu_disable(void)
{
    MPU->CTRL = 0;
    __DSB();
    __ISB();
}

void mpu_enable_protected_region(void *addr)
{
    /*
     * Region requirements:
     * - 256-byte region
     * - base address aligned to 256 bytes
     */

    /* Enable MemManage fault */
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

    /* Disable MPU while configuring */
    MPU->CTRL = 0;

    /* Use region 0 */
    MPU->RNR = 0;

    /*
     * RBAR:
     *   bits [31:5] = base address
     *   lower bits contain region/valid fields if used that way
     *
     * Here we just write the aligned address directly.
     */
    MPU->RBAR = ((uint32_t)addr & MPU_RBAR_ADDR_Msk);

    /*
     * RASR fields:
     *   SIZE = 7  => region size = 2^(7+1) = 256 bytes
     *   AP   = 0  => no access
     *   XN   = 0  => executable/not relevant here
     *   ENABLE = 1
     *
     * TEX/S/C/B left as 0 for simple strongly-ordered/default-like behavior
     * for this demo.
     */
    MPU->RASR =
          (7u << MPU_RASR_SIZE_Pos)        /* 256 bytes */
        | (0u << MPU_RASR_AP_Pos)          /* no access */
        | MPU_RASR_ENABLE_Msk;

    /*
     * Enable MPU, but leave background map enabled for privileged accesses
     * outside defined MPU regions.
     */
    MPU->CTRL =
          MPU_CTRL_ENABLE_Msk
        | MPU_CTRL_PRIVDEFENA_Msk;

    __DSB();
    __ISB();
}

void dma_init(void)
{
    /* Enable DMA clock */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
}

/*
 * Configure DMA for memory-to-memory transfer
 *
 * Requirements:
 * - 32-bit transfers
 * - increment source and destination
 * - memory-to-memory mode
 * - blocking until complete
 */
void dma_memcpy_words(volatile uint32_t *dst,
                      volatile const uint32_t *src,
                      uint32_t count)
{
    /* Disable channel before reconfiguring */
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;
    while (DMA1_Channel1->CCR & DMA_CCR_EN) {
        /* wait until really disabled */
    }

    /* Clear any old interrupt/complete/error flags */
    DMA1->IFCR =
          DMA_IFCR_CGIF1
        | DMA_IFCR_CTCIF1
        | DMA_IFCR_CHTIF1
        | DMA_IFCR_CTEIF1;

    /*
     * For MEM2MEM mode on STM32 DMA:
     *   - CPAR is used as source
     *   - CMAR is used as destination
     */
    DMA1_Channel1->CPAR  = (uint32_t)src;
    DMA1_Channel1->CMAR  = (uint32_t)dst;
    DMA1_Channel1->CNDTR = count;

    DMA1_Channel1->CCR =
          DMA_CCR_MEM2MEM      /* memory-to-memory mode */
        | DMA_CCR_MINC         /* increment destination */
        | DMA_CCR_PINC         /* increment source */
        | DMA_CCR_MSIZE_1      /* destination size = 32-bit */
        | DMA_CCR_PSIZE_1      /* source size = 32-bit */
        | DMA_CCR_PL_1;        /* high priority */

    /* Start transfer */
    DMA1_Channel1->CCR |= DMA_CCR_EN;

    /* Wait for transfer complete */
    while ((DMA1->ISR & DMA_ISR_TCIF1) == 0) {
    }

    /* Optional: check TEIF for error */
    if (DMA1->ISR & DMA_ISR_TEIF1) {
        while (1) {
            __NOP();
        }
    }

    /* Clear transfer-complete flag */
    DMA1->IFCR = DMA_IFCR_CTCIF1;

    /* Disable channel after transfer for cleanliness */
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;
}



/* ------------------------------------------------------------- */
/* Cycle counter                                                  */
/* ------------------------------------------------------------- */

static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t cycles(void)
{
    return DWT->CYCCNT;
}

/* ------------------------------------------------------------- */
/* CPU copy function                                              */
/* ------------------------------------------------------------- */

static void cpu_copy_words(volatile uint32_t *dst,
                           volatile const uint32_t *src,
                           uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        dst[i] = src[i];
    }
}

/* ------------------------------------------------------------- */

static void init_buffers(void)
{
    for (uint32_t i = 0; i < BUF_WORDS; i++) {

        src_buf[i] = 0x12340000 + i;

        dst_cpu[i] = 0;
        dst_dma[i] = 0;
    }

    for (uint32_t i = 0; i < 64; i++) {
        protected_buf[i] = 0xFEEDFACE;
    }
}

/* ------------------------------------------------------------- */

int main(void)
{
    SystemInit();

    dwt_init();
    dma_init();

    init_buffers();

    /* -----------------------------------------
       PART 1 — CPU vs DMA copy timing
       ----------------------------------------- */
    uint32_t start = cycles();
    cpu_copy_words(dst_cpu, src_buf, BUF_WORDS);
    uint32_t end = cycles();

    cpu_cycles = end - start;

    printf("CPU Copy took %d cycles\n", cpu_cycles);


    start = cycles();

    dma_memcpy_words(dst_dma, src_buf, BUF_WORDS);

    end = cycles();

    dma_cycles = end - start;

    printf("DMA Cycles took %d cycles\n", dma_cycles);


    //To complete part 2, delete part 1, comment this out, #if 0 this out
    while(1);

    /* -----------------------------------------
       PART 2 — MPU DMA Experiments
       ----------------------------------------- */

    mpu_enable_protected_region((void*)protected_buf);

    uint32_t secret_data;

    secret_data = protected_buf[0];

    printf("Secret Data is: %x\n", secret_data);


    while (1) {
    }
}