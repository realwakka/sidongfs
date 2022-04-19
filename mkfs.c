#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#include "sidongfs.h"

#define SIDONGFS_BLOCK_SIZE 1024
#define SIDONGFS_ROOT_INO 1
#define SIDONGFS_MAX_INODE_CNT 1024 / 32

void write_root_inode(int fd)
{
	struct sidongfs_inode inode;

	// write a dummy node
	write(fd, &inode, sizeof(inode));

	inode.i_mode = S_IFDIR + 0755;
	inode.i_uid = getuid();
	inode.i_size = 32;
	inode.i_time = time(NULL);
	inode.i_gid = getgid();
	inode.i_nlinks = 2; // why?
	write(fd, &inode, sizeof(inode));	
}

void set_bit(char map[SIDONGFS_BLOCK_SIZE], int offset, bool value)
{
	int nbyte = offset / 8;
	int nbit = offset % 8;
	map[nbyte] |= 1U << nbit;
}

bool get_bit(char map[SIDONGFS_BLOCK_SIZE], int offset)
{
	int nbyte = offset / 8;
	int nbit = offset % 8;
	return (map[nbyte] >> nbit) & 1U;
}

int main(int argc, char** argv)
{
	int fd;
	char buf[10];
	int got;
	char imap[SIDONGFS_BLOCK_SIZE];
	char zmap[SIDONGFS_BLOCK_SIZE];	

	if (argc < 2) {
		fprintf(stderr, "No block device\n");
		return 1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		perror("Error :");
		return 1;
	}

	// BOOT | SUPER | IMAP | ZMAP | INODES | ZONES	
	got = lseek(fd, SIDONGFS_BLOCK_SIZE, SEEK_SET);

	// write super block
	printf("lseek returns %d \n", got);	
	struct sidongfs_super_block sb;
	sprintf(sb.magic, "hello sidong!");
	sb.version=17;
	got = write(fd, &sb, sizeof(sb));

	// write inode map
	got = lseek(fd, SIDONGFS_BLOCK_SIZE*2, SEEK_SET);
	set_bit(imap, SIDONGFS_ROOT_INO, 1);

	// write zone map
	got = lseek(fd, SIDONGFS_BLOCK_SIZE*3, SEEK_SET);
	set_bit(imap, 0, 1);

	got = lseek(fd, SIDONGFS_BLOCK_SIZE*4, SEEK_SET);	
	write_root_inode(fd);
	close(fd);
	return 0;
}
