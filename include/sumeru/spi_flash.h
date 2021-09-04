#ifndef __SUMERU_DEV_SPI_FLASH_H
#define __SUMERU_DEV_SPI_FLASH_H

void	erase_spi_flash_ce(unsigned int unit, void *sc);
void	erase_spi_flash_block64k(unsigned int unit, unsigned int flash_addr);
void	erase_spi_flash_sector(unsigned int unit, unsigned int flash_addr);

void    read_spi_flash(unsigned int unit,
		char *dst, unsigned int flash_addr, unsigned int len);

void	write_spi_flash(unsigned int unit,
		char *src, unsigned int flash_addr, unsigned int len);

#endif
