#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "sidongfs.h"

#define SIDONGFS_BLOCK_SIZE 1024

int main(int argc, char** argv) {
	int fd;
	char buf[10];
	int got;

	if (argc < 2)
		return 1;

	fd = open(argv[1], O_RDWR);
	if (fd < 0)
		perror("Error :");

	got = lseek(fd, SIDONGFS_BLOCK_SIZE, SEEK_SET);
	printf("lseek returns %d \n", got);	

	struct sidong_super_block sb;
	sprintf(sb.magic, "hello sidong!");
	sb.version=17;
	got = write(fd, &sb, sizeof(sb));
	close(fd);
	
	fd = open(argv[1], O_RDWR);
	lseek(fd, SIDONGFS_BLOCK_SIZE, SEEK_SET);
	printf("lseek returns %d \n", got);		
	got = read(fd, &sb, sizeof(sb));
	printf("%d asdf %s\n", got, sb.magic);

	close(fd);
	return 0;
}
