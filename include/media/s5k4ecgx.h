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

#ifndef __S5K4ECGX_H__
#define __S5K4ECGX_H__

#include <linux/ioctl.h>  /* For IOCTL macros */

#define FACTORY_TEST

#define S5K4ECGX_IOCTL_SET_MODE			_IOW('o', 1, struct s5k4ecgx_mode)
#define S5K4ECGX_IOCTL_TEST_PATTERN		_IOW('o', 2, s5k4ecgx_test_pattern)
#define S5K4ECGX_IOCTL_SCENE_MODE		_IOW('o', 3, s5k4ecgx_scene_mode)
#define S5K4ECGX_IOCTL_FOCUS_MODE		_IOW('o', 4, s5k4ecgx_focus_mode)
#define S5K4ECGX_IOCTL_COLOR_EFFECT		_IOW('o', 5, s5k4ecgx_color_effect)
#define S5K4ECGX_IOCTL_WHITE_BALANCE		_IOW('o', 6, s5k4ecgx_white_balance)
#define S5K4ECGX_IOCTL_FLASH_MODE		_IOW('o', 7, s5k4ecgx_flash_mode)
#define S5K4ECGX_IOCTL_EXPOSURE			_IOW('o', 8, s5k4ecgx_exposure)
#define S5K4ECGX_IOCTL_AF_CONTROL		_IOW('o', 9, s5k4ecgx_autofocus_control)
#define S5K4ECGX_IOCTL_AF_RESULT		_IOR('o', 10, struct s5k4ecgx_autofocus_result)
#define S5K4ECGX_IOCTL_ESD_RESET		_IOR('o', 11, s5k4ecgx_esd_reset)
#define S5K4ECGX_IOCTL_LENS_SOFT_LANDING	_IOW('o', 12, unsigned int)	//not use
#define S5K4ECGX_IOCTL_RECORDING_FRAME		_IOW('o', 13, s5k4ecgx_recording_frame)	//not use
#define S5K4ECGX_IOCTL_EXIF_INFO		_IOW('o', 14, struct s5k4ecgx_exif_info)
#define S5K4ECGX_IOCTL_EXPOSURE_METER		_IOW('o', 15, s5k4ecgx_exposure_meter)
#define S5K4ECGX_IOCTL_ISO			_IOW('o', 16, s5k4ecgx_iso)
#define S5K4ECGX_IOCTL_ANTISHAKE		_IOW('o', 17, s5k4ecgx_antishake)	//not use
#define S5K4ECGX_IOCTL_AUTO_CONTRAST		_IOW('o', 18, s5k4ecgx_auto_contrast)
#define S5K4ECGX_IOCTL_TOUCHAF			_IOW('o', 19, struct s5k4ecgx_touchaf_pos)
#ifdef FACTORY_TEST
#define S5K4ECGX_IOCTL_DTP_TEST			_IOW('o', 20, s5k4ecgx_dtp_test)
#endif
#define S5K4ECGX_IOCTL_CAM_MODE			_IOR('o', 21, s5k4ecgx_cam_mode)
#define S5K4ECGX_IOCTL_VT_MODE			_IOR('o', 22, s5k4ecgx_vt_mode)
#define S5K4ECGX_IOCTL_GET_CAPTURE		_IOR('o', 23, unsigned int)
#define S5K4ECGX_IOCTL_CAPTURE_RESOLUTION	_IOR('o', 24, struct s5k4ecgx_capture_resolution)

#define S5K4ECGX_TABLE_WAIT_MS		0xFFFE
#define S5K4ECGX_TABLE_WAIT_MS_8	0xFE
#define S5K4ECGX_TABLE_WAIT_US_8	0xFD
#define S5K4ECGX_TABLE_END		0xFFFF
#define S5K4ECGX_TABLE_END_8		0xFF
#define S5K4ECGX_TABLE_WAIT_US		0xFFF7

struct s5k4ecgx_exif_info {
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

struct s5k4ecgx_otp_data {
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

struct s5k4ecgx_autofocus_result {
	__u32 value;
};

struct s5k4ecgx_reg {
	u16 addr;
	u16 val;
};

int s5k4ecgx_write_table(struct i2c_client *client,
			      const struct s5k4ecgx_reg table[]);

typedef enum {
	S5K4ECGX_ESD_DETECTED = 0,
	S5K4ECGX_ESD_NOT_DETECTED,
	S5K4ECGX_ESD_MODE_MAX
} s5k4ecgx_esd_reset;

typedef enum {
	S5K4ECGX_AF_START = 0,
	S5K4ECGX_AF_STOP,
//	S5K4ECGX_CAF_START,
//	S5K4ECGX_CAF_STOP,
	S5K4ECGX_AF_MODE_MAX
} s5k4ecgx_autofocus_control;

typedef enum {
	S5K4ECGX_SCENE_AUTO = 0,
	S5K4ECGX_SCENE_PORTRAIT,
	S5K4ECGX_SCENE_LANDSCAPE,
	S5K4ECGX_SCENE_NIGHT,
	S5K4ECGX_SCENE_SPORTS,
	S5K4ECGX_SCENE_PARTY,
	S5K4ECGX_SCENE_BEACH,
	S5K4ECGX_SCENE_SUNSET,
	S5K4ECGX_SCENE_DUSK_DAWN,
	S5K4ECGX_SCENE_FALL_COLOR,
	S5K4ECGX_SCENE_FIRE_WORK,
	S5K4ECGX_SCENE_TEXT,
	S5K4ECGX_SCENE_CANDLE_LIGHT,
	S5K4ECGX_SCENE_BACK_LIGHT,
	S5K4ECGX_SCENE_MODE_MAX
} s5k4ecgx_scene_mode;

typedef enum {
	S5K4ECGX_0_DEGREES = 0,
	S5K4ECGX_90_DEGREES,
	S5K4ECGX_180_DEGREES,
	S5K4ECGX_270_DEGREES,
	S5K4ECGX_MAX
} s5k4ecgx_degrees;

typedef enum {
	S5K4ECGX_FOCUS_INVALID = -1,
	S5K4ECGX_FOCUS_AUTO = 0,
	S5K4ECGX_FOCUS_MACRO,
	S5K4ECGX_FOCUS_FIXED,
	S5K4ECGX_FOCUS_MODE_MAX
} s5k4ecgx_focus_mode;

typedef enum {
	S5K4ECGX_EFFECT_NONE = 0,
	S5K4ECGX_EFFECT_MONO,
	S5K4ECGX_EFFECT_SEPIA,
	S5K4ECGX_EFFECT_NEGATIVE,
	S5K4ECGX_EFFECT_POSTERIZE,
	S5K4ECGX_EFFECT_MODE_MAX
} s5k4ecgx_color_effect;

typedef enum {
	S5K4ECGX_WB_AUTO = 0,
	S5K4ECGX_WB_DAYLIGHT,
	S5K4ECGX_WB_INCANDESCENT,
	S5K4ECGX_WB_FLUORESCENT,
	S5K4ECGX_WB_CLOUDY,
	S5K4ECGX_WB_CWF,
	S5K4ECGX_WB_MODE_MAX
} s5k4ecgx_white_balance;

typedef enum {
	S5K4ECGX_FLASH_AUTO = 0,
	S5K4ECGX_FLASH_ON,
	S5K4ECGX_FLASH_OFF,
	S5K4ECGX_FLASH_TORCH,
	S5K4ECGX_FLASH_MODE_MAX
} s5k4ecgx_flash_mode;

typedef enum {
	S5K4ECGX_SHARPNESS_P2P0 = 0,
	S5K4ECGX_SHARPNESS_P1P0,
	S5K4ECGX_SHARPNESS_ZERO,
	S5K4ECGX_SHARPNESS_M1P0,
	S5K4ECGX_SHARPNESS_M2P0,
	S5K4ECGX_SHARPNESS_MODE_MAX
} s5k4ecgx_sharpness;

typedef enum {
	S5K4ECGX_SATURATION_P2P0 = 0,
	S5K4ECGX_SATURATION_P1P0,
	S5K4ECGX_SATURATION_ZERO,
	S5K4ECGX_SATURATION_M1P0,
	S5K4ECGX_SATURATION_M2P0,
	S5K4ECGX_SATURATION_MODE_MAX
} s5k4ecgx_saturation;

typedef enum {
	S5K4ECGX_EXPOSURE_P2P0 = 0,
	S5K4ECGX_EXPOSURE_P1P5,
	S5K4ECGX_EXPOSURE_P1P0,
	S5K4ECGX_EXPOSURE_P0P5,
	S5K4ECGX_EXPOSURE_ZERO,
	S5K4ECGX_EXPOSURE_M0P5,
	S5K4ECGX_EXPOSURE_M1P0,
	S5K4ECGX_EXPOSURE_M1P5,
	S5K4ECGX_EXPOSURE_M2P0,
	S5K4ECGX_EXPOSURE_MODE_MAX
} s5k4ecgx_exposure;

typedef enum {
	S5K4ECGX_EXPOSURE_METER_CENTER = 0,
	S5K4ECGX_EXPOSURE_METER_SPOT,
	S5K4ECGX_EXPOSURE_METER_MATRIX,
	S5K4ECGX_EXPOSURE_METER_MODE_MAX
} s5k4ecgx_exposure_meter;

typedef enum {
	S5K4ECGX_ISO_AUTO = 0,
	S5K4ECGX_ISO_50,
	S5K4ECGX_ISO_100,
	S5K4ECGX_ISO_200,
	S5K4ECGX_ISO_400,
	S5K4ECGX_ISO_SPORT,
	S5K4ECGX_ISO_NIGHT,
	S5K4ECGX_ISO_MODE_MAX
} s5k4ecgx_iso;

typedef enum {
	S5K4ECGX_FPS_AUTO = 0,
	S5K4ECGX_FPS_7,
	S5K4ECGX_FPS_15,
	S5K4ECGX_FPS_20,
	S5K4ECGX_FPS_30,
	S5K4ECGX_FPS_MAX
} s5k4ecgx_frame_rate;

#ifdef FACTORY_TEST
typedef enum {
	S5K4ECGX_DTP_TEST_OFF = 0,
	S5K4ECGX_DTP_TEST_ON,
	S5K4ECGX_DTP_TEST_MAX
} s5k4ecgx_dtp_test;
#endif

typedef enum {
	S5K4ECGX_TEST_PATTERN_NONE = 0,
	S5K4ECGX_TEST_PATTERN_COLORBARS,
	S5K4ECGX_TEST_PATTERN_CHECKERBOARD
} s5k4ecgx_test_pattern;

typedef enum {
	S5K4ECGX_CAM_MODE_OFF = 0,
	S5K4ECGX_CAM_MODE_ON,
	S5K4ECGX_CAM_MODE_MAX
} s5k4ecgx_cam_mode;

typedef enum {
	S5K4ECGX_VT_MODE_OFF = 0,
	S5K4ECGX_VT_MODE_MIRROR,
	S5K4ECGX_VT_MODE_NONMIRROR,
	S5K4ECGX_VT_MODE_MAX
} s5k4ecgx_vt_mode;

typedef enum {
	S5K4ECGX_AUTO_CONTRAST_OFF = 0,
	S5K4ECGX_AUTO_CONTRAST_ON,
	S5K4ECGX_AUTO_CONTRAST_MAX
} s5k4ecgx_auto_contrast;

typedef enum {
	ANTISHAKE_OFF = 0,
	ANTISHAKE_ON,
	ANTISHAKE_MAX
} s5k4ecgx_antishake ;

typedef enum {
	RECORDING_CAF = 0,
	RECORDING_PREVIEW
} s5k4ecgx_recording_frame;

struct s5k4ecgx_touchaf_pos {
	unsigned int xpos;
	unsigned int ypos;
};

struct s5k4ecgx_capture_resolution {
	unsigned int cap_x_res;
	unsigned int cap_y_res;
};

struct s5k4ecgx_reg_8 {
	u8 addr;
	u8 val;
};

struct s5k4ecgx_mode {
	int xres;
	int yres;
	int cap_xres;
	int cap_yres;
	__u32 frame_length;
	__u32 coarse_time;
	__u16 gain;
	unsigned char PreviewActive;
	unsigned char VideoActive;
	unsigned char HalfPress;
	unsigned int StillCount;
	s5k4ecgx_cam_mode camcordmode;
	s5k4ecgx_cam_mode vtcallmode;
};

int s5k4ecgx_write_table_8(struct i2c_client *client,
			      const struct s5k4ecgx_reg_8 *table);

#ifdef __KERNEL__

struct s5k4ecgx_platform_data {
	void (*power_on)(void);
	void (*power_off)(void);
	void (*flash_onoff)(int);
	void (*torch_onoff)(int);
};
#endif /* __KERNEL__ */

#endif  /* __S5K4ECGX_H__ */
