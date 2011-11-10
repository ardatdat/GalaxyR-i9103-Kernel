/*
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

#ifndef _TEGRA_USB_H_
#define _TEGRA_USB_H_

#ifdef CONFIG_MACH_N1
#include <linux/fsa9480.h>
#endif

enum tegra_usb_operating_modes {
	TEGRA_USB_DEVICE,
	TEGRA_USB_HOST,
	TEGRA_USB_OTG,
};

#ifdef CONFIG_MACH_N1
enum tegra_host_state {
	TEGRA_USB_POWEROFF,
	TEGRA_USB_POWERON,
	TEGRA_USB_OVERCURRENT,
};

enum tegra_clk_cause {
	HOST_CAUSE,
	VBUS_CAUSE,
	RESUME_CAUSE,
};
#endif

struct tegra_ehci_platform_data {
	enum tegra_usb_operating_modes operating_mode;
	/* power down the phy on bus suspend */
	int power_down_on_bus_suspend;
	void *phy_config;
#ifdef CONFIG_MACH_N1
	int host_notify;
	int sec_whlist_table_num;
#endif
};

struct tegra_otg_platform_data {
	struct platform_device* (*host_register)(void);
	void (*host_unregister)(struct platform_device*);
#ifdef CONFIG_MACH_N1
	void (*acc_power)(int);
	void (*usb_ldo_en)(int, int);
	void (*set_otg_func)(void(*)(int *, int), unsigned int *);
	struct otg_id_open_data *otg_id_open;
	unsigned int **batt_level;
	int currentlimit_irq;
#endif
};

#endif /* _TEGRA_USB_H_ */
