/*
 *   fs/samplefs/file.c
 *
 *   Copyright (C) International Business Machines  Corp., 2006
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   Sample File System
 *
 *   Primitive example to show how to create a Linux filesystem module
 *
 *   File struct (file instance) related functions
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

struct address_space_operations sfs_aops = {
	.readpage       = simple_readpage,
	.prepare_write  = simple_prepare_write,
	.commit_write   = simple_commit_write
};

struct file_operations sfs_file_operations = {
	.read           = do_sync_read,
	.aio_read	= generic_file_aio_read,
	.write          = do_sync_write,
	.aio_write	= generic_file_aio_write,
	.mmap           = generic_file_mmap,
	.fsync          = simple_sync_file,
	.sendfile       = generic_file_sendfile,
	.llseek         = generic_file_llseek,
};

