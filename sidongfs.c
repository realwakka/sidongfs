#include <linux/kernel.h> //Needed by all modules
#include <linux/module.h> //Needed for KERN_ALERT
#include <linux/init.h> //Needed for the macros
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/container_of.h>
#include <linux/fs.h>

#include "sidongfs.h"

struct sidong_fs_sb_info {

};

struct sidong_fs_inode_info {
	struct inode vfs_inode;
};

static inline struct sidong_fs_inode_info *sidong_fs_i(struct inode *inode)
{
	return container_of(inode, struct sidong_fs_inode_info, vfs_inode);
}

static inline struct sidong_fs_sb_info *sidong_fs_sb(struct super_block *sb)
{
	return sb->s_fs_info;
}


static struct inode *sidong_fs_alloc_inode(struct super_block *sb)
{
	struct sidong_fs_inode_info *inode_info;
	inode_info = kzalloc(sizeof(struct sidong_fs_inode_info), GFP_KERNEL);	
	if (!inode_info)
		return NULL;
	return &inode_info->vfs_inode;
}

static void sidong_fs_free_inode(struct inode *inode)
{
	kfree(sidong_fs_i(inode));
}

static int sidong_fs_write_inode(struct inode *inode, struct writeback_control *wbc)
{
	return 0;
}

static void sidong_fs_evict_inode(struct inode *inode)
{
	truncate_inode_pages_final(&inode->i_data);
	inode->i_size = 0;
	// minix_truncate(inode);
	
	invalidate_inode_buffers(inode);
	clear_inode(inode);

	sidong_fs_free_inode(inode);	
}

static void sidong_fs_put_super(struct super_block *sb)
{
	struct sidong_fs_sb_info *sbi = sidong_fs_sb(sb);
	kfree(sbi);
}

static const struct super_operations sidong_fs_sops = {
	.alloc_inode	= sidong_fs_alloc_inode,
	.free_inode	= sidong_fs_free_inode,
	.write_inode	= sidong_fs_write_inode,
	.evict_inode	= sidong_fs_evict_inode,
	.put_super	= sidong_fs_put_super,
	/* .statfs		= sidong_fs_statfs, */
	/* .remount_fs	= sidong_fs_remount, */
};

static int sidongfs_fill_super(struct super_block *s, void *data, int silent)
{
	struct sidong_super_block *sb;
	struct sidong_fs_sb_info *sbi;
	struct buffer_head *bh;
	struct inode *root_inode;

	printk(KERN_INFO "sidongfs fill super\n");

	if (!sb_set_blocksize(s, 1024)) {
		printk(KERN_INFO "sb_set_blocksize failed\n");		
		return -EINVAL;
	}
	if (!(bh = sb_bread(s, 1))) {
		printk(KERN_INFO "sb_bread failed\n");
		return -EINVAL;
	}

	sb = (struct sidong_super_block *) bh->b_data;
	printk(KERN_INFO "magic: %s version: %d\n", sb->magic, sb->version);
	sbi = kzalloc(sizeof(struct sidong_fs_sb_info), GFP_KERNEL);

	root_inode = iget_locked(s, SIDONGFS_ROOT_INO);
	root_inode->i_mode = S_IFDIR;

	s->s_op = &sidong_fs_sops;
	s->s_time_min = 0;
	s->s_time_max = U32_MAX;
	s->s_root = d_make_root(root_inode);

	brelse(bh);
	return 0;
}

static struct dentry *sidongfs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, sidongfs_fill_super);
}

static struct file_system_type sidong_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "sidongfs",
	.mount		= sidongfs_mount,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
	.init_fs_context    = NULL,
};
MODULE_ALIAS_FS("sidongfs");


int __init sidongfs_module_init(void)
{
	int err;
	printk("Sidongfs Module!\n");
	err = register_filesystem(&sidong_fs_type);
	if (err)
		goto out;
	return 0;
out:
	return err;
}

void __exit sidongfs_module_cleanup(void)
{
	unregister_filesystem(&sidong_fs_type);
	printk("Bye sidongfs Module!\n");
}

module_init(sidongfs_module_init); 
module_exit(sidongfs_module_cleanup); 
MODULE_LICENSE("GPL");
