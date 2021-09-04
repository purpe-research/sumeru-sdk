
#ifndef __YAFFS_SUMERU_GLUE_H__
#define __YAFFS_SUMERU_GLUE_H__


int yaffs_tracemask(unsigned set, unsigned mask);
int yaffs_sumeru_devconfig(char *mp, int flash_dev,
				int start_block, int end_block,
				int flash_erase_size, int flash_prog_size);
int yaffs_dev_ls(void);

#endif
