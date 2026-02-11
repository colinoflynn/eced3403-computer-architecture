/*

Example code for teaching about Arm Faults

(C) Colin O'Flynn, 2026
Released to public domain
*/

#include <stdio.h>
#include <stdint.h>
#include "stm32f3xx.h"

typedef struct {
    uint32_t r0, r1, r2, r3;
    uint32_t r12, lr, pc, xpsr;
} stacked_frame_t;

volatile uint32_t g_cfsr, g_hfsr, g_mmar, g_bfar, g_shcsr;
volatile stacked_frame_t g_fault_frame;
volatile uint32_t g_fault_sp;

volatile const char *g_fault_kind;

static void decode_fault_kind(void) {
    uint32_t cfsr = SCB->CFSR;

    // Memory Management Fault bits live in CFSR[7:0]
    if (cfsr & 0x000000FFu) {
        g_fault_kind = "MemManage fault";
        return;
    }
    // Bus Fault bits live in CFSR[15:8]
    if (cfsr & 0x0000FF00u) {
        g_fault_kind = "BusFault";
        return;
    }
    // Usage Fault bits live in CFSR[31:16]
    if (cfsr & 0xFFFF0000u) {
        g_fault_kind = "UsageFault";
        return;
    }
    g_fault_kind = "Unknown/HardFault escalation";
}

__attribute__((naked)) void HardFault_Handler(void) {
    __asm volatile(
        // Determine which stack pointer was in use when the fault happened.
        // If bit 2 of LR (EXC_RETURN) is 0 => MSP was used; else PSP was used.
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "b hardfault_c\n"
    );
}

void hardfault_c(uint32_t *sp) {
    g_fault_sp = (uint32_t)sp;

    // Copy the stacked registers into a global struct
    g_fault_frame.r0   = sp[0];
    g_fault_frame.r1   = sp[1];
    g_fault_frame.r2   = sp[2];
    g_fault_frame.r3   = sp[3];
    g_fault_frame.r12  = sp[4];
    g_fault_frame.lr   = sp[5];
    g_fault_frame.pc   = sp[6];
    g_fault_frame.xpsr = sp[7];

    g_cfsr  = SCB->CFSR;   // Configurable Fault Status Register
    g_hfsr  = SCB->HFSR;   // HardFault Status Register
    g_mmar  = SCB->MMFAR;  // MemManage Fault Address Register
    g_bfar  = SCB->BFAR;   // BusFault Address Register
    g_shcsr = SCB->SHCSR;  // System Handler Control and State Register

    decode_fault_kind();

    __asm volatile("bkpt #0");  // pause here for inspection
    while (1) { }               // stay here if no debugger
}


void do_illegal(void) {
    __asm volatile(".word 0xFFFFFFFF\n");
}

void jump_illegal(void){
  void (*fptr)(void) = (void (*)(void))(0xFFFFFFFF);
  fptr();
}

void write_illegal(void){
  uint32_t * ptr = (uint32_t *)(0xFFFFFFFF);
  *ptr = 12;
}

int main(void) {

  do_illegal();
  jump_illegal();
  write_illegal();

  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  while(1);

}

/*************************** End of file ****************************/
