/*
 *
 * Sain touch driver
 *
 * Copyright (C) 2009 Sain, Inc.
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
/*
 * Version 2.0.0 : using reg data file (2010/11/05)
 * Version 2.0.1 : syntxt bug fix (2010/11/09)
 * Version 2.0.2 : Save status cmd delay bug (2010/11/10)
 * Version 2.0.3 : modify delay 10ms -> 50ms for clear hw calibration bit
 *		: modify SAIN_TOTAL_NUMBER_OF_Y register (read only -> read/write )
 *		: modify SUPPORTED FINGER NUM register (read only -> read/write )
 * Version 2.0.4 : [20101116]
 *	Modify Firmware Upgrade routine.
 * Version 2.0.5 : [20101118]
 *	add esd timer function & some bug fix.
 *	you can select request_threaded_irq or request_irq, setting USE_THREADED_IRQ.
 * Version 2.0.6 : [20101123]
 *	add ABS_MT_WIDTH_MAJOR Report
 * Version 2.0.7 : [20101201]
 *	Modify sain_early_suspend() / sain_late_resume() routine.
 * Version 2.0.8 : [20101216]
 *	add using spin_lock option
 * Version 2.0.9 : [20101216]
 *	Test Version
 * Version 2.0.10 : [20101217]
 *	add USE_THREAD_METHOD option.
 * Version 2.0.11 : [20101229]
 *	add USE_UPDATE_SYSFS option for update firmware. && TOUCH_MODE == 1 mode.
*/

#include <linux/module.h>
#include <linux/input.h>
#include <linux/i2c.h>		/* I2C_M_NOSTART */
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/ioctl.h>
#include <linux/earlysuspend.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/delay.h>

#include <mach/gpio.h>

#include <linux/regulator/consumer.h>
#include <linux/input/sain_touch.h>

#include "sain_touch_reg_data.h"
#if TOUCH_ONESHOT_UPGRADE
#include "sain_touch_firmware.h"
#endif
#include <linux/wakelock.h>

#include <asm/uaccess.h>
#include <asm/io.h>


#define SAIN_TSP_FIRMWARE_UPDATE_START 7		/* as changing TSP structure(G2 to G1f), firmware updating started at version.7 */

#define	SYSTEM_MAX_X_RESOLUTION	480
#define	SYSTEM_MAX_Y_RESOLUTION	800

#define	SAIN_DEBUG		0

#define	SAIN_DRIVER_NAME	"sain_touch"

#define	SAIN_INIT_RETRY_CNT	10

#define	SAIN_ESD_TIMER_INTERVAL	0	/* second : if 0, no use. */
#define	SAIN_SCAN_RATE_HZ	60

#define	USE_THREAD_METHOD	1	/* 0 = workqueue, 1 = thread */

#define	USE_UPDATE_SYSFS	0
#define	SAIN_TSP_FACTORY_TEST	1

#define	KEY_LED_CONTROL

#ifdef KEY_LED_CONTROL
#define MENU_KEY_LED_MASK	0x0001
#define BACK_KEY_LED_MASK	0x0002
#define MENU_KEY_LED_OFF	BACK_KEY_LED_MASK
#define BACK_KEY_LED_OFF		MENU_KEY_LED_MASK
#define MENU_BACK_KEY_LED_ON	(MENU_KEY_LED_MASK | BACK_KEY_LED_MASK)
static u32 key_led_status = false;

static void key_led_on(u32 val);
#endif

#if	(!USE_THREAD_METHOD)
static struct workqueue_struct *sain_workqueue;
#endif


#if	SAIN_DEBUG
#define	sain_debug_msg(fmt, args...)	printk(KERN_INFO "[%-18s:%5d]" fmt, __FUNCTION__, __LINE__, ## args)
#else
#define	sain_debug_msg(fmt, args...)	do {} while (0)
#endif

#define swap_v(a, b, t)	((t) = (a), (a) = (b), (b) = (t))
#define swap_16(s) (((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)))

#define	SUPPORTED_FINGER_NUM			4

#define	ICON_BUTTON_UNCHANGE			0
#define	ICON_BUTTON_DOWN			1
#define	ICON_BUTTON_UP				2


#define	I2C_SUCCESS				0

#define ts_write_cmd(client, reg) i2c_smbus_write_byte(client, reg)
#define ts_write_reg(client, reg, value) i2c_smbus_write_word_data(client, reg, value)
#define ts_select_reg(client, reg) i2c_smbus_write_byte(client, reg)
#define ts_read_reg(client, reg) i2c_smbus_read_word_data(client, reg)


#if 0
#define ts_read_data(client, reg, values, length) i2c_smbus_read_i2c_block_data(client, reg, length, values)
#else
inline s32 ts_read_data(struct i2c_client *client, u8 reg, u8 *values, u8 length)
{
	s32 ret;

	ret = i2c_master_send(client , &reg , 1);
	if (ret < 0)
		return ret;
	else if (!ret)
		return -EIO;

	udelay(10);

	ret = i2c_master_recv(client, values, length);
	if (ret < 0)
		return ret;
	else if (!ret)
		return -EIO;
	return length;
}

inline s32 ts_read_firmware_data(struct i2c_client *client, u8 reg, u8 *values, u8 length)
{
	s32 ret;

	ret = i2c_master_send(client , &reg , 1);
	if (ret < 0)
		return ret;
	else if (!ret)
		return -EIO;

	mdelay(1);

	ret = i2c_master_recv(client , values , length);
	if (ret < 0)
		return ret;
	else if (!ret)
		return -EIO;

	return length;
}

#endif

#define ts_write_data(client, reg, values, length) i2c_smbus_write_i2c_block_data(client, reg, length, values)

#define	TOUCH_MODE				0

#define	UPGRADE_DATA_SIZE			512
#define	TOUCH_UPGRADE_MINOR			150

typedef	struct{
	u16	address;
	u8	data[UPGRADE_DATA_SIZE];
} _sain_upgrade_info;

typedef	struct{
	u16	x;
	u16	y;
	u8	width;
	u8	sub_status;
} _ts_sain_coord;

typedef	struct{
	u16	status;
#if (TOUCH_MODE == 1)
	u16	event_flag;
#else
	u8	finger_cnt;
	u8	time_stamp;
#endif
	_ts_sain_coord	coord[SUPPORTED_FINGER_NUM];

} _ts_sain_point_info;


#define	TOUCH_V_FLIP	0x01
#define	TOUCH_H_FLIP	0x02
#define	TOUCH_XY_SWAP	0x04

typedef	struct{
	u16 chip_revision;
	u16 chip_firmware_version;
	u16 chip_reg_data_version;
	u32 chip_fw_size;
	u32 MaxX;
	u32 MaxY;
	u32 MinX;
	u32 MinY;
	u32 Orientation;
	u8 gesture_support;
	u8 multi_fingers;
} _ts_capa_info;

#define	SUPPORTED_BUTTON_NUM			2
u32 BUTTON_MAPPING_KEY[SUPPORTED_BUTTON_NUM] = {KEY_MENU, KEY_BACK};

typedef struct{
	struct sain_touch_platform_data *pdata;
	struct input_dev *input_dev;
	struct task_struct *task;
	wait_queue_head_t	wait;
	struct work_struct  work;
	struct i2c_client *client;
	struct semaphore update_lock;
	u32 i2c_dev_addr;
	_ts_capa_info	cap_info;

	bool is_valid_event;
	_ts_sain_point_info touch_info;
	_ts_sain_point_info reported_touch_info;
	u16 icon_event_reg;
	u16 chip_int_mask;
	u16 event_type;
	u32 int_gpio_num;
	u32 irq;
	u8 button[SUPPORTED_BUTTON_NUM];

#if	SAIN_ESD_TIMER_INTERVAL
	u8	use_esd_timer;
	struct semaphore esd_lock;
	struct timer_list esd_timeout_tmr;
	struct timer_list *p_esd_timeout_tmr;
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif

#if	(!USE_THREAD_METHOD)
	struct workqueue_struct *sain_workqueue;
#endif

} sain_touch_dev;

#define	TS_DRIVER_NAME	"sain_touch"
static struct i2c_device_id sain_idtable[] = {
    {TS_DRIVER_NAME, 0},
    { }
};

#ifdef SAIN_TSP_FACTORY_TEST
unsigned int sain_firm_status_data=0;
#endif

static int sain_touch_probe(struct i2c_client *client, const struct i2c_device_id *i2c_id);
static int sain_touch_remove(struct i2c_client *client);

#if (TOUCH_MODE == 1)
static void	sain_report_data(sain_touch_dev *touch_dev, int id);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void sain_early_suspend(struct early_suspend *h);
static void sain_late_resume(struct early_suspend *h);
#endif

#if	SAIN_ESD_TIMER_INTERVAL
static void ts_esd_timer_start(unsigned long sec, sain_touch_dev *touch_dev);
static void ts_esd_timer_stop(sain_touch_dev *touch_dev);
static void ts_esd_timer_modify(unsigned long sec, sain_touch_dev *touch_dev);
static void ts_esd_timer_init(sain_touch_dev *touch_dev);
static void ts_esd_timeout_handler(unsigned long data);
#endif


static struct i2c_driver sain_touch_driver = {
	.probe     = sain_touch_probe,
	.remove    = sain_touch_remove,
	.id_table  = sain_idtable,
	.driver    = {
		.name  = SAIN_DRIVER_NAME,
	},
};


static bool ts_get_samples (sain_touch_dev *touch_dev)
{
	u32 event_type;
	int i;
	struct sain_touch_platform_data *pdata;

	pdata = touch_dev->client->dev.platform_data;

	sain_debug_msg("ts_get_samples+\r\n");

	if (gpio_get_value(touch_dev->int_gpio_num)) {
		/* interrupt pin is high, not valid data. */
		printk(KERN_WARNING "woops... inturrpt pin is high\r\n");
		return false;
	}

#if (TOUCH_MODE == 1)

	memset(&touch_dev->touch_info, 0x0, sizeof(_ts_sain_point_info));

	if (ts_read_data (touch_dev->client, SAIN_POINT_STATUS_REG, (u8 *)(&touch_dev->touch_info), 4) <  0) {
		sain_debug_msg("error read point info using i2c.-\r\n");
		return false;
	}
	sain_debug_msg("status reg = 0x%x , event_flag = 0x%04x\r\n", touch_dev->touch_info.status, touch_dev->touch_info.event_flag);

	if (touch_dev->touch_info.status == 0x0) {
		sain_debug_msg("periodical esd repeated int occured\r\n");
		return true;
	}

	if (sain_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT)) {
		if (ts_read_data (touch_dev->client, SAIN_ICON_STATUS_REG, (u8 *)(&touch_dev->icon_event_reg), 2) < 0) {
			printk(KERN_INFO "error read icon info using i2c.\n");
			return false;
		}
		return true;
	}

	if (!sain_bit_test(touch_dev->touch_info.status, BIT_PT_EXIST)) {
		ts_write_cmd(touch_dev->client, SAIN_CLEAR_INT_STATUS_CMD);
		for (i = 0; i < SUPPORTED_FINGER_NUM; i++) {
			if (sain_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST)) {
				input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
				input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
				input_mt_sync(touch_dev->input_dev);
				touch_dev->reported_touch_info.coord[i].sub_status = 0;
			}
		}
		input_sync(touch_dev->input_dev);
		return false;
	}

	for (i = 0; i < SUPPORTED_FINGER_NUM; i++) {
		if (sain_bit_test(touch_dev->touch_info.event_flag, i)) {
			if (ts_read_data (touch_dev->client, SAIN_POINT_STATUS_REG+2+i, (u8 *)(&touch_dev->touch_info.coord[i]), sizeof(_ts_sain_coord)) < 0) {
				sain_debug_msg("error read point info using i2c.-\r\n");
				return false;
			}
			sain_bit_clr(touch_dev->touch_info.event_flag, i);
			if (touch_dev->touch_info.event_flag == 0) {
				ts_write_cmd(touch_dev->client, SAIN_CLEAR_INT_STATUS_CMD);
				sain_report_data(touch_dev, i);
				return false;
			} else
				sain_report_data(touch_dev, i);
		}
	}


#else
	if (ts_read_data (touch_dev->client, SAIN_POINT_STATUS_REG, (u8 *)(&touch_dev->touch_info), sizeof(_ts_sain_point_info)) < 0) {
		sain_debug_msg("error read point info using i2c.-\r\n");
		return false;
	}
	sain_debug_msg("status reg = 0x%x , point cnt = %d, time stamp = %d\r\n", touch_dev->touch_info.status,
		touch_dev->touch_info.finger_cnt, touch_dev->touch_info.time_stamp);

	if (touch_dev->touch_info.status == 0x0 && touch_dev->touch_info.finger_cnt == 100) {
		sain_debug_msg("periodical esd repeated int occured\r\n");
		return true;
	}

	for (i = 0; i < SUPPORTED_BUTTON_NUM; i++)
		touch_dev->button[i] = ICON_BUTTON_UNCHANGE;

	if (sain_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT)) {
		udelay(10);
		if (ts_read_data (touch_dev->client, SAIN_ICON_STATUS_REG, (u8 *)(&touch_dev->icon_event_reg), 2) < 0) {
			printk(KERN_INFO "error read icon info using i2c.\n");
			return false;
		}
	}
#endif
	sain_debug_msg("ts_get_samples-\r\n");

	return true;
}


static int ts_int_handler(int irq, void *dev)
{
	sain_touch_dev *touch_dev = (sain_touch_dev *)dev;
	sain_debug_msg("interrupt occured +\r\n");
	disable_irq_nosync(irq);
#if USE_THREAD_METHOD
	up(&touch_dev->update_lock);
#else
	queue_work(touch_dev->sain_workqueue, &touch_dev->work);
#endif

	return IRQ_HANDLED;
}

static bool ts_read_coord (sain_touch_dev *hDevice)
{
	sain_touch_dev *touch_dev = (sain_touch_dev *)hDevice;
	unsigned long flags;

	if (ts_get_samples(touch_dev) == false) {
		return false;
	}
	ts_write_cmd(touch_dev->client, SAIN_CLEAR_INT_STATUS_CMD);
	return true;
}

static void ts_power_control(u8 ctl)
{
	/* to be developed */
}

static bool ts_mini_init_touch(sain_touch_dev *touch_dev)
{
	u16	reg_val;
	int i;
	if (ts_write_cmd(touch_dev->client, SAIN_CLEAR_INT_STATUS_CMD) != I2C_SUCCESS)
		goto fail_mini_init;
	if (ts_write_reg(touch_dev->client, SAIN_X_RESOLUTION, (u16)(touch_dev->cap_info.MaxX)) != I2C_SUCCESS)
		goto fail_mini_init;
	if (ts_write_reg(touch_dev->client, SAIN_Y_RESOLUTION, (u16)(touch_dev->cap_info.MaxY)) != I2C_SUCCESS)
		goto fail_mini_init;
	if (ts_write_reg(touch_dev->client, SAIN_SUPPORTED_FINGER_NUM, (u16)touch_dev->cap_info.multi_fingers) != I2C_SUCCESS)
		goto fail_mini_init;
	reg_val = TOUCH_MODE;
	if (ts_write_reg(touch_dev->client, SAIN_TOUCH_MODE, reg_val) != I2C_SUCCESS)
		goto fail_mini_init;
	if (ts_write_cmd(touch_dev->client, SAIN_CALIBRATE_CMD) != I2C_SUCCESS)
		goto fail_mini_init;
	if (ts_write_reg(touch_dev->client, SAIN_INT_ENABLE_FLAG, touch_dev->chip_int_mask) != I2C_SUCCESS)
		goto fail_mini_init;

	/* read garbage data */
	for (i = 0; i < 10; i++)	{
		ts_write_cmd(touch_dev->client, SAIN_CLEAR_INT_STATUS_CMD);
	}

	if (ts_write_reg(touch_dev->client, SAIN_PERIODICAL_INTERRUPT_INTERVAL, SAIN_SCAN_RATE_HZ*SAIN_ESD_TIMER_INTERVAL) != I2C_SUCCESS)
		goto fail_mini_init;

#if	SAIN_ESD_TIMER_INTERVAL
	ts_esd_timer_start(SAIN_ESD_TIMER_INTERVAL*3, touch_dev);
#endif
	return true;
fail_mini_init:
	printk(KERN_ERR "error mini init\n");
	return false;
}


#if	SAIN_ESD_TIMER_INTERVAL

static void ts_esd_timer_start(unsigned long sec, sain_touch_dev *touch_dev)
{
	ts_esd_timer_stop();

	init_timer(&(touch_dev->esd_timeout_tmr));
	touch_dev->esd_timeout_tmr.data = (unsigned long)(touch_dev);
	touch_dev->esd_timeout_tmr.function = ts_esd_timeout_handler;
	touch_dev->esd_timeout_tmr.expires = jiffies + (unsigned long)(HZ*sec);
	touch_dev->p_esd_timeout_tmr = &touch_dev->esd_timeout_tmr;
	if (timer_pending(&touch_dev->esd_timeout_tmr))
		add_timer(&touch_dev->esd_timeout_tmr);
	else
		printk(KERN_WARNING "sain touch : esd timer is already registered..\n");
}

static void ts_esd_timer_stop(sain_touch_dev *touch_dev)
{
	if (touch_dev->p_esd_timeout_tmr != NULL) {
		if (timer_pending(touch_dev->p_esd_timeout_tmr))
			del_timer(touch_dev->p_esd_timeout_tmr);
	}
	touch_dev->p_esd_timeout_tmr = NULL;
}

static void ts_esd_timer_modify(unsigned long sec, sain_touch_dev *touch_dev)
{
	if (timer_pending(&touch_dev->esd_timeout_tmr))
		mod_timer(&touch_dev->esd_timeout_tmr, jiffies + (unsigned long)(HZ*sec));
}

static void ts_esd_timer_init(sain_touch_dev *touch_dev)
{
	init_timer(&(touch_dev->esd_timeout_tmr));
	touch_dev->esd_timeout_tmr.data = (unsigned long)(touch_dev);
	touch_dev->esd_timeout_tmr.function = ts_esd_timeout_handler;
	touch_dev->p_esd_timeout_tmr = NULL;
}

static void ts_esd_timeout_handler(unsigned long data)
{
	sain_touch_dev *touch_dev = (sain_touch_dev *)data;
	u16	reg_val;
	int i;

	touch_dev->p_esd_timeout_tmr = NULL;

	down(&touch_dev->esd_lock);
	disable_irq(touch_dev->client->irq);
	printk(KERN_INFO "error. timeout occured. maybe ts device dead. so reset & reinit.\r\n");
	/* must h/w reset (cold reset) and mdelay(700) */
	regulator_disable(touch_dev->pdata->reg_vdd);
	regulator_disable(touch_dev->pdata->reg_avdd);
	mdelay(10);
	regulator_enable(touch_dev->pdata->reg_vdd);
	regulator_enable(touch_dev->pdata->reg_avdd);
	mdelay(700);

	if (ts_mini_init_touch(touch_dev) == false)
		goto fail_time_out_init;
	up(&touch_dev->esd_lock);
	enable_irq(touch_dev->client->irq);
	return;
fail_time_out_init:
	printk(KERN_INFO "timeout : restart error\r\n");
	ts_esd_timer_start(SAIN_ESD_TIMER_INTERVAL*3, touch_dev);
	up(&touch_dev->esd_lock);
	enable_irq(touch_dev->client->irq);
}
#endif

#if	TOUCH_ONESHOT_UPGRADE
bool ts_check_need_upgrade(u16 curVersion, u16 curRegVersion)
{
	u16	newVersion;
	newVersion = (u16) (m_firmware_data[0] | (m_firmware_data[1] << 8));

	printk(KERN_INFO "cur Version = 0x%x, new Version = 0x%x\n", curVersion, newVersion);

	if (curVersion < newVersion)
		return true;
	else if (curVersion > newVersion)
		return false;
	if (m_firmware_data[0x3FFE] == 0xff && m_firmware_data[0x3FFF] == 0xff)
		return false;

	newVersion = (u16) (m_firmware_data[0x3FFE] | (m_firmware_data[0x3FFF] << 8));

	printk(KERN_INFO "cur RegVersion = 0x%x, new RegVersion = 0x%x\n", curRegVersion, newVersion);
	if ((curRegVersion < newVersion) && (curRegVersion >= SAIN_TSP_FIRMWARE_UPDATE_START))
		return true;	/* as changing TSP structure(G2 to G1f), firmware updating started at version.7 */
	return false;
}


#define	TC_FIRMWARE_SIZE	(16*1024)
#define	TC_PAGE_SZ		64
#define	TC_SECTOR_SZ		8

u8 ts_upgrade_firmware(sain_touch_dev *touch_dev, u8 *firmware_data, u32 size)
{
	u16 flash_addr;
	u32 i;
	u8  *verify_data;
	int	retry_cnt = 0;

	verify_data = (u8 *)kzalloc(size, GFP_KERNEL);
	if (verify_data == NULL) {
		printk(KERN_ERR "cannot alloc verify buffer\n");
		return false;
	}

	do {
		printk(KERN_INFO "reset command\n");
		if (ts_write_cmd(touch_dev->client, SAIN_REST_CMD) != I2C_SUCCESS) {
			printk(KERN_INFO "failed to reset\n");
			goto fail_upgrade;
		}

		printk(KERN_INFO "Erase Flash\n");
		if (ts_write_reg(touch_dev->client, SAIN_ERASE_FLASH, 0xaaaa) != I2C_SUCCESS) {
			printk(KERN_INFO "failed to erase flash\n");
			goto fail_upgrade;
		}

		mdelay(500);

		printk(KERN_INFO "writing firmware data\n");

		for (flash_addr = 0; flash_addr < size; ) {
			for (i = 0; i < TC_PAGE_SZ/TC_SECTOR_SZ; i++) {
				printk(KERN_INFO "addr = %04x, len = %d\n", flash_addr, TC_SECTOR_SZ);
				if (ts_write_data(touch_dev->client, SAIN_WRITE_FLASH, &firmware_data[flash_addr], TC_SECTOR_SZ) < 0) {
					printk(KERN_INFO"error : write sain tc firmare\n");
					goto fail_upgrade;
				}
				flash_addr += TC_SECTOR_SZ;
			}
			mdelay(20);
		}

		printk(KERN_INFO "read firmware data\n");
		for (flash_addr = 0; flash_addr < size; ) {
			for (i = 0; i < TC_PAGE_SZ/TC_SECTOR_SZ; i++) {
				printk(KERN_INFO "addr = %04x, len = %d\n", flash_addr, TC_SECTOR_SZ);
				if (ts_read_firmware_data(touch_dev->client, SAIN_READ_FLASH, &verify_data[flash_addr], TC_SECTOR_SZ) < 0) {
					printk(KERN_INFO "error : read sain tc firmare\n");
					goto fail_upgrade;
				}
				flash_addr += TC_SECTOR_SZ;
			}
		}

		printk(KERN_INFO "verify firmware data\n");
		if (memcmp((u8 *)&firmware_data[0], (u8 *)&verify_data[0], size) == 0) {
			printk(KERN_INFO "upgrade finished\n");
			kfree(verify_data);
			return true;
		}
		printk(KERN_INFO "upgrade fail : so retry... (%d)\n", ++retry_cnt);

	} while (1);

fail_upgrade:
	kfree(verify_data);
	return false;

}
#endif

bool ts_init_touch(sain_touch_dev *touch_dev)
{
	u16	reg_val;
	int	i,ret;
	u16 	SetMaxX = SYSTEM_MAX_X_RESOLUTION;	/* Max Position range from 0x0002 to 0x1fff */
	u16 	SetMaxY = SYSTEM_MAX_Y_RESOLUTION;	/* Max Position range from 0x0002 to 0x1fff */
	u16	SupportedFingerNum = SUPPORTED_FINGER_NUM;

	u16 	CurMaxX = 1024;
	u16 	CurMaxY = 1920;
	u16 	chip_revision;
	u16 	chip_firmware_version;
	u16	chip_reg_data_version;
	u16	chip_eeprom_info;
	s16	stmp;
	struct sain_touch_platform_data *pdata;

	pdata = touch_dev->client->dev.platform_data;

	printk(KERN_INFO "sain touch init +\r\n");

	if (touch_dev == NULL) {
		printk(KERN_ERR "error touch_dev == null?\r\n");
		goto fail_init;
	}

	printk(KERN_INFO "disable interrupt\r\n");
	for (i = 0; i < SAIN_INIT_RETRY_CNT; i++) {
		ret = ts_write_reg(touch_dev->client, SAIN_INT_ENABLE_FLAG, 0x0);
		printk(KERN_INFO "SAIN INT write status = [%d], ret\r\n");
		if (ret == I2C_SUCCESS)
			break;
		//if (ts_write_reg(touch_dev->client, SAIN_INT_ENABLE_FLAG, 0x0) == I2C_SUCCESS)
		//	break;
	}

	if (i == SAIN_INIT_RETRY_CNT) {
		printk(KERN_INFO "fail to write interrupt register\r\n");
		goto fail_init;
	}
	printk(KERN_INFO "successfully disable int. (retry cnt = %d)\r\n", i);

	printk(KERN_INFO "send reset command\r\n");
	if (ts_write_cmd(touch_dev->client, SAIN_REST_CMD) != I2C_SUCCESS)
		goto fail_init;

	/* get chip revision id */
	if (ts_read_data(touch_dev->client, SAIN_CHIP_REVISION, (u8 *)&chip_revision, 2) < 0) {
		printk(KERN_INFO "fail to read chip revision\r\n");
		goto fail_init;
	}
	printk(KERN_INFO "sain touch chip revision id = %x\r\n", chip_revision);

	touch_dev->cap_info.chip_fw_size = 16*1024;
	if (chip_firmware_version >= 0x0a && chip_firmware_version <= 0x0b)
		touch_dev->cap_info.chip_fw_size = 16*1024;

	touch_dev->cap_info.multi_fingers = SUPPORTED_FINGER_NUM;

	/* get chip firmware version */
	if (ts_read_data(touch_dev->client, SAIN_FIRMWARE_VERSION, (u8 *)&chip_firmware_version, 2) < 0)
		goto fail_init;
	printk(KERN_INFO "sain touch chip firmware version = %x\r\n", chip_firmware_version);

#if	TOUCH_ONESHOT_UPGRADE
	chip_reg_data_version = 0xffff;
	if (chip_revision >= 0x0a && chip_firmware_version >= 0x68) {
		if (ts_read_data(touch_dev->client, SAIN_DATA_VERSION_REG, (u8 *)&chip_reg_data_version, 2) < 0)
			goto fail_init;
		printk(KERN_INFO "touch reg data version = %d\r\n", chip_reg_data_version);
	}

	if (ts_check_need_upgrade(chip_firmware_version, chip_reg_data_version) == true) {
		printk(KERN_INFO "start upgrade firmware\n");
		ts_upgrade_firmware(touch_dev, &m_firmware_data[2], touch_dev->cap_info.chip_fw_size);
		mdelay(10);

		/* must h/w reset (cold reset) and mdelay(700) */
		regulator_disable(touch_dev->pdata->reg_vdd);
		regulator_disable(touch_dev->pdata->reg_avdd);
		mdelay(10);
		regulator_enable(touch_dev->pdata->reg_vdd);
		regulator_enable(touch_dev->pdata->reg_avdd);
		mdelay(700);

		/* get chip revision id */
		if (ts_read_data(touch_dev->client, SAIN_CHIP_REVISION, (u8 *)&chip_revision, 2) < 0) {
			printk(KERN_INFO "fail to read chip revision\r\n");
			goto fail_init;
		}
		printk(KERN_INFO "sain touch chip revision id = %x\r\n", chip_revision);

		/* get chip firmware version */
		if (ts_read_data(touch_dev->client, SAIN_FIRMWARE_VERSION, (u8 *)&chip_firmware_version, 2) < 0)
			goto fail_init;
		printk(KERN_INFO "sain touch chip renewed firmware version = %x\r\n", chip_firmware_version);


		if (chip_revision < 0x0a || chip_firmware_version < 0x68) {
			/* h/w calibration */
			if (ts_write_reg(touch_dev->client, SAIN_TOUCH_MODE, 0x07) != I2C_SUCCESS)
				goto fail_init;
			if (ts_write_cmd(touch_dev->client, SAIN_CALIBRATE_CMD) != I2C_SUCCESS)
				goto fail_init;
			if (ts_write_cmd(touch_dev->client, SAIN_REST_CMD) != I2C_SUCCESS)
				goto fail_init;

			mdelay(3000);

			if (ts_write_reg(touch_dev->client, SAIN_TOUCH_MODE, 0x00) != I2C_SUCCESS)
				goto fail_init;
			if (ts_write_cmd(touch_dev->client, SAIN_REST_CMD) != I2C_SUCCESS)
				goto fail_init;
		}
	}
#endif

	if (chip_revision >= 0x0a && chip_firmware_version >= 0x68) {
		if (ts_read_data(touch_dev->client, SAIN_DATA_VERSION_REG, (u8 *)&chip_reg_data_version, 2) < 0)
			goto fail_init;
		printk(KERN_INFO "touch reg data version = %d\r\n", chip_reg_data_version);

		if ((chip_reg_data_version < m_reg_data[SAIN_DATA_VERSION_REG].reg_val) && (chip_reg_data_version >= SAIN_TSP_FIRMWARE_UPDATE_START)) {	/* as changing TSP structure(G2 to G1f), firmware updating started at version.7 */
			printk(KERN_INFO "write new reg data( %d < %d)\r\n", chip_reg_data_version, m_reg_data[SAIN_DATA_VERSION_REG].reg_val);
			for (i = 0; i < MAX_REG_COUNT; i++) {
				if (m_reg_data[i].valid == 1) {
					if (ts_write_reg(touch_dev->client, (u16)i, (u16)(m_reg_data[i].reg_val)) != I2C_SUCCESS)
						goto fail_init;
					if (i == SAIN_TOTAL_NUMBER_OF_X || i == SAIN_TOTAL_NUMBER_OF_Y || i == SAIN_AFE_FREQUENCY)
						mdelay(50);
					if (ts_read_data(touch_dev->client, (u16)i, (u8 *)&stmp, 2) < 0)
						goto fail_init;
					if (memcmp((char *)&m_reg_data[i].reg_val, (char *)&stmp, 2) != 0)
						printk(KERN_WARNING "register data is different. (addr = 0x%02X , %d != %d)\r\n", i, m_reg_data[i].reg_val, stmp);
				}
			}
			printk(KERN_INFO "done new reg data( %d < %d)\r\n", chip_reg_data_version, m_reg_data[SAIN_DATA_VERSION_REG].reg_val);
			if (ts_write_cmd(touch_dev->client, SAIN_SAVE_STATUS_CMD) != I2C_SUCCESS)
				goto fail_init;
			mdelay(500);	/* for fusing eeprom */
		}
		if (ts_read_data(touch_dev->client, SAIN_EEPROM_INFO_REG, (u8 *)&chip_eeprom_info, 2) < 0)
			goto fail_init;
		printk(KERN_INFO "touch eeprom info = 0x%04X\r\n", chip_eeprom_info);

		if (sain_bit_test(chip_eeprom_info, 0)) {	/* hw calibration bit */
			/* h/w calibration */
			if (ts_write_reg(touch_dev->client, SAIN_TOUCH_MODE, 0x07) != I2C_SUCCESS)
				goto fail_init;
			if (ts_write_cmd(touch_dev->client, SAIN_CALIBRATE_CMD) != I2C_SUCCESS)
				goto fail_init;
			if (ts_write_cmd(touch_dev->client, SAIN_REST_CMD) != I2C_SUCCESS)
				goto fail_init;

			/* wait for h/w calibration */
			do {
				mdelay(1000);
				if (ts_read_data(touch_dev->client, SAIN_EEPROM_INFO_REG, (u8 *)&chip_eeprom_info, 2) < 0)
					goto fail_init;
				printk(KERN_INFO "touch eeprom info = 0x%04X\r\n", chip_eeprom_info);
				if (!sain_bit_test(chip_eeprom_info, 0))
					break;
			} while (1);

			if (ts_write_reg(touch_dev->client, SAIN_TOUCH_MODE, 0x00) != I2C_SUCCESS)
				goto fail_init;
			if (ts_write_cmd(touch_dev->client, SAIN_REST_CMD) != I2C_SUCCESS)
				goto fail_init;
			if (ts_write_cmd(touch_dev->client, SAIN_SAVE_STATUS_CMD) != I2C_SUCCESS)
				goto fail_init;
			mdelay(500);
			if (ts_write_cmd(touch_dev->client, SAIN_REST_CMD) != I2C_SUCCESS)
				goto fail_init;
		}
	}

	touch_dev->cap_info.chip_revision = (u16)chip_revision;
	touch_dev->cap_info.chip_firmware_version = (u16)chip_firmware_version;
	touch_dev->cap_info.chip_reg_data_version = (u16)chip_reg_data_version;

	/* initialize */
	if (ts_write_reg(touch_dev->client, SAIN_X_RESOLUTION, (u16)(SetMaxX)) != I2C_SUCCESS)
		goto fail_init;
	if (ts_write_reg(touch_dev->client, SAIN_Y_RESOLUTION, (u16)(SetMaxY)) != I2C_SUCCESS)
		goto fail_init;

	if (ts_read_data(touch_dev->client, SAIN_X_RESOLUTION, (u8 *)&CurMaxX, 2) < 0)
		goto fail_init;
	printk(KERN_INFO "touch max x = %d\r\n", CurMaxX);
	if (ts_read_data(touch_dev->client, SAIN_Y_RESOLUTION, (u8 *)&CurMaxY, 2) < 0)
		goto fail_init;
	printk(KERN_INFO "touch max y = %d\r\n", CurMaxY);

	touch_dev->cap_info.MinX = (u32)0;
	touch_dev->cap_info.MinY = (u32)0;
	touch_dev->cap_info.MaxX = (u32)CurMaxX;
	touch_dev->cap_info.MaxY = (u32)CurMaxY;

	if (touch_dev->cap_info.chip_revision >= 0x0a && touch_dev->cap_info.chip_firmware_version >= 0x75) {
		if (ts_write_reg(touch_dev->client, SAIN_SUPPORTED_FINGER_NUM, (u16)SupportedFingerNum) != I2C_SUCCESS)
			goto fail_init;
		if (ts_read_data(touch_dev->client, SAIN_SUPPORTED_FINGER_NUM, (u8 *)&SupportedFingerNum, 2) < 0)
			goto fail_init;
		printk(KERN_INFO "supported finger num = %d\r\n", SupportedFingerNum);
	}

	touch_dev->cap_info.gesture_support = 0;
	touch_dev->cap_info.multi_fingers = SupportedFingerNum;

	printk(KERN_INFO "set other configuration\r\n");

	reg_val = TOUCH_MODE;
	if (ts_write_reg(touch_dev->client, SAIN_TOUCH_MODE, reg_val) != I2C_SUCCESS)
		goto fail_init;

	/* soft calibration */
	if (ts_write_cmd(touch_dev->client, SAIN_CALIBRATE_CMD) != I2C_SUCCESS)
		goto fail_init;

	reg_val = 0;
	sain_bit_set(reg_val, BIT_PT_CNT_CHANGE);
	sain_bit_set(reg_val, BIT_DOWN);
	sain_bit_set(reg_val, BIT_MOVE);
	sain_bit_set(reg_val, BIT_UP);

	if (SUPPORTED_BUTTON_NUM > 0)
		sain_bit_set(reg_val, BIT_ICON_EVENT);

	touch_dev->chip_int_mask = reg_val;

	if (ts_write_reg(touch_dev->client, SAIN_INT_ENABLE_FLAG, touch_dev->chip_int_mask) != I2C_SUCCESS)
		goto fail_init;

	/* read garbage data */
	for (i = 0; i < 10; i++) {
		ts_write_cmd(touch_dev->client, SAIN_CLEAR_INT_STATUS_CMD);
	}

#if	SAIN_ESD_TIMER_INTERVAL
	if (touch_dev->cap_info.chip_revision >= 0x0a && touch_dev->cap_info.chip_firmware_version >= 0x69) {
		if (ts_write_reg(touch_dev->client, SAIN_PERIODICAL_INTERRUPT_INTERVAL, SAIN_SCAN_RATE_HZ*SAIN_ESD_TIMER_INTERVAL) != I2C_SUCCESS)
			goto fail_init;
		sema_init(&touch_dev->esd_lock, 1);
		touch_dev->use_esd_timer = 1;
		ts_esd_timer_init(touch_dev);
		ts_esd_timer_start(SAIN_ESD_TIMER_INTERVAL*3, touch_dev);
	} else
		touch_dev->use_esd_timer = 0;
#endif
	printk(KERN_INFO "successfully initialized\r\n");
	return true;

fail_init:
	printk(KERN_INFO "failed initiallize\r\n");
	ts_write_cmd(touch_dev->client, SAIN_REST_CMD);
	return false;

}


#if (TOUCH_MODE == 1)
static void	sain_report_data(sain_touch_dev *touch_dev, int id)
{
	int i;
	u32 x, y, width;
	u32 tmp;

	if (id >= SUPPORTED_FINGER_NUM || id < 0) {
		return;
	}

	x = touch_dev->touch_info.coord[id].x;
	y = touch_dev->touch_info.coord[id].y;

	 /* transformation from touch to screen orientation */
	if (touch_dev->cap_info.Orientation & TOUCH_V_FLIP) {
		y = touch_dev->cap_info.MaxY + touch_dev->cap_info.MinY - y;
	}
	if (touch_dev->cap_info.Orientation & TOUCH_H_FLIP) {
		x = touch_dev->cap_info.MaxX + touch_dev->cap_info.MinX - x;
	}
	if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP) {
		swap_v(x, y, tmp);
	}
	touch_dev->reported_touch_info.coord[id].x = x;
	touch_dev->reported_touch_info.coord[id].y = y;
	touch_dev->reported_touch_info.coord[id].width = touch_dev->touch_info.coord[id].width;
	touch_dev->reported_touch_info.coord[id].sub_status = touch_dev->touch_info.coord[id].sub_status;

	for (i = 0; i < SUPPORTED_FINGER_NUM; i++) {
		if (sain_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST)
			|| sain_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_DOWN)
			|| sain_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_MOVE)) {

			if (touch_dev->reported_touch_info.coord[i].width == 0)
				touch_dev->reported_touch_info.coord[i].width = 5;
			input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, (u32)touch_dev->reported_touch_info.coord[i].width);
			input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, (u32)touch_dev->reported_touch_info.coord[i].width);
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
			input_mt_sync(touch_dev->input_dev);
		} else if (sain_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_UP)) {
			input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
			input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
			input_mt_sync(touch_dev->input_dev);
			touch_dev->reported_touch_info.coord[i].sub_status = 0;
		} else
			touch_dev->reported_touch_info.coord[i].sub_status = 0;
	}

	input_sync(touch_dev->input_dev);
}
#endif

#if USE_THREAD_METHOD
static int sain_touch_thread(void *dev_data)
#else
static void sain_touch_work(struct work_struct *work)
#endif
{
	bool read_coord_continued;
	int i;
	u32 x, y, width;
	u32 tmp;
	u8 reported = false;
#if USE_THREAD_METHOD
	sain_touch_dev *touch_dev = (sain_touch_dev *)dev_data;
#else
	sain_touch_dev *touch_dev = container_of(work, sain_touch_dev, work);
#endif

	struct task_struct *tsk = current;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };
	struct sain_touch_platform_data *pdata;

	pdata = touch_dev->client->dev.platform_data;

	sched_setscheduler(tsk, SCHED_FIFO, &param);

	printk(KERN_INFO "touch thread started.. \r\n");

#if USE_THREAD_METHOD
	for (;;) {
		down(&touch_dev->update_lock);
#endif

		sain_debug_msg("sain_touch_thread : semaphore signalled\r\n");

#if	SAIN_ESD_TIMER_INTERVAL
		if (touch_dev->use_esd_timer) {
			if (touch_dev->p_esd_timeout_tmr  !=  NULL) {
				if (timer_pending(touch_dev->p_esd_timeout_tmr) == 0)
					goto continue_read_samples;
				else
					ts_esd_timer_modify(SAIN_ESD_TIMER_INTERVAL*3, touch_dev);
			} else
				ts_esd_timer_start(SAIN_ESD_TIMER_INTERVAL*3, touch_dev);
			/* down(&touch_dev->esd_lock); */
			/* ts_esd_timer_stop(touch_dev); */
		}
#endif
		read_coord_continued = true;
		do {
			if (ts_read_coord(touch_dev) == false) {
				printk(KERN_WARNING "couldn't read touch_dev sample\r\n");
				goto continue_read_samples;
			}

#if (TOUCH_MODE == 1)
			if (touch_dev->touch_info.status == 0x0)
				goto continue_read_samples;
#else
			if (touch_dev->touch_info.status == 0x0 && touch_dev->touch_info.finger_cnt == 100)
				goto continue_read_samples;
#endif
			reported = false;

			if (sain_bit_test(touch_dev->touch_info.status, BIT_ICON_EVENT)) {
				for (i = 0; i < SUPPORTED_BUTTON_NUM; i++)	{
					if (sain_bit_test(touch_dev->icon_event_reg, (BIT_O_ICON0_DOWN+i))) {
						touch_dev->button[i] = ICON_BUTTON_DOWN;
						input_report_key(touch_dev->input_dev, BUTTON_MAPPING_KEY[i], 1);
						reported = true;
						sain_debug_msg("button down = %d\r\n", i);
#ifdef KEY_LED_CONTROL
						if (key_led_status) {
							if (BUTTON_MAPPING_KEY[i] == KEY_MENU) {
								key_led_on(MENU_KEY_LED_OFF);
							} else if (BUTTON_MAPPING_KEY[i] == KEY_BACK){
								key_led_on(BACK_KEY_LED_OFF);
							}
						}
#endif
					}
				}

				for (i = 0; i < SUPPORTED_BUTTON_NUM; i++)	{
					if (sain_bit_test(touch_dev->icon_event_reg, (BIT_O_ICON0_UP+i))) {
						touch_dev->button[i] = ICON_BUTTON_UP;
						input_report_key(touch_dev->input_dev, BUTTON_MAPPING_KEY[i], 0);
						reported = true;
						sain_debug_msg("button up = %d \r\n", i);
#ifdef KEY_LED_CONTROL
						if (key_led_status) {
							key_led_on(MENU_BACK_KEY_LED_ON);
						}
#endif
					}
				}
			}

			if (reported == true) {
#if (TOUCH_MODE == 1)
				for (i = 0; i < SUPPORTED_FINGER_NUM; i++) {
					if (sain_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST)) {
						input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
						input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);
						input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
						input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
						input_mt_sync(touch_dev->input_dev);
					}
					touch_dev->reported_touch_info.coord[i].sub_status = 0;
				}
				input_sync(touch_dev->input_dev);
			}
#else
				for (i = 0; i < SUPPORTED_FINGER_NUM; i++) {
					if (sain_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST)) {
						/* input_report_abs(touch_dev->input_dev,ABS_MT_TRACKING_ID,i); */
						input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
						input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);
						input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
						input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
						input_mt_sync(touch_dev->input_dev);
					}
				}
				memset(&touch_dev->reported_touch_info, 0x0, sizeof(_ts_sain_point_info));
				input_sync(touch_dev->input_dev);
				udelay(100);
				goto continue_read_samples;
			}


			if (touch_dev->touch_info.finger_cnt > SUPPORTED_FINGER_NUM)
				touch_dev->touch_info.finger_cnt = SUPPORTED_FINGER_NUM;

			if (!sain_bit_test(touch_dev->touch_info.status, BIT_PT_EXIST)) {
				for (i = 0; i < SUPPORTED_FINGER_NUM; i++) {
					if (sain_bit_test(touch_dev->reported_touch_info.coord[i].sub_status, SUB_BIT_EXIST)) {
						/* input_report_abs(touch_dev->input_dev,ABS_MT_TRACKING_ID,i); */
						input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
						input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);
						input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
						input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
						input_mt_sync(touch_dev->input_dev);
					}
				}
				memset(&touch_dev->reported_touch_info, 0x0, sizeof(_ts_sain_point_info));
				input_sync(touch_dev->input_dev);
				goto continue_read_samples;
			}

			for (i = 0; i < SUPPORTED_FINGER_NUM; i++) {
				if (sain_bit_test(touch_dev->touch_info.coord[i].sub_status, SUB_BIT_DOWN)
					|| sain_bit_test(touch_dev->touch_info.coord[i].sub_status, SUB_BIT_MOVE)
					|| sain_bit_test(touch_dev->touch_info.coord[i].sub_status, SUB_BIT_EXIST)) {

						x = touch_dev->touch_info.coord[i].x;
						y = touch_dev->touch_info.coord[i].y;

						 /* transformation from touch to screen orientation */
						if (touch_dev->cap_info.Orientation & TOUCH_V_FLIP) {
							y = touch_dev->cap_info.MaxY + touch_dev->cap_info.MinY - y;
						}
						if (touch_dev->cap_info.Orientation & TOUCH_H_FLIP) {
							x = touch_dev->cap_info.MaxX + touch_dev->cap_info.MinX - x;
						}
						if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP) {
							swap_v(x, y, tmp);
						}
						touch_dev->touch_info.coord[i].x = x;
						touch_dev->touch_info.coord[i].y = y;

						printk(KERN_INFO"finger [%02d] x = %d, y = %d \r\n", i, x, y);

						/* input_report_abs(touch_dev->input_dev,ABS_MT_TRACKING_ID,i); */
						if (touch_dev->touch_info.coord[i].width == 0)
							touch_dev->touch_info.coord[i].width = 5;
						input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, (u32)touch_dev->touch_info.coord[i].width);
						input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, (u32)touch_dev->touch_info.coord[i].width);
						input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, x);
						input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, y);
						input_mt_sync(touch_dev->input_dev);

				} else if (sain_bit_test(touch_dev->touch_info.coord[i].sub_status, SUB_BIT_UP)) {
					sain_debug_msg("finger [%02d] up \r\n", i);
					memset(&touch_dev->touch_info.coord[i], 0x0, sizeof(_ts_sain_coord));
					/* input_report_abs(touch_dev->input_dev, ABS_MT_TRACKING_ID,i); */
					input_report_abs(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0);
					input_report_abs(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0);
					input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->reported_touch_info.coord[i].x);
					input_report_abs(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->reported_touch_info.coord[i].y);
					input_mt_sync(touch_dev->input_dev);
				}

				else
					memset(&touch_dev->touch_info.coord[i], 0x0, sizeof(_ts_sain_coord));
			}
			memcpy((char *)&touch_dev->reported_touch_info, (char *)&touch_dev->touch_info, sizeof(_ts_sain_point_info));
			input_sync(touch_dev->input_dev);
#endif
		continue_read_samples:
			if (gpio_get_value(touch_dev->int_gpio_num)) {
				read_coord_continued = false;
				enable_irq(touch_dev->client->irq);
#if	SAIN_ESD_TIMER_INTERVAL
/*				if (touch_dev->use_esd_timer)
 *				{
 *				ts_esd_timer_start(SAIN_ESD_TIMER_INTERVAL*3, touch_dev);
 *				up(&touch_dev->esd_lock);
 *				}
*/
#endif
			} else {
				sain_debug_msg("interrupt pin is still low, so continue read \r\n");
			}
		} while (read_coord_continued);
#if USE_THREAD_METHOD
	}
    return 0;
#endif
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void sain_late_resume(struct early_suspend *h)
{
	u16 reg_val;

	sain_touch_dev *touch_dev;
	touch_dev = container_of(h, sain_touch_dev, early_suspend);
	if (touch_dev == NULL)
		return;

	ts_write_cmd(touch_dev->client, SAIN_WAKEUP_CMD);
	mdelay(10);

	if (ts_mini_init_touch(touch_dev) == false)
		goto fail_resume;
	printk(KERN_INFO "sain_late_resume\n");
	enable_irq(touch_dev->client->irq);
	return;
fail_resume:
	printk(KERN_ERR "failed to wakeup\n");
	enable_irq(touch_dev->client->irq);
	return;
}


static void sain_early_suspend(struct early_suspend *h)
{
	int ret;
	sain_touch_dev *touch_dev;
	touch_dev = container_of(h, sain_touch_dev, early_suspend);
	if (touch_dev == NULL)
		return;

	printk(KERN_INFO "sain_early_suspend\n");
	disable_irq(touch_dev->client->irq);

#if	SAIN_ESD_TIMER_INTERVAL
	if (touch_dev->use_esd_timer) {
		ts_esd_timer_stop(touch_dev);
	}
#endif

#if (!USE_THREAD_METHOD)
	cancel_work_sync(&touch_dev->work);
#endif

	ts_write_reg(touch_dev->client, SAIN_INT_ENABLE_FLAG, 0x0);
	udelay(100);
	ts_write_cmd(touch_dev->client, SAIN_CLEAR_INT_STATUS_CMD);
	if (ts_write_cmd(touch_dev->client, SAIN_SLEEP_CMD) != I2C_SUCCESS) {
		printk(KERN_ERR "failed to enter into sleep mode\n");
		return;
	}
	return;
}
#endif

#ifdef KEY_LED_CONTROL
static struct sain_touch_platform_data *s_sain_platform;
static void key_led_on(u32 val)
{
	/* val > 3 : called from platform through key_led_store() */
	/* val <= 3 : called from touch driver directly */
	if (val > 3) {
		gpio_direction_output(s_sain_platform->led_ldo_en1, 1);
		gpio_direction_output(s_sain_platform->led_ldo_en2, 1);
	} else if (val <= 3) {
		if (val & MENU_KEY_LED_MASK) {
			gpio_direction_output(s_sain_platform->led_ldo_en1, 1);
		} else {
			gpio_direction_output(s_sain_platform->led_ldo_en1, 0);
		}
		if (val & BACK_KEY_LED_MASK) {
			gpio_direction_output(s_sain_platform->led_ldo_en2, 1);
		} else {
			gpio_direction_output(s_sain_platform->led_ldo_en2, 0);
		}
	} else {
		gpio_direction_output(s_sain_platform->led_ldo_en1, 0);
		gpio_direction_output(s_sain_platform->led_ldo_en2, 0);
	}
}

static ssize_t key_led_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	u32 i = 0;
	if (sscanf(buf, "%d", &i) != 1) {
		pr_err("[TSP] keyled write error\n");
	}
	key_led_on(i);
	pr_info("[TSP] Called value by HAL = %d\n", i);
	if (i) key_led_status = true;
	else key_led_status = false;
	
	return size;
}

static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR, NULL, key_led_store);
#endif


#ifdef	SAIN_TSP_FACTORY_TEST
static ssize_t set_tsp_firm_status_show(struct device *dev,
				struct device_attribute *attr,
                                const char *buf)
{
	int count;
	
	printk(KERN_INFO "Enter firmware_status_show by Factory command \n");
	
	if (sain_firm_status_data == 1) {
		count = sprintf(buf,"Downloading\n");
	} else if (sain_firm_status_data == 2) {
		count = sprintf(buf,"PASS\n");
	} else if (sain_firm_status_data == 3) {
		count = sprintf(buf,"FAIL\n");
	} else count = sprintf(buf,"PASS\n");
	
	return count;
	
}

static ssize_t set_tsp_update_show(struct device *dev,
				struct device_attribute *attr,
                                const char *buf, size_t count)
{
	int ret = 0;
	sain_touch_dev *touch_dev = dev_get_drvdata(dev);
#if 0	
	char *filename_ptr;
	const struct firmware *fw = NULL;

	filename_ptr = kzalloc(count*2, GFP_KERNEL);

	if (!filename_ptr) {
		dev_err(dev, "cannot alloc memory\n");
		return -EINVAL;
	}

	ret = sscanf(buf, "%s", filename_ptr);
	if (ret != 1) {
		dev_err(dev, "invalid parameter\n");
		goto fail_fw;
	}

	request_firmware(fw, filename_ptr, dev);
	ret = request_firmware(&fw, filename_ptr, dev);
	if (ret) {
		dev_err(dev, "cannot open firmware %s\n", filename_ptr);
		goto fail_fw;
	}

	if(fw->size != touch_dev->cap_info.chip_fw_size)
	{
		dev_err(dev, "invalid file size( %d != %d)\n", fw->size , touch_dev->cap_info.chip_fw_size);
		goto fail_upgrade_fw;
	}
#endif

	disable_irq(touch_dev->client->irq);
#if	SAIN_ESD_TIMER_INTERVAL	
	if (touch_dev->use_esd_timer) {
		ts_esd_timer_stop(touch_dev);
	}
#endif					
	printk(KERN_INFO "update firmware\n");
	sain_firm_status_data=1;	//start firmware updating

	/*ts_upgrade_firmware(touch_dev, fw, fw->size); */
	if (ts_upgrade_firmware(touch_dev, &m_firmware_data[2],16*1024)) {
		sain_firm_status_data=2;	/* firmware update success */
		printk(KERN_INFO "[TSP]Reprogram done : Firmware update Success~~~~~~~~~~\n");	
	} else {
		sain_firm_status_data=3;	/* firmware update Fail 	 */
		printk(KERN_INFO "[TSP]Reprogram done : Firmware update Fail ~~~~~~~~~~\n");	
		goto fail_upgrade_fw;
	};
	printk(KERN_INFO "reset touch chip\n");	
	/* must h/w reset (cold reset) and mdelay(500) */
	regulator_disable(touch_dev->pdata->reg_vdd);
	regulator_disable(touch_dev->pdata->reg_avdd);
	mdelay(10); 
	regulator_enable(touch_dev->pdata->reg_vdd);
	regulator_enable(touch_dev->pdata->reg_avdd);
	mdelay(600);

	printk(KERN_INFO "initialize touch chip, do not touch.\n");	
	ts_init_touch(touch_dev);
	enable_irq(touch_dev->client->irq);
	/* release_firmware(fw); */
	printk(KERN_INFO "finished update.\n");	
	return count;	
fail_upgrade_fw:
	/* release_firmware(fw);	 */
fail_fw:
	/* kfree(filename_ptr); */
	return -1;

}

/*Current(TSP IC Firmware version) Version*/
static ssize_t set_tsp_firm_version_read_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int error, cnt;
	u8 chip_reg_data_version=0;
	sain_touch_dev* touch_dev = dev_get_drvdata(dev);

	for (cnt=10; cnt>0; cnt--) {
		error = ts_read_data(touch_dev->client, SAIN_DATA_VERSION_REG, (u8*)&chip_reg_data_version, 2);
		if(error < 0) {
			printk(KERN_ERR "[TSP] touch version read fail it will try 2s later");		
			msleep(20);
		} else {
			break;
		}
	}
	if (cnt==0) {
		printk(KERN_ERR "[TSP] set_mxt_firm_version_show failed!!!");
		chip_reg_data_version = 0;
	}
		
	printk(KERN_INFO "[TSP] IC Firmware version [%d]\n",chip_reg_data_version);

	return sprintf(buf, "%02d\n", chip_reg_data_version);  

}

/*Last(Phone) Version*/
static ssize_t set_tsp_firm_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	sain_touch_dev *touch_dev = dev_get_drvdata(dev);
	printk(KERN_INFO "[TSP] Phone firmware version is [%d]\n", touch_dev->cap_info.chip_reg_data_version);
	return sprintf(buf, "%02d\n", touch_dev->cap_info.chip_reg_data_version );    
}

/* touch key threshold */
static ssize_t key_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	printk(KERN_INFO "[TSP] threshold not support\n");
	return sprintf(buf, "%d\n", 0);
}

static ssize_t key_threshold_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	printk(KERN_INFO "[TSP] threshold not support\n");

	return size;
}


static DEVICE_ATTR(set_mxt_firm_status, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_tsp_firm_status_show, NULL);	/* TSP Firmware updating status */
static DEVICE_ATTR(set_mxt_update, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_tsp_update_show, NULL);	/* TSP Firmware update */
static DEVICE_ATTR(set_mxt_firm_version, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_tsp_firm_version_show, NULL);/* PHONE Firmware version*/
static DEVICE_ATTR(set_mxt_firm_version_read, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_tsp_firm_version_read_show, NULL);/*TSP IC Firmware version*/
static DEVICE_ATTR(key_threshold, S_IRUGO | S_IWUSR, key_threshold_show, key_threshold_store);	/* TSK Firmware update */


static struct attribute *sainTouch_attributes[] = {
	&dev_attr_set_mxt_firm_status.attr,
	&dev_attr_set_mxt_update.attr,
	&dev_attr_set_mxt_firm_version.attr,
	&dev_attr_set_mxt_firm_version_read.attr,
	&dev_attr_key_threshold.attr,
	NULL,
};

static struct attribute_group sainTouch_attr_group = {
	.attrs = sainTouch_attributes,
};


/*
*	static struct attribute *sain_attrs[] = {
*	         &dev_fw_manager.attr,
*		NULL
*	};
*	
*	static const struct attribute_group sain_attr_group = 
*	{
*		.attrs = sain_attrs,
*	};
*/
#endif	/* SAIN_TSP_FACTORY_TEST */

static int sain_touch_probe(struct i2c_client *client, const struct i2c_device_id *i2c_id)
{
	int ret;
	sain_touch_dev *touch_dev;
	struct sain_touch_platform_data *pdata;
	int i;
#ifdef KEY_LED_CONTROL
		struct class *leds_class;
		struct device *led_dev;
#endif

	sain_debug_msg("sain_touch_probe+ \r\n");

#if	0	//def SAIN_TSP_FACTORY_TEST
	if (device_create_file(&client->dev, &dev_attr_set_mxt_firm_status) < 0) {
		printk("Failed to create device file!(%s)!\n", dev_attr_set_mxt_firm_status.attr.name);
		ret = -1;
	}
	if (device_create_file(&client->dev, &dev_attr_set_mxt_update) < 0) {
		printk("Failed to create device file!(%s)!\n", dev_attr_set_mxt_update.attr.name);
		ret = -1;
	}		
	if (device_create_file(&client->dev, &dev_attr_set_mxt_firm_version) < 0) {
		printk("Failed to create device file!(%s)!\n", dev_attr_set_mxt_firm_version.attr.name);
		ret = -1;
	}	
	if (device_create_file(&client->dev, &dev_attr_set_mxt_firm_version_read) < 0) {
		printk("Failed to create device file!(%s)!\n", dev_attr_set_mxt_firm_version_read.attr.name);
		ret = -1;
	}			
	if (device_create_file(&client->dev, &dev_attr_key_threshold) < 0) {
		printk("Failed to create device file!(%s)!\n", dev_attr_key_threshold.attr.name);
		ret = -1;
	}	
#endif	/* SAIN_TSP_FACTORY_TEST */

	sain_debug_msg("touch data alloc \r\n");
	touch_dev = kzalloc(sizeof(sain_touch_dev), GFP_KERNEL);

	if (!touch_dev) {
		printk(KERN_ERR "unabled to allocate touch data \r\n");
		ret = -ENOMEM;
		goto err_alloc_dev_data;
	}

	touch_dev->client = client;

	pdata = client->dev.platform_data;

	if (pdata == NULL) {
		pr_err("%s : sain_touch probe fail\n", __func__);
		return  -ENODEV;
	}
	touch_dev->pdata = pdata;

	/* sain regulator config */
	touch_dev->pdata->reg_avdd = regulator_get(NULL, touch_dev->pdata->reg_vdd_name);

	if (IS_ERR(touch_dev->pdata->reg_avdd)) {
		ret = PTR_ERR(touch_dev->pdata->reg_avdd);
		pr_err("[%s: %s]unable to get regulator %s: %d\n", __func__, touch_dev->pdata->platform_name,  touch_dev->pdata->reg_vdd_name, ret);
	}

	touch_dev->pdata->reg_vdd = regulator_get(NULL, touch_dev->pdata->reg_avdd_name);

	if (IS_ERR(touch_dev->pdata->reg_vdd)) {
		ret = PTR_ERR(touch_dev->pdata->reg_vdd);
		pr_err("[%s: %s]unable to get regulator %s: %d\n", __func__, touch_dev->pdata->platform_name, touch_dev->pdata->reg_avdd_name, ret);
	}
	
	touch_dev->pdata->reg_vdd_lvsio = regulator_get(NULL, touch_dev->pdata->reg_vdd_lvsio_name);

	if (IS_ERR(touch_dev->pdata->reg_vdd_lvsio)) {
		ret = PTR_ERR(touch_dev->pdata->reg_vdd);
		pr_err("[%s: %s]unable to get regulator %s: %d\n", __func__, touch_dev->pdata->platform_name, touch_dev->pdata->reg_vdd_lvsio_name, ret);
	}

	/* TSP Power on */
	ret = regulator_enable(touch_dev->pdata->reg_vdd);
	regulator_set_voltage(touch_dev->pdata->reg_vdd,
		touch_dev->pdata->reg_vdd_level, touch_dev->pdata->reg_vdd_level);
	pr_info("enable %s: ret=%d\n", touch_dev->pdata->reg_vdd_name, ret);

	ret = regulator_enable(touch_dev->pdata->reg_avdd);
	regulator_set_voltage(touch_dev->pdata->reg_avdd,
		touch_dev->pdata->reg_avdd_level, touch_dev->pdata->reg_avdd_level);
	pr_info("enable %s: ret=%d\n", touch_dev->pdata->reg_avdd_name, ret);
	
	ret = regulator_enable(touch_dev->pdata->reg_vdd_lvsio);
	regulator_set_voltage(touch_dev->pdata->reg_vdd_lvsio,
		touch_dev->pdata->reg_vdd_lvsio_level, touch_dev->pdata->reg_vdd_lvsio_level);
	pr_info("enable %s: ret=%d\n", touch_dev->pdata->reg_vdd_lvsio_name, ret);

	msleep(700);

	i2c_set_clientdata(client, touch_dev);

#if USE_THREAD_METHOD
	sema_init(&touch_dev->update_lock, 0);
#else
	INIT_WORK(&touch_dev->work, sain_touch_work);
#endif

	if (I2C_SMBUS_BLOCK_MAX < sizeof(_ts_sain_point_info)) {
		printk(KERN_WARNING "max size error : i2c max size = %d, sain packet size = %d \r\n", I2C_SMBUS_BLOCK_MAX, sizeof(_ts_sain_point_info));
		printk(KERN_WARNING "must modify I2C_SMBUS_BLOCK_MAX field in include/linux/i2c.h\r\n");
	}

#if USE_THREAD_METHOD
	sain_debug_msg("touch thread create \r\n");
	touch_dev->task = kthread_create(sain_touch_thread, touch_dev, "sain_touch_thread");
	if (touch_dev->task == NULL) {
		printk(KERN_ERR "unabled to create touch thread \r\n");
		ret = -1;
		goto err_kthread_create_failed;
	}
#else
	touch_dev->sain_workqueue = create_singlethread_workqueue("sain_workqueue");
	if (!touch_dev->sain_workqueue) {
		printk(KERN_ERR "unabled to create touch thread \r\n");
		ret = -1;
		goto err_kthread_create_failed;
	}
#endif

	sain_debug_msg("allocate input device \r\n");
	touch_dev->input_dev = input_allocate_device();
	if (touch_dev->input_dev == 0) {
		printk(KERN_ERR "unabled to allocate input device \r\n");
		ret = -ENOMEM;
		goto err_input_allocate_device;
	}

	touch_dev->int_gpio_num = pdata->irq_gpio;
	sain_debug_msg("request irq (pin = %d) \r\n", touch_dev->int_gpio_num);

	touch_dev->client->irq = gpio_to_irq(touch_dev->int_gpio_num);

	memset(&touch_dev->reported_touch_info, 0x0, sizeof(_ts_sain_point_info));
	if (ts_init_touch(touch_dev) == false)
		goto err_input_allocate_device;

	touch_dev->input_dev->name = "sec_touchscreen";
	touch_dev->input_dev->id.bustype = BUS_I2C;
	touch_dev->input_dev->id.vendor = 0x0001;

	touch_dev->input_dev->id.product = 0x0002;
	touch_dev->input_dev->id.version = 0x0100;
	touch_dev->input_dev->dev.parent = &client->dev;

	set_bit(EV_SYN, touch_dev->input_dev->evbit);
	set_bit(EV_KEY, touch_dev->input_dev->evbit);
	set_bit(BTN_TOUCH, touch_dev->input_dev->keybit);
	set_bit(EV_ABS, touch_dev->input_dev->evbit);

	if (SUPPORTED_BUTTON_NUM > 0) {
		for (i = 0; i < SUPPORTED_BUTTON_NUM; i++)
			set_bit(BUTTON_MAPPING_KEY[i], touch_dev->input_dev->keybit);
	}

	if (touch_dev->cap_info.Orientation & TOUCH_XY_SWAP) {
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->cap_info.MinX, touch_dev->cap_info.MaxX, 0, 0);
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->cap_info.MinY, touch_dev->cap_info.MaxY, 0, 0);
	} else {
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_X, touch_dev->cap_info.MinX, touch_dev->cap_info.MaxX, 0, 0);
		input_set_abs_params(touch_dev->input_dev, ABS_MT_POSITION_Y, touch_dev->cap_info.MinY, touch_dev->cap_info.MaxY, 0, 0);
	}

	input_set_abs_params(touch_dev->input_dev, ABS_TOOL_WIDTH, 0, 255, 0, 0);
	input_set_abs_params(touch_dev->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(touch_dev->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);

	sain_debug_msg("register %s input device \r\n", touch_dev->input_dev->name);
	ret = input_register_device(touch_dev->input_dev);
	if (ret) {
		printk(KERN_ERR "unable to register %s input device\r\n", touch_dev->input_dev->name);
		goto err_input_register_device;
	}

	if (touch_dev->client->irq) {
		ret = request_irq(touch_dev->client->irq, ts_int_handler, IRQF_TRIGGER_LOW|IRQF_DISABLED, SAIN_DRIVER_NAME, touch_dev);
		if (ret) {
			printk(KERN_ERR "unable to register irq.(%s)\r\n", touch_dev->input_dev->name);
			goto err_request_irq;
		}
	}
	touch_dev->irq = touch_dev->client->irq;

	dev_info(&client->dev, "sain touch probe.\r\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	touch_dev->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	touch_dev->early_suspend.suspend = sain_early_suspend;
	touch_dev->early_suspend.resume = sain_late_resume;
	register_early_suspend(&touch_dev->early_suspend);
#endif

#ifdef KEY_LED_CONTROL
	s_sain_platform = pdata;

	/* create sysfs */
	leds_class = class_create(THIS_MODULE, "leds");
	if (IS_ERR(leds_class)) {
		return PTR_ERR(leds_class);
	}
	led_dev = device_create(leds_class, NULL, 0, NULL, "button-backlight");
	if (device_create_file(led_dev, &dev_attr_brightness) < 0) {
		pr_err("Failed to create device file(%s)!\n", dev_attr_brightness.attr.name);
	}

	/* TouchKey LED gpio */
	pr_info("touch key led[1] GPIO = %d\n", touch_dev->pdata->led_ldo_en1);
	pr_info("touch key led[2] GPIO = %d\n", touch_dev->pdata->led_ldo_en2);
#endif

#ifdef SAIN_TSP_FACTORY_TEST
	ret = sysfs_create_group(&touch_dev->client->dev.kobj, &sainTouch_attr_group);
	if (ret)
		goto err_remove_attr_group;
#endif

#if USE_THREAD_METHOD
	wake_up_process(touch_dev->task);
#endif
	return 0;
err_remove_attr_group:
	sysfs_remove_group(&touch_dev->client->dev.kobj, &sainTouch_attr_group);

err_request_irq:
	input_unregister_device(touch_dev->input_dev);
err_input_register_device:
	input_free_device(touch_dev->input_dev);
err_kthread_create_failed:
err_input_allocate_device:
	kfree(touch_dev);
err_alloc_dev_data:
err_create_sysfs:
	regulator_disable(touch_dev->pdata->reg_vdd);
	regulator_put(touch_dev->pdata->reg_vdd);
err_get_regulator2:
	regulator_disable(touch_dev->pdata->reg_avdd);
	regulator_disable(touch_dev->pdata->reg_vdd_lvsio);
	regulator_put(touch_dev->pdata->reg_avdd);
	regulator_put(touch_dev->pdata->reg_vdd_lvsio);

	return ret;
}


static int sain_touch_remove(struct i2c_client *client)
{
	sain_touch_dev *touch_dev = i2c_get_clientdata(client);

	sain_debug_msg("sain_touch_remove+ \r\n");
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&touch_dev->early_suspend);
#endif
	if (touch_dev->client->irq) {
		free_irq(touch_dev->client->irq, touch_dev);
	}

	input_unregister_device(touch_dev->input_dev);
	input_free_device(touch_dev->input_dev);

	regulator_put(touch_dev->pdata->reg_vdd);
	regulator_put(touch_dev->pdata->reg_avdd);

	kfree(touch_dev);

	return 0;
}

static int __devinit sain_touch_init(void)
{
	i2c_add_driver(&sain_touch_driver);
	return 0;
}

static void __exit sain_touch_exit(void)
{
	i2c_del_driver(&sain_touch_driver);
}



module_init(sain_touch_init);
module_exit(sain_touch_exit);

MODULE_DESCRIPTION("touch-screen device driver using i2c interface");
MODULE_AUTHOR("sohnet <swjang@sain.co.kr>");
MODULE_LICENSE("GPL");
