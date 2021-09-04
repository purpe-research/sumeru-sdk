#include <errno.h>

#include <sumeru/cpu/csr.h>
#include <sumeru/cpu/memctl.h>
#include <sumeru/constant.h>

#include "device.h"
#include "timer.h"

typedef struct timer_softc 
{
    unsigned int		sc_unit;
    unsigned int 		sc_max_ticks;
    unsigned int		sc_cur_ticks;
    timer_callback_t 		sc_callback;
} timer_softc_t;


static timer_softc_t timer_softc = { 0, 0, 0, 0 };

static void
no_poll(void)
{ }


static timer_poll_fn_t poll_fn[MAX_POLL_IDX + 1] = {
    no_poll,
    no_poll
};


int
timer_set_poller(unsigned int idx, timer_poll_fn_t f)
{
    if (idx > MAX_POLL_IDX) {
	errno = ENOENT;
	return -1;
    }
    poll_fn[idx] = (f == 0 ? no_poll : f);
    return 0;
}


static void 
timer_intr_handler(timer_softc_t *sc)
{
    csr_timer0_ctrl_write(0);
    /* optimization -- we have only 2 polls, call directly */
    poll_fn[0]();
    poll_fn[1]();
    if (timer_softc.sc_max_ticks &&
	++timer_softc.sc_cur_ticks >= timer_softc.sc_max_ticks)  
    {
	timer_softc.sc_cur_ticks = 0;
	sc->sc_callback();
    }
    csr_timer0_ctrl_write(TIMER_INTERVAL_TICKS | 0xf);
}


int
dev_alloc_timer(unsigned int unit, intr_dispatch_entry_t *e)
{
    e->handler = (device_intr_handler_t) timer_intr_handler;
    e->act = &timer_softc;
    return 0;
}


void
timer_subsystem_start()
{
    timer_softc.sc_callback = 0;
    timer_softc.sc_max_ticks = 0;
    csr_timer0_ctrl_write(TIMER_INTERVAL_TICKS | 0xf);
}


void
timer_subsystem_stop()
{
    timer_softc.sc_callback = 0;
    timer_softc.sc_max_ticks = 0;
    csr_timer0_ctrl_write(0);
}


timer_callback_t
timer_enable_callback(unsigned int ticks, timer_callback_t cb)
{
    timer_callback_t old = timer_softc.sc_callback;

    if (!cb) {
	ticks = 0;
    }

    timer_softc.sc_cur_ticks = 0;
    timer_softc.sc_max_ticks = ticks;
    timer_softc.sc_callback = cb;

    return old;
}
