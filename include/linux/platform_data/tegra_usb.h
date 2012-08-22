/*
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2010-2011 NVIDIA Corporation
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
	FORCE_ALL,
};

#define CAUSE_NUM	3
#endif

enum tegra_usb_phy_type {
	TEGRA_USB_PHY_TYPE_UTMIP = 0,
	TEGRA_USB_PHY_TYPE_LINK_ULPI = 1,
	TEGRA_USB_PHY_TYPE_NULL_ULPI = 2,
	TEGRA_USB_PHY_TYPE_HSIC = 3,
	TEGRA_USB_PHY_TYPE_ICUSB = 4,
};

struct tegra_ehci_platform_data {
	enum tegra_usb_operating_modes operating_mode;
	/* power down the phy on bus suspend */
	int power_down_on_bus_suspend;
	int hotplug;
	int default_enable;
	void *phy_config;
	enum tegra_usb_phy_type phy_type;
};

struct tegra_otg_platform_data {
	struct platform_device *ehci_device;
	struct tegra_ehci_platform_data *ehci_pdata;
#ifdef CONFIG_MACH_N1
	void (*acc_power)(int);
	void (*usb_ldo_en)(int, int);
	void (*set_clk_func)(void(*)(int *, int), unsigned int *);
	struct otg_id_open_data *otg_id_open;
	unsigned int **batt_level;
	int currentlimit_irq;
#endif
};

#endif /* _TEGRA_USB_H_ */
