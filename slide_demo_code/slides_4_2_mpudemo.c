#include <stdint.h>
#include "stm32f3xx.h"

// ------------ Fault capture for debugger ------------
volatile uint32_t mm_cfsr, mm_mmfars, mm_hfsr;
volatile uint32_t mm_stacked_pc, mm_stacked_lr, mm_stacked_xpsr;

__attribute__((naked)) void MemManage_Handler(void) {
    __asm volatile(
        "tst lr, #4        \n"
        "ite eq            \n"
        "mrseq r0, msp     \n"
        "mrsne r0, psp     \n"
        "b memmanage_c     \n"
    );
}

void memmanage_c(uint32_t *sp) {
    // Stacked frame: r0 r1 r2 r3 r12 lr pc xpsr
    mm_stacked_lr   = sp[5];
    mm_stacked_pc   = sp[6];
    mm_stacked_xpsr = sp[7];

    mm_cfsr   = SCB->CFSR;   // MemManage faults are in bits [7:0]
    mm_mmfars = SCB->MMFAR;  // Valid if MMARVALID bit set in CFSR
    mm_hfsr   = SCB->HFSR;

    __BKPT(0);               // Break here: inspect mm_* in debugger
    while (1) {}
}

// ------------ Helpers ------------
static inline void enable_configurable_faults(void) {
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk |
                  SCB_SHCSR_BUSFAULTENA_Msk |
                  SCB_SHCSR_USGFAULTENA_Msk;
}

static inline int mpu_present(void) {
    // If MPU is not implemented, TYPE.DREGION == 0
    return ((MPU->TYPE & MPU_TYPE_DREGION_Msk) != 0);
}

static inline void set_unprivileged_thread_mode(void) {
    // CONTROL.nPRIV = 1 => unprivileged thread mode
    __set_CONTROL(__get_CONTROL() | 1u);
    __ISB();
}

static inline void set_privileged_thread_mode(void) {
    __set_CONTROL(__get_CONTROL() & ~1u);
    __ISB();
}

// Convert size in bytes to MPU RASR SIZE field: SIZE = log2(bytes) - 1
// bytes must be a power of two, >= 32
static inline uint32_t mpu_size_field(uint32_t bytes_pow2) {
    uint32_t lg = 0;
    while ((1u << lg) < bytes_pow2) lg++;
    return (lg - 1u) << MPU_RASR_SIZE_Pos;
}

static void mpu_config_region(uint32_t region_num,
                              uint32_t base_addr,
                              uint32_t size_bytes_pow2,
                              uint32_t rasr_attrs) {
    // Base must be aligned to region size.
    // Assume you pass aligned values - NOT CHECKED IN THIS FUNCTION
    MPU->RNR  = region_num;
    MPU->RBAR = (base_addr & MPU_RBAR_ADDR_Msk) | MPU_RBAR_VALID_Msk | region_num;
    MPU->RASR = rasr_attrs | mpu_size_field(size_bytes_pow2) | MPU_RASR_ENABLE_Msk;
}

// ------------ MPU setup for demo ------------
static void mpu_setup_demo(void) {
    if (!mpu_present()) {
        // No MPU on this core/device build: stop here for the demo
        __BKPT(9);
        while (1) {}
    }

    // Disable MPU while reconfiguring
    MPU->CTRL = 0;

    // Region attributes helpers
    // AP encoding (Cortex-M):
    // 0b000: no access
    // 0b001: priv RW, unpriv no access
    // 0b010: priv RW, unpriv RO
    // 0b011: priv RW, unpriv RW
    // 0b101: priv RO, unpriv no access
    // 0b110: priv RO, unpriv RO
    //
    // XN (Execute Never): 1 = cannot execute from region

    // ---------------------------
    // Region 0: Flash RX (no writes)
    // Typical STM32 flash base 0x08000000.
    // Size: choose 512KB (power of two) for demo; adjust if desired.
    // AP: priv RO/unpriv RO (0b110) and XN=0 (executable)
    // ---------------------------
    mpu_config_region(
        0,
        0x08000000u,
        512u * 1024u,
        (0u << MPU_RASR_XN_Pos) |
        (0b110u << MPU_RASR_AP_Pos)
    );

    // ---------------------------
    // Region 1: SRAM RW, XN (no execute)
    // Base 0x20000000. Size: 64KB for demo; adjust if needed.
    // AP: priv RW/unpriv RW (0b011), XN=1
    // ---------------------------
    mpu_config_region(
        1,
        0x20000000u,
        64u * 1024u,
        (1u << MPU_RASR_XN_Pos) |
        (0b011u << MPU_RASR_AP_Pos)
    );

    // ---------------------------
    // Region 2: Peripherals privileged-only RW, XN
    // Base 0x40000000. Size: 512MB for demo coverage of peripheral space.
    // AP: priv RW/unpriv no access (0b001), XN=1
    // ---------------------------
    mpu_config_region(
        2,
        0x40000000u,
        512u * 1024u * 1024u,
        (1u << MPU_RASR_XN_Pos) |
        (0b001u << MPU_RASR_AP_Pos)
    );

    // ---------------------------
    // Region 3: Stack guard region (no access)
    // We’ll place a 256-byte no-access region just below current MSP.
    // This catches simple stack overflows.
    // ---------------------------
    uint32_t msp = __get_MSP();
    const uint32_t guard_size = 256u;                   // must be power of two >= 32
    uint32_t guard_base = (msp - guard_size) & ~(guard_size - 1u);

    mpu_config_region(
        3,
        guard_base,
        guard_size,
        (1u << MPU_RASR_XN_Pos) |
        (0b000u << MPU_RASR_AP_Pos)   // no access
    );

    // Enable MPU:
    // - ENABLE: turn on MPU
    // - PRIVDEFENA: privileged code uses default memory map for areas not covered by regions
    //   (unprivileged still needs an explicit region to access)
    MPU->CTRL = MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
    __DSB();
    __ISB();
}

// ------------ “Bad operations” to demonstrate protection ------------
static void test_unprivileged_write_flash(void) {
    volatile uint32_t *p = (volatile uint32_t *)0x08000000u;
    *p = 0xDEADBEEFu;  // should MemManage fault: flash is RO
}

static void test_unprivileged_peripheral_access(void) {
    volatile uint32_t *p = (volatile uint32_t *)0x40000000u;
    (void)*p;          // should MemManage fault: peripherals priv-only
}

static void test_execute_from_sram(void) {
    // Put a tiny function stub in SRAM and try to execute it.
    // We'll place a BX LR instruction word into SRAM and call it.
    // BX LR in Thumb is 0x4770 (16-bit). We'll store it and call.
    static uint16_t thunk[2] __attribute__((aligned(2))) = { 0x4770, 0x4770 };

    // Function pointer into SRAM (Thumb bit = 1)
    void (*fn)(void) = (void (*)(void))(((uint32_t)thunk) | 1u);

    fn();              // should MemManage fault: SRAM is XN
}

static void test_stack_guard_overflow(void) {
    // Deliberately scribble into the guard region by moving a pointer below MSP.
    uint32_t msp = __get_MSP();
    volatile uint32_t *p = (volatile uint32_t *)(msp - 300u);
    *p = 0x12345678u;  // should MemManage fault: guard region no access
}

// ------------ Main ------------
int main(void) {
    enable_configurable_faults();

    // Make vector table explicit (good hygiene for exception demos)
    SCB->VTOR = FLASH_BASE;
    __DSB(); __ISB();

    // Configure MPU regions
    mpu_setup_demo();

    // Switch Thread mode to unprivileged
    set_unprivileged_thread_mode();

    // ---- Choose ONE test at a time (set breakpoints here) ----
    // test_unprivileged_write_flash();
    // test_unprivileged_peripheral_access();
    test_execute_from_sram();
    // test_stack_guard_overflow();

    while (1) {
        __NOP();
    }
}