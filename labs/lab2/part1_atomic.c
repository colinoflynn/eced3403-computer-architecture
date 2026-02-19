#include <stdio.h>
#include <stdint.h>
#include "stm32f3xx.h"


// ------------------- atomic() helper -------------------
// Keep it tiny and restore prior state (important if nested / already masked)
static inline uint32_t atomic_enter(void) {
    uint32_t pm = __get_PRIMASK();
    __disable_irq();
    return pm;
}
static inline void atomic_exit(uint32_t pm) {
    __set_PRIMASK(pm);
}

// Convenience macro that looks like atomic(...)
#define atomic(block) do { \
    uint32_t __pm = atomic_enter(); \
    block \
    atomic_exit(__pm); \
} while (0)


volatile int a;
volatile int inv_a;
volatile int corrupt_count = 0;

void SysTick_Handler (void ) {

    if (inv_a != ~a) {
        corrupt_count++;
    }

}


int main(void) {

  SysTick->LOAD = 1000; // reload value
  SysTick->VAL  = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                  SysTick_CTRL_TICKINT_Msk |
                  SysTick_CTRL_ENABLE_Msk;
  int dlycnt = 0;
  int old_corrupt = -1;

  while(1) {
    if(old_corrupt != corrupt_count){
      printf("corrupt: %d\n", corrupt_count);
      old_corrupt = corrupt_count;
    }

    a++;
    inv_a = ~a;

    dlycnt++;

    if((dlycnt % 20000) == 0){
      printf("dlycnt: %d\n", dlycnt);
    }
  }
}

