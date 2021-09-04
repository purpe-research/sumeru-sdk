#ifndef __SUMERU_FATFS_H
#define __SUMERU_FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

    int		sumeru_fatfs_init();
    void	sumeru_fatfs_dnit();
    void*	sumeru_fatfs_open(const char *name, int flags, int mode);
    int		sumeru_fatfs_close(void *fd);
    int		sumeru_fatfs_read(void *fp, char *buf, int len);
    int		sumeru_fatfs_write(void *fp, char *buf, int len);
    int		sumeru_fatfs_seek(void *fp, int offset, int whence);


#ifdef __cplusplus
}
#endif

#endif
