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
#include "samplefs.h"

/* helpful if this is different than other fs */
#define SAMPLEFS_MAGIC     0x73616d70 /* "SAMP" */

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
				if (size > 0)
					sfs_sb->rsize = size;
			}
		} else if (strncasecmp(data, "wsize", 5) == 0) {
			if (value && *value) {
				ret = kstrtoul(value, 0, &size);
				if (ret) {
					pr_err("kstrtoul error:%d\n", ret);
					return;
				}
				if (size > 0)
					sfs_sb->wsize = size;
			}
		} /* else unknown mount option */
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

/* Eventually replace iget with:
	inode = samplefs_get_inode(sb, S_IFDIR | 0755, 0); */

	inode = iget_locked(sb, SAMPLEFS_ROOT_I);

	if (!inode)
		return -ENOMEM;

	unlock_new_inode(inode);

	sb->s_fs_info = kzalloc(sizeof(struct samplefs_sb_info), GFP_KERNEL);
	sfs_sb = SFS_SB(sb);
	if (!sfs_sb) {
		iput(inode);
		return -ENOMEM;
	}

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


static int __init init_samplefs_fs(void)
{
	return register_filesystem(&samplefs_fs_type);
}

static void __exit exit_samplefs_fs(void)
{
	unregister_filesystem(&samplefs_fs_type);
}

module_init(init_samplefs_fs)
module_exit(exit_samplefs_fs)
MODULE_LICENSE("GPL");
