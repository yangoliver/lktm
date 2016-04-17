/*
 *   blk/sampleblk/sample_blk.c
 *
 *   Copyright (C) Oliver Yang 2016
 *   Author(s): Yong Yang (yangoliver@gmail.com)
 *
 *   Sample Block Driver
 *
 *   Primitive example to show how to create a Linux block driver
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
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>

static int sampleblk_major;

static int __init sampleblk_init(void)
{
	sampleblk_major = register_blkdev(0, "sampleblk");
	if (sampleblk_major < 0)
		return sampleblk_major;

	pr_info("sampleblk: module loaded\n");
	return 0;
}

static void __exit sampleblk_exit(void)
{
	unregister_blkdev(sampleblk_major, "sampleblk");

	pr_info("sampleblk: module unloaded\n");
}

module_init(sampleblk_init);
module_exit(sampleblk_exit);
MODULE_LICENSE("GPL");
