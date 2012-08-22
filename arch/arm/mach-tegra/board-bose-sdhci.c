/*
 * arch/arm/mach-tegra/board-n1-sdhci.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2011 NVIDIA Corporation.
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

#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/wlan_plat.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mmc/host.h>

#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>

#include "gpio-names.h"
#if defined (CONFIG_MACH_BOSE_ATT)
#include <mach/gpio-bose.h>
#else
#include <mach/gpio-n1.h>
#endif
#include "board.h"

#define BOARD_REV06 0x09
#define BOARD_REV08 0x0B


static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;
static int n1_wifi_status_register(void (*callback)(int , void *), void *);
static struct clk *wifi_32k_clk;
static int n1_wifi_reset(int on);
int n1_wifi_power(int on);
struct platform_device *tegra_sdhci_device0_ptr;

static int n1_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

// N1_ICS
static int n1_wifi_set_carddetect(int val)
{
	pr_debug("%s: %d\n", __func__, val);
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		pr_warning("%s: Nobody to notify\n", __func__);
	return 0;
}

int n1_wifi_power(int on)
{
	pr_debug("%s: %d\n", __func__, on);

	if (system_rev == 0 || system_rev == 1)
		gpio_set_value(TEGRA_GPIO_PQ2, on);
	else
		gpio_set_value(GPIO_WLAN_EN, on);

	mdelay(100);

	if (on)
		clk_enable(wifi_32k_clk);
	else
		clk_disable(wifi_32k_clk);

	return 0;
}
EXPORT_SYMBOL(n1_wifi_power);

static int n1_wifi_reset(int on)
{
	pr_debug("%s: do nothing\n", __func__);
	return 0;
}

static struct wifi_platform_data n1_wifi_control = {
	.set_power      = n1_wifi_power,
	.set_reset      = n1_wifi_reset,
	.set_carddetect = n1_wifi_set_carddetect,
};

static struct platform_device n1_wifi_device = {
	.name           = "bcmdhd_wlan",
	.id             = 1,
	.dev            = {
		.platform_data = &n1_wifi_control,
	},
};

static struct resource sdhci_resource0[] = {
	[0] = {
		.start  = INT_SDMMC1,
		.end    = INT_SDMMC1,
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC1_BASE,
		.end	= TEGRA_SDMMC1_BASE + TEGRA_SDMMC1_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource sdhci_resource2[] = {
	[0] = {
		.start  = INT_SDMMC3,
		.end    = INT_SDMMC3,
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC3_BASE,
		.end	= TEGRA_SDMMC3_BASE + TEGRA_SDMMC3_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource sdhci_resource3[] = {
	[0] = {
		.start  = INT_SDMMC4,
		.end    = INT_SDMMC4,
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC4_BASE,
		.end	= TEGRA_SDMMC4_BASE + TEGRA_SDMMC4_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct embedded_sdio_data embedded_sdio_data0 = {
	.cccr   = {
		.sdio_vsn   	= 2,
		.multi_block    = 1,
		.low_speed  	= 0,
		.wide_bus   	= 0,
		.high_power 	= 1,
		.high_speed 	= 1,
	},
	.cis  = {
		.vendor     	= 0x02d0,
		.device     	= 0x4329,
	},
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data0 = {
	.mmc_data = {
		.register_status_notify	= n1_wifi_status_register,
		.embedded_sdio = &embedded_sdio_data0,
		.built_in = 1,
	},
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data2 = {
#if defined (CONFIG_MACH_BOSE_ATT)
	.cd_gpio = TEGRA_GPIO_PV6,
#else
	.cd_gpio = -1,
#endif
	.wp_gpio = -1,
	.power_gpio = -1,
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data3 = {
	.is_8bit = 1,
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.mmc_data = {
		.built_in = 1,
	},
};

struct platform_device tegra_sdhci_device0 = {
	.name		= "sdhci-tegra",
	.id		= 0,
	.resource	= sdhci_resource0,
	.num_resources	= ARRAY_SIZE(sdhci_resource0),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data0,
	},
};

static struct platform_device tegra_sdhci_device2 = {
	.name		= "sdhci-tegra",
	.id		= 2,
	.resource	= sdhci_resource2,
	.num_resources	= ARRAY_SIZE(sdhci_resource2),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data2,
	},
};

static struct platform_device tegra_sdhci_device3 = {
	.name		= "sdhci-tegra",
	.id		= 3,
	.resource	= sdhci_resource3,
	.num_resources	= ARRAY_SIZE(sdhci_resource3),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data3,
	},
};

static int __init n1_wifi_init(void)
{
	wifi_32k_clk = clk_get_sys(NULL, "blink");
	if (IS_ERR(wifi_32k_clk)) {
		pr_err("%s: unable to get blink clock\n", __func__);
		return PTR_ERR(wifi_32k_clk);
	}

	if (system_rev == 0 || system_rev == 1) {
		gpio_request(TEGRA_GPIO_PQ2, "wlan_power");
		tegra_gpio_enable(TEGRA_GPIO_PQ2);
		gpio_direction_output(TEGRA_GPIO_PQ2, 0);
	} else {
		gpio_request(GPIO_WLAN_EN, "wlan_power");
		tegra_gpio_enable(GPIO_WLAN_EN);
		gpio_direction_output(GPIO_WLAN_EN, 0);
	}

	return platform_device_register(&n1_wifi_device);
}

int __init n1_sdhci_init(void)
{
	if(system_rev >= BOARD_REV08)
#if defined CONFIG_MACH_BOSE_ATT
		tegra_sdhci_platform_data2.cd_gpio = 0xffff;
#else
		tegra_sdhci_platform_data2.cd_gpio = -1;
#endif
	platform_device_register(&tegra_sdhci_device3);
	platform_device_register(&tegra_sdhci_device2);
	platform_device_register(&tegra_sdhci_device0);

	device_init_wakeup(&tegra_sdhci_device0.dev, 1);
	device_set_wakeup_enable(&tegra_sdhci_device0.dev, 0);

	tegra_sdhci_device0_ptr = &tegra_sdhci_device0;

	n1_wifi_init();
	return 0;
}

EXPORT_SYMBOL(tegra_sdhci_device0_ptr);
