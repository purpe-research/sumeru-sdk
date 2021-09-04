#ifndef SUMERU_DEV_SPI_MICROSD_H
#define SUMERU_DEV_SPI_MICROSD_H

int  	microsd_init(unsigned int unit, unsigned char speed);

int	microsd_write(unsigned int unit, unsigned int sector, const char *data);
int	microsd_read(unsigned int unit, unsigned int sector, char *data);

int	microsd_write_multiple(
		unsigned int unit,
		unsigned int sector, const char *data, unsigned int count);

int	microsd_read_multiple(
		unsigned int unit,
		unsigned int sector, char *data, unsigned int count);

#endif
