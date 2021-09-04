#ifndef __SAKE_CSR_H
#define __SAKE_CSR_H

#include "constant.h"

#define CSR_RW(name, reg)				\
		__attribute__ ((always_inline)) 	\
		inline unsigned int			\
		name##_rw(unsigned int x)		\
		{					\
    		    asm volatile("csrrw %0, %1, %0;" 	\
			: "=r"(x) : "i"(reg));		\
		    return x;				\
		}

#define CSR_READ(name, reg)				\
		__attribute__ ((always_inline)) 	\
		inline unsigned int 			\
		name##_read()				\
		{					\
    		    unsigned int x;			\
		    asm volatile("csrrsi %0, %1, 0;" 	\
			: "=r"(x) : "i"(reg));		\
    		    return x;				\
		}

#define CSR_WRITE(name, reg)				\
		__attribute__ ((always_inline)) 	\
		inline void				\
		name##_write(unsigned int x)		\
		{					\
		    asm volatile("csrrw zero, %1, %0;" 	\
			: : "r"(x), "i"(reg));		\
		}

#define CSR_READ_WV(name, reg, in)			\
		__attribute__ ((always_inline)) 	\
		inline unsigned int 			\
		name##_read()				\
		{					\
    		    unsigned int x = in;	\
		    asm volatile("csrrw %0, %2, %1;" 	\
			: "=r"(x) : "r"(x), "i"(reg));	\
    		    return x;				\
		}

#define CSR_SET(name, reg)				\
		__attribute__ ((always_inline)) 	\
		inline void				\
		name##_set(unsigned int x)		\
		{					\
		    asm volatile("csrrs zero, %1, %0;" 	\
			: : "r"(x), "i"(reg));		\
		}

#define CSR_CLR(name, reg)				\
		__attribute__ ((always_inline)) 	\
		inline void				\
		name##_clr(unsigned int x)		\
		{					\
		    asm volatile("csrrc zero, %1, %0;" 	\
			: : "r"(x), "i"(reg));		\
		}

CSR_READ_WV(csr_uart0_rx, CSR_REG_UART0_RX, 0x80000000)
CSR_READ_WV(csr_uart0_tx, CSR_REG_UART0_TX, 0x80000000)

CSR_WRITE(csr_uart0_rx, CSR_REG_UART0_RX)
CSR_WRITE(csr_uart0_tx, CSR_REG_UART0_TX)

CSR_WRITE(csr_uart0_rx_baud, CSR_REG_UART0_RX_BAUD)
CSR_WRITE(csr_uart0_tx_baud, CSR_REG_UART0_TX_BAUD)

CSR_READ_WV(csr_uart1_rx, CSR_REG_UART1_RX, 0x80000000)
CSR_READ_WV(csr_uart1_tx, CSR_REG_UART1_TX, 0x80000000)

CSR_WRITE(csr_uart1_rx, CSR_REG_UART1_RX)
CSR_WRITE(csr_uart1_tx, CSR_REG_UART1_TX)

CSR_WRITE(csr_uart1_rx_baud, CSR_REG_UART1_RX_BAUD)
CSR_WRITE(csr_uart1_tx_baud, CSR_REG_UART1_TX_BAUD)

CSR_READ_WV(csr_uart2_rx, CSR_REG_UART2_RX, 0x80000000)
CSR_READ_WV(csr_uart2_tx, CSR_REG_UART2_TX, 0x80000000)

CSR_WRITE(csr_uart2_rx, CSR_REG_UART2_RX)
CSR_WRITE(csr_uart2_tx, CSR_REG_UART2_TX)

CSR_WRITE(csr_uart2_rx_baud, CSR_REG_UART2_RX_BAUD)
CSR_WRITE(csr_uart2_tx_baud, CSR_REG_UART2_TX_BAUD)

CSR_WRITE(csr_ivector, CSR_REG_IVECTOR_ADDR)

CSR_READ(csr_timer0_value, CSR_REG_TIMER0_VALUE)
CSR_WRITE(csr_timer0_ctrl, CSR_REG_TIMER0_CTRL)

CSR_WRITE(csr_gpio_dir, CSR_REG_GPIO_DIR)
CSR_SET(csr_gpio_dir, CSR_REG_GPIO_DIR)
CSR_CLR(csr_gpio_dir, CSR_REG_GPIO_DIR)
CSR_READ(csr_gpio_dir, CSR_REG_GPIO_DIR)

CSR_WRITE(csr_gpio_out, CSR_REG_GPIO_OUT)
CSR_SET(csr_gpio_out, CSR_REG_GPIO_OUT)
CSR_CLR(csr_gpio_out, CSR_REG_GPIO_OUT)
CSR_READ(csr_gpio_out, CSR_REG_GPIO_OUT)

CSR_WRITE(csr_gpio_intrmask, CSR_REG_GPIO_INTRMASK)
CSR_SET(csr_gpio_intrmask, CSR_REG_GPIO_INTRMASK)
CSR_CLR(csr_gpio_intrmask, CSR_REG_GPIO_INTRMASK)
CSR_READ(csr_gpio_intrmask, CSR_REG_GPIO_INTRMASK)

CSR_READ(csr_gpio_input, CSR_REG_GPIO_INPUT)

CSR_READ(csr_counter_cycle, CSR_REG_CTR_CYCLE)
CSR_READ(csr_counter_time, CSR_REG_CTR_TIME)
CSR_READ(csr_counter_instret, CSR_REG_CTR_INSTRET)

CSR_READ(csr_counter_cycle_h, CSR_REG_CTR_CYCLE_H)
CSR_READ(csr_counter_cycleh, CSR_REG_CTR_CYCLE_H)
CSR_READ(csr_counter_time_h, CSR_REG_CTR_TIME_H)
CSR_READ(csr_counter_timeh, CSR_REG_CTR_TIME_H)
CSR_READ(csr_counter_instret_h, CSR_REG_CTR_INSTRET_H)
CSR_READ(csr_counter_instreth, CSR_REG_CTR_INSTRET_H)

CSR_READ(csr_ctx_pcsave, CSR_REG_CTX_PCSAVE)
CSR_WRITE(csr_ctx_pcswitch, CSR_REG_CTX_PCSWITCH)
CSR_WRITE(csr_ctx_doswitch, CSR_REG_SWITCH)

CSR_SET(csr_intr_ctrl, CSR_REG_INTR_CTRL)
CSR_CLR(csr_intr_ctrl, CSR_REG_INTR_CTRL)
CSR_WRITE(csr_intr_ctrl, CSR_REG_INTR_CTRL)
CSR_READ(csr_intr_ctrl, CSR_REG_INTR_CTRL)

CSR_WRITE(csr_pwm, CSR_REG_PWM)

CSR_WRITE(csr_spi0_ctrl_1, CSR_REG_SPI0_1)
CSR_WRITE(csr_spi0_ctrl_2, CSR_REG_SPI0_2)

CSR_WRITE(csr_spi1_ctrl_1, CSR_REG_SPI1_1)
CSR_WRITE(csr_spi1_ctrl_2, CSR_REG_SPI1_2)

CSR_WRITE(csr_spi2_ctrl_1, CSR_REG_SPI2_1)
CSR_WRITE(csr_spi2_ctrl_2, CSR_REG_SPI2_2)

CSR_WRITE(csr_spi3_ctrl_1, CSR_REG_SPI3_1)
CSR_WRITE(csr_spi3_ctrl_2, CSR_REG_SPI3_2)

CSR_WRITE(csr_spi4_ctrl_1, CSR_REG_SPI4_1)
CSR_WRITE(csr_spi4_ctrl_2, CSR_REG_SPI4_2)

CSR_WRITE(csr_i2c0_ctrl_1, CSR_REG_I2C0_1)
CSR_WRITE(csr_i2c0_ctrl_2, CSR_REG_I2C0_2)
CSR_READ(csr_i2c0_ctrl_s, CSR_REG_I2C0_S)

CSR_WRITE(csr_i2c1_ctrl_1, CSR_REG_I2C1_1)
CSR_WRITE(csr_i2c1_ctrl_2, CSR_REG_I2C1_2)
CSR_READ(csr_i2c1_ctrl_s, CSR_REG_I2C1_S)

#define rdcycle()	csr_counter_cycle_read()
#define rdtime()	csr_counter_time_read()
#define rdinstret()	csr_counter_instret_read()

#define rdcycle_h()	csr_counter_cycle_h_read()
#define rdtime_h()	csr_counter_time_h_read()
#define rdinstret_h()	csr_counter_instret_h_read()

#endif
