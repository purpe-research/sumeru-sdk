#include <errno.h>

#include <sumeru/cpu/csr.h>
#include <sumeru/cpu/memctl.h>
#include <sumeru/constant.h>

#include "device.h"
#include "gpio.h"

typedef struct gpio_softc 
{
    unsigned int		sc_unit;
    volatile unsigned int	sc_intr_pending;
    gpio_user_intr_callback_t	sc_cb;

} gpio_softc_t;


static gpio_softc_t gpio_softc = { 0, 0, 0 };


int
gpio_set_user_intr_callback(gpio_user_intr_callback_t cb)
{
    gpio_softc.sc_cb = cb;
    return 0;
}


static void 
gpio_intr_handler(gpio_softc_t *sc)
{
    gpio_softc.sc_intr_pending = 0;
    if (gpio_softc.sc_cb)
	(*gpio_softc.sc_cb)();
}


int
dev_alloc_gpio(unsigned int unit, intr_dispatch_entry_t *e)
{
    e->handler = (device_intr_handler_t) gpio_intr_handler;
    e->act = &gpio_softc;
    return 0;
}


int
gpio_wait_intr()
{
    gpio_softc.sc_intr_pending = 1;
    csr_gpio_intrmask_set(1);
    while (gpio_softc.sc_intr_pending == 1)
	;
    return 0;
}
