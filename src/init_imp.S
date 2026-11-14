    .section .vectors, "ax", @progbits
    .globl _init
    .type _init, @function
    .align 2

_init:
    j _start

    .org 8
    .globl _interrupt_vector
    .type _interrupt_vector, @function
_interrupt_vector:
    j _interrupt_handler 

    .section .text

    .globl _interrupt_handler
    .type _interrupt_handler, @function
_interrupt_handler:
    # gp is hardcoded to 0x01000400 and the range 0x01000000 - 0x010007FF is reserved for this purpose
    sw x1, -1024(gp)
    sw x2, -1020(gp)
    sw x5, -1016(gp)
    sw x6, -1012(gp)
    sw x7, -1008(gp)
    sw x8, -1004(gp)
    sw x9, -1000(gp)
    sw x10, -996(gp)
    sw x11, -992(gp)
    sw x12, -988(gp)
    sw x13, -984(gp)
    sw x14, -980(gp)
    sw x15, -976(gp)
    addi sp, gp, 1020
    call interrupt_handler
    lw x1, -1024(gp)
    lw x2, -1020(gp)
    lw x5, -1016(gp)
    lw x6, -1012(gp)
    lw x7, -1008(gp)
    lw x8, -1004(gp)
    lw x9, -1000(gp)
    lw x10, -996(gp)
    lw x11, -992(gp)
    lw x12, -988(gp)
    lw x13, -984(gp)
    lw x14, -980(gp)
    lw x15, -976(gp)
    mret
