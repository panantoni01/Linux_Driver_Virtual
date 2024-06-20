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
#include <linux/iio/iio.h>

struct si7210_data {
	struct i2c_client* client;
	u8 temp_offset;
	u8 temp_gain;
};

static const struct iio_chan_spec si7210_channels[] = {
	{
		.type = IIO_MAGN,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_OFFSET)
	},
	{
		.type = IIO_TEMP,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_OFFSET)
	}
};

static int si7210_device_init(struct si7210_data *data)
{
	/* TODO: use regmap to read gain and offset values and place them in `data` */

	/* TODO: wake up from possible sleep mode */

	/* TODO: sleep 1 ms before starting the measurements */
	return 0;
}

static int si7210_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct si7210_data *data;
	struct iio_dev *indio_dev;
	int ret;

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	i2c_set_clientdata(client, indio_dev);
	data->client = client;

	indio_dev->name = dev_name(&client->dev);
	indio_dev->modes = INDIO_DIRECT_MODE;
	/* TODO: define struct iio_info */
	indio_dev->info = NULL;
	indio_dev->channels = si7210_channels;
	indio_dev->num_channels = ARRAY_SIZE(si7210_channels);

	ret = si7210_device_init(data);
	if (ret)
		return dev_err_probe(&client->dev, ret, "device initialization failed\n");

	return devm_iio_device_register(&client->dev, indio_dev);
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
