/*
 * wm8994_samsung.h  --  WM8994 Soc Audio driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _WM8994_SAMSUNG_H
#define _WM8994_SAMSUNG_H

#include <sound/soc.h>
#include <linux/mfd/wm8994/wm8994_pdata.h>
#include <mach/tegra_das.h>

/* Sources for AIF1/2 SYSCLK - use with set_dai_sysclk() */
#define WM8994_SYSCLK_MCLK1 1
#define WM8994_SYSCLK_MCLK2 2
#define WM8994_SYSCLK_FLL1  3
#define WM8994_SYSCLK_FLL2  4

#define WM8994_FLL1 1
#define WM8994_FLL2 2

/* Added belows codes by Samsung Electronics.*/

#include "wm8994_def.h"

#define WM8994_SYSCLK_MCLK     1
#define WM8994_SYSCLK_FLL      2

#define AUDIO_COMMON_DEBUG	1

#define WM8994_FACTORY_LOOPBACK

#define FM_PATH_DRC_BLOCK		0

#define AUDIENCE_CODEC_MASTER		1

#define DEACTIVE		0x00
#define PLAYBACK_ACTIVE		0x01
#define CAPTURE_ACTIVE		0x02
#define CALL_ACTIVE		0x04
#define FMRADIO_ACTIVE		0x08

#define PCM_STREAM_DEACTIVE	0x00
#define PCM_STREAM_PLAYBACK	0x01
#define PCM_STREAM_CAPTURE	0x02

/*
Codec Output Path BIT
[0:11]		: For output device
[12:15]	: For mode
[16]		: For gain code
*/
#define PLAYBACK_MODE	(0x01 << 12)
#define VOICECALL_MODE	(0x01 << 13)
#define RECORDING_MODE	(0x01 << 14)
#define FMRADIO_MODE	(0x01 << 15)

#define GAIN_DIVISION_BIT	(0x01 << 16)
#define VOIPCALL_MODE	(0x01 << 17)	/* 0x20000 */
#define COMMON_SET_BIT		(0x01 << 0)
#define PLAYBACK_RCV		(0x01 << 1)
#define PLAYBACK_SPK		(0x01 << 2)
#define PLAYBACK_HP		(0x01 << 3)
#define PLAYBACK_BT		(0x01 << 4)
#define PLAYBACK_SPK_HP	(0x01 << 5)
#define PLAYBACK_RING_SPK	(0x01 << 6)
#define PLAYBACK_RING_HP	(0x01 << 7)
#define PLAYBACK_RING_SPK_HP	(0x01 << 8)
#define PLAYBACK_HP_NO_MIC	(0x01 << 9)
#define PLAYBACK_LINEOUT	(0x01 << 10)
#define PLAYBACK_SPK_LINEOUT	(0x01 << 11) /* 0x800 */

#define VOICECALL_RCV		(0x01 << 1)
#define VOICECALL_SPK		(0x01 << 2)
#define VOICECALL_HP		(0x01 << 3)
#define VOICECALL_HP_NO_MIC	(0x01 << 4)
#define VOICECALL_BT		(0x01 << 5)
#if defined (CONFIG_MACH_BOSE_ATT)
#define VOICECALL_RCV_AUDIENCE	(0x01 << 6)
#define VOICECALL_SPK_AUDIENCE	(0x01 << 7)
#define VOICECALL_RCV_HAC		(0x01 << 8)
#define VOICECALL_HP_TTY		(0x01 << 9)
#endif

#define RECORDING_MAIN		(0x01 << 1)
#define RECORDING_HP		(0x01 << 2)
#define RECORDING_SUB		(0x01 << 3)
#define RECORDING_BT		(0x01 << 4)
#define RECORDING_REC_MAIN	(0x01 << 5)
#define RECORDING_REC_HP	(0x01 << 6)
#define RECORDING_REC_SUB	(0x01 << 7)
#define RECORDING_REC_BT	(0x01 << 8)
#define RECORDING_CAM_MAIN	(0x01 << 9)
#define RECORDING_CAM_HP	(0x01 << 10)
#define RECORDING_CAM_SUB	(0x01 << 11)
#define RECORDING_CAM_BT	(0x01 << 12)

#define VOIPCALL_RCV		(0x01 << 1)
#define VOIPCALL_SPK		(0x01 << 2)
#define VOIPCALL_HP		(0x01 << 3)
#define VOIPCALL_MAINMIC	(0x01 << 4)
#define VOIPCALL_SUBMIC		(0x01 << 5)
#define VOIPCALL_HPMIC		(0x01 << 6)
#define VOIPCALL_RCV_OTHER	(0x01 << 7)
#define VOIPCALL_SPK_OTHER	(0x01 << 8)
#define VOIPCALL_HP_OTHER	(0x01 << 9)
#define VOIPCALL_MAINMIC_OTHER	(0x01 << 10)
#define VOIPCALL_SUBMIC_OTHER	(0x01 << 11)
#define VOIPCALL_HPMIC_OTHER	(0x01 << 12)

#define FMRADIO_HP			(0x01 << 1)
#define FMRADIO_SPK			(0x01 << 2)

#define PLAYBACK_GAIN_NUM 58
#define VOICECALL_GAIN_NUM 33
#define RECORDING_GAIN_NUM 36
#define FMRADIO_GAIN_NUM 24
#define GAIN_CODE_NUM 13
#define VOIPCALL_GAIN_NUM 38

#if defined (CONFIG_MACH_BOSE_ATT)
#define TTY_OFF      0
#define TTY_ON       1
#define AUDIENCE_OFF 2
#define AUDIENCE_ON  3
#define HAC_OFF      4
#define HAC_ON       5
#endif

/*
 * Definitions of enum type
 */
enum audio_path	{
	OFF, RCV, SPK, HP, HP_NO_MIC, BT, SPK_HP,
	RING_SPK, RING_HP, RING_NO_MIC, RING_SPK_HP,
	LINEOUT, SPK_LINEOUT
};
#if defined(CONFIG_MACH_N1_CHN)
enum mic_path		{MAIN, HP_MIC, SUB, BT_REC, VOICE_ALL,VOICE_RX, MIC_OFF};
#else
enum mic_path		{MAIN, HP_MIC, SUB, BT_REC, MIC_OFF};
#endif
enum power_state	{CODEC_OFF, CODEC_ON };
enum ringtone_state	{RING_OFF, RING_ON};
enum input_source_state	{DEFAULT, RECOGNITION, CAMCORDER};
enum voip_state		{VOIP_OFF, VOIP_ON, VOIP_ON_OTHER};
enum fmradio_path	{FMR_OFF, FMR_HP, FMR_SPK,
				FMR_SPK_MIX, FMR_HP_MIX, FMR_DUAL_MIX};
enum fmradio_mix_path	{FMR_MIX_OFF, FMR_MIX_HP,
				FMR_MIX_SPK, FMR_MIX_DUAL};
enum mic_control	{
	ALL_MIC_OFF,
	MAIN_MIC_ON,
	SUB_MIC_ON,
	TWO_MIC_ON,
	EAR_MIC_ON
};
enum wb_voice_state	{WB_OFF, WB_ON};

#ifdef WM8994_FACTORY_LOOPBACK
enum loopback_state	{LOOPBACK_OFF, LOOPBACK_RCV, LOOPBACK_SPK, LOOPBACK_HP, LOOPBACK_HP_3POLE};
#endif

typedef void (*select_route)(struct snd_soc_codec *);
typedef void (*select_mic_route)(struct snd_soc_codec *);
typedef int (*select_clock_control)(struct snd_soc_codec *, int);


struct wm8994_setup_data {
	int i2c_bus;
	unsigned short i2c_address;
};

enum wm8994_dc_servo_slots {
	DCS_MEDIA = 0,
	DCS_VOICE = 1,
	DCS_SPK_HP = 2,
};

struct wm8994_priv {
	struct snd_soc_codec *codec;
	struct delayed_work delayed_work;
	int master;
	int sysclk_source;
	unsigned int mclk_rate;
	unsigned int sysclk_rate;
	unsigned int fs;
	unsigned int bclk;
	unsigned int hw_version;
	unsigned int codec_state;
	unsigned int  stream_state;
	enum audio_path cur_path;
	enum mic_path rec_path;
	enum fmradio_path fmradio_path;
	enum fmradio_mix_path fmr_mix_path;
	enum power_state power_state;
	enum input_source_state input_source;
	enum ringtone_state ringtone_active;
	enum wb_voice_state wb_state;
	select_route *universal_playback_path;
	select_route *universal_voicecall_path;
	select_mic_route *universal_mic_path;
	select_route *universal_fmradio_path;
	select_clock_control universal_clock_control;
	struct wm8994_platform_data *pdata;
	int gain_code;
	u16 dc_servo[3];
	int testmode_config_flag;
	int codecgain_reserve;
	enum voip_state voip_call_active;
	int fm_radio_vol;
#if defined (CONFIG_MACH_BOSE_ATT)
	unsigned int cur_audience;
        int AUDIENCE_state;
	int Fac_SUB_MIC_state;
	int TTY_state;
	int HAC_state;
	int clock_state;
#endif
	int music_mode;
#ifdef WM8994_FACTORY_LOOPBACK
	int loopback_path_control;	/* for AT command codec loopback test */
#endif
	int mute_pop;
	int boot_state;
	enum  dap_connection_status dap_state;
};

struct gain_info_t {
	unsigned int mode;
	unsigned short reg;
	unsigned short mask;
	unsigned short gain;
};

#if AUDIO_COMMON_DEBUG
#define DEBUG_LOG(format, ...)\
	printk(KERN_INFO "[ "SUBJECT " (%s,%d) ] " format "\n", \
			__func__, __LINE__, ## __VA_ARGS__);
#else
#define DEBUG_LOG(format, ...)
#endif

#define DEBUG_LOG_ERR(format, ...)\
	printk(KERN_ERR "[ "SUBJECT " (%s,%d) ] " format "\n", \
			__func__, __LINE__, ## __VA_ARGS__);

/* Definitions of function prototype. */
void wm8994_shutdown(struct snd_pcm_substream *substream,
			    struct snd_soc_dai *codec_dai);
unsigned int wm8994_read(struct snd_soc_codec *codec, unsigned int reg);
int wm8994_write(struct snd_soc_codec *codec,
		unsigned int reg, unsigned int value);
void wm8994_register_dump(struct snd_soc_codec *codec);
int wm8994_configure_clock(struct snd_soc_codec *codec, int en);
void wm8994_disable_path(struct snd_soc_codec *codec);
void wm8994_disable_rec_path(struct snd_soc_codec *codec);
void wm8994_disable_fmradio_path(struct snd_soc_codec *codec,
				enum fmradio_path path);
void wm8994_record_main_mic(struct snd_soc_codec *codec);
void wm8994_record_headset_mic(struct snd_soc_codec *codec);
void wm8994_record_sub_mic(struct snd_soc_codec *codec);
void wm8994_record_bluetooth(struct snd_soc_codec *codec);
void wm8994_set_playback_receiver(struct snd_soc_codec *codec);
void wm8994_set_playback_headset(struct snd_soc_codec *codec);
void wm8994_set_playback_speaker(struct snd_soc_codec *codec);
void wm8994_set_playback_bluetooth(struct snd_soc_codec *codec);
void wm8994_set_playback_speaker_headset(struct snd_soc_codec *codec);
void wm8994_set_playback_extra_dock_speaker(struct snd_soc_codec *codec);
void wm8994_set_playback_speaker_lineout(struct snd_soc_codec *codec);
void wm8994_set_voicecall_common_setting(struct snd_soc_codec *codec);
void wm8994_set_voicecall_receiver(struct snd_soc_codec *codec);
void wm8994_set_voicecall_headset(struct snd_soc_codec *codec);
void wm8994_set_voicecall_headphone(struct snd_soc_codec *codec);
void wm8994_set_voicecall_speaker(struct snd_soc_codec *codec);
void wm8994_set_voicecall_bluetooth(struct snd_soc_codec *codec);
void wm8994_set_recording_during_voicecall(struct snd_soc_codec *codec);
#if defined (CONFIG_MACH_BOSE_ATT)
void wm8994_set_voicecall_tty(struct snd_soc_codec *codec);
void wm8994_set_voicecall_hac(struct snd_soc_codec *codec);
void wm8994_set_voicecall_receiver_audience(struct snd_soc_codec *codec);
void wm8994_set_voicecall_speaker_audience(struct snd_soc_codec *codec);
void loopback_mute_timer_start(void);
#endif

#if defined(CONFIG_MACH_N1_CHN)
void wm8994_record_Voice_all(struct snd_soc_codec *codec);
void wm8994_record_Voice_rx(struct snd_soc_codec *codec);
#endif

int wm8994_set_codec_gain(struct snd_soc_codec *codec, u32 mode, u16 device);
void wm8994_set_fmradio_headset(struct snd_soc_codec *codec);
void wm8994_set_fmradio_speaker(struct snd_soc_codec *codec);
extern int gain_code_check(void);
u16 wm8994_get_codec_gain(u32 mode, u16 device, u16 reg);
void wm8994_reset_analog_vol_work(struct work_struct *work);
#endif
