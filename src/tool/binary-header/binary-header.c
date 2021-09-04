#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>
#include <fcntl.h>
#include <err.h>


#define KERNEL_MAGIC	0x0a0d3153


int
main(int argc, char **argv)
{
    if (argc < 2)
	return 0;


    int fd = open(argv[1], O_RDWR, 0);
    if (fd < 0)
	err(1, "Error opening file: %s", argv[1]);

    struct stat sb;
    int sz = fstat(fd, &sb);
    if (sz < 0)
	err(1, "Error stat-ing file");
    lseek(fd, 8, SEEK_SET);
    sz = KERNEL_MAGIC;
    if (write(fd, &sz, 4) != 4)
	err(1, "Error writing kernel magic");
    sz = sb.st_size;
    if (write(fd, &sz, 4) != 4)
	err(1, "Error writing kernel length");
    close(fd);
    return 0;
}
