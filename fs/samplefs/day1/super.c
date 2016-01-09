/*
 *   fs/samplefs/super.c
 *
 *   Copyright (C) International Business Machines  Corp., 2006,2007
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
#include <linux/version.h>

/* helpful if this is different than other fs */
#define SAMPLEFS_MAGIC     0x73616d70 /* "SAMP" */

static int samplefs_fill_super(struct super_block * sb, void * data, int silent)
{
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
struct super_block * samplefs_get_sb(struct file_system_type *fs_type,
        int flags, const char *dev_name, void *data)
{
	return get_sb_nodev(fs_type, flags, data, samplefs_fill_super);
}
#else
int samplefs_get_sb(struct file_system_type *fs_type,
        int flags, const char *dev_name, void *data, struct vfsmount *mnt)
{
	return get_sb_nodev(fs_type, flags, data, samplefs_fill_super, mnt);
}
#endif


static struct file_system_type samplefs_fs_type = {
	.owner = THIS_MODULE,
	.name = "samplefs",
	.get_sb = samplefs_get_sb,
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

