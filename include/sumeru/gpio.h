#ifndef __SUMERU_DEV_GPIO_H
#define __SUMERU_DEV_GPIO_H

typedef void (*gpio_user_intr_callback_t)();

int	gpio_wait_intr();
int     gpio_set_user_intr_callback(gpio_user_intr_callback_t h);

#endif
