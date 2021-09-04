#ifndef __SAKE_CONSTANT_H
#define __SAKE_CONSTANT_H

#define CPU_CLK_FREQ			72000000	/* 72 MHz */
#define CPU_CLK_TICKS_PER_US		72

#define CPU_CACHE_LINE_SIZE          	16
#define CPU_CACHE_LINE_MASK          	0xFFFFFFF0

#define CPU_IVEC_MAX			16
#define CPU_PERIPH_MAX			8  /* uart0 and spi0 are hard-wired */

#define CPU_MEM_MAX			(32 * 1048576)	/* 32 MiB */

#define CSR_REG_GPIO_DIR                0x881
#define CSR_REG_GPIO_OUT                0x882
#define CSR_REG_GPIO_INTRMASK           0x883
#define CSR_REG_GPIO_INPUT              0xCC1

#define CSR_REG_TIMER0_CTRL             0x884
#define CSR_REG_TIMER0_VALUE            0xCC2

#define CSR_REG_CTR_CYCLE               0xC00
#define CSR_REG_CTR_CYCLE_H             0xC80
#define CSR_REG_CTR_INSTRET             0xC02
#define CSR_REG_CTR_INSTRET_H           0xC82
#define CSR_REG_CTR_TIME                0xC01
#define CSR_REG_CTR_TIME_H              0xC81


#define CSR_REG_CTX_PCSAVE              0xCC0
#define CSR_REG_CTX_PCSWITCH            0x880
#define CSR_REG_SWITCH                  0x9C0
#define CSR_REG_IVECTOR_ADDR            0x9C1

#define CSR_REG_INTR_CTRL            	0x890
#define CSR_REG_PWM            		0x892

#define CSR_REG_UART0_RX                0x888
#define CSR_REG_UART0_TX                0x889
#define CSR_REG_UART0_RX_BAUD           0x88A
#define CSR_REG_UART0_TX_BAUD           0x88B

#define CSR_REG_UART1_RX                0x898
#define CSR_REG_UART1_TX                0x899
#define CSR_REG_UART1_RX_BAUD           0x89A
#define CSR_REG_UART1_TX_BAUD           0x89B

#define CSR_REG_UART2_RX                0x8B1
#define CSR_REG_UART2_TX                0x8B2
#define CSR_REG_UART2_RX_BAUD           0x8B3
#define CSR_REG_UART2_TX_BAUD           0x8B4

#define CSR_REG_SPI0_1			0x88C
#define CSR_REG_SPI0_2			0x88D

#define CSR_REG_SPI1_1			0x88E
#define CSR_REG_SPI1_2			0x88F

#define CSR_REG_SPI2_1			0x89C
#define CSR_REG_SPI2_2			0x89D

#define CSR_REG_SPI3_1			0x89E
#define CSR_REG_SPI3_2			0x89F

#define CSR_REG_SPI4_1			0x8BC
#define CSR_REG_SPI4_2			0x8BD

#define CSR_REG_I2C0_1			0x8A0
#define CSR_REG_I2C0_2			0x8A1
#define CSR_REG_I2C0_S			0x8A2

#define CSR_REG_I2C1_1			0x8A4
#define CSR_REG_I2C1_2			0x8A5
#define CSR_REG_I2C1_S			0x8A6


#define INTR_ID_1		1
#define INTR_ID_2	        2
#define INTR_ID_3	        3
#define INTR_ID_4               4
#define INTR_ID_5               5
#define INTR_ID_6               6
#define INTR_ID_7           	7
#define INTR_ID_8          	8
#define INTR_ID_9              	9
#define INTR_ID_10             	10
#define INTR_ID_11             	11
#define INTR_ID_12             	12
#define INTR_ID_13             	13
#define INTR_ID_14           	14
#define INTR_ID_15          	15
#define INTR_ID_NONE		0

#define I2C_CLK_SEL_36M			(0 << 0)
#define I2C_CLK_SEL_18M			(1 << 1)
#define I2C_CLK_SEL_9M			(2 << 2)
#define I2C_CLK_SEL_4p5M		(3 << 3)
#define I2C_CLK_SEL_2p25M		(4 << 4)
#define I2C_CLK_SEL_1p125M		(5 << 5)
#define I2C_CLK_SEL_562p5K		(6 << 6)
#define I2C_CLK_SEL_281p125K		(7 << 7)

#define SPI_CLK_SEL_36M			(0 << 0)
#define SPI_CLK_SEL_18M			(1 << 1)
#define SPI_CLK_SEL_9M			(2 << 2)
#define SPI_CLK_SEL_4p5M		(3 << 3)
#define SPI_CLK_SEL_2p25M		(4 << 4)
#define SPI_CLK_SEL_1p125M		(5 << 5)
#define SPI_CLK_SEL_562p5K		(6 << 6)
#define SPI_CLK_SEL_281p125K		(7 << 7)

#define I2C_CLKSEL_SHIFT		28
#define SPI_CLKSEL_SHIFT		28
#define SPI_AUX_SHIFT			25

#define PWM_MAX_FREQ			281250
#define PWM_MIN_FREQ			2196

#endif
