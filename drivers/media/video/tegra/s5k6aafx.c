/*
 * s5k6aafx.c - s5k6aafx sensor driver
 *
 * Copyright (C) 2010 Google Inc.
 *
 * Contributors:
 *      Rebecca Schultz Zavin <rebecca@android.com>
 *
 * s5k6aafx.c
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <media/s5k6aafx.h>
#include <media/tegra_camera.h>
#ifdef CONFIG_MACH_BOSE_ATT
#include <s5k6aafx_setting_bose.h>
#else
#include "s5k6aafx_setting.h"
#endif
#include <linux/regulator/consumer.h>
#include <linux/delay.h>

#define DEBUG_PRINTS 0
#if DEBUG_PRINTS
	#define FUNC_ENTR	\
		printk(KERN_INFO "[S5K6AAFX] %s Entered!!!\n", __func__)
	#define FUNC_EXIT	\
		printk(KERN_INFO "[S5K6AAFX] %s Exited!!!\n", __func__)
#else
	#define FUNC_ENTR
	#define FUNC_EXIT
#endif

/*#define CONFIG_LOAD_FILE 1*/

#ifdef CONFIG_LOAD_FILE
#include <linux/vmalloc.h>
#include <linux/mm.h>
/*#define max_size 200000*/

struct test {
	char data;
	struct test *nextBuf;
};

struct test *s5k6aafx_testBuf;

#endif

struct s5k6aafx_info {
	int mode;
	int oprmode;
	int framesize_index;	/* for preview or capture resolution*/
	u16 check_vender;	/* for vender check*/
	u8 geffect_index;	/* for effect*/
	u8 gev_index;		/* for ev*/
	u8 gwb_index;		/* for wb*/
#ifdef CONFIG_MACH_BOSE_ATT
	u16 gLowLight_value;	/* for lowLight value*/
#endif
	struct i2c_client *i2c_client;
	struct s5k6aafx_platform_data *pdata;
};

extern struct class *camera_class;
struct device *s5k6aafx_dev;
extern struct i2c_client *i2c_client_camera;

extern struct regulator *reg_mipi_1v2;

struct s5k6aafx_mode front_mode;
struct s5k6aafx_exif_info front_exif_info;
static struct s5k6aafx_info *info;

/* extern struct i2c_client *i2c_client_pmic; */
#ifdef FACTORY_TEST
static s5k6aafx_dtp_test dtpTest = S5K6AAFX_DTP_TEST_OFF;
#endif

#define S5K6AAFX_MAX_RETRIES 5
#define S5K6AAFX_READ_STATUS_RETRIES 50

enum {
	SYSTEM_INITIALIZE_MODE = 0,
	MONITOR_MODE,
	CAPTURE_MODE,
	RECORD_MODE,
#ifndef CONFIG_MACH_BOSE_ATT
	VT_MODE
#endif
};

enum s5k6aafx_oprmode {
	S5K6AAFX_OPRMODE_VIDEO = 0,
	S5K6AAFX_OPRMODE_IMAGE = 1,
};

enum s5k6aafx_frame_size {
	S5K6AAFX_PREVIEW_VGA = 0,	/* VGA - 640x480 */
#ifndef CONFIG_MACH_BOSE_ATT
	S5K6AAFX_PREVIEW_RECORD_MMS3,	/* MMS3 - 528x432 */
#endif
	S5K6AAFX_PREVIEW_VT,		/* VT - 352x288 */
	S5K6AAFX_PREVIEW_VT_QVGA,	/* QVGA - 320x240 */
	S5K6AAFX_PREVIEW_RECORD_MMS,	/* MMS - 176x144 */
	S5K6AAFX_CAPTURE_1280x960,	/* 1280x960 */
};

struct s5k6aafx_enum_framesize {
	/* mode is 0 for preview, 1 for capture */
	enum s5k6aafx_oprmode mode;
	unsigned int index;
	unsigned int width;
	unsigned int height;
};

static struct s5k6aafx_enum_framesize s5k6aafx_framesize_list[] = {
	{ S5K6AAFX_OPRMODE_VIDEO, S5K6AAFX_PREVIEW_VGA,		640,	480 },
#ifndef CONFIG_MACH_BOSE_ATT
	{ S5K6AAFX_OPRMODE_VIDEO, S5K6AAFX_PREVIEW_RECORD_MMS3,	528,	432 },
#endif
	{ S5K6AAFX_OPRMODE_VIDEO, S5K6AAFX_PREVIEW_VT,		352,	288 },
	{ S5K6AAFX_OPRMODE_VIDEO, S5K6AAFX_PREVIEW_VT_QVGA,	320,	240 },
	{ S5K6AAFX_OPRMODE_VIDEO, S5K6AAFX_PREVIEW_RECORD_MMS,	176,	144 },
	{ S5K6AAFX_OPRMODE_IMAGE, S5K6AAFX_CAPTURE_1280x960,	1280,	960 },
};

static void s5k6aafx_esdreset(struct s5k6aafx_info *info)
{
	FUNC_ENTR;

	info->pdata->power_off();
	info->mode = SYSTEM_INITIALIZE_MODE;
	usleep_range(200000, 300000);
	info->pdata->power_on();

	FUNC_EXIT;
}

static int s5k6aafx_write_reg_8(struct i2c_client *client, u8 addr, u8 val)
{
	int err;
	struct i2c_msg msg;
	unsigned char data[2];
	int retry = 0;

	if (!client->adapter)
		return -ENODEV;

	data[0] = (u8) addr;
	data[1] = (u8) val;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = data;

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;
		retry++;
		pr_err("s5k6aafx(8): i2c transfer failed, retrying %x %x err:%d\n",
		       addr, val, err);
		usleep_range(3000 * retry, 4000 * retry);
	} while (retry <= S5K6AAFX_MAX_RETRIES);

	return err;
}


static int s5k6aafx_write_reg(struct i2c_client *client, u16 addr, u16 val)
{
	int err;
	struct i2c_msg msg;
	unsigned char data[4];
	int retry = 0;

	if (!client->adapter)
		return -ENODEV;

	data[0] = (u8) (addr >> 8);
	data[1] = (u8) (addr & 0xff);
	data[2] = (u8) (val >> 8);
	data[3] = (u8) (val & 0xff);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 4;
	msg.buf = data;

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;
		retry++;
		pr_err("s5k6aafx: i2c transfer failed, retrying %x %x err:%d\n",
		       addr, val, err);
		usleep_range(3000 * retry, 4000 * retry);
	} while (retry <= S5K6AAFX_MAX_RETRIES);

	return err;
}

#define BURST_MODE_BUFFER_MAX_SIZE 2700
unsigned char s5k6aafx_buf_for_burstmode[BURST_MODE_BUFFER_MAX_SIZE];

static int s5k6aafx_sensor_burst_write_list(
		struct i2c_client *client, const struct s5k6aafx_reg table[]
		, int size, struct s5k6aafx_mode *mode)
{
	int err = -EINVAL;
	int i = 0;
	int idx = 0;

	u16 subaddr = 0, next_subaddr = 0;
	u16 value = 0;

	struct i2c_msg msg = {client->addr, 0, 0, s5k6aafx_buf_for_burstmode};

	FUNC_ENTR;

	for (i = 0; i < size; i++) {
		if (idx > (BURST_MODE_BUFFER_MAX_SIZE - 10)) {
			pr_err("BURST MODE buffer overflow!!!\n");
			return err;
		}

		subaddr = table[i].addr;
		value = table[i].val;

		switch (subaddr) {
		case 0x0F12:
			if (idx == 0) {
				s5k6aafx_buf_for_burstmode[idx++] = 0x0F;
				s5k6aafx_buf_for_burstmode[idx++] = 0x12;
			}
			s5k6aafx_buf_for_burstmode[idx++] = value >> 8;
			s5k6aafx_buf_for_burstmode[idx++] = value & 0xFF;

			if (next_subaddr != 0x0F12) {
				msg.len = idx;
				err = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
				idx = 0;
			}
			break;

		case 0xFFFE:
			if (mode->camcordmode == 2)
				if (value >= 100)
					value -= 20;
			usleep_range(value * 1000, (value + 1) * 1000);
			break;

		case 0xFFFF:
			return 0;
			break;

		default:
			idx = 0;
			err = s5k6aafx_write_reg(client, subaddr, value);
			break;
		}
	}

	FUNC_EXIT;

	if (unlikely(err < 0)) {
		pr_err("%s: register set failed\n", __func__);
		return err;
	}
	return 0;

}

int s5k6aafx_write_table(struct i2c_client *client,
			      const struct s5k6aafx_reg table[])
{
	int err;
	const struct s5k6aafx_reg *next;
	u16 val;

	FUNC_ENTR;

	for (next = table; next->addr != S5K6AAFX_TABLE_END; next++) {
		if (next->addr == S5K6AAFX_TABLE_WAIT_MS) {
			usleep_range(next->val * 1000, (next->val + 1) * 1000);
			continue;
		}

		val = next->val;

		err = s5k6aafx_write_reg(client, next->addr, val);
		if (err < 0)
			return err;
	}

	FUNC_EXIT;

	return 0;
}

int s5k6aafx_write_table_8(struct i2c_client *client,
			      const struct s5k6aafx_reg_8 *table)
{
	int err;
	const struct s5k6aafx_reg_8 *next;
	u8 val;

	for (next = table; next->addr != S5K6AAFX_TABLE_END_8; next++) {
		if (next->addr == S5K6AAFX_TABLE_WAIT_MS_8) {
			usleep_range(next->val * 1000, (next->val + 1) * 1000);
			continue;
		}

		val = next->val;

		err = s5k6aafx_write_reg_8(client, next->addr, val);
		if (err)
			return err;
	}
	return 0;
}

static int s5k6aafx_read_reg(struct i2c_client *client
		, u16 val, u8 *r_data, u16 length)
{
	int err;
	int retry = 0;
	struct i2c_msg msg[2];
	unsigned char data[2];

	if (!client->adapter)
		return -ENODEV;

	err = s5k6aafx_write_reg(client, 0x002c, 0x7000);
	if (err) {
		pr_err("s5k6aafx_write_reg: i2c transfer failed, 0x002C 0x7000\n");
		return err;
	}
	err = s5k6aafx_write_reg(client, 0x002e, val);
	if (err) {
		pr_err("s5k6aafx_write_reg: i2c transfer failed, 0x002E\n");
		return err;
	}

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = data;

	/* high byte goes out first */
	data[0] = 0x0f;
	data[1] = 0x12;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = length;
	msg[1].buf = r_data;

	do {
		err = i2c_transfer(client->adapter, msg, 2);
		if (err == 2)
			return 0;
		retry++;
		pr_err("s5k6aafx_read_reg: i2c transfer failed, retrying err:%d\n", err);
		usleep_range(3000 * retry, 4000 * retry);
	} while (retry <= S5K6AAFX_MAX_RETRIES);

	return 0;
}


#ifdef CONFIG_LOAD_FILE
static inline int s5k6aafx_write(struct i2c_client *client,
		unsigned int addr_reg, unsigned int data_reg)
{
	struct i2c_msg msg[1];
	unsigned char buf[4];
	int ret;

	buf[0] = ((addr_reg >> 8) & 0xFF);
	buf[1] = (addr_reg) & 0xFF;
	buf[2] = ((data_reg >> 8) & 0xFF);
	buf[3] = (data_reg) & 0xFF;

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 4;
	msg->buf = buf;

	/* (unsigned long *)buf = cpu_to_be32(packet); */

	ret = i2c_transfer(client->adapter, &msg, 1);

	if (unlikely(ret < 0)) {
		dev_err(&client->dev, "%s: (a:%x,d:%x) write failed\n", __func__, addr_reg, data_reg);
		return ret;
	}

	return (ret != 1) ? -1 : 0;
}

static char *s5k6aafx_regs_table;

static int s5k6aafx_regs_table_size;

static int loadFile(void)
{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int ret;
	mm_segment_t fs = get_fs();

	pr_err("%s %d\n", __func__, __LINE__);

	set_fs(get_ds());
	filp = filp_open("/mnt/sdcard/s5k6aafx_setting.h", O_RDONLY, 0);

	if (IS_ERR(filp)) {
		pr_err("file open error\n");
		return PTR_ERR(filp);
	}

	l = filp->f_path.dentry->d_inode->i_size;
	pr_err("l = %ld\n", l);
	dp = kmalloc(l, GFP_KERNEL);
	if (dp == NULL)	{
		pr_err("Out of Memory\n");
		filp_close(filp, current->files);
	}

	pos = 0;
	memset(dp, 0, l);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);

	if (ret != l) {
		pr_err("Failed to read file ret = %d\n", ret);
		kfree(dp);
		filp_close(filp, current->files);
		return -EINVAL;
	}

	filp_close(filp, current->files);

	set_fs(fs);

	s5k6aafx_regs_table = dp;

	s5k6aafx_regs_table_size = l;

	*((s5k6aafx_regs_table + s5k6aafx_regs_table_size) - 1) = '\0';

	pr_err("s5k6aafx_reg_table_init\n");

	return 0;
}

void s5k6aafx_regs_table_exit(void)
{
	pr_err("%s start\n", __func__);

	if (s5k6aafx_regs_table) {
		kfree(s5k6aafx_regs_table);
		s5k6aafx_regs_table = NULL;
	}

	pr_err("%s done\n", __func__);
}

static int s5k6aafx_write_tuningmode(struct i2c_client *client, char *name)
{
	char *start, *end, *reg;
	unsigned short addr;
	unsigned int value;
	char reg_buf[7], data_buf[7];
	int ret;

	int i = 0;

	FUNC_ENTR;

	*(reg_buf + 6) = '\0';
	*(data_buf + 6) = '\0';

	start = strstr(s5k6aafx_regs_table, name);
	end = strstr(start, "};");

	while (1) {
		/* Find Address */
		reg = strstr(start, "{0x");
		if (reg)
			start = (reg + 17);

		if ((reg == NULL) || (reg > end))
			break;

		/* Write Value to Address */
		if (reg != NULL) {
			memcpy(reg_buf, (reg + 1), 6);
			memcpy(data_buf, (reg + 9), 6);
			addr = (unsigned short)simple_strtoul(reg_buf, NULL, 16);
			value = (unsigned int)simple_strtoul(data_buf, NULL, 16);

			if (addr == 0xFFFE)
				usleep_range(value * 1000, (value + 1) * 1000);
			else if ((addr == 0xFFFF) && (value == 0xFFFF))
				return 0;
			else
				ret = s5k6aafx_write_reg(client, addr, value);
		}
	}

	pr_err("func(%s) s5k6aafx_regs_table_write end!\n", __func__);

	FUNC_EXIT;

	return 0;
}
#endif

#ifdef CONFIG_MACH_BOSE_ATT
static int s5k6aafx_get_LowLightCondition(struct i2c_client *client
		, struct s5k6aafx_info *info)
{
	struct i2c_msg msg[2];
	unsigned char data[2];
	int err = -1;
	u8 r_data[2] = {0, 0};

	FUNC_ENTR;

	err = s5k6aafx_write_reg(client, 0x002C, 0x7000);
	if (err) {
		pr_err("s5k6aafx_write_reg: i2c transfer failed, 0x002C 0x7000\n");
		return err;
	}

	err = s5k6aafx_write_reg(client, 0x002E, 0x1AAA);
	if (err) {
		pr_err("s5k6aafx_write_reg: i2c transfer failed, 0x002E 0x1AAA\n");
		return err;
	}
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = data;

	/* high byte goes out first */
	data[0] = 0x0f;
	data[1] = 0x12;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = r_data;

	err = i2c_transfer(client->adapter, msg, 2);
	if (err != 2) {
		pr_err("s5k6aafx_read_reg: i2c transfer failed, 0x%x\n"
				, (unsigned int) r_data);
		return -EINVAL;
	}

	info->gLowLight_value = ((r_data[0] << 8) | r_data[1]);
	pr_err("func(%s):line(%d) info->gLowLight_value : 0x%x\n"
			, __func__, __LINE__, info->gLowLight_value);

	FUNC_EXIT;

	return err;
}
#endif

static int s5k6aafx_get_framesize_index
	(struct s5k6aafx_info *info, struct s5k6aafx_mode *mode)
{
	int i = -1;
	struct s5k6aafx_enum_framesize *frmsize;
	int previewcapture_ratio = (mode->xres * 10) / mode->yres;

	FUNC_ENTR;

	if (mode->PreviewActive && !mode->StillCount)
		info->oprmode = S5K6AAFX_OPRMODE_VIDEO;
	else if (!mode->PreviewActive && mode->StillCount)
		info->oprmode = S5K6AAFX_OPRMODE_IMAGE;
	else
		info->oprmode = S5K6AAFX_OPRMODE_VIDEO;

	/* Check for video/image mode */
	for (i = 0; i < (sizeof(s5k6aafx_framesize_list) / sizeof(struct s5k6aafx_enum_framesize)); i++) {
		frmsize = &s5k6aafx_framesize_list[i];

		if (frmsize->mode != info->oprmode)
			continue;

		/* In case of image capture mode, if the given image resolution is not supported,
		 * return the next higher image resolution. */
		/* Must search wide*/
		if (frmsize->width == mode->xres
				&& frmsize->height == mode->yres)
			return frmsize->index;
	}

	/* If it fails, return the default value. Front camera is same resolution */
	if (info->oprmode == S5K6AAFX_OPRMODE_VIDEO)
		return (previewcapture_ratio > 15) ? S5K6AAFX_PREVIEW_VGA : S5K6AAFX_PREVIEW_VGA;
	else
		return (previewcapture_ratio > 15) ? S5K6AAFX_CAPTURE_1280x960 : S5K6AAFX_CAPTURE_1280x960;

	pr_err("%s: returned error\n", __func__);

	return 0;
}

static int s5k6aafx_check_vender(struct s5k6aafx_info *info)
{
	int err;
	struct i2c_msg msg[2];
	unsigned char data[2];
	u8 r_data[2] = {0, 0};

	FUNC_ENTR;

	if (!info->i2c_client->adapter)
		return -ENODEV;

	err = s5k6aafx_write_reg(info->i2c_client, 0x002c, 0xD000);
	if (err) {
		pr_err("s5k6aafx_write_reg: i2c transfer failed, 0x002C 0x7000\n");
		return err;
	}
	err = s5k6aafx_write_reg(info->i2c_client, 0x002e, 0x1006);
	if (err) {
		pr_err("s5k6aafx_write_reg: i2c transfer failed, 0x002E\n");
		return err;
	}

	msg[0].addr = info->i2c_client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = data;

	/* high byte goes out first */
	data[0] = 0x0f;
	data[1] = 0x12;

	msg[1].addr = info->i2c_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = r_data;

	err = i2c_transfer(info->i2c_client->adapter, msg, 2);
	if (err != 2) {
		pr_err("s5k6aafx_read_reg: i2c transfer failed, 0x%x err:%d\n"
				, (unsigned int) r_data, err);
		return -EINVAL;
	}

	info->check_vender = ((r_data[0] << 8) | r_data[1]);

	FUNC_EXIT;

	return 0;
}


static int s5k6aafx_set_preview_resolution
	(struct s5k6aafx_info *info, struct s5k6aafx_mode *mode)
{
	int err = -1;

	FUNC_ENTR;

	if (info->mode == CAPTURE_MODE) {
		pr_err("[CAPTURE_MODE]:func(%s):line(%d)\n"
				, __func__, __LINE__);
#ifdef CONFIG_LOAD_FILE
		switch (info->framesize_index) {
		case S5K6AAFX_PREVIEW_VGA:
			if (mode->camcordmode == 1) {
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_recording_640x480");
			else
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_preview_640x480");
			break;

#ifndef CONFIG_MACH_BOSE_ATT
		case S5K6AAFX_PREVIEW_RECORD_MMS3:
			if (mode->camcordmode == 1)
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_recording_528x432");
			else
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_preview_528x432");
			break;
#endif

		case S5K6AAFX_PREVIEW_VT:
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_vt_352x288");
			break;

		case S5K6AAFX_PREVIEW_VT_QVGA:
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_vt_320x240");
			break;

		case S5K6AAFX_PREVIEW_RECORD_MMS:
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_recording_176x144");
			break;

		default:
			pr_err("%s: returned error.(%d)\n"
					, __func__, __LINE__);
			return -EINVAL;
			break;
		}
#else
		switch (info->framesize_index) {
		case S5K6AAFX_PREVIEW_VGA:
			if (mode->camcordmode == 1)
				err = s5k6aafx_write_table(info->i2c_client
						, mode_recording_640x480);
			else
				err = s5k6aafx_write_table(info->i2c_client
						, mode_preview_640x480);
			break;
#ifndef CONFIG_MACH_BOSE_ATT
		case S5K6AAFX_PREVIEW_RECORD_MMS3:
			if (mode->camcordmode == 1)
				err = s5k6aafx_write_table(info->i2c_client
						, mode_recording_528x432);
			else
				err = s5k6aafx_write_table(info->i2c_client
						, mode_preview_528x432);
			break;
#endif

		case S5K6AAFX_PREVIEW_VT:
			err = s5k6aafx_write_table(info->i2c_client
					, mode_vt_352x288);
			break;

		case S5K6AAFX_PREVIEW_VT_QVGA:
			err = s5k6aafx_write_table(info->i2c_client
					, mode_vt_320x240);
			break;

		case S5K6AAFX_PREVIEW_RECORD_MMS:
			err = s5k6aafx_write_table(info->i2c_client
					, mode_recording_176x144);
			break;

		default:
			pr_err("%s: returned error.(%d)\n"
					, __func__, __LINE__);
			return -EINVAL;
			break;
		}
#endif
		return err;
	} else if (info->mode == SYSTEM_INITIALIZE_MODE) {
		pr_err("[SYSTEM_INITIALIZE_MODE]:func(%s):line(%d)\n"
				, __func__, __LINE__);
#ifdef CONFIG_LOAD_FILE
		switch (info->framesize_index) {
		case S5K6AAFX_PREVIEW_VGA:
			if (mode->camcordmode == 1) {
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_sensor_recording_50Hz_init");
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_recording_640x480");
				info->mode = CAPTURE_MODE;
			} else {
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_sensor_init");
				if (dtpTest) {
					pr_err("%s : dtpTest = %d\n", __func__, dtpTest);
					usleep_range(500000, 600000);
					err = s5k6aafx_write_tuningmode(info->i2c_client
							, "mode_test_pattern");
				}
				info->mode = CAPTURE_MODE;
			}
			break;

#ifndef CONFIG_MACH_BOSE_ATT
		case S5K6AAFX_PREVIEW_RECORD_MMS3:
			if (mode->camcordmode == 1) {
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_sensor_recording_50Hz_init");
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_recording_528x432");
				info->mode = CAPTURE_MODE;
			} else {
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_sensor_init");
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_preview_528x432");

				info->mode = CAPTURE_MODE;
			}
			break;
#endif

		case S5K6AAFX_PREVIEW_VT:
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_sensor_vt_init");
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_vt_352x288");
			info->mode = CAPTURE_MODE;
			break;

		case S5K6AAFX_PREVIEW_VT_QVGA:
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_sensor_vt_init");
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_vt_320x240");
			info->mode = CAPTURE_MODE;
			break;

		case S5K6AAFX_PREVIEW_RECORD_MMS:
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_sensor_recording_50Hz_init");
			err = s5k6aafx_write_tuningmode(info->i2c_client
					, "mode_recording_176x144");
			info->mode = CAPTURE_MODE;
			break;

		default:
			pr_err("%s: returned error.(%d)\n", __func__, __LINE__);
			return -EINVAL;
			break;
		}
#else
		switch (info->framesize_index) {
		case S5K6AAFX_PREVIEW_VGA:
			if (mode->camcordmode == 1) {
				err = s5k6aafx_sensor_burst_write_list(info->i2c_client
						, mode_sensor_recording_50Hz_init
						, (sizeof(mode_sensor_recording_50Hz_init)
						/ sizeof(mode_sensor_recording_50Hz_init[0]))
						, mode);
				err = s5k6aafx_write_table(info->i2c_client
						, mode_recording_640x480);
			info->mode = CAPTURE_MODE;
			} else {
				err = s5k6aafx_sensor_burst_write_list(info->i2c_client
						, mode_sensor_init
						, (sizeof(mode_sensor_init)
						/ sizeof(mode_sensor_init[0]))
						, mode);
				if (dtpTest) {
					pr_err("%s : dtpTest = %d\n"
							, __func__, dtpTest);
					usleep_range(500000, 600000);
					err = s5k6aafx_write_table(info->i2c_client
							, mode_test_pattern);
				}
				info->mode = CAPTURE_MODE;
			}
			break;

#ifndef CONFIG_MACH_BOSE_ATT
		case S5K6AAFX_PREVIEW_RECORD_MMS3:
			if (mode->camcordmode == 1) {
				err = s5k6aafx_sensor_burst_write_list(info->i2c_client
						, mode_sensor_recording_50Hz_init
						, (sizeof(mode_sensor_recording_50Hz_init)
						 / sizeof(mode_sensor_recording_50Hz_init[0]))
						, mode);
				info->mode = CAPTURE_MODE;
			} else {
				err = s5k6aafx_sensor_burst_write_list(info->i2c_client
						, mode_sensor_init
						, (sizeof(mode_sensor_init)
						/ sizeof(mode_sensor_init[0]))
						, mode);
				err = s5k6aafx_write_table(info->i2c_client
						, mode_preview_528x432);
				info->mode = CAPTURE_MODE;
			}
			break;
#endif

		case S5K6AAFX_PREVIEW_VT:
			err = s5k6aafx_sensor_burst_write_list(info->i2c_client
					, mode_sensor_vt_init
					, (sizeof(mode_sensor_vt_init)
					/ sizeof(mode_sensor_vt_init[0]))
					, mode);
			err = s5k6aafx_write_table(info->i2c_client
					, mode_vt_352x288);
			info->mode = CAPTURE_MODE;

			break;

		case S5K6AAFX_PREVIEW_VT_QVGA:
			err = s5k6aafx_sensor_burst_write_list(info->i2c_client
					, mode_sensor_vt_init
					, (sizeof(mode_sensor_vt_init)
					/ sizeof(mode_sensor_vt_init[0]))
					, mode);
			err = s5k6aafx_write_table(info->i2c_client
					, mode_vt_320x240);
			info->mode = CAPTURE_MODE;

			break;

		case S5K6AAFX_PREVIEW_RECORD_MMS:
			err = s5k6aafx_sensor_burst_write_list(info->i2c_client
					, mode_sensor_recording_50Hz_init
					, (sizeof(mode_sensor_recording_50Hz_init)
					/ sizeof(mode_sensor_recording_50Hz_init[0]))
					, mode);
			err = s5k6aafx_write_table(info->i2c_client
					, mode_recording_176x144);
			info->mode = CAPTURE_MODE;

			break;

		default:
			pr_err("%s: returned error.(%d)\n", __func__, __LINE__);
			return -EINVAL;
			break;
		}
#endif
		return 0;
	} else {
		pr_err("%s: invalid preview resolution supplied to set mode %d %d info->mode(%d)\n",
				__func__, mode->xres, mode->yres, info->mode);
		return -EINVAL;
	}

	FUNC_EXIT;

	return err;
}

static int s5k6aafx_set_capture_mode
	(struct s5k6aafx_info *info, struct s5k6aafx_mode *mode)
{
	int err = -1;

	FUNC_ENTR;

	pr_info("s5k6aafx_set_capture_mode -index : %d\n"
			, info->framesize_index);

#ifdef CONFIG_LOAD_FILE
		switch (info->framesize_index) {
		case S5K6AAFX_CAPTURE_1280x960: /* 1280x960 */
#ifndef CONFIG_MACH_BOSE_ATT
			switch (info->mode) {
			case VT_MODE:
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_vt_capture");
				break;

			case RECORD_MODE:
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_recording_capture");
				break;

			case CAPTURE_MODE:
#endif
				err = s5k6aafx_write_tuningmode(info->i2c_client
						, "mode_capture_1280x960");
#ifndef CONFIG_MACH_BOSE_ATT
				break;
			}
#endif
			break;

		default:
			/* The framesize index was not set properly.
			 * Check s_fmt call - it must be for video mode. */
			pr_err("%s: invalid capture resolution supplied to set mode (%d x %d). info->index(%d)\n",
							__func__, mode->xres, mode->yres, info->framesize_index);

			return -EINVAL;
			break;
		}
#else
		switch (info->framesize_index) {
		case S5K6AAFX_CAPTURE_1280x960: /* 1280x960 */
#ifndef CONFIG_MACH_BOSE_ATT
			switch (info->mode) {
			case VT_MODE:
				err = s5k6aafx_write_table(info->i2c_client
						, mode_vt_capture);
				break;

			case RECORD_MODE:
				err = s5k6aafx_write_table(info->i2c_client
						, mode_recording_capture);
				break;

			case CAPTURE_MODE:
#endif
				err = s5k6aafx_write_table(info->i2c_client
						, mode_capture_1280x960);
#ifndef CONFIG_MACH_BOSE_ATT
				break;
			}
#endif
			break;

		default:
			/* The framesize index was not set properly.
			 * Check s_fmt call - it must be for video mode. */
			pr_err("%s: invalid capture resolution supplied to set mode (%d x %d). info->index(%d)\n",
							__func__, mode->xres, mode->yres, info->framesize_index);
			return -EINVAL;
			break;
		}
#endif

	/* Set capture mode */
	if (err < 0)
		pr_err("%s: s5k6aafx_set_capture_mode() returned error, %d/framesize_index(%d)\n"
				, __func__, err, info->framesize_index);

	FUNC_EXIT;

	return err;
}

static int s5k6aafx_get_exif_info(struct s5k6aafx_info *info
		, struct s5k6aafx_exif_info *exifinfo)
{
	int i;
	u8 shutter_data0[2] = {0, 0};
	u8 shutter_data1[2] = {0, 0};
	int shutter_for_exif = 0, shutter_exif0 = 0, shutter_exif1 = 0;
	static char str_shutter_value[8];

	u8 iso_data[2] = {0, 0};
	int iso_value = 0;
	int iso_for_exif = 0, iso_exif = 0;
	static char str_iso_value[8];

	/* standard values */
	u16 iso_std_values[] = { 10, 12, 16, 20, 25, 32, 40, 50, 64, 80,
		100, 125, 160, 200, 250, 320, 400, 500, 640, 800,
		1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400, 8000};
	/* quantization table */
	u16 iso_qtable[] = { 11, 14, 17, 22, 28, 35, 44, 56, 71, 89,
		112, 141, 178, 224, 282, 356, 449, 565, 712, 890,
		1122, 1414, 1782, 2245, 2828, 3564, 4490, 5657, 7127, 8909};

	FUNC_ENTR;

	/* get shutter speed(exposure time) for exif */
	s5k6aafx_read_reg(info->i2c_client, 0x1508, shutter_data0, 2);
	s5k6aafx_read_reg(info->i2c_client, 0x150a, shutter_data1, 2);

	shutter_exif0 = ((shutter_data0[0] << 8)|(shutter_data0[1]));
	shutter_exif1 = ((shutter_data1[0] << 8)|(shutter_data1[1]));

	shutter_for_exif = ((shutter_data1[0] << 24)
				|(shutter_data1[1] << 16)
				|(shutter_data0[0] << 8)
				|(shutter_data0[1]));

	sprintf(str_shutter_value, "%d", shutter_for_exif);
	shutter_for_exif = simple_strtol(str_shutter_value, NULL, 10);

	/* get ISO for exif */
	s5k6aafx_read_reg(info->i2c_client, 0x1508, iso_data, 2);

	iso_exif = ((iso_data[0] << 8)|(iso_data[1]));

	sprintf(str_iso_value, "%d", iso_exif);
	iso_value = (simple_strtol(str_iso_value, NULL, 10) / 256);
#ifndef CONFIG_MACH_BOSE_ATT
	if (iso_value <= 50)
		iso_for_exif = 50;
	else if (iso_value > 50 && iso_value <= 100)
		iso_for_exif = 100;
	else if (iso_value > 100 && iso_value <= 200)
		iso_for_exif = 200;
	else if (iso_value > 200)
		iso_for_exif = 400;
#else
	for (i = 0; i < sizeof(iso_qtable); i++) {
		if (iso_value <= iso_qtable[i]) {
			iso_for_exif = iso_std_values[i];
			break;
		}
	}
#endif

	exifinfo->info_iso = iso_for_exif;
	exifinfo->info_exptime_numer = shutter_for_exif;
	exifinfo->info_exptime_denumer = 500000;

	FUNC_EXIT;

	return 0;
}

bool isLastPreviewActive_front;

static int s5k6aafx_set_mode(struct s5k6aafx_info *info
		, struct s5k6aafx_mode *mode)
{
	int err = 0;
	FUNC_ENTR;
	/*
	pr_err("func(%s): xres %u yres %u\n", __func__, mode->xres, mode->yres);
	pr_err("func(%s):mode->vtcallmode(%d)\n", __func__, mode->vtcallmode);
	pr_err("mode->camcord(%d),modemode->PreviewActive(%d)\n"
			, mode_camcordmode, mode->PreviewActive);
	pr_err("mode->StillCount(%d),mode->VideoActive(%d)\n"
			, mode->StillCount, mode->VideoActive);
	*/

	info->framesize_index = s5k6aafx_get_framesize_index(info, mode);

	if (isLastPreviewActive_front && mode->PreviewActive && !mode->VideoActive) {
		info->pdata->power_off();
		info->mode = SYSTEM_INITIALIZE_MODE;
		info->pdata->power_on();
		err = s5k6aafx_set_preview_resolution(info, mode);
	} else {
		if (mode->PreviewActive) {
			err = s5k6aafx_set_preview_resolution(info, mode);
			if (err < 0) {
				pr_err("%s: s5k6aafx_set_preview_resolution(preview) returned %d\n"
						, __func__, err);
				return -EINVAL;
			}
			if (mode->camcordmode != 2)
				usleep_range(300000, 400000);
			/*After preview set, Sensor must have 300ms delay.*/
			isLastPreviewActive_front = true;
		} else if (mode->StillCount) {
			err = s5k6aafx_set_capture_mode(info, mode);
			s5k6aafx_get_exif_info(info, &front_exif_info);
			isLastPreviewActive_front = false;
		} else if (mode->VideoActive == 1) {
			pr_err("%s: invalid recording resolution supplied to set mode %d %d\n"
					, __func__, mode->xres, mode->yres);
			err = 0;
			isLastPreviewActive_front = false;
		}
	}

	FUNC_EXIT;

	return err;
}

static int s5k6aafx_set_color_effect(struct s5k6aafx_info *info
		, s5k6aafx_color_effect arg)
{
	int err = -1;

	FUNC_ENTR;

	if (info->geffect_index != arg) {
		info->geffect_index = arg;
		pr_err("func(%s): info->geffect_index %d\n"
				, __func__, info->geffect_index);
	} else if (info->geffect_index == arg) {
		pr_err("func(%s): color_effect mode is not changed\n"
				, __func__);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
	case S5K6AAFX_EFFECT_NONE:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_coloreffect_none");
		break;

	case S5K6AAFX_EFFECT_MONO:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_coloreffect_mono");
		break;

	case S5K6AAFX_EFFECT_SEPIA:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_coloreffect_sepia");
		break;

	case S5K6AAFX_EFFECT_NEGATIVE:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_coloreffect_negative");
		break;

	default:
		pr_err("%s: Invalid Color Effect, %d\n", __func__, arg);
		return 0;
		break;
	}
#else
	switch (arg) {
	case S5K6AAFX_EFFECT_NONE:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_coloreffect_none);
		break;

	case S5K6AAFX_EFFECT_MONO:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_coloreffect_mono);
		break;

	case S5K6AAFX_EFFECT_SEPIA:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_coloreffect_sepia);
		break;

	case S5K6AAFX_EFFECT_NEGATIVE:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_coloreffect_negative);
		break;

	default:
		pr_err("%s: Invalid Color Effect, %d\n", __func__, arg);
		return 0;
		break;
	}
#endif

	if (err < 0)
		pr_err("%s: s5k6aafx_write_table() returned error, %d, %d\n"
				, __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k6aafx_set_white_balance(struct s5k6aafx_info *info
		, s5k6aafx_white_balance arg)
{
	int err = -1;

	FUNC_ENTR;

	if (info->gwb_index != arg) {
		info->gwb_index = arg;
		pr_err("func(%s): info->gwb_index %d\n"
				, __func__, info->gwb_index);
	} else if (info->gwb_index == arg) {
		pr_err("func(%s): white balance mode is not changed\n"
				, __func__);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
	case S5K6AAFX_WB_AUTO:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_WB_auto");
		break;

	case S5K6AAFX_WB_DAYLIGHT:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_WB_daylight");
		break;

	case S5K6AAFX_WB_INCANDESCENT:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_WB_incandescent");
		break;

	case S5K6AAFX_WB_FLUORESCENT:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_WB_fluorescent");
		break;

	default:
		pr_err("%s: Invalid White Balance, %d\n", __func__, arg);
		return 0;
		break;
	}

#else
	switch (arg) {
	case S5K6AAFX_WB_AUTO:
		err = s5k6aafx_write_table(info->i2c_client, mode_WB_auto);
		break;

	case S5K6AAFX_WB_DAYLIGHT:
		err = s5k6aafx_write_table(info->i2c_client, mode_WB_daylight);
		break;

	case S5K6AAFX_WB_INCANDESCENT:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_WB_incandescent);
		break;

	case S5K6AAFX_WB_FLUORESCENT:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_WB_fluorescent);
		break;

	default:
		pr_err("%s: Invalid White Balance, %d\n", __func__, arg);
		return 0;
		break;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k6aafx_write_table() returned error, %d, %d\n"
				, __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k6aafx_set_exposure(struct s5k6aafx_info *info
		, s5k6aafx_exposure arg)
{
	int err = -1;

	FUNC_ENTR;

	if (info->gev_index != arg) {
		info->gev_index = arg;
		pr_err("func(%s): info->gev_index %d\n"
				, __func__, info->gev_index);
	} else if (info->gev_index == arg) {
		pr_err("func(%s): exposure mode is not changed\n"
				, __func__);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
	case S5K6AAFX_EXPOSURE_P2P0:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_p2p0");
		break;

	case S5K6AAFX_EXPOSURE_P1P5:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_p1p5");
		break;

	case S5K6AAFX_EXPOSURE_P1P0:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_p1p0");
		break;

	case S5K6AAFX_EXPOSURE_P0P5:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_p0p5");
		break;

	case S5K6AAFX_EXPOSURE_ZERO:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_0");
		break;

	case S5K6AAFX_EXPOSURE_M0P5:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_m0p5");
		break;

	case S5K6AAFX_EXPOSURE_M1P0:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_m1p0");
		break;

	case S5K6AAFX_EXPOSURE_M1P5:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_m1p5");
		break;

	case S5K6AAFX_EXPOSURE_M2P0:
		err = s5k6aafx_write_tuningmode(info->i2c_client
				, "mode_exposure_m2p0");
		break;

	default:
		pr_err("%s: Invalid Exposure Value, %d\n", __func__, arg);
		return 0;
		break;
	}
#else	/* CONFIG_LOAD_FILE */
	switch (arg) {
	case S5K6AAFX_EXPOSURE_P2P0:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_p2p0);
		break;

	case S5K6AAFX_EXPOSURE_P1P5:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_p1p5);
		break;

	case S5K6AAFX_EXPOSURE_P1P0:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_p1p0);
		break;

	case S5K6AAFX_EXPOSURE_P0P5:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_p0p5);
		break;

	case S5K6AAFX_EXPOSURE_ZERO:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_0);
		break;

	case S5K6AAFX_EXPOSURE_M0P5:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_m0p5);
		break;

	case S5K6AAFX_EXPOSURE_M1P0:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_m1p0);
		break;

	case S5K6AAFX_EXPOSURE_M1P5:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_m1p5);
		break;

	case S5K6AAFX_EXPOSURE_M2P0:
		err = s5k6aafx_write_table(info->i2c_client
				, mode_exposure_m2p0);
		break;

	default:
		pr_err("%s: Invalid Exposure Value, %d\n", __func__, arg);
		return 0;
		break;
	}
#endif	/* CONFIG_LOAD_FILE */

	if (err < 0)
		pr_err("%s: s5k6aafx_write_table() returned error, %d, %d\n"
				, __func__, arg, err);

	FUNC_EXIT;

	return err;
}

#ifdef FACTORY_TEST
static int s5k6aafx_return_normal_preview(struct s5k6aafx_info *info)
{
	FUNC_ENTR;

	info->pdata->power_off();
	regulator_disable(reg_mipi_1v2);

	usleep_range(20000, 30000);

	regulator_enable(reg_mipi_1v2);
	info->pdata->power_on();

#ifdef CONFIG_LOAD_FILE
	s5k6aafx_write_tuningmode(info->i2c_client, "mode_sensor_init");
#else
	s5k6aafx_write_table(info->i2c_client, mode_sensor_init);
#endif
	usleep_range(300000, 400000);
	/*After preview set, Sensor must have 300ms delay.*/

	FUNC_EXIT;

	return 0;
}
#endif

static long s5k6aafx_ioctl(struct file *file,
			unsigned int cmd, unsigned long arg)
{
	struct s5k6aafx_info *info = file->private_data;

	/*printk(KERN_ERR "func(%s): cmd(%d)\n", __func__, cmd - 1074032384);*/

	if (dtpTest == S5K6AAFX_DTP_TEST_ON) {
		if ((cmd != S5K6AAFX_IOCTL_SET_MODE) &&
			(cmd != S5K6AAFX_IOCTL_DTP_TEST)) {
			pr_err(KERN_ERR "func(%s):line(%d)s5k6aafx_DTP_TEST_ON. cmd(%d)\n"
					, __func__, __LINE__, cmd);
			return 0;
		}
	}

	switch (cmd) {
	case S5K6AAFX_IOCTL_SET_MODE:
		if (copy_from_user(&front_mode, (const void __user *)arg
					, sizeof(struct s5k6aafx_mode))) {
			pr_info("%s %d\n", __func__, __LINE__);
			return -EFAULT;
		}
		return s5k6aafx_set_mode(info, &front_mode);

	case S5K6AAFX_IOCTL_ESD_RESET:
		if (dtpTest == S5K6AAFX_DTP_TEST_OFF)
			s5k6aafx_esdreset(info);
		break;

	case S5K6AAFX_IOCTL_COLOR_EFFECT:
		return s5k6aafx_set_color_effect(info
				, (s5k6aafx_color_effect) arg);

	case S5K6AAFX_IOCTL_WHITE_BALANCE:
		return s5k6aafx_set_white_balance(info
				, (s5k6aafx_white_balance) arg);

	case S5K6AAFX_IOCTL_EXPOSURE:
		return s5k6aafx_set_exposure(info, (s5k6aafx_exposure) arg);

	case S5K6AAFX_IOCTL_EXIF_INFO:
		if (copy_to_user((void __user *)arg, &front_exif_info
					, sizeof(struct s5k6aafx_exif_info)))
			return -EFAULT;
		return 0;

#ifdef FACTORY_TEST
	case S5K6AAFX_IOCTL_DTP_TEST:
	{
		int status = 0;
		if (dtpTest == S5K6AAFX_DTP_TEST_ON &&
				(s5k6aafx_dtp_test) arg == 0) {
			pr_err("[S5K6AAFX]%s: S5K6AAFX_IOCTL_DTP_TEST Entered!!! dtpTest = %d\n"
					, __func__, (s5k6aafx_dtp_test) arg);
			status = s5k6aafx_return_normal_preview(info);
		}
		dtpTest = (s5k6aafx_dtp_test) arg;
		return status;
	}
#endif
	default:
		pr_err("%s: s5k6aafx_ioctl() returned error.\n"
				, __func__);
		return -EINVAL;
	}

	return 0;
}

#ifdef FACTORY_TEST
static ssize_t front_camtype_file_cmd_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char camType[] = "SLSI_S5K6AAFX_NONE";

	return sprintf(buf, "%s", camType);
}

static ssize_t front_camtype_file_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(front_camtype, 0660, front_camtype_file_cmd_show
		, front_camtype_file_cmd_store);
#endif	/* FACTORY_TEST */

static int s5k6aafx_open(struct inode *inode, struct file *file)
{
	int err = -1;

	struct s5k6aafx_info *pinfo;
	file->private_data = info;
	pinfo = (struct s5k6aafx_info *)file->private_data;

	FUNC_ENTR;

	if (pinfo->pdata && pinfo->pdata->power_on) {
		regulator_enable(reg_mipi_1v2);
		pinfo->pdata->power_on();
	}

#ifdef CONFIG_LOAD_FILE
	err = loadFile();
	if (unlikely(err)) {
		pr_err("%s: failed to init\n", __func__);
		return err;
	}
#endif

	err = s5k6aafx_check_vender(info);
	if (err < 0) {
		pr_err("%s: s5k6aafx_check_vender returned error, %d\n"
				, __func__, err);
		pinfo->pdata->power_off();
		regulator_disable(reg_mipi_1v2);
		return -ENODEV;
	}


	info->mode = SYSTEM_INITIALIZE_MODE;
	info->check_vender = 0;				/* for vender check*/
	info->geffect_index = S5K6AAFX_EFFECT_NONE;	/* for effect*/
	info->gev_index = S5K6AAFX_EXPOSURE_ZERO;	/* for ev*/
	info->gwb_index = S5K6AAFX_WB_AUTO;		/* for wb*/
	front_mode.xres = 0;
	front_mode.yres = 0;
	front_mode.frame_length = 0;
	front_mode.coarse_time = 0;
	front_mode.gain = 0;
	front_mode.PreviewActive = 0;
	front_mode.VideoActive = 0;
	front_mode.HalfPress = 0;
	front_mode.StillCount = 0;
	front_mode.camcordmode = 0;
	front_mode.vtcallmode = 0;
	dtpTest = S5K6AAFX_DTP_TEST_OFF;
	isLastPreviewActive_front = false;

	FUNC_EXIT;

	return 0;
}

int s5k6aafx_release(struct inode *inode, struct file *file)
{
	struct s5k6aafx_info *pinfo;
	pinfo = (struct s5k6aafx_info *)file->private_data;

	FUNC_ENTR;

	if (pinfo->pdata && pinfo->pdata->power_off) {
		pinfo->pdata->power_off();
		regulator_disable(reg_mipi_1v2);
	}

	FUNC_EXIT;

	return 0;
}


static const struct file_operations s5k6aafx_fileops = {
	.owner = THIS_MODULE,
	.open = s5k6aafx_open,
	.unlocked_ioctl = s5k6aafx_ioctl,
	.compat_ioctl = s5k6aafx_ioctl,
	.release = s5k6aafx_release,
};

static struct miscdevice s5k6aafx_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "s5k6aafx",
	.fops = &s5k6aafx_fileops,
};

static int s5k6aafx_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err;

	FUNC_ENTR;

	info = kzalloc(sizeof(struct s5k6aafx_info), GFP_KERNEL);
	if (!info) {
		pr_err("s5k6aafx: Unable to allocate memory!\n");
		return -ENOMEM;
	}
	err = misc_register(&s5k6aafx_device);
	if (err) {
		pr_err("s5k6aafx: Unable to register misc device!\n");
		kfree(info);
		return err;
	}

	info->i2c_client = client;
	i2c_client_camera = client;

	if (client->dev.platform_data == NULL) {
		pr_err("s5k6aafx probe: client->dev.platform_data is NULL!\n");
		return -ENXIO;
	}

	info->pdata = client->dev.platform_data;

	i2c_set_clientdata(client, info);

	s5k6aafx_dev = device_create(camera_class, NULL, 0, NULL, "front");
	if (IS_ERR(s5k6aafx_dev)) {
		pr_err("Failed to create device!");
		return -ENXIO;
	}
#ifdef FACTORY_TEST
	if (device_create_file(s5k6aafx_dev, &dev_attr_front_camtype) < 0) {
		pr_err("Failed to create camtype device file!(%s)!\n"
				, dev_attr_front_camtype.attr.name);
		return -ENXIO;
	}
#endif

	FUNC_EXIT;

	return 0;
}

static int s5k6aafx_remove(struct i2c_client *client)
{
	struct s5k6aafx_info *info;

	FUNC_ENTR;

	info = i2c_get_clientdata(client);
	misc_deregister(&s5k6aafx_device);
	kfree(info);

	FUNC_EXIT;

	return 0;
}

static const struct i2c_device_id s5k6aafx_id[] = {
	{ "s5k6aafx", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, s5k6aafx_id);

static struct i2c_driver s5k6aafx_i2c_driver = {
	.driver = {
		.name = "s5k6aafx",
		.owner = THIS_MODULE,
	},
	.probe = s5k6aafx_probe,
	.remove = s5k6aafx_remove,
	.id_table = s5k6aafx_id,
};

static int __init s5k6aafx_init(void)
{
	int status;

	FUNC_ENTR;

	pr_info("s5k6aafx sensor driver loading\n");

	status = i2c_add_driver(&s5k6aafx_i2c_driver);
	if (status) {
		printk(KERN_ERR "s5k6aafx error\n");
		return status;
	}

	FUNC_EXIT;

	return 0;
}

static void __exit s5k6aafx_exit(void)
{
	FUNC_ENTR;

	i2c_del_driver(&s5k6aafx_i2c_driver);

	FUNC_EXIT;
}

module_init(s5k6aafx_init);
module_exit(s5k6aafx_exit);
