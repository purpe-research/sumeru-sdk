#include <sumeru/cpu/csr.h>
#include <sumeru/constant.h>
#include <sumeru/uart.h>

#include "sumeru_unistd.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/time.h>

#include <yportenv.h>
#include <yaffs_sumeru_glue.h>
#include <yaffsfs.h>

static int s_root_mounted;

#define CHECK_ROOT_MOUNT() 	\
    do { 			\
	if (!s_root_mounted) { 	\
	    errno = ENOENT; 	\
	    return -1;		\
	}			\
    } while (0)


#ifdef SUMERU_FATFS

#define FATFS_START_FD		256
#define FATFS_MAX_OPEN		128

#include <machine/spi_microsd.h>
#include "sumeru_fatfs.h"

static int s_sdcard_mounted;

#define CHECK_SDCARD_MOUNT() 	\
    do { 			\
	if (!s_sdcard_mounted) { \
	    errno = ENOENT; 	\
	    return -1;		\
	}			\
    } while (0)


#define CHECK_GET_FP(fp, fd)		\
    do {				\
	if ( (fp = fat_get_fp(fd)) == 0) { \
	    errno = ENOENT;		\
	    return -1;			\
	}				\
    } while (0)

#endif

#define UART1_FD		3
#define UART2_FD		4


struct _reent g_REENT;

struct _reent*
__getreent (void)
{
    return &g_REENT;
}


void __sinit (struct _reent *);

void __init_reent()
{
    _REENT_INIT_PTR_ZEROED(&g_REENT);
    __sinit(&g_REENT);
}


/********************* FILESYSTEM FUNCTIONS *******************************/
#ifdef SUMERU_FATFS

typedef struct fat_fdmap {
    void 	*ff_fp;
} fat_fdmap_t;

static fat_fdmap_t g_fat_fdmap[FATFS_MAX_OPEN];

static int
fat_alloc_fd()
{
    for (int i = 0; i < FATFS_MAX_OPEN; ++i) {
	if (g_fat_fdmap[i].ff_fp == 0)
	    return i + FATFS_START_FD;
    }
    return -1;
}


__attribute__ ((always_inline))
static inline void
fat_free_fp(int fd)
{
    g_fat_fdmap[fd - FATFS_START_FD].ff_fp = 0;
}


__attribute__ ((always_inline))
static inline void
fat_store_fp(int fd, void *fp)
{
    g_fat_fdmap[fd - FATFS_START_FD].ff_fp = fp;
}


__attribute__ ((always_inline))
static inline void*
fat_get_fp(int fd)
{
    return g_fat_fdmap[fd - FATFS_START_FD].ff_fp;
}

#endif


int
sumeru_mount_root_fs()
{
    int res;

    if (s_root_mounted) {
	errno = EBUSY;
	return -1;
    }

    res = yaffs_sumeru_devconfig(
			"/", 0, 
			FLASH_ROOT_FS_START, 
			FLASH_ROOT_FS_END, 
			ROOT_FS_ERASESIZE, 
			ROOT_FS_PROGSIZE);
    if (res < 0)
	return res;

    res = yaffs_mount("/");
    if (res == 0)
	s_root_mounted = 1;
    
    return res;
}


int
sumeru_mount_fs(
    char *mp, int flash_dev,
    int start_block, int end_block,
    int flash_erase_size, int flash_prog_size)
{
    int res;
    res = yaffs_sumeru_devconfig(
			mp, flash_dev, start_block, end_block,
			flash_erase_size, flash_prog_size);
    if (res < 0)
	return res;

    res = yaffs_mount(mp);
    if (res == 0 && (strcmp(mp, "/") == 0))
	s_root_mounted = 1;

    return res;
}


/********************* FILE FUNCTIONS *******************************/
int
sumeru_umount_fs(char *mp)
{
    int i = 0;

    if (strcmp(mp, "/") == 0) {
    	if (s_root_mounted && ((i =yaffs_unmount(mp)) == 0))
	    s_root_mounted = 0;
    } else {
	i = yaffs_unmount(mp);
    }

    return i;
}


int
_unlink_r(const char *name)
{
    errno = ENOENT;
    return -1;
}


int
_link_r(struct _reent *ptr, const char *oldname, const char *newname)
{
    ptr->_errno = ENOENT;
    return -1;
}


int
_rename_r(struct _reent *ptr, const char *oldname, const char *newname)
{
    ptr->_errno = ENOENT;
    return -1;
}


int
_stat_r(struct _reent *ptr, char *file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}


char *
getcwd(char *__buf, size_t __size)
{
    return 0;
}


int
_open_r(struct _reent *ptr, const char *name, int flags, int mode)
{
#ifdef SUMERU_FATFS
    int fat_fd;
    void *fat_fp;

    if (strncmp(name, "/sdcard/", 8) == 0) {
	CHECK_SDCARD_MOUNT();
	fat_fd = fat_alloc_fd();
	if (fat_fd < 0) {
	    return -1;
	} 
	fat_fp = sumeru_fatfs_open(name + 7, flags, mode);
	if (fat_fp == 0) {
	    ptr->_errno = EINVAL;
	    return -1;
	}
	fat_store_fp(fat_fd, fat_fp);
	return fat_fd;
    }
#endif
    CHECK_ROOT_MOUNT();
    return yaffs_open(name, flags, mode);
}


int
_close_r(struct _reent *ptr, int fd)
{
    if (fd > 15) {
#ifdef SUMERU_FATFS
	if (fd >= FATFS_START_FD) {
	    void *fp;
	    CHECK_SDCARD_MOUNT();
	    CHECK_GET_FP(fp, fd);
	    if (sumeru_fatfs_close(fp) == 0) {
		fat_free_fp(fd);
		return 0;
	    }
	    return -1;
	} else {
#endif
	    CHECK_ROOT_MOUNT();
	    return yaffs_close(fd);
#ifdef SUMERU_FATFS
	}
#endif
    }

    return 0;
}


int
_lseek_r(struct _reent *ptr, int fd, int offset, int whence)
{
    if (fd > 15) {
#ifdef SUMERU_FATFS
	if (fd >= FATFS_START_FD) {
	    void *fp;
	    CHECK_SDCARD_MOUNT();
	    CHECK_GET_FP(fp, fd);
    	    return sumeru_fatfs_seek(fp, offset, whence);
	} else {
#endif
	    CHECK_ROOT_MOUNT();
	    return yaffs_lseek(fd, offset, whence);
#ifdef SUMERU_FATFS
	}
#endif
    }
    /* no seeking for character files */
    return ESPIPE;
}


int
_read_r(struct _reent *ptr, int fd, char *buf, int len) 
{
    if (fd > 15) {
#ifdef SUMERU_FATFS
	if (fd >= FATFS_START_FD) {
	    void *fp;
	    CHECK_SDCARD_MOUNT();
	    CHECK_GET_FP(fp, fd);
    	    return sumeru_fatfs_read(fp, buf, len);
	} else {
#endif
	    CHECK_ROOT_MOUNT();
	    return yaffs_read(fd, buf, len);
#ifdef SUMERU_FATFS
	}
#endif
    }

    if (fd == UART2_FD)
	return uart_read(2, buf, len);
    else if (fd == UART1_FD)
	return uart_read(1, buf, len);
    else if (fd >= 0 && fd < UART1_FD)
	return uart_read(0, buf, len);

    errno = ENOENT;
    return -1;
}


int
_write_r(struct _reent *ptr, int fd, char *buf, int len)
{
    if (fd > 15) {
#ifdef SUMERU_FATFS
	if (fd >= FATFS_START_FD) {
	    void *fp;
	    CHECK_SDCARD_MOUNT();
	    CHECK_GET_FP(fp, fd);
    	    return sumeru_fatfs_write(fp, buf, len);
	} else {
#endif
	    CHECK_ROOT_MOUNT();
    	    return yaffs_write(fd, buf, len);
#ifdef SUMERU_FATFS
	}
#endif
    }

    if (fd == UART2_FD)
	return uart_write(2, buf, len);
    else if (fd == UART1_FD)
	return uart_write(1, buf, len);
    else if (fd >= 0 && fd < UART1_FD)
	return uart_write(0, buf, len);

    errno = ENOENT;
    return -1;
}


int
access(const char *pathname, int mode)
{
  errno = ENOENT;
  return -1;
}


int
_fstat_r(struct _reent *ptr, int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}


int
_isatty_r(struct _reent *ptr, int file)
{
    if (file > 15)
	return 0;

    return 1;
}


int
_kill_r(struct _reent *ptr, int pid, int sig) 
{
  ptr->_errno = EINVAL;
  return -1;
}


int
_getpid_r(struct _reent *ptr)
{
    return 1;
}


void 
_exit(int status)
{
    while (1)
	csr_gpio_out_write(rdtime() >> 23 & 1);
}


caddr_t
_sbrk_r(struct _reent *ptr, int incr)
{
  extern char _end;
  static char *heap_end;
  char *prev_heap_end;
  char *stack_ptr;

  if (heap_end == 0) {
    heap_end = &_end;
  }
  prev_heap_end = heap_end;

  asm("mv %0, sp;" : "=r"(stack_ptr));

  if (heap_end + incr > stack_ptr) {
    _write_r(ptr, 1, "Heap and stack collision\n", 25);
    _exit(1);
  }

  heap_end += incr;
  return (caddr_t) prev_heap_end;
}


int
_gettimeofday_r(struct _reent *ptr, struct timeval *tv, struct timezone *tz)
{
    uint64_t t, z;
    uint32_t x;

    do {
    	x = rdtime_h();
	t = x;
    	t <<= 32;
    	t |= rdtime();
    } while (rdtime_h() != x);

    z = t / CPU_CLK_FREQ;
    tv->tv_sec = z;
    t = t - (z * CPU_CLK_FREQ);
    tv->tv_usec = t / CPU_CLK_TICKS_PER_US;

    return 0;
}

struct tms;

int
_times_r(struct _reent *ptr, struct tms *buf)
{
  return 0;
}


unsigned int
sleep(unsigned int seconds)
{
    waitms(1000 * seconds);
    return 0;
}


char*
ttyname(int fd)
{
    return 0;
}


int
sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    return -1;
}


int
ioctl(int fd, unsigned long request, ...)
{
    return -1;
}


int
stty(int fd, void *arg)
{
    return 0;
}


int
gtty(int fd, void *arg)
{
    return -1;
}


void
waitms(unsigned int ms)
{
    struct timeval tv;
    time_t a, b;

    gettimeofday(&tv, 0);
    a = (tv.tv_sec * 1000000) + tv.tv_usec;
    a /= 1000;
    a += ms;
    while (1) {
        gettimeofday(&tv, 0);
        b = (tv.tv_sec * 1000000) + tv.tv_usec;
        b /= 1000;
        if (b >= a)
            break;
    }
}


unsigned int
alarm(unsigned int secs)
{
    return 0;
}


int
pause(void)
{
    errno = EINTR;
    return -1;
}

//#define MAX_PATHLEN	255
//static char g_curdir[MAX_PATHLEN + 1];	// +1 for trailing \0

int
chdir(const char *path)
{
    errno = ENOENT;
    return -1;
}


void
_abort()
{
    exit(1);
}


/********************* FILESYSTEM FUNCTIONS *******************************/
#ifdef SUMERU_FATFS

int
sumeru_mount_sdcard()
{
    errno = EINVAL;

    if (s_sdcard_mounted) {
	errno = EBUSY;
	return -1;
    }

    if (sumeru_fatfs_init() != 0) {
	return -1;
    }

    s_sdcard_mounted = 1;
    return 0;
}


int
sumeru_umount_sdcard()
{
    if (s_sdcard_mounted) {
	sumeru_fatfs_dnit();
	s_sdcard_mounted = 0;
    }
    return 0;
}

#endif
