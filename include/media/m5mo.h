/*
 * Copyright (C) 2010 SAMSUNG ELECTRONICS.
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

#ifndef __m5mo_H__
#define __m5mo_H__

#include <linux/ioctl.h>  /* For IOCTL macros */

#define FACTORY_TEST

#define M5MO_IOCTL_SET_MODE		_IOW('o', 1, struct m5mo_mode)
#define M5MO_IOCTL_TEST_PATTERN       _IOW('o', 2, enum m5mo_test_pattern)
#define M5MO_IOCTL_SCENE_MODE	        _IOW('o', 3, enum m5mo_scene_mode)
#define M5MO_IOCTL_FOCUS_MODE	        _IOW('o', 4, enum m5mo_focus_mode)
#define M5MO_IOCTL_COLOR_EFFECT       _IOW('o', 5, enum m5mo_color_effect)
#define M5MO_IOCTL_WHITE_BALANCE      _IOW('o', 6, enum m5mo_white_balance)
#define M5MO_IOCTL_FLASH_MODE	        _IOW('o', 7, enum m5mo_flash_mode)
#define M5MO_IOCTL_EXPOSURE	        _IOW('o', 8, enum m5mo_exposure)
#define M5MO_IOCTL_AF_CONTROL	        _IOW('o', 9, enum m5mo_autofocus_control)
#define M5MO_IOCTL_AF_RESULT	        _IOR('o', 10, struct m5mo_autofocus_result)
#define M5MO_IOCTL_ESD_RESET	        _IOR('o', 11, enum m5mo_esd_reset)
#define M5MO_IOCTL_LENS_SOFT_LANDING	_IOW('o', 12, unsigned int)
#define M5MO_IOCTL_RECORDING_FRAME    _IOW('o', 13, enum m5mo_recording_frame)
#define M5MO_IOCTL_EXIF_INFO          _IOW('o', 14, struct m5mo_exif_info)

#define M5MO_IOCTL_EXPOSURE_METER	_IOW('o', 15, enum m5mo_exposure_meter)
#define M5MO_IOCTL_ISO			_IOW('o', 16, enum m5mo_iso)
#define M5MO_IOCTL_ANTISHAKE		_IOW('o', 17, enum m5mo_antishake)
#define M5MO_IOCTL_AUTOCONTRAST		_IOW('o', 18, enum m5mo_autocontrast)
#define M5MO_IOCTL_TOUCHAF		_IOW('o', 19, struct m5mo_touchaf_pos)

#define M5MO_TABLE_STOP 0xFFFF
#define M5MO_TABLE_END 0xFFFE
#define M5MO_TABLE_WAIT_MS 0xFFFD
#define M5MO_TABLE_LOAD 0xFFFC
#define M5MO_TABLE_DIRECT_WRITE_I2C 0xFFFB
#define M5MO_TABLE_WRITE 0xFFFA
#define M5MO_TABLE_WRITE_MEMORY 0xFFF9
#define M5MO_TABLE_READ 0xFFF8
#define M5MO_TABLE_WAIT_US 0xFFF7
#define M5MO_TABLE_TOUCHAF 0xFFF6

#ifdef FACTORY_TEST
#define M5MO_IOCTL_DTP_TEST 		_IOR('o', 20, enum m5mo_dtp_test)

enum m5mo_dtp_test{
	M5MO_DTP_TEST_OFF = 0,
	M5MO_DTP_TEST_ON
};
#endif

#define M5MO_IOCTL_CAM_MODE 		_IOR('o', 21, enum m5mo_cam_mode)
enum m5mo_cam_mode {
	M5MO_CAM_MODE_OFF = 0,
	M5MO_CAM_MODE_ON
};

#define M5MO_IOCTL_VT_MODE 		_IOR('o', 22, enum m5mo_vt_mode)
enum m5mo_vt_mode {
	M5MO_VT_MODE_OFF = 0,
	M5MO_VT_MODE_MIRROR,
	M5MO_VT_MODE_NONMIRROR
};

#define M5MO_IOCTL_FACE_BEAUTY 		_IOR('o', 23, enum m5mo_face_beauty)
enum m5mo_face_beauty {
	M5MO_FACE_BEAUTY_OFF = 0,
	M5MO_FACE_BEAUTY_ON
};

#define M5MO_IOCTL_FW_VERSION		_IOW('o', 24, struct m5mo_fw_version)
struct m5mo_fw_version {
	char unique_id[7];
};

#define M5MO_IOCTL_SAMSUNG_CAMERA 		_IOR('o', 25, enum m5mo_samsung_camera)
enum m5mo_samsung_camera {
	M5MO_SAMSUNG_CAMERA_OFF = 0,
	M5MO_SAMSUNG_CAMERA_ON
};

struct m5mo_exif_info {
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

enum m5mo_isp_mode {
	MODE_SYSTEM_INIT = 0,
	MODE_PARAMETER_SETTING,
	MODE_MONITOR,
	MODE_STILL_CAPTURE
};

enum m5mo_esd_reset {
	ESD_DETECTED = 0,
	ESD_NOT_DETECTED
};

struct m5mo_autofocus_result {
	__u32 value;
};

enum m5mo_autofocus_control {
	AF_START = 0,
	AF_STOP,
	CAF_START,
	CAF_STOP
};

enum m5mo_scene_mode {
	SCENE_AUTO = 0,
	SCENE_PORTRAIT,
	SCENE_LANDSCAPE,
	SCENE_NIGHT,
	SCENE_SPORTS,
	SCENE_PARTY,
	SCENE_BEACH,
	SCENE_SUNSET,
	SCENE_DUSK_DAWN,
	SCENE_FALL_COLOR,
	SCENE_FIRE_WORK,
	SCENE_TEXT,
	SCENE_CANDLE_LIGHT,
	SCENE_BACK_LIGHT,
	SCENE_MODE_MAX
};

enum m5mo_focus_mode {
	FOCUS_INVALID = -1,
	FOCUS_AUTO = 0,
	FOCUS_MACRO,
	FOCUS_FIXED,
	FOCUS_FACE_DETECT,
	FOCUS_CONTINUOUS_VIDEO,
	FOCUS_MODE_MAX
};

enum m5mo_color_effect {
	EFFECT_NONE = 0,
	EFFECT_MONO,
	EFFECT_SEPIA,
	EFFECT_NEGATIVE,
	EFFECT_SOLARIZE,
	EFFECT_POSTERIZE,
	EFFECT_MODE_MAX
};

enum m5mo_white_balance{
	WB_AUTO = 0,
	WB_DAYLIGHT,
	WB_INCANDESCENT,
	WB_FLUORESCENT,
	WB_CLOUDY,
	WB_MODE_MAX
};

enum m5mo_flash_mode {
	FLASH_AUTO = 0,
	FLASH_ON,
	FLASH_OFF,
	FLASH_TORCH,
	FLASH_MODE_MAX
};

enum m5mo_exposure {
	EXPOSURE_P2P0 = 0,
	EXPOSURE_P1P5,
	EXPOSURE_P1P0,
	EXPOSURE_P0P5,
	EXPOSURE_ZERO,
	EXPOSURE_M0P5,
	EXPOSURE_M1P0,
	EXPOSURE_M1P5,
	EXPOSURE_M2P0,
	EXPOSURE_MODE_MAX
};

enum m5mo_exposure_meter {
	EXPOSURE_METER_CENTER = 0,
	EXPOSURE_METER_SPOT,
	EXPOSURE_METER_MATRIX,
	EXPOSURE_METER_MAX
};

enum m5mo_iso {
	ISO_AUTO = 0,
	ISO_100,
	ISO_200,
	ISO_400,
	ISO_800,
	ISO_MAX
};

enum m5mo_antishake {
	ANTISHAKE_OFF = 0,
	ANTISHAKE_ON,
	ANTISHAKE_MAX
};

enum m5mo_autocontrast {
	AUTOCONTRAST_OFF = 0,
	AUTOCONTRAST_ON,
	AUTOCONTRAST_MAX
};

enum m5mo_test_pattern {
	TEST_PATTERN_NONE,
	TEST_PATTERN_COLORBARS,
	TEST_PATTERN_CHECKERBOARD
};

/*firmware standard*/
enum {
	FWUPDATE = 1,
	FWDUMP
};

#ifdef FACTORY_TEST
enum {
	FACTORY_FLASH_OFF = 0,
	FACTORY_FLASH_ON
};
#endif

struct m5mo_otp_data {
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

struct m5mo_mode {
	int xres;
	int yres;
	__u32 frame_length;
	__u32 coarse_time;
	__u16 gain;
	unsigned char PreviewActive;
	unsigned char VideoActive;
	unsigned char HalfPress;
	unsigned int StillCount;
	enum m5mo_cam_mode camcordmode;
	enum m5mo_vt_mode vtcallmode;
};

enum m5mo_recording_frame {
	RECORDING_CAF = 0,
	RECORDING_PREVIEW
};

enum m5mo_support_param {
	M5MO_CAN_NOT_SUPPORT_PARAM = 0,
	M5MO_CAN_SUPPORT_PARAM
};

struct m5mo_reg {
	__u16 addr;
	__u16 val;
};

struct m5mo_touchaf_pos {
	unsigned int xpos;
	unsigned int ypos;
};

int m5mo_write_reg(struct i2c_client *client, __u16 addr, __u8 val);
int m5mo_write_reg8(struct i2c_client *client, __u8 addr, __u8 val);
int m5mo_write_table(struct i2c_client *client,
		const struct m5mo_reg *table,
		int size);

#ifdef __KERNEL__
struct m5mo_platform_data {
	void (*power_on)(void);
	void (*power_off)(void);
	unsigned int (*isp_int_read)(void);
};
#endif /* __KERNEL__ */

#endif  /* __m5mo_H__ */
