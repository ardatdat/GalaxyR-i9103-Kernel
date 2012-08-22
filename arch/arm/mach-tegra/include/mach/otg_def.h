/*
 * arch/arm/mach-tegra/include/mach/otg_def.h
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * Author:
 *  Dongrak.Shin <dongrak.shin@samsung.com>
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
#ifndef __MACH_TEGRA_OTG_DEF_H
#define __MACH_TEGRA_OTG_DEF_H

#include <linux/fsa9480.h>

#ifdef CONFIG_MACH_N1
struct otg_detect_data {
	void (*check_detection)(int *, int);
	unsigned int *clk_cause;
};

struct gpioi2c_otg_data {
	struct otg_id_open_data *otg_open;
	struct otg_detect_data *otg_clk_data;
	void (*vbus_en)(int);
};

void fsa9480_otg_data_init(struct otg_id_open_data *o,
				struct otg_detect_data *d
					, void (*vbus_en)(int));
#endif

#endif /* defined(__MACH_TEGRA_OTG_DEF_H) */
