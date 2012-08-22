/*
 * Copyright (C) 2010-2011 NVIDIA, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/device.h>

#include <mach/clk.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/pinmux.h>
#include <mach/iomap.h>
#include <mach/io.h>
#include <mach/kbc.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>


#define N1_ROW_COUNT	16
#define N1_COL_COUNT	7

static const u32 n1_keymap[] = {
	KEY(0, 1, KEY_VOLUMEUP),
	KEY(1, 1, KEY_VOLUMEDOWN),
};

static const struct matrix_keymap_data n1_keymap_data = {
	.keymap = n1_keymap,
	.keymap_size = ARRAY_SIZE(n1_keymap),
};

static struct tegra_kbc_wake_key n1_wake_cfg[] = {
	[0] = {
		.row = 0,
		.col = 0,
	},
};

static struct tegra_kbc_platform_data n1_kbc_platform_data = {
	.debounce_cnt = 20,
	.repeat_cnt = 50 * 32,
	.wake_cnt = 1,
	.wake_cfg = &n1_wake_cfg[0],
	.keymap_data = &n1_keymap_data,
	.use_fn_map = false,
	.wakeup = false,
#ifdef CONFIG_ANDROID
	.disable_ev_rep = true,
#endif
};

static struct resource n1_kbc_resources[] = {
	[0] = {
		.start = TEGRA_KBC_BASE,
		.end   = TEGRA_KBC_BASE + TEGRA_KBC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_KBC,
		.end   = INT_KBC,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device n1_kbc_device = {
	.name = "tegra-kbc",
	.id = -1,
	.dev = {
		.platform_data = &n1_kbc_platform_data,
	},
	.resource = n1_kbc_resources,
	.num_resources = ARRAY_SIZE(n1_kbc_resources),
};

int __init n1_kbc_init(void)
{
	struct tegra_kbc_platform_data *data = &n1_kbc_platform_data;
	int i;

	pr_info("KBC: n1_kbc_init\n");
	for (i = 0; i < N1_ROW_COUNT; i++) {
		data->pin_cfg[i].num = i;
		data->pin_cfg[i].is_row = true;
		data->pin_cfg[i].en = true;
	}
	for (i = 0; i < N1_COL_COUNT; i++) {
		data->pin_cfg[i + KBC_PIN_GPIO_16].num = i;
		data->pin_cfg[i + KBC_PIN_GPIO_16].en = true;
	}

	platform_device_register(&n1_kbc_device);
	return 0;
}
