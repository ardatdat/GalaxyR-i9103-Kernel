/*
*  atmel_maxtouch.c - Atmel maXTouch Touchscreen Controller
*
*  Version 0.2a
*
*  An early alpha version of the maXTouch Linux driver.
*
*
*  Copyright (C) 2010 Iiro Valkonen <iiro.valkonen@atmel.com>
*  Copyright (C) 2009 Ulf Samuelsson <ulf.samuelsson@atmel.com>
*  Copyright (C) 2009 Raphael Derosso Pereira <raphaelpereira@gmail.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/earlysuspend.h>
#include <linux/firmware.h>
#include <linux/wakelock.h>

#include <linux/delay.h>
#include <linux/i2c.h>

#include <linux/atmel_mxt224E.h>
#include "atmel_mxt224E_cfg.h"
#include "device_config.h"

#define READ_FW_FROM_HEADER 1

#define ATMEL_DEBUG	0401

u8 firmware_latest[] = {
	#include "atmel_mxt224e_fw_ver10_aa.h"
	};
/*
u8 firmware_testver[] = {
	#include "atmel_mxt224e_fw_ver04_f6.h"
	};
*/
/*
static gen_powerconfig_t7_config_t          power_config = { 0 };
static gen_acquisitionconfig_t8_config_t    acquisition_config = { 0 };
static touch_multitouchscreen_t9_config_t   touchscreen_config = { 0 };
static procg_noisesuppression_t22_config_t  noise_suppression_config = { 0 };
static spt_cteconfig_t28_config_t           cte_config = { 0 };
static proci_gripsuppression_t40_config_t   gripsupression_config = { 0 };
static proci_palmsuppression_t41_config_t   palmsupression_config = { 0 };
*/


/******************************************************************************
*
*
*       QT602240 Object table init
*
*
* *****************************************************************************/
/* General Object */

static gen_powerconfig_t7_config_t			power_config = { 0 };                 /* Power config settings. */
gen_acquisitionconfig_t8_config_t			acquisition_config = { 0 };     /* Acquisition config. */
EXPORT_SYMBOL(acquisition_config);


/* Touch Object */

static touch_multitouchscreen_t9_config_t		touchscreen_config = { 0 };    /* Multitouch screen config. */
static touch_keyarray_t15_config_t			keyarray_config = { 0 };              /* Key array config. */

/* Signal Processing Objects */

static touch_proximity_t23_config_t			proximity_config = { 0 };    /* Proximity Config */
static proci_onetouchgestureprocessor_t24_config_t     onetouch_gesture_config = {0};  /* One-touch gesture config. */

/* Support Objects */
static spt_gpiopwm_t19_config_t				gpiopwm_config = { 0 };             /* GPIO/PWM config */
static spt_selftest_t25_config_t			selftest_config = { 0 };            /* Selftest config. */

static spt_comcconfig_t18_config_t			comc_config = { 0 };            /* Communication config settings. */


/* MXT224E Objects */
static proci_gripsuppression_t40_config_t		gripsuppression_t40_config = { 0 };
static proci_touchsuppression_t42_config_t		touchsuppression_t42_config = { 0 };
static spt_cteconfig_t46_config_t			cte_t46_config = { 0 };
static proci_stylus_t47_config_t			stylus_t47_config = { 0 };
static procg_noisesuppression_t48_config_t		noisesuppression_t48_config = { 0 };

static spt_userdata_t38_t				userdata_t38 = { 0 };

/*   */
/* PROCI_STYLUS_T47 */
/* PROCG_NOISESUPPRESSION_T48  */

int mxt_get_object_values(struct mxt_data *mxt, int obj_type)
{
	struct i2c_client *client = mxt->client;


	u8 *obj = NULL;
	u16 obj_size = 0;
	u16 obj_addr = 0;
	int error;


	switch (obj_type) {
	case MXT_GEN_POWERCONFIG_T7:
		obj = (u8 *)&power_config;
		break;

	case MXT_GEN_ACQUIRECONFIG_T8:
		obj = (u8 *)&acquisition_config;
		break;

	case MXT_TOUCH_MULTITOUCHSCREEN_T9:
		obj = (u8 *)&touchscreen_config;
		break;

	case MXT_PROCI_GRIPSUPPRESSION_T40:
		obj = (u8 *)&gripsuppression_t40_config;
		break;

	case MXT_PROCI_TOUCHSUPPRESSION_T42:
		obj = (u8 *)&touchsuppression_t42_config;
		break;

	case MXT_SPT_CTECONFIG_T46:
		obj = (u8 *)&cte_t46_config;
		break;

	case MXT_PROCI_STYLUS_T47:
		obj = (u8 *)&stylus_t47_config;
		break;

	case MXT_PROCG_NOISESUPPRESSION_T48:
		obj = (u8 *)&noisesuppression_t48_config;
		break;
	case MXT_USER_INFO_T38:
		obj = (u8 *)&userdata_t38;
		break;
	default:
		pr_err("[TSP] Not supporting object type (object type: %d)", obj_type);
		return -1;
	}

	obj_addr = MXT_BASE_ADDR(obj_type);
	obj_size = MXT_GET_SIZE(obj_type);


	if ((obj_addr == 0) || (obj_size == 0)) {
		pr_err("[TSP] Not supporting object type (object type: %d)", obj_type);
		return -1;
	}

	error = mxt_read_block(client, obj_addr, obj_size, obj);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}



int mxt_copy_object(struct mxt_data *mxt, u8 *buf, int obj_type)
{
	u8 *obj = NULL;
	u16 obj_size = 0;

	switch (obj_type) {
	case MXT_GEN_POWERCONFIG_T7:
		obj = (u8 *)&power_config;
		obj_size = MXT_GET_SIZE(obj_type);
		break;

	case MXT_GEN_ACQUIRECONFIG_T8:
		obj = (u8 *)&acquisition_config;
		obj_size = MXT_GET_SIZE(obj_type);
		break;

	case MXT_TOUCH_MULTITOUCHSCREEN_T9:
		obj = (u8 *)&touchscreen_config;
		obj_size = MXT_GET_SIZE(obj_type);
		break;

	case MXT_PROCI_GRIPSUPPRESSION_T40:
		obj = (u8 *)&gripsuppression_t40_config;
		obj_size = MXT_GET_SIZE(obj_type);
		break;

	case MXT_PROCI_TOUCHSUPPRESSION_T42:
		obj = (u8 *)&touchsuppression_t42_config;
		obj_size = MXT_GET_SIZE(obj_type);
		break;

	case MXT_SPT_CTECONFIG_T46:
		obj = (u8 *)&cte_t46_config;
		obj_size = MXT_GET_SIZE(obj_type);
		break;

	case MXT_PROCI_STYLUS_T47:
		obj = (u8 *)&stylus_t47_config;
		obj_size = MXT_GET_SIZE(obj_type);
		break;

	case MXT_PROCG_NOISESUPPRESSION_T48:
		obj = (u8 *)&noisesuppression_t48_config;
		obj_size = MXT_GET_SIZE(obj_type);
		break;
	case MXT_USER_INFO_T38:
		obj = (u8 *)&userdata_t38;
		obj_size = MXT_GET_SIZE(obj_type);
		break;
	default:
		pr_err("[TSP] Not supporting object type (object type: %d)", obj_type);
		return -1;
	}

	pr_info("[TSP] obj type: %d, obj size: %d", obj_type, obj_size);


	if (memcpy(buf, obj, obj_size) == NULL) {
		pr_err("[TSP] memcpy failed! (%s, %d)\n", __func__, __LINE__);
		return -1;
	}
	return 0;
}

int mxt_userdata_config(struct mxt_data *mxt)

{
        struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_USER_INFO_T38);
	obj_size = MXT_GET_SIZE(MXT_USER_INFO_T38);

	if(T38_USERDATA0 != 0) userdata_t38.data[0] = T38_USERDATA0;
	if(T38_USERDATA1 != 0) userdata_t38.data[1] = T38_USERDATA1;
	if(T38_USERDATA2 != 0) userdata_t38.data[2] = T38_USERDATA2;
	if(T38_USERDATA3 != 0) userdata_t38.data[3] = T38_USERDATA3;
	if(T38_USERDATA4 != 0) userdata_t38.data[4] = T38_USERDATA4;
	if(T38_USERDATA5 != 0) userdata_t38.data[5] = T38_USERDATA5;
	if(T38_USERDATA6 != 0) userdata_t38.data[6] = T38_USERDATA6;
	if(T38_USERDATA7 != 0) userdata_t38.data[7] = T38_USERDATA7;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&userdata_t38);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}


int mxt_power_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;

	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_GEN_POWERCONFIG_T7);
	obj_size = MXT_GET_SIZE(MXT_GEN_POWERCONFIG_T7);

	power_config.idleacqint     = T7_IDLEACQINT;     /* Set Idle Acquisition Interval to 32 ms. */
	power_config.actvacqint     = T7_ACTVACQINT;     /* Set Active Acquisition Interval to 16 ms. */            ;
	power_config.actv2idleto    = T7_ACTV2IDLETO;    /* Set Active to Idle Timeout to 4 s (one unit = 200ms). */

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&power_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}
	return 0;
}

extern bool mxt_reconfig_flag;
int mxt_acquisition_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8);
	obj_size = MXT_GET_SIZE(MXT_GEN_ACQUIRECONFIG_T8);

	acquisition_config.chrgtime     = T8_CHRGTIME;
	acquisition_config.reserved     = T8_ATCHDRIFT;
	acquisition_config.tchdrift     = T8_TCHDRIFT;
	acquisition_config.driftst      = T8_DRIFTST;
	acquisition_config.tchautocal   = T8_TCHAUTOCAL;
	acquisition_config.sync         = T8_SYNC;
	acquisition_config.atchcalst    = T8_ATCHCALST;
	acquisition_config.atchcalsthr  = T8_ATCHCALSTHR;
	acquisition_config.atchcalfrcthr = T8_ATCHFRCCALTHR;     /*!< Anti-touch force calibration threshold */
	acquisition_config.atchcalfrcratio = T8_ATCHFRCCALRATIO;  /*!< Anti-touch force calibration ratio */

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&acquisition_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}
EXPORT_SYMBOL(mxt_acquisition_config);

int mxt_multitouch_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9);
	obj_size = MXT_GET_SIZE(MXT_TOUCH_MULTITOUCHSCREEN_T9);

	touchscreen_config.ctrl         = T9_CTRL;
	touchscreen_config.xorigin      = T9_XORIGIN;
	touchscreen_config.yorigin      = T9_YORIGIN;
	touchscreen_config.xsize        = T9_XSIZE;
	touchscreen_config.ysize        = T9_YSIZE;
	touchscreen_config.akscfg       = T9_AKSCFG;
	touchscreen_config.blen         = T9_BLEN;

	touchscreen_config.tchthr       = T9_TCHTHR;

	touchscreen_config.tchdi        = T9_TCHDI;
	touchscreen_config.orient       = T9_ORIENT;
	touchscreen_config.mrgtimeout   = T9_MRGTIMEOUT;
	touchscreen_config.movhysti     = T9_MOVHYSTI;
	touchscreen_config.movhystn     = T9_MOVHYSTN;
	touchscreen_config.movfilter    = T9_MOVFILTER;
	touchscreen_config.numtouch     = T9_NUMTOUCH;
	touchscreen_config.mrghyst      = T9_MRGHYST;
	touchscreen_config.mrgthr       = T9_MRGTHR;
	touchscreen_config.amphyst      = T9_AMPHYST;
	touchscreen_config.xrange       = T9_XRANGE;
	touchscreen_config.yrange       = T9_YRANGE;
	touchscreen_config.xloclip      = T9_XLOCLIP;
	touchscreen_config.xhiclip      = T9_XHICLIP;
	touchscreen_config.yloclip      = T9_YLOCLIP;
	touchscreen_config.yhiclip      = T9_YHICLIP;
	touchscreen_config.xedgectrl    = T9_XEDGECTRL;
	touchscreen_config.xedgedist    = T9_XEDGEDIST;
	touchscreen_config.yedgectrl    = T9_YEDGECTRL;
	touchscreen_config.yedgedist    = T9_YEDGEDIST;
	touchscreen_config.jumplimit    = T9_JUMPLIMIT;

	touchscreen_config.tchhyst     = T9_TCHHYST;
	touchscreen_config.xpitch      = T9_XPITCH;
	touchscreen_config.ypitch      = T9_YPITCH;

	touchscreen_config.nexttchdi   = T9_NEXTTCHDI;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&touchscreen_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}
	return 0;
}

int mxt_keyarray_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_TOUCH_KEYARRAY_T15);
	obj_size = MXT_GET_SIZE(MXT_TOUCH_KEYARRAY_T15);

	keyarray_config.ctrl        = T15_CTRL;
	if ((mxt->pdata->board_rev <= 9) || (mxt->pdata->board_rev >= 13)) {
		keyarray_config.xorigin     = T15_XORIGIN;
		keyarray_config.xsize       = T15_XSIZE;
	} else { /* board rev1.0~1.2, touch key is 4 key array */
		keyarray_config.xorigin     = T15_XORIGIN_4KEY;
		keyarray_config.xsize       = T15_XSIZE_4KEY;
	}
	keyarray_config.yorigin     = T15_YORIGIN;
	keyarray_config.ysize       = T15_YSIZE;
	keyarray_config.akscfg      = T15_AKSCFG;
	keyarray_config.blen        = T15_BLEN;
	keyarray_config.tchthr      = T15_TCHTHR;
	keyarray_config.tchdi       = T15_TCHDI;
	keyarray_config.reserved[0] = T15_RESERVED_0;
	keyarray_config.reserved[1] = T15_RESERVED_1;


	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&keyarray_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}
	return 0;
}


int mxt_comc_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_SPT_COMMSCONFIG_T18);
	obj_size = MXT_GET_SIZE(MXT_SPT_COMMSCONFIG_T18);

	comc_config.ctrl        = T18_CTRL;
	comc_config.cmd         = T18_COMMAND;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&comc_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}

int mxt_gpio_pwm_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_SPT_GPIOPWM_T19);
	obj_size = MXT_GET_SIZE(MXT_SPT_GPIOPWM_T19);

	gpiopwm_config.ctrl         = T19_CTRL      ;
	gpiopwm_config.reportmask   = T19_REPORTMASK;
	gpiopwm_config.dir          = T19_DIR       ;
	gpiopwm_config.intpullup    = T19_INTPULLUP ;
	gpiopwm_config.out          = T19_OUT       ;
	gpiopwm_config.wake         = T19_WAKE      ;
	gpiopwm_config.pwm          = T19_PWM       ;
	gpiopwm_config.period       = T19_PERIOD    ;
	gpiopwm_config.duty[0]      = T19_DUTY_0    ;
	gpiopwm_config.duty[1]      = T19_DUTY_1    ;
	gpiopwm_config.duty[2]      = T19_DUTY_2    ;
	gpiopwm_config.duty[3]      = T19_DUTY_3    ;
	gpiopwm_config.trigger[0]   = T19_TRIGGER_0 ;
	gpiopwm_config.trigger[1]   = T19_TRIGGER_1 ;
	gpiopwm_config.trigger[2]   = T19_TRIGGER_2 ;
	gpiopwm_config.trigger[3]   = T19_TRIGGER_3 ;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&gpiopwm_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}


int mxt_proximity_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_TOUCH_PROXIMITY_T23);
	obj_size = MXT_GET_SIZE(MXT_TOUCH_PROXIMITY_T23);

	proximity_config.ctrl       = T23_CTRL;
	proximity_config.xorigin    = T23_XORIGIN;
	proximity_config.yorigin    = T23_YORIGIN;
	proximity_config.xsize      = T23_XSIZE;
	proximity_config.ysize      = T23_YSIZE;
	proximity_config.reserved   = T23_RESERVED;
	proximity_config.blen       = T23_BLEN;
	proximity_config.fxddthr    = T23_FXDDTHR;
	proximity_config.fxddi      = T23_FXDDI;
	proximity_config.average    = T23_AVERAGE;
	proximity_config.mvnullrate = T23_MVNULLRATE;
	proximity_config.mvdthr     = T23_MVDTHR;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&proximity_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}


int mxt_one_touch_gesture_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_PROCI_ONETOUCHGESTUREPROCESSOR_T24);
	obj_size = MXT_GET_SIZE(MXT_PROCI_ONETOUCHGESTUREPROCESSOR_T24);

	/* Disable one touch gestures. */
	onetouch_gesture_config.ctrl            = T24_CTRL      ;
	onetouch_gesture_config.numgest         = T24_NUMGEST   ;
	onetouch_gesture_config.gesten          = T24_GESTEN    ;
	onetouch_gesture_config.process         = T24_PROCESS   ;
	onetouch_gesture_config.tapto           = T24_TAPTO     ;
	onetouch_gesture_config.flickto         = T24_FLICKTO   ;
	onetouch_gesture_config.dragto          = T24_DRAGTO    ;
	onetouch_gesture_config.spressto        = T24_SPRESSTO  ;
	onetouch_gesture_config.lpressto        = T24_LPRESSTO  ;
	onetouch_gesture_config.reppressto      = T24_REPPRESSTO;
	onetouch_gesture_config.flickthr        = T24_FLICKTHR  ;
	onetouch_gesture_config.dragthr         = T24_DRAGTHR   ;
	onetouch_gesture_config.tapthr          = T24_TAPTHR    ;
	onetouch_gesture_config.throwthr        = T24_THROWTHR  ;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&onetouch_gesture_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}



int mxt_selftest_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_SPT_SELFTEST_T25);
	obj_size = MXT_GET_SIZE(MXT_SPT_SELFTEST_T25);

	selftest_config.ctrl    = T25_CTRL;
	selftest_config.cmd     = T25_CMD;

	/* Multiple Touch Touchscreen T9 */
	selftest_config.siglim[0].upsiglim = T25_SIGLIM_0_UPSIGLIM;
	selftest_config.siglim[0].losiglim = T25_SIGLIM_0_LOSIGLIM;

	/* Key Array T15 */
	selftest_config.siglim[1].upsiglim = T25_SIGLIM_1_UPSIGLIM;
	selftest_config.siglim[1].losiglim = T25_SIGLIM_1_LOSIGLIM;

	/* Proximity T23 */
	selftest_config.siglim[2].upsiglim = T25_SIGLIM_2_UPSIGLIM;
	selftest_config.siglim[2].losiglim = T25_SIGLIM_2_LOSIGLIM;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&selftest_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}


int mxt_gripsuppression_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_PROCI_GRIPSUPPRESSION_T40);
	obj_size = MXT_GET_SIZE(MXT_PROCI_GRIPSUPPRESSION_T40);

	gripsuppression_t40_config.ctrl     = T40_CTRL;
	gripsuppression_t40_config.xlogrip  = T40_XLOGRIP;
	gripsuppression_t40_config.xhigrip  = T40_XHIGRIP;
	gripsuppression_t40_config.ylogrip  = T40_YLOGRIP;
	gripsuppression_t40_config.yhigrip  = T40_YHIGRIP;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&gripsuppression_t40_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}


int mxt_touch_suppression_t42_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_PROCI_TOUCHSUPPRESSION_T42);
	obj_size = MXT_GET_SIZE(MXT_PROCI_TOUCHSUPPRESSION_T42);

	touchsuppression_t42_config.ctrl             = T42_CTRL;
	touchsuppression_t42_config.apprthr          = T42_APPRTHR;
	touchsuppression_t42_config.maxapprarea      = T42_MAXAPPRAREA;
	touchsuppression_t42_config.maxtcharea       = T42_MAXTCHAREA;
	touchsuppression_t42_config.supstrength      = T42_SUPSTRENGTH;
	touchsuppression_t42_config.supextto         = T42_SUPEXTTO;
	touchsuppression_t42_config.maxnumtchs       = T42_MAXNUMTCHS;
	touchsuppression_t42_config.shapestrength    = T42_SHAPESTRENGTH;


	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&touchsuppression_t42_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}


int mxt_cte_t46_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_SPT_CTECONFIG_T46);
	obj_size = MXT_GET_SIZE(MXT_SPT_CTECONFIG_T46);

	/* Set CTE config */
	cte_t46_config.ctrl			= T46_CTRL;
	cte_t46_config.mode		= T46_MODE;
	cte_t46_config.idlesyncsperx	= T46_IDLESYNCSPERX;
	cte_t46_config.actvsyncsperx	= T46_ACTVSYNCSPERX;
	cte_t46_config.adcspersync		= T46_ADCSPERSYNC;
	cte_t46_config.pulsesperadc		= T46_PULSESPERADC;
	cte_t46_config.xslew			= T46_XSLEW;
	cte_t46_config.syncdelay		= T46_SYNCDELAY;

	/* Write CTE config to chip. */
	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&cte_t46_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}


int mxt_stylus_t47_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_PROCI_STYLUS_T47);
	obj_size = MXT_GET_SIZE(MXT_PROCI_STYLUS_T47);

	stylus_t47_config.ctrl          = T47_CTRL;
	stylus_t47_config.contmin       = T47_CONTMIN;
	stylus_t47_config.contmax       = T47_CONTMAX;
	stylus_t47_config.stability     = T47_STABILITY;
	stylus_t47_config.maxtcharea    = T47_MAXTCHAREA;
	stylus_t47_config.amplthr       = T47_AMPLTHR;
	stylus_t47_config.styshape      = T47_STYSHAPE;
	stylus_t47_config.hoversup      = T47_HOVERSUP;
	stylus_t47_config.confthr       = T47_CONFTHR;
	stylus_t47_config.syncsperx     = T47_SYNCSPERX;


	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&stylus_t47_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}


int mxt_noisesuppression_t48_config(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48);
	obj_size = MXT_GET_SIZE(MXT_PROCG_NOISESUPPRESSION_T48);

	noisesuppression_t48_config.ctrl  				= T48_CTRL ;
	noisesuppression_t48_config.cfg  				= T48_CFG ;
	noisesuppression_t48_config.calcfg  			= (T48_CALCFG | T48_CHGON_BIT) ;
	noisesuppression_t48_config.basefreq  			= T48_BASEFREQ ;
	noisesuppression_t48_config.freq_0  			= T48_RESERVED0 ;
	noisesuppression_t48_config.freq_1  			= T48_RESERVED1 ;
	noisesuppression_t48_config.freq_2  			= T48_RESERVED2 ;
	noisesuppression_t48_config.freq_3  			= T48_RESERVED3 ;
	noisesuppression_t48_config.mffreq_2  			= T48_MFFREQ_2 ;
	noisesuppression_t48_config.mffreq_3  			= T48_MFFREQ_3 ;
	noisesuppression_t48_config.nlgain  			= T48_RESERVED4 ;
	noisesuppression_t48_config.nlthr  				= T48_RESERVED5 ;
	noisesuppression_t48_config.gclimit  			= T48_RESERVED6 ;
	noisesuppression_t48_config.gcactvinvldadcs  	= T48_GCACTVINVLDADCS ;
	noisesuppression_t48_config.gcidleinvldadcs  	= T48_GCIDLEINVLDADCS ;
	noisesuppression_t48_config.gcinvalidthr  		= T48_RESERVED7 ;
	/* noisesuppression_t48_config.reserved8  			= T48_RESERVED8 ; */
	noisesuppression_t48_config.gcmaxadcsperx  		= T48_GCMAXADCSPERX ;
	noisesuppression_t48_config.gclimitmin  		= T48_GCLIMITMIN ;
	noisesuppression_t48_config.gclimitmax  		= T48_GCLIMITMAX ;
	noisesuppression_t48_config.gccountmintgt  		= T48_GCCOUNTMINTGT ;
	noisesuppression_t48_config.mfinvlddiffthr  	= T48_MFINVLDDIFFTHR ;
	noisesuppression_t48_config.mfincadcspxthr  	= T48_MFINCADCSPXTHR ;
	noisesuppression_t48_config.mferrorthr  		= T48_MFERRORTHR ;
	noisesuppression_t48_config.selfreqmax  		= T48_SELFREQMAX ;
	noisesuppression_t48_config.reserved9  			= T48_RESERVED9 ;
	noisesuppression_t48_config.reserved10  		= T48_RESERVED10 ;
	noisesuppression_t48_config.reserved11  		= T48_RESERVED11 ;
	noisesuppression_t48_config.reserved12  		= T48_RESERVED12 ;
	noisesuppression_t48_config.reserved13  		= T48_RESERVED13 ;
	noisesuppression_t48_config.reserved14 		 	= T48_RESERVED14 ;
	noisesuppression_t48_config.blen  				= T48_BLEN ;
	noisesuppression_t48_config.tchthr  			= T48_TCHTHR ;
	noisesuppression_t48_config.tchdi  				= T48_TCHDI ;
	noisesuppression_t48_config.movhysti  			= T48_MOVHYSTI ;
	noisesuppression_t48_config.movhystn  			= T48_MOVHYSTN ;
	noisesuppression_t48_config.movfilter  			= T48_MOVFILTER ;
	noisesuppression_t48_config.numtouch  			= T48_NUMTOUCH ;
	noisesuppression_t48_config.mrghyst  			= T48_MRGHYST ;
	noisesuppression_t48_config.mrgthr  			= T48_MRGTHR ;
	noisesuppression_t48_config.xloclip  			= T48_XLOCLIP ;
	noisesuppression_t48_config.xhiclip  			= T48_XHICLIP ;
	noisesuppression_t48_config.yloclip  			= T48_YLOCLIP ;
	noisesuppression_t48_config.yhiclip  			= T48_YHICLIP ;
	noisesuppression_t48_config.xedgectrl  			= T48_XEDGECTRL ;
	noisesuppression_t48_config.xedgedist  			= T48_XEDGEDIST ;
	noisesuppression_t48_config.yedgectrl  			= T48_YEDGECTRL ;
	noisesuppression_t48_config.yedgedist  			= T48_YEDGEDIST ;
	noisesuppression_t48_config.jumplimit  			= T48_JUMPLIMIT ;
	noisesuppression_t48_config.tchhyst  			= T48_TCHHYST ;
	noisesuppression_t48_config.nexttchdi  			= T48_NEXTTCHDI ;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&noisesuppression_t48_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}
EXPORT_SYMBOL(mxt_noisesuppression_t48_config);

int mxt_noisesuppression_t48_config_for_TA(struct mxt_data *mxt)
{
	struct i2c_client *client = mxt->client;
	u16 obj_addr, obj_size;
	int error;

	obj_addr = MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48);
	obj_size = MXT_GET_SIZE(MXT_PROCG_NOISESUPPRESSION_T48);

	noisesuppression_t48_config.ctrl  				= T48_CTRL_TA ;
	noisesuppression_t48_config.cfg  				= T48_CFG_TA;
	noisesuppression_t48_config.calcfg  			= T48_CALCFG_TA;
	noisesuppression_t48_config.basefreq  			= T48_BASEFREQ_TA;
	noisesuppression_t48_config.freq_0  			= T48_RESERVED0_TA;
	noisesuppression_t48_config.freq_1  			= T48_RESERVED1_TA;
	noisesuppression_t48_config.freq_2  			= T48_RESERVED2_TA;
	noisesuppression_t48_config.freq_3  			= T48_RESERVED3_TA;
	noisesuppression_t48_config.mffreq_2  			= T48_MFFREQ_2_TA;
	noisesuppression_t48_config.mffreq_3  			= T48_MFFREQ_3_TA;
	noisesuppression_t48_config.nlgain  			= T48_RESERVED4_TA;
	noisesuppression_t48_config.nlthr  				= T48_RESERVED5_TA;
	noisesuppression_t48_config.gclimit  			= T48_RESERVED6_TA;
	noisesuppression_t48_config.gcactvinvldadcs  	= T48_GCACTVINVLDADCS_TA;
	noisesuppression_t48_config.gcidleinvldadcs  	= T48_GCIDLEINVLDADCS_TA;
	noisesuppression_t48_config.gcinvalidthr  		= T48_RESERVED7_TA;
	/* noisesuppression_t48_config.reserved8  			= T48_RESERVED8_TA ; */
	noisesuppression_t48_config.gcmaxadcsperx  		= T48_GCMAXADCSPERX_TA;
	noisesuppression_t48_config.gclimitmin  		= T48_GCLIMITMIN_TA;
	noisesuppression_t48_config.gclimitmax  		= T48_GCLIMITMAX_TA;
	noisesuppression_t48_config.gccountmintgt  		= T48_GCCOUNTMINTGT_TA;
	noisesuppression_t48_config.mfinvlddiffthr  	= T48_MFINVLDDIFFTHR_TA;
	noisesuppression_t48_config.mfincadcspxthr  	= T48_MFINCADCSPXTHR_TA;
	noisesuppression_t48_config.mferrorthr  		= T48_MFERRORTHR_TA;
	noisesuppression_t48_config.selfreqmax  		= T48_SELFREQMAX_TA;
	noisesuppression_t48_config.reserved9  			= T48_RESERVED9_TA;
	noisesuppression_t48_config.reserved10  		= T48_RESERVED10_TA;
	noisesuppression_t48_config.reserved11  		= T48_RESERVED11_TA;
	noisesuppression_t48_config.reserved12  		= T48_RESERVED12_TA;
	noisesuppression_t48_config.reserved13  		= T48_RESERVED13_TA;
	noisesuppression_t48_config.reserved14 		 	= T48_RESERVED14_TA;
	noisesuppression_t48_config.blen  				= T48_BLEN_TA;
	noisesuppression_t48_config.tchthr  			= T48_TCHTHR_TA;
	noisesuppression_t48_config.tchdi  				= T48_TCHDI_TA;
	noisesuppression_t48_config.movhysti  			= T48_MOVHYSTI_TA;
	noisesuppression_t48_config.movhystn  			= T48_MOVHYSTN_TA;
	noisesuppression_t48_config.movfilter  			= T48_MOVFILTER_TA;
	noisesuppression_t48_config.numtouch  			= T48_NUMTOUCH_TA;
	noisesuppression_t48_config.mrghyst  			= T48_MRGHYST_TA;
	noisesuppression_t48_config.mrgthr  			= T48_MRGTHR_TA;
	noisesuppression_t48_config.xloclip  			= T48_XLOCLIP_TA;
	noisesuppression_t48_config.xhiclip  			= T48_XHICLIP_TA;
	noisesuppression_t48_config.yloclip  			= T48_YLOCLIP_TA;
	noisesuppression_t48_config.yhiclip  			= T48_YHICLIP_TA;
	noisesuppression_t48_config.xedgectrl  			= T48_XEDGECTRL_TA;
	noisesuppression_t48_config.xedgedist  			= T48_XEDGEDIST_TA;
	noisesuppression_t48_config.yedgectrl  			= T48_YEDGECTRL_TA;
	noisesuppression_t48_config.yedgedist  			= T48_YEDGEDIST_TA;
	noisesuppression_t48_config.jumplimit  			= T48_JUMPLIMIT_TA;
	noisesuppression_t48_config.tchhyst  			= T48_TCHHYST_TA;
	noisesuppression_t48_config.nexttchdi  			= T48_NEXTTCHDI_TA;

	error = mxt_write_block(client, obj_addr, obj_size, (u8 *)&noisesuppression_t48_config);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] mxt_write_block failed! (%s, %d)\n", __func__, __LINE__);
		return -EIO;
	}

	return 0;
}
EXPORT_SYMBOL(mxt_noisesuppression_t48_config_for_TA);



int mxt_config_settings(struct mxt_data *mxt)
{
	pr_info("[TSP] mxt_config_settings");


	if (mxt_userdata_config(mxt) < 0)		/* T38 */
		return -1;

	if (mxt_power_config(mxt) < 0)             /* T7 */
		return -1;

	if (mxt_acquisition_config(mxt) < 0)      /* T8 */
		return -1;

	if (mxt_multitouch_config(mxt) < 0)       /* T9 */
		return -1;

	/*--------------------------------------------------------------------*/
	if (mxt_keyarray_config(mxt) < 0)         /* T15 */
		return -1;

	if (mxt_comc_config(mxt) < 0)             /* T18 */
		return -1;

	if (mxt_gpio_pwm_config(mxt) < 0)         /* T19 */
		return -1;

	if (mxt_proximity_config(mxt) < 0)        /* T23 */
		return -1;

	if (mxt_selftest_config(mxt) < 0)              /* T25 */
		return -1;

	if (mxt_gripsuppression_config < 0)            /* T40 */
		return -1;
	/*-------------------------------------------------------------------*/

	if (mxt_touch_suppression_t42_config(mxt) < 0) /* T42 */
		return -1;

	if (mxt_cte_t46_config(mxt) < 0)               /* T46 */
		return -1;

	if (mxt_stylus_t47_config(mxt) < 0)            /* T47 */
		return -1;

	if (mxt_noisesuppression_t48_config(mxt) < 0)  /* T48 */
		return -1;
	return 0;
}

/*
* Bootloader functions
*/

static void bootloader_status(u8 value)
{
	u8 *str = NULL;

	switch (value) {
	case 0xC0:
		str = "WAITING_BOOTLOAD_CMD"; break;
	case 0x80:
		str = "WAITING_FRAME_DATA"; break;
	case 0x40:
		str = "APP_CRC_FAIL"; break;
	case 0x02:
		str = "FRAME_CRC_CHECK"; break;
	case 0x03:
		str = "FRAME_CRC_FAIL"; break;
	case 0x04:
		str = "FRAME_CRC_PASS"; break;
	default:
		str = "Unknown Status";
	}

	pr_info("[TSP] bootloader status: %s (0x%02X)\n", str, value);
}

static int check_bootloader(struct i2c_client *client, unsigned int status)
{
	u8 val = 0;
	u16 retry = 0;

	msleep(10);  /* recommendation from ATMEL */

recheck:
	if (i2c_master_recv(client, &val, 1) != 1) {
		pr_err("[TSP] i2c recv failed");
		return -EIO;
	}

	switch (status) {
	case WAITING_BOOTLOAD_COMMAND:
	case WAITING_FRAME_DATA:
		val &= ~BOOTLOAD_STATUS_MASK;
		bootloader_status(val);
		if (val == APP_CRC_FAIL) {
			pr_info("[TSP] We've got a APP_CRC_FAIL, so try again (count=%d)", ++retry);
			goto recheck;
		}
		break;

	case FRAME_CRC_PASS:
		bootloader_status(val);
		if (val == FRAME_CRC_CHECK) {
			goto recheck;
		}
		break;

	default:
		return -EINVAL;
	}

	if (val != status) {
		pr_err("[TSP] Invalid status: 0x%02X ", val);
		return -EINVAL;
	}

	return 0;
}

static int unlock_bootloader(struct i2c_client *client)
{
	u8 cmd[2] = { 0};

	cmd[0] = 0xdc;  /* MXT_CMD_UNLOCK_BL_LSB */
	cmd[1] = 0xaa;  /*MXT_CMD_UNLOCK_BL_MSB */

	return i2c_master_send(client, cmd, 2);
}

int mxt_load_firmware(struct device *dev, const char *fn)
{
	struct mxt_data *mxt = dev_get_drvdata(dev);

	unsigned int frame_size;
	unsigned int pos = 0;
	unsigned int retry;
	int ret;

#if READ_FW_FROM_HEADER
	struct firmware *fw = NULL;
	fw = kzalloc(sizeof(struct firmware), GFP_KERNEL);

	fw->data = firmware_latest;
	fw->size = sizeof(firmware_latest);

#else
	const struct firmware *fw = NULL;

	ret = request_firmware(&fw, fn, dev);
	if (ret < 0) {
		dev_err(&client->dev, "[TSP] Unable to open firmware %s\n", fn);
		return -ENOMEM;
	}
#endif

	/* set resets into bootloader mode */
	reset_chip(mxt, RESET_TO_BOOTLOADER);
	msleep(250);  /* mdelay(100); */

	/* change to slave address of bootloader */
	if (mxt->client->addr == MXT_I2C_APP_ADDR) {
		pr_info("[TSP] I2C address: 0x%02X --> 0x%02X", MXT_I2C_APP_ADDR, MXT_I2C_BOOTLOADER_ADDR);
		mxt->client->addr = MXT_I2C_BOOTLOADER_ADDR;
	}

	ret = check_bootloader(mxt->client, WAITING_BOOTLOAD_COMMAND);
	if (ret < 0) {
		pr_err("[TSP] ... Waiting bootloader command: Failed");
		goto err_fw;
	}

	/* unlock bootloader */
	unlock_bootloader(mxt->client);
	msleep(200);  /* mdelay(100); */

	/* reading the information of the firmware */
	pr_info("[TSP] Firmware info: version [0x%02X], build [0x%02X]", fw->data[0], fw->data[1]);
	pr_info("Updating progress: ");
	pos += 2;

	while (pos < fw->size) {
		retry = 0;
		ret = check_bootloader(mxt->client, WAITING_FRAME_DATA);
		if (ret < 0) {
			pr_err("... Waiting frame data: Failed");
			goto err_fw;
		}

		frame_size = ((*(fw->data + pos) << 8) | *(fw->data + pos + 1));

		/* We should add 2 at frame size as the the firmware data is not
		* included the CRC bytes.
		*/
		frame_size += 2;

		/* write one frame to device */
try_to_resend_the_last_frame:
		i2c_master_send(mxt->client, (u8 *)(fw->data + pos), frame_size);

		ret = check_bootloader(mxt->client, FRAME_CRC_PASS);
		if (ret < 0) {
			if (++retry < 10) {
				check_bootloader(mxt->client, WAITING_FRAME_DATA);  /* recommendation from ATMEL */
				pr_info("[TSP] We've got a FRAME_CRC_FAIL, so try again up to 10 times (count=%d)", retry);
				goto try_to_resend_the_last_frame;
			}
			pr_err("... CRC on the frame failed after 10 trials!");
			goto err_fw;
		}

		pos += frame_size;

		pr_info("#");
		pr_info("%zd / %zd (bytes) updated...", pos, fw->size);
	}
	pr_info("\n[TSP] Updating firmware completed!\n");
	pr_info("[TSP] note: You may need to reset this target.\n");

err_fw:
	/* change to slave address of application */
	if (mxt->client->addr == MXT_I2C_BOOTLOADER_ADDR) {
		pr_info("[TSP] I2C address: 0x%02X --> 0x%02X", MXT_I2C_BOOTLOADER_ADDR, MXT_I2C_APP_ADDR);
		mxt->client->addr = MXT_I2C_APP_ADDR;
	}

#if READ_FW_FROM_HEADER
	kfree(fw);
#endif

	return ret;
}
