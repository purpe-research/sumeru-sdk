#include <string.h>

#include <sumeru/constant.h>

#include "os.h"
#include "spi.h"
#include "spi_microsd.h"


#define MICROSD_CMD_RESET	0x40
#define MICROSD_CMD_CHKVOLTAGE	0x48
#define MICROSD_CMD_ACMD	0x77
#define MICROSD_CMD_INIT	0x69

#define MICROSD_CMD_READOCR	0x7a

#define MICROSD_CMD_READBLK	0x51
#define MICROSD_CMD_WRITEBLK	0x58

#define CRC_CMD_RESET		0x95
#define CRC_CMD_CHKVOLTAGE	0x87
#define CRC_CMD_ACMD		0x65
#define CRC_CMD_INIT		0x77
#define CRC_CMD_READOCR		0xfd

#define CMD_CHKVOLTAGE_ARG	0x000001AA
#define CMD_INIT_HCSARG		0x40000000

#define CMD_INIT_RETRIES	100
#define CMD_READ_RETRIES	100
#define CMD_WRITE_RETRIES	100

#define TRANSCEIVE(u,d,l,a) \
    spi_transceive(u, d, l, a)


    static inline void
microsd_create_cmd(
    unsigned char cmd, 
    unsigned int arg, 
    unsigned char crc,
    char *buf, int buf_sz)
{
    unsigned char *ptr = (unsigned char *)&arg;

    memset(buf, 0xff, buf_sz);
    buf[0] = cmd, 
    buf[1] = ptr[3];
    buf[2] = ptr[2];
    buf[3] = ptr[1];
    buf[4] = ptr[0];
    buf[5] = crc;
}


int
microsd_init(unsigned int unit, unsigned char speed)
{
    char buf[16];
    int retries;

    if (spi_set_speed(unit, speed) == -1)
	return -1;

    /* Dummy Clock */
    memset(buf, 0xff, 10);
    TRANSCEIVE(unit, buf, 10, 3);

    /* Reset */
    microsd_create_cmd(MICROSD_CMD_RESET, 0, CRC_CMD_RESET, buf, 8);
    TRANSCEIVE(unit, buf, 8, 2);
    if (buf[7] != 0x01)
	return -2;

    /* Check for SDC V2 */
    microsd_create_cmd(
	    MICROSD_CMD_CHKVOLTAGE, CMD_CHKVOLTAGE_ARG, CRC_CMD_CHKVOLTAGE, 
	    buf, 12);
    TRANSCEIVE(unit, buf, 12, 2);
    if (buf[7] != 0x01 || buf[10] != 0x01 || buf[11] != 0xAA)
	return -3;

    retries = CMD_INIT_RETRIES;

    while (retries > 0) {
	--retries;
    	microsd_create_cmd(MICROSD_CMD_ACMD, 0x00000000, CRC_CMD_ACMD, buf, 8);
	TRANSCEIVE(unit, buf, 8, 2);
    	if (buf[7] != 0x1)
	    return -4;
    	
    	microsd_create_cmd(
	    MICROSD_CMD_INIT, CMD_INIT_HCSARG, CRC_CMD_INIT, buf, 8);
	TRANSCEIVE(unit, buf, 8, 2);

	if (buf[7] == 0x00) {
	    break;
	} else if (buf[7] != 0x01)
	    return -5;

	os_waitms(10);
    }

    if (retries == 0)
    	return -6;

    microsd_create_cmd(MICROSD_CMD_READOCR, 0, 0, buf, 12);
    TRANSCEIVE(unit, buf, 12, 2);
    if ((buf[8] & 0xc0) != 0xc0)
	return -7;

    return 0; 
}


int
microsd_write(unsigned int unit, unsigned int sector, const char *data)
{
    char buf[16];
    int retries;

    microsd_create_cmd(MICROSD_CMD_WRITEBLK, sector, 0xff, buf, 10);
    buf[9] = 0xfe;
    TRANSCEIVE(unit, buf, 10, 0);
    if (buf[7] != 0) {
	return -1;
    }

    TRANSCEIVE(unit, (char *)data, 512, 4);

    buf[0] = buf[1] = buf[2] = 0xff;
    TRANSCEIVE(unit, buf, 3, 2);

    if ((buf[2] & 0x1f) != 5)
    	return -2;

    retries = CMD_WRITE_RETRIES;
    while (retries > 0) {
	--retries;
	os_waitms(1);
    	buf[0] = 0xff;
	TRANSCEIVE(unit, buf, 1, 2);
	if (buf[0] != 0)
	    return 0;
    }

    return -3;
}


int
microsd_read(unsigned int unit, unsigned int sector, char *data)
{
    char buf[16];
    int retries;

    /* Read directly to data buffer */
    memset(data, 0xff, 512);

    microsd_create_cmd(MICROSD_CMD_READBLK, sector, 0xff, buf, 8);
    TRANSCEIVE(unit, buf, 8, 0);
    if (buf[7] == 0) {
	retries = CMD_READ_RETRIES;
	while (retries > 0) {
	    os_waitms(1);
	    buf[0] = 0xff;
	    TRANSCEIVE(unit, buf, 1, 0);
	    if (buf[0] == 0xfe)
		break;
	    --retries;
    	};

	if (buf[0] != 0xfe)
	    return -2;
    } else {
	return -1;
    }

    TRANSCEIVE(unit, data, 512, 0);
    buf[0] = buf[1] = 0xff;
    TRANSCEIVE(unit, buf, 2, 2);

    return 0;
}


int
microsd_write_multiple(unsigned int unit,
	unsigned int sector, const char *data, unsigned count)
{
    unsigned int i;

    for(i = 0; i < count; ++i)
	if (microsd_write(unit, sector + i, data + (i * 512)) != 0)
	    return 0;

    return 1;
}


int
microsd_read_multiple(unsigned int unit,
	unsigned int sector, char *data, unsigned count)
{
    unsigned int i;

    for(i = 0; i < count; ++i)
	if (microsd_read(unit, sector + i, (char *)data + (i * 512)) != 0)
	    return 0;

    return 1;
}

