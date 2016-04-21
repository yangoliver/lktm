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
#include <linux/slab.h>
#include <linux/blkdev.h>

static int sampleblk_major;
#define SAMPLEBLK_MINOR	1
static int sampleblk_size = CONFIG_BLK_DEV_RAM_SIZE;

struct sampleblk_dev {
	int minor;
	spinlock_t lock;
	struct request_queue *queue;
	struct gendisk *disk;
};

struct sampleblk_dev *sampleblk_dev = NULL;

static void sampleblk_request(struct request_queue *rq)
{
}

static int sampleblk_ioctl(struct block_device *bdev, fmode_t mode,
			unsigned command, unsigned long argument)
{
	return 0;
}

static int sampleblk_open(struct block_device *bdev, fmode_t mode)
{
	return 0;
}

static void sampleblk_release(struct gendisk *disk, fmode_t mode)
{
}

static const struct block_device_operations sampleblk_fops = {
	.owner = THIS_MODULE,
	.open = sampleblk_open,
	.release = sampleblk_release,
	.ioctl = sampleblk_ioctl,
};

static int sampleblk_alloc(int minor)
{
	struct gendisk *disk;
	int rv = 0;

	sampleblk_dev = kzalloc(sizeof(sampleblk_dev), GFP_KERNEL);
	if (!sampleblk_dev) {
		rv = ENOMEM;
		goto fail;
	}

	sampleblk_dev->minor = minor;

	spin_lock_init(&sampleblk_dev->lock);

	sampleblk_dev->queue = blk_init_queue(sampleblk_request,
	    &sampleblk_dev->lock);
	if (!sampleblk_dev->queue) {
		rv = ENOMEM;
		goto fail_dev;
	}

	disk = alloc_disk(minor);
	if (!disk) {
		rv = ENOMEM;
		goto fail_queue;
	}
	smapleblk_dev->disk = disk;

	disk->major = sampleblk_major;
	disk->first_minor = minor;
	disk->fops = &sampleblk_fops;
	disk->private_data = sampleblk_dev;
	disk->queue = sampleblk_dev->queue;
	sprintf(disk->disk_name, "sampleblk%d", minor);
	set_capacity(disk, sampleblk_size);
	add_disk(disk);

	return 0;

fail_queue:
	blk_cleanup_queue(sampleblk_dev->queue);
fail_dev:
	kfree(sampleblk_dev);
fail:
	return rv;
}

static void sampleblk_free(struct sampleblk_dev *sampleblk_dev)
{
	del_gendisk(sampleblk_dev->disk);
	blk_cleanup_queue(sampleblk_dev->queue);
	kfree(sampleblk_dev);
}

static int __init sampleblk_init(void)
{
	int rv = 0;

	sampleblk_major = register_blkdev(0, "sampleblk");
	if (sampleblk_major < 0)
		return sampleblk_major;

	rv = sampleblk_alloc(SAMPLEBLK_MINOR);
	if (rv < 0)
		pr_info("sampleblk: disk allocation failed with %d\n", rv);

	pr_info("sampleblk: module loaded\n");
	return 0;
}

static void __exit sampleblk_exit(void)
{
	sampleblk_free(sampleblk_dev);
	unregister_blkdev(sampleblk_major, "sampleblk");

	pr_info("sampleblk: module unloaded\n");
}

module_init(sampleblk_init);
module_exit(sampleblk_exit);
MODULE_LICENSE("GPL");
