#ifndef __SAKE_PERIPH_H
#define __SAKE_PERIPH_H

#define PERIPH_NONE		0
#define PERIPH_UART		1
#define PERIPH_SPI		2
#define PERIPH_I2C		3
#define PERIPH_PSEUDO		7

#define CPU_PERIPH_VERSION	1

/* 
 * Upon boot the CPU will store the Peripheral configuration word (32 bits)
 * at this address.
 */
#define CPU_PERIP_ADDR		0x8

/* 
 * CPU_PERIPH_MARK8_DEFAULT = 0x12da48a 
 */ 
#define CPU_PERIPH_MARK8_DEFAULT ((CPU_PERIPH_VERSION << 24) | \
	(PERIPH_UART << 21) | \
	(PERIPH_I2C << 18) | \
	(PERIPH_I2C << 15) | \
	(PERIPH_SPI << 12) | \
	(PERIPH_SPI << 9) | \
	(PERIPH_SPI << 6) | \
	(PERIPH_UART << 3) | \
	(PERIPH_SPI << 0))

/*
 * The CPU always has one UART (console) and one SPI (flash) bus
 */
#define MARK8_MIN_DEVUNIT_UART	1
#define MARK8_MIN_DEVUNIT_SPI	1
#define MARK8_MIN_DEVUNIT_I2C	0

#endif
