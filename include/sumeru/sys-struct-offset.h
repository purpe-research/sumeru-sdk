#ifndef __SUMERU_STRUCT_OFFSET_H
#define __SUMERU_STRUCT_OFFSET_H

#define GTABLE_BASE_ADDR		0x10

#define GTABLE_SCRATCH_0		(GTABLE_BASE_ADDR + 0)
#define GTABLE_SCRATCH_1		(GTABLE_BASE_ADDR + 4)
#define GTABLE_CURTHREAD		(GTABLE_BASE_ADDR + 8)
#define GTABLE_ICTX_GP			(GTABLE_BASE_ADDR + 12)
#define GTABLE_ICTX_REENT		(GTABLE_BASE_ADDR + 16)

#ifndef _ASSEMBLER
static inline uint32_t 
gtable_get(uint32_t addr)
{
    return *(uint32_t*)addr;
}
#endif


#define TP_OFFSET_PC                	(128 - 4)
#define TP_OFFSET_STACKMEM          	(128 + 0)
#define TP_OFFSET_REENT             	(128 + 4)

#endif

