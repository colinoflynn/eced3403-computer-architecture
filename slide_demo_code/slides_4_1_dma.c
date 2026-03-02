#include <stdint.h>
#include "stm32f3xx.h"

#define LEN_WORDS 256

//static uint32_t src[LEN_WORDS];
//static uint32_t dst[LEN_WORDS];

// Variables need to be in DMAable memory - TCM/CCM is NOT. By default some linker/compiler defaults
// may place them there (segger seems too in ES), so instead we pick random memory locations to clobber

uint32_t * src = (uint32_t *)0x20001000;
uint32_t * dst = (uint32_t *)0x20002000;

static void dma1_ch1_mem2mem_init(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    // Clear all ch1 flags
    DMA1->IFCR = DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1;

    // MEM2MEM + increment both + 32-bit width
    // IMPORTANT: leave DIR=0 (P->M). In MEM2MEM, CPAR is treated as "source".
    DMA1_Channel1->CCR =
        DMA_CCR_MEM2MEM |
        DMA_CCR_MINC |        // increment destination (CMAR)
        DMA_CCR_PINC |        // increment source (CPAR)
        DMA_CCR_MSIZE_1 |     // 32-bit
        DMA_CCR_PSIZE_1 |     // 32-bit
        DMA_CCR_PL_1;         // high priority (optional)
}

static void start_dma_memcpy_u32(const uint32_t *from, uint32_t *to, uint16_t nwords) {
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    DMA1->IFCR = DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1;

    // In MEM2MEM, CPAR = source, CMAR = destination
    DMA1_Channel1->CPAR  = (uint32_t)from;
    DMA1_Channel1->CMAR  = (uint32_t)to;
    DMA1_Channel1->CNDTR = nwords;

    __DSB();
    DMA1_Channel1->CCR |= DMA_CCR_EN;
}

int main(void) {
    for (uint32_t i = 0; i < LEN_WORDS; i++) {
        src[i] = 0xA5A50000u ^ i;
        dst[i] = 0;
    }

    // Set a breakpoint here and watch src/dst + DMA regs
    dma1_ch1_mem2mem_init();

    // Open destination memory to watch the magic!

    // Single step in/over and you'll see almost instant transfer
    start_dma_memcpy_u32(src, dst, LEN_WORDS);

    while (1) {
        uint32_t isr = DMA1->ISR;

        if (isr & DMA_ISR_TEIF1) {
            // Transfer error - trigger debugger if attached, otherwise spin
            __BKPT(1);
            while(1);
        }
        if (isr & DMA_ISR_TCIF1) {
            // Done (watch dst[] now matches src[])
            DMA1->IFCR = DMA_IFCR_CTCIF1;
            break;
        }
        __NOP();
    }

    //Transfer done!
    // Clear TC flag (good hygiene)
    DMA1->IFCR = DMA_IFCR_CTCIF1;

    //Code would continue here - we just break
    __BKPT(1);
    while(1);

}