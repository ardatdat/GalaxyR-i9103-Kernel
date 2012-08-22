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

#ifndef __S5K6AAFX_H__
#define __S5K6AAFX_H__

#include <linux/ioctl.h>  /* For IOCTL macros */

#define FACTORY_TEST

#define S5K6AAFX_IOCTL_SET_MODE			_IOW('o', 1, struct s5k6aafx_mode)
#define S5K6AAFX_IOCTL_TEST_PATTERN		_IOW('o', 7, enum s5k6aafx_test_pattern)
#define S5K6AAFX_IOCTL_COLOR_EFFECT		_IOW('o', 11, s5k6aafx_color_effect)
#define S5K6AAFX_IOCTL_WHITE_BALANCE		_IOW('o', 12, s5k6aafx_white_balance)
#define S5K6AAFX_IOCTL_EXPOSURE			_IOW('o', 14, s5k6aafx_exposure)
#define S5K6AAFX_IOCTL_CAM_MODE			_IOR('o', 16, s5k6aafx_cam_mode)
#define S5K6AAFX_IOCTL_VT_MODE			_IOR('o', 17, s5k6aafx_vt_mode)
#define S5K6AAFX_IOCTL_EXIF_INFO		_IOW('o', 18, struct s5k6aafx_exif_info)
#define S5K6AAFX_IOCTL_ESD_RESET		_IOR('o', 20, s5k6aafx_esd_reset)


#define S5K6AAFX_TABLE_WAIT_MS		0xFFFE
#define S5K6AAFX_TABLE_WAIT_MS_8	0xFE
#define S5K6AAFX_TABLE_WAIT_US_8	0xFD
#define S5K6AAFX_TABLE_END		0xFFFF
#define S5K6AAFX_TABLE_END_8		0xFF

#ifdef FACTORY_TEST
#define S5K6AAFX_IOCTL_DTP_TEST			_IOW('o', 15, s5k6aafx_dtp_test)

typedef enum {
	S5K6AAFX_DTP_TEST_OFF,
	S5K6AAFX_DTP_TEST_ON,
	S5K6AAFX_DTP_TEST_MODE_MAX
} s5k6aafx_dtp_test ;
#endif

typedef enum {
	S5K6AAFX_ESD_DETECTED = 0,
	S5K6AAFX_ESD_NOT_DETECTED,
	S5K6AAFX_ESD_MODE_MAX
} s5k6aafx_esd_reset;

enum s5k6aafx_test_pattern {
	S5K6AAFX_TEST_PATTERN_NONE,
	S5K6AAFX_TEST_PATTERN_COLORBARS,
	S5K6AAFX_TEST_PATTERN_CHECKERBOARD,
};

typedef enum {
	S5K6AAFX_CAM_MODE_OFF = 0,
	S5K6AAFX_CAM_MODE_ON,
	S5K6AAFX_CAM_MODE_MAX
} s5k6aafx_cam_mode ;

typedef enum {
	S5K6AAFX_VT_MODE_OFF = 0,
	S5K6AAFX_VT_MODE_MIRROR,
	S5K6AAFX_VT_MODE_NONMIRROR,
	S5K6AAFX_VT_MODE_MAX
}s5k6aafx_vt_mode;

struct s5k6aafx_exif_info {
	unsigned int info_exptime_numer;
	unsigned int info_exptime_denumer;
	unsigned int info_tv_numer;
	unsigned int info_tv_denumer;
	unsigned int info_av_numer;
	unsigned int info_av_denumer;
	unsigned int info_bv_numer;
	int info_bv_denumer;
	unsigned int info_ebv_numer;
	int info_ebv_denumer;
	__u16 info_iso;
	unsigned int info_flash;
};

struct s5k6aafx_otp_data {
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

struct s5k6aafx_mode {
	int xres;
	int yres;
	__u32 frame_length;
	__u32 coarse_time;
	__u16 gain;
	unsigned char PreviewActive;
	unsigned char VideoActive;
	unsigned char HalfPress;
	unsigned int StillCount;
	s5k6aafx_cam_mode camcordmode;
	s5k6aafx_cam_mode vtcallmode;
};

typedef enum {
    S5K6AAFX_0_DEGREES = 0,
    S5K6AAFX_90_DEGREES,
    S5K6AAFX_180_DEGREES,
    S5K6AAFX_270_DEGREES,
    S5K6AAFX_MAX
} s5k6aafx_degrees;

typedef enum {
	S5K6AAFX_EFFECT_NONE = 0,
	S5K6AAFX_EFFECT_MONO,
	S5K6AAFX_EFFECT_SEPIA,
	S5K6AAFX_EFFECT_NEGATIVE,
	S5K6AAFX_EFFECT_SOLARIZE,
	S5K6AAFX_EFFECT_POSTERIZE,
	S5K6AAFX_EFFECT_MODE_MAX
} s5k6aafx_color_effect;

typedef enum {
	S5K6AAFX_EXPOSURE_METER_CENTER = 0,
	S5K6AAFX_EXPOSURE_METER_SPOT,
	S5K6AAFX_EXPOSURE_METER_MATRIX,
	S5K6AAFX_EXPOSURE_METER_MODE_MAX
} s5k6aafx_exposure_meter;

typedef enum {
	S5K6AAFX_WB_AUTO = 0,
	S5K6AAFX_WB_DAYLIGHT,
	S5K6AAFX_WB_INCANDESCENT,
	S5K6AAFX_WB_FLUORESCENT,
	S5K6AAFX_WB_MODE_MAX
} s5k6aafx_white_balance;

typedef enum {
	S5K6AAFX_EXPOSURE_P2P0 = 0,
	S5K6AAFX_EXPOSURE_P1P5,
	S5K6AAFX_EXPOSURE_P1P0,
	S5K6AAFX_EXPOSURE_P0P5,
	S5K6AAFX_EXPOSURE_ZERO,
	S5K6AAFX_EXPOSURE_M0P5,
	S5K6AAFX_EXPOSURE_M1P0,
	S5K6AAFX_EXPOSURE_M1P5,
	S5K6AAFX_EXPOSURE_M2P0,
	S5K6AAFX_EXPOSURE_MODE_MAX
} s5k6aafx_exposure;

#define S5K6AAFX_IOCTL_PRETTY			_IOR('o', 18, s5k6aafx_pretty)
typedef enum {
	S5K6AAFX_PRETTY_P0 = 0,
	S5K6AAFX_PRETTY_P1,
	S5K6AAFX_PRETTY_P2,
	S5K6AAFX_PRETTY_P3,
	S5K6AAFX_PRETTY_MODE_MAX
} s5k6aafx_pretty;


struct s5k6aafx_reg_8 {
	u8 addr;
	u8 val;
};
struct s5k6aafx_reg {
	u16 addr;
	u16 val;
};

int s5k6aafx_write_table_8(struct i2c_client *client,
			      const struct s5k6aafx_reg_8 *table);
int s5k6aafx_write_table(struct i2c_client *client,
			      const struct s5k6aafx_reg table[]);

#ifdef __KERNEL__

struct s5k6aafx_platform_data {
	void (*power_on)(void);
	void (*power_off)(void);
};
#endif /* __KERNEL__ */

#endif  /* __S5K6AAFX_H__ */
