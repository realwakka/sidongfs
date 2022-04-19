#include <linux/fs.h>

#include "sidongfs.h"

static int sidongfs_readdir(struct file *file, struct dir_context *ctx)
{
	struct inode *inode = file_inode(file);

	printk("sidongfs_readdir\n");
	dir_emit(ctx, ".", 1, SIDONGFS_ROOT_INO, DT_DIR);
	return 0;
}

const struct file_operations sidongfs_dir_operations = {
	.llseek		= generic_file_llseek,
	.read		= generic_read_dir,
	.iterate_shared	= sidongfs_readdir,
	.fsync		= generic_file_fsync,
};
/*
static int sidongfs_create(struct user_namespace *mnt_userns, struct inode *dir,
			struct dentry *dentry, umode_t mode, bool excl)
{
	return -1;
}

static struct dentry *sidongfs_lookup(struct inode * dir, struct dentry *dentry, unsigned int flags)
{
	struct inode * inode = NULL;
	ino_t ino;

	if (dentry->d_name.len > sidongfs_sb(dir->i_sb)->s_namelen)
		return ERR_PTR(-ENAMETOOLONG);

	ino = sidongfs_inode_by_name(dentry);
	if (ino)
		inode = sidongfs_iget(dir->i_sb, ino);
	return d_splice_alias(inode, dentry);
}

const struct inode_operations sidongfs_dir_inode_operations = {
	.create		= sidongfs_create,
	.lookup		= sidongfs_lookup,
	.link		= sidongfs_link,
	.unlink		= sidongfs_unlink,
	.symlink	= sidongfs_symlink,
	.mkdir		= sidongfs_mkdir,
	.rmdir		= sidongfs_rmdir,
	.mknod		= sidongfs_mknod,
	.rename		= sidongfs_rename,
	.getattr	= sidongfs_getattr,
	.tmpfile	= sidongfs_tmpfile,
};

*/
