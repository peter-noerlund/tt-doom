//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//    Fixed point implementation.
//



#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#include "m_fixed.h"


#if defined(__riscv) && __riscv_xlen == 32

#if defined(__riscv_mul)

__attribute__((naked)) fixed_t FixedMul(fixed_t a, fixed_t b)
{
    __asm__ volatile (
        "mulh t1, a0, a1\n"
        "mul t0, a0, a1\n"
        "srli a0, t0, 16\n"
        "slli a1, t1, 16\n"
        "or a0, a1, a0\n"
        "ret"
    );
}

#elifdef TT_XMUL16

__attribute__((naked)) fixed_t FixedMul(fixed_t a, fixed_t b)
{
    __asm__ volatile (
        "srli t0, a0, 16\n"                 // t0 = ah
        "srli t1, a1, 16\n"                 // t1 = bh
        ".insn r 0x0b, 0, 0, t2, a0, a1\n"  // t2 = al * bl (mul16u)
        ".insn r 0x0b, 1, 0, a2, t0, a1\n"  // a2 = ah * bl (mull6su)
        ".insn r 0x0b, 1, 0, a3, t1, a0\n"  // a3 = al * bh (mul16su)
        ".insn r 0x0b, 1, 0, a4, t0, t1\n"  // t0 = ah * bh (mul16su)
        "srli t2, t2, 16\n"
        "add t2, t2, a2\n"
        "add t2, t2, a3\n"
        "slli a4, a4, 16\n"
        "add a0, t2, a4\n"
        "ret\n");
}

#else // __riscv_mul || TT_XMUL16

__attribute__((naked)) fixed_t FixedMul(fixed_t a, fixed_t b)
{
    // From rvint library (https://github.com/benmesander/rvint/blob/main/src/mul32.s)
    // Credit to Benard Mesander
    __asm__ volatile (
        // Determine result sign
        "xor a5, a0, a1\n"

        // Absolute values
#ifdef __riscv_zbb
        "neg t0, a0\n"
        "max a0, a0, t0\n"
        "neg t0, a1\n"
        "max a1, a1, t0\n"
#else // __riscv_zbb
        "srai t0, a0, 31\n"
        "xor a0, a0, t0\n"
        "sub a0, a0, t0\n"
        "srai t0, a1, 31\n"
        "xor a1, a1, t0\n"
        "sub a1, a1, t0\n"
#endif // __riscv_zbb

        // Swap for optimization
        "bgeu a0, a1, .LMatMulNoSwap\n"
        "mv t0, a0\n"
        "mv a0, a1\n"
        "mv a1, t0\n"

        // Loop setup
        ".LMatMulNoSwap:\n"
        "mv a2, a1\n"   // a2 = Multiplier (small)
        "mv a3, a0\n"   // a3 = mcand low
        "li a4, 0\n"    // a4 = mcand high
        "li t0, 0\n"    // t0 = prod low
        "li t1, 0\n"    // t1 = prod high

        ".LMatMulLoop:\n"
        "andi t2, a2, 1\n"
        "beqz t2, .LMatMulSkip\n"
        
        // Add 64-bit product += mcand
        "add t0, t0, a3\n"  // lo += lo
        "sltu t2, t0, a3\n" // carry
        "add t1, t1, t2\n"  // hi += carry
        "add t1, t1, a4\n"  // hi += hi

        ".LMatMulSkip:\n"
        "srli a2, a2, 1\n"  // multiplier >>= 1
        "beqz a2, .LMatMulSign\n"

        // Shift 64-bit mand <<= 1
        "slli a4, a4, 1\n"  // hi << 1
        "srli t2, a3, 31\n" // extract MSB of lo
        "or a4, a4, t2\n"   // Merge MSB into hi
        "slli a3, a3, 1\n"  // lo << 1
        "j .LMatMulLoop\n"

        // Restore sign
        ".LMatMulSign:\n"
        "bgez a5, .LMatMulDone\n"
        
        // Negate 64-bit (prod = ~prod + 1)
        "not t0, t0\n"
        "not t1, t1\n"
        "addi t0, t0, 1\n"  // lo += 1
        "sltu t2, t0, 1\n"  // carry out of low
        "add t1, t1, t2\n"  // hi += carry

        // Shift FRACBITS
        ".LMatMulDone:\n"
        "srli a0, t0, 16\n"
        "slli a1, t1, 16\n"
        "or a0, a1, a0\n"
        "ret\n"
    );
}

#endif // __riscv_mul

#else // __riscv && __riscv_xlen == 32

fixed_t FixedMul(fixed_t a, fixed_t b)
{
    return ((int64_t)a * (int64_t)b) >> FRACBITS;
}

#endif // __riscv

fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
    {
        return (a ^ b) < 0 ? INT_MIN : INT_MAX;
    }
    else
    {
        int64_t result;
        result = ((int64_t) a << 16) / b;
        return (fixed_t) result;
    }
}

