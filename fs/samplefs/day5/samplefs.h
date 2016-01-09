/*
 *   fs/samplefs/samplefs.h
 *
 *   Copyright (C) International Business Machines  Corp., 2006, 2007
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   Sample File System
 *
 *   Primitive example to show how to create a Linux filesystem module
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

#define SAMPLEFS_ROOT_I 2
/* samplefs mount flags */
#define SFS_MNT_CASE 1

/* This is an example of filesystem specific mount data that a file system might
   want to store.  FS per-superblock data varies widely and some fs do not
   require any information beyond the generic info which is already in
   struct super_block */
struct samplefs_sb_info {
	unsigned int rsize;
	unsigned int wsize;
	int flags;
	struct nls_table *local_nls;
};

static inline struct samplefs_sb_info *
SFS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}
