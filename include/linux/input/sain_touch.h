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

#ifndef __LINUX_SAIN_TOUCH_H
#define __LINUX_SAIN_TOUCH_H

#define SAIN_BUTTON_KEY_NUM_MAX	10

/* Board specific touch screen initial values */
struct sain_touch_platform_data {
	const char *platform_name;
	u32	irq_gpio;
	u32	led_ldo_en1;
	u32	led_ldo_en2;

	const char *reg_vdd_name;
	const char *reg_avdd_name;
	const char *reg_vdd_lvsio_name;
	
	struct regulator *reg_vdd;  /* TSP_VDD_1V8 */
	struct regulator *reg_avdd;  /* TSP_AVDD_3V3 */
	struct regulator *reg_vdd_lvsio;	/* TSP_VDD_LVSIO_2V8 (for level shifter) */

	int	reg_vdd_level;	/* uV range */
	int	reg_avdd_level;	/* uV range */
	int	reg_vdd_lvsio_level;	/* uV range */
};

#ifndef SAIN_REG_HEADER
#define SAIN_REG_HEADER


#define	TOUCH_ONESHOT_UPGRADE	1


#define SAIN_REST_CMD			0x00
#define SAIN_WAKEUP_CMD			0x01

#define SAIN_IDLE_CMD			0x04
#define SAIN_SLEEP_CMD			0x05

#define	SAIN_CLEAR_INT_STATUS_CMD	0x03
#define	SAIN_CALIBRATE_CMD		0x06
#define	SAIN_SAVE_STATUS_CMD		0x07
#define	SAIN_RECALL_FACTORY_CMD		0x0f

/* 0x10~12 */
#define SAIN_TOUCH_MODE			0x10
#define SAIN_CHIP_REVISION		0x13
#define SAIN_EEPROM_INFO		0x14

/* 0x20~21 */
#define SAIN_TOTAL_NUMBER_OF_X		0x20
#define SAIN_TOTAL_NUMBER_OF_Y		0x21
#define SAIN_SUPPORTED_FINGER_NUM	0x22

#define SAIN_AFE_FREQUENCY		0x23

#define	SAIN_X_RESOLUTION		0x28
#define	SAIN_Y_RESOLUTION		0x29

/* 0x30~33 */
#define SAIN_CALIBRATION_REF		0x30
#define SAIN_CALIBRATION_DEFAULT_N 	0x31
#define SAIN_NUMBER_OF_CALIBRATION 	0x32
#define SAIN_CALIBRATION_ACCURACY	0x33

#define	SAIN_PERIODICAL_INTERRUPT_INTERVAL	0x35

#define	SAIN_POINT_STATUS_REG		0x80
#define	SAIN_ICON_STATUS_REG		0x9a	/* icon event - four icon */

#define	SAIN_RAWDATA_REG		0x9F	/* raw data 320byte */

#define	SAIN_EEPROM_INFO_REG		0xaa
#define	SAIN_DATA_VERSION_REG		0xab

#define SAIN_FIRMWARE_VERSION		0xc9

#define	SAIN_ERASE_FLASH		0xc9
#define	SAIN_WRITE_FLASH		0xc8
#define	SAIN_READ_FLASH			0xca


/* 0xF0  */
#define	SAIN_INT_ENABLE_FLAG		0xf0
/*--------------------------------------------------------------------- */

/* Interrupt & status register flag bit
/*------------------------------------------------- */
#define	BIT_PT_CNT_CHANGE			0
#define	BIT_DOWN				1
#define	BIT_MOVE				2
#define	BIT_UP					3
#define	BIT_HOLD				4
#define	BIT_LONG_HOLD				5
#define	RESERVED_0				6
#define	RESERVED_1				7
#define	BIT_WEIGHT_CHANGE			8
#define	BIT_PT_NO_CHANGE			9
#define	BIT_REJECT				10
#define	BIT_PT_EXIST				11		/* status register only */
/*------------------------------------------------- */
#define	RESERVED_2				12
#define	RESERVED_3				13
#define	RESERVED_4				14
#define	BIT_ICON_EVENT				15

/* 4 icon */
#define	BIT_O_ICON0_DOWN			0
#define	BIT_O_ICON1_DOWN			1
#define	BIT_O_ICON2_DOWN			2
#define	BIT_O_ICON3_DOWN			3
#define	BIT_O_ICON4_DOWN			4
#define	BIT_O_ICON5_DOWN			5
#define	BIT_O_ICON6_DOWN			6
#define	BIT_O_ICON7_DOWN			7

#define	BIT_O_ICON0_UP				8
#define	BIT_O_ICON1_UP				9
#define	BIT_O_ICON2_UP				10
#define	BIT_O_ICON3_UP				11
#define	BIT_O_ICON4_UP				12
#define	BIT_O_ICON5_UP				13
#define	BIT_O_ICON6_UP				14
#define	BIT_O_ICON7_UP				15


#define	SUB_BIT_EXIST			0		/* status register only */
#define	SUB_BIT_DOWN			1
#define	SUB_BIT_MOVE			2
#define	SUB_BIT_UP			3
#define	SUB_BIT_UPDATE			4
#define	SUB_BIT_WAIT			5


#define	sain_bit_set(val, n)		((val) &= ~(1<<(n)), (val) |= (1<<(n)))
#define	sain_bit_clr(val, n)		((val) &= ~(1<<(n)))
#define	sain_bit_test(val, n)		((val) & (1<<(n)))
#endif /*SAIN_REG_HEADER */

#endif /*  __LINUX_SAIN_TOUCH_H */
