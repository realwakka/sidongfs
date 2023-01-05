#include <linux/kernel.h> //Needed by all modules
#include <linux/module.h> //Needed for KERN_ALERT
#include <linux/init.h> //Needed for the macros
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/container_of.h>
#include <linux/fs.h>

#include "sidongfs.h"

struct sidongfs_sb_info {
	struct buffer_head *sbh;
};

struct sidongfs_inode_info {
	struct inode vfs_inode;
};

static inline struct sidongfs_inode_info *sidongfs_i(struct inode *inode)
{
	return container_of(inode, struct sidongfs_inode_info, vfs_inode);
}

static inline struct sidongfs_sb_info *sidongfs_sb(struct super_block *sb)
{
	return sb->s_fs_info;
}

static struct kmem_cache *sidongfs_inode_cachep;

static struct inode *sidongfs_alloc_inode(struct super_block *sb)
{
	struct sidongfs_inode_info *inode_info;
	// we use kmem_cache_alloc() because of kill_block_super()
	inode_info = alloc_inode_sb(sb, sidongfs_inode_cachep, GFP_KERNEL);
	if (!inode_info)
		return NULL;
	return &inode_info->vfs_inode;
}

static void sidongfs_free_inode(struct inode *inode)
{
	kmem_cache_free(sidongfs_inode_cachep, sidongfs_i(inode));
	// kfree(sidongfs_i(inode));
}

static void init_once(void *foo)
{
	struct sidongfs_inode_info *ei = (struct sidongfs_inode_info *) foo;
	inode_init_once(&ei->vfs_inode);
}

static int __init init_inodecache(void)
{
	sidongfs_inode_cachep = kmem_cache_create("sidongfs_inode_cache",
											  sizeof(struct sidongfs_inode_info),
											  0, (SLAB_RECLAIM_ACCOUNT|
												  SLAB_MEM_SPREAD|SLAB_ACCOUNT),
											  init_once);
	if (sidongfs_inode_cachep == NULL)
		return -ENOMEM;
	return 0;
}

static int sidongfs_write_inode(struct inode *inode, struct writeback_control *wbc)
{
	printk(KERN_INFO "sidong fs write inode\n");
	return 0;
}

static void sidongfs_evict_inode(struct inode *inode)
{
	printk(KERN_INFO "sidong fs evict inode\n");
	truncate_inode_pages_final(&inode->i_data);

	if (!inode->i_nlink) {
		inode->i_size = 0;
		// sidongfs_truncate(inode);
	}

	invalidate_inode_buffers(inode);
	clear_inode(inode);

	if (!inode->i_nlink)
		sidongfs_free_inode(inode);
}

static void sidongfs_put_super(struct super_block *sb)
{
	struct sidongfs_sb_info *sbi = sidongfs_sb(sb);

	printk("sidongfs_put_super!!!\n");
	sb->s_fs_info = NULL;

	brelse(sbi->sbh);
	kfree(sbi);
}

static const struct super_operations sidongfs_sops = {
	.alloc_inode	= sidongfs_alloc_inode,
	.free_inode	= sidongfs_free_inode,
	.write_inode	= sidongfs_write_inode,
	.evict_inode	= sidongfs_evict_inode,
	.put_super	= sidongfs_put_super,
	/* .statfs		= sidongfs_statfs, */
	/* .remount_fs	= sidongfs_remount, */
};

void sidongfs_set_inode(struct inode *inode, dev_t rdev)
{
	if (S_ISREG(inode->i_mode)) {
		printk("regular file is not implemented yet.\n");
	} else if (S_ISDIR(inode->i_mode)) {
		printk("sidongfs_set_inode dir node.\n");
		inode->i_fop = &sidongfs_dir_operations;
	} else if (S_ISLNK(inode->i_mode)) {
		printk("link file is not implemented yet.\n");
	} else
		printk(" file is not implemented yet.\n");
}

static struct inode *sidongfs_iget(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;
	struct sidongfs_inode *sidongfs_inode;
	struct sidongfs_inode *nodes;
	struct buffer_head *bh;

	inode = iget_locked(sb, ino);
	if (!inode)
		return ERR_PTR(-ENOMEM);
	if (!(inode->i_state & I_NEW))
		return inode;

	bh = sb_bread(sb, 4);

	nodes = (struct sidongfs_inode*)bh->b_data;
	sidongfs_inode = &nodes[ino];

	inode->i_mode = sidongfs_inode->i_mode;
	i_uid_write(inode, sidongfs_inode->i_uid);
	i_gid_write(inode, sidongfs_inode->i_gid);
	set_nlink(inode, sidongfs_inode->i_nlinks);
	printk("sidongfs nlink %u\n", inode->i_nlink);
	inode->i_size = sidongfs_inode->i_size;
	inode->i_mtime.tv_sec = inode->i_atime.tv_sec = inode->i_ctime.tv_sec = sidongfs_inode->i_time;
	inode->i_mtime.tv_nsec = 0;
	inode->i_atime.tv_nsec = 0;
	inode->i_ctime.tv_nsec = 0;
	inode->i_blocks = 0;

	sidongfs_set_inode(inode, sidongfs_inode->i_mode);
	brelse(bh);
	unlock_new_inode(inode);
	return inode;
}

static int sidongfs_fill_super(struct super_block *s, void *data, int silent)
{
	struct sidongfs_super_block *sb;
	struct sidongfs_sb_info *sbi;
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

	sbi = kzalloc(sizeof(struct sidongfs_sb_info), GFP_KERNEL);
	if (!sbi)
		return -ENOMEM;
	s->s_fs_info = sbi;

	sb = (struct sidongfs_super_block *) bh->b_data;

	sbi->sbh = bh;

	s->s_maxbytes = 1024;
	s->s_magic = sb->version;
	s->s_op = &sidongfs_sops;
	s->s_time_min = 0;
	s->s_time_max = U32_MAX;
	s->s_d_op = NULL;
	root_inode = sidongfs_iget(s, SIDONGFS_ROOT_INO);
	if (IS_ERR(root_inode)) {
		printk("sidongfs: sidongfs_iget failed\n");
		kfree(sbi);
		s->s_fs_info = NULL;
		return PTR_ERR(root_inode);
	}

	s->s_root = d_make_root(root_inode);
	if (!s->s_root) {
		kfree(sbi);
		s->s_fs_info = NULL;
		printk("sidongfs: get root inode failed\n");
		return -ENOMEM;
	}

	return 0;
}

static struct dentry *sidongfs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, sidongfs_fill_super);
}

static struct file_system_type sidongfs_type = {
	.owner		= THIS_MODULE,
	.name		= "sidongfs",
	.mount		= sidongfs_mount,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};
MODULE_ALIAS_FS("sidongfs");


int __init sidongfs_module_init(void)
{
	int err;

	printk("Sidongfs Module!\n");
	err = init_inodecache();
	if (err)
		return err;

	err = register_filesystem(&sidongfs_type);
	if (err)
		goto out;
	return 0;
out:
	return err;
}

static void destroy_inodecache(void)
{
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(sidongfs_inode_cachep);
}

void __exit sidongfs_module_cleanup(void)
{
	unregister_filesystem(&sidongfs_type);
	destroy_inodecache();
	printk("Bye sidongfs Module!\n");
}

module_init(sidongfs_module_init);
module_exit(sidongfs_module_cleanup);
MODULE_LICENSE("GPL");
