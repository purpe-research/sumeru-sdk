#ifndef __SUMERU_DEVICE_H
#define __SUMERU_DEVICE_H

typedef void (*device_intr_handler_t)(void *act);

typedef struct intr_dispatch_entry
{
    device_intr_handler_t	handler;
    void			*act;
} intr_dispatch_entry_t;

typedef unsigned int 	(*register_read_fn_t)();
typedef void 		(*register_write_fn_t)(unsigned int value);
typedef void 		(*register_bit_set_fn_t)(unsigned int value);
typedef void 		(*register_bit_clr_fn_t)(unsigned int value);

int	dev_alloc_timer(unsigned int unit, intr_dispatch_entry_t *e);
int	dev_alloc_gpio(unsigned int unit, intr_dispatch_entry_t *e);
int	dev_alloc_spi(unsigned int unit, intr_dispatch_entry_t *e);
int	dev_alloc_i2c(unsigned int unit, intr_dispatch_entry_t *e);
int	dev_alloc_uart(unsigned int unit, 
				intr_dispatch_entry_t *e_tx,
				intr_dispatch_entry_t *e_rx);

/*
 * dev_handle_interrupt
 *
 *	intr_id is INTR_ID_1, INTR_ID_2 ... , INTR_ID_15
 *
 *	Handler should return zero if control should return to the
 *	interrupted thread or it should return a thread pointer 
 *	to switch to another thread.
 */

void	dev_handle_interrupt(unsigned int intr_id);

void	timer_subsystem_start();
void	timer_subsystem_stop();

int	uart_subsystem_start();
int	uart_subsystem_stop();

int	pwm_subsystem_start();
int	pwm_subsystem_stop();

#endif
