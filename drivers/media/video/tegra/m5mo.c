/*
 * m5mo.c - m5mo sensor driver
 *
 * Copyright (C) 2010 SAMSUNG ELECTRONICS.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <linux/clk.h>
#include <linux/ioctl.h>

#include <media/m5mo.h>

#include <mach/pinmux.h>

#include <media/tegra_camera.h>

#include <linux/file.h>
#include <linux/vmalloc.h>

#include <linux/syscalls.h>
#include <linux/regulator/consumer.h>

#define I2C_WRITE_SIZE 2048

struct m5mo_reg_isp {
	u16 command;
	u32 numbytes;
	u8 data[9];
};

struct m5mo_info {
	int mode;
	struct i2c_client *i2c_client_isp;
	struct m5mo_platform_data *pdata;
	struct m5mo_exif_info exif_info;
	struct m5mo_fw_version fw_ver;
	enum m5mo_scene_mode scenemode;
	enum m5mo_focus_mode focusmode;
	enum m5mo_samsung_camera is_samsung_camera;
	bool power_status;
	bool touchaf_enable;
};

extern struct class *camera_class;
struct device *m5mo_dev;
extern struct i2c_client *i2c_client_pmic;
struct regulator *reg_mipi_1v2;

#ifdef FACTORY_TEST
static enum m5mo_dtp_test dtpTest = M5MO_DTP_TEST_OFF;
#endif

static int read_fw_cnt;
static int cur_cammod;
static int enable_scene;
struct file *fp;
union INFO_CLK {
	long unsigned int a;
	struct tegra_camera_clk_info b;
} info_clk;


#define CAMERA_FW_FILE_PATH	"/system/cameradata/RS_M5LS.bin"
#define CAMERA_FW_FILE_EXTERNAL_PATH	"/sdcard/RS_M5LS.bin"
#define CAMERA_FW_DUMP_FILE_PATH		"/sdcard/m5mo_dump.bin"
#define MISC_PARTITION_PATH	"/dev/block/mmcblk0p6"
#define START_POSITION_OF_VERSION_STRING		0x16ff00
#define LENGTH_OF_VERSION_STRING		21
#define FIRMWARE_ADDRESS_START  0x10000000
#define FIRMWARE_ADDRESS_END    0x101F7FFF

#define M5MO_MAX_RETRIES 4
#define M5MO_FIRMWARE_READ_MAX_COUNT	50
#define M5MO_FIRMEWARE_READ_MAX_BUFFER_SIZE	2051


#define DEBUG_PRINTS 0
#if DEBUG_PRINTS
	#define FUNC_ENTR	\
		printk(KERN_INFO "[m5mo] %s Entered!!!\n", __func__)
	#define I2C_DEBUG 1
#else
	#define FUNC_ENTR
	#define I2C_DEBUG 0
#endif

static int misc_inform_write(int u_count);
static int misc_inform_read(void);

static const struct m5mo_reg_isp mode_scene_auto[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_scene_portrait[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x11} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_scene_landscape[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x06} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_sports[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_scene_party[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_beach[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x06} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_sunset[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x06} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x06} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x03, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_dusk_dawn[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x07} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x07} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x03, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_fall_color[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x08} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x08} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_night[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x09} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x09} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_fire[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x0B} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x0B} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_text[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x0C} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x0C} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x07} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_candle[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x0D} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x0D} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x03, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_scene_back_light[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x0A} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x0A} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x12, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x11, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x1D, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_flash_torch[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} }, /*All flash register Off*/
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x03} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_flash_auto[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_flash_on[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_flash_off[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x41, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x40, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_exposure_p2p0[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x08} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_p1p5[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x07} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_p1p0[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x06} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_p0p5[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x05} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_0[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_m0p5[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x03} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_m1p0[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_m1p5[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_m2p0[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_af_stop[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x02, 0x00} }, /* Stop  AF */
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_af_start[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x02, 0x01} },/* Start AF */
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_af_set_normal_default[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_af_set_macro_default[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_af_set_touch_default[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_touchaf_enable[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_touchaf_pos[] = {
	{M5MO_TABLE_TOUCHAF,	6,	{0x06, 0x02, 0x0A, 0x30, 0x00, 0x00 } },/* Touch AF X position*/
	{M5MO_TABLE_TOUCHAF,	6,	{0x06, 0x02, 0x0A, 0x32, 0x00, 0x00} },/* Touch AF Y position*/
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_caf_start[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x02, 0x02} },/* Start CAF */
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_af_result[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x0A, 0x03, 0x01} },/* AF Result */
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_focus_auto[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_focus_macro[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_focus_fixed[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x06} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_focus_continuous_video[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_focus_face_detect[] = {
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x01, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x02, 0x0B} },
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x11} },
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x03} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_focus_face_detect_disable[] = {
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_lens_soft_landing[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x07} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_isp_parameter[] = {
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_isp_monitor[] = {
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x02} },
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} }, /*clear interrupt7*/
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_isp_moderead[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x00, 0x0B, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};


static const struct m5mo_reg_isp mode_fwver_read[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x00, 0x0A, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_afcal_read[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x0A, 0x1D, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_awbcal_RGread_H[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x0E, 0x3C, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_awbcal_RGread_L[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x0E, 0x3D, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_awbcal_GBread_H[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x0E, 0x3E, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_awbcal_GBread_L[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x0E, 0x3F, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_isp_start[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp mode_gammaeffect_off[] = {
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x0B, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_read_gammaeffect[] = {
	{M5MO_TABLE_READ,		5,	{0x05, 0x01, 0x01, 0x0B, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_coloreffect_off[] = {
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0B, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_coloreffect_none[] = {
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0B, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_coloreffect_mono[] = {
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0B, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x09, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0A, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_coloreffect_sepia[] = {
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0B, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x09, 0xD8} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0A, 0x18} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_coloreffect_posterize[] = { /* antique*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0B, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x09, 0xD0} },
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0A, 0x30} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_coloreffect_negative[] = {
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x0B, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_coloreffect_solarize[] = {
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x0B, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_WB_auto[] = {
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_WB_incandescent[] = {
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x03, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_WB_daylight[] = {
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x03, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_WB_fluorescent[] = {
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x03, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_WB_cloudy[] = {
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x02, 0x02} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x03, 0x05} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp aeawb_lock[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x00, 0x01} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x00, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp aeawb_unlock[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x00, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x06, 0x00, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_exposure_meter_center[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x03} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_meter_spot[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x06} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_exposure_meter_matrix[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x01, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_iso_auto[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x05, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_iso_100[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x05, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_iso_200[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x05, 0x03} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_iso_400[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x05, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_iso_800[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x05, 0x05} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_antishake_off[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x00} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_antishake_on[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0A, 0x0E} },
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x0B, 0x0E} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp antibanding_60hz[] = {
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x06, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_autocontrast_off[] = {
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x25, 0x05} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x2C, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_autocontrast_on[] = {
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x25, 0x09} },
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x2C, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_firmware_write[] = {
	{0x0902,		9,	{0x00, 0x04, 0x50, 0x00, 0x03, 0x08, 0x00, 0x01, 0x7E} },
	{0x0502,		5,	{0x05, 0x02, 0x0F, 0x13, 0x01} },
	{M5MO_TABLE_WRITE_MEMORY,	380928,	{0x68, 0x00, 0x00, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp mode_recording_caf[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_recording_preview[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x01, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

/* face beuaty */
static const struct m5mo_reg_isp mode_face_beauty_off[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x53, 0x00} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static const struct m5mo_reg_isp mode_face_beauty_on[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x53, 0x01} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

/* exif information*/
static struct m5mo_reg_isp mode_exif_exptime_numer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x00, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_exptime_denumer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x04, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_tv_numer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x08, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_tv_denumer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x0C, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_av_numer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x10, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_av_denumer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x14, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_bv_numer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x18, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_bv_denumer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x1C, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_ebv_numer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x20, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_ebv_denumer[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x24, 0x04} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_iso_info[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x28, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};
static struct m5mo_reg_isp mode_exif_flash_info[] = {
	{M5MO_TABLE_READ,	5,	{0x05, 0x01, 0x07, 0x2A, 0x02} },
	{M5MO_TABLE_END,	0,	{0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_1080_change[] = {
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x25} },/*  Preview size to YUV 1920X1080 */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x18} },/* Framerate : 24fps fixed */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,	{0x05, 0x02, 0x0A, 0x02, 0x02} },/* Excute CAF mode*/
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_720_change[] = {
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x21} },/*  Preview size to YUV 1280X720 */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x1E} },/* Framerate : 30fps fixed */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_1920_1080[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x25} },/*  Preview size to YUV 1920X1080 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x01} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x18} },/* Framerate : 24fps fixed */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },/* Chroma Saturation ENABLE*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },/* Chroma Saturation LVL*/
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },/* Face Detection */
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },/* AE_INDEX Default -> Add cuz request of Techwin*/
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_1280_720[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x21} },/*  Preview size to YUV 1280 x 720 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x01} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x1E} },/* Framerate : 30fps fixed */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },/* Chroma Saturation ENABLE*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },/* Chroma Saturation LVL*/
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },/* Face Detection */
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },/* AE_INDEX Default -> Add cuz request of Techwin*/
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_800_480[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x1A} },/*  Preview size to YUV 800 x 480 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x01} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x1E} },/* Framerate : 30fps fixed */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },/* Chroma Saturation ENABLE*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },/* Chroma Saturation LVL*/
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },/* Face Detection */
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },/* AE_INDEX Default -> Add cuz request of Techwin*/
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_720_480[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x18} },/*  Preview size to YUV 720 x 480 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x01} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x1E} },/* Framerate : 30fps fixed */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },/* Chroma Saturation ENABLE*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },/* Chroma Saturation LVL*/
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },/* Face Detection */
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },/* AE_INDEX Default -> Add cuz request of Techwin*/
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_640_480[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x17} },/*  Preview size to YUV 640 x 480 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x01} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x1E} },/* Framerate : 30fps fixed */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },/* Chroma Saturation ENABLE*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },/* Chroma Saturation LVL*/
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },/* Face Detection */
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },/* AE_INDEX Default -> Add cuz request of Techwin*/
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_528_432[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x2C} },/*  Preview size to YUV 528 x 432 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x01} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x1E} },/* Framerate : 30fps fixed */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },/* Chroma Saturation ENABLE*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },/* Chroma Saturation LVL*/
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },/* Face Detection */
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },/* AE_INDEX Default -> Add cuz request of Techwin*/
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_320_240[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x09} },/*  Preview size to YUV 320 x 240 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x01} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x1E} },/* Framerate : 30fps fixed */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },/* Chroma Saturation ENABLE*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },/* Chroma Saturation LVL*/
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },/* Face Detection */
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },/* AE_INDEX Default -> Add cuz request of Techwin*/
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camcorder_176_144[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x05} },/*  Preview size to YUV 176 x 144 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x01} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x1E} },/* Framerate : 30fps fixed */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x10, 0x01} },/* Chroma Saturation ENABLE*/
	{0x0502,		5,	{0x05, 0x02, 0x02, 0x0F, 0x03} },/* Chroma Saturation LVL*/
	{0x0502,		5,	{0x05, 0x02, 0x09, 0x00, 0x00} },/* Face Detection */
	{0x0502,		5,	{0x05, 0x02, 0x03, 0x38, 0x04} },/* AE_INDEX Default -> Add cuz request of Techwin*/
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x01, 0x17} },/*  Preview size to YUV 640 x 480 */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x25} },/*  Capture size to YUV 3264 x 2448 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera_176_144[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x01, 0x05} },/*  Preview size to YUV 176*144 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x00} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera_320_240[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x01, 0x09} },/*  Preview size to YUV 320*240 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x00} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera_528_432[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x01, 0x2C} },/*  Preview size to YUV 528*432 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x00} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera_640_480[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x01, 0x17} },/*  Preview size to YUV 640*480 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x00} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera_720_480[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x01, 0x18} },/*  Preview size to YUV 720*480 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x00} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera_800_480[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x01, 0x1A} },/*  Preview size to YUV 800*480 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x00} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera_1280_720[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x01, 0x21} },/*  Preview size to YUV 1280*720 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x00} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Camera_1280_960[] = {
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x01, 0x24} },/*  Preview size to YUV 1280*960 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x31, 0x00} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Capture_3264x2448[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0C, 0x0A, 0x64} },/* Delay for 100ms */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x25} },/*  Capture size to YUV 3264 x 2448 */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x08} },/* Enable Interrupt factor */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x03} },/* Capture mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,      {0x05, 0x02, 0x0C, 0x06, 0x01} },/* Select image number */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Capture_3264x1968[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0C, 0x0A, 0x64} },/* Delay for 100ms */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x2D} },/*  Capture size to YUV 3264 x 1968 */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x08} },/* Enable Interrupt factor */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x03} },/* Capture mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,      {0x05, 0x02, 0x0C, 0x06, 0x01} },/* Select image number */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Capture_2048x1536[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0C, 0x0A, 0x64} },/* Delay for 100ms */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x1B} },/*  Capture size to YUV 2048 x 1536 */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x08} },/* Enable Interrupt factor */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x03} },/* Capture mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,      {0x05, 0x02, 0x0C, 0x06, 0x01} },/* Select image number */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Capture_2048x1232[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0C, 0x0A, 0x64} },/* Delay for 100ms */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x2C} },/*  Capture size to YUV 2048 x 1232 */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x08} },/* Enable Interrupt factor */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x03} },/* Capture mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,      {0x05, 0x02, 0x0C, 0x06, 0x01} },/* Select image number */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Capture_1280x960[] = {
	{0x0502,                5,      {0x05, 0x02, 0x0C, 0x0A, 0x64} },/* Delay for 100ms */
	{0x0502,                5,      {0x05, 0x02, 0x0B, 0x01, 0x14} },/*  Capture size to YUV 1280 x 960 */
	{0x0502,                5,      {0x05, 0x02, 0x00, 0x11, 0x08} },/* Enable Interrupt factor */
	{0x0502,                5,      {0x05, 0x02, 0x00, 0x0B, 0x03} },/* Capture mode */
	{M5MO_TABLE_STOP,       5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,                5,      {0x05, 0x02, 0x0C, 0x06, 0x01} },/* Select image number */
	{M5MO_TABLE_END,        0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Capture_800x480[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0C, 0x0A, 0x64} },/* Delay for 100ms */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x0A} },/*  Capture size to YUV 800 x 480 */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x08} },/* Enable Interrupt factor */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x03} },/* Capture mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,      {0x05, 0x02, 0x0C, 0x06, 0x01} },/* Select image number */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Capture_640x480[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0C, 0x0A, 0x64} },/* Delay for 100ms */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x09} },/*  Capture size to YUV 640x 480 */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x08} },/* Enable Interrupt factor */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x03} },/* Capture mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,      {0x05, 0x02, 0x0C, 0x06, 0x01} },/* Select image number */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_Capture_1600x1200[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0C, 0x0A, 0x64} },/* Delay for 100ms */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x17} },/*  Capture size to YUV 1600x1200 */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x08} },/* Enable Interrupt factor */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x0B, 0x03} },/* Capture mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,      {0x05, 0x02, 0x0C, 0x06, 0x01} },/* Select image number */
	{M5MO_TABLE_END,	0,      {0x00} }
};

static struct m5mo_reg_isp SetModeSequence_ISP_Capture_transfer[] = {
	{0x0502,		5,	{0x05, 0x02, 0x0C, 0x09, 0x01} },
	{M5MO_TABLE_END,	0,      {0x00} }
};

static struct m5mo_reg_isp SetModeSequence_ISP_Preview_Return[] = {
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{M5MO_TABLE_END,	0,      {0x00} }
};

#ifdef FACTORY_TEST
static struct m5mo_reg_isp SetModeSequence_ISP_Preview_Start_Testpattern[] = {
#if 0 /* Sensor test pattern */
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,		{0x05, 0x02, 0x01, 0x01, 0x24} },/*  Preview size to YUV 1280*960 */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x25} },/*  Capture size to YUV 3264 x 2448 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x07, 0x00} },/* shading off */
	{0x0502,		5,	{0x05, 0x02, 0x0D, 0x1C, 0x02} },/* sensor color bar setting on*/
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
#else /*  ISP test pattern */
	{0x0502,		5,      {0x05, 0x02, 0x0F, 0x12, 0x01} },/* Starts Camera program */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt7 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x00, 0x02} },/* Select output interface to MIPI */
	{0x0502,		5,		{0x05, 0x02, 0x01, 0x01, 0x17} },/*  Preview size to YUV 640*480 */
	{0x0502,		5,      {0x05, 0x02, 0x0B, 0x01, 0x25} },/*  Capture size to YUV 3264 x 2448 */
	{0x0502,		5,      {0x05, 0x02, 0x01, 0x32, 0x00} },/* 00 Monitor 01 MOVIE */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,		5,	{0x05, 0x02, 0x0B, 0x00, 0x00} },/* select main image format */
	{0x0502,		5,	{0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,		5,	{0x05, 0x02, 0x0D, 0x1B, 0x01} },/* ISP test pattern on */
#endif
	{M5MO_TABLE_END,	0,      {0x00} }
};

static struct m5mo_reg_isp SetModeSequence_ISP_Return_Normal_Preview[] = {
#if 0 /* Sensor test pattern */
	{0x0502,		5,	{0x05, 0x02, 0x01, 0x07, 0x01} },/* shading off */
	{0x0502,		5,	{0x05, 0x02, 0x0D, 0x1C, 0x00} },/* sensor color bar setting on*/
	{0x0502,		5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,	5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
#else /*  ISP test pattern */
	{0x0502,		5,	{0x05, 0x02, 0x0D, 0x1B, 0x00} },/* ISP test pattern off */
#endif
	{M5MO_TABLE_END,	0,      {0x00} }
};
#endif

static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Size_176_144[] = {
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x01, 0x05} },/*  Preview size to YUV 640*480 */
	{M5MO_TABLE_END,      0,      {0x00} }
};
static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Size_320_240[] = {
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x01, 0x09} },/*  Preview size to YUV 640*480 */
	{M5MO_TABLE_END,      0,      {0x00} }
};
static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Size_528_432[] = {
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x01, 0x2C} },/*  Preview size to YUV 640*480 */
	{M5MO_TABLE_END,      0,      {0x00} }
};
static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Size_720_480[] = {
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x01, 0x18} },/*  Preview size to YUV 640*480 */
	{M5MO_TABLE_END,      0,      {0x00} }
};
static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Size_VGA[] = {
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x01, 0x17} },/*  Preview size to YUV 640*480 */
	{M5MO_TABLE_END,      0,      {0x00} }
};
static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Size_WVGA[] = {
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x01, 0x1A} },/*  Preview size to YUV 800*480 */
	{M5MO_TABLE_END,      0,      {0x00} }
};
static const struct m5mo_reg_isp SetModeSequence_ISP_Preview_Size_HD[] = {
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x02, 0x01} },/* Framerate : auto */
	{0x0502,                5,      {0x05, 0x02, 0x01, 0x01, 0x21} },/*  Preview size to YUV 1280*720 */
	{M5MO_TABLE_END,      0,      {0x00} }
};

static const struct m5mo_reg_isp SetModeSequence_ISP_State_Monitor[] = {
	{0x0502,                5,      {0x05, 0x02, 0x00, 0x11, 0x01} },/* Enable Interrupt factor */
	{0x0502,                5,      {0x05, 0x02, 0x00, 0x0B, 0x02} },/* Go to monitor mode */
	{M5MO_TABLE_STOP,     5,      {0x05, 0x01, 0x00, 0x10, 0x01} },/* clear interrupt */
	{0x0502,                5,      {0x05, 0x02, 0x03, 0x00, 0x00} },/* unlock AE */
	{0x0502,                5,      {0x05, 0x02, 0x06, 0x00, 0x00} },/* unlock AWB */
	{M5MO_TABLE_END,      0,      {0x00} }
};

enum {
	m5mo_MODE_1280x720_MON,
	m5mo_MODE_1280x960_MON,/*Using for Preview and Capture*/
	m5mo_MODE_1920x1080_MON,
	m5mo_MODE_1600x1200_CAP,
	m5mo_MODE_3264x2448_CAP,
};

enum {
	SYSTEM_INITIALIZE_MODE,
	MONITOR_MODE,
	CAPTURE_MODE
};

#if 0
static const struct m5mo_reg_isp *mode_table[] = {
	[m5mo_MODE_1280x720_MON] = SetModeSequence_ISP_Preview_Start_Camcorder_720_change,
	[m5mo_MODE_1280x960_MON] = SetModeSequence_ISP_Preview_Return,
	[m5mo_MODE_1920x1080_MON] = SetModeSequence_ISP_Preview_Start_Camcorder_1080_change,
	[m5mo_MODE_1600x1200_CAP] = SetModeSequence_ISP_Capture_1600x1200,
	[m5mo_MODE_3264x2448_CAP] = SetModeSequence_ISP_Capture_3264x2448,
};
#endif

static const struct m5mo_reg_isp *scene_table[] = {
	[SCENE_AUTO] = mode_scene_auto,
	[SCENE_PORTRAIT] = mode_scene_portrait,
	[SCENE_LANDSCAPE] = mode_scene_landscape,
	[SCENE_NIGHT] = mode_scene_night,
	[SCENE_SPORTS] = mode_scene_sports,
	[SCENE_PARTY] = mode_scene_party,
	[SCENE_BEACH] = mode_scene_beach,
	[SCENE_SUNSET] = mode_scene_sunset,
	[SCENE_DUSK_DAWN] = mode_scene_dusk_dawn,
	[SCENE_FALL_COLOR] = mode_scene_fall_color,
	[SCENE_FIRE_WORK] = mode_scene_fire,
	[SCENE_TEXT] = mode_scene_text,
	[SCENE_CANDLE_LIGHT] = mode_scene_candle,
	[SCENE_BACK_LIGHT] = mode_scene_back_light,
};


static const struct m5mo_reg_isp *wb_table[] = {
	[WB_AUTO] = mode_WB_auto,
	[WB_DAYLIGHT] = mode_WB_daylight,
	[WB_INCANDESCENT] = mode_WB_incandescent,
	[WB_FLUORESCENT] = mode_WB_fluorescent,
	[WB_CLOUDY] = mode_WB_cloudy
};

static const struct m5mo_reg_isp *flash_table[] = {
	[FLASH_AUTO] = mode_flash_auto,
	[FLASH_ON] = mode_flash_on,
	[FLASH_OFF] = mode_flash_off,
	[FLASH_TORCH] = mode_flash_torch
};

static const struct m5mo_reg_isp *exposure_table[] = {
	[EXPOSURE_P2P0] = mode_exposure_p2p0,
	[EXPOSURE_P1P5] = mode_exposure_p1p5,
	[EXPOSURE_P1P0] = mode_exposure_p1p0,
	[EXPOSURE_P0P5] = mode_exposure_p0p5,
	[EXPOSURE_ZERO] = mode_exposure_0,
	[EXPOSURE_M0P5] = mode_exposure_m0p5,
	[EXPOSURE_M1P0] = mode_exposure_m1p0,
	[EXPOSURE_M1P5] = mode_exposure_m1p5,
	[EXPOSURE_M2P0] = mode_exposure_m2p0
};

static const struct m5mo_reg_isp *exposure_meter_table[] = {
	[EXPOSURE_METER_CENTER] = mode_exposure_meter_center,
	[EXPOSURE_METER_SPOT] = mode_exposure_meter_spot,
	[EXPOSURE_METER_MATRIX] = mode_exposure_meter_matrix
};

static const struct m5mo_reg_isp *iso_table[] = {
	[ISO_AUTO] = mode_iso_auto,
	[ISO_100] = mode_iso_100,
	[ISO_200] = mode_iso_200,
	[ISO_400] = mode_iso_400,
	[ISO_800] = mode_iso_800
};

static const struct m5mo_reg_isp *antishake_table[] = {
	[ANTISHAKE_OFF] = mode_antishake_off,
	[ANTISHAKE_ON] = mode_antishake_on
};

static const struct m5mo_reg_isp *autocontrast_table[] = {
	[AUTOCONTRAST_OFF] = mode_autocontrast_off,
	[AUTOCONTRAST_ON] = mode_autocontrast_on
};

static struct m5mo_info *info;

int m5mo_write_reg(struct i2c_client *client, u16 addr, u8 val)
{
	int err;
	struct i2c_msg msg;
	unsigned char data[3];
	int retry = 0;
	FUNC_ENTR;

	if (!client->adapter)
		return -ENODEV;

	data[0] = (0xFF00 & addr) >> 8;
	data[1] = (0x00FF & addr);
	data[2] = val;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 3;
	msg.buf = data;
#if I2C_DEBUG
	int i;
	for (i = 0; i < msg.len; i++)
		pr_err(KERN_INFO "0x%2x ", msg.buf[i]);
	pr_err(KERN_INFO "\n");
#endif

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;
		retry++;
		pr_err("m5mo: i2c transfer failed, retrying %x %x\n",
				addr, val);
		msleep(3);
	} while (retry <= M5MO_MAX_RETRIES);

	return err;
}

int m5mo_write_reg8(struct i2c_client *client, u8 addr, u8 val)
{
	int err;
	struct i2c_msg msg;
	unsigned char data[2];
	int retry = 0;
	FUNC_ENTR;

	if (!client->adapter)
		return -ENODEV;

	data[0] = addr;
	data[1] = val;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = data;
#if I2C_DEBUG
	int i;
	for (i = 0; i < msg.len; i++)
		pr_err(KERN_INFO "0x%2x ", msg.buf[i]);
	pr_err(KERN_INFO "\n");
#endif

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;
		retry++;
		pr_err("m5mo: i2c transfer failed, retrying %x %x\n",
				addr, val);
		msleep(3);
	} while (retry <= M5MO_MAX_RETRIES);

	return err;
}

int m5mo_write_table(struct i2c_client *client,
		const struct m5mo_reg *table,
		int size)
{
	int err;
	const struct m5mo_reg *next;
	FUNC_ENTR;

	for (next = table; next->addr != M5MO_TABLE_END; next++) {
		if (next->addr == M5MO_TABLE_WAIT_MS) {
			msleep(next->val);
			continue;
		}

		if (next->addr == M5MO_TABLE_WAIT_US) {
			udelay(next->val);
			continue;
		}
		if (size == 2)
			err = m5mo_write_reg8 \
			      (client, next->addr, next->val);
		else if (size == 3)
			err = m5mo_write_reg \
			      (client, next->addr, next->val);
		if (err)
			return err;
	}
	return 0;
}

static int m5mo_write_i2c(struct i2c_client *client, u8 *pdata, u16 flags, u16 len)
{
	int err;
	int retry;

	struct i2c_msg msg = {
		.addr = client->addr,
		.buf = pdata,
		.flags = flags,
		.len = len
	};

	FUNC_ENTR;
	retry = 0;
	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		pr_err("m5mo: i2c transfer failed, retrying(client : %d, msg : %d) - open(%d)\n",
				client->addr, msg.addr, msg.flags);
		msleep(3);
	} while (++retry <= M5MO_MAX_RETRIES);
#if I2C_DEBUG
	{
	int i;
	pr_debug("%d size write : \n", msg.len);
	for (i = 0; i < msg.len; i++)
			pr_info("0x%2x ", msg.buf[i]);
		pr_info("\n");
	}
#endif

	return err;
}

#if 0
static int check_exist_file(unsigned char *filename)
{
	struct file *ftestexist = NULL;
	int ret;

	FUNC_ENTR;

	ftestexist = filp_open(filename, O_RDONLY, 0);
	pr_debug("%s : (%p)\n", __func__, ftestexist);

	if (IS_ERR_OR_NULL(ftestexist))
		ret = -1;
	else {
		ret = 0;
		filp_close(ftestexist, current->files);
	}
	pr_debug("%s : (%d)\n", __func__, ret);
	return ret;
}
#endif

static unsigned int get_file_size(unsigned char *filename)
{
	unsigned int file_size = 0;
	struct file *filep = NULL;
	mm_segment_t old_fs;
	unsigned char external_fw[30] = {CAMERA_FW_FILE_EXTERNAL_PATH};

	FUNC_ENTR;

	filep = filp_open(external_fw, O_RDONLY, 0) ;
	if (IS_ERR_OR_NULL(filep)) {
		filep = filp_open(filename, O_RDONLY, 0) ;
		if (IS_ERR_OR_NULL(filep))
			return file_size;
	} else {
		strcpy(filename, external_fw);
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	file_size = filep->f_op->llseek(filep, 0, SEEK_END);

	filp_close(filep, current->files);

	set_fs(old_fs);

	pr_debug("%s: File size is %d\n", __func__, file_size);

	return file_size;
}

static unsigned int read_fw_data
	(unsigned char *buf, unsigned char *filename, unsigned int size)
{
	struct file *filep;
	mm_segment_t old_fs;

	FUNC_ENTR;

	filep = filp_open(filename, O_RDONLY, 0) ;

	if (IS_ERR_OR_NULL(filep))
		return 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	filep->f_op->read(filep, buf, size, &filep->f_pos);

	filp_close(filep, current->files);

	set_fs(old_fs);

	return 1;
}

static int m5mo_write_table_Isp(struct m5mo_info *info,
		const struct m5mo_reg_isp table[],
		void *userdata)
{
	const struct m5mo_reg_isp *next;
	int i;
	u8 readbuffer[32];
	u8 pdata[32];
	unsigned int intstate;
	int ret = 1;
	struct i2c_client *client = info->i2c_client_isp;

	struct m5mo_touchaf_pos *tpos;
	u8 *rdata;

	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(get_ds());

	FUNC_ENTR;

	for (next = table; next->command != M5MO_TABLE_END; next++) {
		if (next->command == M5MO_TABLE_WAIT_MS) {
			msleep(next->numbytes);
			continue;
		} else if (next->command == M5MO_TABLE_STOP) {
			/*waiting for I2C*/
			i = 0;
			pr_info("%s waiting ISP INT... (line : %d)\n", __func__, __LINE__);
			do {
				intstate = info->pdata->isp_int_read();
				if (i == 150)	{
					i = 0;
					pr_err("%s No Interrupt, cancel waiting loop... (line : %d)\n"
							, __func__, __LINE__);
					return -1;
				}
				i++;
				msleep(10);
			} while (intstate != 1);

			for (i = 0; i < next->numbytes; i++)
				pdata[i] = next->data[i];

			ret = m5mo_write_i2c(client, pdata, 0, next->numbytes);
			if (ret != 1)
				return -1;

			pr_info("INT clear..\n");

			ret = m5mo_write_i2c(client, readbuffer, I2C_M_RD, 2);
			if (ret != 1)
				return -1;

			pr_info("INT cleared!! %x%x\n", readbuffer[0], readbuffer[1]);

			i = 0;

			do {
				intstate = info->pdata->isp_int_read();
				if (i == 150) {
					i = 0;
					pr_debug("%s No Interrupt, cancel waiting loop... (line : %d)\n",
							__func__, __LINE__);
					return -1;
				}
				i++;
				msleep(10);
			} while (intstate);
		} else if (next->command == M5MO_TABLE_WRITE_MEMORY) {
			unsigned char *bdata;
			unsigned char *fw_code = NULL;
			unsigned int address;
			unsigned int flash_addr = 0x10000000;
			unsigned int flash_addr2 = 0x101F0000;
			unsigned int fw_filesize;
			unsigned int fw_size = 0;
			unsigned int k;
			unsigned int addr = 0;
			unsigned int fw_addr = 0;
			unsigned int false = 0;
			unsigned int count = 0;
			unsigned int retry = 0;
			unsigned char fw[30] = {CAMERA_FW_FILE_PATH};

			pr_info("--FIRMWARE WRITE--\n");

			fw_filesize = get_file_size(fw);

			/* To store the firmware code */
			fw_size += ((fw_filesize-1) & ~0x3) + 4;
			pr_info("1.filename = %s, fw_size = %d\n", fw, fw_size);
			pr_info("2.filename = %s, filesize = %d\n", fw, fw_filesize);

			if (fw_size >= 0) {
				fw_code = vmalloc(fw_size);
				if (NULL == fw_code) {
					pr_err("fw_code is NULL!!!\n");
					return false;
				} else {
					pr_err("fw_code address = %p, %p\n", fw_code, &fw_code);
				}
			} else {
				pr_err("Invalid input %d\n", fw_size);
				return -1;
			}

			if (!read_fw_data(fw_code, fw, fw_filesize)) {
				pr_err("Error reading firmware file.\n");
				goto write_fw_err;
			}

			addr = (unsigned int) fw_code;
			for (count = 0; count < 31; count++) {
				pr_info("count: %d", count);
				/* Set FLASH ROM memory address */
				pdata[0] = 0x08;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x00;
				pdata[4] = (unsigned char) (flash_addr >> 24);
				pdata[5] = (unsigned char) ((flash_addr >> 16) & 0xff);
				pdata[6] = (unsigned char) ((flash_addr >> 8) & 0xff);
				pdata[7] = (unsigned char) (flash_addr & 0xff);

				ret = m5mo_write_i2c(client, pdata, 0, 8);
				if (ret != 1)
					goto write_fw_err;

				/* Erase FLASH ROM entire memory */
				pdata[0] = 0x05;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x06;
				pdata[4] = 0x01;

				ret = m5mo_write_i2c(client, pdata, 0, 5);
				if (ret != 1)
					goto write_fw_err;

				retry = 0;
				do {
					/* Response while sector-erase is operating. */
					pdata[0] = 0x05;
					pdata[1] = 0x01;
					pdata[2] = 0x0F;
					pdata[3] = 0x06;
					pdata[4] = 0x01;

					ret = m5mo_write_i2c(client, pdata, 0, 5);
					if (ret != 1)
						goto write_fw_err;

					ret = m5mo_write_i2c(client, readbuffer, I2C_M_RD, 2);
					if (ret != 1)
						goto write_fw_err;

					if (readbuffer[1] == 0x00)
						break;

					msleep(20);
				} while (++retry < M5MO_FIRMWARE_READ_MAX_COUNT);

				/* Set FLASH ROM programming size to 64kB */
				pdata[0] = 0x06;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x04;
				pdata[4] = 0x00;
				pdata[5] = 0x00;

				ret = m5mo_write_i2c(client, pdata, 0, 6);
				if (ret != 1)
					goto write_fw_err;

				/* clear M-5MoLS internal ram */
				pdata[0] = 0x05;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x08;
				pdata[4] = 0x01;

				ret = m5mo_write_i2c(client, pdata, 0, 5);
				if (ret != 1)
					goto write_fw_err;

				msleep(10);

				/* Set FLASH ROM memory address */
				pdata[0] = 0x08;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x00;
				pdata[4] = (unsigned char) (flash_addr >> 24);
				pdata[5] = (unsigned char) ((flash_addr >> 16) & 0xff);
				pdata[6] = (unsigned char) ((flash_addr >> 8) & 0xff);
				pdata[7] = (unsigned char) (flash_addr & 0xff);

				ret = m5mo_write_i2c(client, pdata, 0, 8);
				if (ret != 1)
					goto write_fw_err;

				address = 0x68000000;

				bdata = vmalloc(I2C_WRITE_SIZE + 8);
				if (!bdata)
					goto write_fw_err;

				for (k = 0; k < 0x10000; k += I2C_WRITE_SIZE) {

					bdata[0] = 0x00;
					bdata[1] = 0x04;
					bdata[2] = (address & 0xFF000000) >> 24;
					bdata[3] = (address & 0x00FF0000) >> 16;
					bdata[4] = (address & 0x0000FF00) >> 8;
					bdata[5] = (address & 0x000000FF);
					bdata[6] = (0xFF00 & I2C_WRITE_SIZE) >> 8;
					bdata[7] = (0x00FF & I2C_WRITE_SIZE);

					fw_addr = addr + (count * 0x10000) + k;

					memcpy(bdata+8 , (char *)fw_addr,
						I2C_WRITE_SIZE);

					address += I2C_WRITE_SIZE;
					ret = m5mo_write_i2c(client, bdata, 0, I2C_WRITE_SIZE + 8);
					if (ret != 1) {
						vfree(bdata);
						goto write_fw_err;
					}
				}
				vfree(bdata);

				/* Start programming */
				pdata[0] = 0x05;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x07;
				pdata[4] = 0x01;

				ret = m5mo_write_i2c(client, pdata, 0, 5);
				if (ret != 1)
					goto write_fw_err;

				retry = 0;
				do {
					/* Response while sector-erase is operating */
					pdata[0] = 0x05;
					pdata[1] = 0x01;
					pdata[2] = 0x0F;
					pdata[3] = 0x07;
					pdata[4] = 0x01;

					ret = m5mo_write_i2c(client, pdata, 0, 5);
					if (ret != 1)
						goto write_fw_err;

					ret = m5mo_write_i2c(client, readbuffer, I2C_M_RD, 2);
					if (ret != 1)
						goto write_fw_err;

					if (readbuffer[1] == 0x00)
						break;
					msleep(20);
				} while (++retry < M5MO_FIRMWARE_READ_MAX_COUNT);

				msleep(20);

				flash_addr += 0x10000;
			}

			for (count = 0; count < 4; count++) {
				pr_info("count: %d", count);
				/* Set FLASH ROM memory address */
				pdata[0] = 0x08;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x00;
				pdata[4] = (unsigned char) (flash_addr2 >> 24);
				pdata[5] = (unsigned char) ((flash_addr2 >> 16) & 0xff);
				pdata[6] = (unsigned char) ((flash_addr2 >> 8) & 0xff);
				pdata[7] = (unsigned char) (flash_addr2 & 0xff);

				ret = m5mo_write_i2c(client, pdata, 0, 8);
				if (ret != 1)
					goto write_fw_err;

				/* Erase FLASH ROM entire memory */
				pdata[0] = 0x05;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x06;
				pdata[4] = 0x01;

				ret = m5mo_write_i2c(client, pdata, 0, 5);
				if (ret != 1)
					goto write_fw_err;

				retry = 0;
				do {
					/* Response while sector-erase is operating. */
					pdata[0] = 0x05;
					pdata[1] = 0x01;
					pdata[2] = 0x0F;
					pdata[3] = 0x06;
					pdata[4] = 0x01;

					ret = m5mo_write_i2c(client, pdata, 0, 5);
					if (ret != 1)
						goto write_fw_err;

					ret = m5mo_write_i2c(client, readbuffer, I2C_M_RD, 2);
					if (ret != 1)
						goto write_fw_err;

					if (readbuffer[1] == 0x00)
						break;
					msleep(20);
				} while (++retry < M5MO_FIRMWARE_READ_MAX_COUNT);

				/* Set FLASH ROM programming size to 64kB */
				pdata[0] = 0x06;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x04;
				pdata[4] = 0x20;
				pdata[5] = 0x00;

				ret = m5mo_write_i2c(client, pdata, 0, 6);
				if (ret != 1)
					goto write_fw_err;

				/* clear M-5MoLS internal ram */
				pdata[0] = 0x05;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x08;
				pdata[4] = 0x01;

				ret = m5mo_write_i2c(client, pdata, 0, 5);
				if (ret != 1)
					goto write_fw_err;

				msleep(10);

				/* Set FLASH ROM memory address */
				pdata[0] = 0x08;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x00;
				pdata[4] = (unsigned char) (flash_addr2 >> 24);
				pdata[5] = (unsigned char) ((flash_addr2 >> 16) & 0xff);
				pdata[6] = (unsigned char) ((flash_addr2 >> 8) & 0xff);
				pdata[7] = (unsigned char) (flash_addr2 & 0xff);

				ret = m5mo_write_i2c(client, pdata, 0, 8);
				if (ret != 1)
					goto write_fw_err;

				address = 0x68000000;

				bdata = vmalloc(I2C_WRITE_SIZE + 8);
				if (!bdata)
					goto write_fw_err;

				for (k = 0; k < 0x2000; k += I2C_WRITE_SIZE) {
					bdata[0] = 0x00;
					bdata[1] = 0x04;
					bdata[2] = (address & 0xFF000000) >> 24;
					bdata[3] = (address & 0x00FF0000) >> 16;
					bdata[4] = (address & 0x0000FF00) >> 8;
					bdata[5] = (address & 0x000000FF);
					bdata[6] = (0xFF00 & I2C_WRITE_SIZE) >> 8;
					bdata[7] = (0x00FF & I2C_WRITE_SIZE);

					fw_addr = addr + (31 * 0x10000) + (count * 0x2000) + k;

					memcpy(bdata + 8, (char *)fw_addr,
						I2C_WRITE_SIZE);
					address += I2C_WRITE_SIZE;
					ret = m5mo_write_i2c(client, bdata, 0, I2C_WRITE_SIZE + 8);
					if (ret != 1) {
						vfree(bdata);
						goto write_fw_err;
					}
				}
				vfree(bdata);

				/* Start programming */
				pdata[0] = 0x05;
				pdata[1] = 0x02;
				pdata[2] = 0x0F;
				pdata[3] = 0x07;
				pdata[4] = 0x01;

				ret = m5mo_write_i2c(client, pdata, 0, 5);
				if (ret != 1)
					goto write_fw_err;

				retry = 0;
				do {
					/* Response while sector-erase is operating. */
					pdata[0] = 0x05;
					pdata[1] = 0x01;
					pdata[2] = 0x0F;
					pdata[3] = 0x07;
					pdata[4] = 0x01;

					ret = m5mo_write_i2c(client, pdata, 0, 5);
					if (ret != 1)
						goto write_fw_err;

					ret = m5mo_write_i2c(client, readbuffer, I2C_M_RD, 2);
					if (ret != 1)
						goto write_fw_err;

					if (readbuffer[1] == 0x00)
						break;

					msleep(20);
				} while (++retry < M5MO_FIRMWARE_READ_MAX_COUNT);
				msleep(20);

				flash_addr2 += 0x2000;
			}
			vfree(fw_code);

			pr_info("--FIRMWARE WRITE FINISH!!!--\n");
			return 0;
write_fw_err:
			pr_info("--FIRMWARE WRITE ABORTED DUE TO ERROR!!!--\n");
			vfree(fw_code);
			return -1;
		} else if (next->command == M5MO_TABLE_READ) {
			for (i = 0; i < next->numbytes; i++)
				pdata[i] = next->data[i];

			ret = m5mo_write_i2c(client, pdata, 0, next->numbytes);
			if (ret != 1)
				return -1;

			if (userdata)
				rdata = (u8 *) userdata;
			else
				return -1;

			ret = m5mo_write_i2c(client, rdata, I2C_M_RD, pdata[4] + 1);
			if (ret != 1)
				return -1;
		} else if (next->command == M5MO_TABLE_TOUCHAF) {
			unsigned int position;
			if (userdata)
				tpos = (struct m5mo_touchaf_pos *) userdata;
			else
				return -1;
			/*in case of touch af max numbytes is 6
			  , and remain number 2 is position*/
			for (i = 0; i < next->numbytes - 2; i++)
				pdata[i] = next->data[i];

			if (pdata[3] == 0x30)
				position = tpos->xpos;
			else
				position = tpos->ypos;

			pdata[4] = (position & 0x0000FF00) >> 8;
			/* touch AF position value is just 2 bye.*/
			pdata[5] = (position & 0x000000FF);

			/*TODO make i2c behavior*/
			ret = m5mo_write_i2c(client, pdata, 0, next->numbytes);
			if (ret != 1)
				return -1;
		} else {
			/*Write I2C Data*/
			for (i = 0; i < next->numbytes; i++)
				pdata[i] = next->data[i];

			ret = m5mo_write_i2c(client, pdata, 0, next->numbytes);
			if (ret != 1)
				return -1;
		}
	}
	return 0;
}

static int m5mo_firmware_write(struct m5mo_info *info)
{
	int status = 0;

	status = m5mo_write_table_Isp(info, mode_firmware_write, NULL);
	mdelay(100);
	return status;
}

static int m5mo_check_firmware_version(struct m5mo_info *fw_info)
{
	int err, i, m_pos = 0;
	char t_buf[30] = {0,};

	for (i = 0; i < 6; i++) {
		err = m5mo_write_table_Isp(info, mode_fwver_read, t_buf);
		fw_info->fw_ver.unique_id[m_pos] = t_buf[1];
		m_pos++;
	}

	fw_info->fw_ver.unique_id[i] = '\0';

	pr_debug("*************************************\n");
	pr_debug("F/W Version: %s\n", fw_info->fw_ver.unique_id);
	pr_debug("*************************************\n");

	return err;
}

static int m5mo_get_exif_info(struct m5mo_info *info)
{
	int status, i;
	u8 val[5] = {0};

	/* standard values */
	u16 iso_std_values[] = { 10, 12, 16, 20, 25, 32, 40, 50, 64, 80,
		100, 125, 160, 200, 250, 320, 400, 500, 640, 800,
		1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400, 8000};
	/* quantization table */
	u16 iso_qtable[] = { 11, 14, 17, 22, 28, 35, 44, 56, 71, 89,
		112, 141, 178, 224, 282, 356, 449, 565, 712, 890,
		1122, 1414, 1782, 2245, 2828, 3564, 4490, 5657, 7127, 8909};

	struct m5mo_exif_info *exif_info = &info->exif_info;

	status = m5mo_write_table_Isp(info, mode_exif_exptime_numer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_exptime_numer = val[4] +
		(val[3] << 8) + (val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_exptime_numer = %d, %d, %d\n",
		exif_info->info_exptime_numer, val[1], val[2]);

	status = m5mo_write_table_Isp(info, mode_exif_exptime_denumer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_exptime_denumer = val[4] +
		(val[3] << 8) + (val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_exptime_denumer = %d\n",
		exif_info->info_exptime_denumer);

	status = m5mo_write_table_Isp(info, mode_exif_tv_numer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_tv_numer = val[4] +
		(val[3] << 8) + (val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_tv_numer = %d\n",
		exif_info->info_tv_numer);

	status = m5mo_write_table_Isp(info, mode_exif_tv_denumer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_tv_denumer = val[4] +
		(val[3] << 8) + (val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_tv_denumer = %d\n",
		exif_info->info_tv_denumer);

	status = m5mo_write_table_Isp(info, mode_exif_av_numer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_av_numer = val[4] +
		(val[3] << 8) + (val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_av_numer = %d\n",
		exif_info->info_av_numer);

	status = m5mo_write_table_Isp(info, mode_exif_av_denumer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_av_denumer = val[4] +
		(val[3] << 8) + (val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_av_denumer = %d\n",
		exif_info->info_av_denumer);

	status = m5mo_write_table_Isp(info, mode_exif_bv_numer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_bv_numer = val[4] +
		(val[3] << 8) + (val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_bv_numer = %d\n",
		exif_info->info_bv_numer);

	status = m5mo_write_table_Isp(info, mode_exif_bv_denumer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_bv_denumer = val[4] +
		(val[3] << 8) + (val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_bv_denumer = %d\n",
		exif_info->info_bv_denumer);

	status = m5mo_write_table_Isp(info, mode_exif_ebv_numer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_ebv_numer = val[4] + (val[3] << 8) +
		(val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_ebv_numer = %d\n",
		exif_info->info_ebv_numer);

	status = m5mo_write_table_Isp(info, mode_exif_ebv_denumer, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_ebv_denumer = val[4] + (val[3] << 8) +
		(val[2] << 16) + (val[1] << 24);
	pr_debug("m5mo_get_exif_info: info_ebv_denumer = %d\n",
		exif_info->info_ebv_denumer);

	status = m5mo_write_table_Isp(info, mode_exif_iso_info, val);
	if (status < 0)
		goto exif_err;
	exif_info->info_iso = (u16)(val[2] + (val[1] << 8));
	for (i = 0; i < sizeof(iso_qtable); i++) {
		if (exif_info->info_iso <= iso_qtable[i]) {
			exif_info->info_iso = iso_std_values[i];
			break;
		}
	}
	pr_debug("m5mo_get_exif_info: info_iso = %u\n", exif_info->info_iso);

	status = m5mo_write_table_Isp(info, mode_exif_flash_info, val);
	if (status < 0)
		goto exif_err;
	if ((val[2] + (val[1] << 8)) == 0x18 ||
		(val[2] + (val[1] << 8)) == 0x10) /*flash is not fire*/
		exif_info->info_flash = 0;
	else
		exif_info->info_flash = 1;
	pr_debug("m5mo_get_exif_info: read_val = 0x%xm, info_flash = %d\n",
		val[2] + (val[1] << 8), exif_info->info_flash);

	return 0;
exif_err:
	pr_err("m5mo_get_exif_info: error\n");
	return status;
}

static int m5mo_verify_isp_mode(struct m5mo_info *info, enum m5mo_isp_mode isp_mode)
{
	int i,  err;
	u8 status[2] = {0};
	FUNC_ENTR;
	for (i = 0; i < 100; i++) {
		err = m5mo_write_table_Isp(info, mode_isp_moderead, status);

		if (err < 0)
			return -1;

		pr_debug("%s: Isp mode status = 0x%x, trial = %d\n ", __func__, status[1], i);

		if (isp_mode == MODE_PARAMETER_SETTING) {
			if (status[1] == MODE_PARAMETER_SETTING)
				return 0;
		} else if (isp_mode == MODE_MONITOR) {
			if (status[1] == MODE_MONITOR)
				return 0;
		}
		msleep(20);
	}

	return -EBUSY;
}

static int m5mo_set_preview_resolution
	(struct m5mo_info *info, struct m5mo_mode *mode)
{
	int err = 0;
	FUNC_ENTR;

	if (dtpTest) {
		pr_err("%s:  dtpTest = %d\n", __func__, dtpTest);
		err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Testpattern, NULL);
		if (err < 0) {
			pr_err("%s fail, line is%d\n",
					__func__, __LINE__);
			return -1;
		}
		return 0;
	}

	if (info->mode == CAPTURE_MODE) {
		err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Return, NULL);
		if (err < 0) {
			pr_err("%s fail, line is%d\n",
					__func__, __LINE__);
			return -1;
		}
		return 0;
	}

	if (cur_cammod) {
		if (mode->xres == 1280 && mode->yres == 720)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camcorder_1280_720, NULL);
		else if (mode->xres == 800 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camcorder_800_480, NULL);
		else if (mode->xres == 720 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camcorder_720_480, NULL);
		else if (mode->xres == 640 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camcorder_640_480, NULL);
		else if (mode->xres == 528 && mode->yres == 432)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camcorder_528_432, NULL);
		else if (mode->xres == 320 && mode->yres == 240)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camcorder_320_240, NULL);
		else if (mode->xres == 176 && mode->yres == 144)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camcorder_176_144, NULL);
		else {
			pr_err("%s: invalid preview resolution supplied to set mode %d %d\n",
					__func__, mode->xres, mode->yres);
			return -EINVAL;
		}
		if (err < 0) {
			pr_err("%s: set mode fail %d %d %d\n",
					__func__, cur_cammod, mode->xres, mode->yres);
			return -1;
		}

	} else {
		if (mode->xres == 1280 && mode->yres == 720)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camera_1280_720, NULL);
		else if (mode->xres == 800 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camera_800_480, NULL);
		else if (mode->xres == 720 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camera_720_480, NULL);
		else if (mode->xres == 640 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camera_640_480, NULL);
		else if (mode->xres == 528 && mode->yres == 432)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camera_528_432, NULL);
		else if (mode->xres == 320 && mode->yres == 240)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camera_320_240, NULL);
		else if (mode->xres == 176 && mode->yres == 144)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Start_Camera_176_144, NULL);
		else {
			pr_err("%s: invalid preview resolution supplied to set mode %d %d\n",
					__func__, mode->xres, mode->yres);
			return -EINVAL;
		}
		if (err < 0) {
			pr_err("%s: set mode fail %d %d %d\n",
					__func__, cur_cammod, mode->xres, mode->yres);
			return -1;
		}

	}

	return 0;
}

static int m5mo_set_mode
	(struct m5mo_info *info, struct m5mo_mode *mode)
{
	int sensor_mode = info->mode;
	int err = 0;
	FUNC_ENTR;

	if (sensor_mode != MONITOR_MODE) {
		err = m5mo_set_preview_resolution(info, mode);
		if (err < 0) {
			pr_err("%s: m5mo_set_preview_resolution() returned %d\n",
					__func__, err);
			return -EINVAL;
		}

		if (read_fw_cnt == 0) {
			err = m5mo_check_firmware_version(info);
			if (err < 0) {
				pr_err("%s: m5mo_check_firmware_version() returned %d\n",
						__func__, err);
				return -EINVAL;
			}
			read_fw_cnt = 1;
		}

		sensor_mode = MONITOR_MODE;

		err = m5mo_write_table_Isp(info, aeawb_unlock, NULL);
		if (err) {
			/* if I2C error occured when change aeawb lock state,
			 * only log the error but don't return one because it's
			 * not a serious problem.
			 */
			pr_err("%s : ae_awb unlock failed!\n", __func__);
		}

		err = m5mo_write_table_Isp(info, antibanding_60hz, NULL);
		if (err) {
			/* if I2C error occured when setting antibanding to 60Hz because It is standard in USA,
			 *  It only showes the log at that time and It doesn't call return because it's
			 * not a serious problem.
			 */
			pr_err("%s : To set antibanding to 60hz failed!\n", __func__);
		}
	} else if (mode->StillCount) {
		if (mode->xres == 3264 && mode->yres == 2448)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Capture_3264x2448, NULL);
		else if (mode->xres == 3264 && mode->yres == 1968)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Capture_3264x1968, NULL);
		else if (mode->xres == 2048 && mode->yres == 1536)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Capture_2048x1536, NULL);
		else if (mode->xres == 2048 && mode->yres == 1232)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Capture_2048x1232, NULL);
		else if (mode->xres == 1280 && mode->yres == 960)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Capture_1280x960, NULL);
		else if (mode->xres == 800 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Capture_800x480, NULL);
		else if (mode->xres == 640 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Capture_640x480, NULL);
		else {
			pr_err("%s: invalid capture resolution supplied to set mode %d %d\n",
					__func__, mode->xres, mode->yres);
			return -EINVAL;
		}

		if (err < 0) {
			pr_err("%s: set mode fail %d %d\n",
					__func__,  mode->xres, mode->yres);
			return -1;
		}

		sensor_mode = CAPTURE_MODE;

		err = m5mo_get_exif_info(info);
		if (err)
			goto setmode_err;

		err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Capture_transfer, NULL);
		if (err)
			goto setmode_err;
	} else if (sensor_mode == MONITOR_MODE) {
		err = m5mo_write_table_Isp(info, mode_isp_parameter, NULL);
		if (err)
			return err;
		err = m5mo_verify_isp_mode(info, MODE_PARAMETER_SETTING);
		if (err)
			return err;

		if (mode->xres == 1280 && mode->yres == 720)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Size_HD, NULL);
		else if (mode->xres == 800 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Size_WVGA, NULL);
		else if (mode->xres == 720 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Size_720_480, NULL);
		else if (mode->xres == 640 && mode->yres == 480)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Size_VGA, NULL);
		else if (mode->xres == 528 && mode->yres == 432)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Size_528_432, NULL);
		else if (mode->xres == 320 && mode->yres == 240)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Size_320_240, NULL);
		else if (mode->xres == 176 && mode->yres == 144)
			err = m5mo_write_table_Isp(info, SetModeSequence_ISP_Preview_Size_176_144, NULL);

		err = m5mo_write_table_Isp(info, SetModeSequence_ISP_State_Monitor, NULL);

	}

	info->mode = sensor_mode;
	return 0;

setmode_err:
	info->power_status = false;
	return err;
}

static int m5mo_set_scene_mode
	(struct m5mo_info *info, enum m5mo_scene_mode arg)
{
	pr_debug("%s : %d\n", __func__, arg);
	if (arg < SCENE_MODE_MAX && arg >= 0)
		info->scenemode = arg;
	else
		return -EINVAL;
	return m5mo_write_table_Isp(info, scene_table[arg], NULL);
}
static int m5mo_set_focus_mode
	(struct m5mo_info *info, enum m5mo_focus_mode arg)
{
	int ret;
	pr_err("%s : %d, %d\n", __func__, arg, info->focusmode);

	if (info->focusmode == arg)
		return -1;

	if (info->focusmode == FOCUS_FACE_DETECT && arg != FOCUS_FACE_DETECT) {
		ret = m5mo_write_table_Isp(info, mode_focus_face_detect_disable, NULL);
		if (ret < 0) {
			pr_err("%s fail, line is%d\n",
					__func__, __LINE__);
			return -1;
		}
	}

	info->focusmode = arg;

	switch (arg) {
	case FOCUS_AUTO:
		ret = m5mo_write_table_Isp(info, mode_focus_auto, NULL);
		break;

	case FOCUS_MACRO:
		ret = m5mo_write_table_Isp(info, mode_focus_macro, NULL);
		break;

	case FOCUS_FIXED:
		/*ret = m5mo_write_table_Isp(info, mode_focus_fixed, NULL);
		It is temporary code because ISP can't support pan
		focus mode by mistake*/
		ret = m5mo_write_table_Isp(info
				, mode_focus_auto, NULL);
		break;

	case FOCUS_FACE_DETECT:
		ret = m5mo_write_table_Isp(info
				, mode_focus_face_detect, NULL);
		break;

	case FOCUS_CONTINUOUS_VIDEO:
		ret = m5mo_write_table_Isp(info
				, mode_focus_continuous_video, NULL);
		break;

	default:
		pr_err("%s: Invalid Focus Mode%d\n", __func__, arg);
		return -EINVAL;
		break;
	}

	return ret;
}

static int m5mo_set_color_effect
	(struct m5mo_info *info, enum m5mo_color_effect arg)
{
	int ret;
	pr_debug("%s : %d\n", __func__, arg);

	if (arg > EFFECT_MODE_MAX || arg < 0)
		return -EINVAL;

	if (arg == EFFECT_NEGATIVE || arg == EFFECT_SOLARIZE) {
		ret = m5mo_write_table_Isp(info, mode_coloreffect_off, NULL);
		if (ret)
			return ret;
		ret = m5mo_write_table_Isp(info, mode_isp_parameter, NULL);
		if (ret)
			return ret;
		ret = m5mo_verify_isp_mode(info, MODE_PARAMETER_SETTING);
		if (ret)
			return ret;
	} else {
		u8 geffect[2];
		ret = m5mo_write_table_Isp(info
				, mode_read_gammaeffect, geffect);
		if (ret)
			return ret;
		if (geffect[1]) {
			ret = m5mo_write_table_Isp(info
					, mode_isp_parameter, NULL);
			if (ret)
				return ret;
			ret = m5mo_verify_isp_mode(info
					, MODE_PARAMETER_SETTING);
			if (ret)
				return ret;
			ret = m5mo_write_table_Isp(info
					, mode_gammaeffect_off, NULL);
			if (ret)
				return ret;
			ret = m5mo_write_table_Isp(info
					, mode_isp_monitor, NULL);
			if (ret)
				return ret;
		}
	}

	switch (arg) {
	case EFFECT_NONE:
		ret = m5mo_write_table_Isp(info
				, mode_coloreffect_off, NULL);
		if (ret)
			return ret;
		break;
	case EFFECT_MONO:
		ret = m5mo_write_table_Isp(info
				, mode_coloreffect_mono, NULL);
		if (ret)
			return ret;
		break;
	case EFFECT_SEPIA:
		ret = m5mo_write_table_Isp(info
				, mode_coloreffect_sepia, NULL);
		if (ret)
			return ret;
		break;
	case EFFECT_POSTERIZE:
		ret = m5mo_write_table_Isp(info
				, mode_coloreffect_posterize, NULL);
		if (ret)
			return ret;
		break;
	case EFFECT_NEGATIVE:
		ret = m5mo_write_table_Isp(info
				, mode_coloreffect_negative, NULL);
		if (ret)
			return ret;
		break;
	case EFFECT_SOLARIZE:
		ret = m5mo_write_table_Isp(info
				, mode_coloreffect_solarize, NULL);
		if (ret)
			return ret;
		break;
	default:
		/* can't happen due to checks above but to quiet compiler
		 * warning
		 */
		break;
	}

	if (arg == EFFECT_NEGATIVE || arg == EFFECT_SOLARIZE) {
		ret = m5mo_write_table_Isp(info, mode_isp_monitor, NULL);
		if (ret) {
			pr_err("%s fail, line is%d\n",
					__func__, __LINE__);
			return ret;
	}
	}
	return 0;
}
static int m5mo_set_white_balance
	(struct m5mo_info *info, enum m5mo_white_balance arg)
{
	pr_debug("%s : %d\n", __func__, arg);
	if (info->scenemode == SCENE_SUNSET ||
			info->scenemode == SCENE_CANDLE_LIGHT)
		return 0;
	if (arg < WB_MODE_MAX && arg >= 0)
		return m5mo_write_table_Isp(info
				, wb_table[arg], NULL);
	else
		return -EINVAL;
}
static int m5mo_set_flash_mode
	(struct m5mo_info *info, enum m5mo_flash_mode arg)
{
	pr_debug("%s : %d\n", __func__, arg);
	if (arg < FLASH_MODE_MAX && arg >= 0)
		return  m5mo_write_table_Isp(info
				, flash_table[arg], NULL);
	else
		return -EINVAL;
}
static int m5mo_set_exposure
	(struct m5mo_info *info, enum m5mo_exposure arg)
{
	pr_debug("%s : %d\n", __func__, arg);
	if (arg < EXPOSURE_MODE_MAX && arg >= 0)
		return m5mo_write_table_Isp(info
				, exposure_table[arg], NULL);
	else
		return -EINVAL;
}
static int m5mo_set_exposure_meter
	(struct m5mo_info *info, enum m5mo_exposure_meter arg)
{
	pr_debug("%s : %d\n", __func__, arg);
	if (arg < EXPOSURE_METER_MAX && arg >= 0)
		return m5mo_write_table_Isp(info
				, exposure_meter_table[arg], NULL);
	else
		return -EINVAL;
}
static int m5mo_set_iso
	(struct m5mo_info *info, enum m5mo_iso arg)
{
	pr_debug("%s : %d\n", __func__, arg);
	if (arg < ISO_MAX && arg >= 0)
		return m5mo_write_table_Isp(info
				, iso_table[arg], NULL);
	else
		return -EINVAL;
}
static int m5mo_set_antishake
	(struct m5mo_info *info, enum m5mo_antishake arg)
{
	pr_debug("%s : %d\n", __func__, arg);
	if (arg < ANTISHAKE_MAX && arg >= 0)
		return m5mo_write_table_Isp(info
				, antishake_table[arg], NULL);
	else
		return -EINVAL;
}
static int m5mo_set_autocontrast
	(struct m5mo_info *info, enum m5mo_autocontrast arg)
{
	pr_debug("%s : %d\n", __func__, arg);
	if (arg < AUTOCONTRAST_MAX && arg >= 0)
		return m5mo_write_table_Isp(info
				, autocontrast_table[arg], NULL);
	else
		return -EINVAL;
}

static int m5mo_set_autofocus
	(struct m5mo_info *info, enum m5mo_autofocus_control arg)
{
	int err;
	pr_debug("%s : %d\n", __func__, arg);
	switch (arg) {
	case AF_START:
		if (info->touchaf_enable) {
			err = m5mo_write_table_Isp(info, mode_touchaf_enable, NULL);
			if (err) {
				pr_err("mode_touchaf_enable failed!\n");
				return err;
			}
		}
		if (info->is_samsung_camera == M5MO_SAMSUNG_CAMERA_ON) {
			err = m5mo_write_table_Isp(info, aeawb_unlock, NULL);
			if (err) {
				pr_err("ae_awb unlock failed!\n");
				return err;
			}
			mdelay(10);

			err = m5mo_write_table_Isp(info, aeawb_lock, NULL);
			if (err) {
				pr_err("ae_awb lock failed!\n");
				return err;
			}
		}

		return m5mo_write_table_Isp(info, mode_af_start, NULL);
	case AF_STOP:
		{
			u8 sysmode = 0;
			err = m5mo_write_table_Isp(info, mode_af_stop, NULL);
			if (err) {
				pr_err("%s : AF Stop Failed!\n", __func__);
				return err;
			}
			err = m5mo_set_focus_mode(info, info->focusmode);
			if (err == -EINVAL)
				return err;
			err = m5mo_write_table_Isp(info, mode_isp_moderead,
							&sysmode);
			if (err) {
				pr_err("%s : read system mode failed!\n", __func__);
				return err;
			}
			if (sysmode == 0x02) {/* if ISP state is monitor */
				err = m5mo_write_table_Isp(info, aeawb_unlock, NULL);
				if (err) {
					pr_err("ae_awb unlock failed!\n");
					return err;
				}

				if (info->focusmode == FOCUS_AUTO) {
					err = m5mo_write_table_Isp(info, mode_af_set_normal_default, NULL);
					if (err) {
						pr_err("mode_af_set_normal_default set failed!\n");
						return err;
					}
				}
				if (info->focusmode == FOCUS_MACRO) {
					err = m5mo_write_table_Isp(info, mode_af_set_macro_default, NULL);
					if (err) {
						pr_err("mode_af_set_normal_default set failed!\n");
						return err;
					}
				}
				if (info->touchaf_enable)
					info->touchaf_enable = 0;
			}
		}
		break;
	case CAF_START:
		return m5mo_write_table_Isp(info, mode_caf_start, NULL);
	case CAF_STOP:
		return m5mo_write_table_Isp(info, mode_af_stop, NULL);
	}
	return 0;
}

static int m5mo_set_lens_soft_landing(struct m5mo_info *info)
{
	pr_debug("%s\n", __func__);
	return m5mo_write_table_Isp(info, mode_lens_soft_landing, NULL);
}

static int m5mo_get_af_result
	(struct m5mo_info *info, u8 *status)
{
	int err;

	FUNC_ENTR;

	err = m5mo_write_table_Isp(info, mode_af_result, status);
	if (err) {
		pr_err("mode_af_result failed!\n");
		goto FAIL;
	}
	pr_info("af status = 0x%x", status[1]);

	if (info->touchaf_enable) {
		if (info->is_samsung_camera == M5MO_SAMSUNG_CAMERA_ON) {
			if ((status[1] == 0x00) || (status[1] == 0x02))
				err = m5mo_write_table_Isp(info, aeawb_unlock, NULL);
			if (err) {
				pr_err("ae_awb unlock failed!\n");
				goto FAIL;
			}
		}
	}
FAIL:
	return err;
}

static int m5mo_esd_camera_reset
	(struct m5mo_info *info, enum m5mo_esd_reset arg)
{
	FUNC_ENTR;
	if (arg == ESD_DETECTED) {
		info->pdata->power_off();
		info->mode = SYSTEM_INITIALIZE_MODE;
		msleep(200);
		info->pdata->power_on();
	}
	return 0;
}

#ifdef FACTORY_TEST
static int m5mo_return_normal_preview(struct m5mo_info *info)
{
	int status = 0;

	status = m5mo_write_table_Isp(info, SetModeSequence_ISP_Return_Normal_Preview
					, NULL);

	return status;
}
#endif

static int m5mo_set_face_beauty(struct m5mo_info *info, enum m5mo_recording_frame arg)
{
	int err;

	FUNC_ENTR;

	switch (arg) {
	case M5MO_FACE_BEAUTY_OFF:
		pr_err("%s: Invalid face beauty Value, %d\n", __func__, arg);
		err = m5mo_write_table_Isp(info, mode_face_beauty_off, NULL);
		break;
	case M5MO_FACE_BEAUTY_ON:
		pr_err("%s: M5MO_FACE_BEAUTY_ON face beauty Value, %d\n"
				, __func__, arg);
		err = m5mo_write_table_Isp(info, mode_face_beauty_on, NULL);
		break;
	default:
		pr_err("%s: Invalid face beauty Value, %d\n", __func__, arg);
		return 0;
		break;
	}

	if (err < 0)
		pr_err("%s: m5mo_write_table() returned error, %d, %d\n",
			__func__, arg, err);

	return err;
}

static int m5mo_set_recording_frame(struct m5mo_info *info
			, enum m5mo_recording_frame arg)
{
	int err = 0;

	pr_debug("test recording frame!!!\n");

	switch (arg) {
	case RECORDING_CAF:
		pr_debug("test recording frame - CAF!!!\n");
		err = m5mo_write_table_Isp(info, mode_recording_caf, NULL);
		break;
	case RECORDING_PREVIEW:
		pr_debug("test recording frame - PREVIEW!!!\n");
		err = m5mo_write_table_Isp(info, mode_recording_preview,
				NULL);
		break;
	default:
		pr_err("%s: Invalid recording frame Value, %d\n",
		__func__, arg);
		break;
	}

	if (err < 0)
		pr_err("%s: m5mo_write_table() returned error, %d, %d\n",
			__func__, arg, err);

	return err;
}

static int m5mo_set_touchaf(struct m5mo_info *info,
		struct m5mo_touchaf_pos *tpos)
{
	int err = 0;

	pr_debug("Touch AF!!n");

	info->touchaf_enable = 1;

	err = m5mo_write_table_Isp(info, mode_touchaf_pos, tpos);

	pr_debug("x pos : %u, ypos : %u\n", tpos->xpos, tpos->ypos);

	return err;
}

#if 1
static int m5mo_check_camcorder_mode(unsigned int cmd)
{
	if (cur_cammod) {
		/*pr_err(KERN_INFO "m5mo_check_camcorder_mode :
		  cmd = %d\n", cmd);*/
		if ((cmd == M5MO_IOCTL_SCENE_MODE) || \
			(cmd == M5MO_IOCTL_EXPOSURE_METER) ||\
			(cmd == M5MO_IOCTL_ANTISHAKE) ||\
			(cmd == M5MO_IOCTL_AUTOCONTRAST) ||\
			(cmd == M5MO_IOCTL_SCENE_MODE)\
		) {
			/*pr_err(KERN_INFO "[m5mo_check_camcorder_mode]
			  camcorder mode dosen't need to set %d cmd\n", cmd);*/
			return M5MO_CAN_NOT_SUPPORT_PARAM;
		} else {
			/*pr_err(KERN_INFO "[m5mo_check_camcorder_mode]
			  camcorder mode !!\n");*/
			return M5MO_CAN_SUPPORT_PARAM;
		}
	} else {
		/*pr_err(KERN_INFO "[m5mo_check_camcorder_mode]
		  camera mode !!\n");*/
		return M5MO_CAN_SUPPORT_PARAM;
	}
}
#endif

/*
static int m5mo_ioctl(struct inode *inode, struct file *file,
unsigned int cmd, unsigned long arg)
*/
static long m5mo_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
	struct m5mo_info *info = file->private_data;

	/*pr_debug(KERN_INFO "\nm5mo_ioctl : cmd = %d\n", cmd);*/

#if 1
	if (m5mo_check_camcorder_mode(cmd) == M5MO_CAN_NOT_SUPPORT_PARAM)
		return 0;
#endif

	if (enable_scene) {
		if (cmd == M5MO_IOCTL_WHITE_BALANCE ||
				cmd == M5MO_IOCTL_ANTISHAKE) {
			/*pr_info("Now Scene mode is setted!!\n");*/
			return 0;
		}
	}

	if (dtpTest == M5MO_DTP_TEST_ON) {
		if ((cmd != M5MO_IOCTL_SET_MODE) &&
			(cmd != M5MO_IOCTL_DTP_TEST)) {
			pr_err("func(%s):line(%d)s5k6aafx_DTP_TEST_ON. cmd(%d)\n"
					, __func__, __LINE__, cmd);
			return 0;
		}
	}

	switch (cmd) {
	case M5MO_IOCTL_SET_MODE:
	{
		struct m5mo_mode mode;
		if (copy_from_user(&mode, (const void __user *)arg,
			sizeof(struct m5mo_mode))) {
			pr_info("%s %d\n", __func__, __LINE__);
			return -EFAULT;
		}
		cur_cammod = mode.camcordmode;
		return m5mo_set_mode(info, &mode);
	}
	case M5MO_IOCTL_SCENE_MODE:
		if (arg == 0)
			enable_scene = 0;
		else
			enable_scene = 1;

		return m5mo_set_scene_mode(info, (enum m5mo_scene_mode) arg);
	case M5MO_IOCTL_FOCUS_MODE:
		return m5mo_set_focus_mode(info, (enum m5mo_focus_mode) arg);
	case M5MO_IOCTL_COLOR_EFFECT:
		return m5mo_set_color_effect(info
				, (enum m5mo_color_effect) arg);
	case M5MO_IOCTL_WHITE_BALANCE:
		return m5mo_set_white_balance(info
				, (enum m5mo_white_balance) arg);
	case M5MO_IOCTL_FLASH_MODE:
		return m5mo_set_flash_mode(info, (enum m5mo_flash_mode) arg);
	case M5MO_IOCTL_EXPOSURE:
		return m5mo_set_exposure(info, (enum m5mo_exposure) arg);
	case M5MO_IOCTL_EXPOSURE_METER:
		return m5mo_set_exposure_meter(info
				, (enum m5mo_exposure_meter) arg);
	case M5MO_IOCTL_ISO:
		return m5mo_set_iso(info, (enum m5mo_iso) arg);
	case M5MO_IOCTL_ANTISHAKE:
		return m5mo_set_antishake(info, (enum m5mo_antishake) arg);
	case M5MO_IOCTL_AUTOCONTRAST:
		return m5mo_set_autocontrast(info
				, (enum m5mo_autocontrast) arg);
	case M5MO_IOCTL_AF_CONTROL:
		return m5mo_set_autofocus(info
				, (enum m5mo_autofocus_control) arg);
	case M5MO_IOCTL_LENS_SOFT_LANDING:
		return m5mo_set_lens_soft_landing(info);
	case M5MO_IOCTL_AF_RESULT:
	{
		int err = 0;
		u8 status[2] = {0};
		err = m5mo_get_af_result(info, status);
		if (err)
			return err;
		if (copy_to_user((void __user *)arg, &status[1], 1)) {
			pr_info("%s %d\n", __func__, __LINE__);
			return -EFAULT;
		}
		return 0;
	}
	case M5MO_IOCTL_ESD_RESET:
		if (dtpTest == M5MO_DTP_TEST_OFF)
			m5mo_esd_camera_reset(info, (enum m5mo_esd_reset) arg);
		break;
	case M5MO_IOCTL_EXIF_INFO:
		if (copy_to_user((void __user *)arg, &info->exif_info,
					sizeof(info->exif_info)))
			return -EFAULT;
		break;
#ifdef FACTORY_TEST
	case M5MO_IOCTL_DTP_TEST:
	{
		int status = 0;
		pr_debug("\n : M5MO_IOCTL_DTP_TEST arg = %lu\n", arg);
		if (dtpTest == 1 && (enum m5mo_dtp_test) arg == 0)
			status = m5mo_return_normal_preview(info);
		dtpTest = (enum m5mo_dtp_test) arg;
		return status;
	}
#endif
	case M5MO_IOCTL_FACE_BEAUTY:
		return m5mo_set_face_beauty(info, (enum m5mo_face_beauty) arg);

	case M5MO_IOCTL_RECORDING_FRAME:
		return m5mo_set_recording_frame(info
				, (enum m5mo_recording_frame) arg);

	case M5MO_IOCTL_TOUCHAF:
		{
			struct m5mo_touchaf_pos tpos;
			if (copy_from_user(&tpos, (const void __user *)arg,
				sizeof(struct m5mo_touchaf_pos))) {
				pr_info("%s %d\n", __func__, __LINE__);
				return -EFAULT;
			}

			return m5mo_set_touchaf(info, &tpos);
		}

	case M5MO_IOCTL_FW_VERSION:
		if (copy_to_user((void __user *)arg, &info->fw_ver,
					sizeof(info->fw_ver)))
			return -EFAULT;
		break;

	case M5MO_IOCTL_SAMSUNG_CAMERA:
		pr_warning("SAMSUNG_CAMERA is %s\n"
			, (enum m5mo_samsung_camera) arg ?
			"enabled" : "disabled");
		info->is_samsung_camera = (enum m5mo_samsung_camera) arg;
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static int firmware_read(void)
{
	int err;
	u8 pdata[8];
	u8 *readbuffer;
	unsigned int address = FIRMWARE_ADDRESS_START;
	struct file *filp;
	mm_segment_t oldfs;
	struct i2c_client *client = info->i2c_client_isp;

	readbuffer = vmalloc(M5MO_FIRMEWARE_READ_MAX_BUFFER_SIZE);
	if (!readbuffer)
		return -ENOMEM;

	filp = filp_open(CAMERA_FW_DUMP_FILE_PATH, O_RDWR | O_CREAT, 0666);
	if (IS_ERR_OR_NULL(filp)) {
		pr_err("firmware_read: File open error\n");
		vfree(readbuffer);
		return -1;
	}

	pdata[0] = 0x00;
	pdata[1] = 0x03;

	filp->f_pos = 0;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	while (address < FIRMWARE_ADDRESS_END) {
		pdata[2] = (address & 0xFF000000) >> 24;
		pdata[3] = (address & 0x00FF0000) >> 16;
		pdata[4] = (address & 0x0000FF00) >> 8;
		pdata[5] = (address & 0x000000FF);

		if (address + I2C_WRITE_SIZE <= FIRMWARE_ADDRESS_END) {
			pdata[6] = 0x08;
			pdata[7] = 0x00;
			err = m5mo_write_i2c(client, pdata, 0, 8);
			if (err != 1)
				goto firmwarereaderr;
			err = m5mo_write_i2c(client, readbuffer,
					1, 3 + I2C_WRITE_SIZE);
			if (err != 1)
				goto firmwarereaderr;
			filp->f_op->write(filp, &readbuffer[3],
					I2C_WRITE_SIZE, &filp->f_pos);
		} else {
			pdata[6] = ((FIRMWARE_ADDRESS_END - address + 1) &
				0xFF00) >> 8;
			pdata[7] = ((FIRMWARE_ADDRESS_END - address + 1) &
				0x00FF);
			err = m5mo_write_i2c(client, pdata, 0, 8);
			if (err != 1)
				goto firmwarereaderr;
			err = m5mo_write_i2c(client, readbuffer, 1,
					FIRMWARE_ADDRESS_END - address + 4);
			if (err != 1)
				goto firmwarereaderr;
			err = filp->f_op->write(filp, &readbuffer[3],
				FIRMWARE_ADDRESS_END - address + 1,
				&filp->f_pos);
			if (err < 0)
				goto firmwarereaderr;
		}
		address += I2C_WRITE_SIZE;
	}
	fput(filp);
	filp_close(filp, NULL);

	pr_info("dump end\n");

	set_fs(oldfs);
	vfree(readbuffer);
	return 0;
firmwarereaderr:
	set_fs(oldfs);
	filp_close(filp, NULL);
	vfree(readbuffer);
	return err;
}

static int m5mo_get_FW_info(char *fw_info)
{
	int err, i, w_count = 0;
	int m_pos = 0;
	char t_buf[35] = {0};
	struct file *filp;
	mm_segment_t oldfs;

	/*CAM FW*/
	for (i = 0; i < 30; i++) {
		err = m5mo_write_table_Isp(info, mode_fwver_read, t_buf);
		if (err < 0) {
			pr_err("%s fail, line is%d\n",
					__func__, __LINE__);
		}
		if (t_buf[1] == 0x00)
			break;
		fw_info[m_pos] = t_buf[1];
		m_pos++;
	}

	/* Make blank*/
	fw_info[m_pos++] = ' ';

	/* PHONE FW */
#if 0
	filp = filp_open(CAMERA_FW_FILE_PATH, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(filp)) {
		pr_err("Error with open MISC(filp)\n");
		return -1;
	}
#endif

	filp = filp_open(CAMERA_FW_FILE_EXTERNAL_PATH, O_RDONLY, 0);

	if (IS_ERR_OR_NULL(filp)) {
		filp = filp_open(CAMERA_FW_FILE_PATH, O_RDONLY, 0);

		if (IS_ERR_OR_NULL(filp))
			return -1;
	}

	filp->f_pos = START_POSITION_OF_VERSION_STRING;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	filp->f_op->read(filp, t_buf, LENGTH_OF_VERSION_STRING, &filp->f_pos);
	set_fs(oldfs);

	fput(filp);
	filp_close(filp, NULL);

	for (i = 0; i < LENGTH_OF_VERSION_STRING; i++) {
		fw_info[m_pos] = t_buf[i];
		m_pos++;
	}

	/* Make blank*/
	fw_info[m_pos++] = ' ';

	w_count = 0; /*always 0*/
	sprintf(fw_info + m_pos, "%d", w_count);
	do {
		m_pos++;
	} while (fw_info[m_pos] != 0);

	/* Make blank*/
	fw_info[m_pos++] = ' ';

	/* AF CAL */
	err = m5mo_write_table_Isp(info, mode_afcal_read, t_buf);
	if (err < 0) {
		pr_err("%s fail, line is%d\n",
				__func__, __LINE__);
	}
	if (t_buf[1] == 0xA0 || t_buf[1] == 0xA1) {
		sprintf(fw_info + m_pos, "%X", t_buf[1]);
		do {
			m_pos++;
		} while (fw_info[m_pos] != 0);

		/* Make blank*/
		fw_info[m_pos++] = ' ';

		sprintf(fw_info + m_pos, "%X", t_buf[1]);
		do {
			m_pos++;
		} while (fw_info[m_pos] != 0);
	} else {
		fw_info[m_pos++] = 'F';
		fw_info[m_pos++] = 'F';

		/* Make blank*/
		fw_info[m_pos++] = ' ';

		fw_info[m_pos++] = 'F';
		fw_info[m_pos++] = 'F';
	}

	/* Make blank*/
	fw_info[m_pos++] = ' ';

	/* AWB CAL RG High*/
	err = m5mo_write_table_Isp(info, mode_awbcal_RGread_H, t_buf);
	if (err < 0) {
		pr_err("%s fail, line is%d\n",
				__func__, __LINE__);
	}
	sprintf(fw_info + m_pos, "%X", t_buf[1]);
	do {
		m_pos++;
	} while (fw_info[m_pos] != 0);

	/* Make blank*/
	fw_info[m_pos++] = ' ';

	/* AWB CAL RG Low*/
	err = m5mo_write_table_Isp(info, mode_awbcal_RGread_L, t_buf);
	if (err < 0) {
		pr_err("%s fail, line is%d\n",
				__func__, __LINE__);
	}
	sprintf(fw_info + m_pos, "%X", t_buf[1]);
	do {
		m_pos++;
	} while (fw_info[m_pos] != 0);

	/* Make blank*/
	fw_info[m_pos++] = ' ';

	/* AWB CAL GB*/
	err = m5mo_write_table_Isp(info, mode_awbcal_GBread_H, t_buf);
	if (err < 0) {
		pr_err("%s fail, line is%d\n",
				__func__, __LINE__);
	}
	sprintf(fw_info + m_pos, "%X", t_buf[1]);
	do {
		m_pos++;
	} while (fw_info[m_pos] != 0);

	/* Make blank*/
	fw_info[m_pos++] = ' ';

	/* AWB CAL GB*/
	err = m5mo_write_table_Isp(info, mode_awbcal_GBread_L, t_buf);
	if (err < 0) {
		pr_err("%s fail, line is%d\n",
				__func__, __LINE__);
	}
	sprintf(fw_info + m_pos, "%X", t_buf[1]);
	do {
		m_pos++;
	} while (fw_info[m_pos] != 0);

	/* Make blank*/
	fw_info[m_pos++] = ' ';

	/* Make end */
	fw_info[m_pos] = '\0';

	pr_err("\nfirmware information: %s\n", fw_info);

	return err;
}

static ssize_t rear_camfw_file_cmd_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int status;
	char fw_info[100] = {0};

	pr_info("called %s\n", __func__);
	fp = filp_open("/dev/tegra_camera", O_RDONLY, 0);
	info_clk.b.id = TEGRA_CAMERA_MODULE_VI;
	info_clk.b.clk_id = TEGRA_CAMERA_VI_SENSOR_CLK;
	info_clk.b.rate = 24000000;

	fp->f_op->unlocked_ioctl(fp, TEGRA_CAMERA_IOCTL_SENSOR_FW_FOR_SAMSUNG, info_clk.a);
	info->pdata->power_on();

	status = m5mo_write_table_Isp(info, mode_isp_start, NULL);
	if (status != 0)
		return status;
	status = m5mo_get_FW_info(fw_info);
	if (status != 0)
		return status;
	info->pdata->power_off();
	filp_close(fp, current->files);
	return sprintf(buf, "%s\n", fw_info);
}

static ssize_t rear_camfw_file_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	int status;

	pr_info("called %s\n", __func__);
	sscanf(buf, "%d", &value);
	fp = filp_open("/dev/tegra_camera", O_RDONLY, 0);

	info_clk.b.id = TEGRA_CAMERA_MODULE_VI;
	info_clk.b.clk_id = TEGRA_CAMERA_VI_SENSOR_CLK;
	info_clk.b.rate = 24000000;

	fp->f_op->unlocked_ioctl(fp, TEGRA_CAMERA_IOCTL_SENSOR_FW_FOR_SAMSUNG, info_clk.a);
	info->pdata->power_on();

	if (value == FWUPDATE) {
		pr_err("[fwupdate set]m5mo_firmware_write start\n");
		status = m5mo_firmware_write(info);
		if (status != 0)
			return -1;
		pr_err("[fwupdate set]m5mo_firmware_write end\n");

	} else if (value == FWDUMP) {
		status = firmware_read();
		if (status != 0)
			return -1;
		pr_info("FWDUMP is done!!\n");
	}

	info->pdata->power_off();
	filp_close(fp, current->files);
	return size;
}
static DEVICE_ATTR(rear_camfw, 0660, rear_camfw_file_cmd_show
		, rear_camfw_file_cmd_store);

#ifdef FACTORY_TEST
static ssize_t rear_flash_file_cmd_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	/*Reserved*/
	return 0;
}

static ssize_t rear_flash_file_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	int err = 0;

	sscanf(buf, "%d", &value);

	if (value == FACTORY_FLASH_OFF) {
		printk(KERN_INFO "[Factory flash]OFF\n");
		err = m5mo_write_table_Isp(info, mode_flash_off, NULL);
		if (err < 0) {
			pr_err("%s fail, line is%d\n",
					__func__, __LINE__);
		}
		info->pdata->power_off();
		filp_close(fp, current->files);
	} else {
		fp = filp_open("/dev/tegra_camera", O_RDONLY, 0);
		info_clk.b.id = TEGRA_CAMERA_MODULE_VI;
		info_clk.b.clk_id = TEGRA_CAMERA_VI_SENSOR_CLK;
		info_clk.b.rate = 24000000;
		fp->f_op->unlocked_ioctl(fp, TEGRA_CAMERA_IOCTL_SENSOR_FW_FOR_SAMSUNG, info_clk.a);
		info->pdata->power_on();
		printk(KERN_INFO "[Factory flash]ON\n");
		err = m5mo_write_table_Isp(info, mode_isp_start, NULL);
		if (err < 0) {
			pr_err("%s fail, line is%d\n",
					__func__, __LINE__);
		}
		err = m5mo_write_table_Isp(info, mode_flash_torch, NULL);
		if (err < 0) {
			pr_err("%s fail, line is%d\n",
					__func__, __LINE__);
		}
	}
	return size;
}

static DEVICE_ATTR(rear_flash, 0660, rear_flash_file_cmd_show
		, rear_flash_file_cmd_store);

static ssize_t rear_camtype_file_cmd_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char camType[] = "SONY_IMX105_M5MO";

	return sprintf(buf, "%s", camType);
}

static ssize_t rear_camtype_file_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(rear_camtype, 0660, rear_camtype_file_cmd_show
		, rear_camtype_file_cmd_store);
#endif

static int m5mo_open(struct inode *inode, struct file *file)
{
	int status = 0;
	u8 sysmode = 0;

	FUNC_ENTR;

	file->private_data = info;

	info->scenemode = SCENE_AUTO;
	info->power_status = false;
	info->focusmode = FOCUS_INVALID;
	info->touchaf_enable = false;
	info->is_samsung_camera = M5MO_SAMSUNG_CAMERA_OFF;

	if (info->pdata && info->pdata->power_on)
		info->pdata->power_on();
/*
	status = m5mo_write_table_Isp(info, mode_isp_moderead, &sysmode);
	if (status < 0) {
		info->pdata->power_off();
		info->power_status = false;
	}
*/
	info->mode = SYSTEM_INITIALIZE_MODE;
	dtpTest = M5MO_DTP_TEST_OFF;
	read_fw_cnt = 0;
	cur_cammod = 0;
	enable_scene = 0;

	return status;
}

int m5mo_release(struct inode *inode, struct file *file)
{
	FUNC_ENTR;

	if (info->pdata && info->pdata->power_off) {
		info->pdata->power_off();
		info->power_status = false;
		info->is_samsung_camera = M5MO_SAMSUNG_CAMERA_OFF;
	}
	file->private_data = NULL;
	read_fw_cnt = 0;

	return 0;
}

static const struct file_operations m5mo_fileops = {
	.owner = THIS_MODULE,
	.open = m5mo_open,
	.unlocked_ioctl = m5mo_ioctl,
	.compat_ioctl = m5mo_ioctl,
	.release = m5mo_release,
};

static struct miscdevice m5mo_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "m5mo",
	.fops = &m5mo_fileops,
};

static int m5mo_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err;
	int dev;
	char m5mo_name[20] = "m5mo";
	char m5mo_pmic_name[20] = "m5mo_pmic";
	FUNC_ENTR;

	pr_debug("%s , %x probing i2c(%lu)", id->name, client->addr
			, id->driver_data);

	if (strcmp(m5mo_name, id->name) == 0)
		dev = 0;
	if (strcmp(m5mo_pmic_name, id->name) == 0)
		dev = 1;

	switch (dev) {
	case 0:
		info = kzalloc(sizeof(*info), GFP_KERNEL);
		if (!info) {
			pr_err("m5mo: Unable to allocate memory!\n");
			return -ENOMEM;
		}
		info->i2c_client_isp = client;
		if (!info->i2c_client_isp) {
			pr_err("m5mo: Unknown I2C client!\n");
			err = -ENODEV;
			goto probeerr;
		}
		info->pdata = client->dev.platform_data;
		if (!info->pdata) {
			pr_err("m5mo: Unknown platform data!\n");
			err = -ENODEV;
			goto probeerr;
		}
		err = misc_register(&m5mo_device);
		if (err) {
			pr_err("m5mo: Unable to register misc device!\n");
			goto probeerr;
		}

		m5mo_dev = device_create(camera_class, NULL, 0, NULL
					, "rear");
		if (IS_ERR(m5mo_dev)) {
			pr_err("Failed to create device!");
			goto probeerr;
		}
		if (device_create_file(m5mo_dev, &dev_attr_rear_camfw)
				< 0) {
			pr_err("Failed to create device file!(%s)!\n"
					, dev_attr_rear_camfw.attr.name);
			goto probeerr;
		}
#ifdef FACTORY_TEST
		if (device_create_file(m5mo_dev, &dev_attr_rear_flash)
				< 0) {
			pr_debug("Failed to create device file!(%s)!\n"
				, dev_attr_rear_flash.attr.name);
			goto probeerr;
		}

		if (device_create_file(m5mo_dev, &dev_attr_rear_camtype)
				< 0) {
			pr_debug("Failed to create device file!(%s)!\n"
					, dev_attr_rear_camtype.attr.name);
			goto probeerr;
		}
#endif
		reg_mipi_1v2 = regulator_get(NULL, "VAP_MIPI_1V2");

		if (IS_ERR(reg_mipi_1v2)) {
			pr_err("%s: VAP_MIPI_1V2 regulator not found\n"
					, __func__);
			reg_mipi_1v2 = NULL;
		}

		break;

	case 1:
		i2c_client_pmic = client;
		break;
	}
	i2c_set_clientdata(client, info);

	return 0;
probeerr:
	kfree(info);
	return err;
}

static int m5mo_remove(struct i2c_client *client)
{
	struct m5mo_info *info;
	FUNC_ENTR;
	info = i2c_get_clientdata(client);
	misc_deregister(&m5mo_device);
	kfree(info);
	device_remove_file(m5mo_dev, &dev_attr_rear_camfw);
	return 0;
}

/*i2c_m5mo*/
static const struct i2c_device_id m5mo_id[] = {
	{ "m5mo", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, m5mo_id);

static struct i2c_driver m5mo_i2c_driver = {
	.driver = {
		.name = "m5mo",
		.owner = THIS_MODULE,
	},
	.probe = m5mo_probe,
	.remove = m5mo_remove,
	.id_table = m5mo_id,
};

static const struct i2c_device_id m5mo_pmic_id[] = {
	{ "m5mo_pmic", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, m5mo_pmic_id);

static struct i2c_driver m5mo_i2c_pmic_driver = {
	.driver = {
		.name = "m5mo_pmic",
		.owner = THIS_MODULE,
	},
	.probe = m5mo_probe,
	.remove = m5mo_remove,
	.id_table = m5mo_pmic_id,
};

static int __init m5mo_init(void)
{
	int status;
	FUNC_ENTR;
	pr_info("m5mo sensor driver loading\n");
	status = i2c_add_driver(&m5mo_i2c_driver);
	if (status) {
		pr_err("m5mo error\n");
		return status;
	}
	status = i2c_add_driver(&m5mo_i2c_pmic_driver);
	if (status) {
		pr_err("m5mo_pmic error\n");
		return status;
	}
	return 0;
}

static void __exit m5mo_exit(void)
{
	FUNC_ENTR;
	i2c_del_driver(&m5mo_i2c_driver);
}

module_init(m5mo_init);
module_exit(m5mo_exit);
