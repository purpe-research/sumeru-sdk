#ifndef __SAKE_MEMCTL_H
#define __SAKE_MEMCTL_H

#include "constant.h"

/*
 * Flush dcache is implemented using a custom instruction having
 * and S-type encoding. For e.g. the instruction 0x0007B023
 * implies
 *
 *  0	 0	 0     7	    B	     0		 2   3
 *  --------------------------------------------------------------
 *  0000 0000  0 0000  0111 1       011      0000 0      010 0011
 *  ---------  ------  ------       ---     -------      ---------
 *  immediate  rs2	rs1         funct3  immediate    opcode
 *  unused     unused	flush-addr   	    (unused)
 *
 *  Any rs1 register can be used, the code below uses a5 (reg 15)
 */

__attribute__ ((always_inline))
inline void
flush_dcache_line(unsigned int x)
{
    x ^= 0x10000;
    x &= CPU_CACHE_LINE_MASK;
    asm volatile(" \
        addi a5,%0,0; \
        .word 0x0007B023; " : : "r"(x) : "a5");
}

__attribute__ ((always_inline))
inline void
flush_dcache_range(char *start, char *end)
{
    start = (char*)(((unsigned int) start) & CPU_CACHE_LINE_MASK);
    while (start < end) {
        flush_dcache_line((unsigned int)start);
        start += CPU_CACHE_LINE_SIZE;
    }
}

#endif
