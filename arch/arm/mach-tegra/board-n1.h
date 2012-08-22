/*
 * arch/arm/mach-tegra/board-n1.h
 *
 * Copyright (C) 2010 Google, Inc.
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

#ifndef _MACH_TEGRA_BOARD_N1_H
#define _MACH_TEGRA_BOARD_N1_H

int n1_regulator_init(void);
int n1_sdhci_init(void);
int n1_pinmux_init(void);
int n1_panel_init(void);
int n1_gpio_i2c_init(int lpm_mode);
int n1_sensors_init(void);
int n1_emc_init(void);
int n1_kbc_init(void);
void wm8994_set_mic_bias(bool on);
void wm8994_set_sub_mic_bias(bool on);
void wm8994_set_ear_sel(bool on);

#ifdef CONFIG_BT_BCM4330
extern void bcm_bt_lpm_exit_lpm_locked(struct uart_port *uport);
#endif

/* Interrupt numbers from external peripherals */
#define MAX8907C_INT_BASE       TEGRA_NR_IRQS
#define MAX8907C_INT_END        (MAX8907C_INT_BASE + 32)

#define TDIODE_OFFSET	(9000)	/* in millicelsius */

#ifdef CONFIG_USB_ANDROID_ACCESSORY
#    define ANDROID_ACCESSORY_CONFIG_STRING		"ACCESSORY Only(ADK mode)"
#    define ANDROID_ACCESSORY_ADB_CONFIG_STRING	"ACCESSORY _ADB (ADK + ADB mode)"
#  define USBSTATUS_ACCESSORY			0x100
#endif
#endif
