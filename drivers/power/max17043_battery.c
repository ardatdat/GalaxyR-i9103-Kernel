/*
 *  max17043_battery.c
 *  fuel-gauge systems for lithium-ion (Li+) batteries
 *
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/max17043_battery.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#define MAX17043_VCELL		0x02
#define MAX17043_SOC		0x04
#define MAX17043_MODE		0x06
#define MAX17043_VERSION	0x08
#define MAX17043_CONFIG	0x0C
#define MAX17043_COMMAND	0xFE

#define MAX17043_DELAY		1000
#define MAX17043_BATTERY_FULL	95

/* 5% alert setting */
#define MAX17043_5_ALERT	0x1B

static struct i2c_client *fg_i2c_client;
static struct work_struct low_bat_work;
static struct wake_lock low_battery_wake_lock;
int  max17043_low_battery = 0;

struct max17043_chip {
	struct i2c_client		*client;
	struct power_supply		battery;
	struct max17043_platform_data	*pdata;

	/* State Of Connect */
	int online;
	/* battery voltage */
	int vcell;
	/* battery capacity */
	int soc;
	/* State Of Charge */
	int status;
	/* State Of Charge */
	int pure_soc;
	/* State Of Charge */
	int comp_full_temp;
};

static int max17043_write_reg(struct i2c_client *client, int reg, u16 value)
{
	int ret;

	ret = i2c_smbus_write_word_data(client, reg, swab16(value));

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int max17043_read_reg(struct i2c_client *client, int reg)
{
	int ret;

	ret = swab16(i2c_smbus_read_word_data(client, reg));

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static void max17043_reset(struct i2c_client *client)
{
	u16 rst_cmd = 0x4000;

	max17043_write_reg(client, MAX17043_MODE, rst_cmd);
	msleep(500);

	dev_info(&client->dev, "MAX17043 Quick star! \n");
}

static void max17043_set_rcomp(struct i2c_client *client, u8 rcomp)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	u16 config = 0;

#if defined (CONFIG_MACH_BOSE_ATT)
	if (chip->soc <= 5)
		config = ((rcomp << 8) | chip->pdata->alert_flag);
	else
		config = ((rcomp << 8) | MAX17043_5_ALERT);
#else
	config = ((rcomp << 8) | chip->pdata->alert_flag);
#endif

	max17043_write_reg(client, MAX17043_CONFIG, config);
}

static void max17043_calc_rcomp_from_temp(struct i2c_client *client, int temp)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	struct power_supply *psp = power_supply_get_by_name("battery");
	union power_supply_propval val;

	s16 temp_rcomp = 0;
	u8 new_rcomp = 0;

	if (!psp) {
		pr_err("%s: fail to get battery ps\n", __func__);
		return;
	}

	psp->get_property(psp, POWER_SUPPLY_PROP_STATUS, &val);

	if (val.intval == POWER_SUPPLY_STATUS_CHARGING) {
		if (temp < chip->pdata->standard_temp)
			temp_rcomp = chip->pdata->charging_rcomp+ (5 * ( chip->pdata->standard_temp - temp));
		else
			temp_rcomp = chip->pdata->charging_rcomp - (22 * (temp - chip->pdata->standard_temp) / 10);
	} else {
		if (temp < chip->pdata->standard_temp)
			temp_rcomp = chip->pdata->discharging_rcomp + (5 * ( chip->pdata->standard_temp - temp));
		else
			temp_rcomp = chip->pdata->discharging_rcomp - (22 * (temp - chip->pdata->standard_temp) / 10);
	}

	if (temp_rcomp > 0xFF)
		new_rcomp = 0xFF;
	else if (temp_rcomp < 0)
		new_rcomp = 0;
	else
		new_rcomp = temp_rcomp;

	max17043_set_rcomp(client, new_rcomp);
}

#define COMP_FULL_MARGIN	300	/* 3% */
static void max17043_update_full(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	if (chip->pure_soc > chip->pdata->comp_full - COMP_FULL_MARGIN)
		chip->comp_full_temp =
			(chip->pure_soc > 10000) ? 10000 : chip->pure_soc;
	else
		chip->comp_full_temp = chip->pdata->comp_full -
			COMP_FULL_MARGIN;

	chip->comp_full_temp =
		((chip->comp_full_temp - chip->pdata->comp_empty) * 99 / 100) +
		chip->pdata->comp_empty;

	pr_info("%s: comp_full is update as %d\n",
		__func__, chip->comp_full_temp);
}

static int max17043_set_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    const union power_supply_propval *val)
{
	struct max17043_chip *chip = container_of(psy,
				struct max17043_chip, battery);

	int temp;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (val->intval == POWER_SUPPLY_STATUS_FULL)
			max17043_update_full(chip->client);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		max17043_update_full(chip->client);
		break;
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
		temp = val->intval/10;
		max17043_calc_rcomp_from_temp(chip->client, temp);
		break;
	default:
		return -EINVAL;
	}
}

void max17043_reset_soc(void)
{
	struct i2c_client *client = fg_i2c_client;
	max17043_reset(client);
}
EXPORT_SYMBOL(max17043_reset_soc);

static void max17043_get_vcell(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	u16 data;

	data = max17043_read_reg(client, MAX17043_VCELL);

	chip->vcell = (data >> 4) * 1250;
}

static void max17043_get_soc(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	u16 data;
	int pure_soc, adj_soc, soc;

	data = max17043_read_reg(client, MAX17043_SOC);
	pure_soc = ((data * 100)>> 8);

	adj_soc = (pure_soc - chip->pdata->comp_empty) * 100 /
		(chip->comp_full_temp - chip->pdata->comp_empty);

	if (adj_soc > 100)
		adj_soc = 100;
	else if (adj_soc < 0)
		adj_soc = 0;
	else if (pure_soc < 100)	/* if fuel alert is generated, power  off will begin */
		adj_soc = 0;

	chip->soc = adj_soc;
	chip->pure_soc = pure_soc;
}

static void max17043_get_version(struct i2c_client *client)
{
	u16 data;

	data = max17043_read_reg(client, MAX17043_VERSION);

	dev_info(&client->dev, "MAX17043 Fuel-Gauge Ver %x\n", data);
}

static void max17043_init_register(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	int ret;
	u16 config;

#if defined (CONFIG_MACH_BOSE_ATT)
	max17043_get_soc(client);
	if (chip->soc <= 5)
		config = ((chip->pdata->discharging_rcomp << 8) | chip->pdata->alert_flag);
	else
		config = ((chip->pdata->discharging_rcomp << 8) | MAX17043_5_ALERT);
#else
	config = ((chip->pdata->discharging_rcomp << 8) | chip->pdata->alert_flag);
#endif

	/* RCOMP = 0xD7, Alert is generated when soc is below 1% */
	ret = max17043_write_reg(client, MAX17043_CONFIG, config);
	config = max17043_read_reg(client, MAX17043_CONFIG);

	dev_info(&client->dev, "MAX17043_CONFIG 0x%04x\n", config);
}

static void max17043_get_online(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	if (chip->pdata->battery_online)
		chip->online = chip->pdata->battery_online();
	else
		chip->online = 1;
}

static void max17043_get_status(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	if (!chip->pdata->charger_online || !chip->pdata->charger_enable) {
		chip->status = POWER_SUPPLY_STATUS_UNKNOWN;
		return;
	}

	if (chip->pdata->charger_online()) {
		if (chip->pdata->charger_enable())
			chip->status = POWER_SUPPLY_STATUS_CHARGING;
		else
			chip->status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	} else {
		chip->status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	if (chip->soc > MAX17043_BATTERY_FULL)
		chip->status = POWER_SUPPLY_STATUS_FULL;
}

static int max17043_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct max17043_chip *chip = container_of(psy,
				struct max17043_chip, battery);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		max17043_get_status(chip->client);
		val->intval = chip->status;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		max17043_get_online(chip->client);
		val->intval = chip->online;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		max17043_get_vcell(chip->client);
		val->intval = chip->vcell;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		max17043_get_soc(chip->client);
		val->intval = chip->soc;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		max17043_get_soc(chip->client);
		val->intval = chip->pure_soc;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static bool max17043_check_status(struct i2c_client *client)
{
	bool ret = false;
	u16 data;
	struct max17043_chip *chip = i2c_get_clientdata(client);

	data = max17043_read_reg(client, MAX17043_CONFIG);

	dev_info(&client->dev, "%s : MAX17043_CONFIG(%x)\n",
			__func__, data);

	/* check if ALRT was generated */
	if (data & (0x1 << 5))
		ret = true;

	/* clear ALRT bit */
	data &= ~(0x1 << 5);
	max17043_write_reg(client, MAX17043_CONFIG, data);

	/* update SOC */
	max17043_get_soc(client);

	dev_info(&client->dev, "%s : soc = %d\n",
			__func__, chip->soc);

	return ret;
}

static void max17043_low_bat_work(struct work_struct *work)
{
	struct i2c_client *client  = fg_i2c_client;
	struct max17043_chip *chip = i2c_get_clientdata(client);
	int new_config = 0, config = 0, athd = 0; /* Alert Threshold*/
	int ret =0;

	if (max17043_check_status(chip->client))
		chip->pdata->low_batt_cb();

	config = max17043_read_reg(client, MAX17043_CONFIG);
	athd = config & 0x1f;

	if(athd  != chip->pdata->alert_flag) {
		/* soc 5% alret */
		new_config = ((config & 0xff00) | chip->pdata->alert_flag);

		ret = max17043_write_reg(client, MAX17043_CONFIG, new_config);
		config = max17043_read_reg(client, MAX17043_CONFIG);
		dev_info(&client->dev, "%s : max17043 config update 0x%04x\n", __func__, config);
	}

	wake_lock_timeout(&low_battery_wake_lock, HZ * 120);
}

static irqreturn_t max17043_irq_thread(int irq, void *data)
{
	printk("%s : fuel alert is generated\n", __func__);
	schedule_work(&low_bat_work);

	return IRQ_HANDLED;
}

#if 0
static irqreturn_t max17043_irq_thread(int irq, void *data)
{
	struct max17043_chip *chip = data;

	if (max17043_check_status(chip->client))
		chip->pdata->low_batt_cb();

	return IRQ_HANDLED;
}
#endif

static int max17043_irq_init(struct max17043_chip *chip)
{
	int ret;

	if (chip->client->irq) {
		ret = request_threaded_irq(chip->client->irq, NULL,
			max17043_irq_thread, IRQ_TYPE_EDGE_FALLING,
			"max17043 fuel alert", chip);
		if (ret) {
			dev_err(&chip->client->dev, "failed to reqeust IRQ\n");
			return ret;
		}

		ret = enable_irq_wake(chip->client->irq);
		if (ret < 0)
			dev_err(&chip->client->dev,
				"failed to enable wakeup src %d\n", ret);
	}

	return 0;
}

static enum power_supply_property max17043_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
};

static int __devinit max17043_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct max17043_chip *chip;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = fg_i2c_client = client;
	chip->pdata = client->dev.platform_data;

	i2c_set_clientdata(client, chip);

	chip->battery.name		= "fuelgauge";
	chip->battery.type		= POWER_SUPPLY_TYPE_BATTERY;
	chip->battery.get_property	= max17043_get_property;
	chip->battery.set_property = max17043_set_property;
	chip->battery.properties	= max17043_battery_props;
	chip->battery.num_properties	= ARRAY_SIZE(max17043_battery_props);

	chip->comp_full_temp = chip->pdata->comp_full;

	ret = power_supply_register(&client->dev, &chip->battery);
	if (ret) {
		dev_err(&client->dev, "failed: power supply register\n");
		kfree(chip);
		return ret;
	}

	INIT_WORK(&low_bat_work, max17043_low_bat_work);
	wake_lock_init(&low_battery_wake_lock, WAKE_LOCK_SUSPEND, "low_battery_wake_lock");
	max17043_get_version(client);
	max17043_init_register(client);

	/* register low batt intr*/
	ret = max17043_irq_init(chip);
	if (ret)
		goto err_kfree;

	return 0;

err_kfree:
	kfree(chip);
	return ret;
}

static int __devexit max17043_remove(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	power_supply_unregister(&chip->battery);
	kfree(chip);
	return 0;
}

#ifdef CONFIG_PM

static int max17043_suspend(struct i2c_client *client,
		pm_message_t state)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	return 0;
}

static int max17043_resume(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	return 0;
}

#else

#define max17043_suspend NULL
#define max17043_resume NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id max17043_id[] = {
	{ "max17043", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max17043_id);

static struct i2c_driver max17043_i2c_driver = {
	.driver	= {
		.name	= "max17043",
	},
	.probe		= max17043_probe,
	.remove		= __devexit_p(max17043_remove),
	.suspend	= max17043_suspend,
	.resume		= max17043_resume,
	.id_table	= max17043_id,
};

static int __init max17043_init(void)
{
	return i2c_add_driver(&max17043_i2c_driver);
}
module_init(max17043_init);

static void __exit max17043_exit(void)
{
	i2c_del_driver(&max17043_i2c_driver);
}
module_exit(max17043_exit);

MODULE_AUTHOR("Minkyu Kang <mk7.kang@samsung.com>");
MODULE_DESCRIPTION("MAX17043 Fuel Gauge");
MODULE_LICENSE("GPL");

