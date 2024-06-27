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
#include <linux/regmap.h>

#define SI7210_REG_DSPSIGM	0xC1
#define SI7210_REG_DSPSIGL	0xC2
#define SI7210_REG_DSPSIGSEL	0xC3
#define SI7210_REG_ARAUTOINC	0xC5
#define SI7210_REG_OTP_ADDR	0xE1
#define SI7210_REG_OTP_DATA	0xE2
#define SI7210_BIT_OTP_READ_EN	BIT(1)
#define SI7210_REG_OTP_CTRL	0xE3

#define SI7210_OTPREG_TMP_OFF	0x1D
#define SI7210_OTPREG_TMP_GAIN	0x1E

static const struct regmap_range si7210_read_reg_ranges[] = {
	regmap_reg_range(SI7210_REG_DSPSIGM, SI7210_REG_DSPSIGSEL),
	regmap_reg_range(SI7210_REG_ARAUTOINC, SI7210_REG_ARAUTOINC),
	regmap_reg_range(SI7210_REG_OTP_ADDR, SI7210_REG_OTP_CTRL),
};

static const struct regmap_access_table si7210_readable_regs = {
	.yes_ranges = si7210_read_reg_ranges,
	.n_yes_ranges = ARRAY_SIZE(si7210_read_reg_ranges),
};

static const struct regmap_range si7210_write_reg_ranges[] = {
	regmap_reg_range(SI7210_REG_DSPSIGSEL, SI7210_REG_DSPSIGSEL),
	regmap_reg_range(SI7210_REG_ARAUTOINC, SI7210_REG_ARAUTOINC),
	regmap_reg_range(SI7210_REG_OTP_ADDR, SI7210_REG_OTP_CTRL),
};

static const struct regmap_access_table si7210_writeable_regs = {
	.yes_ranges = si7210_write_reg_ranges,
	.n_yes_ranges = ARRAY_SIZE(si7210_write_reg_ranges),
};

static const struct regmap_range si7210_volatile_reg_ranges[] = {
	/* TODO: check if dsisigsel, arautoinc and otps are volatile */
	regmap_reg_range(SI7210_REG_DSPSIGM, SI7210_REG_DSPSIGL),
};

static const struct regmap_access_table si7210_volatile_regs = {
	.yes_ranges = si7210_volatile_reg_ranges,
	.n_yes_ranges = ARRAY_SIZE(si7210_volatile_reg_ranges),
};

static const struct regmap_config si7210_regmap_conf = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = SI7210_REG_OTP_CTRL,

	.rd_table = &si7210_readable_regs,
	.wr_table = &si7210_writeable_regs,
	.volatile_table = &si7210_volatile_regs,
};

struct si7210_data {
	struct i2c_client* client;
	struct regmap *regmap;
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

static int si7210_read_otpreg_val(struct si7210_data *data, unsigned int otpreg, unsigned int* val)
{
	unsigned int reg_otp_ctrl;
	int ret;

	ret = regmap_write(data->regmap, SI7210_REG_OTP_ADDR, otpreg);
	if (ret < 0)
		return ret;

	/* "When writing a particular bit field, it is best to use a read, modify, write
	procedure to ensure that other bit fields are not unintentionally changed."*/
	ret = regmap_read(data->regmap, SI7210_REG_OTP_CTRL, &reg_otp_ctrl);
	if (ret < 0)
		return ret;
	ret = regmap_write(data->regmap, SI7210_REG_OTP_CTRL,
			reg_otp_ctrl | SI7210_BIT_OTP_READ_EN);
	if (ret < 0)
		return ret;

	ret = regmap_read(data->regmap, SI7210_REG_OTP_DATA, val);
	if (ret < 0)
		return ret;

	return 0;
}

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

	data->regmap = devm_regmap_init_i2c(client, &si7210_regmap_conf);
	if (IS_ERR(data->regmap))
		return dev_err_probe(&client->dev, PTR_ERR(data->regmap),
				"failed to register regmap\n");

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
