#include <sumeru/cpu/csr.h>
#include <sumeru/pwm.h>
#include <sumeru/os.h>
#include <sumeru/timer.h>

#include <stdio.h>

/* Timer callback is called in interrupt context with
 * interrupts disabled, sharing the stack of the interrupted
 * thread.
 */
static void
timer_callback()
{
    static int ctr = 0;

    if (ctr++ & 1)
	csr_gpio_out_clr(1);		/* Turn-on led */
    else
	csr_gpio_out_set(1);
}


int
main(int argc, char **argv)
{
    /* Set LED GPIO (bit 1) line direction to output, turn led-off (1) */
    /* LED will be blinked by timer callback */

    csr_gpio_dir_set(1);
    csr_gpio_out_set(1);

    timer_enable_callback(100, timer_callback);

    /* PWM master clock is by default initialized to 140.625 KHz,
     * pwm_start(freq) can be called to reset it to another value,
     * on the fly.
     * e.g. pwm_start(100000);
     */

    pwm_start_unit(0, 50);	/* 50% duty cycle */
    pwm_start_unit(1, 50);
    pwm_start_unit(2, 50);

    while (1) {
	printf("Hello World!\n");
	os_waitms(1000);
    }

    return 0;
}
