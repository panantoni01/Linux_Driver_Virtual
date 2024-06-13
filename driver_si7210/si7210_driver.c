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
#include <linux/i2c.h>

static int si7210_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	return 0;
}

static const struct i2c_device_id si7210_id[] = {
	{ "si7210", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, si7210_id);

static const struct of_device_id si7210_dt_ids[] = {
	{ .compatible = "silabs,si7210" },
	{ }
};
MODULE_DEVICE_TABLE(of, si7210_dt_ids);

static struct i2c_driver si7210_driver = {
	.driver = {
		.name = "si7210",
		.of_match_table = si7210_dt_ids,
	},
	.probe		= si7210_probe,
	.id_table	= si7210_id,
};

module_i2c_driver(si7210_driver);
MODULE_AUTHOR("Antoni Pokusinski <apokusinski@o2.pl>");
MODULE_DESCRIPTION("Silicon Labs Si7210 Hall Effect sensor I2C driver");
MODULE_LICENSE("GPL v2");
