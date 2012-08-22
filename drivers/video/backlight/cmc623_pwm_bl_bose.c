/*
 *  cmc623_pwm Backlight Driver based on SWI Driver.
 *
 *  Copyright (c) 2009 Samsung Electronics
 *  InKi Dae <inki.dae@samsung.com>
 *
 *  Based on Sharp's Corgi Backlight Driver
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>

#include <mach/hardware.h>
#include <mach/gpio.h>

#include "../../../arch/arm/mach-tegra/cpu-tegra.h"

#define CMC623_PWM_MAX_INTENSITY		255
#define CMC623_PWM_DEFAULT_INTENSITY	103
#define MAX_LEVEL			1600

// brightness tuning
#define MAX_BRIGHTNESS_LEVEL 255
#define MID_BRIGHTNESS_LEVEL 103
#define LOW_BRIGHTNESS_LEVEL 10
#define DIM_BRIGHTNESS_LEVEL 10

#define MAX_BACKLIGHT_VALUE 1600 	/* 100%*/
#define MID_BACKLIGHT_VALUE 640  	/*40%*/
#define LOW_BACKLIGHT_VALUE 64
#define DIM_BACKLIGHT_VALUE 64


#define GAMMA_22 0
#define GAMMA_19 1

#if defined (CONFIG_MACH_BOSE_ATT)
#define DIM_BL	20
#define MIN_BL	30
#define MAX_BL	255
#define MAX_GAMMA_VALUE	24	// we have 25 levels. -> 16 levels -> 24 levels
#define CRITICAL_BATTERY_LEVEL 5
#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define SUPPORT_ACL_SET

extern int n1_spi_write(u8 addr, u8 data);
#else
extern void cmc623_pwm_apply(int value);
extern int cmc623_suspend(void);
extern int cmc623_resume(void);
#endif


static struct early_suspend	st_early_suspend;
static struct platform_device *bl_pdev;

static int s_backlight_level = MID_BACKLIGHT_VALUE;
static int s_gamma_mode = 0 ;
static int s_acl_level = 0 ;
static int s_acl_enable = 0 ;


static int cmc623_pwm_suspended;
static int current_intensity;
//static DEFINE_SPINLOCK(cmc623_pwm_lock);
static DEFINE_MUTEX(cmc623_pwm_mutex);

#if defined (CONFIG_MACH_BOSE_ATT)
const unsigned short s6e63m0_22gamma_300cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x170,	0x16E,	0x14E,	0x1BC,
	0x1C0,	0x1AF,	0x1B3,	0x1B8,	0x1A5,	0x1C5,	0x1C7,	0x1BB,
	0x100,	0x1B9,	0x100,	0x1B8,	0x100,	0x1FC,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_290cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x171,	0x170,	0x150,	0x1BD,
	0x1C1,	0x1B0,	0x1B2,	0x1B8,	0x1A4,	0x1C6,	0x1C7,	0x1BB,
	0x100,	0x1B6,	0x100,	0x1B6,	0x100,	0x1FA,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_280cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x16E,	0x16C,	0x14D,	0x1BE,
	0x1C3,	0x1B1,	0x1B3,	0x1B8,	0x1A5,	0x1C6,	0x1C8,	0x1BB,
	0x100,	0x1B4,	0x100,	0x1B3,	0x100,	0x1F7,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_270cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x171,	0x16C,	0x150,	0x1BD,
	0x1C3,	0x1B0,	0x1B4,	0x1B8,	0x1A6,	0x1C6,	0x1C9,	0x1BB,
	0x100,	0x1B2,	0x100,	0x1B1,	0x100,	0x1F4,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_260cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x174,	0x16E,
	0x154,	0x1BD,	0x1C2,	0x1B0,	0x1B5,	0x1BA,
	0x1A7,	0x1C5,	0x1C9,	0x1BA,	0x100,	0x1B0,
	0x100,	0x1AE,	0x100,	0x1F1,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_250cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x174,	0x16D,
	0x154,	0x1BF,	0x1C3,	0x1B2,	0x1B4,	0x1BA,
	0x1A7,	0x1C6,	0x1CA,	0x1BA,	0x100,	0x1AD,
	0x100,	0x1AB,	0x100,	0x1ED,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_240cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x176,	0x16F,
	0x156,	0x1C0,	0x1C3,	0x1B2,	0x1B5,	0x1BA,
	0x1A8,	0x1C6,	0x1CB,	0x1BB,	0x100,	0x1AA,
	0x100,	0x1A8,	0x100,	0x1E9,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_230cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x175,	0x16F,
	0x156,	0x1BF,	0x1C3,	0x1B2,	0x1B6,	0x1BB,
	0x1A8,	0x1C7,	0x1CB,	0x1BC,	0x100,	0x1A8,
	0x100,	0x1A6,	0x100,	0x1E6,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_220cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x178,	0x16F,
	0x158,	0x1BF,	0x1C4,	0x1B3,	0x1B5,	0x1BB,
	0x1A9,	0x1C8,	0x1CC,	0x1BC,	0x100,	0x1A6,
	0x100,	0x1A3,	0x100,	0x1E2,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_210cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x179,	0x16D,
	0x157,	0x1C0,	0x1C4,	0x1B4,	0x1B7,	0x1BD,
	0x1AA,	0x1C8,	0x1CC,	0x1BD,	0x100,	0x1A2,
	0x100,	0x1A0,	0x100,	0x1DD,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_200cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x179,	0x16D,
	0x158,	0x1C1,	0x1C4,	0x1B4,	0x1B6,	0x1BD,
	0x1AA,	0x1CA,	0x1CD,	0x1BE,	0x100,	0x19F,
	0x100,	0x19D,	0x100,	0x1D9,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_190cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17A,	0x16D,
	0x159,	0x1C1,	0x1C5,	0x1B4,	0x1B8,	0x1BD,
	0x1AC,	0x1C9,	0x1CE,	0x1BE,	0x100,	0x19D,
	0x100,	0x19A,	0x100,	0x1D5,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_180cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17B,	0x16D,
	0x15B,	0x1C0,	0x1C5,	0x1B3,	0x1BA,	0x1BE,
	0x1AD,	0x1CA,	0x1CE,	0x1BF,	0x100,	0x199,
	0x100,	0x197,	0x100,	0x1D0,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_170cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17C,	0x16D,
	0x15C,	0x1C0,	0x1C6,	0x1B4,	0x1BB,	0x1BE,
	0x1AD,	0x1CA,	0x1CF,	0x1C0,	0x100,	0x196,
	0x100,	0x194,	0x100,	0x1CC,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_160cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17F,	0x16E,
	0x15F,	0x1C0,	0x1C6,	0x1B5,	0x1BA,	0x1BF,
	0x1AD,	0x1CB,	0x1CF,	0x1C0,	0x100,	0x194,
	0x100,	0x191,	0x100,	0x1C8,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_150cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x180,	0x16E,
	0x15F,	0x1C1,	0x1C6,	0x1B6,	0x1BC,	0x1C0,
	0x1AE,	0x1CC,	0x1D0,	0x1C2,	0x100,	0x18F,
	0x100,	0x18D,	0x100,	0x1C2,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_140cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x180,	0x16C,
	0x15F,	0x1C1,	0x1C6,	0x1B7,	0x1BC,	0x1C1,
	0x1AE,	0x1CD,	0x1D0,	0x1C2,	0x100,	0x18C,
	0x100,	0x18A,	0x100,	0x1BE,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_130cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x18C,	0x16C,
	0x160,	0x1C3,	0x1C7,	0x1B9,	0x1BC,	0x1C1,
	0x1AF,	0x1CE,	0x1D2,	0x1C3,	0x100,	0x188,
	0x100,	0x186,	0x100,	0x1B8,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_120cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x182,	0x16B,
	0x15E,	0x1C4,	0x1C8,	0x1B9,	0x1BD,	0x1C2,
	0x1B1,	0x1CE,	0x1D2,	0x1C4,	0x100,	0x185,
	0x100,	0x182,	0x100,	0x1B3,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_110cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x186,	0x16A,
	0x160,	0x1C5,	0x1C7,	0x1BA,	0x1BD,	0x1C3,
	0x1B2,	0x1D0,	0x1D4,	0x1C5,	0x100,	0x180,
	0x100,	0x17E,	0x100,	0x1AD,
	ENDDEF, 0x0000
};


const unsigned short s6e63m0_22gamma_100cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x186,	0x169,	0x160,	0x1C6,
	0x1C8,	0x1BA,	0x1BF,	0x1C4,	0x1B4,	0x1D0,	0x1D4,	0x1C6,
	0x100,	0x17C,	0x100,	0x17A,	0x100,	0x1A7,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_90cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x189,	0x169,	0x164,	0x1C7,
	0x1C8,	0x1BB,	0x1C0,	0x1C5,	0x1B4,	0x1D2,	0x1D5,	0x1C9,
	0x100,	0x177,	0x100,	0x176,	0x100,	0x1A0,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_80cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x189,	0x168,
	0x165,	0x1C9,	0x1C9,	0x1BC,	0x1C1,	0x1C5,
	0x1B6,	0x1D2,	0x1D5,	0x1C9,	0x100,	0x173,
	0x100,	0x172,	0x100,	0x19A,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_70cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x18E,	0x162,	0x16B,	0x1C7,
	0x1C9,	0x1BB,	0x1C3,	0x1C7,	0x1B7,	0x1D3,	0x1D7,	0x1CA,
	0x100,	0x16E,	0x100,	0x16C,	0x100,	0x194,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_60cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x191,	0x15E,
	0x16E,	0x1C9,	0x1C9,	0x1BD,	0x1C4,	0x1C9,
	0x1B8,	0x1D3,	0x1D7,	0x1CA,	0x100,	0x169,
	0x100,	0x167,	0x100,	0x18D,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_50cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x196,	0x158,
	0x172,	0x1CB,	0x1CA,	0x1BF,	0x1C6,	0x1C9,
	0x1BA,	0x1D6,	0x1D9,	0x1CD,	0x100,	0x161,
	0x100,	0x161,	0x100,	0x183,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_40cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x197,	0x158,	0x171,	0x1CC,
	0x1CB,	0x1C0,	0x1C5,	0x1C9,	0x1BA,	0x1D9,	0x1DC,	0x1D1,
	0x100,	0x15B,	0x100,	0x15A,	0x100,	0x17A,
	ENDDEF, 0x0000
};

const unsigned short s6e63m0_22gamma_30cd[] = {
	//gamma set
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x1A1,	0x151,	0x17B,	0x1CE,
	0x1CB,	0x1C2,	0x1C7,	0x1CB,	0x1BC,	0x1DA,	0x1DD,	0x1D3,
	0x100,	0x153,	0x100,	0x152,	0x100,	0x16F,
	ENDDEF, 0x0000
};


const unsigned short *p22Gamma_set[] = {

	s6e63m0_22gamma_30cd,//0
	s6e63m0_22gamma_40cd,
	s6e63m0_22gamma_70cd,
	s6e63m0_22gamma_90cd,
	s6e63m0_22gamma_100cd,
	s6e63m0_22gamma_110cd,  //5
	s6e63m0_22gamma_120cd,
	s6e63m0_22gamma_130cd,
	s6e63m0_22gamma_140cd,
	s6e63m0_22gamma_150cd,
	s6e63m0_22gamma_160cd,   //10
	s6e63m0_22gamma_170cd,
	s6e63m0_22gamma_180cd,
	s6e63m0_22gamma_190cd,
	s6e63m0_22gamma_200cd,
	s6e63m0_22gamma_210cd,  //15
	s6e63m0_22gamma_220cd,
	s6e63m0_22gamma_230cd,
	s6e63m0_22gamma_240cd,
	s6e63m0_22gamma_250cd,
	s6e63m0_22gamma_260cd,  //20
	s6e63m0_22gamma_270cd,
	s6e63m0_22gamma_280cd,
	s6e63m0_22gamma_290cd,
	s6e63m0_22gamma_300cd,//24
};

const unsigned short s6e63m0_19gamma_30cd[] = {
	0x0FA,
	0x102, 	0x118, 	0x108, 	0x124, 	0x19D, 	0x175, 	0x17C, 	0x1D0,
	0x1D0, 	0x1C6, 	0x1CD, 	0x1D1, 	0x1C3, 	0x1DE, 	0x1E1, 	0x1D8,
	0x100, 	0x153, 	0x100, 	0x152, 	0x100, 	0x16F,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_40cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x193,	0x177,	0x172,	0x1CF,
	0x1D0,	0x1C5,	0x1CB,	0x1CF,	0x1C1,	0x1DD,	0x1E0,	0x1D6,
	0x100,	0x15B,	0x100,	0x15A,	0x100,	0x17A,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_70cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x192,	0x17F,	0x175,	0x1CC,
	0x1CF,	0x1C2,	0x1C7,	0x1CC,	0x1BD,	0x1D8,	0x1DC,	0x1CF,
	0x100,	0x16E,	0x100,	0x16C,	0x100,	0x194,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_90cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x188,	0x181,	0x16D,	0x1CB,
	0x1CE,	0x1C0,	0x1C6,	0x1CA,	0x1BB,	0x1D6,	0x1DA,	0x1CE,
	0x100,	0x178,	0x100,	0x176,	0x100,	0x1A1,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_100cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x186,	0x181,	0x16B,	0x1CA,
	0x1CD,	0x1BE,	0x1C6,	0x1C9,	0x1BB,	0x1D5,	0x1DA,	0x1CD,
	0x100,	0x17C,	0x100,	0x17A,	0x100,	0x1A7,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_110cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x185,	0x181,	0x16A,	0x1CB,
	0x1CD,	0x1BF,	0x1C4,	0x1C8,	0x1B9,	0x1D5,	0x1D9,	0x1CC,
	0x100,	0x180,	0x100,	0x17E,	0x100,	0x1AD,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_120cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x183,	0x17F,	0x169,	0x1C9,
	0x1CC,	0x1BE,	0x1C3,	0x1C8,	0x1B8,	0x1D4,	0x1D7,	0x1CB,
	0x100,	0x185,	0x100,	0x183,	0x100,	0x1B3,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_130cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x182,	0x17F,	0x169,	0x1C8,
	0x1CC,	0x1BD,	0x1C2,	0x1C7,	0x1B6,	0x1D4,	0x1D7,	0x1CB,
	0x100,	0x188,	0x100,	0x186,	0x100,	0x1B8,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_140cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17E,	0x17F,	0x165,	0x1C8,
	0x1CC,	0x1BC,	0x1C2,	0x1C6,	0x1B6,	0x1D3,	0x1D6,	0x1C9,
	0x100,	0x18C,	0x100,	0x18A,	0x100,	0x1BE,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_150cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17E,	0x17F,	0x163,	0x1C8,
	0x1CB,	0x1BC,	0x1C1,	0x1C6,	0x1B6,	0x1D2,	0x1D6,	0x1C8,
	0x100,	0x18F,	0x100,	0x18D,	0x100,	0x1C2,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_160cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17C,	0x17E,	0x161,	0x1C7,
	0x1CB,	0x1BB,	0x1C1,	0x1C5,	0x1B5,	0x1D1,	0x1D5,	0x1C8,
	0x100,	0x193,	0x100,	0x190,	0x100,	0x1C7,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_170cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17D,	0x17F,	0x161,	0x1C7,
	0x1CA,	0x1BB,	0x1C0,	0x1C5,	0x1B5,	0x1D1,	0x1D4,	0x1C8,
	0x100,	0x196,	0x100,	0x194,	0x100,	0x1CB,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_180cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x180,	0x182,	0x165,	0x1C6,
	0x1C9,	0x1BB,	0x1BF,	0x1C4,	0x1B3,	0x1D0,	0x1D4,	0x1C6,
	0x100,	0x199,	0x100,	0x197,	0x100,	0x1D0,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_190cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17D,	0x17F,	0x163,	0x1C5,
	0x1C9,	0x1BA,	0x1BF,	0x1C4,	0x1B2,	0x1D0,	0x1D3,	0x1C6,
	0x100,	0x19C,	0x100,	0x19A,	0x100,	0x1D5,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_200cd[] = {

	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17B,	0x17E,	0x161,	0x1C5,
	0x1C9,	0x1B9,	0x1BF,	0x1C3,	0x1B2,	0x1CF,	0x1D3,	0x1C5,
	0x100,	0x19F,	0x100,	0x19D,	0x100,	0x1D9,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_210cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17B,	0x17F,	0x161,	0x1C6,
	0x1C8,	0x1B9,	0x1BE,	0x1C2,	0x1B2,	0x1CE,	0x1D3,	0x1C4,
	0x100,	0x1A2,	0x100,	0x1A0,	0x100,	0x1DD,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_220cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17C,	0x17F,	0x162,	0x1C5,
	0x1C8,	0x1B9,	0x1BC,	0x1C1,	0x1B0,	0x1CE,	0x1D2,	0x1C3,
	0x100,	0x1A5,	0x100,	0x1A3,	0x100,	0x1E2,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_230cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17A,	0x17E,	0x161,	0x1C4,
	0x1C8,	0x1B8,	0x1BB,	0x1C1,	0x1AF,	0x1CE,	0x1D1,	0x1C3,
	0x100,	0x1A8,	0x100,	0x1A6,	0x100,	0x1E6,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_240cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17A,	0x17F,	0x161,	0x1C2,
	0x1C7,	0x1B6,	0x1BC,	0x1C1,	0x1AF,	0x1CE,	0x1D0,	0x1C3,
	0x100,	0x1AB,	0x100,	0x1A9,	0x100,	0x1EA,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_250cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x178,	0x17C,	0x15E,	0x1C3,
	0x1C8,	0x1B6,	0x1BC,	0x1C0,	0x1AF,	0x1CC,	0x1CF,	0x1C2,
	0x100,	0x1AE,	0x100,	0x1AC,	0x100,	0x1EE,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_260cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x179,	0x17C,	0x15E,	0x1C3,
	0x1C7,	0x1B6,	0x1BA,	0x1BF,	0x1AE,	0x1CC,	0x1D0,	0x1C2,
	0x100,	0x1B1,	0x100,	0x1AE,	0x100,	0x1F2,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_270cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17C,	0x17E,	0x161,	0x1C1,
	0x1C6,	0x1B5,	0x1BA,	0x1BF,	0x1AD,	0x1CC,	0x1CF,	0x1C2,
	0x100,	0x1B3,	0x100,	0x1B1,	0x100,	0x1F5,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_280cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x17B,	0x17D,	0x15F,	0x1C1,
	0x1C7,	0x1B5,	0x1BA,	0x1BE,	0x1AD,	0x1CC,	0x1CE,	0x1C2,
	0x100,	0x1B5,	0x100,	0x1B4,	0x100,	0x1F8,
	ENDDEF,	0x00
};

const unsigned short s6e63m0_19gamma_290cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x178,	0x17A,	0x15B,	0x1C2,
	0x1C7,	0x1B6,	0x1BA,	0x1BE,	0x1AC,	0x1CB,	0x1CE,	0x1C2,
	0x100,	0x1B8,	0x100,	0x1B6,	0x100,	0x1FB,
	ENDDEF,	0x00

};

const unsigned short s6e63m0_19gamma_300cd[] = {
	0x0FA,
	0x102,	0x118,	0x108,	0x124,	0x179,	0x17A,	0x15B,	0x1C1,
	0x1C5,	0x1B5,	0x1B8,	0x1BD,	0x1AB,	0x1CB,	0x1CE,	0x1C1,
	0x100,	0x1B8,	0x100,	0x1B7,	0x100,	0x1FC,
	ENDDEF,	0x00
};


const unsigned short *p19Gamma_set[] = {

	s6e63m0_19gamma_30cd,//0
	s6e63m0_19gamma_40cd,
	s6e63m0_19gamma_70cd,
	s6e63m0_19gamma_90cd,
	s6e63m0_19gamma_100cd,
	s6e63m0_19gamma_110cd,  //5
	s6e63m0_19gamma_120cd,
	s6e63m0_19gamma_130cd,
	s6e63m0_19gamma_140cd,
	s6e63m0_19gamma_150cd,
	s6e63m0_19gamma_160cd,   //10
	s6e63m0_19gamma_170cd,
	s6e63m0_19gamma_180cd,
	s6e63m0_19gamma_190cd,
	s6e63m0_19gamma_200cd,
	s6e63m0_19gamma_210cd,  //15
	s6e63m0_19gamma_220cd,
	s6e63m0_19gamma_230cd,
	s6e63m0_19gamma_240cd,
	s6e63m0_19gamma_250cd,
	s6e63m0_19gamma_260cd,  //20
	s6e63m0_19gamma_270cd,
	s6e63m0_19gamma_280cd,
	s6e63m0_19gamma_290cd,
	s6e63m0_19gamma_300cd,//24
};


const unsigned short gamma_update[] = {
	//gamma update
	0x0FA,
	0x103,

//	SLEEPMSEC, 10,

	//Display on
	0x029,
	ENDDEF, 0x0000
};


static const unsigned short SEQ_ACL_ON[] = {
	0x0C1,
	0x14D,	0x196,	0x11D,	0x100,	0x100,	0x101,	0x1DF,	0x100,	0x100,
	0x103,	0x11F,	0x100,	0x100,	0x100,	0x100,	0x100,	0x101,	0x108,
	0x10E,	0x115,	0x11B,	0x122,	0x129,	0x12F,	0x136,	0x13C,	0x143,
	0x0C0,	0x101,
	ENDDEF, 0x00
};

static const unsigned short SEQ_ACL_OFF[] = {
	0x0C0, 0x100,
	ENDDEF, 0x00
};

static const unsigned short SEQ_ACL_40P[] = {
	0x0C1,
	0x14D,	0x196,	0x11D,	0x100,	0x100,	0x101,	0x1DF,	0x100,	0x100,
	0x103,	0x11F,	0x100,	0x100,	0x100,	0x100,	0x100,	0x101,	0x106,
	0x10C,	0x111,	0x116,	0x11C,	0x121,	0x126,	0x12B,	0x131,	0x136,
	0x0C0, 	0x101,
	ENDDEF, 0x00
};

static const unsigned short SEQ_ACL_43P[] = {
	0x0C1,
	0x14D,	0x196,	0x11D,	0x100,	0x100,	0x101,	0x1DF,	0x100,	0x100,
	0x103,	0x11F,	0x100,	0x100,	0x100,	0x100,	0x100,	0x101,	0x107,
	0x10C,	0x112,	0x118,	0x11E,	0x123,	0x129,	0x12F,	0x134,	0x13A,
	0x0C0, 	0x101,
	ENDDEF, 0x00
};

static const unsigned short SEQ_ACL_45P[] = {
	0x0C1,
	0x14D,	0x196,	0x11D,	0x100,	0x100,	0x101,	0x1DF,	0x100,	0x100,
	0x103,	0x11F,	0x100,	0x100,	0x100,	0x100,	0x100,	0x101,	0x107,
	0x10D,	0x113,	0x119,	0x11F,	0x125,	0x12B,	0x131,	0x137,	0x13D,
	0x0C0, 	0x101,
	ENDDEF, 0x00
};

static const unsigned short SEQ_ACL_47P[] = {
	0x0C1,
	0x14D,	0x196,	0x11D,	0x100,	0x100,	0x101,	0x1DF,	0x100,	0x100,
	0x103,	0x11F,	0x100,	0x100,	0x100,	0x100,	0x100,	0x101,	0x107,
	0x10E,	0x114,	0x11B,	0x121,	0x127,	0x12E,	0x134,	0x13B,	0x141,
	0x0C0,	0x101,
	ENDDEF, 0x00
};

static const unsigned short SEQ_ACL_48P[] = {
	0x0C1,
	0x14D,	0x196,	0x11D,	0x100,	0x100,	0x101,	0x1DF,	0x100,	0x100,
	0x103,	0x11F,	0x100,	0x100,	0x100,	0x100,	0x100,	0x101,	0x108,
	0x10E,	0x115,	0x11B,	0x122,	0x129,	0x12F,	0x136,	0x13C,	0x143,
	0x0C0,	0x101,

	ENDDEF, 0x00
};

static const unsigned short SEQ_ACL_50P[] = {
	0x0C1,
	0x14D,	0x196,	0x11D,	0x100,	0x100,	0x101,	0x1DF,	0x100,	0x100,
	0x103,	0x11F,	0x100,	0x100,	0x100,	0x100,	0x100,	0x101,	0x108,
	0x10F,	0x116,	0x11D,	0x124,	0x12A,	0x131,	0x138,	0x13F,	0x146,
	0x0C0,	0x101,
	ENDDEF, 0x00
};

static const unsigned short *ACL_cutoff_set[] = {
	SEQ_ACL_OFF,
	SEQ_ACL_40P,
	SEQ_ACL_43P,
	SEQ_ACL_45P,
	SEQ_ACL_47P,
	SEQ_ACL_48P,
	SEQ_ACL_50P,
};


static const unsigned short SEQ_SM2_ELVSS_49[] = {
	0x0B2,
	0x110,	0x110,	0x110,	0x110,
	0x0B1,
	0x10B,
	ENDDEF, 0x00
};

static const unsigned short SEQ_SM2_ELVSS_45[] = {
	0x0B2,
	0x114,	0x114,	0x114,	0x114,
	0x0B1,
	0x10B,
	ENDDEF, 0x00

};

static const unsigned short SEQ_SM2_ELVSS_42[] = {
	0x0B2,
	0x117,	0x117,	0x117,	0x117,
	0x0B1,
	0x10B,
	ENDDEF, 0x00

};

static const unsigned short SEQ_SM2_ELVSS_36[] = {
	0x0B2,
	0x11D,	0x11D,	0x11D,	0x11D,
	0x0B1,
	0x10B,
	ENDDEF, 0x00

};


static const unsigned short *SEQ_SM2_ELVSS_set[] = {
	SEQ_SM2_ELVSS_36,
	SEQ_SM2_ELVSS_42,
	SEQ_SM2_ELVSS_45,
	SEQ_SM2_ELVSS_49,
};



static int s6e63m0_panel_send_sequence(const unsigned short *wbuf)
{
	int i = 0;
	int ret = 0;

//	mutex_lock(&spi_use);
//	printk("s6e63m0_panel_send_sequence... \n");

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC){
			n1_spi_write((wbuf[i] & 0x100)>>8, wbuf[i] & 0xFF);
			i+=1;}
		else{
			msleep(wbuf[i+1]);
			i+=2;}
	}

//	mutex_unlock(&spi_use);
	return ret ;
}

static int get_gamma_value_from_bl(int bl)
{
#if 0
	int gamma_value = 0;
	int gamma_val_x10 = 0;

	if (bl >= MIN_BL)	{
		gamma_val_x10 = 10*(MAX_GAMMA_VALUE-1)*bl/(MAX_BL-MIN_BL) + (10 - 10*(MAX_GAMMA_VALUE-1)*(MIN_BL)/(MAX_BL-MIN_BL)) ;
		gamma_value = (gamma_val_x10+5)/10;
	}
	else {
		gamma_value = 0;
	}

//	printk("get_gamma_value_from_bl gamma_value = %d  !!!\n", gamma_value);

	return gamma_value;
#else
	int backlightlevel;

	/* brightness setting from platform is from 0 to 255
	 * But in this driver, brightness is only supported from 0 to 24 */

	switch(bl){
		case 0 ... 10:
			backlightlevel=0;
			break;
		case 11 ... 21:
			backlightlevel=1;
			break;
		case 22 ... 32:
			backlightlevel=2;
			break;
		case 33 ... 42:
			backlightlevel=3;
			break;
		case 43 ... 52:
			backlightlevel=4;
			break;
		case 53 ... 63:
			backlightlevel=5;
			break;
		case 64 ... 73:
			backlightlevel=6;
			break;
		case 74 ... 83:
			backlightlevel=7;
			break;
		case 84 ... 95:
			backlightlevel=8;
			break;
		case 96 ... 105:
			backlightlevel=9;
			break;
		case 106 ... 116:
			backlightlevel=10;
			break;
		case 117 ... 127:
			backlightlevel=11;
			break;
		case 128 ... 136:
			backlightlevel=12;
			break;
		case 137 ... 146:
			backlightlevel=13;
			break;
		case 147 ... 156:
			backlightlevel=14;
			break;
		case 157 ... 169:
			backlightlevel=15;
			break;
		case 170 ... 180:
			backlightlevel=16;
			break;
		case 181 ... 191:
			backlightlevel=17;
			break;
		case 192 ... 201:
			backlightlevel=18;
			break;
		case 202 ... 211:
			backlightlevel=19;
			break;
		case 212 ... 223:
			backlightlevel=20;
			break;
		case 224 ... 233:
			backlightlevel=21;
			break;
		case 234 ... 243:
			backlightlevel=22;
			break;
		case 244 ... 249:
			backlightlevel=23;
			break;
		case 250 ... 255:
			backlightlevel=24;
			break;
		default :
			break;
	}
	return backlightlevel;


#endif
}

static int s6e63m0_set_elvss(int level)
{
	int ret = 0;
//	printk("[AMOLED] s6e63m0_set_elvss..%d ~.\n",level);

	switch (level)
	{
		case 0 ... 4: /* 30cd ~ 100cd */
			ret = s6e63m0_panel_send_sequence(SEQ_SM2_ELVSS_set[0]);
			break;
		case 5 ... 10: /* 110cd ~ 160cd */
			ret = s6e63m0_panel_send_sequence(SEQ_SM2_ELVSS_set[1]);
			break;
		case 11 ... 14: /* 170cd ~ 200cd */
			ret = s6e63m0_panel_send_sequence(SEQ_SM2_ELVSS_set[2]);
			break;
		case 15 ... 24: /* 210cd ~ 300cd */
			ret = s6e63m0_panel_send_sequence(SEQ_SM2_ELVSS_set[3]);
			break;
		default:
			break;
	}

	if (ret) {
		return -EIO;
	}
	return ret;

}

static int s6e63m0_set_acl(int acl_enable)
{
	int ret = 0;

//	printk("[AMOLED] s6e63m0_set_acl..ACL %d ~.\n",acl_enable);

	if (acl_enable) {
		if((s_acl_level == 0)&&(s_backlight_level!=0)&&(s_backlight_level!=1))  {
			ret = s6e63m0_panel_send_sequence(SEQ_ACL_ON);
		}
		switch (s_backlight_level) {
		case 0 ... 1: /* 30cd ~ 40cd */
			if (s_acl_level != 0) {
				ret = s6e63m0_panel_send_sequence(ACL_cutoff_set[0]);
				s_acl_level = 0;
//				printk("[AMOLED] ACL LEVEL %d , BL LEVEL %d~.\n",s_acl_level, s_backlight_level);
			}
			break;
		case 2 ... 12: /* 70cd ~ 180cd */
			if (s_acl_level != 40) {
				ret = s6e63m0_panel_send_sequence(ACL_cutoff_set[1]);
				s_acl_level = 40;
//				printk("[AMOLED] ACL LEVEL %d , BL LEVEL %d~.\n",s_acl_level, s_backlight_level);
			}
			break;
		case 13: /* 190cd */
			if (s_acl_level != 43) {
				ret = s6e63m0_panel_send_sequence(ACL_cutoff_set[2]);
				s_acl_level = 43;
//				printk("[AMOLED] ACL LEVEL %d , BL LEVEL %d~.\n",s_acl_level, s_backlight_level);
			}
			break;
		case 14: /* 200cd */
			if (s_acl_level != 45) {
				ret = s6e63m0_panel_send_sequence(ACL_cutoff_set[3]);
				s_acl_level = 45;
//				printk("[AMOLED] ACL LEVEL %d , BL LEVEL %d~.\n",s_acl_level, s_backlight_level);
			}
			break;
		case 15: /* 210cd */
			if (s_acl_level != 47) {
				ret = s6e63m0_panel_send_sequence(ACL_cutoff_set[4]);
				s_acl_level = 47;
//				printk("[AMOLED] ACL LEVEL %d , BL LEVEL %d~.\n",s_acl_level, s_backlight_level);
			}
			break;
		case 16: /* 220cd */
			if (s_acl_level != 48) {
				ret = s6e63m0_panel_send_sequence(ACL_cutoff_set[5]);
				s_acl_level = 48;
//				printk("[AMOLED] ACL LEVEL %d , BL LEVEL %d~.\n",s_acl_level, s_backlight_level);
			}
			break;
		default:
			if (s_acl_level != 50) {
				ret = s6e63m0_panel_send_sequence(ACL_cutoff_set[6]);
				s_acl_level = 50;
//				printk("[AMOLED] ACL LEVEL %d , BL LEVEL %d~.\n",s_acl_level, s_backlight_level);
			}
			break;
		}
	}
	else{
			ret = s6e63m0_panel_send_sequence(ACL_cutoff_set[0]);
			s_acl_level = 0;
//			printk("[AMOLED] ACL LEVEL %d , BL LEVEL %d~.\n",s_acl_level, s_backlight_level);

	}

	if (ret) {
		return -EIO;
	}
	return ret;
}

void s6e63m0_set_backlight(int gamma, int level)
{

	s6e63m0_set_elvss(level);

	s6e63m0_set_acl(s_acl_enable);

	if (gamma==GAMMA_22) {
		s6e63m0_panel_send_sequence(p22Gamma_set[level]);
		s6e63m0_panel_send_sequence(gamma_update);
	} else if (gamma==GAMMA_19){
		s6e63m0_panel_send_sequence(p19Gamma_set[level]);
		s6e63m0_panel_send_sequence(gamma_update);
	}
}

extern int muxtex_temp;

#endif


#ifdef SUPPORT_ACL_SET

static ssize_t acl_set_store(struct device *dev, struct
device_attribute *attr, const char *buf, size_t size)
{
	int value;
	int rc;

	printk("[AMOLED] acl_set_store.. %s\n", buf);

	rc = strict_strtoul(buf, (unsigned int) 0, (unsigned long *)&value);
	if (rc < 0)
		{
		printk("[AMOLED] return %d \n", rc);
		return rc;

		}

	s_acl_enable = value;

	s6e63m0_set_acl(s_acl_enable);

	return size;

}

static ssize_t gamma_mode_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	char temp[10];

	switch (s_gamma_mode) {
	case 0:
		sprintf(temp, "2.2 mode\n");
		strcat(buf, temp);
		break;
	case 1:
		sprintf(temp, "1.9 mode\n");
		strcat(buf, temp);
		break;
	default:
		dev_info(dev, "gamma mode could be 0:2.2, 1:1.9 or 2:1.7)n");
		break;
	}

	return strlen(buf);
}

static ssize_t gamma_mode_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t len)
{
	int rc;
	rc = strict_strtoul(buf, 0, (unsigned long *)&s_gamma_mode);

	if (rc < 0)
		return rc;

	if (s_gamma_mode > 1)
	{
		s_gamma_mode = GAMMA_22 ;
		dev_err(dev, "there are only 2 types of gamma mode(0:2.2, 1:1.9)\n");
	}
	else
		dev_info(dev, "%s :: gamma_mode=%d\n", __FUNCTION__, s_gamma_mode);

	printk("gamma_mode_store : gamma = %d , blevel= %d \n", s_gamma_mode, s_backlight_level);

	if (muxtex_temp!=2)
		return ;

	s6e63m0_set_backlight(s_gamma_mode, s_backlight_level);

	return len;
}

static DEVICE_ATTR(acl_set, 0664, NULL, acl_set_store);
static DEVICE_ATTR(gamma_mode, 0664, gamma_mode_show, gamma_mode_store);

#endif



static void cmc623_pwm_apply_brightness(struct platform_device *pdev, int level)
{
	s_backlight_level = level;

	if (muxtex_temp!=2)
		return ;

	s6e63m0_set_backlight(s_gamma_mode, s_backlight_level);

//	dev_dbg(&pdev->dev, "%s : apply_level=%d\n", __func__, level);
//	printk("%s : apply_level=%d\n", __func__, level);
}

void amoled_set_backlight_in_resume(void)
{
	pr_info("amoled_set_backlight_in_resume : gamma %d bl: %d\n",
		s_gamma_mode, s_backlight_level);

	s6e63m0_set_backlight(s_gamma_mode, s_backlight_level);
	muxtex_temp = 2 ;
}

EXPORT_SYMBOL(amoled_set_backlight_in_resume);


static void cmc623_pwm_backlight_ctl(struct platform_device *pdev, int intensity)
{
	int tune_level;

#if defined (CONFIG_MACH_BOSE_ATT)
	tune_level = get_gamma_value_from_bl(intensity);
#else
	// brightness tuning
	if (intensity >= MID_BRIGHTNESS_LEVEL)
		tune_level = (intensity - MID_BRIGHTNESS_LEVEL) * (MAX_BACKLIGHT_VALUE-MID_BACKLIGHT_VALUE) / (MAX_BRIGHTNESS_LEVEL-MID_BRIGHTNESS_LEVEL) + MID_BACKLIGHT_VALUE;
	else if (intensity >= LOW_BRIGHTNESS_LEVEL)
		tune_level = (intensity - LOW_BRIGHTNESS_LEVEL) * (MID_BACKLIGHT_VALUE-LOW_BACKLIGHT_VALUE) / (MID_BRIGHTNESS_LEVEL-LOW_BRIGHTNESS_LEVEL) + LOW_BACKLIGHT_VALUE;
	else if (intensity >= DIM_BRIGHTNESS_LEVEL)
		tune_level = (intensity - DIM_BRIGHTNESS_LEVEL) * (LOW_BACKLIGHT_VALUE-DIM_BACKLIGHT_VALUE) / (LOW_BRIGHTNESS_LEVEL-DIM_BRIGHTNESS_LEVEL) + DIM_BACKLIGHT_VALUE;
	else if (intensity > 0)
		tune_level = DIM_BACKLIGHT_VALUE;
	else
		tune_level = intensity;
#endif

//	pr_info("  [Backlight Control] %d (%d)\n", intensity, tune_level);

	cmc623_pwm_apply_brightness(pdev, tune_level);
}

static void cmc623_pwm_send_intensity(struct backlight_device *bd)
{
	//unsigned long flags;
	int intensity = bd->props.brightness;
	struct platform_device *pdev = NULL;

	pdev = dev_get_drvdata(&bd->dev);
	if (pdev == NULL)
		{
		printk(KERN_ERR "%s:failed to get platform device.\n", __func__);
		return;
	}
#if 0
	if (bd->props.power != FB_BLANK_UNBLANK ||
		bd->props.fb_blank != FB_BLANK_UNBLANK ||
		cmc623_pwm_suspended)
		{
		printk("[cmc]i:%d(c:%d)\n", intensity, current_intensity);
		if (!current_intensity)
			return;
		msleep(1);
		intensity = 0;
	}
#endif
	//spin_lock_irqsave(&cmc623_pwm_lock, flags);
	//spin_lock(&cmc623_pwm_lock);
	mutex_lock(&cmc623_pwm_mutex);

	cmc623_pwm_backlight_ctl(pdev, intensity);

	//spin_unlock_irqrestore(&cmc623_pwm_lock, flags);
	//spin_unlock(&cmc623_pwm_lock);
	mutex_unlock(&cmc623_pwm_mutex);

	current_intensity = intensity;
}

static void cmc623_pwm_gpio_init()
{
//	s3c_gpio_cfgpin(GPIO_LCD_CABC_PWM_R05, S3C_GPIO_SFN(3));	//mdnie pwm
//    s3c_gpio_setpull(GPIO_LCD_CABC_PWM_R05, S3C_GPIO_PULL_NONE);
}

#ifdef CONFIG_PM
static int cmc623_pwm_suspend(struct platform_device *swi_dev, pm_message_t state)
{
	struct backlight_device *bd = platform_get_drvdata(swi_dev);

	cmc623_pwm_suspended = 1;
	cmc623_pwm_send_intensity(bd);

	return 0;
}

static int cmc623_pwm_resume(struct platform_device *swi_dev)
{
	struct backlight_device *bd = platform_get_drvdata(swi_dev);

	bd->props.brightness = CMC623_PWM_DEFAULT_INTENSITY;
	cmc623_pwm_suspended = 0;
	cmc623_pwm_send_intensity(bd);

	return 0;
}
#else
#define cmc623_pwm_suspend		NULL
#define cmc623_pwm_resume		NULL
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void cmc623_pwm_early_suspend(struct early_suspend *h)
{
	struct backlight_device *bd = platform_get_drvdata(bl_pdev);

	cmc623_pwm_suspended = 1;
	cmc623_pwm_send_intensity(bd);

	return 0;
}

static void cmc623_pwm_early_resume(struct early_suspend *h)
{
	struct backlight_device *bd = platform_get_drvdata(bl_pdev);

	//bd->props.brightness = cmc623_pwm_DEFAULT_INTENSITY;
	cmc623_pwm_suspended = 0;

	cmc623_pwm_gpio_init();

	cmc623_pwm_send_intensity(bd);
	return 0;
}
#endif

static int cmc623_pwm_set_intensity(struct backlight_device *bd)
{
	tegra_cpu_lock_speed(1000000, 0);

	pr_info("BD->PROPS.BRIGHTNESS = %d\n", bd->props.brightness);

	cmc623_pwm_send_intensity(bd);

	tegra_cpu_unlock_speed();

	return 0;
}


static int cmc623_pwm_get_intensity(struct backlight_device *bd)
{
	return current_intensity;
}


static struct backlight_ops cmc623_pwm_ops = {
	.get_brightness = cmc623_pwm_get_intensity,
	.update_status  = cmc623_pwm_set_intensity,
};

//for measuring luminance
void cmc623_pwm_set_brightness(int brightness)
{
	//unsigned long flags;

	pr_info("%s: value=%d\n", __func__, brightness);

	//spin_lock_irqsave(&cmc623_pwm_lock, flags);
	//spin_lock(&cmc623_pwm_lock);
	mutex_lock(&cmc623_pwm_mutex);

	cmc623_pwm_apply_brightness(bl_pdev, brightness);

	//spin_unlock_irqrestore(&cmc623_pwm_lock, flags);
	//spin_unlock(&cmc623_pwm_lock);
	mutex_unlock(&cmc623_pwm_mutex);
}
EXPORT_SYMBOL(cmc623_pwm_set_brightness);

static int cmc623_pwm_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct backlight_properties props;
	struct backlight_device *bd;

	pr_info("cmc623_pwm Probe START!!!\n");

	props.type = BACKLIGHT_RAW;
	bd = backlight_device_register("pwm-backlight", &pdev->dev, pdev, &cmc623_pwm_ops, &props);

	if (IS_ERR(bd))
		return PTR_ERR(bd);

	platform_set_drvdata(pdev, bd);

	bd->props.max_brightness = CMC623_PWM_MAX_INTENSITY;
	bd->props.brightness = CMC623_PWM_DEFAULT_INTENSITY;

	cmc623_pwm_gpio_init();

//	s5p_mdine_pwm_enable(1);

//	cmc623_pwm_set_intensity(bd);

	dev_info(&pdev->dev, "cmc623_pwm backlight driver is enabled.\n");


#ifdef SUPPORT_ACL_SET
	ret = device_create_file(&(pdev->dev), &dev_attr_acl_set);
	if (ret < 0)
		dev_err(&(pdev->dev), "failed to add sysfs entries\n");

	ret = device_create_file(&(pdev->dev), &dev_attr_gamma_mode);
	if (ret < 0)
		dev_err(&(pdev->dev), "failed to add sysfs entries\n");

#endif

	bl_pdev = pdev;

#ifdef CONFIG_HAS_EARLYSUSPEND
	//st_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	st_early_suspend.level =	EARLY_SUSPEND_LEVEL_DISABLE_FB;
	st_early_suspend.suspend = cmc623_pwm_early_suspend;
	st_early_suspend.resume = cmc623_pwm_early_resume;
	register_early_suspend(&st_early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

	pr_info("cmc623_pwm Probe END!!!\n");
	return 0;

}

static int cmc623_pwm_remove(struct platform_device *pdev)
{
	struct backlight_device *bd = platform_get_drvdata(pdev);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&st_early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

	bd->props.brightness = 0;
	bd->props.power = 0;
	cmc623_pwm_send_intensity(bd);

	backlight_device_unregister(bd);

	return 0;
}

static struct platform_driver cmc623_pwm_driver = {
	.driver		= {
		.name	= "cmc623_pwm_bl",
		.owner	= THIS_MODULE,
	},
	.probe		= cmc623_pwm_probe,
	.remove		= cmc623_pwm_remove,
#if !(defined CONFIG_HAS_EARLYSUSPEND)
	.suspend	= cmc623_pwm_suspend,
	.resume		= cmc623_pwm_resume,
#endif
};

static int __init cmc623_pwm_init(void)
{
	return platform_driver_register(&cmc623_pwm_driver);
}

static void __exit cmc623_pwm_exit(void)
{
	platform_driver_unregister(&cmc623_pwm_driver);
}

module_init(cmc623_pwm_init);
module_exit(cmc623_pwm_exit);
