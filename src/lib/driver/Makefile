include ${SUMERU_DIR}/conf/sumeru-pre.mk

LIB = libsumeru_driver.a

CFLAGS += -Werror
CFILES = $(wildcard *.c)

INCLUDE_FILES = os.h thread.h device.h gpio.h i2c.h spi.h timer.h uart.h pwm.h
INCLUDE_FILES += spi_flash.h spi_microsd.h spi_lcd.h spi_touch.h
INCLUDE_FILES += thread_machine_state.h

INCLUDE_INSTALL_PREFIX = sumeru

include ${SUMERU_DIR}/conf/sumeru-lib-post.mk
