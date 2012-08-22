/*
 * Copyright (C) 2010 Motorola, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#ifndef __S5K5BAFX_H__
#define __S5K5BAFX_H__

#include <linux/ioctl.h>  /* For IOCTL macros */

#define FACTORY_TEST

#define S5K5BAFX_IOCTL_SET_MODE		_IOW('o', 1, struct s5k5bafx_mode)
#define S5K5BAFX_IOCTL_TEST_PATTERN       _IOW('o', 7, enum s5k5bafx_test_pattern)
#define S5K5BAFX_IOCTL_COLOR_EFFECT       _IOW('o', 11, s5k5bafx_color_effect)
#define S5K5BAFX_IOCTL_WHITE_BALANCE      _IOW('o', 12, s5k5bafx_white_balance)
#define S5K5BAFX_IOCTL_EXPOSURE	        _IOW('o', 14, s5k5bafx_exposure)

#define S5K5BAFX_TABLE_WAIT_MS 0xFFFE
#define S5K5BAFX_TABLE_WAIT_MS_8 0xFE
#define S5K5BAFX_TABLE_WAIT_US_8 0xFD
#define S5K5BAFX_TABLE_END 0xFFFF
#define S5K5BAFX_TABLE_END_8 0xFF

#ifdef FACTORY_TEST
#define S5K5BAFX_IOCTL_DTP_TEST	        _IOW('o', 15, s5k5bafx_dtp_test)

typedef enum {
	S5K5BAFX_DTP_TEST_OFF,
	S5K5BAFX_DTP_TEST_ON
} s5k5bafx_dtp_test ;
#endif

enum s5k5bafx_test_pattern {
	S5K5BAFX_TEST_PATTERN_NONE,
	S5K5BAFX_TEST_PATTERN_COLORBARS,
	S5K5BAFX_TEST_PATTERN_CHECKERBOARD
};

#define S5K5BAFX_IOCTL_CAM_MODE 	_IOR('o', 16, s5k5bafx_cam_mode)
typedef enum {
	S5K5BAFX_CAM_MODE_OFF = 0,
	S5K5BAFX_CAM_MODE_ON
} s5k5bafx_cam_mode ;

#define S5K5BAFX_IOCTL_VT_MODE 		_IOR('o', 17, s5k5bafx_vt_mode)
typedef enum {
	S5K5BAFX_VT_MODE_OFF = 0,
	S5K5BAFX_VT_MODE_MIRROR,
	S5K5BAFX_VT_MODE_NONMIRROR,
}s5k5bafx_vt_mode;

struct s5k5bafx_otp_data {
	/* Only the first 5 bytes are actually used. */
	__u8 sensor_serial_num[6];
	__u8 part_num[8];
	__u8 lens_id[1];
	__u8 manufacture_id[2];
	__u8 factory_id[2];
	__u8 manufacture_date[9];
	__u8 manufacture_line[2];

	__u32 module_serial_num;
	__u8 focuser_liftoff[2];
	__u8 focuser_macro[2];
	__u8 reserved1[12];
	__u8 shutter_cal[16];
	__u8 reserved2[183];

	/* Big-endian. CRC16 over 0x00-0x41 (inclusive) */
	__u16 crc;
	__u8 reserved3[3];
	__u8 auto_load[2];
} __attribute__ ((packed));

struct s5k5bafx_mode {
	int xres;
	int yres;
	__u32 frame_length;
	__u32 coarse_time;
	__u16 gain;
	unsigned char PreviewActive;
	unsigned char VideoActive;
	unsigned char HalfPress;
	unsigned int StillCount;
	s5k5bafx_cam_mode camcordmode;
	s5k5bafx_cam_mode vtcallmode;
};

typedef enum {
	FRONT_EFFECT_NONE = 0,
	FRONT_EFFECT_MONO,
	FRONT_EFFECT_SEPIA,
	FRONT_EFFECT_NEGATIVE,
	FRONT_EFFECT_SOLARIZE,
	FRONT_EFFECT_POSTERIZE
} s5k5bafx_color_effect;

typedef enum {
	FRONT_WB_AUTO = 0,
	FRONT_WB_DAYLIGHT,
	FRONT_WB_INCANDESCENT,
	FRONT_WB_FLUORESCENT
} s5k5bafx_white_balance;

typedef enum {
	FRONT_EXPOSURE_P2P0 = 0,
	FRONT_EXPOSURE_P1P5,
	FRONT_EXPOSURE_P1P0,
	FRONT_EXPOSURE_P0P5,
	FRONT_EXPOSURE_ZERO,
	FRONT_EXPOSURE_M0P5,
	FRONT_EXPOSURE_M1P0,
	FRONT_EXPOSURE_M1P5,
	FRONT_EXPOSURE_M2P0,
	FRONT_EXPOSURE_MODE_MAX
} s5k5bafx_exposure;

#define S5K5BAFX_IOCTL_PRETTY 		_IOR('o', 18, s5k5bafx_pretty)
typedef enum {
	FRONT_PRETTY_P0 = 0,
	FRONT_PRETTY_P1,
	FRONT_PRETTY_P2,
	FRONT_PRETTY_P3,
} s5k5bafx_pretty;


struct s5k5bafx_reg_8 {
	u8 addr;
	u8 val;
};

int s5k5bafx_write_table_8(struct i2c_client *client,
			      const struct s5k5bafx_reg_8 *table);

#ifdef __KERNEL__

struct s5k5bafx_platform_data {
	void (*power_on)(void);
	void (*power_off)(void);
};
#endif /* __KERNEL__ */

#endif  /* __S5K5BAFX_H__ */
