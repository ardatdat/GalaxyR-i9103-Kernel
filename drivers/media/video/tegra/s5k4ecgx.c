/*
 * s5k4ecgx.c - s5k4ecgx sensor driver
 *
 * Copyright (C) 2010 Google Inc.
 *
 * Contributors:
 *      Rebecca Schultz Zavin <rebecca@android.com>
 *
 * s5k4ecgx.c
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
#include <media/s5k4ecgx.h>
#include <media/tegra_camera.h>
#include "s5k4ecgx_setting.h"
#include <linux/regulator/consumer.h>
#include <linux/delay.h>

#define DEBUG_PRINTS 0
#if DEBUG_PRINTS
	#define FUNC_ENTR	\
		printk(KERN_INFO "[S5K4ECGX] %s Entered!!!\n", __func__)
	#define FUNC_EXIT	\
		printk(KERN_INFO "[S5K4ECGX] %s Exited!!!\n", __func__)
#else
	#define FUNC_ENTR
	#define FUNC_EXIT
#endif

//#define CONFIG_LOAD_FILE 1

#ifdef CONFIG_LOAD_FILE
#include <linux/vmalloc.h>
#include <linux/mm.h>
/* #define max_size 200000 */

struct test {
	char data;
	struct test *nextBuf;
};

struct test *s5k4ecgx_testBuf;
#endif

struct s5k4ecgx_info {
	int mode;
	int oprmode;
	int framesize_index;	// for preview resolution
	int captureframesize_index;	// for capture resolution
	bool gLowLight_check;	// for lowLight check
	bool first_af_check;	// for first af result
	bool p_record;		// for 720p setting in record mode
	u8 af_status;		// for af start, stop
	u8 giso_index;		// for iso
	u8 geffect_index;	// for effect
	u8 gmeter_index;	// for exposure meter
	u8 gev_index;		// for ev
	u8 gwb_index;		// for wb
	u8 gsharpness_index;	// for sharpness
	u8 gsaturation_index;	// for saturation
	u8 gscene_index;	// for scene
	u8 gfocus_index;	// for Normal, Macro
	u8 af_flash_mode;	// for flash mode
	u8 af_result;		// for second af result
	u16 gLowLight_value;	// for lowLight value
	u8 gauto_cont_index;	// for auto contrast
	u8 gfps_range;		// for fps range
	u8 pre_flash_ae_skip;	// for When pre flash, 200ms delay is needed.
	u8 pre_flash_ae_stable_check;	// for When pre flash, AE stable check.
	u16 cam_mode;
//	u16 check_vender;	// for vender check
	struct i2c_client *i2c_client;
	struct s5k4ecgx_platform_data *pdata;
	bool touch_af_enable;
	bool flash_fired;
};

extern struct class *sec_class;
struct device *s5k4ecgx_dev;
extern struct i2c_client *i2c_client_pmic;

extern unsigned int system_rev;

struct regulator *reg_mipi_1v2;	//LDO17
struct s5k4ecgx_mode rear_mode;
struct s5k4ecgx_exif_info rear_exif_info;

/* extern struct i2c_client *i2c_client_pmic; */
#ifdef FACTORY_TEST
static s5k4ecgx_dtp_test dtpTest = S5K4ECGX_DTP_TEST_OFF;
#endif

static struct s5k4ecgx_info *info;

#define S5K4ECGX_MAX_RETRIES 5
#define S5K4ECGX_READ_STATUS_RETRIES 50
#define S5K4ECGX_AE_STABLE_RETRIES 3
#define S5K4ECGX_CAPTURE_DELAY_RETRIES 100
#define S5K4ECGX_RETURN_DELAY_RETRIES 100

enum {
	SYSTEM_INITIALIZE_MODE = 0,
	MONITOR_MODE,
	CAPTURE_MODE,
	RECORD_MODE
};

enum s5k4ecgx_oprmode {
	S5K4ECGX_OPRMODE_VIDEO = 0,
	S5K4ECGX_OPRMODE_IMAGE = 1,
};

enum s5k4ecgx_frame_size {
	S5K4ECGX_PREVIEW_720P = 0,	/* 720P - 1280x720*/
	S5K4ECGX_PREVIEW_WVGA,		/* WVGA - 800x480 */
	S5K4ECGX_PREVIEW_D1,		/* D1 - 720x480 */
	S5K4ECGX_PREVIEW_VGA,		/* VGA - 640x480 */
	S5K4ECGX_PREVIEW_MMS3,		/* MMS - 528x432 */
	S5K4ECGX_PREVIEW_VT,		/* VT - 352x288 */
	S5K4ECGX_PREVIEW_QVGA,		/* QVGA - 320x240 */
	S5K4ECGX_PREVIEW_MMS,		/* MMS - 176x144 */
	S5K4ECGX_CAPTURE_5M,		/* 5M - 2560x1920 */
	S5K4ECGX_CAPTURE_W4M,		/* W4M - 2560x1536 */
	S5K4ECGX_CAPTURE_3M,		/* 3M - 2048x1536 */
	S5K4ECGX_CAPTURE_W2M,		/* W2M	- 2048x1232 */
	S5K4ECGX_CAPTURE_WVGA,		/* WVGA - 800x480 */
	S5K4ECGX_CAPTURE_VGA,		/* VGA - 640x480 */
};

struct s5k4ecgx_enum_framesize {
	/* mode is 0 for preview, 1 for capture */
	enum s5k4ecgx_oprmode mode;
	unsigned int index;
	unsigned int width;
	unsigned int height;
};

static struct s5k4ecgx_enum_framesize s5k4ecgx_framesize_list[] = {
	{ S5K4ECGX_OPRMODE_VIDEO, S5K4ECGX_PREVIEW_720P,	1280,	720 },
	{ S5K4ECGX_OPRMODE_VIDEO, S5K4ECGX_PREVIEW_WVGA,	800,	480 },
	{ S5K4ECGX_OPRMODE_VIDEO, S5K4ECGX_PREVIEW_D1,		720,	480 },
	{ S5K4ECGX_OPRMODE_VIDEO, S5K4ECGX_PREVIEW_VGA, 	640,	480 },
	{ S5K4ECGX_OPRMODE_VIDEO, S5K4ECGX_PREVIEW_MMS3,		528,	432 },
	{ S5K4ECGX_OPRMODE_VIDEO, S5K4ECGX_PREVIEW_VT,		352,	288 },
	{ S5K4ECGX_OPRMODE_VIDEO, S5K4ECGX_PREVIEW_QVGA,	320,	240 },
	{ S5K4ECGX_OPRMODE_VIDEO, S5K4ECGX_PREVIEW_MMS,		176,	144 },
	{ S5K4ECGX_OPRMODE_IMAGE, S5K4ECGX_CAPTURE_5M,		2560,	1920 },
	{ S5K4ECGX_OPRMODE_IMAGE, S5K4ECGX_CAPTURE_W4M, 	2560,	1536 },
	{ S5K4ECGX_OPRMODE_IMAGE, S5K4ECGX_CAPTURE_3M,		2048,	1536 },
	{ S5K4ECGX_OPRMODE_IMAGE, S5K4ECGX_CAPTURE_W2M, 	2048,	1232 },
	{ S5K4ECGX_OPRMODE_IMAGE, S5K4ECGX_CAPTURE_WVGA,	800,	480 },
	{ S5K4ECGX_OPRMODE_IMAGE, S5K4ECGX_CAPTURE_VGA,		640,	480 },
};


static void s5k4ecgx_esdreset(struct s5k4ecgx_info *info)
{
	FUNC_ENTR;

	info->pdata->power_off();
	info->mode = SYSTEM_INITIALIZE_MODE;
	//msleep(500);
	usleep_range(500000, 600000);//500ms
	info->pdata->power_on();
}

static int s5k4ecgx_frame_delay(struct s5k4ecgx_info *info)
{
	FUNC_ENTR;

	if (info->cam_mode == S5K4ECGX_CAM_MODE_MAX)
		return 0;

	switch(info->gscene_index) {
		case S5K4ECGX_SCENE_FIRE_WORK:
		case S5K4ECGX_SCENE_NIGHT:
			// Night, Firework 1 frame delay of SOC processor
			//pr_err("func(%s):line(%d) 250ms delay\n",__func__, __LINE__);
			//msleep(250);
			usleep_range(250000, 350000);//250ms
			break;

		default:
			// 1 frame delay of SOC processor
			//pr_err("func(%s):line(%d) 100ms delay\n",__func__, __LINE__);
			//msleep(100);
			usleep_range(100000, 200000);//100ms
 			break;
	}

	FUNC_EXIT;

	return 0;
}

static int s5k4ecgx_write_reg_8(struct i2c_client *client, u8 addr, u8 val)
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
		pr_err("s5k4ecgx_write_reg_8: i2c transfer failed, retrying %x %x err : %d\n",
		       addr, val, err);
		//msleep(3);
		usleep_range(3000 * retry, 4000 * retry);//3ms
	} while (retry <= S5K4ECGX_MAX_RETRIES);

	return err;
}


static int s5k4ecgx_write_reg(struct i2c_client *client, u16 addr, u16 val)
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
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, retrying %x %x err:%d\n",
		       addr, val, err);
		//msleep(3);
		usleep_range(3000 * retry, 4000 * retry);//3ms
	} while (retry <= S5K4ECGX_MAX_RETRIES);

	return err;
}

int s5k4ecgx_write_table(struct i2c_client *client,
			      const struct s5k4ecgx_reg table[])
{
	int err;
	const struct s5k4ecgx_reg *next;
	u16 val;

	//FUNC_ENTR;

	for (next = table; next->addr != S5K4ECGX_TABLE_END; next++) {
		if (next->addr == S5K4ECGX_TABLE_WAIT_MS) {
			//msleep(next->val);
			usleep_range(next->val * 1000, (next->val + 1) * 1000);
			continue;
		}
		//printk(KERN_ERR "next->addr(%x):val(%x)\n",next->addr, next->val);

		val = next->val;

		err = s5k4ecgx_write_reg(client, next->addr, val);
		
		//udelay(10);	//preview limitation - GREEN
		//usleep_range(10, 20);//10us
		
		//printk(KERN_ERR "next->addr(%x):val(%x)\n",next->addr, next->val);
		if (err) {
			pr_err("s5k4ecgx_write_table: i2c transfer failed, 0x%x 0x%x err:%d\n", next->addr, val, err);
			return err;
		}
	}

	return 0;
}

#define BURST_MODE_BUFFER_MAX_SIZE 2700
unsigned char s5k4ecgx_buf_for_burstmode[BURST_MODE_BUFFER_MAX_SIZE];

static int s5k4ecgx_sensor_burst_write_list(struct i2c_client *client, const struct s5k4ecgx_reg table[], int size)
{
	int err = -EINVAL;
	int i = 0;
	int idx = 0;

	u16 subaddr = 0, next_subaddr = 0;
	u16 value = 0;

	struct i2c_msg msg = {client->addr, 0, 0, s5k4ecgx_buf_for_burstmode};

	FUNC_ENTR;

	for (i = 0; i < size; i++)
	{
		if(idx > (BURST_MODE_BUFFER_MAX_SIZE - 10))
		{
			printk("BURST MODE buffer overflow!!!\n");
			return err;
		}

		subaddr = table[i].addr;
		value = table[i].val;

		switch(subaddr)
		{
			case 0x0F12:
				{
					// make and fill buffer for burst mode write
					if(idx == 0)
					{
						s5k4ecgx_buf_for_burstmode[idx++] = 0x0F;
						s5k4ecgx_buf_for_burstmode[idx++] = 0x12;
					}
					s5k4ecgx_buf_for_burstmode[idx++] = value >> 8;
					s5k4ecgx_buf_for_burstmode[idx++] = value & 0xFF;


					//write in burstmode
					if(next_subaddr != 0x0F12)
					{
						msg.len = idx;
						err = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
						//printk("s5k5ccgx_sensor_burst_write, idx = %d\n",idx);
						idx=0;
					}

				}
				break;

			case 0xFFFE:
				{
					pr_info("burst_mode --- s5k4ecgx give delay: %d\n", value);
					//msleep(value);
					usleep_range(value * 1000, (value+1) * 1000);
				}
				break;
			case 0xFFFF:
				return 0;
				break;

			default:
				{
					idx = 0;
					//pr_info("burst_mode --- s5k4ecgx : 0x%x 0x%x\n", subaddr, value);
					err = s5k4ecgx_write_reg(client, subaddr, value);
				}
				break;
				

		}
	}

	FUNC_EXIT;

	if (unlikely(err < 0))
	{
		printk("%s: register set failed\n",__func__);
		return err;
	}
	return 0;

}

int s5k4ecgx_write_table_8(struct i2c_client *client,
			      const struct s5k4ecgx_reg_8 *table)
{
	int err;
	const struct s5k4ecgx_reg_8 *next;
	u8 val;

	for (next = table; next->addr != S5K4ECGX_TABLE_END_8; next++) {
		if (next->addr == S5K4ECGX_TABLE_WAIT_MS_8) {
			//msleep(next->val);
			usleep_range(next->val * 1000, (next->val + 1) * 1000);
			continue;
		}

		val = next->val;

		err = s5k4ecgx_write_reg_8(client, next->addr, val);
		if (err) {
			pr_err("s5k4ecgx_write_table_8: i2c transfer failed, 0x%x 0x%x err:%d\n", next->addr, val, err);
			return err;
		}
	}

	return 0;
}

static int s5k4ecgx_i2c_read_two(struct i2c_client *client, u8 *r_data, u16 length)
{
	int err;
	int retry = 0;
	struct i2c_msg msg[2];
	unsigned char data[2];

	if (!client->adapter)
		return -ENODEV;

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
		pr_err("s5k4ecgx_i2c_read_two: i2c transfer failed, retrying err:%d\n", err);
		//msleep(3);
		usleep_range(3000 * retry, 4000 * retry);//3ms
	} while (retry <= S5K4ECGX_MAX_RETRIES);

	return 0;
}

static int s5k4ecgx_read_reg(struct i2c_client *client, u16 val, u8 *r_data, u16 length)
{
	int err;
	int retry = 0;
	struct i2c_msg msg[2];
	unsigned char data[2];

	if (!client->adapter)
		return -ENODEV;

	err = s5k4ecgx_write_reg(client, 0xfcfc, 0xd000);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0xFCFC 0xD000\n");
		return err;
	}

	err = s5k4ecgx_write_reg(client, 0x002c, 0x7000);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0x002C 0x7000\n");
		return err;
	}
	err = s5k4ecgx_write_reg(client, 0x002e, val);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0x002E \n");
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
		pr_err("s5k4ecgx_read_reg: i2c transfer failed, retrying err:%d\n", err);
		//msleep(3);
		usleep_range(3000 * retry, 4000 * retry);//3ms
	} while (retry <= S5K4ECGX_MAX_RETRIES);

	return 0;
}

static int s5k4ecgx_check_vender(struct s5k4ecgx_info *info)
{
	int err;
	struct i2c_msg msg[2];
	unsigned char data[2];
	u8 r_data[2] = {0, 0};

	FUNC_ENTR;

	if (!info->i2c_client->adapter)
		return -ENODEV;

	err = s5k4ecgx_write_reg(info->i2c_client, 0xfcfc, 0xd000);
	if (err) {
		pr_err("%s:s5k4ecgx_write_reg: i2c transfer failed, 0xFCFC 0xD000\n",__func__);
		return err;
	}
	err = s5k4ecgx_write_reg(info->i2c_client, 0x0012, 0x0001);	// s/w core reset
	if (err) {
		pr_err("%s:s5k4ecgx_write_reg: i2c transfer failed, 0x0012 0x0001\n",__func__);
		return err;
	}
	err = s5k4ecgx_write_reg(info->i2c_client, 0x007a, 0x0000);	// clock enable to control block
	if (err) {
		pr_err("%s:s5k4ecgx_write_reg: i2c transfer failed, 0x007A 0x0000\n",__func__);
		return err;
	}
	err = s5k4ecgx_write_reg(info->i2c_client, 0xa000, 0x0004);	// make initail state
	if (err) {
		pr_err("%s:s5k4ecgx_write_reg: i2c transfer failed, 0xA000 0x0004\n",__func__);
		return err;
	}
	err = s5k4ecgx_write_reg(info->i2c_client, 0xa002, 0x0006);	// set the PAGE 6 of OTP
	if (err) {
		pr_err("%s:s5k4ecgx_write_reg: i2c transfer failed, 0xA002 0x0006\n",__func__);
		return err;
	}
	err = s5k4ecgx_write_reg(info->i2c_client, 0xa000, 0x0001);	// set read mode
	if (err) {
		pr_err("%s:s5k4ecgx_write_reg: i2c transfer failed, 0xA000 0x0001\n",__func__);
		return err;
	}
	/*
	msleep(50);

	err = s5k4ecgx_write_reg(info->i2c_client, 0xfcfc, 0xd000);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0xFCFC 0xD000\n");
		return err;
	}

	err = s5k4ecgx_write_reg(info->i2c_client, 0x002c, 0xd000);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0x002C 0xD000\n");
		return err;
	}
	err = s5k4ecgx_write_reg(info->i2c_client, 0x002e, 0xa006);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0x002E 0xA006\n");
		return err;
	}

	msg[0].addr = info->i2c_client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = data;

	// high byte goes out first
	data[0] = 0x0f;
	data[1] = 0x12;

	msg[1].addr = info->i2c_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = r_data;

	err = i2c_transfer(info->i2c_client->adapter, msg, 2);
	if (err != 2) {
		pr_err("s5k4ecgx_read_reg: i2c transfer failed, 0x%x\n", (unsigned int) r_data);
		return -EINVAL;
	}
	*/

	//pr_err("s5k4ecgx_check_vender: 0x%x 0x%x\n", r_data[0], r_data[1]);
	//info->check_vender = ((r_data[0] << 8) | r_data[1]);
	//pr_err("s5k4ecgx_check_vender: 0x%x\n", info->check_vender);

	return 0;
}

static int s5k4ecgx_write_AlgEn_reg(struct s5k4ecgx_info *info, u16 bit, u16 enable)
{
	int err;
	struct i2c_msg msg;
	unsigned char data[4];
	int retry = 0;
	u8 r_data[2] = {0,0};

	FUNC_ENTR;
	
	if (!info->i2c_client->adapter)
		return -ENODEV;

	s5k4ecgx_read_reg(info->i2c_client, 0x04e6, r_data, 2);
	//printk(KERN_ERR "func(%s):line(%d) r_data : 0x%x%x\n",__func__, __LINE__,r_data[0],r_data[1]);
	err = s5k4ecgx_write_reg(info->i2c_client, 0xfcfc, 0xd000);
	if (err) {
		pr_err("s5k4ecgx_write_table: i2c transfer failed, 0x0028 0x7000\n");
		return err;
	}

	err = s5k4ecgx_write_reg(info->i2c_client, 0x0028, 0x7000);
	if (err) {
		pr_err("s5k4ecgx_write_table: i2c transfer failed, 0x0028 0x7000\n");
		return err;
	}

	err = s5k4ecgx_write_reg(info->i2c_client, 0x002a, 0x04e6);
	if (err) {
		pr_err("s5k4ecgx_write_table: i2c transfer failed, 0x002A 0x04E6\n");
		return err;
	}

	data[0] = 0x0f;
	data[1] = 0x12;
	if( bit == 3 ){
		data[2] = r_data[0];
		switch(enable) {
			case 0 :
				data[3] = (r_data[1] & 0xF7);
				break;
 
			case 1 :
				data[3] = (r_data[1] | 0x08);
				break;
 
			default :
				data[3] = (r_data[1] | 0x08);
				break;
		}
		//printk(KERN_ERR "func(%s):line(%d) 0x%x%x\n",__func__, __LINE__,data[2],data[3]);
	} else if( bit == 5 ) {
		switch(enable) {
			case 0 :
				data[2] = (r_data[0] & 0xDF);
				break;
 
			case 1 :
				data[2] = (r_data[0] | 0x20);
				break;
 
			default :
				data[2] = (r_data[0] | 0x20);
				break;
		}
		data[3] = r_data[1];
		//printk(KERN_ERR "func(%s):line(%d) 0x%x%x\n",__func__, __LINE__,data[2],data[3]);
	} else {
		pr_err("s5k4ecgx_write_AlgEn_reg: bit is failed.");
		data[2] = r_data[0];
		data[3] = r_data[1];
	}

	msg.addr = info->i2c_client->addr;
	msg.flags = 0;
	msg.len = 4;
	msg.buf = data;

	s5k4ecgx_frame_delay(info);

	do {
		err = i2c_transfer(info->i2c_client->adapter, &msg, 1);
		if (err == 1) {
			return 0;
		}
		retry++;
		pr_err("s5k4ecgx_write_AlgEn_reg: i2c transfer failed, retry err:%d\n", err);
		//msleep(3);
		usleep_range(3000 * retry, 4000 * retry);//3ms
	} while (retry <= S5K4ECGX_MAX_RETRIES);

	if (err < 0)
		printk(KERN_ERR "func(%s) s5k4ecgx_write_AlgEn_reg end!\n", __func__);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_write_AlgEn_to_reg(struct s5k4ecgx_info *info, u16 enable)
{
	int err;
	struct i2c_msg msg;
	unsigned char to_data[4];
	int retry = 0;
	u8 r_data[2] = {0,0};

	FUNC_ENTR;
	
	if (!info->i2c_client->adapter)
		return -ENODEV;

	s5k4ecgx_read_reg(info->i2c_client, 0x04e6, r_data, 2);
	//intk(KERN_ERR "func(%s):line(%d) r_data : 0x%x%x\n",__func__, __LINE__,r_data[0],r_data[1]);
	err = s5k4ecgx_write_reg(info->i2c_client, 0xfcfc, 0xd000);
	if (err) {
		pr_err("s5k4ecgx_write_table: i2c transfer failed, 0x0028 0x7000\n");
		return err;
	}

	err = s5k4ecgx_write_reg(info->i2c_client, 0x0028, 0x7000);
	if (err) {
		pr_err("s5k4ecgx_write_table: i2c transfer failed, 0x0028 0x7000\n");
		return err;
	}

	err = s5k4ecgx_write_reg(info->i2c_client, 0x002a, 0x04e6);
	if (err) {
		pr_err("s5k4ecgx_write_table: i2c transfer failed, 0x002A 0x04E6\n");
		return err;
	}

	to_data[0] = 0x0f;
	to_data[1] = 0x12;
	if( (enable >= 0) && (enable <=3) ){
		to_data[2] = r_data[0];
		switch(enable) {
			case 0 :	//bit5 : 0 / bit3 : 0
				to_data[2] = (r_data[0] & 0xDF);
				to_data[3] = (r_data[1] & 0xF7);
				break;
 
			case 1 :	//bit5 : 0 / bit3 : 1
				to_data[2] = (r_data[0] & 0xDF);
				to_data[3] = (r_data[1] | 0x08);
				break;
 
			case 2 :	//bit5 : 1 / bit3 : 0
				to_data[2] = (r_data[0] | 0x20);
				to_data[3] = (r_data[1] & 0xF7);
				break;
 
			case 3 :	//bit5 : 1 / bit3 : 1
				to_data[2] = (r_data[0] | 0x20);
				to_data[3] = (r_data[1] | 0x08);
				break;
 
			default :	//bit5 : 0 / bit3 : 0
				to_data[2] = (r_data[0] & 0xDF);
				to_data[3] = (r_data[1] & 0xF7);
				break;
		}
		//intk(KERN_ERR "func(%s):line(%d) to_data : 0x%x%x\n",__func__, __LINE__,to_data[2],to_data[3]);
	} else {
		pr_err("s5k4ecgx_write_AlgEn_to_reg: bit is failed.");
		to_data[2] = r_data[0];
		to_data[3] = r_data[1];
	}

	msg.addr = info->i2c_client->addr;
	msg.flags = 0;
	msg.len = 4;
	msg.buf = to_data;

	s5k4ecgx_frame_delay(info);

	do {
		err = i2c_transfer(info->i2c_client->adapter, &msg, 1);
		if (err == 1) {
			return 0;
		}
		retry++;
		pr_err("s5k4ecgx_write_AlgEn_to_reg: i2c transfer failed, retry err:%d\n", err);
		//msleep(3);
		usleep_range(3000 * retry, 4000 * retry);//3ms
	} while (retry <= S5K4ECGX_MAX_RETRIES);

	FUNC_EXIT;

	if (err < 0)
		printk(KERN_ERR "func(%s) s5k4ecgx_write_AlgEn_to_reg end!\n", __func__);

	return err;
}

#ifdef CONFIG_LOAD_FILE
static inline int s5k4ecgx_write(struct i2c_client *client,
		unsigned int addr_reg, unsigned int data_reg)
{
	struct i2c_msg msg;
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

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 4;
	msg.buf = buf;

	/* printk("========s5k4ecgx_write=======(a:%x,d:%x)\n", addr_reg, data_reg); */
	/* (unsigned long *)buf = cpu_to_be32(packet); */

	ret = i2c_transfer(client->adapter, &msg, 1);

	if (unlikely(ret < 0)) {
		dev_err(&client->dev, "%s: (a:%x,d:%x) write failed\n", __func__, addr_reg, data_reg);
		return ret;
	}

	return (ret != 1) ? -1 : 0;
}

static char *s5k4ecgx_regs_table = NULL; 

static int s5k4ecgx_regs_table_size;

static int loadFile(void)

{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int ret;
	mm_segment_t fs = get_fs();

	printk("%s %d\n", __func__, __LINE__);

	set_fs(get_ds());
#if 0
	filp = filp_open("/data/camera/s5k4ecgx.h", O_RDONLY, 0);
#else
	filp = filp_open("/mnt/sdcard/s5k4ecgx_setting.h", O_RDONLY, 0);
#endif

	if (IS_ERR(filp)) 
	{
		printk("file open error\n");
		return PTR_ERR(filp);
	}

	l = filp->f_path.dentry->d_inode->i_size;
	printk("l = %ld\n", l);
	dp = kmalloc(l, GFP_KERNEL);
	if (dp == NULL) 
	{
		printk("Out of Memory\n");
		filp_close(filp, current->files);
	}

	pos = 0;
	memset(dp, 0, l);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);

	if (ret != l) 
	{
		printk("Failed to read file ret = %d\n", ret);
		kfree(dp);
		filp_close(filp, current->files);
		return -EINVAL;
	}

	filp_close(filp, current->files);

	set_fs(fs);

	s5k4ecgx_regs_table = dp;

	s5k4ecgx_regs_table_size = l;

	*((s5k4ecgx_regs_table + s5k4ecgx_regs_table_size) - 1) = '\0';

	printk("s5k4ecgx_reg_table_init\n");

	return 0;
}

void s5k4ecgx_regs_table_exit(void)
{
	printk("%s start\n", __func__);

	if (s5k4ecgx_regs_table) 
	{
		kfree(s5k4ecgx_regs_table);
		s5k4ecgx_regs_table = NULL;
	}

	printk("%s done\n", __func__);
}

static int s5k4ecgx_write_tuningmode(struct i2c_client *client, char *name)
{
	char *start, *end, *reg;
	unsigned short addr;
	unsigned int value;
	char reg_buf[7], data_buf[7]; //, len_buf[2];
	int err = -1;

	FUNC_ENTR;

	*(reg_buf + 6) = '\0';
	*(data_buf + 6) = '\0';
	//*(len_buf + 1) = '\0';

	//printk(KERN_ERR "func(%s) s5k4ecgx_write_tuningmode start!\n", __func__);

	start = strstr(s5k4ecgx_regs_table, name);
	end = strstr(start, "};");

	while (1) 
	{
		/* Find Address */
		reg = strstr(start,"{0x");
		if (reg)
		{
			start = (reg + 17);  //{0x000b, 0x0004},
		}

		if ((reg == NULL) || (reg > end))
		{
			break;
		}

		/* Write Value to Address */
		if (reg != NULL) 
		{
			memcpy(reg_buf, (reg + 1), 6);
			memcpy(data_buf, (reg + 9), 6);
			//memcpy(len_buf, (reg + 17), 1);
			addr = (unsigned short)simple_strtoul(reg_buf, NULL, 16); 
			value = (unsigned int)simple_strtoul(data_buf, NULL, 16); 
			//len = (unsigned int)simple_strtoul(len_buf, NULL, 10); 

			//printk(KERN_ERR "func(%s) 0x%04x, 0x%04x\n", __func__, addr, value);

			if(addr == S5K4ECGX_TABLE_END) {
				return 0;
			} else if (addr == S5K4ECGX_TABLE_WAIT_MS) {
				//msleep(value);
				usleep_range(value * 1000, (value + 1) * 1000);
				//printk(KERN_ERR "func(%s) delay 0x%04x, value 0x%04x\n", __func__, addr, value);
			} else {
				//s5k4ecgx_i2c_write_multi(client, addr, value);
				//ret = s5k4ecgx_write(client, addr, value);
				err = s5k4ecgx_write_reg(client, addr, value);
				//udelay(10);	//preview limitation - GREEN
				//usleep_range(10, 20);//10us
			}
		}
	}

	FUNC_EXIT;

	if (err < 0)
		printk(KERN_ERR "func(%s) s5k4ecgx_write_tuningmode end!\n", __func__);

	return 0;
}

#if 0
static int s5k4ecgx_write_tuningmode(struct i2c_client *client, char s_name[])
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
	tempData = &s5k4ecgx_testBuf[0];
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
				/* get 10 strings */
				data[0] = '0';
				for (i = 1; i < 11; i++) {
					data[i] = tempData->data;
					tempData = tempData->nextBuf;
				}
				/* printk("%s\n", data); */
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

		/* let search... */
		if ((temp & 0xFFFE0000) == 0xFFFE0000) {
			delay = temp & 0xFFFF;
			printk("func(%s):line(%d):delay(0x%x):delay(%d)\n", __func__, __LINE__, delay, delay);
			msleep(delay);
			continue;
		}
		/* ret = s5k4ecgx_write(client, temp); */

		addr_reg = (temp >> 16)&0xffff;
		data_reg = (temp & 0xffff);

		ret = s5k4ecgx_write(client, addr_reg, data_reg);

		/* printk("addr = %x\n", addr_reg); */
		/* printk("data = %x\n", data_reg); */

		/* In error circumstances */
		/* Give second shot */
		if (unlikely(ret)) {
			dev_info(&client->dev,
					"s5k4ecgx i2c retry one more time\n");
			ret = s5k4ecgx_write(client, addr_reg, data_reg);

			/* Give it one more shot */
			if (unlikely(ret)) {
				dev_info(&client->dev,
						"s5k4ecgx i2c retry twice\n");
				ret = s5k4ecgx_write(client, addr_reg, data_reg);
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
	struct test *nextBuf = s5k4ecgx_testBuf;
	int i = 0;

	int starCheck = 0;
	int check = 0;
	int ret = 0;
	loff_t pos;

	printk("%s\n", __func__);

	mm_segment_t fs = get_fs();
	set_fs(get_ds());

	fp = filp_open("/mnt/sdcard/s5k4ecgx_setting.h", O_RDONLY, 0);

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

	s5k4ecgx_testBuf = (struct test *)vmalloc(sizeof(struct test) * l);

	printk("%s: s5k4ecgx_testBuf = 0x%x\n", __func__, s5k4ecgx_testBuf);

	pos = 0;
	memset(nBuf, 0, l);
	memset(s5k4ecgx_testBuf, 0, l * sizeof(struct test));

	ret = vfs_read(fp, (char __user *)nBuf, l, &pos);

	if (ret != l) {
		printk("failed to read file ret = %d\n", ret);
		kfree(nBuf);
		kfree(s5k4ecgx_testBuf);
		filp_close(fp, current->files);
		return -1;
	}

	filp_close(fp, current->files);

	set_fs(fs);

	i = max_size;

	printk("i = %d\n", i);

	while (i) {
		s5k4ecgx_testBuf[max_size - i].data = *nBuf;
		if (i != 1) {
			s5k4ecgx_testBuf[max_size - i].nextBuf = &s5k4ecgx_testBuf[max_size - i + 1];
		} else {
			s5k4ecgx_testBuf[max_size - i].nextBuf = NULL;
			break;
		}
		i--;
		nBuf++;
	}

	i = max_size;
	nextBuf = &s5k4ecgx_testBuf[0];
#if 1
	while (i - 1) {
		if (!check && !starCheck) {
			if (s5k4ecgx_testBuf[max_size - i].data == '/') {
				if (s5k4ecgx_testBuf[max_size-i].nextBuf != NULL) {
					if (s5k4ecgx_testBuf[max_size-i].nextBuf->data == '/') {
						check = 1;	/* when find '\/\/' */
						i--;
					} else if (s5k4ecgx_testBuf[max_size-i].nextBuf->data == '*') {
						starCheck = 1;	/* when find '\/\*' */
						i--;
					}
				} else {
					break;
				}
			}
			if (!check && !starCheck)
				/* ignore '\t' */
				if (s5k4ecgx_testBuf[max_size - i].data != '\t') {
					nextBuf->nextBuf = &s5k4ecgx_testBuf[max_size-i];
					nextBuf = &s5k4ecgx_testBuf[max_size - i];
				}
		} else if (check && !starCheck) {
			if (s5k4ecgx_testBuf[max_size - i].data == '/') {
				if (s5k4ecgx_testBuf[max_size-i].nextBuf != NULL) {
					if (s5k4ecgx_testBuf[max_size-i].nextBuf->data == '*') {
						starCheck = 1;	/* when find '\/\*' */
						check = 0;
						i--;
					}
				} else {
					break;
				}
			}
			/* when find '\n' */
			if (s5k4ecgx_testBuf[max_size - i].data == '\n' && check) {
				check = 0;
				nextBuf->nextBuf = &s5k4ecgx_testBuf[max_size - i];
				nextBuf = &s5k4ecgx_testBuf[max_size - i];
			}
		} else if (!check && starCheck) {
			if (s5k4ecgx_testBuf[max_size - i].data == '*') {
				if (s5k4ecgx_testBuf[max_size-i].nextBuf != NULL) {
					if (s5k4ecgx_testBuf[max_size-i].nextBuf->data == '/') {
						starCheck = 0;	/* when find '\/\*' */
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
		if (s5k4ecgx_testBuf[max_size - i].nextBuf == NULL) {
			nextBuf = NULL;
			break;
		}
	}
#endif
#if 0
	printk("i = %d\n", i);
	nextBuf = &s5k4ecgx_testBuf[0];
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
#endif

static int s5k4ecgx_get_LowLightCondition(struct i2c_client *client, struct s5k4ecgx_info *info)
{
	int err = -1;
	u8 r_data[4] = {0,0,0,0};

	FUNC_ENTR;

	err = s5k4ecgx_write_reg(client, 0xfcfc, 0xd000);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0xFCFC 0xD000\n");
		return err;
	}

	err = s5k4ecgx_write_reg(client, 0x002c, 0x7000);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0x002C 0x7000\n");
		return err;
	}
	err = s5k4ecgx_write_reg(client, 0x002e, 0x2c18);
	if (err) {
		pr_err("s5k4ecgx_write_reg: i2c transfer failed, 0x002E 0x2c18\n");
		return err;
	}

	err = s5k4ecgx_i2c_read_two(client, r_data, 4);
//	printk(KERN_ERR "func(%s):line(%d) 0x%x %x %x %x\n",__func__, __LINE__, r_data[0], r_data[1], r_data[2], r_data[3]);

	if (err < 0)
		pr_err("%s: s5k4ecgx_get_LowLightCondition() returned error, %d\n", __func__, err);

	info->gLowLight_value = ((r_data[0] << 8)|(r_data[1])|(r_data[2] << 24)|(r_data[3] << 16));
	//printk(KERN_ERR "func(%s):line(%d) info->gLowLight_value : 0x%x\n",__func__, __LINE__,info->gLowLight_value);

	if(info->gLowLight_value < 0x0094){
		info->gLowLight_check = true;	//Low light
	} else {
		info->gLowLight_check = false;	//Not Lowlight
	}

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_get_framesize_index
	(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int i = -1;
	struct s5k4ecgx_enum_framesize *frmsize;
	int previewcapture_ratio = 0;

	FUNC_ENTR;
	
	if(info->oprmode == S5K4ECGX_OPRMODE_VIDEO) {
		//pr_err("%s: S5K4ECGX_OPRMODE_VIDEO Ratio: %dx%d\n", __func__, mode->xres, mode->yres);	//log for check
		previewcapture_ratio = (mode->xres * 10) / mode->yres;
	} else {
		//pr_err("%s: S5K4ECGX_OPRMODE_IMAGE Ratio: %dx%d\n", __func__, mode->cap_xres, mode->cap_yres);	//log for check
		if (info->cam_mode != S5K4ECGX_CAM_MODE_MAX) {
			previewcapture_ratio = (mode->cap_xres * 10) / mode->cap_yres;
		} else {
			previewcapture_ratio = (mode->xres * 10) / mode->yres;
		}

	}

	/*if(mode->PreviewActive && !mode->StillCount) {
		info->oprmode = S5K4ECGX_OPRMODE_VIDEO;
	} else if(!mode->PreviewActive && mode->StillCount) {
		info->oprmode = S5K4ECGX_OPRMODE_IMAGE; 
	} else {
		info->oprmode = S5K4ECGX_OPRMODE_VIDEO;
	}*/
	
	//printk(KERN_ERR "func(%s):line(%d)info->oprmode(%d)\n",__func__, __LINE__,info->oprmode);	//log for check

	/* Check for video/image mode */
	for(i = 0; i < (sizeof(s5k4ecgx_framesize_list)/sizeof(struct s5k4ecgx_enum_framesize)); i++){
		frmsize = &s5k4ecgx_framesize_list[i];

		if(frmsize->mode != info->oprmode)
			continue;

		/* In case of image capture mode, if the given image resolution is not supported,
 		 * return the next higher image resolution. */
		//must search wide
		//pr_err("%s: Res: frmsize %dx%d\n", __func__, frmsize->width, frmsize->height);	//log for check
		if(info->oprmode == S5K4ECGX_OPRMODE_VIDEO) {
			//pr_err("%s: S5K4ECGX_OPRMODE_VIDEO Requested Res: %dx%d\n", __func__, mode->xres, mode->yres);	//log for check
			if((frmsize->width == mode->xres) && (frmsize->height == mode->yres)) {
				return frmsize->index;
			}
		} else {
			//pr_err("%s: S5K4ECGX_OPRMODE_IMAGE Requested Res: %dx%d\n", __func__, mode->cap_xres, mode->cap_yres);	//log for check
			if (info->cam_mode != S5K4ECGX_CAM_MODE_MAX) {
				if((frmsize->width == mode->cap_xres) && (frmsize->height == mode->cap_yres)) {
					return frmsize->index;
				}
			} else {
				if((frmsize->width == mode->xres) && (frmsize->height == mode->yres)) {
					return frmsize->index;
				}
			}
		}
	} 

	/* If it fails, return the default value. */
	if (info->oprmode == S5K4ECGX_OPRMODE_VIDEO) {
		return (previewcapture_ratio > 15) ? S5K4ECGX_PREVIEW_WVGA : S5K4ECGX_PREVIEW_VGA;
	} else {
		return (previewcapture_ratio > 15) ? S5K4ECGX_CAPTURE_W4M : S5K4ECGX_CAPTURE_5M;
	}

	pr_err("%s: s5k4ecgx_get_framesize_index() returned error\n", __func__);

	return 0;
}

static int s5k4ecgx_set_frame_rate(struct s5k4ecgx_info *info, s5k4ecgx_frame_rate arg)
{
	int err = -1;
	
	FUNC_ENTR;

	if(info->gfps_range != arg) {
		printk(KERN_ERR "func(%s): fps is gfps_range :%d / arg %d\n", __func__, info->gfps_range, arg);
		info->gfps_range = arg;
	} else if(info->gfps_range == arg) {
		//printk(KERN_ERR "func(%s): fps is not changed gfps_range : %d / arg %d\n", __func__, info->gfps_range, arg);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch(arg) {
		case S5K4ECGX_FPS_AUTO:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_FPS_Auto");
			break;

		case S5K4ECGX_FPS_7:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_FPS_7");
			break;

		case S5K4ECGX_FPS_15:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_FPS_15");
			break;

		case S5K4ECGX_FPS_20:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_FPS_20");
			break;

		case S5K4ECGX_FPS_30:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_FPS_30");
			break;

		default:
			pr_err("%s: Invalid frame rate, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	switch(arg) {
		case S5K4ECGX_FPS_AUTO:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_FPS_Auto);
			break;

		case S5K4ECGX_FPS_7:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_FPS_7);
			break;

		case S5K4ECGX_FPS_15:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_FPS_15);
			break;

		case S5K4ECGX_FPS_20:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_FPS_20);
			break;

		case S5K4ECGX_FPS_30:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_FPS_30);
			break;

		default:
			pr_err("%s: Invalid frame rate, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif

	if (err < 0)
		pr_err("%s: s5k4ecgx_set_frame_rate() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_preview_size
	(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int err = -1;
	int index = info->framesize_index;

	FUNC_ENTR;

	pr_err("s5k4ecgx_set_preview_size ---------index : %d,mode->camcordmode : %d\n", index, mode->camcordmode);

#ifdef CONFIG_LOAD_FILE
	switch(index)
	{
		case S5K4ECGX_PREVIEW_720P:	/* 1280x720 for record*/
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_recording_720p");
			//info->p_record = true;
			break;

		case S5K4ECGX_PREVIEW_WVGA:	/* 800x480 */
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_800_Preview");
			break;

		case S5K4ECGX_PREVIEW_D1:	/* 720x480 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_720x480\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_recording_720x480");
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_720_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_720_Preview");
			}
			break;

		case S5K4ECGX_PREVIEW_MMS3:	/* 528x432 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_528x432\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_recording_176x144");
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_528_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_176_Preview");
			}
			break;

		case S5K4ECGX_PREVIEW_VGA:	/* 640x480 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_640x480\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_recording_640x480");
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_640_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_640_Preview");
			}
			break;

		case S5K4ECGX_PREVIEW_VT:	/* 356x244 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_352_Preview\n",__func__, __LINE__);
				//pr_err("func(%s):line(%d) Record is not setted this resolution\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_352_Preview");
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_352_Preview\n",__func__, __LINE__);
				//pr_err("func(%s):line(%d) VT is only setted this resolution\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_352_Preview");
			}
			// FPS Config - Default 15fps, vt call
			err = s5k4ecgx_set_frame_rate(info, S5K4ECGX_FPS_15);
			if(err < 0) {
				pr_err("%s: Preview fps set Failed\n", __func__);
				return -EINVAL;
			}
			break;

		case S5K4ECGX_PREVIEW_QVGA:	/* 320x240 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_320x240\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_recording_320x240");
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_320_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_320_Preview");
			}
			break;

		case S5K4ECGX_PREVIEW_MMS:	/* 176x144 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_176x144\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_recording_176x144");
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_176_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_176_Preview");
			}
			break;

		default:
			/* When running in image capture mode, the call comes here.
			 * Set the default video resolution - S5K4ECGX_PREVIEW_VGA
			 */ 
			pr_err("%s: invalid preview resolution supplied to set mode (%d x %d). info->index(%d), mode->camcordmode : %d\n",
							__func__, mode->xres, mode->yres, info->framesize_index, mode->camcordmode);
			return -EINVAL;
			break;
	}
#else
	switch(index)
	{
		case S5K4ECGX_PREVIEW_720P:	/* 1280x720 for record*/
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_recording_720p);
			//info->p_record = true;
			break;

		case S5K4ECGX_PREVIEW_WVGA:	/* 800x480 */
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_800_Preview);
			break;

		case S5K4ECGX_PREVIEW_D1:	/* 720x480 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_720x480\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_recording_720x480);
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_720_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_720_Preview);
			}
			break;

		case S5K4ECGX_PREVIEW_MMS3:	/* 528x432 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_528x432\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_recording_528x432);
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_528_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_528_Preview);
			}
			break;

		case S5K4ECGX_PREVIEW_VGA:	/* 640x480 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_640x480\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_recording_640x480);
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_640_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_640_Preview);
			}
			break;

		case S5K4ECGX_PREVIEW_VT:	/* 356x244 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_352_Preview\n",__func__, __LINE__);
				//pr_err("func(%s):line(%d) Record is not setted this resolution\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_352_Preview);
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_352_Preview\n",__func__, __LINE__);
				//pr_err("func(%s):line(%d) VT is only setted this resolution\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_352_Preview);
			}
			// FPS Config - Default 15fps, vt call
			err = s5k4ecgx_set_frame_rate(info, S5K4ECGX_FPS_15);
			if(err < 0) {
				pr_err("%s: Preview fps set Failed\n", __func__);
				return -EINVAL;
			}
			break;

		case S5K4ECGX_PREVIEW_QVGA:	/* 320x240 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_320x240\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_recording_320x240);
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_320_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_320_Preview);
			}
			break;

		case S5K4ECGX_PREVIEW_MMS:	/* 176x144 */
			if(mode->camcordmode == 1) {
				//pr_err("func(%s):line(%d) s5k4ecgx_recording_176x144\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_recording_176x144);
			} else {
				//pr_err("func(%s):line(%d) s5k4ecgx_176_Preview\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_176_Preview);
			}
			break;

		default:
			/* When running in image capture mode, the call comes here.
			 * Set the default video resolution - S5K4ECGX_PREVIEW_VGA
			 */ 
			pr_err("%s: invalid preview resolution supplied to set mode (%d x %d). info->index(%d), mode->camcordmode : %d\n",
							__func__, mode->xres, mode->yres, info->framesize_index, mode->camcordmode);
			return -EINVAL;
			break;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_preview_size() returned error, %d / index(%d)\n", __func__, err, index);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_capture_size
	(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int err = -1;
	//int index = info->framesize_index;

	FUNC_ENTR;

	info->oprmode = S5K4ECGX_OPRMODE_IMAGE;
	info->captureframesize_index = s5k4ecgx_get_framesize_index(info, mode);
	pr_err("s5k4ecgx_set_capture_size ---------index : %d\n", info->captureframesize_index);

#ifdef CONFIG_LOAD_FILE
	switch(info->captureframesize_index)
	{
		case S5K4ECGX_CAPTURE_5M: /* 2560x1920 */
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_5M_Capture");
			break;

		case S5K4ECGX_CAPTURE_W4M: /* 2560x1536 */
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_W4M_Capture");
			break;

		case S5K4ECGX_CAPTURE_3M: /* 2048x1536 */
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_3M_Capture");
			break;

		case S5K4ECGX_CAPTURE_W2M: /* 2048x1232 */
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_W2M_Capture");
			break;

		case S5K4ECGX_CAPTURE_WVGA: /* 800x480 */
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WVGA_Capture");
			break;

		case S5K4ECGX_CAPTURE_VGA: /* 640x480 */
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_VGA_Capture");
			break;

		default:
			/* The framesize index was not set properly. 
	 		 * Check s_fmt call - it must be for video mode. */	 
			pr_err("%s: invalid capture resolution supplied to set mode (%d x %d). info->captureframesize_index(%d)\n",
							__func__, mode->cap_xres, mode->cap_yres, info->captureframesize_index);

			return -EINVAL;
			break;
	}
#else
	switch(info->captureframesize_index)
	{
		case S5K4ECGX_CAPTURE_5M: /* 2560x1920 */
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_5M_Capture);
			break;

		case S5K4ECGX_CAPTURE_W4M: /* 2560x1536 */
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_W4M_Capture);
			break;

		case S5K4ECGX_CAPTURE_3M: /* 2048x1536 */
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_3M_Capture);
			break;

		case S5K4ECGX_CAPTURE_W2M: /* 2048x1232 */
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_W2M_Capture);
			break;

		case S5K4ECGX_CAPTURE_WVGA: /* 800x480 */
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WVGA_Capture);
			break;

		case S5K4ECGX_CAPTURE_VGA: /* 640x480 */
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_VGA_Capture);
			break;

		default:
			/* The framesize index was not set properly. 
	 		 * Check s_fmt call - it must be for video mode. */	 
			pr_err("%s: invalid capture resolution supplied to set mode (%d x %d). info->captureframesize_index(%d)\n",
							__func__, mode->cap_xres, mode->cap_yres, info->captureframesize_index);

			return -EINVAL;
			break;
	}
#endif

	/* Set capture image size */
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_capture_size() returned error, %d / info->captureframesize_index(%d)\n", __func__, err, info->captureframesize_index);

	FUNC_EXIT;

	if (info->cam_mode == S5K4ECGX_CAM_MODE_MAX) {
		s5k4ecgx_frame_delay(info);
	}

	return err;
}

static int s5k4ecgx_set_af_flash_control(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int err = -1;

	FUNC_ENTR;

#ifdef CONFIG_LOAD_FILE
	switch (info->af_flash_mode) {
		case S5K4ECGX_FLASH_AUTO:
			//printk(KERN_ERR "func(%s):line(%d) AF flash start.(AUTO)\n",__func__, __LINE__);
			//s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
			if (info->gLowLight_check == true) {
				//printk(KERN_ERR "func(%s):line(%d) AF flash AUTO - Low Light\n",__func__, __LINE__);
				if(info->gwb_index == S5K4ECGX_WB_AUTO) {
					err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_awb_unlock");
				}
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ae_unlock");
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Main_Flash_On");
				info->pdata->flash_onoff(1);
				info->flash_fired = true;
				rear_exif_info.info_flash = 1;
				/* ae delay 200ms of SOC processor */
				//msleep(200);
				//usleep_range(200000, 300000);//200ms
				
			} else {
				//printk(KERN_ERR "func(%s):line(%d) AF flash AUTO - Not Low Light\n",__func__, __LINE__);
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = 0;
			}
			break;

		case S5K4ECGX_FLASH_ON:
			//printk(KERN_ERR "func(%s):line(%d) AF flash start.(ON)\n",__func__, __LINE__);
			if(info->gwb_index == S5K4ECGX_WB_AUTO) {
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_awb_unlock");
			}
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ae_unlock");
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Main_Flash_On");
			info->pdata->flash_onoff(1);
			info->flash_fired = true;
			rear_exif_info.info_flash = 1;
			/* ae delay 200ms of SOC processor */
			//msleep(200);
			//usleep_range(200000, 300000);//200ms
			break;

		case S5K4ECGX_FLASH_OFF:
			//printk(KERN_ERR "func(%s):line(%d) AF flash start.(OFF)\n",__func__, __LINE__);
			err = 0;
			break;

		default:
			pr_err("%s: Invalid flash mode\n", __func__);
			return 0;
			break;
	}
#else
	switch (info->af_flash_mode) {
		case S5K4ECGX_FLASH_AUTO:
			//printk(KERN_ERR "func(%s):line(%d) AF flash start.(AUTO)\n",__func__, __LINE__);
			//s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
			if (info->gLowLight_check == true) {
				//printk(KERN_ERR "func(%s):line(%d) AF flash AUTO - Low Light\n",__func__, __LINE__);
				if(info->gwb_index == S5K4ECGX_WB_AUTO) {
					err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_awb_unlock);
				}
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ae_unlock);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Main_Flash_On);
				info->pdata->flash_onoff(1);
				info->flash_fired = true;
				rear_exif_info.info_flash = 1;
				/* ae delay 200ms of SOC processor */
				//msleep(200);
				//usleep_range(200000, 300000);//200ms
				
			} else {
				//printk(KERN_ERR "func(%s):line(%d) AF flash AUTO - Not Low Light\n",__func__, __LINE__);
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = 0;
			}
			break;

		case S5K4ECGX_FLASH_ON:
			//printk(KERN_ERR "func(%s):line(%d) AF flash start.(ON)\n",__func__, __LINE__);
			if(info->gwb_index == S5K4ECGX_WB_AUTO) {
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_awb_unlock);
			}
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ae_unlock);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Main_Flash_On);
			info->pdata->flash_onoff(1);
			info->flash_fired = true;
			rear_exif_info.info_flash = 1;
			/* ae delay 200ms of SOC processor */
			//msleep(200);
			//usleep_range(200000, 300000);//200ms
			break;

		case S5K4ECGX_FLASH_OFF:
			//printk(KERN_ERR "func(%s):line(%d) AF flash start.(OFF)\n",__func__, __LINE__);
			err = 0;
			break;	

		default:
			pr_err("%s: Invalid flash mode\n", __func__);
			return 0;
			break;
	}
#endif
	if (err < 0)
		pr_err("%s: %d, s5k4ecgx_set_af_flash_control() returned error, %d\n", __func__, __LINE__, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_get_exif_info(struct s5k4ecgx_info *info, struct s5k4ecgx_exif_info *exifinfo)
{
	u8 shutter_data0[2] = {0,0};
	u8 shutter_data1[2] = {0,0};
	u32 shutter_speed = 0;

	u8 iso_data0[2] = {0,0};
	u8 iso_data1[2] = {0,0};
	u16 iso_value = 0;
	
	FUNC_ENTR;
	
	/* get shutter speed(exposure time) for exif */
	s5k4ecgx_read_reg(info->i2c_client, 0x2bc0, shutter_data0, 2);
	s5k4ecgx_read_reg(info->i2c_client, 0x2bc2, shutter_data1, 2);

	shutter_speed = (shutter_data1[0] << 24)|(shutter_data1[1] << 16)|(shutter_data0[0] << 8)|(shutter_data0[1]);
	//printk(KERN_ERR "func(%s):line(%d) shutter_speed0(10:%d 16:%x)\n",__func__, __LINE__, (shutter_data0[0] << 8)|(shutter_data0[1]), (shutter_data0[0] << 8)|(shutter_data0[1]));
	//printk(KERN_ERR "func(%s):line(%d) shutter_speed1(10:%d 16:%x)\n",__func__, __LINE__, (shutter_data1[0] << 8)|(shutter_data1[1]), (shutter_data1[0] << 8)|(shutter_data1[1]));
	//printk(KERN_ERR "func(%s):line(%d) shutter_speed(%d)(%d)\n",__func__, __LINE__, shutter_speed, (shutter_speed*1000/400));

	/* get ISO for exif */
	s5k4ecgx_read_reg(info->i2c_client, 0x2bc4, iso_data0, 2);
	s5k4ecgx_read_reg(info->i2c_client, 0x2bc6, iso_data1, 2);
	iso_value = (((iso_data0[0] << 8)|(iso_data0[1])) * ((iso_data1[0] << 8)|(iso_data1[1]))) / 512;

	//printk(KERN_ERR "func(%s):line(%d) iso_data0(10:%d 16:%x)\n",__func__, __LINE__, ((iso_data0[0] << 8)|(iso_data0[1])), ((iso_data0[0] << 8)|(iso_data0[1])));
	//printk(KERN_ERR "func(%s):line(%d) iso_data1(10:%d 16:%x)\n",__func__, __LINE__, ((iso_data1[0] << 8)|(iso_data1[1])), ((iso_data1[0] << 8)|(iso_data1[1])));
	//printk(KERN_ERR "func(%s):line(%d) iso(%u)\n",__func__, __LINE__, iso_value);

	if (iso_value < 0x00C0) {
		//pr_err("%s: %d, iso 50\n", __func__, __LINE__);
		exifinfo->info_iso = 50;
	} else if (iso_value < 0x017F) {
		//pr_err("%s: %d, iso 100\n", __func__, __LINE__);
		exifinfo->info_iso = 100;
	} else if (iso_value < 0x0300) {
		//pr_err("%s: %d, iso 200\n", __func__, __LINE__);
		exifinfo->info_iso = 200;
	} else {
		//pr_err("%s: %d, iso 400\n", __func__, __LINE__);
		exifinfo->info_iso = 400;
	}

	exifinfo->info_exptime_numer = shutter_speed;
	exifinfo->info_exptime_denumer = 400000;

	FUNC_EXIT;

	return 0;
}

static int s5k4ecgx_get_af_flash_control(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int err = -1;

	FUNC_ENTR;

#ifdef CONFIG_LOAD_FILE
	switch (info->af_flash_mode) {
		case S5K4ECGX_FLASH_AUTO:
			//printk(KERN_ERR "func(%s):line(%d) AF flash stop.(AUTO) - Low Light\n",__func__, __LINE__);
			if (info->gLowLight_check == true) {
				info->pdata->flash_onoff(0);
				if (info->flash_fired) {
					info->flash_fired = false;
					err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Main_Flash_Off");
					/* ae delay 200ms of SOC processor */
					//msleep(200);
					//usleep_range(200000, 300000);//200ms
				}
			} else {
				printk(KERN_ERR "func(%s):line(%d) AF flash stop.(AUTO) - Not Low Light\n",__func__, __LINE__);
				err = 0;
			}
			break;

		case S5K4ECGX_FLASH_ON:
			//printk(KERN_ERR "func(%s):line(%d) AF flash stop(ON)\n",__func__, __LINE__);
			info->pdata->flash_onoff(0);
			info->flash_fired = false;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Main_Flash_Off");
			/* ae delay 200ms of SOC processor */
			//msleep(200);
			//usleep_range(200000, 300000);//200ms
			break;

		case S5K4ECGX_FLASH_OFF:
			//printk(KERN_ERR "func(%s):line(%d) AF flash stop.(OFF)\n",__func__, __LINE__);
			err = 0;
			break;

		default:
			pr_err("%s: Invalid flash mode\n", __func__);
			return 0;
			break;
	}
#else
	switch (info->af_flash_mode) {
		case S5K4ECGX_FLASH_AUTO:
			//printk(KERN_ERR "func(%s):line(%d) AF flash stop.(AUTO) - Low Light\n",__func__, __LINE__);
			if (info->gLowLight_check == true) {
				info->pdata->flash_onoff(0);
				if (info->flash_fired) {
					info->flash_fired = false;
					err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Main_Flash_Off);
					/* ae delay 200ms of SOC processor */
					//msleep(200);
					//usleep_range(200000, 300000);//200ms
				}
			} else {
				//printk(KERN_ERR "func(%s):line(%d) AF flash stop.(AUTO) - Not Low Light\n",__func__, __LINE__);
				err = 0;
			}
			break;

		case S5K4ECGX_FLASH_ON:
			//printk(KERN_ERR "func(%s):line(%d) AF flash stop(ON)\n",__func__, __LINE__);
			info->pdata->flash_onoff(0);
			info->flash_fired = false;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Main_Flash_Off);
			/* ae delay 200ms of SOC processor */
			//msleep(200);
			//usleep_range(200000, 300000);//200ms
			break;

		case S5K4ECGX_FLASH_OFF:
			//printk(KERN_ERR "func(%s):line(%d) AF flash stop.(OFF)\n",__func__, __LINE__);
			err = 0;
			break;

		default:
			pr_err("%s: Invalid flash mode\n", __func__);
			return 0;
			break;
	}
#endif

	if (err < 0)
		pr_err("%s: %d, s5k4ecgx_get_af_flash_control() returned error, %d\n", __func__, __LINE__, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_capture_mode
	(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int err = -1;
	int timeout_cnt = 0;
	u8 r_data[2] = {0,0};
	u16 capture_delay = 0;
	int capture_index = info->framesize_index;

	FUNC_ENTR;

	//pr_err("s5k4ecgx_set_capture_mode ---------index : %d\n", capture_index);

	if (info->cam_mode == S5K4ECGX_CAM_MODE_MAX) {
		err = s5k4ecgx_set_capture_size(info, mode);
		if(err < 0) {
			pr_err("%s: s5k4ecgx_set_capture_size() returned %d. capture_index: %d\n",
					__func__, err, capture_index);
			return -EINVAL;
		}
	}
	
	//s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
#ifdef CONFIG_LOAD_FILE
	if (info->gLowLight_check == true) {
		//printk(KERN_ERR "func(%s):line(%d) capture start - Low Light\n",__func__, __LINE__);
		switch (info->af_flash_mode) {
			case S5K4ECGX_FLASH_AUTO:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash AUTO\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						s5k4ecgx_set_af_flash_control(info, mode);
						break;
				}
				break;

			case S5K4ECGX_FLASH_ON:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash ON\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						s5k4ecgx_set_af_flash_control(info, mode);
						break;
				}
				break;

			default:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash default\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						// 1 frame delay of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
				}
				break;
		}
	} else {
		//printk(KERN_ERR "func(%s):line(%d) capture start - Not Low Light\n",__func__, __LINE__);
		switch (info->af_flash_mode) {
			case S5K4ECGX_FLASH_AUTO:
				printk(KERN_ERR "func(%s):line(%d) capture start - flash AUTO\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						//err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						s5k4ecgx_set_af_flash_control(info, mode);
						break;
				}
				break;

			case S5K4ECGX_FLASH_ON:
				printk(KERN_ERR "func(%s):line(%d) capture start - flash ON\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						//err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						s5k4ecgx_set_af_flash_control(info, mode);
						break;
				}
				break;

			default:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash default\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						//err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						// 1 frame delay of SOC processor
						//s5k4ecgx_frame_delay(info);
						break;
				}
				break;
		}
	}

	/* Capture start */
	err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Capture_Start");
#else
	if (info->gLowLight_check == true) {
		//printk(KERN_ERR "func(%s):line(%d) capture start - Low Light\n",__func__, __LINE__);
		switch (info->af_flash_mode) {
			case S5K4ECGX_FLASH_AUTO:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash AUTO\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						s5k4ecgx_set_af_flash_control(info, mode);
						break;
				}
				break;

			case S5K4ECGX_FLASH_ON:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash ON\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						s5k4ecgx_set_af_flash_control(info, mode);
						break;
				}
				break;

			default:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash default\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						// 1 frame delay of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
				}
				break;
		}
	} else {
		//printk(KERN_ERR "func(%s):line(%d) capture start - Not Low Light\n",__func__, __LINE__);
		switch (info->af_flash_mode) {
			case S5K4ECGX_FLASH_AUTO:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash AUTO\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						//err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_On");
						s5k4ecgx_set_af_flash_control(info, mode);
						break;
				}
				break;

			case S5K4ECGX_FLASH_ON:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash ON\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						s5k4ecgx_set_af_flash_control(info, mode);
						break;
				}
				break;

			default:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash default\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture start, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						// Night, Firework shot delay 250ms of SOC processor
						s5k4ecgx_frame_delay(info);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture start, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_On);
						// 1 frame delay of SOC processor
						//s5k4ecgx_frame_delay(info);
						break;
				}
				break;
		}
	}

	/* Capture start */
	err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Capture_Start);
#endif

	/* capture delay - polling is changed*/
	/*switch(info->gscene_index) {
		case S5K4ECGX_SCENE_FIRE_WORK:
		case S5K4ECGX_SCENE_NIGHT:
			//printk(KERN_ERR "func(%s):line(%d) capture delay, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
			// Night, Firework shot delay 300ms of SOC processor
			//msleep(300);
			usleep_range(300000, 400000);//300ms
			break;

		default:
			//printk(KERN_ERR "func(%s):line(%d) capture delay, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
			// Default shot delay 300ms of SOC processor
			//msleep(120);
			usleep_range(120000, 220000);//120ms
			break;
	}*/
	/* capture delay - polling is changed*/
	do {
		timeout_cnt++;
		//msleep(1);
		usleep_range(10000, 15000);	//10ms
		s5k4ecgx_read_reg(info->i2c_client, 0x0244, r_data, 2);
		capture_delay = ((r_data[0] << 8) & 0xFF00) | (r_data[1] & 0xFF);
		//printk(KERN_ERR "func(%s):line(%d) capture delay(0x%x) (%x/%x)\n",__func__, __LINE__, capture_delay, r_data[0], r_data[1]);
		if (timeout_cnt > S5K4ECGX_CAPTURE_DELAY_RETRIES) {
			pr_err("%s: Entering capture delay timed out \n", __func__);
			break;
		}
	} while ((capture_delay & 0xFFFF));

	/*switch(info->gscene_index) {
		case S5K4ECGX_SCENE_FIRE_WORK:
		case S5K4ECGX_SCENE_NIGHT:
			//printk(KERN_ERR "func(%s):line(%d) capture delay, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
			// Night, Firework shot delay 300ms of SOC processor
			//msleep(300);
			usleep_range(300000, 400000);//300ms
			break;

		default:
			do {
				timeout_cnt++;
				//msleep(1);
				usleep_range(10000, 15000);	//10ms
				s5k4ecgx_read_reg(info->i2c_client, 0x0244, r_data, 2);
				capture_delay = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
				//printk(KERN_ERR "func(%s):line(%d) capture delay(0x%x)\n",__func__, __LINE__, capture_delay);
				if (timeout_cnt > S5K4ECGX_CAPTURE_DELAY_RETRIES) {
					pr_err("%s: Entering capture delay timed out \n", __func__);
					break;
				}
			} while ((capture_delay & 0xFFFF));
	}*/

	/* Set capture mode */
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_capture_mode() returned error, %d / capture_index(%d)\n", __func__, err, capture_index);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_get_capture_mode
	(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int err = -1;
	int capture_index = info->framesize_index;

	FUNC_ENTR;
	//exif information
	//err = s5k4ecgx_get_exif_info(info, &rear_exif_info);

#ifdef CONFIG_LOAD_FILE
	if (info->gLowLight_check == true) {
		//printk(KERN_ERR "func(%s):line(%d) capture stop - Low Light\n",__func__, __LINE__);
		switch (info->af_flash_mode) {
			case S5K4ECGX_FLASH_AUTO:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash AUTO\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						s5k4ecgx_get_af_flash_control(info, mode);
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
				}
				break;

			case S5K4ECGX_FLASH_ON:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash ON\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						s5k4ecgx_get_af_flash_control(info, mode);
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
				}
				break;

			default:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash default\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
				}
				break;
		}
	} else {
		//printk(KERN_ERR "func(%s):line(%d) capture stop - Not Low Light\n",__func__, __LINE__);
		switch (info->af_flash_mode) {
			case S5K4ECGX_FLASH_AUTO:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash AUTO\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						break;
				}
				break;

			case S5K4ECGX_FLASH_ON:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash ON\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = 0;
						break;
				}
				break;

			default:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash default\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = 0;
						break;
				}
				break;
		}
	}

	/* Before preview return , ae & awb unlock */
	/*if(info->gwb_index == S5K4ECGX_WB_AUTO) {
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_awb_unlock");
	}
	err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ae_unlock");*/
#else
	if (info->gLowLight_check == true) {
		//printk(KERN_ERR "func(%s):line(%d) capture top - Low Light\n",__func__, __LINE__);
		switch (info->af_flash_mode) {
			case S5K4ECGX_FLASH_AUTO:
				//printk(KERN_ERR "func(%s):line(%d) capture start - flash AUTO\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						s5k4ecgx_get_af_flash_control(info, mode);
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
				}
				break;

			case S5K4ECGX_FLASH_ON:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash ON\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						s5k4ecgx_get_af_flash_control(info, mode);
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
				}
				break;

			default:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash default\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
				}
				break;
		}
	} else {
		//printk(KERN_ERR "func(%s):line(%d) capture stop - Not Low Light\n",__func__, __LINE__);
		switch (info->af_flash_mode) {
			case S5K4ECGX_FLASH_AUTO:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash AUTO\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = 0;
						break;
				}
				break;

			case S5K4ECGX_FLASH_ON:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash ON\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = 0;
						break;
				}
				break;

			default:
				//printk(KERN_ERR "func(%s):line(%d) capture stop - flash default\n",__func__, __LINE__);
				switch(info->gscene_index) {
					case S5K4ECGX_SCENE_FIRE_WORK:
					case S5K4ECGX_SCENE_NIGHT:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						/* Not have Night mode cap on */
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
						break;
		 
					default:
						//printk(KERN_ERR "func(%s):line(%d) capture stop, Not scene(NIGHT, FIREWORK)\n",__func__, __LINE__);
						err = 0;
						break;
				}
				break;
		}
	}

	/* Before preview return , ae & awb unlock */
	/*if(info->gwb_index == S5K4ECGX_WB_AUTO) {
		err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_awb_unlock);
	}
	err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ae_unlock);*/
#endif

	/* Set capture image size */
	if (err < 0)
		pr_err("%s: s5k4ecgx_get_capture_mode() returned error, %d / capture_index(%d)\n", __func__, err, capture_index);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_color_effect(struct s5k4ecgx_info *info, s5k4ecgx_color_effect arg)
{
	int err = -1;

	FUNC_ENTR;

/*	if(info->gscene_index != S5K4ECGX_SCENE_AUTO) {
		printk(KERN_ERR "func(%s) : Not scene mode is not setted color_effect\n", __func__);
		return 0;
	} else */
	if(info->geffect_index != arg) {
		printk(KERN_ERR "func(%s): info->geffect_index %d / arg %d\n", __func__, info->geffect_index, arg);
		info->geffect_index = arg;
	} else if(info->geffect_index == arg) {
		//printk(KERN_ERR "func(%s): color_effect mode is not changed\n", __func__);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_EFFECT_NONE:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Effect_Normal");
			break;

		case S5K4ECGX_EFFECT_MONO:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Effect_Black_White");
			break;

		case S5K4ECGX_EFFECT_SEPIA:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Effect_Sepia");
			break;

		case S5K4ECGX_EFFECT_NEGATIVE:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Effect_Negative");
			break;

		default:
			pr_err("%s: Invalid Color Effect, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	switch (arg) {
		case S5K4ECGX_EFFECT_NONE:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Effect_Normal);
			break;

		case S5K4ECGX_EFFECT_MONO:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Effect_Black_White);
			break;

		case S5K4ECGX_EFFECT_SEPIA:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Effect_Sepia);
			break;

		case S5K4ECGX_EFFECT_NEGATIVE:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Effect_Negative);
			break;

		default:
			pr_err("%s: Invalid Color Effect, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_color_effect() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_white_balance(struct s5k4ecgx_info *info, s5k4ecgx_white_balance arg)
{
	int err = -1;

	FUNC_ENTR;

	/*if(info->gscene_index != S5K4ECGX_SCENE_AUTO) {
		printk(KERN_ERR "func(%s): Not scene mode is not setted white_balance\n",__func__);
		//printk(KERN_ERR "func(%s):line(%d)info->gscene_index(%d)\n",__func__,__LINE__,info->gscene_index);
		return 0;
	} else */if(info->gwb_index != arg) {
		printk(KERN_ERR "func(%s): info->gwb_index %d / arg %d\n", __func__, info->gwb_index, arg);
		info->gwb_index = arg;
	} else if(info->gwb_index == arg) {
		//printk(KERN_ERR "func(%s): white balance mode is not changed\n", __func__);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_WB_AUTO:
			//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			break;

		case S5K4ECGX_WB_DAYLIGHT:
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Sunny");
			break;

		case S5K4ECGX_WB_INCANDESCENT:
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Tungsten");
			break;

		case S5K4ECGX_WB_FLUORESCENT:
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Fluorescent");
			break;

		case S5K4ECGX_WB_CLOUDY:
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Cloudy");
			break;

		default:
			pr_err("%s: Invalid White Balance, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	switch (arg) {
		case S5K4ECGX_WB_AUTO:
			//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			break;

		case S5K4ECGX_WB_DAYLIGHT:
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Sunny);
			break;

		case S5K4ECGX_WB_INCANDESCENT:
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Tungsten);
			break;

		case S5K4ECGX_WB_FLUORESCENT:
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Fluorescent);
			break;

		case S5K4ECGX_WB_CLOUDY:
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Cloudy);
			break;

		default:
			pr_err("%s: Invalid White Balance, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_white_balance() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

/*static int s5k4ecgx_set_saturation(struct s5k4ecgx_info *info, s5k4ecgx_saturation arg)
{
	FUNC_ENTR;

	int err = -1;

	printk(KERN_ERR "func(%s):line(%d)arg(%d)\n",__func__, __LINE__,arg);

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_SATURATION_P2P0:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Saturation_Plus_2");
			break;

		case S5K4ECGX_SATURATION_P1P0:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Saturation_Plus_1");
			break;

		case S5K4ECGX_SATURATION_ZERO:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Saturation_Default");
			break;

		case S5K4ECGX_SATURATION_M1P0:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Saturation_Minus_1");
			break;

		case S5K4ECGX_SATURATION_M2P0:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Saturation_Minus_2");
			break;

		default:
			pr_err("%s: Invalid Saturation Value, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	 switch (arg) {
	 	case S5K4ECGX_SATURATION_P2P0:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Saturation_Plus_2);
			break;

		case S5K4ECGX_SATURATION_P1P0:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Saturation_Plus_1);
			break;

		case S5K4ECGX_SATURATION_ZERO:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Saturation_Default);
			break;

		case S5K4ECGX_SATURATION_M1P0:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Saturation_Minus_1);
			break;

		case S5K4ECGX_SATURATION_M2P0:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Saturation_Minus_2);
			break;

		default:
			pr_err("%s: Invalid Saturation Value, %d\n", __func__, arg);
			return 0;
			break;
	 }
#endif
	
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_saturation() returned error, %d, %d\n", __func__, arg, err);

	return err;
}*/

/*static int s5k4ecgx_set_sharpness(struct s5k4ecgx_info *info, s5k4ecgx_sharpness arg)
{
	FUNC_ENTR;

	int err = -1;

	printk(KERN_ERR "func(%s):line(%d)arg(%d)\n",__func__, __LINE__,arg);

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_SHARPNESS_P2P0:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Sharpness_Plus_2");
			break;

		case S5K4ECGX_SHARPNESS_P1P0:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Sharpness_Plus_1");
			break;

		case S5K4ECGX_SHARPNESS_ZERO:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Sharpness_Default");
			break;

		case S5K4ECGX_SHARPNESS_M1P0:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Sharpness_Minus_1");
			break;

		case S5K4ECGX_SHARPNESS_M2P0:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Sharpness_Minus_2");
			break;

		default:
			pr_err("%s: Invalid Sharpness Value, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	 switch (arg) {
	 	case S5K4ECGX_SHARPNESS_P2P0:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Sharpness_Plus_2);
			break;

		case S5K4ECGX_SHARPNESS_P1P0:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Sharpness_Plus_1);
			break;

		case S5K4ECGX_SHARPNESS_ZERO:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Sharpness_Default);
			break;

		case S5K4ECGX_SHARPNESS_M1P0:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Sharpness_Minus_1);
			break;

		case S5K4ECGX_SHARPNESS_M2P0:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Sharpness_Minus_2);
			break;

		default:
			pr_err("%s: Invalid Sharpness Value, %d\n", __func__, arg);
			return 0;
			break;
	 }
#endif
	
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_sharpness() returned error, %d, %d\n", __func__, arg, err);

	return err;
}*/

static int s5k4ecgx_set_exposure(struct s5k4ecgx_info *info, s5k4ecgx_exposure arg)
{
	int err = -1;

	FUNC_ENTR;

	/*if(info->gscene_index != S5K4ECGX_SCENE_AUTO) {
		printk(KERN_ERR "func(%s): Not scene mode is not setted exposure\n", __func__);
		//printk(KERN_ERR "func(%s):line(%d)info->gscene_index(%d)\n",__func__,__LINE__,info->gscene_index);
		return 0;
	} else */if(info->gev_index != arg) {
		printk(KERN_ERR "func(%s): info->gev_index %d / arg %d\n", __func__, info->gev_index, arg);
		info->gev_index = arg;
	} else if(info->gev_index == arg) {
		//printk(KERN_ERR "func(%s): exposure mode is not changed\n", __func__);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_EXPOSURE_P2P0:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Plus_4");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Plus_4");
			break;

		case S5K4ECGX_EXPOSURE_P1P5:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Plus_3");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Plus_3");
			break;

		case S5K4ECGX_EXPOSURE_P1P0:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Plus_2");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Plus_2");
			break;

		case S5K4ECGX_EXPOSURE_P0P5:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Plus_1");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Plus_1");
			break;

		case S5K4ECGX_EXPOSURE_ZERO:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Default");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");
			break;

		case S5K4ECGX_EXPOSURE_M0P5:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Minus_1");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Minus_1");
			break;

		case S5K4ECGX_EXPOSURE_M1P0:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Minus_2");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Minus_2");
			break;

		case S5K4ECGX_EXPOSURE_M1P5:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Minus_3");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Minus_3");
			break;

		case S5K4ECGX_EXPOSURE_M2P0:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Camcorder_Minus_4");
			else
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Minus_4");
			break;

		default:
			pr_err("%s: Invalid Exposure Value, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	switch (arg) {
		case S5K4ECGX_EXPOSURE_P2P0:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Plus_4);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Plus_4);
			break;

		case S5K4ECGX_EXPOSURE_P1P5:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Plus_3);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Plus_3);
			break;

		case S5K4ECGX_EXPOSURE_P1P0:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Plus_2);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Plus_2);
			break;

		case S5K4ECGX_EXPOSURE_P0P5:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Plus_1);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Plus_1);
			break;

		case S5K4ECGX_EXPOSURE_ZERO:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Default);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);
			break;

		case S5K4ECGX_EXPOSURE_M0P5:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Minus_1);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Minus_1);
			break;

		case S5K4ECGX_EXPOSURE_M1P0:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Minus_2);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Minus_2);
			break;

		case S5K4ECGX_EXPOSURE_M1P5:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Minus_3);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Minus_3);
			break;

		case S5K4ECGX_EXPOSURE_M2P0:
			if (info->mode == RECORD_MODE)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Camcorder_Minus_4);
			else
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Minus_4);
			break;

		default:
			pr_err("%s: Invalid Exposure Value, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_exposure() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_exposure_meter(struct s5k4ecgx_info *info, s5k4ecgx_exposure_meter arg)
{
	int err = -1;

	FUNC_ENTR;

	/*if(info->gscene_index != S5K4ECGX_SCENE_AUTO) {
		printk(KERN_ERR "func(%s): Not scene mode is not setted exposure_meter\n",__func__);
		//printk(KERN_ERR "func(%s):line(%d)info->gscene_index(%d)\n",__func__,__LINE__,info->gscene_index);
		return 0;
	} else */if(info->gmeter_index != arg) {
		printk(KERN_ERR "func(%s): info->gmeter_index %d / arg %d\n", __func__, info->gmeter_index, arg);
		info->gmeter_index = arg;
	} else if(info->gmeter_index == arg) {
		//printk(KERN_ERR "func(%s): exposure meter mode is not changed\n", __func__);
		return 0;
	}


#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_EXPOSURE_METER_CENTER:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");
			break;

		case S5K4ECGX_EXPOSURE_METER_SPOT:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Spot");
			break;

		case S5K4ECGX_EXPOSURE_METER_MATRIX:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Matrix");
			break;

		default:
			pr_err("%s: Invalid Exposure meter, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	switch (arg) {
		case S5K4ECGX_EXPOSURE_METER_CENTER:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center);
			break;

		case S5K4ECGX_EXPOSURE_METER_SPOT:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Spot);
			break;

		case S5K4ECGX_EXPOSURE_METER_MATRIX:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Matrix);
			break;

		default:
			pr_err("%s: Invalid Exposure meter, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_exposure_meter() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_iso(struct s5k4ecgx_info *info, s5k4ecgx_iso arg)
{
	int err = -1;

	FUNC_ENTR;

	/*if(info->gscene_index != S5K4ECGX_SCENE_AUTO) {
		printk(KERN_ERR "func(%s): Not scene mode is not setted iso\n",__func__);
		//printk(KERN_ERR "func(%s):line(%d)info->gscene_index(%d)\n",__func__,__LINE__,info->gscene_index);
		return 0;
	} else */if(info->giso_index != arg) {
		printk(KERN_ERR "func(%s): info->giso_index %d / arg %d\n", __func__, info->giso_index, arg);
		info->giso_index = arg;
	} else if(info->giso_index == arg) {
		//printk(KERN_ERR "func(%s): iso mode is not changed\n", __func__);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_ISO_AUTO:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");
			break;

		case S5K4ECGX_ISO_50:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_50");
			break;

		case S5K4ECGX_ISO_100:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_100");
			break;

		case S5K4ECGX_ISO_200:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_200");
			break;

		case S5K4ECGX_ISO_400:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_400");
			break;

		default:
			pr_err("%s: Invalid Iso, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	switch (arg) {
		case S5K4ECGX_ISO_AUTO:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);
			break;

		case S5K4ECGX_ISO_50:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_50);
			break;

		case S5K4ECGX_ISO_100:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_100);
			break;

		case S5K4ECGX_ISO_200:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_200);
			break;

		case S5K4ECGX_ISO_400:
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_400);
			break;

		default:
			pr_err("%s: Invalid Iso, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_iso() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_focus_mode(struct s5k4ecgx_info *info, s5k4ecgx_focus_mode arg)
{
	int err = -1;

	FUNC_ENTR;

	/*if(info->gscene_index != S5K4ECGX_SCENE_AUTO) {
		printk(KERN_ERR "func(%s): Not scene mode is not setted focus\n",__func__);
		//printk(KERN_ERR "func(%s):line(%d)info->gscene_index(%d)\n",__func__,__LINE__,info->gscene_index);
		return 0;
	} else */if (info->gfocus_index != arg) {
		printk(KERN_ERR "func(%s): info->gfocus_index %d / arg %d\n", __func__, info->gfocus_index, arg);
		info->gfocus_index = arg;
	} else if (info->gfocus_index == arg) {
		//printk(KERN_ERR "func(%s): focus mode is not changed\n", __func__);
		return 0;
	}

	s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
#ifdef CONFIG_LOAD_FILE
	if (info->gLowLight_check == true) {
		switch (arg) {
			case S5K4ECGX_FOCUS_AUTO:
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_1");
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_2");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_3");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				break;

			case S5K4ECGX_FOCUS_MACRO:
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_Macro_mode_1");
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_Macro_mode_2");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_Macro_mode_3");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				break;

			case S5K4ECGX_FOCUS_FIXED:
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_1");
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_2");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_3");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				//err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Return_Inf_pos");
				break;

			default:
				pr_err("%s: Invalid focus mode(Low light), %d\n", __func__, arg);
				return 0;
				break;
		}
	} else {
		switch (arg) {
			case S5K4ECGX_FOCUS_AUTO:
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Normal_mode_1");
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Normal_mode_2");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Normal_mode_3");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				break;

			case S5K4ECGX_FOCUS_MACRO:
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Macro_mode_1");
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Macro_mode_2");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Macro_mode_3");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				break;

			case S5K4ECGX_FOCUS_FIXED:
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_1");
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_2");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_3");
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				//err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Return_Inf_pos");
				break;

			default:
				pr_err("%s: Invalid focus mode(Not Low light), %d\n", __func__, arg);
				return 0;
				break;
		}	
	}
#else
	if (info->gLowLight_check == true) {
		switch (arg) {
			case S5K4ECGX_FOCUS_AUTO:
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_1);
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_2);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_3);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				break;

			case S5K4ECGX_FOCUS_MACRO:
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_Macro_mode_1);
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_Macro_mode_2);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_Macro_mode_3);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				break;

			case S5K4ECGX_FOCUS_FIXED:
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_1);
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_2);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_3);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Return_Inf_pos);
				break;

			default:
				pr_err("%s: Invalid focus mode(Low light), %d\n", __func__, arg);
				return 0;
				break;
		}
	} else {
		switch (arg) {
			case S5K4ECGX_FOCUS_AUTO:
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_1);
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_2);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_3);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				break;

			case S5K4ECGX_FOCUS_MACRO:
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Macro_mode_1);
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Macro_mode_2);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Macro_mode_3);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				break;

			case S5K4ECGX_FOCUS_FIXED:
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_1);
				// 1 frame delay of SOC processor
				//s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_2);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_3);
				// 1 frame delay of SOC processor
				s5k4ecgx_frame_delay(info);
				//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Return_Inf_pos);
				break;

			default:
				pr_err("%s: Invalid focus mode(Not Low light), %d\n", __func__, arg);
				return 0;
				break;
		}	
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_focus_mode() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_flash_mode(struct s5k4ecgx_info *info, s5k4ecgx_flash_mode arg)
{
	int err = -1;

	FUNC_ENTR;

	if(info->af_flash_mode != arg) {
		printk(KERN_ERR "func(%s): info->af_flash_mode %d / arg %d\n", __func__, info->af_flash_mode, arg);
		info->af_flash_mode = arg;
	} else if(info->af_flash_mode == arg) {
		//printk(KERN_ERR "func(%s): flash mode is not changed\n", __func__);
		return 0;
	}

	switch (arg) {
		case S5K4ECGX_FLASH_AUTO:
			//printk(KERN_ERR "func(%s):line(%d) flash auto.\n",__func__, __LINE__);
			info->af_flash_mode = S5K4ECGX_FLASH_AUTO;
			err = 0;
			break;

		case S5K4ECGX_FLASH_ON:
			//printk(KERN_ERR "func(%s):line(%d) flash on.\n",__func__, __LINE__);
			info->af_flash_mode = S5K4ECGX_FLASH_ON;
			err = 0;
			break;

		case S5K4ECGX_FLASH_OFF:
			//printk(KERN_ERR "func(%s):line(%d) flash off.\n",__func__, __LINE__);
			info->pdata->torch_onoff(0);
			info->af_flash_mode = S5K4ECGX_FLASH_OFF;
			err = 0;
			break;

		case S5K4ECGX_FLASH_TORCH:
			//printk(KERN_ERR "func(%s):line(%d) flash torch.\n",__func__, __LINE__);
			info->pdata->torch_onoff(1);
			info->af_flash_mode = S5K4ECGX_FLASH_TORCH;
			err = 0;
			break;

		default:
			pr_err("%s: Invalid flash mode, %d\n", __func__, arg);
			return 0;
			break;
	}

	if (err < 0)
		pr_err("%s: s5k4ecgx_set_flash_mode() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_scene_mode(struct s5k4ecgx_info *info, s5k4ecgx_scene_mode arg)
{
	int err = -1;

	FUNC_ENTR;

	if(arg == S5K4ECGX_SCENE_TEXT) {
		err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_MACRO);		//focus - macro
	} else {
		err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);		//focus - auto
	}

	if(info->gscene_index != arg) {
		printk(KERN_ERR "func(%s): info->gscene_index %d / arg %d\n", __func__, info->gscene_index, arg);
		info->gscene_index = arg;
	}

#ifdef CONFIG_LOAD_FILE
	err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Default");

	switch (arg) {
		case S5K4ECGX_SCENE_AUTO:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF;
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			//err = s5k4ecgx_set_focus_mode(info, info->gfocus_index);					//focus
			break;

		case S5K4ECGX_SCENE_PORTRAIT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Portrait");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			//info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			//err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				/focus
			}*/
			break;

		case S5K4ECGX_SCENE_LANDSCAPE:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Landscape");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_MATRIX;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Matrix");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_SPORTS:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Sports");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_PARTY:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Party_Indoor");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			//info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			//err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_BEACH:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Beach_Snow");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_P1P0;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Plus_1");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_SUNSET:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 2);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Sunset");
			info->gwb_index = S5K4ECGX_WB_DAYLIGHT;	
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Sunny");			//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_DUSK_DAWN:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 2);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Duskdawn");
			info->gwb_index = S5K4ECGX_WB_FLUORESCENT;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Fluorescent");		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_FALL_COLOR:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Fall_Color");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_NIGHT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Nightshot");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_BACK_LIGHT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Backlight");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			if(info->af_flash_mode = S5K4ECGX_FLASH_ON) {
				info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");	//exposure_meter
			} else if(info->af_flash_mode = S5K4ECGX_FLASH_OFF) {
				info->gmeter_index = S5K4ECGX_EXPOSURE_METER_SPOT;
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Spot");	//exposure_meter
			}
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			//info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			//err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_FIRE_WORK:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Fireworks");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		case S5K4ECGX_SCENE_TEXT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Text");
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Auto");			//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			//info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			//err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			//err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_MACRO);					//focus
			break;

		case S5K4ECGX_SCENE_CANDLE_LIGHT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 2);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Scene_Candle_Light");
			info->gwb_index = S5K4ECGX_WB_DAYLIGHT;	
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_WB_Sunny");			//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ISO_Auto");			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Metering_Center");		//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_EV_Default");		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);				//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);				//focus
			}*/
			break;

		default:
			pr_err("%s: Invalid Scene mode, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Default);

	switch (arg) {
		case S5K4ECGX_SCENE_AUTO:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			//err = s5k4ecgx_set_focus_mode(info, info->gfocus_index);				//focus
			break;

		case S5K4ECGX_SCENE_PORTRAIT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Portrait);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			//info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			//err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);		//focus
			}*/
			break;

		case S5K4ECGX_SCENE_LANDSCAPE:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Landscape);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_MATRIX;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Matrix); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_SPORTS:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Sports);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_PARTY:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Party_Indoor);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_200;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_200); 		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			//info->af_flash_mode = S5K4ECGX_FLASH_OFF;
			//err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_BEACH:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Beach_Snow);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_50;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_50);			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_P1P0;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Plus_1);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_SUNSET:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 2);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Sunset);
			info->gwb_index = S5K4ECGX_WB_DAYLIGHT; 
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Sunny);		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_DUSK_DAWN:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 2);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Duskdawn);
			info->gwb_index = S5K4ECGX_WB_FLUORESCENT;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Fluorescent);		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_FALL_COLOR:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Fall_Color);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_NIGHT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Nightshot);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_BACK_LIGHT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Backlight);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			if (info->af_flash_mode == S5K4ECGX_FLASH_ON) {
				info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); //exposure_meter
			} else if (info->af_flash_mode == S5K4ECGX_FLASH_OFF) {
				info->gmeter_index = S5K4ECGX_EXPOSURE_METER_SPOT;
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Spot);	//exposure_meter
			}
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			//info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			//err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_FIRE_WORK:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 0);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 1);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Fireworks);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_50;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_50);			//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		case S5K4ECGX_SCENE_TEXT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 1);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 3);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Text);
			info->gwb_index = S5K4ECGX_WB_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Auto); 		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			//info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			//err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			//err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_MACRO);				//focus
			break;

		case S5K4ECGX_SCENE_CANDLE_LIGHT:
			/*
			err = s5k4ecgx_write_AlgEn_reg(info, 5, 1);
			err = s5k4ecgx_write_AlgEn_reg(info, 3, 0);
			*/
			err = s5k4ecgx_write_AlgEn_to_reg(info, 2);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Scene_Candle_Light);
			info->gwb_index = S5K4ECGX_WB_DAYLIGHT; 
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_WB_Sunny);		//wb
			info->giso_index = S5K4ECGX_ISO_AUTO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ISO_Auto);		//iso
			info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Metering_Center); 	//exposure_meter
			info->gev_index = S5K4ECGX_EXPOSURE_ZERO;
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_EV_Default);		//exposure
			info->af_flash_mode = S5K4ECGX_FLASH_OFF; 
			err = s5k4ecgx_set_flash_mode(info, info->af_flash_mode);			//flash
			/*if(info->gfocus_index == S5K4ECGX_FOCUS_MACRO) {
				err = s5k4ecgx_set_focus_mode(info, S5K4ECGX_FOCUS_AUTO);			//focus
			}*/
			break;

		default:
			pr_err("%s: Invalid Scene mode, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif

	FUNC_EXIT;

	if (err < 0)
		pr_err("%s: s5k4ecgx_set_scene_mode() returned error, %d, %d\n", __func__, arg, err);

	return err;
}

static int s5k4ecgx_set_auto_contrast(struct s5k4ecgx_info *info, s5k4ecgx_auto_contrast arg)
{
	int err = -1;

	FUNC_ENTR;

	if(info->gauto_cont_index != arg) {
		printk(KERN_ERR "func(%s): info->gauto_cont_index %d / arg %d\n", __func__, info->gauto_cont_index, arg);
		info->gauto_cont_index = arg;
	} else if(info->gauto_cont_index == arg) {
		//printk(KERN_ERR "func(%s): auto contrast is not changed\n", __func__);
		return 0;
	}

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_AUTO_CONTRAST_OFF:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Auto_Contrast_OFF");
			break;

		case S5K4ECGX_AUTO_CONTRAST_ON:
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Auto_Contrast_ON");
			break;

		default:
			pr_err("%s: Invalid auto contrast, %d\n", __func__, arg);
			return 0;
			break;
	}
#else
	switch (arg) {
		case S5K4ECGX_AUTO_CONTRAST_OFF:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Auto_Contrast_OFF);
			break;

		case S5K4ECGX_AUTO_CONTRAST_ON:
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Auto_Contrast_ON);
			break;

		default:
			pr_err("%s: Invalid auto contrast, %d\n", __func__, arg);
			return 0;
			break;
	}
#endif

	if (err < 0)
		pr_err("%s: s5k4ecgx_write_table() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

#if 0
static int s5k4ecgx_set_pre_flash_control(struct s5k4ecgx_info *info, s5k4ecgx_flash_mode arg)
{
	int err = 0;
	int timeout_cnt = 0;
	u8 r_data[2] = {0,0};
	u8 ae_data = 0;

	FUNC_ENTR;

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_FLASH_AUTO:
			s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
			if (info->gLowLight_check == true) {
				//printk(KERN_ERR "func(%s):line(%d) Pre flash AUTO - Low Light\n",__func__, __LINE__);
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Pre_Flash_On");
				info->pdata->torch_onoff(1);
				info->flash_fired = true;
				//msleep(200); //recommand from module team
				//usleep_range(100000, 200000); //recommand from module team
				do {
					timeout_cnt++;
					//msleep(1);
					usleep_range(200000, 250000);//1ms
					s5k4ecgx_read_reg(info->i2c_client, 0x2c74, r_data, 2);
					ae_data = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
					//printk(KERN_ERR "func(%s):line(%d) AE stabilization(0x%x)\n",__func__, __LINE__, ae_data);
					if (timeout_cnt > S5K4ECGX_AE_STABLE_RETRIES) {
						pr_err("%s: Entering AE stabilization timed out \n", __func__);
						break;
					}
				} while (!(ae_data & 0xFFFF));
			} else {
				//printk(KERN_ERR "func(%s):line(%d) Pre flash AUTO - Not Low Light\n",__func__, __LINE__);
				err = 0;
			}
			break;

		case S5K4ECGX_FLASH_ON:
			//printk(KERN_ERR "func(%s):line(%d) Pre flash ON\n",__func__, __LINE__);
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Pre_Flash_On");
			info->pdata->torch_onoff(1);
			info->flash_fired = true;
			//msleep(200); //recommand from module team
			//usleep_range(100000, 200000); //recommand from module team
			do {
				timeout_cnt++;
				//msleep(1);
				usleep_range(200000, 250000);//1ms
				s5k4ecgx_read_reg(info->i2c_client, 0x2c74, r_data, 2);
				ae_data = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
				//printk(KERN_ERR "func(%s):line(%d) AE stabilization(0x%x)\n",__func__, __LINE__, ae_data);
				if (timeout_cnt > S5K4ECGX_AE_STABLE_RETRIES) {
					pr_err("%s: Entering AE stabilization timed out \n", __func__);
					break;
				}
			} while (!(ae_data & 0xFFFF));
			//printk(KERN_ERR "func(%s):line(%d) AE stabilization AE(0x%x)\n",__func__, __LINE__, ae_data);
			//printk(KERN_ERR "func(%s):line(%d) timeout_cnt(%d)\n",__func__, __LINE__, timeout_cnt);
			break;

		case S5K4ECGX_FLASH_OFF:
			//printk(KERN_ERR "func(%s):line(%d) Pre flash OFF\n",__func__, __LINE__);
			info->pdata->torch_onoff(0);
			//err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Pre_Flash_Off");
			break;

		case S5K4ECGX_FLASH_TORCH:
			//printk(KERN_ERR "func(%s):line(%d) Torch\n",__func__, __LINE__);
			info->pdata->torch_onoff(1);
			break;

		default:
			printk(KERN_ERR "func(%s):line(%d) wrong flash mode\n",__func__, __LINE__);
			return -EFAULT;
	}
#else
	switch (arg) {
		case S5K4ECGX_FLASH_AUTO:
			s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
			if (info->gLowLight_check == true) {
				//printk(KERN_ERR "func(%s):line(%d) Pre flash AUTO - Low Light\n",__func__, __LINE__);
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Pre_Flash_On);
				info->pdata->torch_onoff(1);
				info->flash_fired = true;
				//msleep(200); //recommand from module team
				//usleep_range(100000, 200000); //recommand from module team
				do {
					timeout_cnt++;
					//msleep(1);
					usleep_range(200000, 250000);//1ms
					s5k4ecgx_read_reg(info->i2c_client, 0x2c74, r_data, 2);
					ae_data = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
					//printk(KERN_ERR "func(%s):line(%d) AE stabilization(0x%x)\n",__func__, __LINE__, ae_data);
					if (timeout_cnt > S5K4ECGX_AE_STABLE_RETRIES) {
						pr_err("%s: Entering AE stabilization timed out \n", __func__);
						break;
					}
				} while (!(ae_data & 0xFFFF));
			} else {
				//printk(KERN_ERR "func(%s):line(%d) Pre flash AUTO - Not Low Light\n",__func__, __LINE__);
				err = 0;
			}
			break;

		case S5K4ECGX_FLASH_ON:
			//printk(KERN_ERR "func(%s):line(%d) Pre flash ON\n",__func__, __LINE__);
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Pre_Flash_On);
			info->pdata->torch_onoff(1);
			info->flash_fired = true;
			//msleep(200); //recommand from module team
			//usleep_range(100000, 200000); //recommand from module team
			do {
				timeout_cnt++;
				//msleep(1);
				usleep_range(200000, 250000);//1ms
				s5k4ecgx_read_reg(info->i2c_client, 0x2c74, r_data, 2);
				ae_data = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
				//printk(KERN_ERR "func(%s):line(%d) AE stabilization(0x%x)\n",__func__, __LINE__, ae_data);
				if (timeout_cnt > S5K4ECGX_AE_STABLE_RETRIES) {
					pr_err("%s: Entering AE stabilization timed out \n", __func__);
					break;
				}
			} while (!(ae_data & 0xFFFF));
			//printk(KERN_ERR "func(%s):line(%d) AE stabilization AE(0x%x)\n",__func__, __LINE__, ae_data);
			//printk(KERN_ERR "func(%s):line(%d) timeout_cnt(%d)\n",__func__, __LINE__, timeout_cnt);
			break;

		case S5K4ECGX_FLASH_OFF:
			//printk(KERN_ERR "func(%s):line(%d) Pre flash OFF\n",__func__, __LINE__);
			info->pdata->torch_onoff(0);
			//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Pre_Flash_Off);
			break;

		case S5K4ECGX_FLASH_TORCH:
			//printk(KERN_ERR "func(%s):line(%d) Torch\n",__func__, __LINE__);
			info->pdata->torch_onoff(1);
			break;

		default:
			printk(KERN_ERR "func(%s):line(%d) wrong flash mode\n",__func__, __LINE__);
			return -EFAULT;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_pre_flash_control() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}
#endif

static int s5k4ecgx_set_af_control(struct s5k4ecgx_info *info, s5k4ecgx_autofocus_control arg)
{
	int err = -1;

	FUNC_ENTR;

#ifdef CONFIG_LOAD_FILE
	switch (arg) {
		case S5K4ECGX_AF_START:
			/*switch (info->af_flash_mode) {
				case S5K4ECGX_FLASH_AUTO:
				case S5K4ECGX_FLASH_ON:
					// Low Light
					//printk(KERN_ERR "func(%s):line(%d) 1st AF Flash\n",__func__, __LINE__);
					s5k4ecgx_set_pre_flash_control(info, info->af_flash_mode);
					break;
				default:
					//printk(KERN_ERR "func(%s):line(%d) 1st AF NOT Flash\n",__func__, __LINE__);
					break;
			}*/

			switch (info->af_flash_mode) {
				case S5K4ECGX_FLASH_AUTO:
					s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
					if (info->gLowLight_check == true) {
						//printk(KERN_ERR "func(%s):line(%d) Pre flash AUTO - Low Light\n",__func__, __LINE__);
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Pre_Flash_On");
						info->pdata->torch_onoff(1);
						info->flash_fired = true;
						info->pre_flash_ae_stable_check = 0;
					} else {
						//printk(KERN_ERR "func(%s):line(%d) Pre flash AUTO - Not Low Light\n",__func__, __LINE__);
						info->pdata->torch_onoff(0);
						if(info->gwb_index == S5K4ECGX_WB_AUTO && !info->touch_af_enable)
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_awb_lock");
						if (!info->touch_af_enable) {
							if (info->cam_mode != S5K4ECGX_CAM_MODE_MAX) {
								err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ae_lock");
							}
						}
						s5k4ecgx_frame_delay(info);
						err = s5k4ecgx_write_tuningmode(info->i2c_client, s5k4ecgx_Single_AF_Start);
						info->touch_af_enable = false;
						info->pre_flash_ae_stable_check = 1;
						err = 0;
					}
					break;
			
				case S5K4ECGX_FLASH_ON:
					//printk(KERN_ERR "func(%s):line(%d) Pre flash ON\n",__func__, __LINE__);
					err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Pre_Flash_On");
					info->pdata->torch_onoff(1);
					info->flash_fired = true;
					info->pre_flash_ae_stable_check = 0;
					break;
			
				case S5K4ECGX_FLASH_OFF:
					//printk(KERN_ERR "func(%s):line(%d) Pre flash OFF\n",__func__, __LINE__);
					info->pdata->torch_onoff(0);
					if(info->gwb_index == S5K4ECGX_WB_AUTO && !info->touch_af_enable)
						err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_awb_lock");
					if (!info->touch_af_enable) {
						if (info->cam_mode != S5K4ECGX_CAM_MODE_MAX) {
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ae_lock");
						}
					}
					s5k4ecgx_frame_delay(info);
					err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Single_AF_Start");
					info->touch_af_enable = false;
					info->pre_flash_ae_stable_check = 1;
					break;
			
				case S5K4ECGX_FLASH_TORCH:
					info->pdata->torch_onoff(1);
					info->pre_flash_ae_stable_check = 1;
					break;
			
				default:
					printk(KERN_ERR "func(%s):line(%d) wrong flash mode\n",__func__, __LINE__);
					return -EFAULT;
			}
			break;

		case S5K4ECGX_AF_STOP:
			if(info->gwb_index == S5K4ECGX_WB_AUTO)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_awb_unlock");
			if (!info->touch_af_enable)
				err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ae_unlock");

			if(rear_mode.PreviewActive && (!rear_mode.StillCount)) {
				//err = s5k4ecgx_set_focus_mode(info,info->gfocus_index);
				//for focus in AF_STOP
				s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
				if (info->gLowLight_check == true) {
					switch (info->gfocus_index) {
						case S5K4ECGX_FOCUS_AUTO:
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_1");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_2");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_normal_mode_3");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							break;

						case S5K4ECGX_FOCUS_MACRO:
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_Macro_mode_1");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_Macro_mode_2");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Low_Light_Macro_mode_3");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							break;

						default:
							pr_err("%s: Invalid focus mode(Low light), %d\n", __func__, arg);
							info->pre_flash_ae_stable_check = 0;
							return 0;
							break;
					}
				} else {
					switch (info->gfocus_index) {
						case S5K4ECGX_FOCUS_AUTO:
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Normal_mode_1");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Normal_mode_2");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Normal_mode_3");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							break;

						case S5K4ECGX_FOCUS_MACRO:
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Macro_mode_1");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Macro_mode_2");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Macro_mode_3");
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							break;

						default:
							pr_err("%s: Invalid focus mode(Not Low light), %d\n", __func__, arg);
							info->pre_flash_ae_stable_check = 0;
							return 0;
							break;
					}	
				}
				//s5k4ecgx_set_pre_flash_control(info, S5K4ECGX_FLASH_OFF);
			}
			info->pdata->torch_onoff(0);
			if (info->flash_fired) {
				info->flash_fired = false;
				if(rear_mode.StillCount)
					err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Main_Flash_Off");
				else
					err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Pre_Flash_Off");
			}
			info->pre_flash_ae_stable_check = 0;
			info->af_result = 1;
			return 0;
			break;

		default:
			pr_err("%s: Invalid af mode, %d\n", __func__, arg);
			info->pre_flash_ae_stable_check = 0;
			return 0;
			break;
	}
#else
	switch (arg) {
		case S5K4ECGX_AF_START:
			/*switch (info->af_flash_mode) {
				case S5K4ECGX_FLASH_AUTO:
				case S5K4ECGX_FLASH_ON:
					// Low Light
					//printk(KERN_ERR "func(%s):line(%d) 1st AF Flash\n",__func__, __LINE__);
					s5k4ecgx_set_pre_flash_control(info, info->af_flash_mode);
					break;
				default:
					//printk(KERN_ERR "func(%s):line(%d) 1st AF NOT Flash\n",__func__, __LINE__);
					break;
			}*/

			switch (info->af_flash_mode) {
				case S5K4ECGX_FLASH_AUTO:
					s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
					if (info->gLowLight_check == true) {
						//printk(KERN_ERR "func(%s):line(%d) Pre flash AUTO - Low Light\n",__func__, __LINE__);
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Pre_Flash_On);
						info->pdata->torch_onoff(1);
						info->flash_fired = true;
						info->pre_flash_ae_stable_check = 0;
					} else {
						//printk(KERN_ERR "func(%s):line(%d) Pre flash AUTO - Not Low Light\n",__func__, __LINE__);
						info->pdata->torch_onoff(0);
						if(info->gwb_index == S5K4ECGX_WB_AUTO && !info->touch_af_enable)
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_awb_lock);
						if (!info->touch_af_enable) {
							if (info->cam_mode != S5K4ECGX_CAM_MODE_MAX) {
								err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ae_lock);
							}
						}
						s5k4ecgx_frame_delay(info);
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Single_AF_Start);
						info->touch_af_enable = false;
						info->pre_flash_ae_stable_check = 1;
						err = 0;
					}
					break;
			
				case S5K4ECGX_FLASH_ON:
					//printk(KERN_ERR "func(%s):line(%d) Pre flash ON\n",__func__, __LINE__);
					err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Pre_Flash_On);
					info->pdata->torch_onoff(1);
					info->flash_fired = true;
					info->pre_flash_ae_stable_check = 0;
					break;
			
				case S5K4ECGX_FLASH_OFF:
					//printk(KERN_ERR "func(%s):line(%d) Pre flash OFF\n",__func__, __LINE__);
					info->pdata->torch_onoff(0);
					if(info->gwb_index == S5K4ECGX_WB_AUTO && !info->touch_af_enable)
						err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_awb_lock);
					if (!info->touch_af_enable) {
						if (info->cam_mode != S5K4ECGX_CAM_MODE_MAX) {
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ae_lock);
						}
					}
					s5k4ecgx_frame_delay(info);
					err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Single_AF_Start);
					info->touch_af_enable = false;
					info->pre_flash_ae_stable_check = 1;
					break;
			
				case S5K4ECGX_FLASH_TORCH:
					info->pdata->torch_onoff(1);
					info->pre_flash_ae_stable_check = 1;
					break;
			
				default:
					printk(KERN_ERR "func(%s):line(%d) wrong flash mode\n",__func__, __LINE__);
					return -EFAULT;
			}
			break;

		case S5K4ECGX_AF_STOP:
			if(info->gwb_index == S5K4ECGX_WB_AUTO)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_awb_unlock);
			if (!info->touch_af_enable)
				err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ae_unlock);

			if(rear_mode.PreviewActive && (!rear_mode.StillCount)) {
				//err = s5k4ecgx_set_focus_mode(info,info->gfocus_index);
				//for focus in AF_STOP
				s5k4ecgx_get_LowLightCondition(info->i2c_client, info);
				if (info->gLowLight_check == true) {
					switch (info->gfocus_index) {
						case S5K4ECGX_FOCUS_AUTO:
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_1);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_2);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_normal_mode_3);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							break;

						case S5K4ECGX_FOCUS_MACRO:
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_Macro_mode_1);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_Macro_mode_2);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Low_Light_Macro_mode_3);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							break;

						default:
							pr_err("%s: Invalid focus mode(Low light), %d\n", __func__, arg);
							info->pre_flash_ae_stable_check = 0;
							return 0;
							break;
					}
				} else {
					switch (info->gfocus_index) {
						case S5K4ECGX_FOCUS_AUTO:
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_1);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_2);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_3);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							break;

						case S5K4ECGX_FOCUS_MACRO:
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Macro_mode_1);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Macro_mode_2);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Macro_mode_3);
							// 1 frame delay of SOC processor
							//s5k4ecgx_frame_delay(info);
							break;

						default:
							pr_err("%s: Invalid focus mode(Not Low light), %d\n", __func__, arg);
							info->pre_flash_ae_stable_check = 0;
							return 0;
							break;
					}	
				}
				//s5k4ecgx_set_pre_flash_control(info, S5K4ECGX_FLASH_OFF);
			}
			info->pdata->torch_onoff(0);
			if (info->flash_fired) {
				info->flash_fired = false;
				if(rear_mode.StillCount)
					err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Main_Flash_Off);
				else
					err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Pre_Flash_Off);
			}
			info->pre_flash_ae_stable_check = 0;
			info->af_result = 1;
			return 0;
			break;

		default:
			pr_err("%s: Invalid af mode, %d\n", __func__, arg);
			info->pre_flash_ae_stable_check = 0;
			return 0;
			break;
	}
#endif
	if (err < 0)
		pr_err("%s: s5k4ecgx_set_af_control() returned error, %d, %d\n", __func__, arg, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_get_af_result(struct s5k4ecgx_info *info)
{
	//FUNC_ENTR;

	int err = -1;

	u8 r_data[2] = {0,0};
	u8 r_data1[2] = {0,0};
	u8 r_data2[1] = {0};
	u8 af_data1 = 0;
	u8 af_data2 = 0;
	u8 ae_data = 0;

#ifdef CONFIG_LOAD_FILE
	switch (info->pre_flash_ae_stable_check) {
		case 0:
			//pr_err("%s: line(%d)[AF-TEMP] info->pre_flash_ae_stable_check(0)\n", __func__, __LINE__);
			switch (info->af_flash_mode) {
				case S5K4ECGX_FLASH_AUTO:
					if(info->gLowLight_check == true) {
						if(info->pre_flash_ae_skip > 2) {
							s5k4ecgx_read_reg(info->i2c_client, 0x2c74, r_data, 2);
							ae_data = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
							//pr_err("func(%s):line(%d) AE stabilization AE(0x%x) info->pre_flash_ae_skip(%d)\n",__func__, __LINE__, ae_data, info->pre_flash_ae_skip);
							info->pre_flash_ae_skip++;
							if(ae_data & 0xFFFF) {
								info->pre_flash_ae_skip = 0;
								info->pre_flash_ae_stable_check = 1;
								//pr_err("%s: line(%d)[AF-TEMP] check change 0->1\n", __func__, __LINE__);
								if(info->gwb_index == S5K4ECGX_WB_AUTO && !info->touch_af_enable)
									err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_awb_lock");
								if (!info->touch_af_enable)
									err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ae_lock");
								s5k4ecgx_frame_delay(info);
								err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Single_AF_Start");
								info->touch_af_enable = false;
								//pr_err("%s: line(%d)[AF-TEMP] S5K4ECGX_FLASH_ON or AUTO check change 1->2\n", __func__, __LINE__);
							} else {
								err = 0;
							}
						} else {
							info->pre_flash_ae_skip++;
							info->pre_flash_ae_stable_check = 0;
							err = 0;
						}
					} else {
						info->pre_flash_ae_stable_check = 1;
						err = 0;
					}
					break;

				case S5K4ECGX_FLASH_ON:
					if(info->pre_flash_ae_skip > 2) {
						s5k4ecgx_read_reg(info->i2c_client, 0x2c74, r_data, 2);
						ae_data = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
						//pr_err("func(%s):line(%d) AE stabilization AE(0x%x) info->pre_flash_ae_skip(%d)\n",__func__, __LINE__, ae_data, info->pre_flash_ae_skip);
						info->pre_flash_ae_skip++;
						if(ae_data & 0xFFFF) {
							info->pre_flash_ae_skip = 0;
							info->pre_flash_ae_stable_check = 1;
							//pr_err("%s: line(%d)[AF-TEMP] check change 0->1\n", __func__, __LINE__);
							if(info->gwb_index == S5K4ECGX_WB_AUTO && !info->touch_af_enable)
								err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_awb_lock");
							if (!info->touch_af_enable)
								err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_ae_lock");
							s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Single_AF_Start");
							info->touch_af_enable = false;
							//pr_err("%s: line(%d)[AF-TEMP] S5K4ECGX_FLASH_ON or AUTO check change 1->2\n", __func__, __LINE__);
						} else {
							err = 0;
						}
					} else {
						info->pre_flash_ae_skip++;
						info->pre_flash_ae_stable_check = 0;
						err = 0;
					}
					break;

				case S5K4ECGX_FLASH_OFF:
					err = 0;
					break;

				case S5K4ECGX_FLASH_TORCH:
					err = 0;
					break;

				default:
					printk(KERN_ERR "func(%s):line(%d) wrong flash mode\n",__func__, __LINE__);
					return -EFAULT;
			}
			info->af_result = 1;
			break;

		case 1:
			//pr_err("%s: line(%d)[AF-TEMP] info->pre_flash_ae_stable_check(2)\n", __func__, __LINE__);
			if (info->pre_flash_ae_skip >= 3) {
				if (!(info->first_af_check)) {
					s5k4ecgx_read_reg(info->i2c_client, 0x2eee, r_data1, 2);
					af_data1 = ((r_data1[0] & 0x00)|(r_data1[1] & 0xFF));
					//printk(KERN_ERR "func(%s):line(%d) 1st AF r_data:0x%x%x | af_data1 : 0x%x\n",__func__, __LINE__, r_data1[0], r_data1[1],af_data1);
					if ((af_data1 & 0xFF) == 0x01) {
						/* 1st AF ing */
						//printk(KERN_ERR "func(%s):line(%d) 1st AF ing\n",__func__, __LINE__);
						info->af_result = 1;
						err = 0;
					} else if ((af_data1 & 0xFF) == 0x02){
						/* 1st AF Success */
						//printk(KERN_ERR "func(%s):line(%d) 1st AF success\n",__func__, __LINE__);
						info->af_result = 1;
						info->first_af_check = true;
						err = 0;
					} else {
						/* 1st AF Fail */
						//printk(KERN_ERR "func(%s):line(%d) 1st AF fail\n",__func__, __LINE__);
						/* pre flash OFF */
						//s5k4ecgx_set_pre_flash_control(info, S5K4ECGX_FLASH_OFF);
						info->pdata->torch_onoff(0);
						if (info->flash_fired) {
							info->flash_fired = false;
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Pre_Flash_Off");
						}
						info->af_result = 0;
						info->pre_flash_ae_stable_check = 0;
						info->pre_flash_ae_skip = 0;
						return 0;
					}
				} else {
					//printk(KERN_ERR "func(%s):line(%d) 2st AF Start.\n", __func__, __LINE__);
					s5k4ecgx_read_reg(info->i2c_client, 0x2207, r_data2, 1);
					af_data2 = (r_data2[0] & 0xFF);
					//printk(KERN_ERR "func(%s):line(%d) 2st AF r_data:0x%x | af_data2 : 0x%x\n",__func__, __LINE__, r_data2[0],af_data2);
					if ((af_data2 & 0xFF) == 0x00){
						/* 2st AF */
						//printk(KERN_ERR "func(%s):line(%d) 2st AF Success\n",__func__, __LINE__);
						/* pre flash OFF */
						//err = s5k4ecgx_set_pre_flash_control(info, S5K4ECGX_FLASH_OFF);
						info->pdata->torch_onoff(0);
						if (info->flash_fired) {
							info->flash_fired = false;
							err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Pre_Flash_Off");
						}
						info->af_result = 2;
						info->first_af_check = false;
						info->pre_flash_ae_stable_check = 0;
						info->pre_flash_ae_skip = 0;
						return err;
					} else {
						/* 2st AF */
						//printk(KERN_ERR "func(%s):line(%d) 2st AF ing\n",__func__, __LINE__);
						info->af_result = 1;
						err = 0;
					}
				}
			} else {
				info->af_result = 1;
				info->pre_flash_ae_skip++;
				return 0;
			}
			break;
	}	
#else
	switch (info->pre_flash_ae_stable_check) {
		case 0:
			//pr_err("%s: line(%d)[AF-TEMP] info->pre_flash_ae_stable_check(0)\n", __func__, __LINE__);
			switch (info->af_flash_mode) {
				case S5K4ECGX_FLASH_AUTO:
					if(info->gLowLight_check == true) {
						if(info->pre_flash_ae_skip > 2) {
							s5k4ecgx_read_reg(info->i2c_client, 0x2c74, r_data, 2);
							ae_data = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
							//pr_err("func(%s):line(%d) AE stabilization AE(0x%x) info->pre_flash_ae_skip(%d)\n",__func__, __LINE__, ae_data, info->pre_flash_ae_skip);
							info->pre_flash_ae_skip++;
							//pr_err("func(%s):line(%d) AE stabilization AE(0x%x) info->pre_flash_ae_skip(%d)\n",__func__, __LINE__, ae_data, info->pre_flash_ae_skip);
							if(ae_data & 0xFFFF) {
								info->pre_flash_ae_skip = 0;
								info->pre_flash_ae_stable_check = 1;
								//pr_err("%s: line(%d)[AF-TEMP] check change 0->1\n", __func__, __LINE__);
								if(info->gwb_index == S5K4ECGX_WB_AUTO && !info->touch_af_enable)
									err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_awb_lock);
								if (!info->touch_af_enable)
									err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ae_lock);
								s5k4ecgx_frame_delay(info);
								err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Single_AF_Start);
								info->touch_af_enable = false;
								//pr_err("%s: line(%d)[AF-TEMP] S5K4ECGX_FLASH_ON or AUTO check change 1->2\n", __func__, __LINE__);
							} else {
								err =0;
							}
						} else {
							info->pre_flash_ae_skip++;
							info->pre_flash_ae_stable_check = 0;
							err = 0;
						}
					} else {
						info->pre_flash_ae_stable_check = 1;
						err = 0;
					}
					break;

				case S5K4ECGX_FLASH_ON:
					if(info->pre_flash_ae_skip > 2) {
						s5k4ecgx_read_reg(info->i2c_client, 0x2c74, r_data, 2);
						ae_data = ((r_data[0] & 0x00)|(r_data[1] & 0xFF));
						//pr_err("func(%s):line(%d) AE stabilization AE(0x%x) info->pre_flash_ae_skip(%d)\n",__func__, __LINE__, ae_data, info->pre_flash_ae_skip);
						info->pre_flash_ae_skip++;
						if(ae_data & 0xFFFF) {
							info->pre_flash_ae_skip = 0;
							info->pre_flash_ae_stable_check = 1;
							//pr_err("%s: line(%d)[AF-TEMP] check change 0->1\n", __func__, __LINE__);
							if(info->gwb_index == S5K4ECGX_WB_AUTO && !info->touch_af_enable)
								err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_awb_lock);
							if (!info->touch_af_enable)
								err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_ae_lock);
							s5k4ecgx_frame_delay(info);
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Single_AF_Start);
							info->touch_af_enable = false;
							//pr_err("%s: line(%d)[AF-TEMP] S5K4ECGX_FLASH_ON or AUTO check change 1->2\n", __func__, __LINE__);
						} else {
							err = 0;
						}
					} else {
						info->pre_flash_ae_skip++;
						info->pre_flash_ae_stable_check = 0;
						err = 0;
					}
					break;

				case S5K4ECGX_FLASH_OFF:
					err = 0;
					break;

				case S5K4ECGX_FLASH_TORCH:
					err = 0;
					break;

				default:
					printk(KERN_ERR "func(%s):line(%d) wrong flash mode\n",__func__, __LINE__);
					return -EFAULT;
			}
			info->af_result = 1;
			break;

		case 1:
			//pr_err("%s: line(%d)[AF-TEMP] info->pre_flash_ae_stable_check(2)\n", __func__, __LINE__);
			if (info->pre_flash_ae_skip >= 3) {
				if (!(info->first_af_check)) {
					s5k4ecgx_read_reg(info->i2c_client, 0x2eee, r_data1, 2);
					af_data1 = ((r_data1[0] & 0x00)|(r_data1[1] & 0xFF));
					//printk(KERN_ERR "func(%s):line(%d) 1st AF r_data:0x%x%x | af_data1 : 0x%x\n",__func__, __LINE__, r_data1[0], r_data1[1],af_data1);
					if ((af_data1 & 0xFF) == 0x01) {
						/* 1st AF ing */
						//printk(KERN_ERR "func(%s):line(%d) 1st AF ing\n",__func__, __LINE__);
						info->af_result = 1;
						err = 0;
					} else if ((af_data1 & 0xFF) == 0x02){
						/* 1st AF Success */
						//printk(KERN_ERR "func(%s):line(%d) 1st AF success\n",__func__, __LINE__);
						info->af_result = 1;
						info->first_af_check = true;
						err = 0;
					} else {
						/* 1st AF Fail */
						//printk(KERN_ERR "func(%s):line(%d) 1st AF fail\n",__func__, __LINE__);
						/* pre flash OFF */
						//s5k4ecgx_set_pre_flash_control(info, S5K4ECGX_FLASH_OFF);
						info->pdata->torch_onoff(0);
						if (info->flash_fired) {
							info->flash_fired = false;
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Pre_Flash_Off);
						}
						info->af_result = 0;
						info->pre_flash_ae_stable_check = 0;
						info->pre_flash_ae_skip = 0;
						return 0;
					}
				} else {
					//printk(KERN_ERR "func(%s):line(%d) 2st AF Start.\n", __func__, __LINE__);
					s5k4ecgx_read_reg(info->i2c_client, 0x2207, r_data2, 1);
					af_data2 = (r_data2[0] & 0xFF);
					//printk(KERN_ERR "func(%s):line(%d) 2st AF r_data:0x%x | af_data2 : 0x%x\n",__func__, __LINE__, r_data2[0],af_data2);
					if ((af_data2 & 0xFF) == 0x00){
						/* 2st AF */
						//printk(KERN_ERR "func(%s):line(%d) 2st AF Success\n",__func__, __LINE__);
						/* pre flash OFF */
						//err = s5k4ecgx_set_pre_flash_control(info, S5K4ECGX_FLASH_OFF);
						info->pdata->torch_onoff(0);
						if (info->flash_fired) {
							info->flash_fired = false;
							err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Pre_Flash_Off);
						}
						info->af_result = 2;
						info->first_af_check = false;
						info->pre_flash_ae_stable_check = 0;
						info->pre_flash_ae_skip = 0;
						return err;
					} else {
						/* 2st AF */
						//printk(KERN_ERR "func(%s):line(%d) 2st AF ing\n",__func__, __LINE__);
						info->af_result = 1;
						err = 0;
					}
				}
			} else {
				info->af_result = 1;
				info->pre_flash_ae_skip++;
				return 0;
			}
			break;
	}
#endif

	if (err < 0)
		pr_err("%s: s5k4ecgx_get_af_result() returned error, %d\n", __func__, err);

	return err;
}

#define INNER_WINDOW_WIDTH_640		72
#define INNER_WINDOW_HEIGHT_480		143
#define OUTER_WINDOW_WIDTH_640		320
#define OUTER_WINDOW_HEIGHT_480		266

#define INNER_WINDOW_WIDTH_800		90
#define OUTER_WINDOW_WIDTH_800		400

static int s5k4ecgx_set_touchaf(struct s5k4ecgx_info *info, struct s5k4ecgx_touchaf_pos *tpos)
{
	int err = -1;

	u16 mapped_x = 0;
	u16 mapped_y = 0;
	u16 inner_window_start_x = 0;
	u16 inner_window_start_y = 0;
	u16 outer_window_start_x = 0;
	u16 outer_window_start_y = 0;
	
	u16 sensor_width = 0;
	u16 sensor_height = 0;
	
	u16 inner_window_width = 0;
	u16 inner_window_height= 0;
	u16 outer_window_width= 0;
	u16 outer_window_height= 0;
	
	u16 touch_width = 0;
	u16 touch_height = 0;
	
	FUNC_ENTR;

	switch (info->framesize_index) {
		case S5K4ECGX_PREVIEW_WVGA:	/* 800x480 */
			sensor_width = 800;
			sensor_height = 480;
			inner_window_width = INNER_WINDOW_WIDTH_800;
			inner_window_height = INNER_WINDOW_HEIGHT_480;
			outer_window_width = OUTER_WINDOW_WIDTH_800;
			outer_window_height = OUTER_WINDOW_HEIGHT_480;
			touch_width = 800;
			touch_height = 480;
			break;

		case S5K4ECGX_PREVIEW_VGA:	/* 640x480 */
			sensor_width = 640;
			sensor_height = 480;
			inner_window_width = INNER_WINDOW_WIDTH_640;
			inner_window_height = INNER_WINDOW_HEIGHT_480;
			outer_window_width = OUTER_WINDOW_WIDTH_640;
			outer_window_height = OUTER_WINDOW_HEIGHT_480;
			touch_width = 640;
			touch_height = 480;
			break;

		case S5K4ECGX_PREVIEW_D1:	/* 720x480 */
		case S5K4ECGX_PREVIEW_QVGA:	/* 320x240 */
		case S5K4ECGX_PREVIEW_MMS:	/* 176x144 */
		case S5K4ECGX_PREVIEW_720P:	/* 1280x720 for record*/
		default:
	 		pr_err("%s: we don't care these size. info->index(%d)\n", __func__, info->framesize_index);
			break;
	}

//	tpos->xpos = touch_width - tpos->xpos;
//	tpos->ypos = touch_height - tpos->ypos;
	
//	printk("\n\n%s : xPos = %d, yPos = %d \n\n", __func__, tpos->xpos, tpos->ypos);
	
	// mapping the touch position on the sensor display
	mapped_x = (tpos->xpos * sensor_width) / touch_width;
	mapped_y = (tpos->ypos * sensor_height) / touch_height;
//	printk("\n\n%s : mapped xPos = %d, mapped yPos = %d\n\n",
//			__func__, mapped_x, mapped_y);
	
	// set X axis
	if (mapped_x <= (inner_window_width / 2)) {
		inner_window_start_x = 0;
		outer_window_start_x = 0;
/*
   		printk("\n\n%s : inbox over the left side. boxes are \
				left side align in_Sx = %d, out_Sx= %d\n\n", \
				__func__, inner_window_start_x, outer_window_start_x);
*/
	} else if (mapped_x <= (outer_window_width / 2)) {
		inner_window_start_x = mapped_x - (inner_window_width / 2);
		outer_window_start_x = 0;
/*
		printk("\n\n%s : outbox only over the left side. \
				outbox is only left side align in_Sx = %d, \
				out_Sx= %d\n\n", __func__, \
				inner_window_start_x, outer_window_start_x);
*/
	} else if (mapped_x >= ((sensor_width - 1) - (inner_window_width / 2))) {
		inner_window_start_x = (sensor_width - 1) - inner_window_width;
		outer_window_start_x = (sensor_width - 1) - outer_window_width;
/*
   		printk("\n\n%s : inbox over the right side. boxes are rightside \
				align in_Sx = %d, out_Sx= %d\n\n", __func__, \
				inner_window_start_x, outer_window_start_x);
*/
	} else if (mapped_x >= ((sensor_width - 1) - (outer_window_width / 2))) {
		inner_window_start_x = mapped_x - (inner_window_width / 2);
		outer_window_start_x = (sensor_width - 1) - outer_window_width;
/*
		printk("\n\n%s : outbox only over the right side. out box is \
				only right side align in_Sx = %d, out_Sx= %d\n\n", \
				__func__, inner_window_start_x, outer_window_start_x);
*/
	} else {
		inner_window_start_x = mapped_x - (inner_window_width / 2);
		outer_window_start_x = mapped_x - (outer_window_width / 2);
/*
		printk("\n\n%s : boxes are in the sensor window. in_Sx = %d, \
				out_Sx= %d\n\n", __func__, inner_window_start_x, \
				outer_window_start_x);
*/
	}

	// set Y axis
	if (mapped_y <= (inner_window_height / 2)) {
		inner_window_start_y = 0;
		outer_window_start_y = 0;
/*
		printk("\n\n%s : inbox over the top side. boxes are \
				top side align in_Sy = %d, out_Sy= %d\n\n", \
				__func__, inner_window_start_y, outer_window_start_y);
*/
	} else if (mapped_y <= (outer_window_height / 2)) {
		inner_window_start_y = mapped_y - (inner_window_height / 2);
		outer_window_start_y = 0;
/*
		printk("\n\n%s : outbox only over the top side. \
				outbox is only top side align in_Sy = %d, \
				out_Sy= %d\n\n", __func__, \
				inner_window_start_y, outer_window_start_y);
*/
	} else if (mapped_y >= ((sensor_height - 1) - (inner_window_height / 2))) {
		inner_window_start_y = (sensor_height - 1) - inner_window_height;
		outer_window_start_y = (sensor_height - 1) - outer_window_height;
/*
		printk("\n\n%s : inbox over the bottom side. boxes are bottom side \
				align in_Sy = %d, out_Sy= %d\n\n", __func__, \
				inner_window_start_y, outer_window_start_y);
*/
	} else if (mapped_y >= ((sensor_height - 1) - (outer_window_height / 2))) {
		inner_window_start_y = mapped_y - (inner_window_height / 2);
		outer_window_start_y = (sensor_height - 1) - outer_window_height;
/*
		printk("\n\n%s : outbox only over the bottom side. out box is only \
				bottom side align in_Sy = %d, out_Sy= %d\n\n", __func__, \
				inner_window_start_y, outer_window_start_y);
*/
	} else {
		inner_window_start_y = mapped_y - (inner_window_height / 2);
		outer_window_start_y = mapped_y - (outer_window_height / 2);
/*
		printk("\n\n%s : boxes are in the sensor window. in_Sy = %d, \
				out_Sy= %d\n\n", __func__, \
				inner_window_start_y, outer_window_start_y);
*/
	}
	
	//calculate the start position value
	inner_window_start_x = inner_window_start_x * 1024 /sensor_width;
	outer_window_start_x = outer_window_start_x * 1024 / sensor_width;
	inner_window_start_y = inner_window_start_y * 1024 / sensor_height;
	outer_window_start_y = outer_window_start_y * 1024 / sensor_height;
//	pr_info("\n\n%s : calculated value inner_window_start_x = %d\n\n",
//			__func__, inner_window_start_x);
//	pr_info("\n\n%s : calculated value inner_window_start_y = %d\n\n",
//			__func__, inner_window_start_y);
//	pr_info("\n\n%s : calculated value outer_window_start_x = %d\n\n",
//			__func__, outer_window_start_x);
//      pr_info("\n\n%s : calculated value outer_window_start_y = %d\n\n",
//			__func__, outer_window_start_y);

	s5k4ecgx_touchaf[2].val = outer_window_start_x;	//outer_window_start_x
	s5k4ecgx_touchaf[3].val = outer_window_start_y;	//outer_window_start_y
	s5k4ecgx_touchaf[5].val = inner_window_start_x;	//outer_window_start_x
	s5k4ecgx_touchaf[6].val = inner_window_start_y;	//outer_window_start_y

#ifdef CONFIG_LOAD_FILE
	err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_touchaf");
#else
	err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_touchaf);
#endif

	info->touch_af_enable = true;

	FUNC_EXIT;

	return err;
}

#ifdef FACTORY_TEST
static int s5k4ecgx_return_normal_preview(struct s5k4ecgx_info *info)
{
	int err = -1;

	FUNC_ENTR;
	regulator_disable(reg_mipi_1v2);
	info->pdata->power_off();

	//msleep(20);
	usleep_range(20000, 30000);//20ms

	regulator_enable(reg_mipi_1v2);
	info->pdata->power_on();

#ifdef CONFIG_LOAD_FILE
	err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_init_reg1");
	/* delay 10ms after wakeup of SOC processor */
	//msleep(10);
	usleep_range(10000, 20000);//10ms
	err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_init_reg2");

	/* REV 0.8 */
	if(system_rev >= 7) {
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Preview_flip");
	}
#else
	err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_init_reg1);
	/* delay 10ms after wakeup of SOC processor */
	//msleep(10);
	usleep_range(10000, 20000);//10ms
	err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_init_reg2);

	/* REV 0.8 */
	if(system_rev >= 7) {
		err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Preview_flip);
	}
#endif

	if (err < 0)
		pr_err("%s: s5k4ecgx_return_normal_preview() returned error, %d\n", __func__, err);

	FUNC_EXIT;

	return err;
}
#endif

static int s5k4ecgx_set_preview_resolution
	(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int err = -1;
	int timeout_cnt = 0;
	u8 r_data[2] = {0,0};
	u16 preview_return_delay = 0;

	FUNC_ENTR;

	//printk(KERN_ERR "func(%s): xres %u yres %u | mode->camcordmode(%d)\n", __func__, mode->xres, mode->yres, mode->camcordmode);

	if (info->mode == CAPTURE_MODE) {
		printk(KERN_ERR "[CAPTURE_MODE]:func(%s):line(%d)\n",__func__, __LINE__);
		//usleep_range(10000, 20000);//10ms
#ifdef CONFIG_LOAD_FILE
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Low_Cap_Off");
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Preview_Return");
#else
		err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Low_Cap_Off);
		err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Preview_Return);
#endif

		/* scene, effect, etc */
		if(info->gscene_index == S5K4ECGX_SCENE_AUTO) {
			if(info->gscene_index != S5K4ECGX_ISO_AUTO) {
				s5k4ecgx_set_iso(info, info->giso_index);
			}
			if (info->gmeter_index != S5K4ECGX_EXPOSURE_METER_CENTER) {
				s5k4ecgx_set_exposure_meter(info, info->gmeter_index);
			}
			if(info->gfocus_index != S5K4ECGX_FOCUS_AUTO) {
				s5k4ecgx_set_focus_mode(info, info->gfocus_index);
			}
			if(info->geffect_index != S5K4ECGX_EFFECT_NONE) {
				s5k4ecgx_set_color_effect(info, info->geffect_index);
			}
			if(info->gwb_index != S5K4ECGX_WB_AUTO) {
				s5k4ecgx_set_white_balance(info, info->gwb_index);
			}
			if(info->gev_index != S5K4ECGX_EXPOSURE_ZERO) {
				s5k4ecgx_set_exposure(info, info->gev_index);
			}
		}  else {
			s5k4ecgx_set_scene_mode(info, info->gscene_index);
		}
		
		if(info->gauto_cont_index != S5K4ECGX_AUTO_CONTRAST_OFF) {
			s5k4ecgx_set_auto_contrast(info, info->gauto_cont_index);
		}
		if(info->af_flash_mode!= S5K4ECGX_FLASH_AUTO) {
			s5k4ecgx_set_flash_mode(info, info->af_flash_mode);
		}

		/* preview return delay - polling is changed*/
		do {
			timeout_cnt++;
			//msleep(1);
			usleep_range(20000, 25000);	//20ms
			s5k4ecgx_read_reg(info->i2c_client, 0x0240, r_data, 2);
			preview_return_delay = ((r_data[0] << 8) & 0xFF00) | (r_data[1] & 0xFF);
			//printk(KERN_ERR "func(%s):line(%d) preview return delay(0x%x) (%x/%x)\n",__func__, __LINE__, preview_return_delay, r_data[0], r_data[1]);
			if (timeout_cnt > S5K4ECGX_RETURN_DELAY_RETRIES) {
				pr_err("%s: Entering preview return delay timed out \n", __func__);
				break;
			}
		} while ((preview_return_delay & 0xFFFF));
		return err;
	} else if (info->mode == SYSTEM_INITIALIZE_MODE) {
		printk(KERN_ERR "[SYSTEM_INITIALIZE_MODE]:func(%s):line(%d)\n",__func__, __LINE__);

#ifdef CONFIG_LOAD_FILE
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_init_reg1");
		/*err = s5k4ecgx_sensor_burst_write_list(info->i2c_client, "s5k4ecgx_init_reg1", 
				(sizeof(s5k4ecgx_init_reg1) / sizeof(s5k4ecgx_init_reg1[0])));*/
		if(err < 0) {
			pr_err("%s: Initial set1 Failed\n", __func__);
			return -EINVAL;
		}

		// delay 10ms after wakeup of SOC processor
		//msleep(10);
		usleep_range(10000, 20000);//10ms
		pr_err("%s: Preview resolution set Success1!\n", __func__);

		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_init_reg2");
		/*err = s5k4ecgx_sensor_burst_write_list(info->i2c_client, "s5k4ecgx_init_reg2", 
				(sizeof(s5k4ecgx_init_reg2) / sizeof(s5k4ecgx_init_reg2[0])));*/
		if(err < 0) {
			pr_err("%s: Initial set2 Failed\n", __func__);
			return -EINVAL;
		}
		pr_err("%s: Preview resolution set Success2!\n", __func__);

		/* REV 0.8 */
		if(system_rev >= 7) {
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Preview_flip");
		}
#else
		//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_init_reg1);
		err = s5k4ecgx_sensor_burst_write_list(info->i2c_client, s5k4ecgx_init_reg1, 
				(sizeof(s5k4ecgx_init_reg1) / sizeof(s5k4ecgx_init_reg1[0])));
		if(err < 0) {
			pr_err("%s: Initial set1 Failed\n", __func__);
			return -EINVAL;
		}

		// delay 10ms after wakeup of SOC processor
		//msleep(10);
		usleep_range(10000, 20000);//10ms

		//err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_init_reg2);
		err = s5k4ecgx_sensor_burst_write_list(info->i2c_client, s5k4ecgx_init_reg2, 
				(sizeof(s5k4ecgx_init_reg2) / sizeof(s5k4ecgx_init_reg2[0])));
		if(err < 0) {
			pr_err("%s: Initial set2 Failed\n", __func__);
			return -EINVAL;
		}
		pr_err("%s: Preview resolution set Success!\n", __func__);

		/* REV 0.8 */
		if(system_rev >= 7) {
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Preview_flip);
		}
#endif
		/* DTP */
#ifdef FACTORY_TEST
		if (dtpTest) {
			pr_err("%s : dtpTest = %d\n", __func__, dtpTest);
#ifdef CONFIG_LOAD_FILE
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_DTP_init");
#else
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_DTP_init);
#endif
		}
#endif

		/* Flash init */
#ifdef CONFIG_LOAD_FILE
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Flash_init");
#else
		err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Flash_init);
#endif
		/* Preview size */
		err = s5k4ecgx_set_preview_size(info, mode);

		/*if (mode->camcordmode == 1 || info->framesize_index == S5K4ECGX_PREVIEW_720P) {
			err = s5k4ecgx_set_preview_size(info, mode);
		} else if (info->framesize_index != S5K4ECGX_PREVIEW_VGA) {
			//err = s5k4ecgx_set_preview_size(info, mode);
#ifdef CONFIG_LOAD_FILE
			err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_800_Preview");
#else
			err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_800_Preview);
#endif
		}*/

		if(err < 0) {
			pr_err("%s: Preview set Failed\n", __func__);
			return -EINVAL;
		}

//		msleep(100);	//recommand from module team.
		if(mode->camcordmode == S5K4ECGX_CAM_MODE_ON) {
			info->mode = RECORD_MODE;
			/* FPS Config - Default 30fps, but Not MMS*/
			switch (info->framesize_index) {
				case S5K4ECGX_PREVIEW_MMS:
				case S5K4ECGX_PREVIEW_MMS3:
					//pr_err("%s: FPS 15 is setted.\n", __func__);
					err = s5k4ecgx_set_frame_rate(info, S5K4ECGX_FPS_15);
					break;

				default:
					//pr_err("%s: FPS 30 is setted.\n", __func__);
					err = s5k4ecgx_set_frame_rate(info, S5K4ECGX_FPS_30);
					break;
			}

			if(err < 0) {
				pr_err("%s: Preview fps set Failed\n", __func__);
				return -EINVAL;
			}
			//msleep(300);	//for AE stable
			usleep_range(300000, 400000);//300ms
		} else if((mode->camcordmode == S5K4ECGX_CAM_MODE_MAX) && (info->framesize_index == S5K4ECGX_PREVIEW_QVGA)) {
			//pr_err("%s: QVGA:FPS 15 is setted.\n", __func__);
			err = s5k4ecgx_set_frame_rate(info, S5K4ECGX_FPS_15);
		}
		/*else if(mode->vtcallmode) {
		}*/ else {
			info->mode = CAPTURE_MODE;
		}

		//msleep(500);	//for AE stable
	}
	else if (info->mode == RECORD_MODE) {
		//printk(KERN_ERR "[RECORD_MODE]:func(%s):line(%d)\n",__func__, __LINE__);

		err = 0;
		/* FPS Config - Default 30fps */
		/*pr_err("%s: s5k4ecgx_set_frame_rate() S5K4ECGX_FPS_30\n", __func__);
		err = s5k4ecgx_set_frame_rate(info, S5K4ECGX_FPS_30);
		if(err < 0) {
			pr_err("%s: Preview fps set Failed\n", __func__);
			return -EINVAL;
		}*/
	} else {
		pr_err("%s: invalid preview resolution supplied to set mode %d %d info->mode(%d)\n",
				__func__, mode->xres, mode->yres, info->mode);
		return -EINVAL;
	}

	if (err < 0)
		pr_err("%s: s5k4ecgx_set_preview_resolution() returned error, %d\n", __func__, err);

	FUNC_EXIT;

	return err;
}

static int s5k4ecgx_set_mode(struct s5k4ecgx_info *info, struct s5k4ecgx_mode *mode)
{
	int err = -1;

	FUNC_ENTR;

	//printk(KERN_ERR "func(%s): xres %u yres %u\n", __func__, mode->xres, mode->yres);
	/*printk(KERN_ERR "func(%s):mode->vtcallmode(%d),mode->camcord(%d),mode->PreviewActive(%d),mode->StillCount(%d),mode->VideoActive(%d)\n",\
		__func__,mode->vtcallmode,mode->camcordmode,mode->PreviewActive,mode->StillCount,mode->VideoActive);*/

	if(!mode->VideoActive) {
		printk(KERN_ERR "func(%s): xres %u yres %u\n", __func__, mode->xres, mode->yres);
		info->oprmode = S5K4ECGX_OPRMODE_VIDEO;
		info->framesize_index = s5k4ecgx_get_framesize_index(info, mode);
		//printk(KERN_ERR "func(%s): info->framesize_index %d\n", __func__, info->framesize_index);
		err = 0;
	}

	if(mode->PreviewActive && (!mode->VideoActive)) {
		err = s5k4ecgx_set_preview_resolution(info, mode);
		if(err < 0) {
			pr_err("%s: s5k4ecgx_set_preview_resolution() returned %d\n", __func__, err);

			return -EINVAL;
		}
	} else if(mode->StillCount) {
		rear_exif_info.info_flash = 0;
		err = s5k4ecgx_set_capture_mode(info, mode);
		//exif information
		err = s5k4ecgx_get_exif_info(info, &rear_exif_info);
	} else if(mode->VideoActive) {
		/*pr_err("%s: invalid recording resolution supplied to set mode %d %d\n",
				__func__, mode->xres, mode->yres);*/
		err = 0;
	}

	if (err < 0)
		pr_err("%s: s5k4ecgx_set_mode() returned error, %d\n", __func__, err);

	FUNC_EXIT;

	return err;
}


static long s5k4ecgx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	//FUNC_ENTR;

	//struct s5k4ecgx_mode mode;
	struct s5k4ecgx_info *info = file->private_data;

	//printk(KERN_ERR "[S5K4ECGX]:func(%s): cmd(%d) arg(%d)\n", __func__, cmd - 1074032384, arg);

	if(dtpTest == S5K4ECGX_DTP_TEST_ON) {
    	    if( ( cmd != S5K4ECGX_IOCTL_SET_MODE ) &&
		( cmd != S5K4ECGX_IOCTL_DTP_TEST ) ) {
			printk(KERN_ERR "func(%s):line(%d)S5K4ECGX_DTP_TEST_ON. cmd(%d)\n",__func__,__LINE__, cmd);
			return 0;
		}
	}

	switch (cmd) {
		case S5K4ECGX_IOCTL_SET_MODE:
			if (copy_from_user(&rear_mode, (const void __user *)arg, sizeof(struct s5k4ecgx_mode))) {
				pr_info("%s %d\n", __func__, __LINE__);
				return -EFAULT;
			}
			return s5k4ecgx_set_mode(info, &rear_mode);

		case S5K4ECGX_IOCTL_COLOR_EFFECT:
			return s5k4ecgx_set_color_effect(info, (s5k4ecgx_color_effect) arg);

		case S5K4ECGX_IOCTL_WHITE_BALANCE:
			return s5k4ecgx_set_white_balance(info, (s5k4ecgx_white_balance) arg);

		case S5K4ECGX_IOCTL_EXPOSURE:
			return s5k4ecgx_set_exposure(info, (s5k4ecgx_exposure) arg);
#ifdef FACTORY_TEST
		case S5K4ECGX_IOCTL_DTP_TEST:
		{
			int status = -1;
			if (dtpTest == S5K4ECGX_DTP_TEST_ON && (s5k4ecgx_dtp_test) arg == 0) {
				pr_err("[S5K4ECGX]%s: S5K4ECGX_IOCTL_DTP_TEST Entered!!! dtpTest = %d\n", __func__, (s5k4ecgx_dtp_test) arg);
				status = s5k4ecgx_return_normal_preview(info);
			}
			dtpTest = (s5k4ecgx_dtp_test) arg;
			return status;
		}
#endif
		case S5K4ECGX_IOCTL_EXPOSURE_METER:
			return s5k4ecgx_set_exposure_meter(info, (s5k4ecgx_exposure_meter) arg);

		case S5K4ECGX_IOCTL_ISO:
			return s5k4ecgx_set_iso(info, (s5k4ecgx_iso) arg);

		case S5K4ECGX_IOCTL_SCENE_MODE:
			return s5k4ecgx_set_scene_mode(info, (s5k4ecgx_scene_mode) arg);

		case S5K4ECGX_IOCTL_FOCUS_MODE:
			return s5k4ecgx_set_focus_mode(info, (s5k4ecgx_focus_mode) arg);

		case S5K4ECGX_IOCTL_AUTO_CONTRAST:
			return s5k4ecgx_set_auto_contrast(info, (s5k4ecgx_auto_contrast) arg);

		case S5K4ECGX_IOCTL_FLASH_MODE:
			return s5k4ecgx_set_flash_mode(info, (s5k4ecgx_flash_mode) arg);

		case S5K4ECGX_IOCTL_AF_CONTROL:
			return s5k4ecgx_set_af_control(info, (s5k4ecgx_autofocus_control) arg);

		case S5K4ECGX_IOCTL_CAM_MODE:
			info->cam_mode = (s5k4ecgx_cam_mode) arg;
			break;

		case S5K4ECGX_IOCTL_VT_MODE:
			break;

		case S5K4ECGX_IOCTL_AF_RESULT:
			s5k4ecgx_get_af_result(info);
			if (copy_to_user((void __user *)arg, &info->af_result, sizeof(u8))) {
				pr_info("%s %d\n", __func__, __LINE__);
				return -EFAULT;
			}
			//pr_err("%s: s5k4ecgx_ioctl(S5K4ECGX_IOCTL_AF_RESULT) returned arg(0x%x)info->af_result(0x%x)\n", __func__, arg, info->af_result);
			return 0;

		case S5K4ECGX_IOCTL_ESD_RESET:
			if (!dtpTest)
				s5k4ecgx_esdreset(info);
				break;

		case S5K4ECGX_IOCTL_EXIF_INFO:
			//printk(KERN_ERR "func(%s):line(%d) EXIF_INFO\n",__func__,__LINE__);
			/*if (copy_from_user(&rear_exif_info, (const void __user *)arg, sizeof(struct s5k4ecgx_exif_info))) {
				pr_info("%s %d\n", __func__, __LINE__);
				return -EFAULT;
			}*/
			if (copy_to_user((void __user *)arg, &rear_exif_info, sizeof(struct s5k4ecgx_exif_info))) {
				return -EFAULT;
			}
			return 0;

		case S5K4ECGX_IOCTL_TOUCHAF:
		{
			struct s5k4ecgx_touchaf_pos tpos;
			if (copy_from_user(&tpos, (const void __user *) arg, sizeof(struct s5k4ecgx_touchaf_pos))) {
				pr_info("%s %d\n", __func__, __LINE__);
				return -EFAULT;
			}
			return s5k4ecgx_set_touchaf(info, &tpos);
		}

		case S5K4ECGX_IOCTL_CAPTURE_RESOLUTION:
		{
			//pr_err("%s: s5k4ecgx_ioctl(S5K4ECGX_IOCTL_CAPTURE_RESOLUTION) returned not error.\n", __func__);
			struct s5k4ecgx_capture_resolution cres;
			if (copy_from_user(&cres, (const void __user *) arg, sizeof(struct s5k4ecgx_capture_resolution))) {
				pr_info("%s %d\n", __func__, __LINE__);
				return -EFAULT;
			}
			rear_mode.cap_xres = cres.cap_x_res;
			rear_mode.cap_yres = cres.cap_y_res;

			if (info->cam_mode != S5K4ECGX_CAM_MODE_MAX) {
				return s5k4ecgx_set_capture_size(info, &rear_mode);
			}
		}

		case S5K4ECGX_IOCTL_LENS_SOFT_LANDING:
			//pr_err("%s: s5k4ecgx_ioctl(S5K4ECGX_IOCTL_LENS_SOFT_LANDING) returned not error.\n", __func__);
			return 0;

		case S5K4ECGX_IOCTL_RECORDING_FRAME:
			pr_err("%s: s5k4ecgx_ioctl(S5K4ECGX_IOCTL_RECORDING_FRAME) returned not error.\n", __func__);
			return 0;

		case S5K4ECGX_IOCTL_ANTISHAKE:
			pr_err("%s: s5k4ecgx_ioctl(S5K4ECGX_IOCTL_ANTISHAKE) returned not error.\n", __func__);
			return 0;

		case S5K4ECGX_IOCTL_GET_CAPTURE:
			//pr_err("%s: s5k4ecgx_ioctl(S5K4ECGX_IOCTL_GET_CAPTURE) returned not error.\n", __func__);
			s5k4ecgx_get_capture_mode(info, &rear_mode);
			break;

		default:
			pr_err("%s: s5k4ecgx_ioctl() returned error.\n", __func__);
			return -EINVAL;

	}

	return 0;
}

//static struct s5k4ecgx_info *info;

#ifdef FACTORY_TEST
static ssize_t cameraflash_file_cmd_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	/* Reserved */
	return 0;
}

static ssize_t cameraflash_file_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);

	if (value == 0) {
		printk(KERN_INFO "[Factory Test] flash OFF\n");
		info->pdata->torch_onoff(0);
	} else {
		printk(KERN_INFO "[Factory Test] flash ON\n");
		info->pdata->torch_onoff(1);
	}

	return size;
}

static DEVICE_ATTR(cameraflash, 0660, cameraflash_file_cmd_show, cameraflash_file_cmd_store);

static ssize_t camtype_file_cmd_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char camType[] = "SLSI_S5K4ECGX_NONE";

	return sprintf(buf, "%s", camType);
}

static ssize_t camtype_file_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(camtype, 0660, camtype_file_cmd_show, camtype_file_cmd_store);
#endif	/* FACTORY_TEST */

static int s5k4ecgx_open(struct inode *inode, struct file *file)
{
	struct s5k4ecgx_info * pinfo;
	file->private_data = info;
	//struct s5k4ecgx_info * pinfo = (struct s5k4ecgx_info *)file->private_data;
	pinfo = (struct s5k4ecgx_info *)file->private_data;
	int err = -1;

	FUNC_ENTR;

	if (pinfo->pdata && pinfo->pdata->power_on) {
		regulator_enable(reg_mipi_1v2);
		pinfo->pdata->power_on();
	}

#ifdef CONFIG_LOAD_FILE
	{
		err = loadFile();
		if (unlikely(err)) {
			printk("%s: failed to init\n", __func__);
			return err;
		}
	}
#endif

	err = s5k4ecgx_check_vender(info);
	if (err < 0){
		pr_err("%s: s5k4ecgx_check_vender returned error, %d\n", __func__, err);
		pinfo->pdata->power_off();
		regulator_disable(reg_mipi_1v2);
		return -ENODEV;
	}

/* #ifdef CONFIG_LOAD_FILE
	err = loadFile();
	if (unlikely(err)) {
		printk("%s: failed to init\n", __func__);
		return err;
	}
	status = s5k4ecgx_write_tuningmode(pinfo->i2c_client, "mode_sensor_init");
#else
	status = s5k4ecgx_write_table(pinfo->i2c_client, mode_table[0]);
#endif

	if (status < 0)
		pinfo->pdata->power_off();
*/
	info->mode = SYSTEM_INITIALIZE_MODE;
	info->framesize_index = 0;				// for preview resolution
	info->captureframesize_index = 0;			// for capture resolution
	info->gLowLight_check = 0;				// for lowLight check
	info->first_af_check = 0;				// for first af result
	info->giso_index = S5K4ECGX_ISO_AUTO;		// for iso
	info->geffect_index = S5K4ECGX_EFFECT_NONE;		// for effect
	info->gmeter_index = S5K4ECGX_EXPOSURE_METER_CENTER;	// for exposure meter
	info->gev_index = S5K4ECGX_EXPOSURE_ZERO;		// for ev
	info->gwb_index = S5K4ECGX_WB_AUTO;			// for wb
	info->gsharpness_index = S5K4ECGX_SHARPNESS_ZERO;	// for sharpness
	info->gsaturation_index = S5K4ECGX_SATURATION_ZERO;	// for saturation
	info->gscene_index = S5K4ECGX_SCENE_AUTO;		// for scene
	info->gfocus_index = S5K4ECGX_FOCUS_AUTO;		// for Normal, Macro
	info->touch_af_enable = false;
	info->flash_fired = false;
	info->gauto_cont_index = false;				// for auto contrast
	info->gfps_range = false;				// for fps range
	info->pre_flash_ae_skip = 0;				//for When pre flash, 200ms delay is needed.
	info->pre_flash_ae_stable_check = 0;			// for When pre flash, AE stable check.

	rear_mode.xres = 0;
	rear_mode.yres = 0;
	rear_mode.frame_length = 0;
	rear_mode.coarse_time = 0;
	rear_mode.gain = 0;
	rear_mode.PreviewActive = 0;
	rear_mode.VideoActive = 0;
	rear_mode.HalfPress = 0;
	rear_mode.StillCount = 0;
	rear_mode.camcordmode = 0;
	rear_mode.vtcallmode = 0;
	dtpTest = S5K4ECGX_DTP_TEST_OFF;

	FUNC_EXIT;

	//return status;
	return 0;
}

int s5k4ecgx_release(struct inode *inode, struct file *file)
{
	int err = -1;

	struct s5k4ecgx_info * pinfo = (struct s5k4ecgx_info *)file->private_data;

	FUNC_ENTR;

	if (info->cam_mode != S5K4ECGX_CAM_MODE_MAX) {
#ifdef CONFIG_LOAD_FILE
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_Preview_Return");
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Normal_mode_1");
		err = s5k4ecgx_write_tuningmode(info->i2c_client, "s5k4ecgx_AF_Normal_mode_2");
#else
		err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_Preview_Return);
		err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_1);
		err = s5k4ecgx_write_table(info->i2c_client, s5k4ecgx_AF_Normal_mode_2);
#endif
		//msleep(500);
		usleep_range(500000, 600000);//500ms
	}

	/* flash off */
	info->pdata->torch_onoff(0);

	if (pinfo->pdata && pinfo->pdata->power_off){
		pinfo->pdata->power_off();
		regulator_disable(reg_mipi_1v2);
	}

	FUNC_EXIT;

	return 0;
}

static const struct file_operations s5k4ecgx_fileops = {
	.owner = THIS_MODULE,
	.open = s5k4ecgx_open,
	.unlocked_ioctl = s5k4ecgx_ioctl,
	.compat_ioctl = s5k4ecgx_ioctl,
	.release = s5k4ecgx_release,
};

static struct miscdevice s5k4ecgx_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "s5k4ecgx",
	.fops = &s5k4ecgx_fileops,
};

static int s5k4ecgx_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err;
	int dev;
	char s5k4ecgx_name[20] = "s5k4ecgx";
	char s5k4ecgx_pmic_name[20] = "s5k4ecgx_pmic";

	FUNC_ENTR;

	pr_err("%s , %x probing i2c(%lu)", id->name, client->addr, id->driver_data);

	if (strcmp(s5k4ecgx_name, id->name) == 0)
		dev = 0;
	if (strcmp(s5k4ecgx_pmic_name, id->name) == 0)
		dev = 1;

	switch (dev) {
		case 0:
			info = kzalloc(sizeof(*info), GFP_KERNEL);
			if (!info) {
				pr_err("s5k4ecgx: Unable to allocate memory!\n");
				return -ENOMEM;
			}

			info->i2c_client = client;
			if (!info->i2c_client) {
				pr_err("s5k4ecgx: Unknown I2C client!\n");
				err = -ENODEV;
				goto probeerr;
			}

			info->pdata = client->dev.platform_data;
			if (!info->pdata) {
				pr_err("s5k4ecgx: Unknown platform data!\n");
				err = -ENODEV;
				goto probeerr;
			}

			err = misc_register(&s5k4ecgx_device);
			if (err) {
				pr_err("s5k4ecgx: Unable to register misc device!\n");
				goto probeerr;
			}

			s5k4ecgx_dev = device_create(sec_class, NULL, 0, NULL, "sec_s5k4ecgx");
			if (IS_ERR(s5k4ecgx_dev)) {
				pr_err("Failed to create device!");
				goto probeerr;
			}
#ifdef FACTORY_TEST
			if (device_create_file(s5k4ecgx_dev, &dev_attr_cameraflash) < 0) {
				printk("Failed to create flash device file!(%s)!\n", dev_attr_cameraflash.attr.name);
				goto probeerr;
			}

			if (device_create_file(s5k4ecgx_dev, &dev_attr_camtype) < 0) {
				printk("Failed to create camtype device file!(%s)!\n", dev_attr_camtype.attr.name);
				goto probeerr;
			}
#endif
			reg_mipi_1v2 = regulator_get(NULL, "VAP_MIPI_1V2");
			if (IS_ERR(reg_mipi_1v2)) {
				printk(KERN_INFO "%s: VAP_MIPI_1V2 regulator not found\n", __func__);
				reg_mipi_1v2 = NULL;
			}

			break;

		case 1:
			i2c_client_pmic = client;
			break;
	}
	i2c_set_clientdata(client, info);

	FUNC_EXIT;

	return 0;
probeerr:
	kfree(info);
	return err;
}

static int s5k4ecgx_remove(struct i2c_client *client)
{
	struct s5k4ecgx_info *info;

	FUNC_ENTR;

	info = i2c_get_clientdata(client);
	misc_deregister(&s5k4ecgx_device);
	kfree(info);

	FUNC_EXIT;

	return 0;
}

static const struct i2c_device_id s5k4ecgx_id[] = {
	{ "s5k4ecgx", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, s5k4ecgx_id);

static struct i2c_driver s5k4ecgx_i2c_driver = {
	.driver = {
		.name = "s5k4ecgx",
		.owner = THIS_MODULE,
	},
	.probe = s5k4ecgx_probe,
	.remove = s5k4ecgx_remove,
	.id_table = s5k4ecgx_id,
};

static const struct i2c_device_id s5k4ecgx_pmic_id[] = {
	{ "s5k4ecgx_pmic", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, s5k4ecgx_pmic_id);

static struct i2c_driver s5k4ecgx_i2c_pmic_driver = {
	.driver = {
		.name = "s5k4ecgx_pmic",
		.owner = THIS_MODULE,
	},
	.probe = s5k4ecgx_probe,
	.remove = s5k4ecgx_remove,
	.id_table = s5k4ecgx_pmic_id,
};

static int __init s5k4ecgx_init(void)
{
	int status;
	FUNC_ENTR;
	pr_info("s5k4ecgx sensor driver loading\n");
	status = i2c_add_driver(&s5k4ecgx_i2c_driver);
	if (status) {
		pr_err("s5k4ecgx error\n");
		return status;
	}
	status = i2c_add_driver(&s5k4ecgx_i2c_pmic_driver);
	if (status) {
		pr_err("s5k4ecgx_pmic error\n");
		return status;
	}

	FUNC_EXIT;

	return 0;
}

static void __exit s5k4ecgx_exit(void)
{
	FUNC_ENTR;

	i2c_del_driver(&s5k4ecgx_i2c_driver);
}

module_init(s5k4ecgx_init);
module_exit(s5k4ecgx_exit);


