/*
 *   fs/samplefs/super.c
 *
 *   Copyright (C) International Business Machines  Corp., 2006, 2007
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   Sample File System
 *
 *   Primitive example to show how to create a Linux filesystem module
 *
 *   superblock related and misc. functions
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/version.h>
#include <linux/nls.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>
#include "samplefs.h"

/* helpful if this is different than other fs */
#define SAMPLEFS_MAGIC     0x73616d70 /* "SAMP" */

unsigned int sample_parm = 0;
module_param(sample_parm, int, 0);
MODULE_PARM_DESC(sample_parm, "An example parm. Default: x Range: y to z");

static void
samplefs_put_super(struct super_block *sb)
{
	struct samplefs_sb_info *sfs_sb;

	sfs_sb = SFS_SB(sb);
	if (sfs_sb == NULL) {
		/* Empty superblock info passed to unmount */
		return;
	}

	unload_nls(sfs_sb->local_nls);

	/* FS-FILLIN your fs specific umount logic here */

	kfree(sfs_sb);
}


struct super_operations samplefs_super_ops = {
	.statfs         = simple_statfs,
	.drop_inode     = generic_delete_inode, /* Not needed, is the default */
	.put_super      = samplefs_put_super,
};

static void
samplefs_parse_mount_options(char *options, struct samplefs_sb_info *sfs_sb)
{
	char *value;
	char *data;
	unsigned long size;
	int ret = 0;

	if (!options)
		return;

	pr_info("samplefs: parsing mount options %s\n", options);

	while ((data = strsep(&options, ",")) != NULL) {
		if (!*data)
			continue;
		value = strchr(data, '=');
		if (value != NULL)
			*value++ = '\0';

		if (strncasecmp(data, "rsize", 5) == 0) {
			if (value && *value) {
				ret = kstrtoul(value, 0, &size);
				if (ret) {
					pr_err("kstrtoul error:%d\n", ret);
					return;
				}
				if (size > 0) {
					sfs_sb->rsize = size;
					pr_info("samplefs: rsize %lu\n", size);
				}

			}
		} else if (strncasecmp(data, "wsize", 5) == 0) {
			if (value && *value) {
				ret = kstrtoul(value, 0, &size);
				if (ret) {
					pr_err("kstrtoul error:%d\n", ret);
					return;
				}
				if (size > 0) {
					sfs_sb->wsize = size;
					pr_info("samplefs: wsize %lu\n", size);
				}
			}
		} else {
			pr_warn("samplefs: bad mount option %s\n", data);
		}
	}
}

static int samplefs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;
	struct samplefs_sb_info *sfs_sb;

	sb->s_maxbytes = MAX_LFS_FILESIZE; /* NB: may be too large for mem */
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = SAMPLEFS_MAGIC;
	sb->s_op = &samplefs_super_ops;
	sb->s_time_gran = 1; /* 1 nanosecond time granularity */


	pr_info("samplefs: fill super\n");

/* Eventually replace iget with:
	inode = samplefs_get_inode(sb, S_IFDIR | 0755, 0); */

	inode = iget_locked(sb, SAMPLEFS_ROOT_I);

	if (!inode)
		return -ENOMEM;

	unlock_new_inode(inode);

#ifdef CONFIG_SAMPLEFS_DEBUG
	pr_info("samplefs: about to alloc s_fs_info\n");
#endif
	sb->s_fs_info = kzalloc(sizeof(struct samplefs_sb_info), GFP_KERNEL);
	sfs_sb = SFS_SB(sb);
	if (!sfs_sb) {
		iput(inode);
		return -ENOMEM;
	}


	pr_info("samplefs: about to alloc root inode\n");

	sb->s_root = d_make_root(inode);
	if (!sb->s_root) {
		iput(inode);
		kfree(sfs_sb);
		return -ENOMEM;
	}

	/* below not needed for many fs - but an example of per fs sb data */
	sfs_sb->local_nls = load_nls_default();

	samplefs_parse_mount_options(data, sfs_sb);

	/* FS-FILLIN your filesystem specific mount logic/checks here */

	return 0;
}

static struct dentry *samplefs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, samplefs_fill_super);
}

static struct file_system_type samplefs_fs_type = {
	.owner = THIS_MODULE,
	.name = "samplefs",
	.mount = samplefs_mount,
	.kill_sb = kill_anon_super,
	/*  .fs_flags */
};

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *proc_fs_samplefs;

static int
samplefs_debug_data_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m,
			"Display Debugging Information\n"
			"-----------------------------\n");

	return 0;
}

static int samplefs_debug_data_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, samplefs_debug_data_proc_show, NULL);
}

static const struct file_operations samplefs_debug_data_proc_fops = {
	.owner      = THIS_MODULE,
	.open       = samplefs_debug_data_proc_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release    = single_release,
};

void
sfs_proc_init(void)
{
	proc_fs_samplefs = proc_mkdir("fs/samplefs", NULL);
	if (proc_fs_samplefs == NULL)
		return;

	proc_create("DebugData", 0, proc_fs_samplefs,
	    &samplefs_debug_data_proc_fops);
}

void
sfs_proc_clean(void)
{
	if (proc_fs_samplefs == NULL)
		return;

	remove_proc_entry("DebugData", proc_fs_samplefs);
	remove_proc_entry("fs/samplefs", NULL);
}
#endif /* CONFIG_PROC_FS */

static int __init init_samplefs_fs(void)
{
	pr_info("samplefs: init moudle\n");
#ifdef CONFIG_PROC_FS
	sfs_proc_init();
#endif

	/* some filesystems pass optional parms at load time */
	if (sample_parm > 256) {
		pr_err("sample_parm %d too large, reset to 10\n", sample_parm);
		sample_parm = 10;
	}

	return register_filesystem(&samplefs_fs_type);
}

static void __exit exit_samplefs_fs(void)
{
	pr_info("samplefs: unload moudle\n");
#ifdef CONFIG_PROC_FS
	sfs_proc_clean();
#endif
	unregister_filesystem(&samplefs_fs_type);
}

module_init(init_samplefs_fs)
module_exit(exit_samplefs_fs)
MODULE_LICENSE("GPL");
