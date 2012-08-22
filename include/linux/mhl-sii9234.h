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

struct mhl_platform_data {
	unsigned int mhl_sel;
	unsigned int mhl_rst;
	unsigned int mhl_int;
	unsigned int mhl_wake_up;
	unsigned int mhl_irq;

	int	(*power_onoff) (int onoff);
	int	(*switch_onoff) (int onoff);
};
