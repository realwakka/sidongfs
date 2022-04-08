#include <linux/kernel.h> //Needed by all modules
#include <linux/module.h> //Needed for KERN_ALERT
#include <linux/init.h> //Needed for the macros
#include <linux/fs.h>

static int sidongfs_fill_super(struct super_block *s, void *data, int silent)
{
	return -EINVAL;
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
