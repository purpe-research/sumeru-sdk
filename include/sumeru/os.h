#ifndef __SUMERU_OS_H
#define __SUMERU_OS_H

#include <stdint.h>

#include <sumeru/cpu/csr.h>
#include <sumeru/cpu/memctl.h>
#include <sumeru/constant.h>

#include "thread.h"

#define OS_VERSION		"SUMERU-01.00"
#define enable_all_intr()	csr_intr_ctrl_set(1)
#define disable_all_intr()	csr_intr_ctrl_clr(1)

/*
 * Functions os_main and os_handle_interrupt are only called by 
 * the crt0 (startup and interrupt) code.
 */

/*
 * os_main is called with 
 * 	(a) interrupts disabed
 *	(b) GTABLE ICTX_GP, GTABLE_ICTX_REENT and GTABLE_CURTHREAD set
 * 	(c) Thread-0 REENT and STACKMEM set
 *
 * 	Among other things os_main should
 * 		configure devices
 * 		enable interrupts
 * 		start uart services
 * 		start timer service
 * 		call main
 *
 *
 * If os_main returns, control is transferred to the bootloader.
 * 
 */

void		os_main(
			unsigned int crt0_version, 
			unsigned int periph_cfg);


/*
 * os_interrupt_handler
 *
 *	id is INTR_ID_1, INTR_ID_2 ... , INTR_ID_15
 *
 *	Handler should return zero if control should return to the
 *	interrupted thread or it should return a thread pointer 
 *	to switch to another thread.
 */

thread_t*	os_handle_interrupt(unsigned int id);

typedef struct os_timeval {
    uint64_t	tv_sec;
    uint64_t	tv_usec;
} os_timeval_t;

void		os_gettimeofday_r(os_timeval_t *tv);
void		os_waitms(unsigned int ms);

#endif
