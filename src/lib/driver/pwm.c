#include <errno.h>

#include <sumeru/constant.h>

#include "os.h"
#include "pwm.h"

static unsigned int s_PWM_STATE = 0;

#define MAX_PWM_DEVICES		3

#define MCLK_FREQ(freq) \
    ((((((CPU_CLK_FREQ * 10) / 2 / 128) / freq) + 5) / 10) - 1)

static unsigned int
pwm_duty_cycle(unsigned char percent)
{
    percent = (percent > 100 ? 100 : percent); 

    if (percent == 0)
	return 0x00;
    else if (percent == 100)
	return 0x80;

    return 0x80 | (128 - (((percent * 128) / 100)));
}


int
pwm_subsystem_start()
{
    return pwm_start(PWM_MAX_FREQ / 2);
}


int
pwm_subsystem_stop()
{
    s_PWM_STATE &= 0xffffff7f;
    csr_pwm_write(s_PWM_STATE);
    return 0;
}


int
pwm_start(unsigned int freq)
{
    if (freq > PWM_MAX_FREQ || freq < PWM_MIN_FREQ) {
	errno = EINVAL;
	return -1;
    }

    s_PWM_STATE = (s_PWM_STATE & (~0xff)) | (0x80 | MCLK_FREQ(freq));
    csr_pwm_write(s_PWM_STATE);
    return 0;
}


int
pwm_stop_unit(unsigned int unit)
{
    if (unit >= MAX_PWM_DEVICES) {
	errno = ENOENT;
    	return -1;
    }

    s_PWM_STATE &= ~(0xff << ((unit + 1) * 8));
    csr_pwm_write(s_PWM_STATE);
    return 0;
}

    
int
pwm_start_unit(unsigned int unit, unsigned char duty_cycle_percent)
{
    if (unit >= MAX_PWM_DEVICES) {
	errno = ENOENT;
    	return -1;
    }

    s_PWM_STATE &= ~(0xff << ((unit + 1) * 8));
    s_PWM_STATE |= 
	((0x80 | pwm_duty_cycle(duty_cycle_percent)) <<  ((unit + 1) * 8));
    csr_pwm_write(s_PWM_STATE);
    return 0;
}

