#include <errno.h>
#include <string.h>

#include <sumeru/constant.h>

#include "os.h"
#include "pwm.h"
#include "spi.h"
#include "spi_lcd.h"

static unsigned char  s_LCD_SPI_CLK_FREQ = LCD_ILI9488_SPI_CLK_FREQ;

#define TRANSCEIVE(u,b,l) \
    spi_transceive(u, (char *)b, l, 0x2 | 0x4);

static void
lcd_set_region(
	unsigned int unit, 
	unsigned int x1, unsigned int y1, 
	unsigned int x2, unsigned int y2)
{
    char buf[5];

    buf[0] = 0x2a;              // col set
    buf[1] = (x1 >> 8) & 0xff;
    buf[2] = x1 & 0xff;
    buf[3] = (x2 >> 8) & 0xff;
    buf[4] = x2 & 0xff;
    TRANSCEIVE(unit, buf, 5);

    buf[0] = 0x2b;              // page set
    buf[1] = (y1 >> 8) & 0xff;
    buf[2] = y1 & 0xff;
    buf[3] = (y2 >> 8) & 0xff;
    buf[4] = y2 & 0xff;
    TRANSCEIVE(unit, buf, 5);
}


static void
lcd_reset(unsigned int reset_pin)
{
    csr_gpio_out_clr(reset_pin);
    os_waitms(250);
    csr_gpio_out_set(reset_pin);
    os_waitms(250);
}


int
init_lcd(
	unsigned int unit,
	unsigned int type, unsigned int depth, 
	unsigned int orient, unsigned int flags,
	unsigned int reset_pin,
	unsigned int backlight_pwm_unit)
{
    char buf[8];

    if (type  == LCD_TYPE_ILI9488)
	s_LCD_SPI_CLK_FREQ = LCD_ILI9488_SPI_CLK_FREQ;
    else
	s_LCD_SPI_CLK_FREQ = LCD_ILI9481_SPI_CLK_FREQ;

    if (spi_set_speed(unit, s_LCD_SPI_CLK_FREQ) == -1)
	return -1;

    pwm_start_unit(backlight_pwm_unit, 99);

    csr_gpio_dir_set(LCD_RESET_PIN);
    csr_gpio_out_set(LCD_RESET_PIN);
    os_waitms(250);

    lcd_reset(reset_pin);

    buf[0] = 0x3a; buf[1] = depth;
    TRANSCEIVE(unit, buf, 2);

    buf[0] = 0x36; buf[1] = orient;
    TRANSCEIVE(unit, buf, 2);

    if (flags & LCD_FLAG_REVERSE_VIDEO)
	TRANSCEIVE(unit, "\x21", 1);

    TRANSCEIVE(unit, "\x11", 1);
    os_waitms(500);
    TRANSCEIVE(unit, "\x29", 1);
    os_waitms(500);

    lcd_set_region(unit, 0, 0, LCD_MAX_ROWS(orient), LCD_MAX_COLS(orient));

    return 0;
}


void
lcd_blit(unsigned int unit,
	    int x1, int y1, int x2, int y2, 
	    const char *buf, unsigned int len)
{
    if ((x1 > x2) || (y1 > y2)) {
	errno = EINVAL;
	return;
    }

    /* XXX opt if last region equals region avoid set region call*/
    /* XXX will stopping refresh help in tearing */

    lcd_set_region(unit, x1, y1, x2, y2);
    TRANSCEIVE(unit, "\x2c", 1);
    TRANSCEIVE(unit, buf, len);
}

    
void
lcd_blit_opt(unsigned int unit,
	    int x1, int y1, int x2, int y2, 
	    const char *buf, unsigned int len)
{
    if ((x1 > x2) || (y1 > y2)) {
	errno = EINVAL;
	return;
    }

    /* XXX opt if last region equals region avoid set region call*/
    /* XXX will stopping refresh help in tearing */

    lcd_set_region(unit, x1, y1, x2, y2);

    /* caller should alloc buf appropriately, buf[-1] is overwritten */
    *((char *)buf - 1) = 0x2c;
    TRANSCEIVE(unit, (buf - 1), (len + 1));
}


void
lcd_set_backlight(
	unsigned int backlight_pwm_unit, 
	unsigned int backlight_duty_cycle)
{
    pwm_start_unit(backlight_pwm_unit, backlight_duty_cycle);
}

