/*
 *   nmi/samplenmi/sample_nmi.c
 *
 *   Copyright (C) Oliver Yang 2016
 *   Author(s): Yong Yang (yangoliver@gmail.com)
 *
 *   Sample Driver which uses the NMI handler.
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
#include <asm/nmi.h>
#include <asm/pgtable.h>
#include <linux/sched/debug.h>

static int nmi_ignore;

static int samplenmi_handler(unsigned int cmd, struct pt_regs *regs)
{
	unsigned long rsp;
	static int i;

	if (nmi_ignore)
		return NMI_DONE;

	rsp = regs->sp;

	if (rsp < __PAGE_OFFSET) {
		show_regs(regs); /* This requires EXPORT_SYMBOL(show_regs) in kernel */
		pr_err("User Stack Pointer: %016lx", rsp);
		pr_err("User Stack Dump: %016lx", *((unsigned long *)rsp));
	} else {
		pr_err("Kernel Stack Count: %d", ++i);
		pr_err("Kernel Stack Pointer: %016lx", rsp);
		pr_err("Kernel Stack Dump: %016lx", *((unsigned long *)rsp));
	}

	return NMI_HANDLED;
}

static int __init samplenmi_init(void)
{
	int rv = 0;

	rv = register_nmi_handler(NMI_LOCAL, samplenmi_handler, 0,
	    "samplenmi_handler");

	if (rv) {
		pr_err("samplenmi: can't register nmi handler\n");
		return rv;
	}

	pr_info("samplenmi: module loaded\n");
	return 0;
}

static void __exit samplenmi_exit(void)
{
	unregister_nmi_handler(NMI_LOCAL, "samplenmi_handler");

	pr_info("samplenmi: module unloaded\n");
}

module_init(samplenmi_init);
module_exit(samplenmi_exit);
MODULE_LICENSE("GPL");
