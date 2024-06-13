// SPDX-License-Identifier: GPL-2.0
/*
 * Silicon Labs Si7210 Hall Effect sensor driver
 *
 * Copyright (c) 2024 Antoni Pokusinski <apokusinski@o2.pl>
 *
 * Datasheet:
 *  https://www.silabs.com/documents/public/data-sheets/si7210-datasheet.pdf
 */

#include <linux/module.h>

static int __init si7210_init(void)
{
	printk(KERN_INFO "s7210_driver registration\n");
	return 0;
}

static void __exit si7210_cleanup(void)
{
	printk(KERN_INFO "si7210_driver removal\n");
}

module_init(si7210_init);
module_exit(si7210_cleanup);

MODULE_AUTHOR("Antoni Pokusinski <apokusinski@o2.pl>");
MODULE_DESCRIPTION("Silicon Labs Si7210 Hall Effect sensor I2C driver");
MODULE_LICENSE("GPL v2");
