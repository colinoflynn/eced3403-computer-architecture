#include <stdio.h>
#include <stdint.h>
#include "stm32f3xx.h"

void SysTick_Handler(void){
  ;
}

int main(void) {

    //Enable the SysTick Interrupt
    SysTick->LOAD = 1000;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk | 
                    SysTick_CTRL_ENABLE_Msk;

    
    asm volatile("ldr r0, =0x123456"); //Student ID - 1st
    asm volatile("ldr r1, =0x223456"); //Student ID - 2nd
    asm volatile("ldr r2, =0x222456"); //Student ID - 3rd (if applicable)
    asm volatile("ldr r3, =0x329848"); //Pick a random number

    //Do work here if needed
    while(1) {
      ;
    }
}