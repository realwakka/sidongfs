#ifndef SIDONG_FS_H
#define SIDONG_FS_H

#include <linux/types.h>

#define SIDONGFS_ROOT_INO 1

struct sidong_super_block {
	char magic[16];
	__u8 version;
};

struct sidong_inode {
	__u16 i_mode;
	__u16 i_uid;
	__u32 i_size;
	__u32 i_time;
	__u8  i_gid;
	__u16 i_zone[9];
};

#endif
