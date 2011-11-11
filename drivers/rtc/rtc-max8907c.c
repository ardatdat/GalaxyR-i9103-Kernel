/*
 * RTC driver for Maxim MAX8907c
 *
 * Copyright (c) 2011, NVIDIA Corporation.
 * Based on drivers/rtc/rtc-max8925.c, Copyright (C) 2009-2010 Marvell International Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>
#include <linux/mfd/max8907c.h>

enum {
	RTC_SEC = 0,
	RTC_MIN,
	RTC_HOUR,
	RTC_WEEKDAY,
	RTC_DATE,
	RTC_MONTH,
	RTC_YEAR1,
	RTC_YEAR2,
};

#define TIME_NUM			8
#define ALARM_1SEC			(1 << 7)
#define HOUR_12				(1 << 7)
#define HOUR_AM_PM			(1 << 5)
#define ALARM0_IRQ			(1 << 3)
#define ALARM1_IRQ			(1 << 2)
#define ALARM0_STATUS			(1 << 2)
#define ALARM1_STATUS			(1 << 1)

struct max8907c_rtc_info {
	struct rtc_device	*rtc_dev;
	struct i2c_client	*i2c;
	struct max8907c		*chip;
};

static irqreturn_t rtc_update_handler(int irq, void *data)
{
	struct max8907c_rtc_info *info = (struct max8907c_rtc_info *)data;

	/* disable ALARM0 except for 1SEC alarm */
	max8907c_set_bits(info->i2c, MAX8907C_REG_ALARM0_CNTL, 0x7f, 0);
	rtc_update_irq(info->rtc_dev, 1, RTC_IRQF | RTC_AF);
	return IRQ_HANDLED;
}

#if defined(CONFIG_MACH_N1_CHN)
static irqreturn_t max8907c_rtc_alarm1_irq(int irq, void *data)
{
	struct max8907c_rtc_info *info = data;
	
	dev_info(info->rtc_dev, "%s:irq(%d)\n", __func__, irq);

	/* disable ALARM0 except for 1SEC alarm */
	rtc_update_irq(info->rtc_dev, 1, RTC_IRQF | RTC_AF);

	return IRQ_HANDLED;
}
#endif  /*--  CHN feature - power_on_alarm_bsystar --*/

int tm_calc(struct rtc_time *tm, u8 *buf, int len)
{
	if (len < TIME_NUM)
		return -EINVAL;
	tm->tm_year = (buf[RTC_YEAR2] >> 4) * 1000
			+ (buf[RTC_YEAR2] & 0xf) * 100
			+ (buf[RTC_YEAR1] >> 4) * 10
			+ (buf[RTC_YEAR1] & 0xf);
	tm->tm_year -= 1900;
	// RTC issue when July 30th to July 31th 
	tm->tm_mon = ((buf[RTC_MONTH] >> 4) & 0x01) * 10
			+ (buf[RTC_MONTH] & 0x0f) -1;
	tm->tm_mday = ((buf[RTC_DATE] >> 4) & 0x03) * 10
			+ (buf[RTC_DATE] & 0x0f);
	tm->tm_wday = buf[RTC_WEEKDAY] & 0x07;
	if (buf[RTC_HOUR] & HOUR_12) {
		tm->tm_hour = ((buf[RTC_HOUR] >> 4) & 0x1) * 10
				+ (buf[RTC_HOUR] & 0x0f);
		if (buf[RTC_HOUR] & HOUR_AM_PM)
			tm->tm_hour += 12;
	} else {
		tm->tm_hour = ((buf[RTC_HOUR] >> 4) & 0x03) * 10
				+ (buf[RTC_HOUR] & 0x0f);
	}
	tm->tm_min = ((buf[RTC_MIN] >> 4) & 0x7) * 10
			+ (buf[RTC_MIN] & 0x0f);
	tm->tm_sec = ((buf[RTC_SEC] >> 4) & 0x7) * 10
			+ (buf[RTC_SEC] & 0x0f);
	return 0;
}

int data_calc(u8 *buf, struct rtc_time *tm, int len)
{
	u8 high, low;

	if (len < TIME_NUM)
		return -EINVAL;

	high = (tm->tm_year + 1900) / 1000;
	low = (tm->tm_year + 1900) / 100;
	low = low - high * 10;
	buf[RTC_YEAR2] = (high << 4) + low;
	high = (tm->tm_year + 1900) / 10;
	low = tm->tm_year + 1900;
	low = low - high * 10;
	high = high - (high / 10) * 10;
	buf[RTC_YEAR1] = (high << 4) + low;

	// RTC issue when July 30th to July 31th 
	high = (tm->tm_mon + 1) / 10;
	low = (tm->tm_mon + 1) % 10;
	buf[RTC_MONTH] = (high << 4) + low; 
	   
	high = tm->tm_mday / 10;
	low = tm->tm_mday;
	low = low - high * 10;
	buf[RTC_DATE] = (high << 4) + low;
	buf[RTC_WEEKDAY] = tm->tm_wday;
	high = tm->tm_hour / 10;
	low = tm->tm_hour;
	low = low - high * 10;
	buf[RTC_HOUR] = (high << 4) + low;
	high = tm->tm_min / 10;
	low = tm->tm_min;
	low = low - high * 10;
	buf[RTC_MIN] = (high << 4) + low;
	high = tm->tm_sec / 10;
	low = tm->tm_sec;
	low = low - high * 10;
	buf[RTC_SEC] = (high << 4) + low;
	return 0;
}

static int max8907c_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct max8907c_rtc_info *info = dev_get_drvdata(dev);
	u8 buf[TIME_NUM];
	int ret;

	ret = max8907c_reg_bulk_read(info->i2c, MAX8907C_REG_RTC_SEC, TIME_NUM, buf);

	if (ret < 0)
		return ret;
	ret = tm_calc(tm, buf, TIME_NUM);
	//tm->tm_year -= 1000;
	pr_info("%s: %02d:%02d:%02d %02d/%02d/%04d\n",
		__func__,
		tm->tm_hour, tm->tm_min, tm->tm_sec,
		tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900);
	return ret;
}

static int max8907c_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	struct max8907c_rtc_info *info = dev_get_drvdata(dev);
	u8 buf[TIME_NUM];
	int ret;
#ifdef CONFIG_MACH_N1
	struct task_struct *task;

	task = current;
	pr_info("%s called by %s[pid:%d]\n", __func__, task->comm, task->pid);
	pr_info("%s: %02d:%02d:%02d %02d/%02d/%04d\n",
		__func__,
		tm->tm_hour, tm->tm_min, tm->tm_sec,
		tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900);
#endif
	ret = data_calc(buf, tm, TIME_NUM);

	if (ret < 0)
		return ret;
	ret = max8907c_reg_bulk_write(info->i2c, MAX8907C_REG_RTC_SEC, TIME_NUM, buf);

	return ret;
}

static int max8907c_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct max8907c_rtc_info *info = dev_get_drvdata(dev);
	unsigned char buf[TIME_NUM];
	int ret;

	ret = max8907c_reg_bulk_read(info->i2c, MAX8907C_REG_ALARM0_SEC, TIME_NUM, buf);
	if (ret < 0)
		return ret;
	ret = tm_calc(&alrm->time, buf, TIME_NUM);
	if (ret < 0)
		return ret;
	ret = max8907c_reg_read(info->i2c, MAX8907C_REG_RTC_IRQ_MASK);
	if (ret < 0)
		return ret;
	if ((ret & ALARM0_IRQ) == 0)
		alrm->enabled = 1;
	else
		alrm->enabled = 0;
	ret = max8907c_reg_read(info->i2c, MAX8907C_REG_RTC_STATUS);
	if (ret < 0)
		return ret;
	if (ret & ALARM0_STATUS)
		alrm->pending = 1;
	else
		alrm->pending = 0;

	return ret;
}

static int max8907c_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct max8907c_rtc_info *info = dev_get_drvdata(dev);
	unsigned char buf[TIME_NUM];
	int ret;
#ifdef CONFIG_MACH_N1
	struct task_struct *task;

	task = current;
	pr_info("%s called by %s[pid:%d]\n", __func__, task->comm, task->pid);
	pr_info("%s: %02d:%02d:%02d %02d/%02d/%04d\n",
		__func__,
		alrm->time.tm_hour, alrm->time.tm_min, alrm->time.tm_sec,
		alrm->time.tm_mon + 1, alrm->time.tm_mday, alrm->time.tm_year + 1900);
#endif
	ret = data_calc(buf, &alrm->time, TIME_NUM);
	if (ret < 0)
		return ret;
	ret = max8907c_reg_bulk_write(info->i2c, MAX8907C_REG_ALARM0_SEC, TIME_NUM, buf);
	if (ret < 0)
		return ret;
	/* only enable alarm on year/month/day/hour/min/sec */
	ret = max8907c_reg_write(info->i2c, MAX8907C_REG_ALARM0_CNTL, 0x77);

	return ret;
}


#if defined(CONFIG_MACH_N1_CHN)

int mode_12_24;
EXPORT_SYMBOL(mode_12_24);
struct max8907c * info_alarm_boot;
EXPORT_SYMBOL(info_alarm_boot);

static int max8907c_rtc_set_alarm_boot(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct max8907c_rtc_info *info = dev_get_drvdata(dev);
	//struct rtc_wkalrm *alm_tmp;
	u8 data[TIME_NUM] ={0,};
	int ret =0;
		
	if(!info) {
		printk("NULL check : info_boot = NULL!!!!!! max8997_rtc_set_alarm_boot\n");
		goto out;
	}
	
	pr_info("%s 3. before read: %02d:%02d:%02d %02d/%02d/%04d\n",
		__func__,
		alrm->time.tm_hour, alrm->time.tm_min, alrm->time.tm_sec,
		alrm->time.tm_mon + 1, alrm->time.tm_mday, alrm->time.tm_year + 1900);
	
	ret = data_calc(data, &alrm->time, TIME_NUM);
	if (ret < 0)
		return ret;

	ret = max8907c_reg_write(info->i2c, MAX8907C_REG_ALARM1_CNTL, 0x00);
	
	ret = max8907c_reg_bulk_write(info->i2c, MAX8907C_REG_ALARM1_SEC, TIME_NUM,data);
	if (ret < 0) {
		dev_err(info->rtc_dev, "%s: fail to write alarm reg(%d)\n",__func__, ret);	
		goto out;
	}
	
	/* only enable alarm on year/month/day/hour/min/sec */
	ret = max8907c_reg_write(info->i2c, MAX8907C_REG_ALARM1_CNTL, 0x77);

	// for debug
	ret = max8907c_reg_bulk_read(info->i2c, MAX8907C_REG_ALARM1_SEC, TIME_NUM,data);
	ret = tm_calc(&alrm->time,data,8);
	pr_info("%s 3. after read: %02d:%02d:%02d %02d/%02d/%04d\n",
		__func__,
		alrm->time.tm_hour, alrm->time.tm_min, alrm->time.tm_sec,
		alrm->time.tm_mon + 1, alrm->time.tm_mday, alrm->time.tm_year + 1900);

out:
	//mutex_unlock(&info->lock);	
	return ret;
}

static void max8907c_rtc_read_alarm_boot(struct platform_device *pdev)
{
	struct max8907c_rtc_info *info = platform_get_drvdata(pdev);
	u8 data[TIME_NUM] ={0,};
	struct rtc_time tm;
	int ret;

	ret = max8907c_reg_bulk_read(info->i2c, MAX8907C_REG_RTC_SEC, TIME_NUM, data);
	if (ret < 0) {
		pr_err("%s: RTC read failed.\n", __func__); 
		goto read_alarm_boot; 
	}
	ret = tm_calc(&tm, data, TIME_NUM);
	if (ret) {
		pr_err("%s: RTC tm_calc failed.\n", __func__); 
		goto read_alarm_boot; 
	}
	pr_info("%s [RTC TIME ] %02d:%02d:%02d %02d/%02d/%04d\n", __func__,
		tm.tm_hour, tm.tm_min, tm.tm_sec,
		tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);
	
read_alarm_boot:
	ret = max8907c_reg_bulk_read(info->i2c, MAX8907C_REG_ALARM1_SEC, TIME_NUM,data);
	if (ret < 0) {
		pr_err("%s: BOOTALARM read failed.\n", __func__); 
		return;
	}
	ret = tm_calc(&tm, data, TIME_NUM);
	if (ret) {
		pr_err("%s: BOOTALARM tm_calc failed.\n", __func__); 
		return; 
	}
	pr_info("%s [BOOTALARM] %02d:%02d:%02d %02d/%02d/%04d\n", __func__,
		tm.tm_hour, tm.tm_min, tm.tm_sec,
		tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);

}
#endif /*--  CHN feature - power_on_alarm_bsystar --*/

static const struct rtc_class_ops max8907c_rtc_ops = {
	.read_time	= max8907c_rtc_read_time,
	.set_time	= max8907c_rtc_set_time,
	.read_alarm	= max8907c_rtc_read_alarm,
#if defined(CONFIG_MACH_N1_CHN)
	.set_alarm_boot = max8907c_rtc_set_alarm_boot,
#endif /*--  CHN feature - power_on_alarm_bsystar --*/
	.set_alarm	= max8907c_rtc_set_alarm,
};

static int __devinit max8907c_rtc_probe(struct platform_device *pdev)
{
	struct max8907c *chip = dev_get_drvdata(pdev->dev.parent);
	struct max8907c_rtc_info *info;
	int irq, irq1, ret;

	info = kzalloc(sizeof(struct max8907c_rtc_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->i2c = chip->i2c_rtc;
	info->chip = chip;

	irq = chip->irq_base + MAX8907C_IRQ_RTC_ALARM0;
#if defined(CONFIG_MACH_N1_CHN)
	irq1= chip->irq_base + MAX8907C_IRQ_RTC_ALARM1;
#endif /*--  CHN feature - power_on_alarm_bsystar --*/

	ret = request_threaded_irq(irq, NULL, rtc_update_handler,
				   IRQF_ONESHOT, "rtc-alarm0", info);
	if (ret < 0) {
		dev_err(chip->dev, "Failed to request IRQ: #%d: %d\n",
			irq, ret);
		goto out_irq;
	}

#if defined(CONFIG_MACH_N1_CHN)
	info_alarm_boot = chip;
#endif /*--  CHN feature - power_on_alarm_bsystar --*/

	info->rtc_dev = rtc_device_register("max8907c-rtc", &pdev->dev,
					&max8907c_rtc_ops, THIS_MODULE);
	ret = PTR_ERR(info->rtc_dev);
	if (IS_ERR(info->rtc_dev)) {
		dev_err(&pdev->dev, "Failed to register RTC device: %d\n", ret);
		goto out_rtc;
	}

#if defined(CONFIG_MACH_N1_CHN)
	ret = request_threaded_irq(irq1, NULL, rtc_update_handler,
				   IRQF_ONESHOT, "rtc-alarm1", info);
	if (ret < 0) {
		dev_err(chip->dev, "Failed to request IRQ: #%d: %d\n",
			irq1, ret);
		goto out_irq;
	}
#endif /*--  CHN feature - power_on_alarm_bsystar --*/

	max8907c_set_bits(chip->i2c_power, MAX8907C_REG_SYSENSEL, 0x2, 0x2);
#if defined(CONFIG_MACH_N1_CHN)
	max8907c_set_bits(chip->i2c_power, MAX8907C_REG_SYSENSEL, 0x4, 0x4);

#endif
	dev_set_drvdata(&pdev->dev, info);

	platform_set_drvdata(pdev, info);

	device_init_wakeup(&pdev->dev, 1);

	max8907c_rtc_read_alarm_boot(pdev); 

	return 0;
out_rtc:
	free_irq(chip->irq_base + MAX8907C_IRQ_RTC_ALARM0, info);
	free_irq(chip->irq_base + MAX8907C_IRQ_RTC_ALARM1, info);

out_irq:
	kfree(info);
	return ret;
}

static int __devexit max8907c_rtc_remove(struct platform_device *pdev)
{
	struct max8907c_rtc_info *info = platform_get_drvdata(pdev);

	if (info) {
		free_irq(info->chip->irq_base + MAX8907C_IRQ_RTC_ALARM0, info);
#if defined(CONFIG_MACH_N1_CHN)
		free_irq(info->chip->irq_base + MAX8907C_IRQ_RTC_ALARM1, info);
#endif /*--  CHN feature - power_on_alarm_bsystar --*/
		rtc_device_unregister(info->rtc_dev);
		kfree(info);
	}
	return 0;
}

#ifdef CONFIG_PM
static int max8907c_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct device *dev=&pdev->dev;
	struct max8907c_rtc_info *info = platform_get_drvdata(pdev);

	if (device_may_wakeup(dev))
		enable_irq_wake(info->chip->irq_base + MAX8907C_IRQ_RTC_ALARM0);
	return 0;
}

static int max8907c_rtc_resume(struct platform_device *pdev)
{
	struct device *dev=&pdev->dev;
	struct max8907c_rtc_info *info = platform_get_drvdata(pdev);

	if (device_may_wakeup(dev))
		disable_irq_wake(info->chip->irq_base + MAX8907C_IRQ_RTC_ALARM0);
	return 0;
}
#endif

static struct platform_driver max8907c_rtc_driver = {
	.driver		= {
		.name	= "max8907c-rtc",
		.owner	= THIS_MODULE,
	},
	.probe		= max8907c_rtc_probe,
	.remove		= __devexit_p(max8907c_rtc_remove),
#ifdef CONFIG_PM
	.suspend	= max8907c_rtc_suspend,
	.resume		= max8907c_rtc_resume,
#endif
};

static int __init max8907c_rtc_init(void)
{
	return platform_driver_register(&max8907c_rtc_driver);
}
module_init(max8907c_rtc_init);

static void __exit max8907c_rtc_exit(void)
{
	platform_driver_unregister(&max8907c_rtc_driver);
}
module_exit(max8907c_rtc_exit);

MODULE_DESCRIPTION("Maxim MAX8907C RTC driver");
MODULE_LICENSE("GPL");

