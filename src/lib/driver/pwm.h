#ifndef SUMERU_DEV_PMW_H
#define SUMERU_DEV_PWM_H

#include <sumeru/cpu/constant.h>

/* 
 * PWM MAX and MIN supported frequencies are defined in 
 * cpu/constant.h
 * 	#define PWM_MAX_FREQ                    281250
 *	#define PWM_MIN_FREQ                    2196
 */

int	pwm_start(unsigned int freq);
int	pwm_stop(unsigned int freq);
int	pwm_stop_unit(unsigned int unit);
int	pwm_start_unit(unsigned int unit, unsigned char duty_cycle_percent);

#endif
