#include <sumeru/constant.h>

#include "spi.h"
#include "spi_flash.h"

#define TRANSCEIVE(u,d,l,a) \
    spi_transceive(u, d, l, a)

void
read_spi_flash(
	unsigned int unit,
	char *dst, unsigned int flash_addr, unsigned int len)
{
    char buf[4];
    char *p = (char *)&flash_addr;

    buf[0] = 0x3;
    buf[1] = p[2];
    buf[2] = p[1];
    buf[3] = p[0];
    TRANSCEIVE(unit, buf, 4, 0x0 | 0x4);
    TRANSCEIVE(unit, dst, len, 0x2);
}


static void
spi_waitbusy(unsigned int unit)
{
    char buf[2];

    buf[1] = 1; 
    while (buf[1] & 1) {
    	buf[0] = 0x05;
    	TRANSCEIVE(unit, buf, 2, 0x2);
    }
}


void
erase_spi_flash_chip(unsigned int unit)
{
    char buf[1];

    buf[0] = 0x06;
    TRANSCEIVE(unit, buf, 1, 0x2 | 0x4);
    buf[0] = 0x60;
    TRANSCEIVE(unit, buf, 1, 0x2 | 0x4);
    spi_waitbusy(unit);   
}


void
erase_spi_flash_block64k(unsigned int unit, unsigned int flash_addr)
{
    char buf[4];
    char *p = (char *)&flash_addr;

    buf[0] = 0x06;
    TRANSCEIVE(unit, buf, 1, 0x2 | 0x4);
    buf[0] = 0xd8;
    buf[1] = p[2];
    buf[2] = p[1];
    buf[3] = p[0];
    TRANSCEIVE(unit, buf, 4, 0x2 | 0x4);
    spi_waitbusy(unit); 
}


void
erase_spi_flash_sector(unsigned int unit, unsigned int flash_addr)
{
    char buf[4];
    char *p = (char *)&flash_addr;

    buf[0] = 0x06;
    TRANSCEIVE(unit, buf, 1, 0x2 | 0x4);
    buf[0] = 0x20;
    buf[1] = p[2];
    buf[2] = p[1];
    buf[3] = p[0];
    TRANSCEIVE(unit, buf, 4, 0x2 | 0x4);
    spi_waitbusy(unit); 
}


void
write_spi_flash(unsigned int unit, char *src, unsigned int flash_addr, unsigned int len)
{
    char buf[4];
    char *p = (char *)&flash_addr;

    buf[0] = 0x06;
    TRANSCEIVE(unit, buf, 1, 0x2 | 0x4);
    buf[0] = 0x02;
    buf[1] = p[2];
    buf[2] = p[1];
    buf[3] = p[0];
    TRANSCEIVE(unit, buf, 4, 0x0 | 0x4);
    TRANSCEIVE(unit, src, len, 0x2 | 0x4);
    spi_waitbusy(unit);   
}
