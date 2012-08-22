/* linux/arch/arm/mach-tegra/board-n1-smd.c
 * Copyright (C) 2010-2011 Samsung Electronics. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include "gpio-names.h"

#define P3_GPIO_CP_ACT		TEGRA_GPIO_PV2
#define P3_GPIO_CP_RST		TEGRA_GPIO_PX1
#define P3_GPIO_CP_ON		TEGRA_GPIO_PO3
#define P3_GPIO_AP_ACT		TEGRA_GPIO_PV1
#define P3_GPIO_SIM_DETECT	NULL
#define P3_GPIO_CP_REQ		TEGRA_GPIO_PQ3
#define P3_GPIO_HSIC_EN		TEGRA_GPIO_PV0
#define P3_GPIO_HSIC_ACTIVE	TEGRA_GPIO_PQ5
#define P3_GPIO_SLAVE_WAKEUP	TEGRA_GPIO_PR4
#define P3_GPIO_HSIC_SUS_REQ        TEGRA_GPIO_PQ0
#define P3_GPIO_HOST_WAKEUP         TEGRA_GPIO_PW2
//For UNIV0.3
#define P3_GPIO_HSIC_SUS_REQ_OLD    TEGRA_GPIO_PW2
#define P3_GPIO_HOST_WAKEUP_OLD     TEGRA_GPIO_PQ0

#define P3_GPIO_FLM_TXD		TEGRA_GPIO_PJ7
#define P3_GPIO_FLM_RXD		TEGRA_GPIO_PB0
#define P3_GPIO_CP_DUMP_INT	TEGRA_GPIO_PT2

static struct resource smd_res[] = {
	[0] = {
		.name = "cp_act",
		.start = P3_GPIO_CP_ACT,
		.end = P3_GPIO_CP_ACT,
		.flags = IORESOURCE_IO,
	},
	[1] = {
		.name = "cp_rst",
		.start = P3_GPIO_CP_RST,
		.end = P3_GPIO_CP_RST,
		.flags = IORESOURCE_IO,
	},
	[2] = {
		.name = "cp_on",
		.start = P3_GPIO_CP_ON,
		.end = P3_GPIO_CP_ON,
		.flags = IORESOURCE_IO,
	},
	[3] = {
		.name = "ap_act",
		.start = P3_GPIO_AP_ACT,
		.end = P3_GPIO_AP_ACT,
		.flags = IORESOURCE_IO,
	},
	[4] = {
		.name = "sim_det",
		.start = P3_GPIO_SIM_DETECT,
		.end = P3_GPIO_SIM_DETECT,
		.flags = IORESOURCE_IO,
	},
	[5] = {
		.name = "hsic_act",
		.start = P3_GPIO_HSIC_ACTIVE,
		.end = P3_GPIO_HSIC_ACTIVE,
		.flags = IORESOURCE_IO,
	},
	[6] = {
		.name = "hsic_sus",
		.start = P3_GPIO_HSIC_SUS_REQ,
		.end = P3_GPIO_HSIC_SUS_REQ,
		.flags = IORESOURCE_IO,
	},
	[7] = {
		.name = "hsic_en",
		.start = P3_GPIO_HSIC_EN,
		.end = P3_GPIO_HSIC_EN,
		.flags = IORESOURCE_IO,
	},
	[8] = {
		.name = "cp_req",
		.start = P3_GPIO_CP_REQ,
		.end = P3_GPIO_CP_REQ,
		.flags = IORESOURCE_IO,
	},
	[9] = {
		.name = "slv_wkp",
		.start = P3_GPIO_SLAVE_WAKEUP,
		.end = P3_GPIO_SLAVE_WAKEUP,
		.flags = IORESOURCE_IO,
	},
	[10] = {
		.name = "hst_wkp",
		.start = P3_GPIO_HOST_WAKEUP,
		.end = P3_GPIO_HOST_WAKEUP,
		.flags = IORESOURCE_IO,
	},
};

static struct platform_device smd = {
	.name = "smd-ctl",
	.id = -1,
	.num_resources = ARRAY_SIZE(smd_res),
	.resource = smd_res,
};

int check_modem_alive(void)
{
	return gpio_get_value(P3_GPIO_CP_ACT);
}

int __init register_smd_resource(void)
{
	int i;

	if(system_rev == -1)
	{
		smd_res[6].start = P3_GPIO_HSIC_SUS_REQ_OLD;
		smd_res[6].end	 = P3_GPIO_HSIC_SUS_REQ_OLD;

		smd_res[10].start = P3_GPIO_HOST_WAKEUP_OLD;
		smd_res[10].end	 = P3_GPIO_HOST_WAKEUP_OLD;
	}

	for (i = 0; i < ARRAY_SIZE(smd_res); i++) {
		if (smd_res[i].start != NULL)
			tegra_gpio_enable(smd_res[i].start);
	}

	tegra_gpio_enable(P3_GPIO_FLM_TXD);
	gpio_request(P3_GPIO_FLM_TXD, "flm_txd");
	gpio_direction_output(P3_GPIO_FLM_TXD, 0);

	tegra_gpio_enable(P3_GPIO_FLM_RXD);
	gpio_request(P3_GPIO_FLM_RXD, "flm_rxd");
	gpio_direction_output(P3_GPIO_FLM_RXD, 0);

	tegra_gpio_enable(P3_GPIO_CP_DUMP_INT);
	gpio_request(P3_GPIO_CP_DUMP_INT, "dump_int");
	gpio_direction_output(P3_GPIO_CP_DUMP_INT, 0);

	return platform_device_register(&smd);
}
