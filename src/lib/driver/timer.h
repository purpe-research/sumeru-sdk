#ifndef __SUMERU_DEV_TIMER_H
#define __SUMERU_DEV_TIMER_H

typedef void (*timer_callback_t)(void);

timer_callback_t 	timer_enable_callback(
				unsigned int ticks, timer_callback_t cb);


#define MAX_POLL_IDX	1

typedef void (*timer_poll_fn_t)(void);

/*
 * Only 2 poll slots are available, passing a null pointer
 * in the f parameter will reset the slot and stop polling
 */

int	timer_set_poller(unsigned int idx, timer_poll_fn_t f);

#endif
