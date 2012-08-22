/*
 * Copyright (C) 2008 Samsung Electronics, Inc.
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

#ifndef __T20_WM8994_H
#define __T20_WM8994_H

#define WM8994_SUB_MIC_OFF		0x00
#define WM8994_SUB_MIC_ON		0x01

struct wm8994_platform_data {
	int ldo;
	void (*set_mic_bias)(bool on);
	void (*set_sub_mic_bias)(bool on);
	bool (*get_codec_state)(void);
	void (*set_ear_sel)(bool on);
	void (*set_dap_connection)(u8 value);
	int wm8994_submic_state;
};
#endif
