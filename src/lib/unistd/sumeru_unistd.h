#ifndef __SUMERU_UNISTD_H
#define __SUMERU_UNISTD_H

#define FIONREAD	1000

int	sumeru_mount_root_fs();

int	sumeru_mount_fs(
    		char *mp, int flash_dev,
    		int start_block, int end_block,
    		int flash_erase_size, int flash_prog_size);

int	sumeru_umount_fs(char *mp);

int	sumeru_mount_sdcard();
int	sumeru_umount_sdcard();

void	waitms(unsigned int ms);

#endif
