/*
 * cmc623.c
 *
 * driver supporting CMC623 ImageConverter functions for Samsung P3 device
 *
 * COPYRIGHT(C) Samsung Electronics Co., Ltd. 2006-2010 All Right Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include <linux/wakelock.h>
#include <linux/blkdev.h>
#include <linux/i2c.h>
#include <mach/gpio.h>

#include <linux/delay.h>
#include <linux/workqueue.h>

#include <linux/regulator/consumer.h>
#include <linux/earlysuspend.h>


#include "cmc623.h"

static struct regulator *cmc623_regulator_csi;

#define USE_IMACONV_CMC623 1 // Set "0" for CMC bypass
#define n1_ld9040 0


#define CABC_ONOFF_TEST 1
#define BYPASS_ONOFF_TEST 1

#if BYPASS_ONOFF_TEST
static int current_bypass_onoff = 0;
void bypass_onoff_ctrl(int);
#endif //BYPASS_ONOFF_TEST

#if 0
#define dprintk(x...) printk(x)
#else
#define dprintk(x...) (0)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define GPIO_LEVEL_LOW      	0
#define GPIO_LEVEL_HIGH     	1

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend	cmc623_early_suspend;
#endif

#define MAX_LEVEL 	1600

#define CMC623_I2C_SPEED_KHZ  400
#define CMC623_DEVICE_ADcDR  0x38

#if defined(CMC623_SERVICE_EXTEND)
extern void tune_cmc623_pwm_brightness(int value);
extern void cmc623_cabc_enable(int enable);
extern int cmc623_service_suspend(void);
extern int cmc623_service_resume(void);
extern void init_cmc623_service(struct cmc623_data *pdata);
#endif

/* Each client has this additional data */
struct cmc623_data {
	struct i2c_client *client;
};

/*
  TODO: These must be made platform agnostic.
  gpio-names.h must not be referenced external to the mach-tegra directory.
  The proper way of doing this is to have a platform_data struct corresponding to cmc623 with these parameters
  When instantiated in board-* fill in the parameters with the corresponding gpio
*/

#define TEGRA_GPIO_PQ4		132
#define TEGRA_GPIO_PJ6		78
#define TEGRA_GPIO_PBB5		221
#define TEGRA_GPIO_PW0		176
#define TEGRA_GPIO_PL5		93
#define TEGRA_GPIO_PX0		184
#define TEGRA_GPIO_PR3		139

#define GPIO_IMA_PWREN TEGRA_GPIO_PQ4
#define GPIO_IMA_N_RST TEGRA_GPIO_PJ6
#define GPIO_IMA_BYPASS TEGRA_GPIO_PBB5
#define GPIO_IMA_SLEEP TEGRA_GPIO_PW0


#define GPIO_IMA_LDOEN1 TEGRA_GPIO_PL5
#define GPIO_IMA_LDOEN2 TEGRA_GPIO_PX0

#define GPIO_LCD_LDO_LED_EN TEGRA_GPIO_PR3

static unsigned int cmc623_suspend_flag;

static struct cmc623_data * p_cmc623_data = NULL;
struct workqueue_struct *ove_wq=NULL;
struct work_struct work_ove;

static struct regulator *reg_cmc623io_1v8;

static struct i2c_client *g_client;
#define I2C_M_WR 0 /* for i2c */
#define I2c_M_RD 1 /* for i2c */

static int current_gamma_level = MAX_LEVEL;

//static int lcdonoff = FALSE;
int lcdonoff = FALSE;

struct cmc623_state_type{
	unsigned int cabc_enabled;
	unsigned int brightness;
	unsigned int suspended;
	int white;
	int black;
	int saturation;
};

static struct cmc623_state_type cmc623_state = {
	.cabc_enabled = TRUE,
	.brightness = 32,
	.suspended = FALSE,
	.white = 0,
	.black = 0,
	.saturation = 0,
};

struct mDNIe_data_type{
	unsigned short addr;
	unsigned short data;
};

enum Lcd_mDNIe_UI{
	mDNIe_UI_MODE,
	mDNIe_VIDEO_MODE,
	mDNIe_VIDEO_WARM_MODE,
	mDNIe_VIDEO_COLD_MODE,
	mDNIe_CAMERA_MODE,
	mDNIe_NAVI,
	mDNIe_DMB_MODE,
	mDNIe_VT_MODE,
};

enum mDNIe_mode_CABC_type{
	mode_type_CABC_none,
	mode_type_CABC_on,
	mode_type_CABC_off,
};

/*mDNIe Set Status Checking Value.*/
static enum Lcd_CMC623_UI_mode current_cmc623_UI = CMC623_UI_MODE;
static int current_cmc623_OutDoor_OnOff = FALSE;
static int current_cmc623_CABC_OnOff = FALSE;

static int setting_first = FALSE;
static int cmc623_bypass_mode = FALSE;
static int current_autobrightness_enable = FALSE;
static int cmc623_current_region_enable = FALSE;

enum mDNIe_mode_CABC_type cmc623_cabc_mode[] = {
	mode_type_CABC_none,		/*UI*/
	mode_type_CABC_on,		/*Video*/
	mode_type_CABC_on,		/*Video warm*/
	mode_type_CABC_on,		/*Video cold*/
	mode_type_CABC_off, 	/*Camera*/
	mode_type_CABC_none,		/*Navi*/
};

#define NUM_ITEM_POWER_LUT	9
#define NUM_POWER_LUT	2

static int current_power_lut_num = FALSE;

unsigned char cmc623_Power_LUT[NUM_POWER_LUT][NUM_ITEM_POWER_LUT]={
	{ 0x48, 0x4d, 0x44, 0x52, 0x48, 0x45, 0x40, 0x3d, 0x45 },
	{ 0x3e, 0x43, 0x3a, 0x48, 0x3e, 0x3b, 0x36, 0x33, 0x3b },
};

static bool cmc623_I2cWrite16(unsigned char Addr, unsigned long Data);
static void cmc623_cabc_pwm_brightness_reg(int value);
static void cmc623_manual_pwm_brightness_reg(int value);
static void cmc623_manual_pwm_brightness_reg_nosync(int value);

unsigned long last_cmc623_Bank = 0xffff;
unsigned long last_cmc623_Algorithm = 0xffff;

/*
*
 * functions for I2C transactions
 */
static int cmc623_I2cWrite(struct i2c_client *client, u8 reg,
				u8 *data, u8 length)
{
	int ret, i;
	u8 buf[length+1];
	struct i2c_msg msg[1];

	buf[0] = reg;
	for(i=0; i<length; i++)
		buf[i+1] = *(data++);

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = length+1;
	msg[0].buf = buf;

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret != 1)
		return -EIO;

	return 0;
}


bool cmc623_I2cWrite16(unsigned char Addr, unsigned long Data)
{
	int err = -1000;
	struct i2c_msg msg[1];
	unsigned char data[3];

	pr_debug("========cmc623_I2cWrite16=======(a:%x,d:%x)\n", Addr, Data);

	if (!p_cmc623_data) {
		pr_err("p_cmc623_data is NULL\n");
		return -ENODEV;
	}
	g_client = p_cmc623_data->client;

	if ((g_client == NULL)) {
		pr_err("cmc623_I2cWrite16 g_client is NULL\n");
		return -ENODEV;
	}

	if (!g_client->adapter) {
		pr_err("cmc623_I2cWrite16 g_client->adapter is NULL\n");
		return -ENODEV;
	}

	if (TRUE == cmc623_state.suspended) {
		pr_info("cmc623 don't need writing while"
			" LCD off(a:%x,d:%x)\n", Addr, Data);
		return 0;
	}


/*
	if (Addr == 0x0000) {
		if(Data == last_cmc623_Bank) {
			pr_err("last_cmc623_Bank is returned\n");
			return 0;
		}
		last_cmc623_Bank = Data;
	} else if (Addr == 0x0001) {
		last_cmc623_Algorithm = Data;
	}
*/
	data[0] = Addr;
	data[1] = ((Data >>8)&0xFF);
	data[2] = (Data)&0xFF;
	msg->addr = g_client->addr;
	msg->flags = I2C_M_WR;
	msg->len = 3;
	msg->buf = data;

	err = i2c_transfer(g_client->adapter, msg, 1);

	if (err >= 0) {
		/* add by inter.park */
		pr_debug("%s %d i2c transfer OK\n", __func__, __LINE__);
		return 1;
	}

	/* add by inter.park */
	pr_err("%s i2c transfer error:%d(a:%d)\n",
			__func__, err, Addr);
	return err;
}


char cmc623_I2cRead(u8 reg, u8 *val, unsigned int len )
{
	int 	 err;
	struct	 i2c_msg msg[1];

	unsigned char data[1];
	if( (g_client == NULL) || (!g_client->adapter) )
		return -ENODEV;

	msg->addr	= g_client->addr;
	msg->flags	= I2C_M_WR;
	msg->len	= 1;
	msg->buf	= data;
	*data		= reg;

	err = i2c_transfer(g_client->adapter, msg, 1);

	if (err >= 0) {
		msg->flags = I2C_M_RD;
		msg->len   = len;
		msg->buf   = val;
		err = i2c_transfer(g_client->adapter, msg, 1);
	}

	if (err >= 0)
		return 0;

	/* add by inter.park */
	pr_err("%s %d i2c transfer error\n", __func__, __LINE__);

	return err;

}


int cmc623_I2cRead16(u8 reg, u16 *val)
{
	int 	 err;
	struct	 i2c_msg msg[2];
	u8 regaddr = reg;
	u8 data[2];

	if (!p_cmc623_data) {
		pr_err("%s p_cmc623_data is NULL\n", __func__);
		return -ENODEV;
	}
	g_client = p_cmc623_data->client;

	if ((g_client == NULL)) {
		pr_err("%s g_client is NULL\n", __func__);
		return -ENODEV;
	}

	if (!g_client->adapter) {
		pr_err("%s g_client->adapter is NULL\n", __func__);
		return -ENODEV;
	}

	if (regaddr == 0x0001) {
		*val = last_cmc623_Algorithm;
		return 0;
	}

	msg[0].addr   = g_client->addr;
	msg[0].flags  = I2C_M_WR;
	msg[0].len	  = 1;
	msg[0].buf	  = &regaddr;
	msg[1].addr   = g_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len	 = 2;
	msg[1].buf	 = &data[0];
	err = i2c_transfer(g_client->adapter, &msg[0], 2);

	if (err >= 0) {
		*val = (data[0]<<8) | data[1];
		return 0;
	}
	/* add by inter.park */
	pr_err("%s %d i2c transfer error: %d\n",
			__func__, __LINE__, err);

	return err;
}



/*#define CMC623_INITSEQ cmc623_init1*/
#if n1_ld9040
#define CMC623_INITSEQ cmc623_init_amoled
#else
#define CMC623_INITSEQ cmc623_init2
#endif



static bool CMC623_SetUp(void)
{
    int i=0;
    int num = ARRAY_SIZE(CMC623_INITSEQ);
	/*pr_err("%s num is %d\n", __func__, num);*/

    for (i=0; i<num; i++) {
		if (!cmc623_I2cWrite16(CMC623_INITSEQ[i].RegAddr,
		CMC623_INITSEQ[i].Data)) {
			pr_err("why return false??!!!\n");
            return FALSE;
        }
        if (CMC623_INITSEQ[i].RegAddr == CMC623_REG_SWRESET &&
            CMC623_INITSEQ[i].Data == 0xffff)
			usleep_range(3000, 5000);
    }
    return TRUE;
}

void cmc623_reg_unmask(void)
{
	if (!p_cmc623_data) {
		pr_err("%s cmc623 is not initialized\n", __func__);
	return;
	}

	cmc623_I2cWrite16(0x28, 0x0000);
}

unsigned int ove_target_value = FALSE;
void ove_workqueue_func(void* data)
{
	int i = 0;
	for (i = 0; i <= 8; ++i) {
		if(cmc623_state.suspended == TRUE)
			return;

		cmc623_I2cWrite16(0x0054,
				(((ove_target_value >> 8) * i / 8) << 8)
				| ((ove_target_value & 0x00ff) * i / 8));
		cmc623_reg_unmask();
		msleep(20);
	}
}

#if !defined(CMC623_SERVICE_EXTEND)

#define CMC623_TUNESEQ cmc623_cabcon3
#define CMC623_TUNESEQ2 cmc623_cabcoff3
/*#define CMC623_TUNESEQ cmc623_bypass*/

void cabc_onoff_ctrl(int value)
{
	if (value == 1) {
		int i = 0;
		int num = ARRAY_SIZE(CMC623_TUNESEQ);

		for (i=0; i<num; i++)
			cmc623_I2cWrite16(CMC623_TUNESEQ[i].RegAddr,
					CMC623_TUNESEQ[i].Data);

		pr_debug("[cabc_on] <= value : %d\n", value);
	} else if (value == 0) {
		int i = 0;
		int num = ARRAY_SIZE(CMC623_TUNESEQ2);

		for (i=0; i<num; i++)
			cmc623_I2cWrite16(CMC623_TUNESEQ2[i].RegAddr,
					CMC623_TUNESEQ2[i].Data);

		pr_debug("[cabc_off] <= value : %d\n", value);
	}

}
#else // CMC623_SERVICE_EXTEND
void cabc_onoff_ctrl(int value)
{
	cmc623_cabc_enable(value);
}
#endif // CMC623_SERVICE_EXTEND

static void CMC623_Set_Mode(void)
{
#if 0
	int i=0;
	int num = ARRAY_SIZE(CMC623_TUNESEQ);
	/*pr_err("%s num is %d\n", __func__, num);	*/
	cmc623_state.cabc_enabled = TRUE;

	for (i=0; i<num; i++) {
		if (!cmc623_I2cWrite16(CMC623_TUNESEQ[i].RegAddr,
					CMC623_TUNESEQ[i].Data))
			return FALSE;
	}
	return TRUE;
#else
	cabc_onoff_ctrl(cmc623_state.cabc_enabled);
#endif
}


/* value: 0 ~ 1600*/
static void cmc623_cabc_pwm_brightness_reg(int value)
{
	int reg;
	unsigned char * p_plut;
	u16 min_duty;

	if (!p_cmc623_data) {
		pr_err("%s cmc623 is not initialized\n", __func__);
		return;
	}

	p_plut = cmc623_Power_LUT[current_power_lut_num];
	min_duty = p_plut[7] * value / 100;
	if (min_duty < 4) {
		reg = 0x4000 | ((max(1, (value*p_plut[3]/100))) << 4);
	} else {

		 /*PowerLUT*/
		cmc623_I2cWrite16(0x76, (p_plut[0] * value / 100) << 8
				| (p_plut[1] * value / 100));
		cmc623_I2cWrite16(0x77, (p_plut[2] * value / 100) << 8
				| (p_plut[3] * value / 100));
		cmc623_I2cWrite16(0x78, (p_plut[4] * value / 100) << 8
				| (p_plut[5] * value / 100));
		cmc623_I2cWrite16(0x79, (p_plut[6] * value / 100) << 8
				| (p_plut[7] * value / 100));
		cmc623_I2cWrite16(0x7a, (p_plut[8] * value / 100) << 8);

	reg = 0x5000 | (value<<4);
	}

	cmc623_I2cWrite16(0xB4, reg);
}

int Islcdonoff(void)
{
	return lcdonoff;
}
EXPORT_SYMBOL(Islcdonoff);

/*value: 0 ~ 100*/
static void cmc623_cabc_pwm_brightness(int value)
{
	int reg;
	unsigned char * p_plut;

	if (!p_cmc623_data) {
		pr_err("%s cmc623 is not initialized\n", __func__);
		return;
	}

	p_plut = cmc623_Power_LUT[current_power_lut_num];

	cmc623_I2cWrite16(0x00,0x0000);	//BANK 0

	cmc623_cabc_pwm_brightness_reg(value);

	cmc623_I2cWrite16(0x28,0x0000);
}

/*value: 0 ~ 100*/
static void cmc623_manual_pwm_brightness(int value)
{
	int reg;

	if (!p_cmc623_data) {
		pr_err("%s cmc623 is not initialized\n", __func__);
		return;
	}

	reg = 0x4000 | (value<<4);

	cmc623_I2cWrite16(0x00, 0x0000);
	cmc623_I2cWrite16(0xB4, reg);
	cmc623_I2cWrite16(0x28, 0x0000);

}

#if !defined(CMC623_SERVICE_EXTEND)
/* value: 0 ~ 1600*/
void cmc623_pwm_brightness(int value)
{
	int data;
	pr_debug("%s : BL brightness level = %d\n", __func__, value);

	if(value<0)
		data = 0;
	else if(value>1600)
		data = 1600;
	else
		data = value;
#if 1//setting ove
	if (data == 1280 && current_autobrightness_enable) {/*outdoor mode on*/
		current_cmc623_OutDoor_OnOff = TRUE;
		cmc623_Set_Mode(current_cmc623_UI, current_cmc623_CABC_OnOff);
	} else if (current_cmc623_OutDoor_OnOff == TRUE && data < 1280) {
		/*outdoor mode off*/
		current_cmc623_OutDoor_OnOff = FALSE;
		cmc623_Set_Mode(current_cmc623_UI, current_cmc623_CABC_OnOff);
	}
#else
	/*CMC623_Set_Mode();*/
#endif
	if (data < 21)
		data = 1; /*Range of data 0~1600, min value 0~15 is same as 0*/
	else
		data = data >> 4;

	cmc623_state.brightness = data;
#if BYPASS_ONOFF_TEST
	if((cmc623_state.cabc_enabled == TRUE)&&(current_bypass_onoff==FALSE))
	{
		cmc623_cabc_pwm_brightness(data);
	}
	else
	{
		cmc623_manual_pwm_brightness(data);
	}
#else //BYPASS_ONOFF_TEST
	if(cmc623_state.cabc_enabled == TRUE)
		cmc623_cabc_pwm_brightness(data);
	else
		cmc623_manual_pwm_brightness(data);
#endif //BYPASS_ONOFF_TEST
}
#else // CMC623_SERVICE_EXTEND
void cmc623_pwm_brightness(int value)
{
	tune_cmc623_pwm_brightness(value);
}
#endif // CMC623_SERVICE_EXTEND

#if BYPASS_ONOFF_TEST
// value: 0 ~ 1600
void cmc623_pwm_brightness_bypass(int value)
{
	int data;
	dprintk(KERN_INFO "%s : BL brightness level = %d\n", __func__, value);

	if(value<0)
		data = 0;
	else if(value>1600)
		data = 1600;
	else
		data = value;

	cmc623_state.brightness = data;
#if 0
	if(cmc623_state.cabc_enabled == TRUE)
	{
		cmc623_cabc_pwm_brightness(data);
	}
	else
#endif
	{
		cmc623_manual_pwm_brightness(data);
	}
}
#endif //BYPASS_ONOFF_TEST


void cmc623_pwm_apply(int level)
{
	int i = 0;
/*
	for(i=0; i<100; i++) {
		if(Islcdonoff())
			break;
		msleep(20);
	};
*/
	if (Islcdonoff()) {
		/*
		pr_debug("%s : BL brightness level = %d\n",
			__func__, level);
		*/
		cmc623_pwm_brightness(level);
	}
	current_gamma_level = level;
}
EXPORT_SYMBOL(cmc623_pwm_apply);

static int cmc623_hw_rst()
{

	return 0;
}

int panel_gpio_init()
{
	int ret;
	pr_info("%s called\n", __func__);

	/* LVDS GPIO Initialize */

	ret = gpio_request(GPIO_IMA_LDOEN1, "GPIO_IMA_LDOEN1");
	if (ret) {
		pr_err(KERN_ERR "failed to request CMC623 GPIO%d\n",
				GPIO_IMA_LDOEN2);
		return ret;
	}

       ret = gpio_request(GPIO_LCD_LDO_LED_EN, "GPIO_LCD_LDO_LED_EN");
	if (ret) {
		pr_err(KERN_ERR "failed to request CMC623 GPIO%d\n",
				GPIO_LCD_LDO_LED_EN);
		return ret;
	}

       ret = gpio_direction_output(GPIO_IMA_LDOEN1, 1);
	if (ret < 0)
		goto cleanup;
	regulator_enable(reg_cmc623io_1v8);

	return 0;
cleanup:
	gpio_free(GPIO_IMA_LDOEN1);
	gpio_free(GPIO_LCD_LDO_LED_EN);
}

int cmc623_gpio_init()
{
	int ret;
	pr_info("%s called\n", __func__);

	/* LVDS GPIO Initialize */
	ret = gpio_request(GPIO_IMA_PWREN, "GPIO_IMA_PWREN");
	if (ret) {
		pr_err("failed to request CMC623 GPIO%d\n",
				GPIO_IMA_PWREN);
		return ret;
	}

	ret = gpio_request(GPIO_IMA_N_RST, "GPIO_IMA_N_RST");
	if (ret) {
		pr_err("failed to request CMC623 GPIO%d\n",
				GPIO_IMA_N_RST);
		return ret;
	}

	ret = gpio_request(GPIO_IMA_BYPASS, "GPIO_IMA_BYPASS");
	if (ret) {
		pr_err("failed to request CMC623 GPIO%d\n",
				GPIO_IMA_BYPASS);
		return ret;
	}

	ret = gpio_request(GPIO_IMA_SLEEP, "GPIO_IMA_SLEEP");
	if (ret) {
		pr_err("failed to request CMC623 GPIO%d\n",
				GPIO_IMA_SLEEP);
		return ret;
	}

	ret = gpio_direction_output(GPIO_IMA_PWREN, 1);
	if (ret < 0)
		goto cleanup;

	ret = gpio_direction_output(GPIO_IMA_N_RST, 1);
	if (ret < 0)
		goto cleanup;

       ret = gpio_direction_output(GPIO_IMA_BYPASS, 1);
	if (ret < 0)
		goto cleanup;

	ret = gpio_direction_output(GPIO_IMA_SLEEP, 1);
	if (ret < 0)
		goto cleanup;

#if 0
	ret = gpio_direction_output(GPIO_IMA_N_RST, 0);
	if (ret < 0)
		goto cleanup;
       msleep(5);

	ret = gpio_direction_output(GPIO_IMA_N_RST, 1);
	if (ret < 0)
		goto cleanup;

       msleep(16);
#endif
	return 0;

cleanup:
	gpio_free(GPIO_IMA_PWREN);
	gpio_free(GPIO_IMA_N_RST);
	gpio_free(GPIO_IMA_BYPASS);
	gpio_free(GPIO_IMA_SLEEP);
	return ret;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
int cmc623_suspend(struct early_suspend *h)
{
	if (cmc623_suspend_flag == 1) {
		printk(KERN_INFO "%s is already called\n", __func__);
		return 0;
	} else {
		cmc623_suspend_flag = 1;
	}

	pr_info("-0- %s called -0-\n", __func__);

	cmc623_state.suspended = TRUE;
	lcdonoff = FALSE;

	if (!p_cmc623_data) {
		pr_err("%s cmc623 is not initialized\n", __func__);
		return 0;
	}

#if defined(CMC623_SERVICE_EXTEND)
	cmc623_service_suspend();
#endif
	/* 1.2V/1.8V/3.3V may be on

	CMC623[0x07] := 0x0004
	cmc623_I2cWrite16(0x07, 0x0004);*/
	gpio_set_value(GPIO_IMA_LDOEN1, GPIO_LEVEL_LOW);

	regulator_disable(reg_cmc623io_1v8);

	/*
	gpio_set_value(GPIO_LCD_LDO_LED_EN, GPIO_LEVEL_LOW);
	*/

	/* CMC623 SLEEPB <= LOW*/
	gpio_set_value(GPIO_IMA_SLEEP, GPIO_LEVEL_LOW);

	/*CMC623 BYPASSB <= LOW*/
	gpio_set_value(GPIO_IMA_BYPASS, GPIO_LEVEL_LOW);

	/*CMC623 RESET_B <= LOW*/
	gpio_set_value(GPIO_IMA_N_RST, GPIO_LEVEL_LOW);

	/* wait 1ms*/
	usleep_range(1000, 2000);

	/* CMC623 FAILSAFEB <= LOW*/
	gpio_set_value(GPIO_IMA_PWREN, GPIO_LEVEL_LOW);

	/*cmc623_state.suspended = TRUE;*/

	return 0;
}
EXPORT_SYMBOL(cmc623_suspend);


int cmc623_pre_resume()
{
#if 0
	/* CMC 623 PIN Default setting */
	/*set RESETB to HIGH*/
	gpio_set_value(GPIO_IMA_PWREN, GPIO_LEVEL_HIGH);
	udelay(30);

	/* Enable LVDS Panel Power, 1.2, 1.8, display 3.3V enable */
	gpio_set_value(GPIO_MLCD_ON, GPIO_LEVEL_HIGH);

	udelay(300);

	/*nReset to high*/
	gpio_set_value(GPIO_LVDS_N_SHDN, GPIO_LEVEL_HIGH);
	udelay(300);
#else
	gpio_set_value(GPIO_IMA_LDOEN1, GPIO_LEVEL_HIGH);

	regulator_enable(reg_cmc623io_1v8);

	gpio_set_value(GPIO_IMA_N_RST, GPIO_LEVEL_HIGH);
	gpio_set_value(GPIO_IMA_PWREN, GPIO_LEVEL_LOW);
	gpio_set_value(GPIO_IMA_BYPASS, GPIO_LEVEL_LOW);
	gpio_set_value(GPIO_IMA_SLEEP, GPIO_LEVEL_LOW);

#endif

	return 0;
}
EXPORT_SYMBOL(cmc623_pre_resume);

/*CAUTION : pre_resume function must be called before using this function*/
int cmc623_resume(struct early_suspend *h)
{
	if (cmc623_suspend_flag == 0) {
		printk(KERN_INFO "cmc623_suspend is not called\n");
		return 0;
	} else {
		cmc623_suspend_flag = 0;
	}

	pr_info("-- %s called -0-\n", __func__);

	cmc623_pre_resume();

	usleep_range(1000, 2000);
	/* FAILSAFEB <= HIGH */
	gpio_set_value(GPIO_IMA_PWREN, GPIO_LEVEL_HIGH);
	usleep_range(1000, 2000);

	/* BYPASSB <= HIGH*/
	gpio_set_value(GPIO_IMA_BYPASS, GPIO_LEVEL_HIGH);
	usleep_range(1000, 2000);

	/* SLEEPB <= HIGH*/
	gpio_set_value(GPIO_IMA_SLEEP, GPIO_LEVEL_HIGH);
	usleep_range(1000, 2000);

	/* RESETB <= LOW*/
	gpio_set_value(GPIO_IMA_N_RST, GPIO_LEVEL_LOW);

	/* wait 4ms or above*/
	usleep_range(5000, 7000);

	/* RESETB(K6) <= HIGH*/
	gpio_set_value(GPIO_IMA_N_RST, GPIO_LEVEL_HIGH);

	/* wait 0.5ms or above*/
	usleep_range(5000, 7000);

#if USE_IMACONV_CMC623 //yd.seo tmp
	cmc623_state.suspended = FALSE;

	if (!p_cmc623_data) {
		pr_err("%s cmc623 is not initialized\n", __func__);
		return 0;
	}

	CMC623_SetUp();
#if !defined(CMC623_SERVICE_EXTEND) //yd.seo check this point
	CMC623_Set_Mode();
#if BYPASS_ONOFF_TEST
	if(current_bypass_onoff)
	{
		bypass_onoff_ctrl(current_bypass_onoff);
		cmc623_pwm_brightness_bypass(current_gamma_level);
	}
	else
		cmc623_pwm_brightness(current_gamma_level);
#else //BYPASS_ONOFF_TEST
	cmc623_pwm_brightness(current_gamma_level);
#endif //BYPASS_ONOFF_TEST
#else  // CMC623_SERVICE_EXTEND
	cmc623_service_resume();
#endif  //CMC623_SERVICE_EXTEND
#else //USE_IMACONV_CMC623
	// wait 16ms or above
	msleep(16);
	gpio_set_value(GPIO_IMA_BYPASS, GPIO_LEVEL_LOW);
#endif //USE_IMACONV_CMC623

	//gpio_set_value(GPIO_LCD_LDO_LED_EN, GPIO_LEVEL_HIGH); //yd.seo check

	lcdonoff = TRUE;


#if 0
	/* restore mode & cabc status*/
	setting_first = TRUE;
	cmc623_state.brightness = 0;
	cmc623_cabc_enable(cmc623_state.cabc_enabled);
	setting_first = FALSE;

	usleep_range(10000, 20000);
#endif

	printk(KERN_INFO "-- end of %s : %d\n", __func__, __LINE__);

	return 0;
}
EXPORT_SYMBOL(cmc623_resume);
#endif

int cmc623_shutdown(struct i2c_client *client)
{
	pr_info("-0- %s called -0-\n", __func__);

	cmc623_state.suspended = TRUE;
	lcdonoff = FALSE;

	if (!p_cmc623_data) {
		pr_err("%s cmc623 is not initialized\n", __func__);
		return 0;
	}

	/* 1.2V/1.8V/3.3V may be on

	CMC623[0x07] := 0x0004
	cmc623_I2cWrite16(0x07, 0x0004);*/

	/* CMC623 SLEEPB <= LOW*/
	gpio_set_value(GPIO_IMA_SLEEP, 0);

	/* CMC623 BYPASSB <= LOW*/
	gpio_set_value(GPIO_IMA_BYPASS, 0);

	/* wait 1ms*/
	usleep_range(1000, 2000);

	/* CMC623 FAILSAFEB <= LOW*/
	gpio_set_value(GPIO_IMA_PWREN, 0);

	usleep_range(1000, 2000);


	return 0;
}

#ifdef CABC_ONOFF_TEST
static int current_cabc_onoff = 1;

static ssize_t cabc_onoff_file_cmd_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	pr_info("called %s\n", __func__);

	return sprintf(buf,"%u\n",current_cabc_onoff);
}

static ssize_t cabc_onoff_file_cmd_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);

	pr_info("[cabc set] in cabc_onoff_file_cmd_store,"
	        " input value = %d\n", value);
#if BYPASS_ONOFF_TEST
	if((cmc623_state.suspended == FALSE) && (lcdonoff == TRUE)&&(current_bypass_onoff == FALSE))
	{
		if((current_cabc_onoff==0) && (value == 1))
		{
			cabc_onoff_ctrl(value);
			current_cabc_onoff = 1;
			cmc623_state.cabc_enabled = TRUE;
			printk(KERN_INFO "[cabc_on] <= value : %d \n",value);
			cmc623_pwm_brightness(current_gamma_level);
		}
		else if((current_cabc_onoff==1) &&(value == 0))
		{
			cabc_onoff_ctrl(value);
			current_cabc_onoff = 0;
			cmc623_state.cabc_enabled = FALSE;
			printk(KERN_INFO "[cabc_off] <= value : %d \n",value);
			cmc623_pwm_brightness(current_gamma_level);
		}
		else
		{
			printk(KERN_INFO "[cabc set] cabc is already = %d \n",current_cabc_onoff);
		}
	}
	else
	{
		printk(KERN_INFO "[cabc set] LCD is suspend = %d \n",cmc623_state.suspended);
	}
#else // BYPASS_ONOFF_TEST
	if ((cmc623_state.suspended == FALSE) && (lcdonoff == TRUE)) {
		if ((current_cabc_onoff == 0) && (value == 1)) {
			cabc_onoff_ctrl(value);
			current_cabc_onoff = 1;
			cmc623_state.cabc_enabled = TRUE;
			pr_info("[cabc_on] <= value : %d\n", value);
			cmc623_pwm_brightness(current_gamma_level);
		} else if ((current_cabc_onoff == 1) && (value == 0)) {
			cabc_onoff_ctrl(value);
			current_cabc_onoff = 0;
			cmc623_state.cabc_enabled = FALSE;
			pr_info("[cabc_off] <= value : %d\n", value);
			cmc623_pwm_brightness(current_gamma_level);
		} else
			pr_info("[cabc set] cabc is already = %d\n",
				current_cabc_onoff);
	} else
		pr_info("[cabc set] LCD is suspend = %d\n",
				cmc623_state.suspended);

#endif //BYPASS_ONOFF_TEST
	return size;
}

static DEVICE_ATTR(cabconoff, 0666, cabc_onoff_file_cmd_show,
			cabc_onoff_file_cmd_store);
#endif

#if BYPASS_ONOFF_TEST
static void cmc623_reinit()
{
	// FAILSAFEB <= HIGH
	gpio_set_value(GPIO_IMA_PWREN, GPIO_LEVEL_HIGH);
	usleep_range(1000, 2000);

	// BYPASSB <= HIGH
	gpio_set_value(GPIO_IMA_BYPASS, GPIO_LEVEL_HIGH);
	//gpio_set_value(GPIO_IMA_BYPASS, GPIO_LEVEL_LOW);
	usleep_range(1000, 2000);

	// SLEEPB <= HIGH
	gpio_set_value(GPIO_IMA_SLEEP, GPIO_LEVEL_HIGH);
	usleep_range(1000, 2000);

	// RESETB <= LOW
	gpio_set_value(GPIO_IMA_N_RST, GPIO_LEVEL_LOW);

	// wait 4ms or above
	usleep_range(5000, 7000);

	// RESETB(K6) <= HIGH
	gpio_set_value(GPIO_IMA_N_RST, GPIO_LEVEL_HIGH);

	// wait 0.3ms or above
	usleep_range(1000, 2000);

	cmc623_state.suspended = FALSE;

	// set registers using I2C
	if(!p_cmc623_data)
	{
		printk(KERN_ERR "%s cmc623 is not initialized\n", __func__);
		return 0;
	}

	CMC623_SetUp();

	lcdonoff = TRUE;

}

void bypass_onoff_ctrl(int value)
{
	if(value == 1)
	{
		gpio_set_value(GPIO_IMA_BYPASS, 0); //GPIO bypass pin low <== bypass on
#if 0
		cmc623_reinit();

		int i = 0;
		int num = ARRAY_SIZE(cmc623_bypass);

		for (i=0; i<num; i++)
		{
			cmc623_I2cWrite16(cmc623_bypass[i].RegAddr, cmc623_bypass[i].Data);
		}
		cmc623_pwm_brightness_bypass(current_gamma_level);
		printk(KERN_INFO "[bypass_onoff] = bypass_on : %d \n",value);
#endif
	}
	else if(value == 0)
	{

		gpio_set_value(GPIO_IMA_BYPASS, 1);
#if 0
		cabc_onoff_ctrl(current_cabc_onoff);
		cmc623_state.cabc_enabled = current_cabc_onoff;
		current_bypass_onoff = 0;
		cmc623_pwm_brightness(current_gamma_level);
		printk(KERN_INFO "[bypass_onoff] = bypass_off : %d \n",value);
#endif
	}
}

static ssize_t bypass_onoff_file_cmd_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	pr_info("called %s \n",__func__);

	return sprintf(buf,"%u\n",current_bypass_onoff);
}

static ssize_t bypass_onoff_file_cmd_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);

	printk(KERN_INFO "[bypass set] in bypass_onoff_file_cmd_store, input value = %d \n",value);

	if((cmc623_state.suspended == FALSE) && (lcdonoff == TRUE))
	{
		if((current_bypass_onoff==0) && (value == 1))
		{
			bypass_onoff_ctrl(value);
			current_bypass_onoff = 1;
		}
		else if((current_bypass_onoff==1) &&(value == 0))
		{
			bypass_onoff_ctrl(value);
			current_bypass_onoff = 0;
		}
		else
		{
			printk(KERN_INFO "[bypass set] bypass is already = %d \n",current_bypass_onoff);
		}
	}
	else
	{
		printk(KERN_INFO "[bypass set] LCD is suspend = %d \n",cmc623_state.suspended);
	}

	return size;
}

static DEVICE_ATTR(bypassonoff, 0666, bypass_onoff_file_cmd_show, bypass_onoff_file_cmd_store);

#endif // BYPASS_ONOFF_TEST

static int cmc623_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct cmc623_data *data;
	int ret;

	pr_info("==============================\n");
	pr_info("cmc623 attach START!!!\n");
	pr_info("==============================\n");

	data = kzalloc(sizeof(struct cmc623_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
    }

	data->client = client;
	i2c_set_clientdata(client, data);

	dev_info(&client->dev, "cmc623 i2c probe success!!!\n");

	p_cmc623_data = data;

	panel_gpio_init();
	cmc623_gpio_init();

#if 0
#if USE_IMACONV_CMC623 //yd.seo
	if(!p_cmc623_data){
		pr_err("%s cmc623 is not initialized\n", __func__);
		return 0;
		}


	cmc623_state.suspended = FALSE;

	/* set registers using I2C*/
	ret = CMC623_SetUp();
	pr_err("%s CMC623_SetUp is %d\n", __func__, ret);
	CMC623_Set_Mode();
        //pr_err("%s CMC623_Set_Mode is %d\n", __func__, ret);

	// BL_EN HIGH
	//gpio_set_value(GPIO_BL_RESET, GPIO_LEVEL_HIGH); //yd.seo tmp
#else
	msleep(16);
	//USE_IMACONV_CMC623 for BYPASS
	ret = gpio_direction_output(GPIO_IMA_BYPASS, 0);
#endif
#endif

#if 0
	/* restore mode & cabc status*/
	setting_first = TRUE;
	cmc623_state.brightness = 0;
	cmc623_cabc_enable(cmc623_state.cabc_enabled);
	setting_first = FALSE;

	usleep_range(10000, 20000);
#endif

#ifdef CMC623_TUNING
	cmc623_set_tuning();	/*for test*/
#endif

#if 0
	setting_first = TRUE;
	/*cmc623_cabc_enable(cmc623_state.cabc_enabled);*/
	setting_first = FALSE;

	/*ret = cmc623_gamma_set();*/
	/*pr_info("cmc623_gamma_set Return value  (%d)\n", ret);*/

	/*cmc623_i2c_client = c;*/
#endif

#if 0
#ifdef CONFIG_HAS_EARLYSUSPEND
	cmc623_early_suspend.level =  EARLY_SUSPEND_LEVEL_DISABLE_FB-2;
	cmc623_early_suspend.suspend = cmc623_suspend;
	cmc623_early_suspend.resume = cmc623_resume;
	register_early_suspend(&cmc623_early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */
#endif
	return 0;
}

static int __devexit cmc623_i2c_remove(struct i2c_client *client)
{
	struct cmc623_data *data = i2c_get_clientdata(client);

	p_cmc623_data = NULL;

	i2c_set_clientdata(client, NULL);

	kfree(data);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&cmc623_early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

	dev_info(&client->dev, "cmc623 i2c remove success!!!\n");

	return 0;
}

static const struct i2c_device_id cmc623[] = {
	{ "image_convertor", 0 },
};
MODULE_DEVICE_TABLE(i2c, cmc623);

struct i2c_driver cmc623_i2c_driver = {
	.driver	= {
	.name	= "image_convertor",
        .owner = THIS_MODULE,
	},
	.probe 		= cmc623_i2c_probe,
	.remove 	= __devexit_p(cmc623_i2c_remove),
	.id_table	= cmc623,
#if !(defined CONFIG_HAS_EARLYSUSPEND)
    .suspend = cmc623_suspend,
	.resume  = cmc623_resume,
#endif
	.shutdown = cmc623_shutdown,
};

extern struct class *sec_class;
struct device *tune_cmc623_dev;

static int __devinit cmc623_probe(struct platform_device *pdev)
{
	int ret=0;

#if 0
	cmc623_regulator_csi = regulator_get(&pdev->dev, "vdd_sm2");
	if (IS_ERR_OR_NULL(cmc623_regulator_csi))
		pr_err("cmc623_probe: Couldn't get regulator sm\n");
#endif

	/*sec_class = class_create(THIS_MODULE, "sec_tune_cmc623");*/

	pr_info("**** < cmc623_probe >      ***********\n");


	reg_cmc623io_1v8 = regulator_get(NULL, "VCMC623_IO_1V8");
	if (IS_ERR(reg_cmc623io_1v8)) {
		printk(KERN_INFO "%s: VCMC623_IO_1V8 regulator not found\n", __func__);
			reg_cmc623io_1v8 = NULL;
	} else {
		regulator_set_voltage(reg_cmc623io_1v8, 1800*1000,1800*1000);
	}


	tune_cmc623_dev = device_create(sec_class, NULL, 0, NULL,
					"sec_tune_cmc623");

	if (IS_ERR(tune_cmc623_dev)) {
		pr_err("Failed to create device!");
		ret = -1;
	}
#if CABC_ONOFF_TEST
	if (device_create_file(tune_cmc623_dev, &dev_attr_cabconoff) < 0) {
		pr_err("Failed to create device file!(%s)!\n",
			dev_attr_cabconoff.attr.name);
		ret = -1;
	}
#endif

#if BYPASS_ONOFF_TEST
	if (device_create_file(tune_cmc623_dev, &dev_attr_bypassonoff) < 0) {
		pr_err("Failed to create device file!(%s)!\n", dev_attr_bypassonoff.attr.name);
		ret = -1;
	}
#endif
	pr_info("<cmc623_i2c_driver Add START>\n");
	ret = i2c_add_driver(&cmc623_i2c_driver);
	pr_info("cmc623_init Return value  (%d)\n", ret);
/*
	ove_wq = create_singlethread_workqueue("ove_wq");
	INIT_WORK(&work_ove, ove_workqueue_func);
*/
#if !defined(CMC623_SERVICE_EXTEND)
	cabc_onoff_ctrl(TRUE); //yd.seo check this point
#else
	init_cmc623_service(p_cmc623_data);
#endif
	cmc623_state.suspended = FALSE;
	lcdonoff = TRUE;

	pr_info("<cmc623_i2c_driver Add END>\n");

	return ret;
}

static int __devexit cmc623_remove(struct platform_device *pdev)
{
	if (ove_wq)
		destroy_workqueue(ove_wq);

	i2c_del_driver(&cmc623_i2c_driver);

	return 0;
}

struct platform_driver sec_cmc623 =  {
	.driver = {
		.name = "sec_cmc623",
		.owner  = THIS_MODULE,
	},
	.probe  = cmc623_probe,
	.remove = cmc623_remove,
};

static int __init cmc623_init(void)
{
	pr_info("**** < cmc623_init  > *****\n");

	return platform_driver_register(&sec_cmc623);
}

static void __exit cmc623_exit(void)
{
	platform_driver_unregister(&sec_cmc623);
}

module_init(cmc623_init);
module_exit(cmc623_exit);

/* Module information */
MODULE_AUTHOR("Samsung");
MODULE_DESCRIPTION("Tuning CMC623 image converter");
MODULE_LICENSE("GPL");
