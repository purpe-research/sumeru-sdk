#include "yaffscfg.h"
#include "yaffsfs.h"
#include "yaffs_packedtags2.h"
#include "yaffs_sumeru_glue.h"
#include "yaffs_trace.h"

#include "malloc.h"

#ifndef HOST_SIM_MODE
#include <sumeru/spi.h>
#include <sumeru/spi_flash.h>
#else
#include <fcntl.h>
#include <err.h>
char *HOST_SIM_FILE;
#endif

#ifndef GLUE_ENABLE_PRINTF
#define printf(...)
#endif

#define SPI_FLASH_UNIT		0

typedef unsigned int __u32;

unsigned yaffs_trace_mask = 0;

static int yaffs_errno;


void yaffs_bug_fn(const char *fn, int n)
{
	printf("yaffs bug at %s:%d\n", fn, n);
}

void *yaffsfs_malloc(size_t x)
{
	return malloc(x);
}

void yaffsfs_free(void *x)
{
	free(x);
}

void yaffsfs_SetError(int err)
{
	yaffs_errno = err;
}

int yaffsfs_GetLastError(void)
{
	return yaffs_errno;
}


int yaffsfs_GetError(void)
{
	return yaffs_errno;
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

__u32 yaffsfs_CurrentTime(void)
{
	return 0;
}

void *yaffs_malloc(size_t size)
{
	return malloc(size);
}

void yaffs_free(void *ptr)
{
	free(ptr);
}

void yaffsfs_LocalInitialisation(void)
{
	/* No locking used */
}


static const char *yaffs_file_type_str(struct yaffs_stat *stat)
{
	switch (stat->st_mode & S_IFMT) {
	case S_IFREG: return "regular file";
	case S_IFDIR: return "directory";
	case S_IFLNK: return "symlink";
	default: return "unknown";
	}
}

static const char *yaffs_error_str(void)
{
	int error = yaffsfs_GetLastError();

	if (error < 0)
		error = -error;

	switch (error) {
	case EBUSY: return "Busy";
	case ENODEV: return "No such device";
	case EINVAL: return "Invalid parameter";
	case ENFILE: return "Too many open files";
	case EBADF:  return "Bad handle";
	case EACCES: return "Wrong permissions";
	case EXDEV:  return "Not on same device";
	case ENOENT: return "No such entry";
	case ENOSPC: return "Device full";
	case EROFS:  return "Read only file system";
	case ERANGE: return "Range error";
	case ENOTEMPTY: return "Not empty";
	case ENAMETOOLONG: return "Name too long";
	case ENOMEM: return "Out of memory";
	case EFAULT: return "Fault";
	case EEXIST: return "Name exists";
	case ENOTDIR: return "Not a directory";
	case EISDIR: return "Not permitted on a directory";
	case ELOOP:  return "Symlink loop";
	case 0: return "No error";
	default: return "Unknown error";
	}
}


int yaffs_tracemask(unsigned set, unsigned mask)
{
	if (set)
		yaffs_trace_mask = mask;

	printf("yaffs trace mask: %08x\n", yaffs_trace_mask);
	return 0;
}

static int yaffs_regions_overlap(int a, int b, int x, int y)
{
	return	(a <= x && x <= b) ||
		(a <= y && y <= b) ||
		(x <= a && a <= y) ||
		(x <= b && b <= y);
}


static int
drv_write_chunk(struct yaffs_dev *dev, int nand_chunk,
	const u8 *data, int data_len,
	const u8 *oob, int oob_len)
{
    yaffs_trace(YAFFS_TRACE_OS, "**drv_write_chunk: %d %d %d",
		nand_chunk, data_len, oob_len);

    nand_chunk *= dev->param.total_bytes_per_chunk;

#ifndef HOST_SIM_MODE
    write_spi_flash(SPI_FLASH_UNIT, data, nand_chunk, data_len);
#else
    int fd = (int)dev->driver_context;
    lseek(fd, nand_chunk, SEEK_SET);
    if (write(fd, data, data_len) != data_len)
	return -1;
#endif
    return YAFFS_OK;
}


static int 
drv_read_chunk(struct yaffs_dev *dev, int nand_chunk,
	u8 *data, int data_len,
	u8 *oob, int oob_len,
	enum yaffs_ecc_result *ecc_result)
{
    yaffs_trace(YAFFS_TRACE_OS, "**drv_read_chunk: %d %d %d",
		nand_chunk, data_len, oob_len);

    nand_chunk *= dev->param.total_bytes_per_chunk;

#ifndef HOST_SIM_MODE
    read_spi_flash(SPI_FLASH_UNIT, data, nand_chunk, data_len);
#else
    int fd = (int)dev->driver_context;
    lseek(fd, nand_chunk, SEEK_SET);
    if (read(fd, data, data_len) != data_len)
	return -1;
#endif

    *ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
    return YAFFS_OK;
}


static int
drv_erase(struct yaffs_dev *dev, int block_no)
{
    yaffs_trace(YAFFS_TRACE_OS, "**drv_erase: %d", block_no);

#ifndef HOST_SIM_MODE
    erase_spi_flash_block64k(SPI_FLASH_UNIT,
			    block_no *
				dev->param.chunks_per_block *
				dev->param.total_bytes_per_chunk);
#else
    int sz = dev->param.chunks_per_block * dev->param.total_bytes_per_chunk;
    char *buf = malloc(sz);
    if (!buf)
	return -1;
    memset(buf, 0xff,sz);
    int fd = (int)dev->driver_context;
    lseek(fd, sz * block_no, SEEK_SET);
    if (write(fd, buf, sz) != sz)
	return -1;
#endif

    return YAFFS_OK;
}


static int
drv_mark_bad(struct yaffs_dev *dev, int block_no)
{
    yaffs_trace(YAFFS_TRACE_OS, "**drv_mark_bad: %d", block_no);
    return YAFFS_OK;
}


static int
drv_check_bad(struct yaffs_dev *dev, int block_no)
{
    yaffs_trace(YAFFS_TRACE_OS, "**drv_check_bad: %d", block_no);
    return YAFFS_OK;
}


static int
drv_initialise(struct yaffs_dev *dev)
{
    yaffs_trace(YAFFS_TRACE_OS, "**drv_initialise");
    return YAFFS_OK;
}


static int
drv_deinitialise(struct yaffs_dev *dev)
{
    yaffs_trace(YAFFS_TRACE_OS, "**drv_deinitialise");
    return YAFFS_OK;
}


int yaffs_sumeru_devconfig(char *_mp, int flash_dev,
			int start_block, int end_block,
			int flash_erase_size, int flash_prog_size)
{
	struct yaffs_dev *dev = NULL;
	struct yaffs_dev *chk;
	char *mp = NULL;

	dev = calloc(1, sizeof(*dev));
	mp = strdup(_mp);

	if (!dev || !mp) {
		/* Alloc error */
		printf("Failed to allocate memory\n");
		goto err;
	}

	if (end_block < start_block) {
		printf("Bad start/end\n");
		goto err;
	}

	/* Check for any conflicts */
	yaffs_dev_rewind();
	while (1) {
		chk = yaffs_next_dev();
		if (!chk)
			break;
		if (strcmp(chk->param.name, mp) == 0) {
			printf("Mount point name already used\n");
			goto err;
		}
#if 0 /* XXX need to revamp check for sumeru */
		if (chk->driver_context == mtd &&
			yaffs_regions_overlap(
				chk->param.start_block, chk->param.end_block,
				start_block, end_block)) {
			printf("Region overlaps with partition %s\n",
				chk->param.name);
			goto err;
		}
#endif
	}

	/* Seems sane, so configure */
	memset(dev, 0, sizeof(*dev));
	dev->param.name = mp;

#ifdef HOST_SIM_MODE
	int fd;
	if ( (fd = open(HOST_SIM_FILE, O_RDWR, 0)) < 0) {
	    err(1, "Error opening HOST_SIM_FILE: %s\n", HOST_SIM_FILE);
	}
	warnx("HOST_SIM_FILE: %s", HOST_SIM_FILE);
	dev->driver_context = (int)fd;
#endif

	dev->param.start_block = start_block;
	dev->param.end_block = end_block;
	dev->param.chunks_per_block = flash_erase_size / flash_prog_size;
	dev->param.total_bytes_per_chunk = flash_prog_size;
	dev->param.is_yaffs2 = 1;
	dev->param.use_nand_ecc = 0;
	dev->param.n_reserved_blocks = 5;
	dev->param.inband_tags = 1;
	dev->param.n_caches = 10;

	dev->drv.drv_write_chunk_fn = drv_write_chunk;
    	dev->drv.drv_read_chunk_fn = drv_read_chunk;
	dev->drv.drv_erase_fn = drv_erase;
	dev->drv.drv_mark_bad_fn = drv_mark_bad;
	dev->drv.drv_check_bad_fn = drv_check_bad;
	dev->drv.drv_initialise_fn = drv_initialise;
	dev->drv.drv_deinitialise_fn = drv_deinitialise;


	yaffs_add_device(dev);

	printf("Configures yaffs mount %s: dev %d start block %d, end block %d %s\n",
		mp, flash_dev, start_block, end_block,
		dev->param.inband_tags ? "using inband tags" : "");
	return 0;

err:
	free(dev);
	free(mp);
	return -1;
}


int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request)
{
        (void) size;
        (void) write_request;

        if(!addr)
                return -1;
        return 0;
}


int yaffs_dev_ls(void)
{
	struct yaffs_dev *dev;
	int flash_dev;
	int free_space;

	yaffs_dev_rewind();

	while (1) {
		dev = yaffs_next_dev();
		if (!dev)
			break;
		/*flash_dev =
			((unsigned) dev->driver_context - (unsigned) nand_info)/
				sizeof(nand_info[0]);*/
		printf("%-10s %5d 0x%05x 0x%05x %s",
			dev->param.name, flash_dev,
			dev->param.start_block, dev->param.end_block,
			dev->param.inband_tags ? "using inband tags, " : "");

		free_space = yaffs_freespace(dev->param.name);
		if (free_space < 0)
			printf("not mounted\n");
		else
			printf("free 0x%x\n", free_space);

	}

	return 0;
}

#ifdef HOST_SIM_MODE

int
main(int argc, char **argv)
{
    int i;

    if (argc < 2)
        errx(1, "Usage: %s <image-file> <specs>...", argv[0]);

    HOST_SIM_FILE = argv[1];

    //yaffs_tracemask(1, ~0);
    if (yaffs_sumeru_devconfig("/", 0, 0, 15, 65536, 256) < 0 || 
	yaffs_mount("/")) 
    {
        errx(1, "Error mounting filesystem, /");
        return 0;
    }

    for (i = 2; i < (argc - 1); i += 2) {

	if (strlen(argv[i]) == 1 && argv[i][0] == '.') {
	    warnx("Creating  directory, target = %s", argv[i+1]);
	    if (yaffs_mkdir(argv[i+1], S_IREAD | S_IWRITE | S_IEXEC) < 0)
		errx(1, "Error creating directory");
	} else {
	    warnx("Copying file, local = %s, target = %s", argv[i], argv[i+1]);

   	    int fd = open(argv[i], O_RDONLY, 0);
	    if (fd < 0)
	        err(1, "Error opening input file: %s", argv[i]);

	    int yfd = yaffs_open(argv[i+1], 
			O_TRUNC | O_RDWR | O_CREAT, 
			S_IREAD | S_IWRITE);

	    if (yfd < 0)
	        err(1, "Error opening output file: %s", argv[i+1]);

	    char buf[512];
	    int c;
	    while ( (c = read(fd, buf, 512)) > 0) {
	        if (yaffs_write(yfd, buf, c) != c)
	    	    err(1, "Error short write on output file: %d", c);
	        if (c != 512)
		    break;
	    }
	    if (c < 0)
	        err(1, "Error writing to output file");
	    close(fd);
	    yaffs_close(yfd);
	}
    }
    return 0;
}

#endif
