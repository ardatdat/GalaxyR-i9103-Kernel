/*
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MAX17043_BATTERY_H_
#define __MAX17043_BATTERY_H_

struct max17043_platform_data {
	u8 alert_flag;
	u8 charging_rcomp;
	u8 discharging_rcomp;
	int standard_temp;
	int comp_full;
	int comp_empty;
	int (*battery_online)(void);
	int (*charger_online)(void);
	int (*charger_enable)(void);
	int (*low_batt_cb)(void);
};

#endif
