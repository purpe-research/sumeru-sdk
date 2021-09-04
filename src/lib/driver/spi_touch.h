#ifndef SUMERU_SPI_TOUCH_H
#define SUMERU_SPI_TOUCH_H

#define TOUCH_MAX_X		320
#define TOUCH_MAX_Y		480

#define TOUCH_STATE_PRESSED    	(1 << 0)        
#define TOUCH_STATE_DRAG    	(1 << 1)        


typedef void (*touch_callback_t)(int x, int y, unsigned int state);

int	touch_start(unsigned int unit, touch_callback_t cb);
void	touch_stop();

#endif
