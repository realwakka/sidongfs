#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

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
	printf("%d %s\n", argc, argv[1]);

	lseek(fd, SEEK_SET, SIDONGFS_BLOCK_SIZE);	
	write(fd, "sidongfs", 8);

	got = read(fd, buf, 10);
	printf("%d %s\n", got, buf);
	return 0;
}
