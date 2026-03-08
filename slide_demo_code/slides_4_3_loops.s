.syntax unified
.cpu cortex-m4
.thumb

.text

/* ============================================================================
 * flash_linear_fn
 *
 * Empirically tuned to match flash_jumpy_fn on STM32F303:
 *
 * Observed on hardware:
 *   flash_jumpy  ~= 600016 cycles for 50000 iterations
 *   => about 12 cycles/iteration + small fixed overhead
 *
 * So we make flash_linear:
 *   4 useful ALU ops
 *   6 useless MOVs
 *   1 loop branch
 *
 * Effective timing observed should be close to:
 *   4 + 6 + 2 = 12 cycles/iteration
 * ==========================================================================*/

.global flash_linear_start
.global flash_linear_fn
.global flash_linear_end
.type flash_linear_fn, %function

.align 2
flash_linear_start:
flash_linear_fn:
    push    {r4, lr}
    movs    r1, #0
    movs    r2, #1

1:
    adds    r1, r1, r2
    eors    r2, r2, r1
    adds    r2, r2, #3
    subs    r0, r0, #1

    mov     r4, r4
    mov     r4, r4
    mov     r4, r4
    mov     r4, r4
    mov     r4, r4
    mov     r4, r4

    bne     1b

    mov     r0, r1
    pop     {r4, pc}

flash_linear_end:
.size flash_linear_fn, . - flash_linear_fn


/* ============================================================================
 * flash_jumpy_fn
 *
 * Intentionally branch-heavy. Empirically measures ~12 cycles/iteration on
 * your setup.
 * ==========================================================================*/

.global flash_jumpy_fn
.type flash_jumpy_fn, %function

.align 2
flash_jumpy_fn:
    push    {r4, lr}
    movs    r1, #0
    movs    r2, #1

loop_top:
    b       step1

    .balign 64
step1:
    adds    r1, r1, r2
    b       step2

    .balign 64
step2:
    eors    r2, r2, r1
    b       step3

    .balign 64
step3:
    adds    r2, r2, #3
    subs    r0, r0, #1
    bne     loop_top

    mov     r0, r1
    pop     {r4, pc}

.size flash_jumpy_fn, . - flash_jumpy_fn