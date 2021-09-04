#ifndef SUMERU_DEV_SPI_LCD_H
#define SUMERU_DEV_SPI_LCD_H

#define LCD_FLAG_REVERSE_VIDEO	(1 << 0)

#define LCD_TYPE_ILI9488	(1 << 0)
#define LCD_TYPE_ILI9481	(1 << 1)

#define LCD_DEPTH_8BPP		0x1
#define LCD_DEPTH_16BPP		0x5
#define LCD_DEPTH_18BPP		0x6

#define LCD_ORIENT_POTRAIT	0x80
#define LCD_ORIENT_LANDSCAPE	0x20

#define LCD_MAX_ROWS(x)		(x == LCD_ORIENT_POTRAIT ? 319 : 479) 
#define LCD_MAX_COLS(x)		(x == LCD_ORIENT_POTRAIT ? 479 : 319) 

#define LCD_BPP_BYTES(x) \
    (x == LCD_DEPTH_8BPP ? 1 : (x == LCD_DEPTH_16BPP ? 2 : 3))

#define LCD_PWM_UNIT   		0

#define LCD_RESET_PIN   	(1 << 2)

#define LCD_ILI9488_SPI_CLK_FREQ	SPI_CLK_SEL_36M
#define LCD_ILI9481_SPI_CLK_FREQ	SPI_CLK_SEL_9M

int     init_lcd(unsigned int unit,
		 unsigned int type, unsigned int depth, 
		 unsigned int orient, unsigned int flags,
		 unsigned int reset_pin, 
		 unsigned int backlight_pwm_unit);

void	lcd_blit(unsigned int unit,
		 int x1, int y1, int x2, int y2, 
		 const char *buf, unsigned int len);

/* 
 * lcd_blit_opt is faster but it overwrites *(buf - 1),
 * use it with appropriately positioned / allocated buffers.
 */

void	lcd_blit_opt(unsigned int unit,
		 int x1, int y1, int x2, int y2, 
		 const char *buf, unsigned int len);

void	lcd_set_backlight(
		unsigned int backlight_pwm_unit,
		unsigned int backlight_duty_cycle);

#endif
