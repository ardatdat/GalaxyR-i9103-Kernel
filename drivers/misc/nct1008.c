/*
 * drivers/misc/nct1008.c
 *
 * Driver for NCT1008, temperature monitoring device from ON Semiconductors
 *
 * Copyright (c) 2010, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/gpio.h>

#include <linux/nct1008.h>
#if defined(CONFIG_MACH_N1)
#include <linux/hwmon.h>
#endif

#define DRIVER_NAME "nct1008"

/* Register Addresses */
#define LOCAL_TEMP_RD				0x00
#define EXTERNAL_TEMP_RD			0x01
#define STATUS_RD					0x02
#define CONFIG_RD					0x03

#define CONFIG_WR					0x09
#define CONV_RATE_WR				0x0A
#define LOCAL_TEMP_HI_LIMIT_WR		0x0B
#define EXT_TEMP_HI_LIMIT_HI_BYTE	0x0D
#define OFFSET_WR					0x11
#define EXT_THERM_LIMIT_WR			0x19
#define LOCAL_THERM_LIMIT_WR		0x20
#define THERM_HYSTERESIS_WR			0x21

/* Configuration Register Bits */
#define EXTENDED_RANGE_BIT			(0x1 << 2)
#define THERM2_BIT					(0x1 << 5)
#define STANDBY_BIT					(0x1 << 6)

/* Max Temperature Measurements */
#define EXTENDED_RANGE_OFFSET		64U
#define STANDARD_RANGE_MAX			127U
#define EXTENDED_RANGE_MAX			(150U + EXTENDED_RANGE_OFFSET)

#define NCT1008_DELAY 1000

struct nct1008_data {
	struct work_struct work;
	struct i2c_client *client;
	struct mutex mutex;
#if defined(CONFIG_MACH_N1)
	struct device *hwmon_dev;
#endif
	u8 config;
	void (*alarm_fn)(bool raised);
	struct nct1008_temp_data *pdata;
	struct nct1008_temp_callbacks callbacks;
};

#ifdef CONFIG_MACH_N1
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/delay.h>
#endif

#if defined(CONFIG_MACH_N1)
static struct i2c_client *g_client;
static ssize_t nct1008_show_temp(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	signed int temp_value = 0;
	u8 data = 0;

	if (!dev || !buf || !attr)
		return -EINVAL;

	data = i2c_smbus_read_byte_data(g_client, STATUS_RD);
	if (data < 0) {
		dev_err(&g_client->dev, "%s: failed to read "
			"temperature\n", __func__);
		return -EINVAL;
	}

	if ((data&0x80) == 0x80)
		udelay(500);

	data = i2c_smbus_read_byte_data(g_client, LOCAL_TEMP_RD);
	if (data < 0) {
		dev_err(&g_client->dev, "%s: failed to read "
			"temperature\n", __func__);
		return -EINVAL;
	}

	temp_value = (signed int)data - 64; /* extended mode */

	printk(KERN_DEBUG "[TM] local temp : %d\n", temp_value);

	data = i2c_smbus_read_byte_data(g_client, EXTERNAL_TEMP_RD);
	if (data < 0) {
		dev_err(&g_client->dev, "%s: failed to read "
			"temperature\n", __func__);
		return -EINVAL;
	}

	printk(KERN_DEBUG "[TM] ext temp : %d\n", data);

	return sprintf(buf, "%d", temp_value);
}
static DEVICE_ATTR(temp, S_IRUGO, nct1008_show_temp, NULL);
#endif

static int
	nct1008_configure_sensor(struct nct1008_data *data);
static void nct1008_re_enable(struct i2c_client *client);

static int nct1008_update_local_temp(struct nct1008_temp_callbacks *ptr)
{
	u8 raw_temp_value = 0, external_temp;
	u8 config_reg;
	u8 status_reg;
	int temp_value = 0;
	int extended_range_offset = 64;
	int err;

	struct nct1008_data *data = container_of(ptr,
		struct nct1008_data, callbacks);
	struct i2c_client *client = data->client;

	config_reg = i2c_smbus_read_byte_data(client, CONFIG_RD);
	if (config_reg == 0x0) {
		dev_err(&client->dev,
			"%s: nct1008 was reset! re-init!\n", __func__);

		err = nct1008_configure_sensor(data); /* sensor is in standby */
		if (err < 0) {
			dev_err(&client->dev,
				"%s: nct1008_configure_sensor error(%d)\n",
				__func__, err);
			return -EINVAL;
		}
		nct1008_re_enable(client);
		msleep(500);	/* it shoud be updated at least one time */
	}

	raw_temp_value = i2c_smbus_read_byte_data(client, STATUS_RD);
	if (raw_temp_value < 0) {
		dev_err(&client->dev, "%s: failed to read "
			"temperature\n", __func__);
		return -EINVAL;
	}

	if ((raw_temp_value&0x80) == 0x80)
		udelay(500);

	raw_temp_value = i2c_smbus_read_byte_data(client, LOCAL_TEMP_RD);
	temp_value = (int)raw_temp_value - extended_range_offset;

	if (raw_temp_value < 0) {
		dev_err(&client->dev, "%s: failed to read "
			"temperature\n", __func__);
		return -EINVAL;
	}

	external_temp = i2c_smbus_read_byte_data(client, EXTERNAL_TEMP_RD);
	if (external_temp < 0) {
		dev_err(&client->dev, "%s: failed to read "
			"temperature (%d), (%d)\n", __func__,
			raw_temp_value, external_temp);
	}

	/* REMOVE this */
	status_reg = i2c_smbus_read_byte_data(client, STATUS_RD);

	pr_info("NCT1008 : temp(%d), raw(%d), ext(%d),"
				"config(0x%02x), status(0x%02x)\n",
		temp_value,
		raw_temp_value,
		external_temp,
		config_reg,
		status_reg);

	return temp_value;
}

static void nct1008_re_enable(struct i2c_client *client)
{
	int ret;
	struct nct1008_data *data = i2c_get_clientdata(client);

	i2c_smbus_write_byte_data(client, CONFIG_WR,
				  data->config & ~STANDBY_BIT);

	ret = i2c_smbus_read_byte_data(client, CONFIG_RD);
	pr_info("NCT1008 : Config reg = 0x%x\n", ret);
}

static void nct1008_enable(struct i2c_client *client)
{
	int ret;
	struct nct1008_data *data = i2c_get_clientdata(client);
	struct regulator *reg = regulator_get(NULL, "VADC_3V3");

	regulator_enable(reg);
	regulator_put(reg);

	i2c_smbus_write_byte_data(client, CONFIG_WR,
				  data->config & ~STANDBY_BIT);

	/*
	ret = i2c_smbus_read_byte_data(client, CONFIG_RD);
	pr_info("NCT1008 : Config reg = 0x%x\n", ret);
	*/
}

static void nct1008_disable(struct i2c_client *client)
{
	int ret;
	struct nct1008_data *data = i2c_get_clientdata(client);
	struct regulator *reg = regulator_get(NULL, "VADC_3V3");

	i2c_smbus_write_byte_data(client, CONFIG_WR,
				  data->config | STANDBY_BIT);

	ret = i2c_smbus_read_byte_data(client, CONFIG_RD);

	regulator_disable(reg);
	regulator_put(reg);
}



static void nct1008_work_func(struct work_struct *work)
{
	struct nct1008_data *data = container_of(work,
						struct nct1008_data, work);
	int irq = data->client->irq;

	mutex_lock(&data->mutex);

	if (data->alarm_fn) {
		/* Therm2 line is active low */
		data->alarm_fn(!gpio_get_value(irq_to_gpio(irq)));
	}

	mutex_unlock(&data->mutex);
}

static irqreturn_t nct1008_irq(int irq, void *dev_id)
{
	struct nct1008_data *data = dev_id;
	schedule_work(&data->work);

	return IRQ_HANDLED;
}

static inline u8 value_to_temperature(bool extended, u8 value)
{
	u8 data;

	if (extended)
		data = (u8)(value - EXTENDED_RANGE_OFFSET);
	else
		data = value;

	return data;
}

static inline u8 temperature_to_value(bool extended, u8 temp)
{
	u8 data;

	if (extended)
		data = temp + EXTENDED_RANGE_OFFSET;
	else
		data = temp;

	return data;
}

static int nct1008_configure_sensor(struct nct1008_data *data)
{
	struct i2c_client *client		   = data->client;
	struct nct1008_platform_data *pdata = client->dev.platform_data;
	u8 value;
	int err;

	if (!pdata || !pdata->supported_hwrev)
		return -ENODEV;

	/*
	 * Initial Configuration - device is placed in standby and
	 * ALERT/THERM2 pin is configured as THERM2
	 */
	data->config = value = pdata->ext_range ?
		(STANDBY_BIT | THERM2_BIT | EXTENDED_RANGE_BIT) :
		(STANDBY_BIT | THERM2_BIT);

	err = i2c_smbus_write_byte_data(client, CONFIG_WR, value);
	if (err < 0)
		goto error;

	/* Temperature conversion rate */
	err = i2c_smbus_write_byte_data(client, CONV_RATE_WR,
							pdata->conv_rate);
	if (err < 0)
		goto error;

	/* External temperature h/w shutdown limit */
	value = temperature_to_value(pdata->ext_range,
						pdata->shutdown_ext_limit);
	err = i2c_smbus_write_byte_data(client, EXT_THERM_LIMIT_WR, value);
	if (err < 0)
		goto error;

	/* Local temperature h/w shutdown limit */
	value = temperature_to_value(pdata->ext_range,
			pdata->shutdown_local_limit);
	err = i2c_smbus_write_byte_data(client, LOCAL_THERM_LIMIT_WR, value);
	if (err < 0)
		goto error;

	/* External Temperature Throttling limit */
	value = temperature_to_value(pdata->ext_range,
					pdata->throttling_ext_limit);
	err = i2c_smbus_write_byte_data(client,
			EXT_TEMP_HI_LIMIT_HI_BYTE, value);
	if (err < 0)
		goto error;

	/* Local Temperature Throttling limit */
	value = pdata->ext_range ? EXTENDED_RANGE_MAX : STANDARD_RANGE_MAX;
	err = i2c_smbus_write_byte_data(client, LOCAL_TEMP_HI_LIMIT_WR, value);
	if (err < 0)
		goto error;

	/* Remote channel offset */
	err = i2c_smbus_write_byte_data(client, OFFSET_WR, pdata->offset);
	if (err < 0)
		goto error;

	/* THERM hysteresis */
	err = i2c_smbus_write_byte_data(client,
		THERM_HYSTERESIS_WR, pdata->hysteresis);
	if (err < 0)
		goto error;

	data->alarm_fn = pdata->alarm_fn;
	return 0;
error:
	return err;
}

static int __devinit nct1008_configure_irq(struct nct1008_data *data)
{
	INIT_WORK(&data->work, nct1008_work_func);

	return request_irq(data->client->irq, nct1008_irq,
				IRQF_TRIGGER_RISING |
				IRQF_TRIGGER_FALLING, DRIVER_NAME, data);
}

static int __devinit nct1008_probe(struct i2c_client *client,
						const struct i2c_device_id *id)
{
	int err;
	struct nct1008_data *data;
	struct nct1008_platform_data *pdata = client->dev.platform_data;

	pr_info("NCT1008 Driver Loading\n");

	data = kzalloc(sizeof(struct nct1008_data), GFP_KERNEL);

	if (!data)
		return -ENOMEM;

	data->client = client;
	data->pdata = pdata->temp;
	i2c_set_clientdata(client, data);
	mutex_init(&data->mutex);

	err = nct1008_configure_sensor(data);	/* sensor is in standby */
	if (err < 0)
		goto error;

	err = nct1008_configure_irq(data);
	if (err < 0)
		goto error;

	nct1008_enable(client);		/* sensor is running */

	data->callbacks.get_temperature = nct1008_update_local_temp;
	if (data->pdata->register_callbacks)
			data->pdata->register_callbacks(&data->callbacks);

#if defined(CONFIG_MACH_N1)
	g_client = client;
	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		dev_err(&client->dev, "%s: hwmon_device_register "
			"failed\n", __func__);
		goto error;
	}

	if (device_create_file(data->hwmon_dev,
				&dev_attr_temp) < 0) {
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_temp.attr.name);
		goto error;
	}
#endif

	return 0;

error:
	kfree(data);
	return err;
}

static int __devexit nct1008_remove(struct i2c_client *client)
{
	struct nct1008_data *data = i2c_get_clientdata(client);

	free_irq(data->client->irq, data);
	cancel_work_sync(&data->work);
	kfree(data);

	return 0;
}

#ifdef CONFIG_PM
static int nct1008_suspend(struct i2c_client *client, pm_message_t state)
{
	disable_irq(client->irq);
	nct1008_disable(client);

	return 0;
}

static int nct1008_resume(struct i2c_client *client)
{
	struct nct1008_data *data = i2c_get_clientdata(client);

	nct1008_enable(client);
	enable_irq(client->irq);
	schedule_work(&data->work);

	return 0;
}
#endif

static const struct i2c_device_id nct1008_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, nct1008_id);

static struct i2c_driver nct1008_driver = {
	.driver = {
		.name	= DRIVER_NAME,
	},
	.probe		= nct1008_probe,
	.remove		= __devexit_p(nct1008_remove),
	.id_table	= nct1008_id,
#ifdef CONFIG_PM
	.suspend	= nct1008_suspend,
	.resume		= nct1008_resume,
#endif
};

static int __init nct1008_init(void)
{
	return i2c_add_driver(&nct1008_driver);
}

static void __exit nct1008_exit(void)
{
	i2c_del_driver(&nct1008_driver);
}

MODULE_DESCRIPTION("Temperature sensor driver for OnSemi NCT1008");
MODULE_LICENSE("GPL");

module_init(nct1008_init);
module_exit(nct1008_exit);
