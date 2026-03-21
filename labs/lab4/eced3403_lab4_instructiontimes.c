#include "stm32f3xx.h"
#include <stdint.h>
#include <stdio.h>

#define DEMCR      (*(volatile uint32_t*)0xE000EDFC)
#define DWT_CTRL   (*(volatile uint32_t*)0xE0001000)
#define DWT_CYCCNT (*(volatile uint32_t*)0xE0001004)

void dwt_init(void)
{
    DEMCR |= (1 << 24);     // Enable DWT
    DWT_CYCCNT = 0;
    DWT_CTRL |= 1;
}

uint32_t bench_run(void (*func)(uint32_t))
{
    uint32_t iters = 0x10000;

    DWT_CYCCNT = 0;
    uint32_t start = DWT_CYCCNT;

    func(iters);

    uint32_t end = DWT_CYCCNT;
    return (end - start) >> 16;
}

volatile uint32_t sink;

// ------------------ ALU ------------------

void bench_add_dep(uint32_t iters)
{
    volatile uint32_t x = 1;
    for (uint32_t i = 0; i < iters; i++) {
        x = x + 3;
        x = x + 3;
        x = x + 3;
        x = x + 3;
    }
    sink = x;
}

void bench_add_indep(uint32_t iters)
{
    volatile uint32_t x0=1,x1=2,x2=3,x3=4;

    for (uint32_t i = 0; i < iters; i++)
    {
        x0 += 3;
        x1 += 3;
        x2 += 3;
        x3 += 3;
    }

    sink = x0 + x1 + x2 + x3;
}

void bench_mul(uint32_t iters)
{
    volatile uint32_t x = 3;
    for (uint32_t i = 0; i < iters; i++) {
        x = x * 5;
        x = x * 5;
        x = x * 5;
        x = x * 5;
    }
    sink = x;
}

void bench_xor(uint32_t iters)
{
    volatile uint32_t x = 0x55;
    for (uint32_t i = 0; i < iters; i++) {
        x ^= 0xAA;
        x ^= 0xAA;
        x ^= 0xAA;
        x ^= 0xAA;
    }
    sink = x;
}

// ------------------ Branch ------------------

void bench_branch_backend(uint32_t iters, uint32_t mask)
{
    volatile uint32_t sum = 0;
    for (uint32_t i = 0; i < iters; i++)
    {
        if (i & mask) {
            sum += i;
        } else {
            sum += i;
        }
    }
    sink = sum;
}


void bench_branch_predictable(uint32_t iters)
{
    bench_branch_backend(iters, 0xFFFF);
}

void bench_branch_alternating(uint32_t iters)
{
    bench_branch_backend(iters, 1);
}

int main(void)
{
    dwt_init();

    printf("add_dep: %u\n", bench_run(bench_add_dep));
    printf("add_indep: %u\n", bench_run(bench_add_indep));
    printf("mul: %u\n", bench_run(bench_mul));
    printf("xor: %u\n", bench_run(bench_xor));

    printf("branch_pred: %u\n", bench_run(bench_branch_predictable));
    printf("branch_alt: %u\n", bench_run(bench_branch_alternating));

    while(1);
}