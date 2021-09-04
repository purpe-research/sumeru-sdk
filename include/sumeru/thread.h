#ifndef __SUMERU_THREAD_H
#define __SUMERU_THREAD_H

#include "thread_machine_state.h"

typedef struct __attribute__((__packed__)) thread
{
    /* crt0 expects tms to be at offset 0 */
    thread_machine_state_t	tms;

    unsigned int	priority;

    struct thread	*runq_next;
    struct thread	*runq_prev;

    struct thread	*t_next;
    struct thread	*t_prev;

} thread_t;

#endif
