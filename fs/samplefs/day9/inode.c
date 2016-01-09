/*
 *   fs/samplefs/inode.c
 *
 *   Copyright (C) International Business Machines  Corp., 2006
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   Sample File System
 *
 *   Primitive example to show how to create a Linux filesystem module
 *
 *   Inode related functions
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
#include "samplefs.h"

extern struct dentry_operations sfs_dentry_ops;
extern struct dentry_operations sfs_ci_dentry_ops;
extern struct inode *samplefs_get_inode(struct super_block *sb, int mode, 
					dev_t dev);

/*
 * Lookup the data, if the dentry didn't already exist, it must be
 * negative.  Set d_op to delete negative dentries to save memory
 * (and since it does not help performance for in memory filesystem).
 */
static struct dentry *sfs_lookup(struct inode *dir, struct dentry *dentry, 
				struct nameidata *nd)
{
	struct samplefs_sb_info * sfs_sb = SFS_SB(dir->i_sb);
	if (dentry->d_name.len > NAME_MAX)
		return ERR_PTR(-ENAMETOOLONG);
	if(sfs_sb->flags & SFS_MNT_CASE)
		dentry->d_op = &sfs_ci_dentry_ops;
	else
		dentry->d_op = &sfs_dentry_ops;

	d_add(dentry, NULL);
	return NULL;
}

static int
sfs_mknod(struct inode *dir, struct dentry *dentry, int mode, dev_t dev)
{
        struct inode * inode = samplefs_get_inode(dir->i_sb, mode, dev);
        int error = -ENOSPC;
	
	printk(KERN_INFO "samplefs: mknod\n");
        if (inode) {
                if (dir->i_mode & S_ISGID) {
                        inode->i_gid = dir->i_gid;
                        if (S_ISDIR(mode))
                                inode->i_mode |= S_ISGID;
                }
                d_instantiate(dentry, inode);
                dget(dentry);   /* Extra count - pin the dentry in core */
                error = 0;
                dir->i_mtime = dir->i_ctime = CURRENT_TIME;

		/* real filesystems would normally use i_size_write function */
		dir->i_size += 0x20;  /* bogus small size for each dir entry */
        }
        return error;
}


static int sfs_mkdir(struct inode * dir, struct dentry * dentry, int mode)
{
        int retval = 0;
	
	retval = sfs_mknod(dir, dentry, mode | S_IFDIR, 0);

	/* link count is two for dir, for dot and dot dot */
        if (!retval)
                dir->i_nlink++;
        return retval;
}

static int sfs_create(struct inode *dir, struct dentry *dentry, int mode, 
			struct nameidata *nd)
{
        return sfs_mknod(dir, dentry, mode | S_IFREG, 0);
}

struct inode_operations sfs_file_inode_ops = {
        .getattr        = simple_getattr,
};

struct inode_operations sfs_dir_inode_ops = {
	.create         = sfs_create,
	.lookup         = sfs_lookup,
	.unlink         = simple_unlink,
	.mkdir          = sfs_mkdir,
	.rmdir          = simple_rmdir,
	.mknod          = sfs_mknod,
	.rename         = simple_rename,
};

