/* arch/arm/mach-tegra/n1-vibrator.c
 *
 * Copyright (C) 2010 Samsung Electronics Co. Ltd. All Rights Reserved.
 * Based on arch/arm/mach-s5pv210/herring-vibrator.c
 * by Rom Lemarchand <rlemarchand@sta.samsung.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/pwm.h>
#include <linux/wakelock.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/workqueue.h>

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#include <asm/mach-types.h>
#if defined (CONFIG_MACH_BOSE_ATT)
#include <mach/gpio-bose.h>
#else
#include <mach/gpio-n1.h>
#endif
#include "../../../drivers/staging/android/timed_output.h"
#include <mach/pinmux.h>
#include "clock.h"

#define MAX_TIMEOUT		10000 /* 10s */

static struct vibrator {
	struct wake_lock wklock;
	struct hrtimer timer;
	struct mutex lock;
	struct work_struct work;
	struct clk *dap_mclk2;
	struct i2c_client *client;
	bool running;
} vibdata;

static int vibrator_write_register(u8 addr, u8 w_data)
{
	int ret;

#if 0 /* HW I2C */

	u8 buf[2];

	buf[0] = addr;
	buf[1] = w_data;

	ret = i2c_master_send(vibdata.client, buf, 2);
	if (ret < 0) {
		pr_err("%s: Failed to write addr=[0x%x], data=[0x%x] (ret:%d)\n",
		   __func__, addr, w_data, ret);
		return -1;
	}

#else
	ret = i2c_smbus_write_byte_data(vibdata.client, addr, w_data);

	if (ret < 0) {
		pr_err("%s: Failed to write addr=[0x%x], data=[0x%x] (ret:%d)\n",
		   __func__, addr, w_data, ret);
		return -1;
	}
#endif
	return 0;
}

static void isa1200_resume(void)
{
	vibrator_write_register(0x30, 0x11);
	vibrator_write_register(0x31, 0xc0);
	vibrator_write_register(0x32, 0x00);
	vibrator_write_register(0x33, 0x23);
	vibrator_write_register(0x34, 0x00);
	vibrator_write_register(0x35, 0x43);
	vibrator_write_register(0x36, 0x86);

	vibrator_write_register(0x30, 0x91);
	vibrator_write_register(0x35, 0x85);

	pr_debug("%s\n", __func__);
}

static void isa1200_suspend(void)
{
	vibrator_write_register(0x35, 0x43);

	pr_debug("%s\n", __func__);
}


static void n1_vibrator_off(void)
{
	if (!vibdata.running)
		return;

	vibdata.running = false;
	isa1200_suspend();
	clk_disable(vibdata.dap_mclk2);
	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CDEV2, TEGRA_TRI_TRISTATE);
	gpio_set_value(GPIO_VIBTONE_EN, 0);
	wake_unlock(&vibdata.wklock);
}

static int n1_vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibdata.timer)) {
		ktime_t r = hrtimer_get_remaining(&vibdata.timer);
		return ktime_to_ms(r);
	}

	return 0;
}

static void n1_vibrator_enable(struct timed_output_dev *dev, int value)
{
	mutex_lock(&vibdata.lock);

	/* cancel previous timer and set GPIO according to value */
	hrtimer_cancel(&vibdata.timer);
	cancel_work_sync(&vibdata.work);
	if (value) {
		wake_lock(&vibdata.wklock);
		if (!vibdata.running) {
			int ret;

			pr_debug("%s: value = %d\n", __func__, value);
			gpio_set_value(GPIO_VIBTONE_EN, 1);
			ret = tegra_pinmux_set_tristate(TEGRA_PINGROUP_CDEV2,
							TEGRA_TRI_NORMAL);
			pr_debug("%s: tegra_pinmux_set_tristate() returned = %d\n",
				__func__, ret);
			ret = clk_enable(vibdata.dap_mclk2);
			pr_debug("%s: clk_enable() returned = %d\n",
				__func__, ret);
			mdelay(1);
			isa1200_resume();
			vibdata.running = true;
		} else
			pr_info("%s: value = %d, already running, rescheduling timer\n",
				__func__, value);

		if (value > 0) {
			value = value + 30;
			if (value > MAX_TIMEOUT)
				value = MAX_TIMEOUT;

			hrtimer_start(&vibdata.timer,
				ns_to_ktime((u64)value * NSEC_PER_MSEC),
				HRTIMER_MODE_REL);
		}
	}
//	else
//		n1_vibrator_off();

	mutex_unlock(&vibdata.lock);
}

static struct timed_output_dev to_dev = {
	.name		= "vibrator",
	.get_time	= n1_vibrator_get_time,
	.enable		= n1_vibrator_enable,
};

static enum hrtimer_restart n1_vibrator_timer_func(struct hrtimer *timer)
{
	schedule_work(&vibdata.work);
	return HRTIMER_NORESTART;
}

static void n1_vibrator_work(struct work_struct *work)
{
	n1_vibrator_off();
}

static int vibrator_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	vibdata.client = client;

	pr_info("vibrator I2C attach success!!!\n");
	return 0;
}

int imm_vibrator_i2c_write(u8 addr, int length, u8 *data)
{
	if (!vibdata.client) {
		pr_err("%s driver is not ready\n", __func__);
		return 0;
	}

	if ((addr>>1) != vibdata.client->addr)
		printk("%s i2c address(%x) not matched to %x\n", __func__, addr, vibdata.client->addr);

	if (length!=2)
		printk("%s length should be 2(len:%d)\n", __func__, length);

	return vibrator_write_register(data[0], data[1]);
}
EXPORT_SYMBOL(imm_vibrator_i2c_write);

int imm_vibrator_clk_enable(void)
{
	int ret;

	if (!vibdata.dap_mclk2) {
		printk("%s driver is not ready\n", __func__);
		return 0;
	}

	mutex_lock(&vibdata.lock);

	wake_lock(&vibdata.wklock);

	ret = tegra_pinmux_set_tristate(TEGRA_PINGROUP_CDEV2,
			TEGRA_TRI_NORMAL);

	pr_debug("%s: tegra_pinmux_set_tristate() returned = %d\n",
				__func__, ret);

	ret = clk_enable(vibdata.dap_mclk2);
	pr_debug("%s: clk_enable() returned = %d\n",
				__func__, ret);

	vibdata.running = true;

	mutex_unlock(&vibdata.lock);

	pr_debug("%s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(imm_vibrator_clk_enable);

int imm_vibrator_clk_disable(void)
{

	if (!vibdata.dap_mclk2) {
		printk("%s driver is not ready\n", __func__);
		return 0;
	}

	if(!vibdata.running)
		return 0;

	mutex_lock(&vibdata.lock);

	clk_disable(vibdata.dap_mclk2);
	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CDEV2, TEGRA_TRI_TRISTATE);

	vibdata.running = false;

	wake_unlock(&vibdata.wklock);

	mutex_unlock(&vibdata.lock);

	pr_debug("%s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(imm_vibrator_clk_disable);

int imm_vibrator_chip_enable(void)
{
	if (!vibdata.client) {
		printk("%s driver is not ready\n", __func__);
		return 0;
	}

	gpio_direction_output(GPIO_VIBTONE_EN, 1);
	mdelay(1);

	pr_debug("%s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(imm_vibrator_chip_enable);

int imm_vibrator_chip_disable(void)
{
	if (!vibdata.client) {
		printk("%s driver is not ready\n", __func__);
		return 0;
	}

	gpio_direction_output(GPIO_VIBTONE_EN, 0);

	pr_debug("%s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(imm_vibrator_chip_disable);

static const struct i2c_device_id vibrator_device_id[] = {
	{"isa1200", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, vibrator_device_id);


static struct i2c_driver vibrator_i2c_driver = {
	.driver = {
		.name = "isa1200",
		.owner = THIS_MODULE,
	},
	.probe     = vibrator_i2c_probe,
	.id_table  = vibrator_device_id,
};

static int __init n1_init_vibrator(void)
{
	int ret;

	if (system_rev < 0x5){
		pr_notice("%s : Vibrator  not support HW Rev =[%d] !!!\n",__func__,system_rev);
		return 0;
	}

	hrtimer_init(&vibdata.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibdata.timer.function = n1_vibrator_timer_func;
	INIT_WORK(&vibdata.work, n1_vibrator_work);

	vibdata.dap_mclk2  = tegra_get_clock_by_name("cdev2");

	tegra_gpio_enable(GPIO_VIBTONE_EN);
	ret = gpio_request(GPIO_VIBTONE_EN, "vibrator-en");
	if (ret < 0)
		goto err_gpio_req2;

	gpio_direction_output(GPIO_VIBTONE_EN, 0);
		ret = i2c_add_driver(&vibrator_i2c_driver);

	if (ret)
		pr_err("%s: i2c_add_driver() failed err = %d\n", __func__, ret);


	wake_lock_init(&vibdata.wklock, WAKE_LOCK_SUSPEND, "vibrator");
	mutex_init(&vibdata.lock);

	ret = timed_output_dev_register(&to_dev);
	if (ret < 0)
		goto err_to_dev_reg;

	return 0;

err_to_dev_reg:
	mutex_destroy(&vibdata.lock);
	wake_lock_destroy(&vibdata.wklock);
	gpio_free(GPIO_VIBTONE_EN);
err_gpio_req2:
	tegra_gpio_disable(GPIO_VIBTONE_EN);

	return ret;
}

device_initcall(n1_init_vibrator);
