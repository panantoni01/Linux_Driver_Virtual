// SPDX-License-Identifier: GPL-2.0-only
/*
 * si7020.c - Silicon Labs Si7013/20/21 Relative Humidity and Temp Sensors
 * Copyright (c) 2013,2014  Uplogix, Inc.
 * David Barksdale <dbarksdale@uplogix.com>
 */

/*
 * The Silicon Labs Si7013/20/21 Relative Humidity and Temperature Sensors
 * are i2c devices which have an identical programming interface for
 * measuring relative humidity and temperature. The Si7013 has an additional
 * temperature input which this driver does not support.
 *
 * Data Sheets:
 *   Si7013: http://www.silabs.com/Support%20Documents/TechnicalDocs/Si7013.pdf
 *   Si7020: http://www.silabs.com/Support%20Documents/TechnicalDocs/Si7020.pdf
 *   Si7021: http://www.silabs.com/Support%20Documents/TechnicalDocs/Si7021.pdf
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/stat.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

/* Measure Relative Humidity, Hold Master Mode */
#define SI7020CMD_RH_HOLD	0xE5
/* Measure Temperature, Hold Master Mode */
#define SI7020CMD_TEMP_HOLD	0xE3
/* Software Reset */
#define SI7020CMD_RESET		0xFE
/* Write User Register */
#define SI7020CMD_USR_WRITE 0xE6
/* "Heater Enabled" bit in the User Register */
#define SI7020_USR_HTRE		BIT(2)

struct si7020_data {
	struct i2c_client *client;
	struct mutex lock;
	u8 user_reg;
};

static int si7020_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan, int *val,
			   int *val2, long mask)
{
	struct si7020_data *data = iio_priv(indio_dev);
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = i2c_smbus_read_word_swapped(data->client,
						  chan->type == IIO_TEMP ?
						  SI7020CMD_TEMP_HOLD :
						  SI7020CMD_RH_HOLD);
		if (ret < 0)
			return ret;
		*val = ret >> 2;
		/*
		 * Humidity values can slightly exceed the 0-100%RH
		 * range and should be corrected by software
		 */
		if (chan->type == IIO_HUMIDITYRELATIVE)
			*val = clamp_val(*val, 786, 13893);
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
		if (chan->type == IIO_TEMP)
			*val = 175720; /* = 175.72 * 1000 */
		else
			*val = 125 * 1000;
		*val2 = 65536 >> 2;
		return IIO_VAL_FRACTIONAL;
	case IIO_CHAN_INFO_OFFSET:
		/*
		 * Since iio_convert_raw_to_processed_unlocked assumes offset
		 * is an integer we have to round these values and lose
		 * accuracy.
		 * Relative humidity will be 0.0032959% too high and
		 * temperature will be 0.00277344 degrees too high.
		 * This is no big deal because it's within the accuracy of the
		 * sensor.
		 */
		if (chan->type == IIO_TEMP)
			*val = -4368; /* = -46.85 * (65536 >> 2) / 175.72 */
		else
			*val = -786; /* = -6 * (65536 >> 2) / 125 */
		return IIO_VAL_INT;
	default:
		break;
	}

	return -EINVAL;
}

static const struct iio_chan_spec si7020_channels[] = {
	{
		.type = IIO_HUMIDITYRELATIVE,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_OFFSET),
	},
	{
		.type = IIO_TEMP,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_OFFSET),
	}
};

static int si7020_update_user_reg(struct si7020_data *data, u8 mask, u8 val)
{
	u8 new = (data->user_reg & ~mask) | val;
	int ret;

	ret = i2c_smbus_write_byte_data(data->client, SI7020CMD_USR_WRITE, new);
	if (!ret)
		data->user_reg = new;

	return ret;
}

static ssize_t si7020_show_heater_en(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct si7020_data *data = iio_priv(indio_dev);

	return sysfs_emit(buf, "%d\n", !!(data->user_reg & SI7020_USR_HTRE));
}

static ssize_t si7020_store_heater_en(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t len)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct si7020_data *data = iio_priv(indio_dev);
	int ret;
	bool val;

	ret = kstrtobool(buf, &val);
	if (ret)
		return ret;

	mutex_lock(&data->lock);
	ret = si7020_update_user_reg(data, SI7020_USR_HTRE, val ? SI7020_USR_HTRE : 0);
	mutex_unlock(&data->lock);

	return ret < 0 ? ret : len;
}

static IIO_DEVICE_ATTR(heater_enable, S_IRUGO | S_IWUSR,
		       si7020_show_heater_en, si7020_store_heater_en, 0);

static struct attribute *si7020_attributes[] = {
	&iio_dev_attr_heater_enable.dev_attr.attr,
	NULL,
};

static const struct attribute_group si7020_attribute_group = {
	.attrs = si7020_attributes,
};

static const struct iio_info si7020_info = {
	.read_raw = si7020_read_raw,
	.attrs = &si7020_attribute_group,
};

static int si7020_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct iio_dev *indio_dev;
	struct si7020_data *data;
	int ret;

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_WRITE_BYTE |
				     I2C_FUNC_SMBUS_READ_WORD_DATA))
		return -EOPNOTSUPP;

	/* Reset device, loads default settings. */
	ret = i2c_smbus_write_byte(client, SI7020CMD_RESET);
	if (ret < 0)
		return ret;
	/* Wait the maximum power-up time after software reset. */
	msleep(15);

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	i2c_set_clientdata(client, indio_dev);
	data->client = client;
	mutex_init(&data->lock);

	indio_dev->name = dev_name(&client->dev);
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->info = &si7020_info;
	indio_dev->channels = si7020_channels;
	indio_dev->num_channels = ARRAY_SIZE(si7020_channels);

	/* Default User Register value */
	data->user_reg = 0x3A;

	return devm_iio_device_register(&client->dev, indio_dev);
}

static const struct i2c_device_id si7020_id[] = {
	{ "my_driver_si7020", 0 },
	{ "th06", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, si7020_id);

static const struct of_device_id si7020_dt_ids[] = {
	{ .compatible = "si7021" },
	{ }
};
MODULE_DEVICE_TABLE(of, si7020_dt_ids);

static struct i2c_driver si7020_driver = {
	.driver = {
		.name = "my_driver_si7020",
		.of_match_table = si7020_dt_ids,
	},
	.probe		= si7020_probe,
	.id_table	= si7020_id,
};

module_i2c_driver(si7020_driver);
MODULE_DESCRIPTION("Silicon Labs Si7013/20/21 Relative Humidity and Temperature Sensors");
MODULE_AUTHOR("David Barksdale <dbarksdale@uplogix.com>");
MODULE_LICENSE("GPL");
