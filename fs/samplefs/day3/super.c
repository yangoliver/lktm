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
#include <linux/proc_fs.h>
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
	return;
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
	int size;

	if (!options)
		return;

	printk(KERN_INFO "samplefs: parsing mount options %s\n", options);

	while ((data = strsep(&options, ",")) != NULL) {
		if (!*data)
			continue;
		if ((value = strchr(data, '=')) != NULL)
			*value++ = '\0';

		if (strnicmp(data, "rsize", 5) == 0) {
			if (value && *value) {
				size = simple_strtoul(value, &value, 0);
				if (size > 0) {
					sfs_sb->rsize = size;
					printk(KERN_INFO
						"samplefs: rsize %d\n", size);
				}

			}
		} else if (strnicmp(data, "wsize", 5) == 0) {
			if (value && *value) {
				size = simple_strtoul(value, &value, 0);
				if (size > 0) {
					sfs_sb->wsize = size;
					printk(KERN_INFO
						"samplefs: wsize %d\n", size);
				}
			}
		} else {
			printk(KERN_WARNING "samplefs: bad mount option %s\n",
				data);
		}
	}
}

static int samplefs_fill_super(struct super_block *sb, void * data, int silent)
{
	struct inode *inode;
	struct samplefs_sb_info *sfs_sb;

	sb->s_maxbytes = MAX_LFS_FILESIZE; /* NB: may be too large for mem */
	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = SAMPLEFS_MAGIC;
	sb->s_op = &samplefs_super_ops;
	sb->s_time_gran = 1; /* 1 nanosecond time granularity */


	printk(KERN_INFO "samplefs: fill super\n");

/* Eventually replace iget with:
	inode = samplefs_get_inode(sb, S_IFDIR | 0755, 0); */

	inode = iget(sb, SAMPLEFS_ROOT_I);

	if (!inode)
		return -ENOMEM;

#ifdef CONFIG_SAMPLEFS_DEBUG
	printk(KERN_INFO "samplefs: about to alloc s_fs_info\n");
#endif
	sb->s_fs_info = kzalloc(sizeof(struct samplefs_sb_info), GFP_KERNEL);
	sfs_sb = SFS_SB(sb);
	if (!sfs_sb) {
		iput(inode);
		return -ENOMEM;
	}


	printk(KERN_INFO "samplefs: about to alloc root inode\n");

	sb->s_root = d_alloc_root(inode);
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

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *proc_fs_samplefs;

static int
sfs_debug_read(char *buf, char **beginBuffer, off_t offset,
		int count, int *eof, void *data)
{
	int length = 0;
	char *original_buf = buf;

	*beginBuffer = buf + offset;

	length = sprintf(buf,
			"Display Debugging Information\n"
			"-----------------------------\n");

	buf += length;

	/* FS-FILLIN - add your debug information here */

	length = buf - original_buf;
	if (offset + count >= length)
		*eof = 1;
	if (length < offset) {
		*eof = 1;
		return 0;
	} else {
		length = length - offset;
	}

	if (length > count)
		length = count;

	return length;
}
void
sfs_proc_init(void)
{
	proc_fs_samplefs = proc_mkdir("samplefs", proc_root_fs);
	if (proc_fs_samplefs == NULL)
		return;

	proc_fs_samplefs->owner = THIS_MODULE;
	create_proc_read_entry("DebugData", 0, proc_fs_samplefs,
				sfs_debug_read, NULL);
}

void
sfs_proc_clean(void)
{
	if (proc_fs_samplefs == NULL)
		return;

	remove_proc_entry("DebugData", proc_fs_samplefs);
	remove_proc_entry("samplefs", proc_root_fs);
}
#endif /* CONFIG_PROC_FS */

static int __init init_samplefs_fs(void)
{
	printk(KERN_INFO "init samplefs\n");
#ifdef CONFIG_PROC_FS
	sfs_proc_init();
#endif

	/* some filesystems pass optional parms at load time */
	if (sample_parm > 256) {
		printk(KERN_ERR "sample_parm %d too large, reset to 10\n",
			sample_parm);
		sample_parm = 10;
	}

	return register_filesystem(&samplefs_fs_type);
}

static void __exit exit_samplefs_fs(void)
{
	printk(KERN_INFO "unloading samplefs\n");
#ifdef CONFIG_PROC_FS
	sfs_proc_clean();
#endif
	unregister_filesystem(&samplefs_fs_type);
}

module_init(init_samplefs_fs)
module_exit(exit_samplefs_fs)

MODULE_LICENSE("GPL");
