#ifndef __SUMERU_THREAD_MACHINE_STATE_H
#define __SUMERU_THREAD_MACHINE_STATE_H

typedef unsigned int	regtype_t;
struct _reent		*reent;

typedef struct __attribute__((__packed__)) thread_machine_state
{
    /* C calling convention caller-saved-registers and
     * ra, sp, gp, tp  come first
     * NOTE: these declarations should not be reordered
     * the asm code depends on these offsets.
     */
    regtype_t		x1, 		/* ra */
			x2, 		/* sp */
			x3, 		/* gp */
			x4, 		/* tp */
			x5, x6, x7,	/* t0 - t3 */
			x10,		/* a0 */
			x11, x12, x13,	/* a1 - a3 */
			x14, x15, x16,	/* a4 - a6 */
			x17,		/* a7 */
			x28, x29, x30,	/* t3 - t5 */
			x31;		/* t6 */

    /* Callee-saved-registers */
    regtype_t  		x8, x9,		/* s0 - s1 */
			x18, x19, x20,	/* s2 - s4 */
			x21, x22, x23,	/* s5 - s7 */
			x24, x25, x26,	/* s8 - s10 */
			x27;		/* s11 */

    regtype_t		pc;

    unsigned char	*stack;
    struct _reent	*reent;

} thread_machine_state_t;

#endif
