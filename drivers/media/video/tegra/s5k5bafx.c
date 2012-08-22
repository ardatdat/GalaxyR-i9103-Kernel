/*
 * s5k5bafx.c - s5k5bafx sensor driver
 *
 * Copyright (C) 2010 Google Inc.
 *
 * Contributors:
 *      Rebecca Schultz Zavin <rebecca@android.com>
 *
 * Leverage OV9640.c
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
#include <media/s5k5bafx.h>
#include <media/tegra_camera.h>
#include <s5k5bafx_setting.h>

#define DEBUG_PRINTS 1
#if DEBUG_PRINTS
	#define FUNC_ENTR	\
		printk(KERN_INFO "[S5K5BAFX] %s Entered!!!\n", __func__)
#else
	#define FUNC_ENTR
#endif

//#define CONFIG_LOAD_FILE	1

#ifdef CONFIG_LOAD_FILE
#include <linux/vmalloc.h>
#include <linux/mm.h>
/*#define max_size 200000*/

struct test {
	char data;
	struct test *nextBuf;
};

struct test *testBuf;
#endif

struct s5k5bafx_info {
	int mode;
	struct i2c_client *i2c_client;
	struct s5k5bafx_platform_data *pdata;
};

/* extern struct i2c_client *i2c_client_pmic; */
#ifdef FACTORY_TEST
static s5k5bafx_dtp_test dtpTest = S5K5BAFX_DTP_TEST_OFF;
#endif

#define S5K5BAFX_MAX_RETRIES 3
#define S5K5BAFX_READ_STATUS_RETRIES 50

enum {
	SYSTEM_INITIALIZE_MODE,
	MONITOR_MODE,
	CAPTURE_MODE
};

enum {
	S5K5BAFX_MODE_SENSOR_INIT,		//Camera - start
	S5K5BAFX_MODE_PREVIEW_640x480,		//Camera - preview return
	S5K5BAFX_MODE_PREVIEW_800x600,		//Camera - preview return
	S5K5BAFX_MODE_CAPTURE_1600x1200,	//Camera - capture
#ifdef FACTORY_TEST
	S5K5BAFX_MODE_TEST_PATTERN,
	S5K5BAFX_MODE_TEST_PATTERN_OFF,
#endif
	S5K5BAFX_MODE_SENSOR_VT_INIT,
	S5K5BAFX_MODE_SENSOR_RECORDING_50HZ_INIT,
};

static struct s5k5bafx_reg *mode_table[] = {
	[S5K5BAFX_MODE_SENSOR_INIT] = mode_sensor_init,
	[S5K5BAFX_MODE_PREVIEW_640x480] = mode_preview_640x480,
	[S5K5BAFX_MODE_PREVIEW_800x600] = mode_preview_800x600,
	[S5K5BAFX_MODE_CAPTURE_1600x1200] = mode_capture_1600x1200,
	[S5K5BAFX_MODE_SENSOR_VT_INIT] = mode_sensor_vt_init,
	[S5K5BAFX_MODE_SENSOR_RECORDING_50HZ_INIT] = mode_sensor_recording_50Hz_init,
#ifdef FACTORY_TEST
	[S5K5BAFX_MODE_TEST_PATTERN] = mode_test_pattern,
	[S5K5BAFX_MODE_TEST_PATTERN_OFF] = mode_test_pattern_off,
#endif
};

static int s5k5bafx_read_reg(struct i2c_client *client, u16 addr, u8 *val, u16 length)
{
	int err;
	struct i2c_msg msg[2];
	unsigned char data[2];

	if (!client->adapter)
		return -ENODEV;

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = data;

	/* high byte goes out first */
	data[0] = (u8) (addr >> 8);;
	data[1] = (u8) (addr & 0xff);

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = length;
	msg[1].buf = val;
	err = i2c_transfer(client->adapter, msg, 2);

	if (err != 2)
		return -EINVAL;

	return 0;
}

static int s5k5bafx_write_reg_8(struct i2c_client *client, u8 addr, u8 val)
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
		pr_err("s5k5bafx(8): i2c transfer failed, retrying %x %x\n",
		       addr, val);
		msleep(3);
	} while (retry <= S5K5BAFX_MAX_RETRIES);

	return err;
}


static int s5k5bafx_write_reg(struct i2c_client *client, u16 addr, u16 val)
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
		pr_err("s5k5bafx: i2c transfer failed, retrying %x %x\n",
		       addr, val);
		msleep(3);
	} while (retry <= S5K5BAFX_MAX_RETRIES);

	return err;
}

static int s5k5bafx_write_table(struct i2c_client *client,
			      const struct s5k5bafx_reg table[])
{
	int err;
	const struct s5k5bafx_reg *next;
	u16 val;

	for (next = table; next->addr != S5K5BAFX_TABLE_END; next++) {
		if (next->addr == S5K5BAFX_TABLE_WAIT_MS) {
			msleep(next->val);
			continue;
		}

		val = next->val;

		err = s5k5bafx_write_reg(client, next->addr, val);
		//printk(KERN_ERR "[kidggang]:next->addr(%x):val(%x)\n",next->addr, val);
		if (err < 0)
			return err;
	}
	return 0;
}

int s5k5bafx_write_table_8(struct i2c_client *client,
			      const struct s5k5bafx_reg_8 *table)
{
	int err;
	const struct s5k5bafx_reg_8 *next;
	u8 val;

	for (next = table; next->addr != S5K5BAFX_TABLE_END_8; next++) {
		if (next->addr == S5K5BAFX_TABLE_WAIT_MS_8) {
			msleep(next->val);
			continue;
		}

		val = next->val;

		err = s5k5bafx_write_reg_8(client, next->addr, val);
		if (err)
			return err;
	}
	return 0;
}

#ifdef CONFIG_LOAD_FILE
static inline int s5k5bafx_write(struct i2c_client *client,
		unsigned int addr_reg, unsigned int data_reg)
{
	struct i2c_msg msg[1];
	unsigned char buf[4];
	int ret;

#if 0
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.buf	= buf,
		.len	= 4,
	};
#endif

	buf[0] = ((addr_reg >> 8) & 0xFF);
	buf[1] = (addr_reg) & 0xFF;
	buf[2] = ((data_reg >> 8) & 0xFF);
	buf[3] = (data_reg) & 0xFF;

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 4;
	msg->buf = buf;

	/*printk("========s5k5bafx_write=======(a:%x,d:%x)\n", addr_reg, data_reg);*?
	/**(unsigned long *)buf = cpu_to_be32(packet);*/

	ret = i2c_transfer(client->adapter, &msg, 1);

	if (unlikely(ret < 0)) {
		dev_err(&client->dev, "%s: (a:%x,d:%x) write failed\n", __func__, addr_reg, data_reg);
		return ret;
	}

	return (ret != 1) ? -1 : 0;
}

static int s5k5bafx_write_tuningmode(struct i2c_client *client, char s_name[])
{
	int ret = -EAGAIN;
	unsigned long temp;
	char delay = 0;
	char data[11];
	int searched = 0;
	int size = strlen(s_name);
	int i;
	unsigned int addr_reg;
	unsigned int data_reg;

	struct test *tempData;

	printk("%s: size = %d, string = %s\n", __func__, size, s_name);
	tempData = &testBuf[0];
	while (!searched) {
		searched = 1;
		for (i = 0; i < size; i++) {
			if (tempData->data != s_name[i]) {
				searched = 0;
				break;
			}
			tempData = tempData->nextBuf;
		}

		if(tempData->nextBuf == NULL) {
			printk(KERN_INFO "%s: tempData->nextBuf is NULL\n", __func__);

			return -1;
		}

		tempData = tempData->nextBuf;
	}

	printk(KERN_INFO "%s: %s is searched\n", __func__, s_name);

	while (1) {
		if (tempData->data == '{')
			break;
		else
			tempData = tempData->nextBuf;
	}

	while (1) {
		searched = 0;
		while (1) {
			if (tempData->data == 'x') {
				/*get 10 strings*/
				data[0] = '0';
				for (i = 1; i < 11; i++) {
					data[i] = tempData->data;
					tempData = tempData->nextBuf;
				}
				/*printk("%s\n", data);*/
				temp = simple_strtoul(data, NULL, 16);
				break;
			} else if (tempData->data == '}') {
				searched = 1;
				break;
			} else {
				tempData = tempData->nextBuf;
			}
			if (tempData->nextBuf == NULL)
				return -1;
		}

		if (searched)
			break;

		/*let search...*/
		if ((temp & 0xFFFE0000) == 0xFFFE0000) {
			delay = temp & 0xFFFF;
			printk("func(%s):line(%d):delay(0x%x):delay(%d)\n", __func__, __LINE__, delay, delay);
			msleep(delay);
			continue;
		}
		/*ret = s5k5bafx_write(client, temp);*/

		addr_reg = (temp >> 16)&0xffff;
		data_reg = (temp & 0xffff);

		ret = s5k5bafx_write(client, addr_reg, data_reg);

		/*printk("addr = %x\n", addr_reg);*/
		/*printk("data = %x\n", data_reg);*/

		/* In error circumstances */
		/* Give second shot */
		if (unlikely(ret)) {
			dev_info(&client->dev,
					"s5k5bafx i2c retry one more time\n");
			ret = s5k5bafx_write(client, addr_reg, data_reg);

			/* Give it one more shot */
			if (unlikely(ret)) {
				dev_info(&client->dev,
						"s5k5bafx i2c retry twice\n");
				ret = s5k5bafx_write(client, addr_reg, data_reg);
			}
		}
	}
	return ret;
}

static int loadFile(void){

	struct file *fp;
	char *nBuf;
	int max_size;
	int l;
	struct test *nextBuf = testBuf;
	int i = 0;

	int starCheck = 0;
	int check = 0;
	int ret = 0;
	loff_t pos;

	printk("%s\n", __func__);

	mm_segment_t fs = get_fs();
	set_fs(get_ds());

	fp = filp_open("/mnt/sdcard/s5k5bafx.h", O_RDONLY, 0);

	if (IS_ERR(fp)) {
		printk("%s : file open error\n", __func__);
		return PTR_ERR(fp);
	}

	printk("%s: file open success\n", __func__);

	l = (int) fp->f_path.dentry->d_inode->i_size;

	max_size = l;

	printk("l = %d\n", l);

	nBuf = vmalloc(l);

	if (nBuf == NULL) {
		printk("Out of Memory\n");
		filp_close(fp, current->files);
	}

	printk("%s: nBuf = 0x%x\n", __func__, nBuf);

	testBuf = (struct test *)vmalloc(sizeof(struct test) * l);

	printk("%s: testBuf = 0x%x\n", __func__, testBuf);

	pos = 0;
	memset(nBuf, 0, l);
	memset(testBuf, 0, l * sizeof(struct test));

	ret = vfs_read(fp, (char __user *)nBuf, l, &pos);

	if (ret != l) {
		printk("failed to read file ret = %d\n", ret);
		kfree(nBuf);
		kfree(testBuf);
		filp_close(fp, current->files);
		return -1;
	}

	filp_close(fp, current->files);

	set_fs(fs);

	i = max_size;

	printk("i = %d\n", i);

	while (i) {
		testBuf[max_size - i].data = *nBuf;
		if (i != 1) {
			testBuf[max_size - i].nextBuf = &testBuf[max_size - i + 1];
		} else {
			testBuf[max_size - i].nextBuf = NULL;
			break;
		}
		i--;
		nBuf++;
	}

	i = max_size;
	nextBuf = &testBuf[0];
#if 1
	while (i - 1) {
		if (!check && !starCheck) {
			if (testBuf[max_size - i].data == '/') {
				if (testBuf[max_size-i].nextBuf != NULL) {
					if (testBuf[max_size-i].nextBuf->data == '/') {
						check = 1;/* when find '//' */
						i--;
					} else if (testBuf[max_size-i].nextBuf->data == '*') {
						starCheck = 1;/* when find '/*' */
						i--;
					}
				} else {
					break;
				}
			}
			if (!check && !starCheck)
				/* ignore '\t' */
				if (testBuf[max_size - i].data != '\t') {
					nextBuf->nextBuf = &testBuf[max_size-i];
					nextBuf = &testBuf[max_size - i];
				}

		} else if (check && !starCheck) {
			if (testBuf[max_size - i].data == '/') {
				if (testBuf[max_size-i].nextBuf != NULL) {
					if (testBuf[max_size-i].nextBuf->data == '*') {
						starCheck = 1;/* when find '/*' */
						check = 0;
						i--;
					}
				} else {
					break;
				}
			}
			/* when find '\n' */
			if (testBuf[max_size - i].data == '\n' && check) {
				check = 0;
				nextBuf->nextBuf = &testBuf[max_size - i];
				nextBuf = &testBuf[max_size - i];
			}
		} else if (!check && starCheck) {
			if (testBuf[max_size - i].data == '*') {
				if (testBuf[max_size-i].nextBuf != NULL) {
					if (testBuf[max_size-i].nextBuf->data == '/') {
						starCheck = 0;/* when find '/*' */
						i--;
					}
				} else {
					break;
				}
			}

		}
		i--;
		if (i < 2) {
			nextBuf = NULL;
			break;
		}
		if (testBuf[max_size - i].nextBuf == NULL) {
			nextBuf = NULL;
			break;
		}
	}
#endif
#if 0
	printk("i = %d\n", i);
	nextBuf = &testBuf[0];
	while (1) {
		if (nextBuf->nextBuf == NULL)
			break;
		printk("%c", nextBuf->data);
		nextBuf = nextBuf->nextBuf;
	}
#endif
	return 0;
}
#endif

static int s5k5bafx_set_preview_resolution
	(struct s5k5bafx_info *info, struct s5k5bafx_mode *mode)
{
	FUNC_ENTR;

	int sensor_mode;
	int err = -1;

	printk(KERN_ERR "[kidggang]:func(%s): xres %u yres %u\n", __func__, mode->xres, mode->yres);
	{
		if (info->mode == CAPTURE_MODE) {
#ifdef CONFIG_LOAD_FILE
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_preview_640X480");
#else
			err = s5k5bafx_write_table(info->i2c_client, mode_table[S5K5BAFX_MODE_PREVIEW_640x480]);
#endif
			return err;
		} else if (info->mode == SYSTEM_INITIALIZE_MODE) {
			if (mode->vtcallmode && (mode->xres == 176 && mode->yres == 144)) {
					pr_err("%s : mode->vtcallmode = %d\n", __func__, mode->vtcallmode);
#ifdef CONFIG_LOAD_FILE
					err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_sensor_vt_init");
#else
					err = s5k5bafx_write_table(info->i2c_client, mode_table[S5K5BAFX_MODE_SENSOR_VT_INIT]);
#endif
			} else if (mode->xres == 640 && mode->yres == 480) {
				if(mode->camcordmode) {
					/* camcordmode */
#ifdef CONFIG_LOAD_FILE
					err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_sensor_recording_50Hz_init");
#else
					err = s5k5bafx_write_table(info->i2c_client, mode_table[S5K5BAFX_MODE_SENSOR_RECORDING_50HZ_INIT]);
#endif
				} else {
					/* Not camcordmode */
#ifdef CONFIG_LOAD_FILE
					err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_sensor_init");
#else
					err = s5k5bafx_write_table(info->i2c_client, mode_table[S5K5BAFX_MODE_SENSOR_INIT]);
#endif
				}
#ifdef FACTORY_TEST
				if (dtpTest) {
					pr_err("%s : dtpTest = %d\n", __func__, dtpTest);
#ifdef CONFIG_LOAD_FILE
					err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_test_pattern");
#else
					err = s5k5bafx_write_table(info->i2c_client, mode_table[S5K5BAFX_MODE_TEST_PATTERN]);
#endif
				}
#endif
			}
			/*else if (mode->xres == 800 && mode->yres == 600) {
				if(mode->camcordmode) {
					// camcordmode
				} else {
					// Not camcordmode
				}
				err = s5k5bafx_write_table(info->i2c_client, mode_table[S5K5BAFX_MODE_PREVIEW_800x600]);
			}*/
			else {
				pr_err("%s: invalid preview resolution supplied to set mode %d %d info->mode(%d)\n",
					__func__, mode->xres, mode->yres, info->mode);
				return -EINVAL;
			}
		} else {
			pr_err("%s: invalid preview resolution supplied to set mode %d %d info->mode(%d)\n",
					__func__, mode->xres, mode->yres, info->mode);
			return -EINVAL;
		}
	}

	return err;
}

static int s5k5bafx_set_mode(struct s5k5bafx_info *info, struct s5k5bafx_mode *mode)
{
	FUNC_ENTR;

	int sensor_mode;
	int err;

	printk(KERN_ERR "[kidggang]:func(%s): xres %u yres %u\n", __func__, mode->xres, mode->yres);
	printk(KERN_ERR "[kidggang]:func(%s):mode->vtcallmode(%d),mode->camcord(%d),modemode->PreviewActive(%d),mode->StillCount(%d),mode->VideoActive(%d)\n",\
	__func__,mode->vtcallmode,mode->camcordmode,mode->PreviewActive,mode->StillCount,mode->VideoActive);

/*	if (mode->xres == 176 && mode->yres == 144) {
		sensor_mode = S5K5BAFX_MODE_PREVIEW_640x480;
	} else if (mode->xres == 800 && mode->yres == 600) {
		sensor_mode = S5K5BAFX_MODE_PREVIEW_800x600;
	} else if (mode->xres == 640 && mode->yres == 480) {
#ifdef FACTORY_TEST
		if (dtpTest) {
			pr_err("%s : dtpTest = %d\n", __func__, dtpTest);

			sensor_mode = S5K5BAFX_MODE_TEST_PATTERN;
		}

		else
#endif
		sensor_mode = S5K5BAFX_MODE_PREVIEW_640x480;
	} else if (mode->xres == 1600 && mode->yres == 1200)
		sensor_mode = S5K5BAFX_MODE_CAPTURE_1600x1200;
	else {
		pr_err("%s: invalid resolution supplied to set mode %d %d\n",
		       __func__, mode->xres, mode->yres);
		return -EINVAL;
	}

#ifdef CONFIG_LOAD_FILE
	if (sensor_mode == S5K5BAFX_MODE_PREVIEW_800x600)
		err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_preview_800X600");
	else if (sensor_mode == S5K5BAFX_MODE_PREVIEW_640x480)
		err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_preview_640X480");
	else if (sensor_mode == S5K5BAFX_MODE_CAPTURE_1600x1200)
		err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_capture_1600X1200");
	else if (sensor_mode == S5K5BAFX_MODE_TEST_PATTERN)
		err = s5k5bafx_write_tuningmode(info->i2c_client, "test_dtp");
#else
	err = s5k5bafx_write_table(info->i2c_client, mode_table[sensor_mode]);
#endif

	if (sensor_mode == S5K5BAFX_MODE_CAPTURE_1600x1200) {
		msleep(300);
	}
	if (err < 0)
		return err;

	info->mode = sensor_mode;
*/
	if(mode->PreviewActive == 1) {
		err = s5k5bafx_set_preview_resolution(info, mode);
		if(err < 0) {
			pr_err("%s: s5k5bafx_set_preview_resolution() returned %d\n",
					__func__, err);

			return -EINVAL;
		}

		sensor_mode = MONITOR_MODE;
	} else if(mode->StillCount == 1) {
		if (mode->xres == 1600 && mode->yres == 1200) {
			// capture size 1
#ifdef CONFIG_LOAD_FILE
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_capture_1600X1200");
#else
			err = s5k5bafx_write_table(info->i2c_client, mode_table[S5K5BAFX_MODE_CAPTURE_1600x1200]);
#endif
			msleep(300);
		}
		/*else if (mode->xres ==  && mode->yres ==  ) {
			// capture size 2
		}*/
		else {
			pr_err("%s: invalid capture resolution supplied to set mode %d %d\n",
					__func__, mode->xres, mode->yres);
			return -EINVAL;
		}

		sensor_mode = CAPTURE_MODE;
	} else if(mode->VideoActive == 1) {
		pr_err("%s: invalid recording resolution supplied to set mode %d %d\n",
				__func__, mode->xres, mode->yres);
		sensor_mode = MONITOR_MODE;
		err = 0;
	}

	info->mode = sensor_mode;

	return err;
}

static int s5k5bafx_set_color_effect(struct s5k5bafx_info *info, s5k5bafx_color_effect arg)
{
	FUNC_ENTR;

	int err;

	switch (arg) {
	case FRONT_EFFECT_NONE:
		err = s5k5bafx_write_table(info->i2c_client, mode_coloreffect_none);
		break;

	case FRONT_EFFECT_MONO:
		err = s5k5bafx_write_table(info->i2c_client, mode_coloreffect_mono);
		break;

	case FRONT_EFFECT_SEPIA:
		err = s5k5bafx_write_table(info->i2c_client, mode_coloreffect_sepia);
		break;

	case FRONT_EFFECT_NEGATIVE:
		err = s5k5bafx_write_table(info->i2c_client, mode_coloreffect_negative);
		break;

	default:
		pr_err("%s: Invalid Color Effect, %d\n", __func__, arg);
		return 0;
		break;
	}

	if (err < 0)
		pr_err("%s: s5k5bafx_write_table() returned error, %d, %d\n", __func__, arg, err);

	return err;
}

static int s5k5bafx_set_white_balance(struct s5k5bafx_info *info, s5k5bafx_white_balance arg)
{
	FUNC_ENTR;

	int err;

	switch (arg) {
	case FRONT_WB_AUTO:
		err = s5k5bafx_write_table(info->i2c_client, mode_WB_auto);
		break;

	case FRONT_WB_DAYLIGHT:
		err = s5k5bafx_write_table(info->i2c_client, mode_WB_daylight);
		break;

	case FRONT_WB_INCANDESCENT:
		err = s5k5bafx_write_table(info->i2c_client, mode_WB_incandescent);
		break;

	case FRONT_WB_FLUORESCENT:
		err = s5k5bafx_write_table(info->i2c_client, mode_WB_fluorescent);
		break;

	default:
		pr_err("%s: Invalid White Balance, %d\n", __func__, arg);
		return 0;
		break;
	}

	if (err < 0)
		pr_err("%s: s5k5bafx_write_table() returned error, %d, %d\n", __func__, arg, err);

	return err;
}

static int s5k5bafx_set_exposure(struct s5k5bafx_info *info, s5k5bafx_exposure arg)
{
	FUNC_ENTR;

	int err;

#if CONFIG_LOAD_FILE
		switch (arg) {
		case FRONT_EXPOSURE_P2P0:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_p2p0");
			break;

		case FRONT_EXPOSURE_P1P5:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_p1p5");
			break;

		case FRONT_EXPOSURE_P1P0:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_p1p0");
			break;

		case FRONT_EXPOSURE_P0P5:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_p0p5");
			break;

		case FRONT_EXPOSURE_ZERO:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_0");
			break;

		case FRONT_EXPOSURE_M0P5:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_m0p5");
			break;

		case FRONT_EXPOSURE_M1P0:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_m1p0");
			break;

		case FRONT_EXPOSURE_M1P5:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_m1p5");
			break;

		case FRONT_EXPOSURE_M2P0:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_exposure_m2p0");
			break;

		default:
			pr_err("%s: Invalid Exposure Value, %d\n", __func__, arg);
			return 0;
			break;
		}
#else	/* CONFIG_LOAD_FILE */
	switch (arg) {
		case FRONT_EXPOSURE_P2P0:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_p2p0);
			break;

		case FRONT_EXPOSURE_P1P5:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_p1p5);
			break;

		case FRONT_EXPOSURE_P1P0:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_p1p0);
			break;

		case FRONT_EXPOSURE_P0P5:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_p0p5);
			break;

		case FRONT_EXPOSURE_ZERO:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_0);
			break;

		case FRONT_EXPOSURE_M0P5:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_m0p5);
			break;

		case FRONT_EXPOSURE_M1P0:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_m1p0);
			break;

		case FRONT_EXPOSURE_M1P5:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_m1p5);
			break;

		case FRONT_EXPOSURE_M2P0:
			err = s5k5bafx_write_table(info->i2c_client, mode_exposure_m2p0);
			break;

		default:
			pr_err("%s: Invalid Exposure Value, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif	/* CONFIG_LOAD_FILE */
	if (err < 0)
		pr_err("%s: s5k5bafx_write_table() returned error, %d, %d\n", __func__, arg, err);

	return err;
}

static int s5k5bafx_set_pretty(struct s5k5bafx_info *info, s5k5bafx_exposure arg)
{
	FUNC_ENTR;

	int err;

#if CONFIG_LOAD_FILE
	switch (arg) {
		case FRONT_PRETTY_P0:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_pretty_0");
			break;

		case FRONT_PRETTY_P1:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_pretty_1");
			break;

		case FRONT_PRETTY_P2:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_pretty_2");
			break;

		case FRONT_PRETTY_P3:
			err = s5k5bafx_write_tuningmode(info->i2c_client, "mode_pretty_3");
			break;

		default:
			pr_err("%s: Invalid Pretty Value, %d\n", __func__, arg);
			return 0;
			break;
	}
#else	/* CONFIG_LOAD_FILE */
	switch (arg) {
		case FRONT_PRETTY_P0:
			err = s5k5bafx_write_table(info->i2c_client, mode_pretty_0);
			break;

		case FRONT_PRETTY_P1:
			err = s5k5bafx_write_table(info->i2c_client, mode_pretty_1);
			break;

		case FRONT_PRETTY_P2:
			err = s5k5bafx_write_table(info->i2c_client, mode_pretty_2);
			break;

		case FRONT_PRETTY_P3:
			err = s5k5bafx_write_table(info->i2c_client, mode_pretty_3);
			break;

		default:
			pr_err("%s: Invalid Pretty Value, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif	/* CONFIG_LOAD_FILE */
	if (err < 0)
		pr_err("%s: s5k5bafx_write_table() returned error, %d, %d\n", __func__, arg, err);

	return err;
}

#ifdef FACTORY_TEST
static int s5k5bafx_return_normal_preview(struct s5k5bafx_info *info)
{
	FUNC_ENTR;
	info->pdata->power_off();

	msleep(20);

	info->pdata->power_on();
	s5k5bafx_write_table(info->i2c_client, mode_table[S5K5BAFX_MODE_SENSOR_INIT]);

	return 0;
}
#endif

static long s5k5bafx_ioctl(struct file *file,
			unsigned int cmd, unsigned long arg)
{
	FUNC_ENTR;

	struct s5k5bafx_mode mode;
	struct s5k5bafx_info *info = file->private_data;

	switch (cmd) {
		case S5K5BAFX_IOCTL_SET_MODE:
			if (copy_from_user(&mode, (const void __user *)arg, sizeof(struct s5k5bafx_mode))) {
				pr_info("%s %d\n", __func__, __LINE__);
				return -EFAULT;
			}
			return s5k5bafx_set_mode(info, &mode);

		case S5K5BAFX_IOCTL_COLOR_EFFECT:
			return s5k5bafx_set_color_effect(info, (s5k5bafx_color_effect) arg);

		case S5K5BAFX_IOCTL_WHITE_BALANCE:
			return s5k5bafx_set_white_balance(info, (s5k5bafx_white_balance) arg);

		case S5K5BAFX_IOCTL_EXPOSURE:
			return s5k5bafx_set_exposure(info, (s5k5bafx_exposure) arg);
#ifdef FACTORY_TEST
		case S5K5BAFX_IOCTL_DTP_TEST:
		{
			int status;
			pr_err("[S5K5BAFX]%s: S5K5BAFX_IOCTL_DTP_TEST Entered!!! dtpTest = %d\n", __func__, (s5k5bafx_dtp_test) arg);
			if (dtpTest == 1 && (s5k5bafx_dtp_test) arg == 0){
				status = s5k5bafx_return_normal_preview(info);
			}
			dtpTest = (s5k5bafx_dtp_test) arg;
			return status;
		}
#endif
		default:
			return -EINVAL;
	}

	return 0;
}

static struct s5k5bafx_info *info;

static int s5k5bafx_open(struct inode *inode, struct file *file)
{
	FUNC_ENTR;

	int status;
#ifdef CONFIG_LOAD_FILE
	int err = -1;
#endif	/* CONFIG_LOAD_FILE */

	file->private_data = info;
	struct s5k5bafx_info * pinfo = (struct s5k5bafx_info *)file->private_data;

	if (pinfo->pdata && pinfo->pdata->power_on)
		pinfo->pdata->power_on();

#ifdef CONFIG_LOAD_FILE
	err = loadFile();
	if (unlikely(err)) {
		printk("%s: failed to init\n", __func__);
		return err;
	}
#endif
/*#ifdef CONFIG_LOAD_FILE
	err = loadFile();
	if (unlikely(err)) {
		printk("%s: failed to init\n", __func__);
		return err;
	}
	status = s5k5bafx_write_tuningmode(pinfo->i2c_client, "mode_sensor_init");
#else
	status = s5k5bafx_write_table(pinfo->i2c_client, mode_table[0]);
#endif

	if (status < 0)
		pinfo->pdata->power_off();
*/
	info->mode = SYSTEM_INITIALIZE_MODE;
	dtpTest = S5K5BAFX_DTP_TEST_OFF;

	//return status;
	return 0;
}

int s5k5bafx_release(struct inode *inode, struct file *file)
{
	FUNC_ENTR;

	struct s5k5bafx_info * pinfo = (struct s5k5bafx_info *)file->private_data;

	if (pinfo->pdata && pinfo->pdata->power_off)
		pinfo->pdata->power_off();
	return 0;
}


static const struct file_operations s5k5bafx_fileops = {
	.owner = THIS_MODULE,
	.open = s5k5bafx_open,
	.unlocked_ioctl = s5k5bafx_ioctl,
	.compat_ioctl = s5k5bafx_ioctl,
	.release = s5k5bafx_release,
};

static struct miscdevice s5k5bafx_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "s5k5bafx",
	.fops = &s5k5bafx_fileops,
};

static int s5k5bafx_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	FUNC_ENTR;

	int err;

	info = kzalloc(sizeof(struct s5k5bafx_info), GFP_KERNEL);
	if (!info) {
		pr_err("s5k5bafx: Unable to allocate memory!\n");
		return -ENOMEM;
	}
	err = misc_register(&s5k5bafx_device);
	if (err) {
		pr_err("s5k5bafx: Unable to register misc device!\n");
		kfree(info);
		return err;
	}

	info->i2c_client = client;

	if (client->dev.platform_data == NULL) {
		pr_err("s5k5bafx probe: client->dev.platform_data is NULL!\n");
		return -ENXIO;
	}

	info->pdata = client->dev.platform_data;

	i2c_set_clientdata(client, info);

	return 0;
}

static int s5k5bafx_remove(struct i2c_client *client)
{
	FUNC_ENTR;

	struct s5k5bafx_info *info;
	info = i2c_get_clientdata(client);
	misc_deregister(&s5k5bafx_device);
	kfree(info);
	return 0;
}

static const struct i2c_device_id s5k5bafx_id[] = {
	{ "s5k5bafx", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, s5k5bafx_id);

static struct i2c_driver s5k5bafx_i2c_driver = {
	.driver = {
		.name = "s5k5bafx",
		.owner = THIS_MODULE,
	},
	.probe = s5k5bafx_probe,
	.remove = s5k5bafx_remove,
	.id_table = s5k5bafx_id,
};

static int __init s5k5bafx_init(void)
{
	FUNC_ENTR;

	int status;
	pr_info("s5k5bafx sensor driver loading\n");

	status = i2c_add_driver(&s5k5bafx_i2c_driver);
	if (status) {
		printk(KERN_ERR "s5k5bafx error\n");
		return status;
	}
	return 0;
}

static void __exit s5k5bafx_exit(void)
{
	FUNC_ENTR;

	i2c_del_driver(&s5k5bafx_i2c_driver);
}

module_init(s5k5bafx_init);
module_exit(s5k5bafx_exit);
