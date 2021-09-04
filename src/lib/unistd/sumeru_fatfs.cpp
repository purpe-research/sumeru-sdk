#include <common/BlockDeviceInterface.h>
#include <FsLib/FsLib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
extern "C" {
#include <sumeru/spi_microsd.h>
}

#include "sumeru_fatfs.h"

static unsigned int s_unit;

#if 0
class sdfat_printer : public print_t
{
 public:
    size_t write(uint8_t b);
    size_t write(const uint8_t *buf, size_t size);
};

size_t
sdfat_printer::write(uint8_t b)
{
    return ::write(1, &b, 1);
}

size_t
sdfat_printer::write(const uint8_t *buf, size_t size)
{
    return ::write(1, buf, size);
}
#endif


class SumeruMicroSD : public BlockDeviceInterface { 
public:
	bool  isBusy();
	bool  readSector(uint32_t sector, uint8_t *dst);
	bool  readSectors(uint32_t sector, uint8_t *dst, size_t ns);
	uint32_t sectorCount();
	bool  syncDevice();
	bool  writeSector(uint32_t sector, const uint8_t *src);
	bool  writeSectors (uint32_t sector, const uint8_t *src, size_t ns);
};

bool
SumeruMicroSD::isBusy()
{ 
    return false;
}


bool
SumeruMicroSD::readSector(uint32_t sector, uint8_t *dst)
{
    return microsd_read(s_unit, sector, (char*)dst) == 0;
}

bool
SumeruMicroSD::readSectors(uint32_t sector, uint8_t *dst, size_t ns)
{
    return microsd_read_multiple(s_unit, sector, (char *)dst, ns) == 0;
}

uint32_t
SumeruMicroSD::sectorCount()
{
    return 0; /* TODO read from CSD */
}


bool
SumeruMicroSD::syncDevice()
{
    return true;
}

bool
SumeruMicroSD::writeSector(uint32_t sector, const uint8_t *src)
{
    /* SDFat multi sector support does not work :( -- FIX */
    return microsd_write(s_unit, sector, (const char*)src) == 0;
}

bool
SumeruMicroSD::writeSectors (uint32_t sector, const uint8_t *src, size_t ns)
{
    return microsd_write_multiple(s_unit, sector, (char *)src, ns) == 0;
}

static SumeruMicroSD s_sd;
static FsVolume *s_vol;

int
sumeru_fatfs_init(unsigned int unit, unsigned char speed)
{
    s_unit = unit;
    if (microsd_init(unit, speed) != 0) {
        return -1;
    }

    if ( (s_vol = new FsVolume) == 0) {
	errno = ENOMEM;
	return -1;
    }

    if (!s_vol->begin(&s_sd)) {
	return -1;
    }

    return 0;
}


void
sumeru_fatfs_dnit()
{
    if (s_vol) {
	s_vol->end();
	delete s_vol;
	s_vol = 0;
    }
}


void*
sumeru_fatfs_open(const char *name, int flags, int mode)
{
    FsFile *fp = new FsFile;
    *fp = s_vol->open(name, flags);
    return fp;
}


int
sumeru_fatfs_close(void *arg)
{
    FsFile *fp = (FsFile *)arg;

    bool r = fp->close();
    if (r) {
	delete fp;
	return 0;
    }
    return -1;
}


int
sumeru_fatfs_read(void *arg, char *buf, int len)
{
    FsFile *fp = (FsFile *)arg;
    return fp->read(buf, len);
}


int
sumeru_fatfs_write(void *arg, char *buf, int len)
{
    FsFile *fp = (FsFile *)arg;
    return fp->write(buf, len);
}


int
sumeru_fatfs_seek(void *arg, int offset, int whence)
{
    bool r = false;

    FsFile *fp = (FsFile *)arg;
    switch (whence) {
	case SEEK_SET:
	    r = fp->seek(offset);
	    break;
	case SEEK_CUR:
	    r = fp->seekCur(offset);
	    break;
	case SEEK_END:
	    r = fp->seekEnd(offset);
	    break;
    }
    if (!r)
	return -1;

    return fp->curPosition();
}
