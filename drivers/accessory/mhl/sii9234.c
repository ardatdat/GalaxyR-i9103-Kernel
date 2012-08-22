/***************************************************************************

*

*   SiI9244 ? MHL Transmitter Driver

*

* Copyright (C) (2011, Silicon Image Inc)

*

* This program is free software; you can redistribute it and/or modify

* it under the terms of the GNU General Public License as published by

* the Free Software Foundation version 2.

*

* This program is distributed ¡°as is¡± WITHOUT ANY WARRANTY of any

* kind, whether express or implied; without even the implied warranty

* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

* GNU General Public License for more details.

*

*****************************************************************************/

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/regulator/consumer.h>
#include <linux/mhl-sii9234.h>
#include <linux/platform_device.h>
#include "Common_Def.h"
#include "sii9234_driver.h"
#include "sii9234_ioctl.h"
#if defined (CONFIG_MACH_BOSE_ATT)
#include <mach/gpio-bose.h>
#else
#include <mach/gpio-n1.h>
#endif
#include<linux/delay.h>

#define SUBJECT "MHL_DRIVER"

#define SII_DEV_DBG(format, ...)\
	pr_err ("[ "SUBJECT " (%s,%d) ] " format "\n", __func__, __LINE__, ## __VA_ARGS__);


struct i2c_driver sii9234_i2c_driver;
struct i2c_client *sii9234_i2c_client;

struct i2c_driver sii9234a_i2c_driver;
struct i2c_client *sii9234a_i2c_client;

struct i2c_driver sii9234b_i2c_driver;
struct i2c_client *sii9234b_i2c_client;

struct i2c_driver sii9234c_i2c_driver;
struct i2c_client *sii9234c_i2c_client;
struct mhl_dev *g_mhl_dev ;

#define SII_ID 0x92
static ssize_t sysfs_check_mhl_command(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int size;
	size = snprintf(buf, 10, "%d\n", 1);

	return size;

}

static CLASS_ATTR(test_result, 0664 , sysfs_check_mhl_command, NULL);

void mhl_hw_reset()
{
	gpio_set_value(GPIO_MHL_RST, 0);
	msleep(200);
	gpio_set_value(GPIO_MHL_RST, 1);
}

static struct i2c_device_id sii9234_id[] = {
	{"SII9234", 0},
	{}
};

static struct i2c_device_id sii9234a_id[] = {
	{"SII9234A", 0},
	{}
};

static struct i2c_device_id sii9234b_id[] = {
	{"SII9234B", 0},
	{}
};

static struct i2c_device_id sii9234c_id[] = {
	{"SII9234C", 0},
	{}
};

int mhl_i2c_init;


struct sii9234_state {
	struct i2c_client *client;
};


irqreturn_t mhl_int_irq_handler(int irq, void *dev_id);
irqreturn_t mhl_wake_up_irq_handler(int irq, void *dev_id);

struct i2c_client *get_sii9234_client(u8 device_id)
{

	struct i2c_client *client_ptr;

	if (device_id == 0x72)
		client_ptr = sii9234_i2c_client;
	else if (device_id == 0x7A)
		client_ptr = sii9234a_i2c_client;
	else if (device_id == 0x92)
		client_ptr = sii9234b_i2c_client;
	else if (device_id == 0xC8)
		client_ptr = sii9234c_i2c_client;
	else
		client_ptr = NULL;

	return client_ptr;
}
EXPORT_SYMBOL(get_sii9234_client);

u8 sii9234_i2c_read(struct i2c_client *client, u8 reg)
{
	u8 ret;

	if (!mhl_i2c_init) {
		SII_DEV_DBG("I2C not ready");
		return 0;
	}

	ret = i2c_smbus_write_byte(client, reg);
	if (ret < 0) {
		printk(KERN_ERR	"%s() I2C cmd write error", __func__);
		return  -EIO;
	}


	ret = i2c_smbus_read_byte(client);

	if (ret < 0) {
		SII_DEV_DBG("i2c read fail");
		return -EIO;
	}

	return ret;

}
EXPORT_SYMBOL(sii9234_i2c_read);


int sii9234_i2c_write(struct i2c_client *client, u8 reg, u8 data)
{
	if (!mhl_i2c_init) {
		SII_DEV_DBG("I2C not ready");
		return 0;
	}

	return i2c_smbus_write_byte_data(client, reg, data);
}
EXPORT_SYMBOL(sii9234_i2c_write);


void sii9234_interrupt_event_work()
{

	pr_err("sii9234_interrupt_event_work() is called\n");

	SiI9234_interrupt_event();

}


irqreturn_t mhl_int_irq_handler(int irq, void *dev_id)
{
	struct mhl_dev *mhl_dev = dev_id;

	pr_err("mhl_int_irq_handler() is called\n");

	queue_work(mhl_dev->sii9234_wq, &mhl_dev->sii9234_int_work);

	return IRQ_HANDLED;
}


irqreturn_t mhl_wake_up_irq_handler(int irq, void *dev_id)
{
	struct mhl_dev *mhl_dev = dev_id;

	pr_err("mhl_wake_up_irq_handler() is called\n");

	queue_work(mhl_dev->sii9234_wq, &mhl_dev->sii9234_int_work);

	return IRQ_HANDLED;
}

static int sii9234_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	/* int retval; */

	struct sii9234_state *state;

	struct device *mhl_dev;

	state = kzalloc(sizeof(struct sii9234_state), GFP_KERNEL);
	if (state == NULL) {
		pr_err("failed to allocate memory \n");
		return -ENOMEM;
	}

	state->client = client;
	i2c_set_clientdata(client, state);



	/* rest of the initialisation goes here. */

	pr_err("SII9234 attach success!!!\n");

	sii9234_i2c_client = client;

	mhl_i2c_init = 1;

	return 0;

}



static int __devexit sii9234_i2c_remove(struct i2c_client *client)
{
	struct sii9234_state *state = i2c_get_clientdata(client);
	kfree(state);

	return 0;
}

static int sii9234a_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct sii9234_state *state;

	state = kzalloc(sizeof(struct sii9234_state), GFP_KERNEL);
	if (state == NULL) {
		pr_err("failed to allocate memory \n");
		return -ENOMEM;
	}

	state->client = client;
	i2c_set_clientdata(client, state);

	/* rest of the initialisation goes here. */

	pr_err("SII9234A attach success!!!\n");

	sii9234a_i2c_client = client;

	return 0;

}



static int __devexit sii9234a_i2c_remove(struct i2c_client *client)
{
	struct sii9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}

static int sii9234b_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct sii9234_state *state;

	state = kzalloc(sizeof(struct sii9234_state), GFP_KERNEL);
	if (state == NULL) {
		pr_err("failed to allocate memory \n");
		return -ENOMEM;
	}

	state->client = client;
	i2c_set_clientdata(client, state);

	/* rest of the initialisation goes here. */

	pr_err("SII9234B attach success!!!\n");

	sii9234b_i2c_client = client;


	return 0;

}



static int __devexit sii9234b_i2c_remove(struct i2c_client *client)
{
	struct sii9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}


static int sii9234c_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct sii9234_state *state;

	state = kzalloc(sizeof(struct sii9234_state), GFP_KERNEL);
	if (state == NULL) {
		pr_err("failed to allocate memory \n");
		return -ENOMEM;
	}

	state->client = client;
	i2c_set_clientdata(client, state);

	/* rest of the initialisation goes here. */

	pr_err("SII9234C attach success!!!\n");

	sii9234c_i2c_client = client;

	return 0;

}



static int __devexit sii9234c_i2c_remove(struct i2c_client *client)
{
	struct sii9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}

struct i2c_driver sii9234_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234",
	},
	.id_table	= sii9234_id,
	.probe	= sii9234_i2c_probe,
	.remove	= __devexit_p(sii9234_i2c_remove),
	.command = NULL,
};

struct i2c_driver sii9234a_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234A",
	},
	.id_table	= sii9234a_id,
	.probe	= sii9234a_i2c_probe,
	.remove	= __devexit_p(sii9234a_i2c_remove),
	.command = NULL,
};

struct i2c_driver sii9234b_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234B",
	},
	.id_table	= sii9234b_id,
	.probe	= sii9234b_i2c_probe,
	.remove	= __devexit_p(sii9234b_i2c_remove),
	.command = NULL,
};

struct i2c_driver sii9234c_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234C",
	},
	.id_table	= sii9234c_id,
	.probe	= sii9234c_i2c_probe,
	.remove	= __devexit_p(sii9234c_i2c_remove),
	.command = NULL,
};

int already_get;
int reg_en;


static int mhl_open(struct inode *ip, struct file *fp)
{
	pr_err("[%s]\n", __func__);
	return 0;

}

static int mhl_release(struct inode *ip, struct file *fp)
{

	pr_err("[%s]\n", __func__);
	return 0;
}


static long mhl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct mhl_dev *mhl_dev = container_of(file->private_data,
						struct mhl_dev,
						mdev);
	unsigned int mhl_irq;
	unsigned int mhl_wakeup_irq;

	pr_err("[%s]\n", __func__);

	switch (cmd) {
#ifdef CONFIG_MHL_SWITCH
	case MHL_SWITCH_ON:
		mhl_dev->pdata->switch_onoff(1);
		break;
	case MHL_SWITCH_OFF:
		mhl_dev->pdata->switch_onoff(0);
		break;
#endif
	case MHL_HW_RESET:
		mhl_dev->pdata->power_onoff(1);
		break;
	case MHL_INIT:
		mhl_irq = gpio_to_irq(mhl_dev->irq_gpio);
		mhl_wakeup_irq = gpio_to_irq(mhl_dev->wake_up_gpio);
		enable_irq(mhl_irq);
		enable_irq(mhl_wakeup_irq);
		break;
	default:
		pr_err("MHL ioctl cmd error\n");
	}
	return 0;
}

static struct file_operations mhl_fops = {
	.owner  = THIS_MODULE,
	.open   = mhl_open,
	.release = mhl_release,
	.unlocked_ioctl = mhl_ioctl,
};


void Sii9234_int_irq_enable()
{
	printk(KERN_INFO	"%s()\n", __func__);
	if (likely(g_mhl_dev != NULL))
		if (likely(g_mhl_dev->pdata != NULL)) {
			enable_irq(g_mhl_dev->pdata->mhl_irq);
			return;
		}
	printk(KERN_ERR	"[ERROR] %s() resource is NULL\n", __func__);
}
EXPORT_SYMBOL(Sii9234_int_irq_enable);


void Sii9234_int_irq_disable(void)
{
	printk(KERN_INFO	"%s()\n", __func__);
	if (likely(g_mhl_dev != NULL))
		if (likely(g_mhl_dev->pdata != NULL)) {
			disable_irq(g_mhl_dev->pdata->mhl_irq);
			return;
		}
	printk(KERN_ERR	"[ERROR] %s() resource is NULL\n", __func__);
}
EXPORT_SYMBOL(Sii9234_int_irq_disable);

void rcp_uevent_report(u8 key)
{
	if (!g_mhl_dev->input) {
		pr_err("sii9234: %s sii9234 input dev is NULL\n", __func__);
		return;
	}

	pr_info("sii9234: %s key: %d\n", __func__, key);
	input_report_key(g_mhl_dev->input, (unsigned int)key + 1, 1);
	input_report_key(g_mhl_dev->input, (unsigned int)key + 1, 0);
	input_sync(g_mhl_dev->input);
}
EXPORT_SYMBOL(rcp_uevent_report);

static int sii9234_probe(struct platform_device *pdev)
{
	int ret;
	struct mhl_platform_data *mhl_pdata = pdev->dev.platform_data;
	struct mhl_dev *mhl_dev;
	unsigned int mhl_wakeup_irq;
	struct class *sec_mhl;
	struct input_dev *input;

	if (mhl_pdata == NULL) {
		pr_err("MHL probe fail\n");
		return -ENODEV;
	}

	ret = i2c_add_driver(&sii9234_i2c_driver);
	if (ret != 0) {
		pr_err("[MHL SII9234] can't add i2c driver\n");
		return ret;
	} else {
		pr_err("[MHL SII9234] add i2c driver\n");
	}

	ret = i2c_add_driver(&sii9234a_i2c_driver);
	if (ret != 0) {
		pr_err("[MHL SII9234A] can't add i2c driver\n");
		goto err_i2c_a_add;
	} else {
		pr_err("[MHL SII9234A] add i2c driver\n");
	}

	ret = i2c_add_driver(&sii9234b_i2c_driver);
	if (ret != 0) {
		pr_err("[MHL SII9234B] can't add i2c driver\n");
		goto err_i2c_b_add;
	} else {
		pr_err("[MHL SII9234B] add i2c driver\n");
	}

	ret = i2c_add_driver(&sii9234c_i2c_driver);
	if (ret != 0) {
		pr_err("[MHL SII9234C] can't add i2c driver\n");
		goto err_i2c_c_add;
	} else {
		pr_err("[MHL SII9234C] add i2c driver\n");
	}

	mhl_dev = kzalloc(sizeof(struct mhl_dev), GFP_KERNEL);
	if (!mhl_dev) {
		ret = -ENOMEM;
		goto err_mem;
	}

	input = input_allocate_device();
	if (!input) {
		pr_err("failed to allocate input device.\n");
		ret = -ENOMEM;
		goto err_input;
	}

	set_bit(EV_KEY, input->evbit);
	bitmap_fill(input->keybit, KEY_MAX);
	mhl_dev->input = input;

	input_set_drvdata(input, mhl_dev);
	input->name = "sii9234_rcp";
	input->id.bustype = BUS_I2C;

	ret = input_register_device(input);
	if (ret < 0) {
		pr_err("fail to register input device\n");
		goto err_misc_register;
	}

	mhl_dev->pdata = mhl_pdata;
	mhl_dev->irq_gpio = mhl_pdata->mhl_int;
	mhl_dev->wake_up_gpio = mhl_pdata->mhl_wake_up;

	INIT_WORK(&mhl_dev->sii9234_int_work, sii9234_interrupt_event_work);
	mhl_dev->sii9234_wq = create_singlethread_workqueue("sii9234_wq");

	mhl_dev->mdev.minor = MISC_DYNAMIC_MINOR;
	mhl_dev->mdev.name = "mhl";
	mhl_dev->mdev.fops = &mhl_fops;

	dev_set_drvdata(&pdev->dev, mhl_dev);

	mhl_dev->process_dev = &pdev->dev;

	ret = misc_register(&mhl_dev->mdev);
	if (ret) {
		pr_err("mhl misc_register failed\n");
		goto err_misc_register;
	}
      g_mhl_dev = mhl_dev;

	mhl_pdata->mhl_irq = gpio_to_irq(mhl_dev->irq_gpio);
	irq_set_irq_type(mhl_pdata->mhl_irq, IRQ_TYPE_EDGE_RISING);
	ret = request_threaded_irq(mhl_pdata->mhl_irq, NULL, mhl_int_irq_handler,
			IRQF_DISABLED, "mhl_int", mhl_dev);
	if (ret) {
		pr_err("unable to request irq mhl_int err:: %d\n", ret);
		goto err_irq_request;
	}

	Sii9234_int_irq_disable();

	pr_info("create mhl sysfile\n");

	sec_mhl = class_create(THIS_MODULE, "mhl");
	if (IS_ERR(sec_mhl)) {
		pr_err("Failed to create class(sec_mhl)!\n");
		goto err_exit;
	}

	ret = class_create_file(sec_mhl, &class_attr_test_result);
	if (ret) {
		pr_err("[ERROR] Failed to create device file in sysfs entries!\n");
		goto err_exit;
	}

#if 0
	mhl_wakeup_irq = gpio_to_irq(mhl_dev->wake_up_gpio);
	set_irq_type(mhl_wakeup_irq, IRQ_TYPE_EDGE_RISING);
	ret = request_threaded_irq(mhl_wakeup_irq, NULL,
				mhl_wake_up_irq_handler,
				IRQF_DISABLED,
				"mhl_wake_up",
				mhl_dev);
	if (ret) {
		pr_err("unable to request irq mhl_wake_up err:: %d\n", ret);
		goto err_wake_up_irq_request;
	}
#endif

#if 0
	disable_irq_nosync(mhl_irq);
	disable_irq_nosync(mhl_wakeup_irq);
#endif
	return 0;
err_exit:
	class_destroy(sec_mhl);
err_wake_up_irq_request:
	free_irq(gpio_to_irq(mhl_dev->irq_gpio), mhl_dev);
err_irq_request:
	mhl_pdata->mhl_irq = NULL;
	misc_deregister(&mhl_dev->mdev);
err_misc_register:
	input_free_device(input);
err_input:
	destroy_workqueue(mhl_dev->sii9234_wq);
	kfree(mhl_dev);
err_mem:
	i2c_del_driver(&sii9234c_i2c_driver);
err_i2c_c_add:
	i2c_del_driver(&sii9234b_i2c_driver);
err_i2c_b_add:
	i2c_del_driver(&sii9234a_i2c_driver);
err_i2c_a_add:
	i2c_del_driver(&sii9234_i2c_driver);

	return ret;
}

static int sii9234_remove(struct platform_device *pdev)
{
	struct mhl_dev *mhl_dev = platform_get_drvdata(pdev);

	SII_DEV_DBG("");

	disable_irq_nosync(gpio_to_irq(mhl_dev->irq_gpio));
	disable_irq_nosync(gpio_to_irq(mhl_dev->wake_up_gpio));

	i2c_del_driver(&sii9234_i2c_driver);
	i2c_del_driver(&sii9234a_i2c_driver);
	i2c_del_driver(&sii9234b_i2c_driver);
	i2c_del_driver(&sii9234c_i2c_driver);

	destroy_workqueue(mhl_dev->sii9234_wq);

	kfree(mhl_dev);
	return 0;
}

static int sii9234_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct mhl_platform_data *mhl_pdata = pdev->dev.platform_data;
	int ret;

	SII_DEV_DBG("");

#ifdef CONFIG_MHL_SWITCH
	ret = gpio_get_value(mhl_pdata->mhl_sel);
	if (ret) {
#endif
		mhl_pdata->power_onoff(0);
		disable_irq_nosync(gpio_to_irq(mhl_pdata->mhl_int));
		disable_irq_nosync(gpio_to_irq(mhl_pdata->mhl_wake_up));
#ifdef CONFIG_MHL_SWITCH
	}
#endif
	return 0;
}

static int sii9234_resume(struct platform_device *pdev)
{
	struct mhl_platform_data *mhl_pdata = pdev->dev.platform_data;
	int ret;

	SII_DEV_DBG("");

#ifdef CONFIG_MHL_SWITCH
	ret = gpio_get_value(mhl_pdata->mhl_sel);
	if (ret) {
#endif
		mhl_pdata->power_onoff(1);
		enable_irq(gpio_to_irq(mhl_pdata->mhl_int));
		enable_irq(gpio_to_irq(mhl_pdata->mhl_wake_up));
#ifdef CONFIG_MHL_SWITCH
	}
#endif
	return 0;
}

static struct platform_driver mhl_driver = {
	.probe		= sii9234_probe,
	.remove		= sii9234_remove,
	.suspend	= sii9234_suspend,
	.resume		= sii9234_resume,
	.driver		= {
		.name	= "mhl",
		.owner	= THIS_MODULE,
	},
};

static int __init sii9234_module_init(void)
{
	int ret;

	ret = platform_driver_register(&mhl_driver);

	return ret;
}
module_init(sii9234_module_init);
static void __exit sii9234_exit(void)
{
	i2c_del_driver(&sii9234_i2c_driver);
	i2c_del_driver(&sii9234a_i2c_driver);
	i2c_del_driver(&sii9234b_i2c_driver);
	i2c_del_driver(&sii9234c_i2c_driver);
	platform_driver_unregister(&mhl_driver);
};
module_exit(sii9234_exit);

MODULE_DESCRIPTION("Sii9234 MHL driver");
MODULE_AUTHOR("Aakash Manik");
MODULE_LICENSE("GPL");
