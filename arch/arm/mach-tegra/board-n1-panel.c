/*
 * arch/arm/mach-tegra/board-n1-panel.c
 *
 * Copyright (c) 2010, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/resource.h>
#include <asm/mach-types.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/kernel.h>
#include <linux/pwm_backlight.h>
#include <linux/spi/spi.h>
#include <mach/nvhost.h>
#include <mach/nvmap.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/dc.h>
#include <mach/fb.h>
#include <mach/tegra_cpufreq.h>

#include "devices.h"
#include "gpio-names.h"
#include "board.h"

#define n1_ld9040 0

/* TODO: find GPIO for hdmi */
/*#define n1_hdmi_hpd	TEGRA_GPIO_PN7*/
/*#define n1_hdmi_enb	TEGRA_GPIO_PV5*/

#if 0
static struct regulator *n1_hdmi_reg = NULL;
static struct regulator *n1_hdmi_pll = NULL;
#endif

static int n1_hdmi_enable(void)
{
#if 0
	if (!n1_hdmi_reg) {
		n1_hdmi_reg = regulator_get(NULL, "AVDD_HDMI_3V3");
		if (IS_ERR_OR_NULL(n1_hdmi_reg)) {
			pr_err("hdmi: couldn't get regulator AVDD_HDMI_3V3\n");
			n1_hdmi_reg = NULL;
			return PTR_ERR(n1_hdmi_reg);
		}
	}
	regulator_enable(n1_hdmi_reg);

	if (!n1_hdmi_pll) {
		n1_hdmi_pll = regulator_get(NULL, "AVDD_HDMI_PLL_1V8");
		if (IS_ERR_OR_NULL(n1_hdmi_pll)) {
			pr_err("hdmi: couldn't get regulator AVDD_HDMI_PLL_1V8\n");
			n1_hdmi_pll = NULL;
			regulator_disable(n1_hdmi_reg);
			n1_hdmi_reg = NULL;
			return PTR_ERR(n1_hdmi_pll);
		}
	}
	regulator_enable(n1_hdmi_pll);
#endif
	return 0;
}

static int n1_hdmi_disable(void)
{
#if 0
	regulator_disable(n1_hdmi_reg);
	regulator_disable(n1_hdmi_pll);
#endif

	return 0;
}

static struct resource n1_disp1_resources[] = {
	{
		.name	= "irq",
		.start	= INT_DISPLAY_GENERAL,
		.end	= INT_DISPLAY_GENERAL,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name	= "regs",
		.start	= TEGRA_DISPLAY_BASE,
		.end	= TEGRA_DISPLAY_BASE + TEGRA_DISPLAY_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "fbmem",
		.flags	= IORESOURCE_MEM,
	},
};
/*
static struct resource n1_disp2_resources[] = {
	{
		.name	= "irq",
		.start	= INT_DISPLAY_B_GENERAL,
		.end	= INT_DISPLAY_B_GENERAL,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name	= "regs",
		.start	= TEGRA_DISPLAY2_BASE,
		.end	= TEGRA_DISPLAY2_BASE + TEGRA_DISPLAY2_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "fbmem",
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "hdmi_regs",
		.start	= TEGRA_HDMI_BASE,
		.end	= TEGRA_HDMI_BASE + TEGRA_HDMI_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};
*/

#if n1_ld9040
static struct tegra_dc_mode n1_panel_modes[] = {
	{
		.pclk = 25270000,
		.h_ref_to_sync = 0,//.h_ref_to_sync = 11,
		.v_ref_to_sync = 0,//.v_ref_to_sync = 1,	
		.h_sync_width = 2,
		.v_sync_width = 2,
		.h_back_porch = 14,
		.v_back_porch = 4,//.v_back_porch = 6,
		.h_active = 480,
		.v_active = 800,
		.h_front_porch = 16,
		.v_front_porch = 10,//.v_front_porch = 4,
	},
};
#else
static struct tegra_dc_mode n1_panel_modes[] = {
	{
		.flags = TEGRA_DC_MODE_FLAG_NEG_V_SYNC | TEGRA_DC_MODE_FLAG_NEG_H_SYNC,
		.pclk = 24000000,
		//.pclk = 27000000,
		.h_ref_to_sync = 0,//.h_ref_to_sync = 11,
		.v_ref_to_sync = 1,//.v_ref_to_sync = 1,	
		.h_sync_width = 10,
		.v_sync_width = 2,
		.h_back_porch = 20,
		.v_back_porch = 4,//.v_back_porch = 6,
		.h_active = 480,
		.v_active = 800,
		.h_front_porch = 10,
		.v_front_porch = 3,//.v_front_porch = 4,
	},
};
#endif

static struct tegra_fb_data n1_fb_data = {
	.win		= 0,
	.xres		= 480,
	.yres		= 800,
	.bits_per_pixel	= 32,
};

/*
static struct tegra_fb_data n1_hdmi_fb_data = {
	.win		= 0,
	.xres		= 480,
	.yres		= 800,
	.bits_per_pixel	= 32,
};
*/
	
#if n1_ld9040
static struct tegra_dc_out_pin n1_dc_out_pins[] = {
	{
		.name	= TEGRA_DC_OUT_PIN_DATA_ENABLE,
		.pol	= TEGRA_DC_OUT_PIN_POL_LOW,
	},
	{
		.name	= TEGRA_DC_OUT_PIN_H_SYNC,
		.pol	= TEGRA_DC_OUT_PIN_POL_LOW,
	},
	{
		.name	= TEGRA_DC_OUT_PIN_V_SYNC,
		.pol	= TEGRA_DC_OUT_PIN_POL_LOW,
	},
	{
		.name	= TEGRA_DC_OUT_PIN_PIXEL_CLOCK,
		.pol	= TEGRA_DC_OUT_PIN_POL_LOW,
	},
};
#else
static struct tegra_dc_out_pin n1_dc_out_pins[] = {
	{
		.name	= TEGRA_DC_OUT_PIN_DATA_ENABLE,
		.pol	= TEGRA_DC_OUT_PIN_POL_HIGH,
	},
	{
		.name	= TEGRA_DC_OUT_PIN_H_SYNC,
		.pol	= TEGRA_DC_OUT_PIN_POL_LOW,
	},
	{
		.name	= TEGRA_DC_OUT_PIN_V_SYNC,
		.pol	= TEGRA_DC_OUT_PIN_POL_LOW,
	},
	{
		.name	= TEGRA_DC_OUT_PIN_PIXEL_CLOCK,
		.pol	= TEGRA_DC_OUT_PIN_POL_LOW,
	},
};
#endif

static struct tegra_dc_out n1_disp1_out = {
	.type		= TEGRA_DC_OUT_RGB,

	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,

	.modes		= n1_panel_modes,
	.n_modes	= ARRAY_SIZE(n1_panel_modes),

	.out_pins	= n1_dc_out_pins,
	.n_out_pins	= ARRAY_SIZE(n1_dc_out_pins),
};

/*
static struct tegra_dc_out n1_disp2_out = {
	.type		= TEGRA_DC_OUT_HDMI,
	.flags		= TEGRA_DC_OUT_HOTPLUG_HIGH,

	.dcc_bus	= 13,
	.hotplug_gpio	= n1_hdmi_hpd,

	.align		= TEGRA_DC_ALIGN_MSB,
	.order		= TEGRA_DC_ORDER_RED_BLUE,

	.enable		= n1_hdmi_enable,
	.disable	= n1_hdmi_disable,
};
*/

static struct tegra_dc_platform_data n1_disp1_pdata = {
	.flags		= TEGRA_DC_FLAG_ENABLED,
	.default_out	= &n1_disp1_out,
	.fb		= &n1_fb_data,
};

/*
static struct tegra_dc_platform_data n1_disp2_pdata = {
	.flags		= 0,
	.default_out	= &n1_disp2_out,
	.fb		= &n1_hdmi_fb_data,
};
*/

static struct nvhost_device n1_disp1_device = {
	.name		= "tegradc",
	.id		= 0,
	.resource	= n1_disp1_resources,
	.num_resources	= ARRAY_SIZE(n1_disp1_resources),
	.dev = {
		.platform_data = &n1_disp1_pdata,
	},
};

/*
static struct nvhost_device n1_disp2_device = {
	.name		= "tegradc",
	.id		= 1,
	.resource	= n1_disp2_resources,
	.num_resources	= ARRAY_SIZE(n1_disp2_resources),
	.dev = {
		.platform_data = &n1_disp2_pdata,
	},
};
*/

static struct nvmap_platform_carveout n1_carveouts[] = {
	[0] = {
		.name		= "iram",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_IRAM,
		.base		= TEGRA_IRAM_BASE,
		.size		= TEGRA_IRAM_SIZE,
		.buddy_size	= 0, /* no buddy allocation for IRAM */
	},
	[1] = {
		.name		= "generic-0",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_GENERIC,
		.buddy_size	= SZ_32K,
	},
};

static struct nvmap_platform_data n1_nvmap_data = {
	.carveouts	= n1_carveouts,
	.nr_carveouts	= ARRAY_SIZE(n1_carveouts),
};

static struct platform_device n1_nvmap_device = {
	.name	= "tegra-nvmap",
	.id	= -1,
	.dev	= {
		.platform_data = &n1_nvmap_data,
	},
};
static struct platform_device n1_device_cmc623 = {
	.name	= "sec_cmc623",
	.id	= -1,
};
static struct platform_device n1_baklight_device = {
	.name	= "cmc623_pwm_bl",
	.id	= -1,
};
static struct spi_board_info n1_spi_device2[] = {
	{
		.modalias = "panel_n1_spi",
		.bus_num = 2,
		.chip_select = 2,
		.max_speed_hz = 1000000,
		.mode = SPI_MODE_3,
	},
};

static struct platform_device *n1_gfx_devices[] __initdata = {
	&n1_nvmap_device,
	&tegra_spi_device3,
	&tegra_grhost_device,
	&n1_baklight_device,
	&n1_device_cmc623,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
/* put early_suspend/late_resume handlers here for the display in order
 * to keep the code out of the display driver, keeping it closer to upstream
 */
struct early_suspend n1_panel_early_suspender;

static void n1_panel_early_suspend(struct early_suspend *h)
{
	printk(KERN_INFO "\n ************ %s : %d\n", __func__, __LINE__);

	if (num_registered_fb > 0)
		fb_blank(registered_fb[0], FB_BLANK_POWERDOWN);
#ifdef CONFIG_CPU_FREQ
	cpufreq_save_default_governor();
	cpufreq_set_conservative_governor(CPUFREQ_DISP_MODE);
	cpufreq_set_conservative_governor_param(
		SET_CONSERVATIVE_GOVERNOR_UP_THRESHOLD,
		SET_CONSERVATIVE_GOVERNOR_DOWN_THRESHOLD);
#endif
}

static void n1_panel_late_resume(struct early_suspend *h)
{
	printk(KERN_INFO "\n ************ %s : %d\n", __func__, __LINE__);

	if (num_registered_fb > 0)
		fb_blank(registered_fb[0], FB_BLANK_UNBLANK);

#ifdef CONFIG_CPU_FREQ
	cpufreq_restore_default_governor(CPUFREQ_DISP_MODE);
#endif
}
#endif

int __init n1_panel_init(void)
{
	int err;
	struct resource *res;

/*	tegra_gpio_enable(n1_hdmi_enb);
	gpio_request(n1_hdmi_enb, "hdmi_5v_en");
	gpio_direction_output(n1_hdmi_enb, 1);

	tegra_gpio_enable(n1_hdmi_hpd);
	gpio_request(n1_hdmi_hpd, "hdmi_hpd");
	gpio_direction_input(n1_hdmi_hpd);
*/
	n1_carveouts[1].base = tegra_carveout_start;
	n1_carveouts[1].size = tegra_carveout_size;

#ifdef CONFIG_HAS_EARLYSUSPEND
	n1_panel_early_suspender.suspend = n1_panel_early_suspend;
	n1_panel_early_suspender.resume = n1_panel_late_resume;
	n1_panel_early_suspender.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&n1_panel_early_suspender);
#endif

	err = platform_add_devices(n1_gfx_devices,
				   ARRAY_SIZE(n1_gfx_devices));

	res = nvhost_get_resource_byname(&n1_disp1_device,
		IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb_start;
	res->end = tegra_fb_start + tegra_fb_size - 1;

/*	res = nvhost_get_resource_byname(&n1_disp2_device,
		IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb2_start;
	res->end = tegra_fb2_start + tegra_fb2_size - 1;
*/
	spi_register_board_info(n1_spi_device2, ARRAY_SIZE(n1_spi_device2));

	if (!err)
		err = nvhost_device_register(&n1_disp1_device);

/*	if (!err)
		err = nvhost_device_register(&n1_disp2_device);
*/
	return err;
}

