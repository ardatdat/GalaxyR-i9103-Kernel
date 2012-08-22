/*
 * Driver for keys on GPIO lines capable of generating interrupts.
 *
 * Copyright 2005 Phil Blundell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
//#include <mach/regs-gpio.h>
//#include <plat/gpio-cfg.h>
#include <asm/gpio.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/earlysuspend.h>
#include <asm/io.h>
#include <mach/gpio-bose.h>

#ifdef CONFIG_CPU_FREQ
/* #include <mach/cpu-freq-v210.h> */
#endif

#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include "n1-cypress-gpio.h"

#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
//#include <plat/gpio-cfg.h>

/*
Melfas touchkey register
*/
#define KEYCODE_REG 0x00
#define FIRMWARE_VERSION 0x01
#define TOUCHKEY_MODULE_VERSION 0x02
#define TOUCHKEY_ADDRESS	0x20

#define UPDOWN_EVENT_BIT 0x08
#define KEYCODE_BIT 0x07
#define ESD_STATE_BIT 0x10

#define I2C_M_WR 0		/* for i2c */

#define DEVICE_NAME "sec_touchkey"
#define TOUCH_FIRMWARE_V04  0x04
#define TOUCH_FIRMWARE_V06  0x06
#define TOUCH_FIRMWARE_V07  0x07
#define TOUCH_FIRMWARE_V09  0x09
#define TOUCH_FIRMWARE_V0A  0x0A



#define DOOSUNGTECH_TOUCH_V1_2  0x0C

static int touchkey_keycode[5] = { 0, KEY_MENU, KEY_HOME, KEY_BACK, KEY_SEARCH };



static u8 menu_sensitivity = 0;
static u8 home_sensitivity = 0;
static u8 back_sensitivity = 0;
static u8 search_sensitivity = 0;
static int touchkey_enable = 0;

static u16 raw_data0 = 0;
static u16 raw_data1 = 0;
static u16 raw_data2 = 0;
static u16 raw_data3 = 0;
static u8 idac0 = 0;
static u8 idac1 = 0;
static u8 idac2 = 0;
static u8 idac3 = 0;
static u8 touchkey_threshold = 0;

int touchkey_autocalibration(void);
/*sec_class sysfs*/
extern struct class *sec_class;
struct device *sec_touchkey;


struct i2c_touchkey_driver {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct early_suspend early_suspend;
};

struct i2c_touchkey_driver *touchkey_driver = NULL;
struct work_struct touchkey_work;
struct workqueue_struct *touchkey_wq;

struct work_struct touch_update_work;
struct delayed_work touch_resume_work;

#ifdef WHY_DO_WE_NEED_THIS
static void __iomem *gpio_pend_mask_mem;
#define		INT_PEND_BASE	0xE0200A54
#endif

static const struct i2c_device_id melfas_touchkey_id[] = {
	{"melfas_touchkey", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, melfas_touchkey_id);

static void init_hw(void);
static int i2c_touchkey_probe(struct i2c_client *client,
			      const struct i2c_device_id *id);

extern int get_touchkey_firmware(char *version);
static int touchkey_led_status = 0;
static int touchled_cmd_reversed=0;

struct i2c_driver touchkey_i2c_driver = {
	.driver = {
		   .name = "melfas_touchkey_driver",
		   },
	.id_table = melfas_touchkey_id,
	.probe = i2c_touchkey_probe,
};

static int touchkey_debug_count = 0;
static char touchkey_debug[104];
static int touch_version = 0;
static int module_version = 0;
extern int touch_is_pressed;
static bool bose_touchkey_ldo;

extern int ISSP_main(void);
static int touchkey_update_status;

static void touch_forced_release(void)
{
}
int touchkey_led_ldo_on(bool on)
{
}


int touchkey_ldo_on(bool on)
{
	struct regulator *regulator;
	bool error = 0;

	regulator = regulator_get(NULL, "T_KEY_3V0");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR"[TouchKey] %s regulatr_get fail\n", __func__);
		bose_touchkey_ldo = on;
		return 0;
	}

	if (on) {
		if (bose_touchkey_ldo == 1)
			error = 1;
		else
			regulator_enable(regulator);
	} else {
		if (bose_touchkey_ldo == 0)
			error = 1;
		else
			if (regulator_is_enabled(regulator))
				regulator_disable(regulator);
	}
	bose_touchkey_ldo = on;
	regulator_put(regulator);

	if (error) {
		printk(KERN_ERR"[TouchKey] touchkey already %s\n",
			(on ? "on" : "off"));
		return 0;
	} else {
		printk(KERN_DEBUG"[TouchKey] %s excute(%d)\n",
			__func__, bose_touchkey_ldo);
		return 1;
	}
}

static void set_touchkey_debug(char value)
{
	if (touchkey_debug_count == 100)
		touchkey_debug_count = 0;

	touchkey_debug[touchkey_debug_count] = value;
	touchkey_debug_count++;
}

static int i2c_touchkey_read(u8 reg, u8 * val, unsigned int len)
{
	int err = 0;
	int retry = 10;
	struct i2c_msg msg[1];

	if ((touchkey_driver == NULL)) {
		printk(KERN_ERR "[TouchKey] touchkey is not enabled.R\n");
		return -ENODEV;
	}

	while (retry--) {
		msg->addr = touchkey_driver->client->addr;
		msg->flags = I2C_M_RD;
		msg->len = len;
		msg->buf = val;
		err = i2c_transfer(touchkey_driver->client->adapter, msg, 1);

		if (err >= 0) {
			return 0;
		}
		printk(KERN_ERR "[TouchKey] %s %d i2c transfer error\n", __func__, __LINE__);	/* add by inter.park */
		mdelay(10);
	}
	return err;

}

#if 1
static int i2c_touchkey_write(u8 *val, unsigned int len)
{
	int err = 0;
	struct i2c_msg msg[1];
	int retry = 2;

	if ((touchkey_driver == NULL) || !(touchkey_enable == 1)) {
		return -ENODEV;
	}

	while (retry--) {
		msg->addr = touchkey_driver->client->addr;
		msg->flags = I2C_M_WR;
		msg->len = len;
		msg->buf = val;
		err = i2c_transfer(touchkey_driver->client->adapter, msg, 1);
		/*printk("write value %d to address %d\n",*val, msg->addr);*/
		if (err >= 0) {

			return 0;

		}
		printk(KERN_DEBUG "[TouchKey] %s %d i2c transfer error\n",
		       __func__, __LINE__);
		mdelay(10);
	}
	return err;
}
#else
static int i2c_touchkey_write(u8 * val, unsigned int len)
{
	int err = 0;
	struct i2c_msg msg[1];
	unsigned char data[2];
	int retry = 2;

	if ((touchkey_driver == NULL) || !(touchkey_enable == 1)) {
		//printk(KERN_ERR "[TouchKey] touchkey is not enabled.\n");
		return -ENODEV;
	}

	while (retry--) {
		data[0] = *val;
		msg->addr = touchkey_driver->client->addr;
		msg->flags = I2C_M_WR;
		msg->len = len;
		msg->buf = data;
		err = i2c_transfer(touchkey_driver->client->adapter, msg, 1);
		//printk("write value %d to address %d\n",*val, msg->addr);
		if (err >= 0) {
			return 0;
		}
		printk(KERN_DEBUG "[TouchKey] %s %d i2c transfer error\n", __func__, __LINE__);
		mdelay(10);
	}
	return err;
}
#endif

#if 1
int touchkey_autocalibration(void)
{
	u8 data[4];
	int count = 0;

	data[0] = 0x50;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x01;

	printk(KERN_DEBUG "[TouchKey] touchkey_autocalibration");

	count = i2c_touchkey_write(data, 4);

	return count;
}

static ssize_t set_touchkey_autocal_testmode(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t size)
{
	int count = 0;
	u8 set_data;
	int on_off;

	if (sscanf(buf, "%d\n", &on_off) == 1) {
		printk(KERN_ERR "[TouchKey] Test Mode : %d \n", on_off);

		if (on_off == 1) {
			set_data = 0x40;
			count = i2c_touchkey_write(&set_data, 1);
		} else {
			touchkey_ldo_on(0);
			msleep(50);
			touchkey_ldo_on(1);
			msleep(50);
			init_hw();
			msleep(50);
			touchkey_autocalibration();
		}
	} else {
		printk(KERN_ERR "[TouchKey] set_touchkey_autocal_testmode Error\n");
	}

	return count;
}

static ssize_t touchkey_raw_data0_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[26];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 26);

	printk(KERN_DEBUG "called %d,%d,%d,%d,thr-%d,%d,\nidac %d,%d,%d,%d,\n"
		"raw (%d,%d),(%d,%d),(%d,%d),(%d,%d),\n%d,%d,%d,%d,%d,%d,%d,%d,\n",
		data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],
		data[9],data[10],data[11],data[12],data[13],data[14],data[15],data[16],
		data[17],data[18],data[19],data[20],data[21],data[22],data[23],data[24],data[25]);

	raw_data0 = ((0x00FF&data[18])<<8)|data[19];
	return sprintf(buf, "%d\n", raw_data0);
}

static ssize_t touchkey_raw_data1_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[26];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 26);
	raw_data1 = ((0x00FF&data[20])<<8)|data[21];
	return sprintf(buf, "%d\n", raw_data1);
}

static ssize_t touchkey_raw_data2_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[26];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 26);
	raw_data2 = ((0x00FF&data[22])<<8)|data[23];
	return sprintf(buf, "%d\n", raw_data2);
}

static ssize_t touchkey_raw_data3_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[26];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 26);
	raw_data3 = ((0x00FF&data[24])<<8)|data[25];
	return sprintf(buf, "%d\n", raw_data3);
}

static ssize_t touchkey_idac0_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	idac0 = data[6];
	return sprintf(buf, "%d\n", idac0);
}

static ssize_t touchkey_idac1_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	idac1 = data[7];
	return sprintf(buf, "%d\n", idac1);
}

static ssize_t touchkey_idac2_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	idac2 = data[8];
	return sprintf(buf, "%d\n", idac2);
}

static ssize_t touchkey_idac3_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	idac3 = data[9];
	return sprintf(buf, "%d\n", idac3);
}

static ssize_t touchkey_threshold_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	touchkey_threshold = data[4];
	return sprintf(buf, "%d\n", touchkey_threshold);
}

void touchkey_firmware_update(void)
{
	char data[3];
	int retry;

	i2c_touchkey_read(KEYCODE_REG, data, 3);
	printk(KERN_ERR"%s F/W version: 0x%x, Module version:0x%x\n", __FUNCTION__, data[1], data[2]);
	retry = 3;

	touch_version = data[1];
	module_version = data[2];

	if (touch_version != 0x09) {
		touchkey_update_status = 1;
		while (retry--) {
			if (ISSP_main() == 0) {
				printk(KERN_ERR"[TOUCHKEY]Touchkey_update succeeded\n");
				touchkey_update_status = 0;
				break;
			}
			printk(KERN_ERR"touchkey_update failed... retry...\n");
	   }
		if (retry <= 0) {
			touchkey_ldo_on(0);
			touchkey_update_status = -1;
			msleep(300);
		}

		init_hw();
	} else {
		 if (touch_version == 0x09) {
			printk(KERN_ERR "[TouchKey] Not F/W update. Cypess touch-key F/W version is latest. \n");
		} else {
			printk(KERN_ERR "[TouchKey] Not F/W update. Cypess touch-key version(module or F/W) is not valid. \n");
		}
	}
}

#endif

extern void TSP_forced_release(void);

/* for debug log. use debug level */
extern int checkTSPKEYdebuglevel;
#define KERNEL_SEC_DEBUG_LEVEL_LOW	(0x574F4C44)
#define KERNEL_SEC_DEBUG_LEVEL_MID	(0x44494D44)
#define KERNEL_SEC_DEBUG_LEVEL_HIGH	(0x47494844)

void touchkey_work_func(struct work_struct *p)
{
	u8 data[10];
	int ret;
	int retry = 10;

	printk(KERN_DEBUG "[TouchKey] %s\n", __func__);

	set_touchkey_debug('a');
	if (!gpio_get_value(_3_GPIO_TOUCH_INT)) {
#ifdef CONFIG_CPU_FREQ
		/* set_dvfs_target_level(LEV_800MHZ); */
#endif
		ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
		menu_sensitivity = data[7];
		back_sensitivity = data[9];
		/******************************************************************
		typedef struct I2CReg
		{
			unsigned char	 BtnStatus;							 		// 0 :
			unsigned char	 Version;								  	// 1 :FW Version
			unsigned char	 PcbStatus;							 		// 2 :Module Version
			unsigned char	 Cmd;									  	// 3 :
			unsigned char	 Chip_id;								  	// 4 :0x55(DEFAULT_CHIP_ID) 0
			unsigned char	 Sens;									   	// 5 :sensitivity grade(0x00(slow),0x01(mid),0x02(fast))
			WORD			 DiffData[CSD_TotalSensorCount];   			//  6, 7 - 8, 9
			WORD			 RawData[CSD_TotalSensorCount];  			// 10,11 - 12,13
			WORD			 Baseline[CSD_TotalSensorCount];   			// 14,15 - 16,17
		}I2CReg;
		******************************************************************/
		set_touchkey_debug(data[0]);
		if ((data[0] & ESD_STATE_BIT) || (ret != 0))
        {
			printk(KERN_DEBUG "[TouchKey] ESD_STATE_BIT set or I2C fail: data: %d, retry: %d\n", data[0], retry);

			/* releae key */
			input_report_key(touchkey_driver->input_dev, touchkey_keycode[1], 0);
			input_report_key(touchkey_driver->input_dev, touchkey_keycode[2], 0);
			input_report_key(touchkey_driver->input_dev, touchkey_keycode[3], 0);
			input_report_key(touchkey_driver->input_dev, touchkey_keycode[4], 0);

			retry = 10;
			while (retry--)
            {
				//gpio_direction_output(_3_GPIO_TOUCH_EN, 0);
				mdelay(300);
				init_hw();

				if (i2c_touchkey_read(KEYCODE_REG, data, 3) >= 0)
                {
					printk(KERN_DEBUG "[TouchKey] %s touchkey init success\n",  __func__);
					set_touchkey_debug('O');
					enable_irq(IRQ_TOUCH_INT);
					return;
				}
				printk(KERN_ERR	"[TouchKey] %s %d i2c transfer error retry = %d\n", __func__, __LINE__, retry);
			}

			/* touchkey die , do not enable touchkey   			enable_irq(IRQ_TOUCH_INT); */
			touchkey_enable = -1;
			//gpio_direction_output(_3_GPIO_TOUCH_EN, 0);
			gpio_direction_output(_3_TOUCH_SDA_28V, 0);
			gpio_direction_output(_3_TOUCH_SCL_28V, 0);
			printk(KERN_DEBUG "[TouchKey] %s touchkey died\n", __func__);
			set_touchkey_debug('D');
			return;
		}

		if (data[0] & UPDOWN_EVENT_BIT) {
			input_report_key(touchkey_driver->input_dev,touchkey_keycode[data[0] & KEYCODE_BIT], 0);
			input_sync(touchkey_driver->input_dev);

		if ((checkTSPKEYdebuglevel == KERNEL_SEC_DEBUG_LEVEL_MID) ||
			(checkTSPKEYdebuglevel == KERNEL_SEC_DEBUG_LEVEL_HIGH)) {
			printk(KERN_DEBUG "[TouchKey] release keycode:%d \n",touchkey_keycode[data[0] & KEYCODE_BIT]);
		}
			if(touchkey_keycode[data[0] & KEYCODE_BIT] == touchkey_keycode[1])
				printk("menu key sensitivity = %d\n", menu_sensitivity);
			if(touchkey_keycode[data[0] & KEYCODE_BIT] == touchkey_keycode[2])
				printk("home key sensitivity = %d\n",back_sensitivity);
            #if 0
			if(touchkey_keycode[data[0] & KEYCODE_BIT] == touchkey_keycode[3])
				printk("back key sensitivity = %d\n",back_sensitivity);
			if(touchkey_keycode[data[0] & KEYCODE_BIT] == touchkey_keycode[4])
				printk("back key sensitivity = %d\n",back_sensitivity);
            #endif
		} else {
			if (touch_is_pressed) {
				printk(KERN_DEBUG "[TouchKey] touchkey pressed but don't send event because touch is pressed. \n");
				set_touchkey_debug('P');
			} else {
				if ((data[0] & KEYCODE_BIT) == 2) {
					/* if back key is pressed, release multitouch */
					printk(KERN_DEBUG "[TouchKey] touchkey release tsp input. \n");
					touch_forced_release();
				}

				input_report_key(touchkey_driver->input_dev,touchkey_keycode[data[0]&KEYCODE_BIT],	 1);
				input_sync(touchkey_driver->input_dev);

				if ((checkTSPKEYdebuglevel == KERNEL_SEC_DEBUG_LEVEL_MID) ||
					(checkTSPKEYdebuglevel == KERNEL_SEC_DEBUG_LEVEL_HIGH)) {
					printk(KERN_DEBUG "[TouchKey] press keycode:%d \n",touchkey_keycode[data[0] & KEYCODE_BIT]);
				}
				if(touchkey_keycode[data[0] & KEYCODE_BIT] == touchkey_keycode[1])
					printk("menu key sensitivity = %d\n",menu_sensitivity);
				if(touchkey_keycode[data[0] & KEYCODE_BIT] == touchkey_keycode[2])
					printk("back key sensitivity = %d\n",back_sensitivity);
	            #if 0
				if(touchkey_keycode[data[0] & KEYCODE_BIT] == touchkey_keycode[3])
					printk("back key sensitivity = %d\n",back_sensitivity);
				if(touchkey_keycode[data[0] & KEYCODE_BIT] == touchkey_keycode[4])
					printk("back key sensitivity = %d\n",back_sensitivity);
	            #endif
			}
		}
		if ((checkTSPKEYdebuglevel == KERNEL_SEC_DEBUG_LEVEL_MID) ||
			(checkTSPKEYdebuglevel == KERNEL_SEC_DEBUG_LEVEL_HIGH)) {
			printk(KERN_ERR"## TOUCH KEY - CODE : %d\n ", touchkey_keycode[data[0] & KEYCODE_BIT]);
		}
	}

#ifdef WHY_DO_WE_NEED_THIS
	/* clear interrupt */
	if (readl(gpio_pend_mask_mem) & (0x1 << 1)) {
		writel(readl(gpio_pend_mask_mem) | (0x1 << 1),
		       gpio_pend_mask_mem);
	}
#endif
	set_touchkey_debug('A');
	enable_irq(IRQ_TOUCH_INT);
}

static irqreturn_t touchkey_interrupt(int irq, void *dummy)
{
    //printk("Interrupt occur!!!\n");
	set_touchkey_debug('I');
	disable_irq_nosync(IRQ_TOUCH_INT);
	queue_work(touchkey_wq, &touchkey_work);

	return IRQ_HANDLED;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int melfas_touchkey_early_suspend(struct early_suspend *h)
{
	int i;

	touchkey_enable = 0;
	set_touchkey_debug('S');
	printk(KERN_DEBUG "[TouchKey] melfas_touchkey_early_suspend\n");
	if (touchkey_enable < 0) {
		printk(KERN_DEBUG "[TouchKey] ---%s---touchkey_enable: %d\n",       __func__
, touchkey_enable);
		return 0;
	}

	disable_irq(IRQ_TOUCH_INT);

	/* touchkey force release */
	for (i = 1; i < 5; i++)
		input_report_key(touchkey_driver->input_dev,
			touchkey_keycode[i], 0);
	input_sync(touchkey_driver->input_dev);

	touchkey_ldo_on(0);


	return 0;
}

static int melfas_touchkey_late_resume(struct early_suspend *h)
{
	set_touchkey_debug('R');
	printk(KERN_DEBUG "[TouchKey] melfas_touchkey_late_resume\n");


	/* enable ldo20 */
	touchkey_ldo_on(1);

	msleep(100);

#ifdef WHY_DO_WE_NEED_THIS
	/* clear interrupt */
	if (readl(gpio_pend_mask_mem) & (0x1 << 1)) {
		writel(readl(gpio_pend_mask_mem) | (0x1 << 1),
		       gpio_pend_mask_mem);
	}
#endif

	touchkey_enable = 1;
	enable_irq(IRQ_TOUCH_INT);

	if(touchled_cmd_reversed) {
		touchled_cmd_reversed = 0;
		i2c_touchkey_write(&touchkey_led_status, 1);
		printk("LED returned on..\n");
	}

#if 1
		msleep(100);
		touchkey_autocalibration();
#endif

#ifdef TEST_JIG_MODE
	i2c_touchkey_write(&get_touch, 1);
#endif

	return 0;
}
#endif

extern int mcsdl_download_binary_data(void);
static int i2c_touchkey_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char data;
	//struct regulator *regulator;

	printk(KERN_DEBUG "[TouchKey] melfas i2c_touchkey_probe\n");

	touchkey_driver =
	    kzalloc(sizeof(struct i2c_touchkey_driver), GFP_KERNEL);
	if (touchkey_driver == NULL) {
		dev_err(dev, "failed to create our state\n");
		return -ENOMEM;
	}

	touchkey_driver->client = client;
	touchkey_driver->client->irq = IRQ_TOUCH_INT;
	strlcpy(touchkey_driver->client->name, "melfas-touchkey",
		I2C_NAME_SIZE);

	input_dev = input_allocate_device();

	if (!input_dev) {
		return -ENOMEM;
	}

	touchkey_driver->input_dev = input_dev;

	input_dev->name = DEVICE_NAME;
	input_dev->phys = "melfas-touchkey/input0";
	input_dev->id.bustype = BUS_HOST;

	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(touchkey_keycode[1], input_dev->keybit);
	set_bit(touchkey_keycode[2], input_dev->keybit);
	set_bit(touchkey_keycode[3], input_dev->keybit);
	set_bit(touchkey_keycode[4], input_dev->keybit);

	err = input_register_device(input_dev);
	if (err) {
		input_free_device(input_dev);
		return err;
	}

#ifdef WHY_DO_WE_NEED_THIS
	gpio_pend_mask_mem = ioremap(INT_PEND_BASE, 0x10);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	touchkey_driver->early_suspend.suspend = (void*) melfas_touchkey_early_suspend;
	touchkey_driver->early_suspend.resume = (void*) melfas_touchkey_late_resume;
	register_early_suspend(&touchkey_driver->early_suspend);
#endif				/* CONFIG_HAS_EARLYSUSPEND */

	/* enable ldo11 */
	touchkey_ldo_on(1);

	msleep(50);

	touchkey_enable = 1;
	data = 1;
#if 0
	i2c_touchkey_write(&data, 1);
#endif

	if (request_irq (IRQ_TOUCH_INT, touchkey_interrupt, IRQF_DISABLED, DEVICE_NAME, NULL)) {
		printk(KERN_ERR "[TouchKey] %s Can't allocate irq ..\n", __func__);
		return -EBUSY;
	}
    /* enable ldo20 */
    // touchkey_led_ldo_on(1);

#if 1
	// touchkey_firmware_update();

	msleep(100);
	touchkey_autocalibration();
	msleep(50);
#endif

	set_touchkey_debug('K');
	return 0;
}

static void init_hw(void)
{

	tegra_gpio_enable(GPIO_TKEY_INT);
	//gpio_request(GPIO_TKEY_INT, "TKEY_int");
	gpio_direction_input(GPIO_TKEY_INT);

	irq_set_irq_type(IRQ_TOUCH_INT, IRQ_TYPE_LEVEL_LOW);

	#if 0
	tegra_gpio_enable(_3_GPIO_TOUCH_INT);
	gpio_request(_3_GPIO_TOUCH_INT, "TKEY_int");
	gpio_direction_input(_3_GPIO_TOUCH_INT);

	set_irq_type(IRQ_TOUCH_INT, IRQ_TYPE_LEVEL_LOW);
    #endif

}

int touchkey_update_open(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t touchkey_update_read(struct file * filp, char *buf, size_t count,
			     loff_t * f_pos)
{
	char data[3] = { 0, };

	get_touchkey_firmware(data);
	put_user(data[1], buf);

	return 1;
}

int touchkey_update_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations touchkey_update_fops = {
	.owner = THIS_MODULE,
	.read = touchkey_update_read,
	.open = touchkey_update_open,
	.release = touchkey_update_release,
};

static struct miscdevice touchkey_update_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "melfas_touchkey",
	.fops = &touchkey_update_fops,
};

static ssize_t touch_version_read(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
#if 0
	char data[3] = { 0, };
	int count;

	init_hw();
	if (get_touchkey_firmware(data) != 0) {
		i2c_touchkey_read(KEYCODE_REG, data, 3);
	}
	count = sprintf(buf, "0x%x\n", data[1]);

	printk(KERN_DEBUG "[TouchKey] touch_version_read 0x%x\n", data[1]);
	return count;
#else
	u8 data[3];
	int ret;

	printk(KERN_DEBUG "called %s \n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 3);
	printk(KERN_DEBUG "called %s data[1] =%d,data[2] = %d\n", __func__, data[1], data[2]);

	return sprintf(buf, "0x%x\n", data[1]);
#endif

}

static ssize_t touch_version_write(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	printk(KERN_DEBUG "[TouchKey] input data --> %s\n", buf);

	return size;
}

void touchkey_update_func(struct work_struct *p)
{
	int retry = 10;
	touchkey_update_status = 1;
	printk(KERN_DEBUG "[TouchKey] %s start\n", __func__);

#if 1
	{
		char data[3];

		i2c_touchkey_read(KEYCODE_REG, data, 3);
		printk(KERN_DEBUG"[%s] F/W version: 0x%x, Module version:0x%x\n",
				__FUNCTION__, data[1], data[2]);
	}
#endif
	while (retry--) {
		if (ISSP_main() == 0) {
			touchkey_update_status = 0;
			printk(KERN_DEBUG
			       "[TouchKey] touchkey_update succeeded\n");
			enable_irq(IRQ_TOUCH_INT);

			msleep(500);
			touchkey_autocalibration();

			return;
		}
	}

	touchkey_update_status = -1;
	printk(KERN_DEBUG "[TouchKey] touchkey_update failed\n");
	return;
}

static ssize_t touch_update_write(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	printk(KERN_DEBUG "[TouchKey] touchkey firmware update \n");

	if (*buf == 'S') {
		disable_irq(IRQ_TOUCH_INT);
		INIT_WORK(&touch_update_work, touchkey_update_func);
		queue_work(touchkey_wq, &touch_update_work);
	}
	return size;
}

static ssize_t touch_update_read(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int count = 0;

	printk(KERN_DEBUG
	       "[TouchKey] touch_update_read: touchkey_update_status %d\n",
	       touchkey_update_status);

	if (touchkey_update_status == 0) {
		count = sprintf(buf, "PASS\n");
	} else if (touchkey_update_status == 1) {
		count = sprintf(buf, "Downloading\n");
	} else if (touchkey_update_status == -1) {
		count = sprintf(buf, "Fail\n");
	}

	return count;
}

static ssize_t touch_led_control(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t size)
{
	int data;
	int errnum;
	if (sscanf(buf, "%d\n", &data) == 1)
    {
		if (data == 0)
			data=32;
		else if (data == 255)
			data=16;
		printk(KERN_DEBUG "[TouchKey] touch_led_control: %d \n", data);
		errnum = i2c_touchkey_write((u8 *)&data, 1);
		if(errnum==-ENODEV) {
			touchled_cmd_reversed = 1;
		}
		touchkey_led_status = data;
	} else {
		printk(KERN_DEBUG "[TouchKey] touch_led_control Error\n");
	}

	return size;
}

static ssize_t touchkey_enable_disable(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
	return size;
}

static ssize_t touchkey_menu_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	u8 data[18];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
	menu_sensitivity = ((0x00FF&data[10])<<8) | data[11];
	return sprintf(buf,"%d\n",menu_sensitivity);
}

static ssize_t touchkey_home_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[18];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
	home_sensitivity = ((0x00FF&data[12])<<8) | data[13];
	return sprintf(buf, "%d\n", home_sensitivity);
}

static ssize_t touchkey_back_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	u8 data[18];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
	back_sensitivity = ((0x00FF&data[14])<<8)  | data[15];
	return sprintf(buf,"%d\n",back_sensitivity);
}

static ssize_t touchkey_search_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 data[18];
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
	search_sensitivity = ((0x00FF&data[16])<<8)  | data[17];
	return sprintf(buf, "%d\n", search_sensitivity);
}



static ssize_t touch_sensitivity_control(struct device *dev, struct
device_attribute *attr, const char *buf, size_t size)
{
	unsigned char data = 0x40;
	i2c_touchkey_write(&data, 1);
	return size;
}

static ssize_t set_touchkey_firm_version_show(struct device *dev, struct
device_attribute *attr, char *buf)
{

    /*TO DO IT */
	int count;
	count = sprintf(buf, "0x%x\n", TOUCH_FIRMWARE_V0A);
	return count;

}

static ssize_t set_touchkey_update_show(struct device *dev, struct
device_attribute *attr, char *buf)
{
    /*TO DO IT */
	int count=0;
	int retry=3;
	touchkey_update_status = 1;
	while (retry--) {
			if (ISSP_main() == 0) {
				printk(KERN_ERR"[TOUCHKEY]Touchkey_update succeeded\n");
				touchkey_update_status = 0;
				count=1;
				break;
			}
			printk(KERN_ERR"touchkey_update failed... retry...\n");
	}
	if (retry <= 0) {
			// disable ldo11
			#if 0
			touchkey_ldo_on(0);
            #endif
			msleep(300);
			count=0;
			printk(KERN_ERR"[TOUCHKEY]Touchkey_update fail\n");
			touchkey_update_status = -1;
			return count;
	}

	init_hw();	//after update, re initalize.
	{
		unsigned char get_touch = 0x40;
		i2c_touchkey_write(&get_touch, 1);
	}
	return count;

}

static ssize_t set_touchkey_firm_version_read_show(struct device *dev, struct
device_attribute *attr, char *buf)
{
#if 0
  char data[3] = { 0, };
  int count;

  init_hw();
  if (get_touchkey_firmware(data) != 0) {
	  i2c_touchkey_read(KEYCODE_REG, data, 3);
  }
  count = sprintf(buf, "0x%x\n", data[1]);

  printk(KERN_DEBUG "[TouchKey] set_touchkey_firm_version_read_show 0x%x\n", data[1]);
  return count;
#else

	u8 data[3];
	int ret;

	printk(KERN_DEBUG "called %s \n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 3);
	printk(KERN_DEBUG "called %s data[1] =%d,data[2] = %d\n", __func__, data[1], data[2]);

	return sprintf(buf, "0x%x\n", data[1]);

#endif
}

static ssize_t set_touchkey_firm_status_show(struct device *dev, struct
device_attribute *attr, char *buf)
{
	int count = 0;

	printk(KERN_DEBUG
	       "[TouchKey] touch_update_read: touchkey_update_status %d\n",
	       touchkey_update_status);

	if (touchkey_update_status == 0) {
		count = sprintf(buf, "PASS\n");
	} else if (touchkey_update_status == 1) {
		count = sprintf(buf, "Downloading\n");
	} else if (touchkey_update_status == -1) {
		count = sprintf(buf, "Fail\n");
	}

	return count;
}

/* sec_touchkey */
static DEVICE_ATTR(touchkey_firm_update, S_IRUGO, set_touchkey_update_show,NULL);
static DEVICE_ATTR(touchkey_firm_update_status, S_IRUGO, set_touchkey_firm_status_show, NULL);
static DEVICE_ATTR(touchkey_firm_version_phone, S_IRUGO, set_touchkey_firm_version_show,NULL);
static DEVICE_ATTR(touchkey_firm_version_panel, S_IRUGO, set_touchkey_firm_version_read_show,NULL);
static DEVICE_ATTR(touchkey_autocal_start, 0664, NULL, set_touchkey_autocal_testmode);

/* misc/melfas_touchkey */
static DEVICE_ATTR(touch_version, 0664, touch_version_read, touch_version_write);
static DEVICE_ATTR(touch_update, 0664, touch_update_read, touch_update_write);
static DEVICE_ATTR(brightness, 0664, NULL, touch_led_control);
static DEVICE_ATTR(enable_disable, 0664, NULL, touchkey_enable_disable);

static DEVICE_ATTR(touchkey_menu, 				S_IRUGO, 								touchkey_menu_show, 					NULL);
static DEVICE_ATTR(touchkey_back, 				S_IRUGO, 								touchkey_back_show, 					NULL);
static DEVICE_ATTR(touchkey_home, S_IRUGO, touchkey_home_show, NULL);
static DEVICE_ATTR(touchkey_search, S_IRUGO, touchkey_search_show, NULL);
static DEVICE_ATTR(touch_sensitivity, S_IRUGO, NULL, touch_sensitivity_control);

static DEVICE_ATTR(touchkey_raw_data0, S_IRUGO, touchkey_raw_data0_show, NULL);
static DEVICE_ATTR(touchkey_raw_data1,	S_IRUGO, touchkey_raw_data1_show, NULL);
static DEVICE_ATTR(touchkey_raw_data2,	S_IRUGO, touchkey_raw_data2_show, NULL);
static DEVICE_ATTR(touchkey_raw_data3,	S_IRUGO, touchkey_raw_data3_show, NULL);
static DEVICE_ATTR(touchkey_idac0,	S_IRUGO, touchkey_idac0_show, NULL);
static DEVICE_ATTR(touchkey_idac1,	S_IRUGO, touchkey_idac1_show, NULL);
static DEVICE_ATTR(touchkey_idac2,	S_IRUGO, touchkey_idac2_show, NULL);
static DEVICE_ATTR(touchkey_idac3,	S_IRUGO, touchkey_idac3_show, NULL);
static DEVICE_ATTR(touchkey_threshold, S_IRUGO, touchkey_threshold_show, NULL);


static int __init touchkey_init(void)
{
	int ret = 0;
	//int retry = 3;
	//char data[3] = { 0, };


	//gpio_request(_3_TOUCH_SDA_28V, "_3_TOUCH_SDA_28V");
	//gpio_request(_3_TOUCH_SCL_28V, "_3_TOUCH_SCL_28V");
	//gpio_request(_3_GPIO_TOUCH_EN, "_3_GPIO_TOUCH_EN");
	gpio_request(_3_GPIO_TOUCH_INT, "_3_GPIO_TOUCH_INT");

    sec_touchkey= device_create(sec_class, NULL, 0, NULL, "sec_touchkey");
	if (IS_ERR(sec_touchkey))
	{
		printk(KERN_ERR "Failed to create device(sec_touchkey)!\n");
	}
	if (device_create_file(sec_touchkey, &dev_attr_touchkey_firm_update)< 0) {
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_firm_update.attr.name);
	}
	if (device_create_file(sec_touchkey, &dev_attr_touchkey_firm_update_status)< 0)	{
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_firm_update_status.attr.name);
	}
	if (device_create_file(sec_touchkey, &dev_attr_touchkey_firm_version_phone)< 0)	{
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_firm_version_phone.attr.name);
	}
	if (device_create_file(sec_touchkey, &dev_attr_touchkey_firm_version_panel)< 0)	{
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_firm_version_panel.attr.name);
	}
	if (device_create_file(sec_touchkey, &dev_attr_touchkey_autocal_start) < 0) {
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_autocal_start.attr.name);
	}


	ret = misc_register(&touchkey_update_device);
	if (ret) {
		printk(KERN_ERR "[TouchKey] %s misc_register fail\n", __func__);
	}

	/* version, update, brightness, enabledisable */
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touch_version) < 0) {
		printk(KERN_ERR "[TouchKey] %s device_create_file fail dev_attr_touch_version\n", __func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touch_version.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touch_update) < 0) {
		printk(KERN_ERR "[TouchKey] %s device_create_file fail dev_attr_touch_update\n", __func__);
		pr_err("Failed to create device file(%s)!\n",dev_attr_touch_update.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_brightness) < 0) {
		printk(KERN_ERR "[TouchKey] %s device_create_file fail dev_attr_touch_update\n", __func__);
		pr_err("Failed to create device file(%s)!\n",dev_attr_brightness.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_enable_disable) < 0) {
		printk(KERN_ERR"[TouchKey] %s device_create_file fail dev_attr_touch_update\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_enable_disable.attr.name);
	}

	/* menu, home, back, search, sensitivity */
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_menu) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_menu\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_menu.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_back) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_back\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_back.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_home) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_home\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_home.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_search) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_search\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_search.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touch_sensitivity) < 0) {
		printk("%s device_create_file fail dev_attr_touch_sensitivity\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touch_sensitivity.attr.name);
	}

	/* rewdata, idac value x4, threshold */
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_raw_data0) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_raw_data0\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_raw_data0.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_raw_data1) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_raw_data1\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_raw_data1.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_raw_data2) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_raw_data2\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_raw_data2.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_raw_data3) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_raw_data3\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_raw_data3.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_idac0) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_idac0\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_idac0.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_idac1) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_idac1\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_idac1.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_idac2) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_idac2\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_idac2.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_idac3) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_idac3\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_idac3.attr.name);
	}
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_touchkey_threshold) < 0) {
		printk(KERN_ERR"%s device_create_file fail dev_attr_touchkey_threshold\n",__func__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_touchkey_threshold.attr.name);
	}

	touchkey_wq = create_singlethread_workqueue("melfas_touchkey_wq");
	if (!touchkey_wq) {
		return -ENOMEM;
	}

	INIT_WORK(&touchkey_work, touchkey_work_func);

	init_hw();

	ret = i2c_add_driver(&touchkey_i2c_driver);

	if (ret) {
		printk(KERN_ERR"[TouchKey] melfas touch keypad registration failed, module not inserted.ret= %d\n",ret);
	}
	/* Cypress Firmware Update>>>>>>>>>>
		i2c_touchkey_read(KEYCODE_REG, data, 3);
		printk(KERN_ERR"%s F/W version: 0x%x, Module version:0x%x\n", __FUNCTION__, data[1], data[2]);
		retry = 3;

		touch_version = data[1];
		module_version = data[2];

		// Firmware check & Update
		if((module_version == DOOSUNGTECH_TOUCH_V1_2)&& (touch_version != TOUCH_FIRMWARE_V04)){
			touchkey_update_status=1;
			while (retry--) {
				if (ISSP_main() == 0) {
					printk(KERN_ERR"[TOUCHKEY]Touchkey_update succeeded\n");
					touchkey_update_status=0;
					break;
				}
				printk(KERN_ERR"touchkey_update failed... retry...\n");
		   }
			if (retry <= 0) {
				// disable ldo11
				touchkey_ldo_on(0);
				touchkey_update_status=-1;
				msleep(300);

			}

			init_hw();	//after update, re initalize.
		}else {
			if(module_version != DOOSUNGTECH_TOUCH_V1_2){
				printk(KERN_ERR "[TouchKey] Not F/W update. Cypess touch-key module is not DOOSUNG TECH. \n");
			} else if(touch_version == TOUCH_FIRMWARE_V04){
				printk(KERN_ERR "[TouchKey] Not F/W update. Cypess touch-key F/W version is latest. \n");
			} else {
				printk(KERN_ERR "[TouchKey] Not F/W update. Cypess touch-key version(module or F/W) is not valid. \n");
			}
		}

	 <<<<<<<<<<<<<< Cypress Firmware Update */

	{
		unsigned char get_touch = 0x40;
		i2c_touchkey_write(&get_touch, 1);
	}

	return ret;

}

static void __exit touchkey_exit(void)
{
	printk(KERN_DEBUG "[TouchKey] %s \n", __func__);
	i2c_del_driver(&touchkey_i2c_driver);
	misc_deregister(&touchkey_update_device);

	if (touchkey_wq) {
		destroy_workqueue(touchkey_wq);
	}

	gpio_free(_3_TOUCH_SDA_28V);
	gpio_free(_3_TOUCH_SCL_28V);
	//gpio_free(_3_GPIO_TOUCH_EN);
	gpio_free(_3_GPIO_TOUCH_INT);
}

late_initcall(touchkey_init);
module_exit(touchkey_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("@@@");
MODULE_DESCRIPTION("melfas touch keypad");
