/*
 * arch/arm/mach-tegra/board-n1-sensors.c
 *
 * Copyright (c) 2010, NVIDIA, All Rights Reserved.
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

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/max17043_battery.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/mfd/wm8994/wm8994_pdata.h>
#include <linux/sec_jack.h>
#include <mach/gpio.h>

#include "gpio-names.h"
#include <mach/gpio-n1.h>
#include <linux/fsa9480.h>

#include <linux/switch.h>
#include <linux/power_supply.h>
#include <mach/sec_battery.h>
#include <linux/nct1008.h>
#include <mach/pinmux.h>
#include <mach/iomap.h>
#include <mach/io.h>
#include <asm/io.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/usb/otg.h>
#include <linux/delay.h>
#include <mach/usb_phy.h>
#include <mach/thermal.h>
#include "board-n1.h"
#include <mach/tegra_das.h>
#include <mach/otg_def.h>

#define GPIO_AUDIO_I2C_SDA TEGRA_GPIO_PG3
#define GPIO_AUDIO_I2C_SCL TEGRA_GPIO_PI0
#define DEV_USB_OTG		(1 << 7)

struct nct1008_temp_callbacks *callbacks;

/* thermal monitor */
static struct i2c_gpio_platform_data tegra_gpio_i2c15_pdata = {
	.sda_pin = GPIO_THERMAL_I2C_SDA,
	.scl_pin = GPIO_THERMAL_I2C_SCL,
	.udelay = 1, /* 200 kHz */
	.timeout = 0, /* jiffies */
};

static struct platform_device tegra_gpio_i2c15_device = {
	.name = "i2c-gpio",
	.id = 15,
	.dev = {
		.platform_data = &tegra_gpio_i2c15_pdata,
	}
};

/* image converter */
static struct i2c_gpio_platform_data tegra_gpio_i2c10_pdata = {
	.sda_pin = GPIO_CMC_I2C_SDA,
	.scl_pin = GPIO_CMC_I2C_SCL,
	.udelay = 1, /* 200 kHz */
	.timeout = 0, /* jiffies */
};

static struct platform_device tegra_gpio_i2c10_device = {
	.name = "i2c-gpio",
	.id = 10,
	.dev = {
		.platform_data = &tegra_gpio_i2c10_pdata,
	}
};

static struct i2c_gpio_platform_data tegra_gpio_i2c9_pdata = {
	.sda_pin = GPIO_ALC_I2C_SDA,
	.scl_pin = GPIO_ALC_I2C_SCL,
	.udelay = 2,    /* 250KHz */
};

static struct platform_device tegra_gpio_i2c9_device = {
	.name = "i2c-gpio",
	.id = 9,
	.dev.platform_data = &tegra_gpio_i2c9_pdata,
};

/* audio codec */
static struct i2c_gpio_platform_data tegra_gpio_i2c8_pdata = {
	.sda_pin = GPIO_CODEC_I2C_SDA,
	.scl_pin = GPIO_CODEC_I2C_SCL,
	.udelay = 1, /* 200 kHz */
	.timeout = 0, /* jiffies */
};

static struct platform_device tegra_gpio_i2c8_device = {
	.name = "i2c-gpio",
	.id = 8,
	.dev = {
		.platform_data = &tegra_gpio_i2c8_pdata,
	}
};

static void *das_base = IO_ADDRESS(TEGRA_APB_MISC_BASE);

static inline unsigned long das_readl(unsigned long offset)
{
	return readl(das_base + offset);
}

static inline void das_writel(unsigned long value, unsigned long offset)
{
	writel(value, das_base + offset);
}

static void tegra_set_dap_connection(u8 value)
{
	int reg_val;

	pr_info("Board N1 : %s : %d\n", __func__, value);

	switch (value) {
	case dap_connection_codec_slave: /* pcm_slave */
		das_writel(DAP_CTRL_SEL_DAP3,
			APB_MISC_DAS_DAP_CTRL_SEL_1);
		das_writel((DAP_MS_SEL_MASTER | DAP_CTRL_SEL_DAP2),
			APB_MISC_DAS_DAP_CTRL_SEL_2);
		break;
	case dap_connection_codec_master: /* pcm_master */
		das_writel(DAP_MS_SEL_MASTER |
			DAP_CTRL_SEL_DAP3, APB_MISC_DAS_DAP_CTRL_SEL_1);
		das_writel((DAP_CTRL_SEL_DAP2),
			APB_MISC_DAS_DAP_CTRL_SEL_2);
		break;
	case dap_connection_bt_call:
		/* DAP1 */
		reg_val = das_readl(APB_MISC_DAS_DAP_CTRL_SEL_0);
		reg_val &= ~(DAP_MS_SEL_DEFAULT_MASK
			<< DAP_MS_SEL_SHIFT);
		reg_val |= (1 << DAP_MS_SEL_SHIFT); /* DAP1 master */
		reg_val &= ~(DAP_CTRL_SEL_DEFAULT_MASK
			<< DAP_CTRL_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAC1
			<< DAP_CTRL_SEL_SHIFT); /* DAP1<-DAC1 */
		das_writel(reg_val, APB_MISC_DAS_DAP_CTRL_SEL_0);

		/* DAP2 */
		reg_val = das_readl(APB_MISC_DAS_DAP_CTRL_SEL_1);
		reg_val &= ~(DAP_MS_SEL_DEFAULT_MASK
			<< DAP_MS_SEL_SHIFT);
		reg_val |= (0 << DAP_MS_SEL_SHIFT); /* DAP2 slave */
		reg_val &= ~(DAP_CTRL_SEL_DEFAULT_MASK
			<< DAP_CTRL_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAP3
			<< DAP_CTRL_SEL_SHIFT); /*DAP2<-DAP3 */
		das_writel(reg_val, APB_MISC_DAS_DAP_CTRL_SEL_1);

		/* DAP3 */
		reg_val = das_readl(APB_MISC_DAS_DAP_CTRL_SEL_2);
		reg_val &= ~(DAP_MS_SEL_DEFAULT_MASK
			<< DAP_MS_SEL_SHIFT);
		reg_val |= (1 << DAP_MS_SEL_SHIFT); /* DAP3 master */
		reg_val &= ~(DAP_CTRL_SEL_DEFAULT_MASK
			<< DAP_CTRL_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAP4
			<< DAP_CTRL_SEL_SHIFT); /* DAP3<-DAP4 */
		das_writel(reg_val, APB_MISC_DAS_DAP_CTRL_SEL_2);

		/* DAP4 */
		reg_val = das_readl(APB_MISC_DAS_DAP_CTRL_SEL_3);
		reg_val &= ~(DAP_MS_SEL_DEFAULT_MASK
			<< DAP_MS_SEL_SHIFT);
		reg_val |= (0 << DAP_MS_SEL_SHIFT); /* DAP4 slave */
		reg_val &= ~(DAP_CTRL_SEL_DEFAULT_MASK
			<< DAP_CTRL_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAP2
			<< DAP_CTRL_SEL_SHIFT); /* DAP4<-DAP2 */
		das_writel(reg_val, APB_MISC_DAS_DAP_CTRL_SEL_3);

		/* DAC1 */
		reg_val =
			das_readl(APB_MISC_DAS_DAC_INPUT_DATA_CLK_SEL_0);
		reg_val &= ~(DAC_SDATA2_SEL_DEFAULT_MASK
			<< DAC_SDATA2_SEL_SHIFT);
		reg_val |= ((DAP_CTRL_SEL_DAP1 - DAP_CTRL_SEL_DAP1)
			<< DAC_SDATA2_SEL_SHIFT); /* DAC1 <- DAP1 */
		reg_val &= ~(DAC_SDATA1_SEL_DEFAULT_MASK
			<< DAC_SDATA1_SEL_SHIFT);
		reg_val |= ((DAP_CTRL_SEL_DAP1 - DAP_CTRL_SEL_DAP1)
			<< DAC_SDATA1_SEL_SHIFT); /* DAC1 <- DAP1 */
		reg_val &= ~(DAC_CLK_SEL_DEFAULT_MASK
			<< DAC_CLK_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAP1
			<< DAC_CLK_SEL_SHIFT); /*  DAC1 <- DAP1 */
		das_writel(reg_val,
			APB_MISC_DAS_DAC_INPUT_DATA_CLK_SEL_0);

		break;
	case dap_connection_bt_voip:
		/* DAP1 */
		reg_val = das_readl(APB_MISC_DAS_DAP_CTRL_SEL_0);
		reg_val &= ~(DAP_MS_SEL_DEFAULT_MASK
			<< DAP_MS_SEL_SHIFT);
		reg_val |= (1 << DAP_MS_SEL_SHIFT); /* DAP1 master */
		reg_val &= ~(DAP_CTRL_SEL_DEFAULT_MASK
			<< DAP_CTRL_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAC1
			<< DAP_CTRL_SEL_SHIFT); /* DAP1<-DAC1 */
		das_writel(reg_val, APB_MISC_DAS_DAP_CTRL_SEL_0);

		/* DAP2 */
		reg_val = das_readl(APB_MISC_DAS_DAP_CTRL_SEL_1);
		reg_val &= ~(DAP_MS_SEL_DEFAULT_MASK
			<< DAP_MS_SEL_SHIFT);
		reg_val |= (1 << DAP_MS_SEL_SHIFT); /* DAP2 slave */
		reg_val &= ~(DAP_CTRL_SEL_DEFAULT_MASK
			<< DAP_CTRL_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAP4
			<< DAP_CTRL_SEL_SHIFT); /* DAP2<-DAP3 */
		das_writel(reg_val, APB_MISC_DAS_DAP_CTRL_SEL_1);

		/* DAP4 */
		reg_val = das_readl(APB_MISC_DAS_DAP_CTRL_SEL_3);
		reg_val &= ~(DAP_MS_SEL_DEFAULT_MASK
			<< DAP_MS_SEL_SHIFT);
		reg_val |= (0 << DAP_MS_SEL_SHIFT); /* DAP4 slave */
		reg_val &= ~(DAP_CTRL_SEL_DEFAULT_MASK
			<< DAP_CTRL_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAP2
			<< DAP_CTRL_SEL_SHIFT); /* DAP4<-DAP2 */
		das_writel(reg_val, APB_MISC_DAS_DAP_CTRL_SEL_3);

		/* DAC1 */
		reg_val = das_readl(
			APB_MISC_DAS_DAC_INPUT_DATA_CLK_SEL_0);
		reg_val &= ~(DAC_SDATA2_SEL_DEFAULT_MASK
			<< DAC_SDATA2_SEL_SHIFT);
		reg_val |= ((DAP_CTRL_SEL_DAP1 - DAP_CTRL_SEL_DAP1)
			<< DAC_SDATA2_SEL_SHIFT); /* DAC1 <- DAP1 */
		reg_val &= ~(DAC_SDATA1_SEL_DEFAULT_MASK
			<< DAC_SDATA1_SEL_SHIFT);
		reg_val |= ((DAP_CTRL_SEL_DAP1 - DAP_CTRL_SEL_DAP1)
			<< DAC_SDATA1_SEL_SHIFT); /* DAC1 <- DAP1 */
		reg_val &= ~(DAC_CLK_SEL_DEFAULT_MASK
			<< DAC_CLK_SEL_SHIFT);
		reg_val |= (DAP_CTRL_SEL_DAP1
			<< DAC_CLK_SEL_SHIFT); /* DAC1 <- DAP1 */
		das_writel(reg_val, APB_MISC_DAS_DAC_INPUT_DATA_CLK_SEL_0);
		break;
	case dap_connection_bt_call_nomix:
		das_writel(DAP_CTRL_SEL_DAP3,
			APB_MISC_DAS_DAP_CTRL_SEL_1);
		das_writel(DAP_CTRL_SEL_DAP3,
			APB_MISC_DAS_DAP_CTRL_SEL_3);
		das_writel((DAP_MS_SEL_MASTER |
			DAP_CTRL_SEL_DAP2 | DAP_CTRL_SEL_DAP4),
			APB_MISC_DAS_DAP_CTRL_SEL_2);
		break;
	default:
		pr_err("Board N1 : %s : %d is not available\n",
					__func__, value);
		break;
	}
}

static struct wm8994_platform_data wm8994_pdata = {
	.ldo = GPIO_CODEC_LDO_EN,
	.set_mic_bias = wm8994_set_mic_bias,
	.set_sub_mic_bias = wm8994_set_sub_mic_bias,
	.set_ear_sel = wm8994_set_ear_sel,
	.set_dap_connection = tegra_set_dap_connection,
	.wm8994_submic_state = WM8994_SUB_MIC_OFF,
};

void wm8994_set_mic_bias(bool on)
{
	pr_info("Board N1 : Enterring %s : on %d\n", __func__, on);
	gpio_set_value(GPIO_MICBIAS1_EN, on);
}

void wm8994_set_sub_mic_bias(bool on)
{
	pr_info("Board N1 : Enterring %s : on %d\n", __func__, on);

	if (wm8994_pdata.wm8994_submic_state && !on) {
		pr_info("%s : do not disable sub_mic_bias" \
			"during recording or voicecall\n", __func__);
		return;
	}

	if (sec_jack_pdata.jack_status == SEC_HEADSET_4POLE && !on) {
		pr_info("%s : do not disable sub_mic_bias" \
			"during 4pole headset connected\n", __func__);
		return;
	}
	gpio_set_value(GPIO_MICBIAS2_EN, on);
}

void wm8994_set_ear_sel(bool on)
{
	pr_info("Board N1 : Enterring %s : on %d\n", __func__, on);
	gpio_set_value(GPIO_EAR_SEL, on);
}


static struct i2c_board_info sec_gpio_i2c8_info[] = {
	{
		I2C_BOARD_INFO("wm8994", 0x34 >> 1),
		.platform_data = &wm8994_pdata,
	},
};

/* compass */
static  struct  i2c_gpio_platform_data  gpio_i2c7_platdata = {
	.sda_pin                = TEGRA_GPIO_PO4,
	.scl_pin                = TEGRA_GPIO_PO2,
	.udelay                 = 2,    /* 250KHz */
};

static struct platform_device n1_device_gpio_i2c7 = {
	.name                   = "i2c-gpio",
	.id                     = 7,
	.dev.platform_data      = &gpio_i2c7_platdata,
};

static void n1_fmradio_init(int lpm_mode)
{
	int rst_gpio = 0;
	pr_info("%s :+++\n", __func__);

	tegra_gpio_enable(GPIO_FM_INT);
	gpio_request(GPIO_FM_INT, "fm_int");
	gpio_direction_input(GPIO_FM_INT);

	if (system_rev <= 4)
		rst_gpio = GPIO_FM_RST_04;
	else
		rst_gpio = GPIO_FM_RST_05;

	tegra_gpio_enable(rst_gpio);
	gpio_request(rst_gpio, "fm_rst");
	if (lpm_mode && system_rev < 7)
		gpio_direction_output(rst_gpio, 1);
	else
		gpio_direction_output(rst_gpio, 0);
}

/* fmradio : si4709 */
static  struct  i2c_gpio_platform_data  gpio_i2c6_platdata = {
	.sda_pin                = GPIO_FM_SDA_18V,
	.scl_pin                = GPIO_FM_SCL_18V,
	.udelay                 = 2,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device n1_device_gpio_i2c6 = {
	.name                   = "i2c-gpio",
	.id                     = 6,
	.dev.platform_data      = &gpio_i2c6_platdata,
};

static struct i2c_gpio_platform_data fuelgauge_gpio_i2c_pdata = {
	.sda_pin		= TEGRA_GPIO_PO0,
	.scl_pin		= TEGRA_GPIO_PO7,
	.udelay			= 1, /* 200 kHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device fuelgauge_gpio_i2c_device = {
	.name				= "i2c-gpio",
	.id				= 5,
	.dev.platform_data		= &fuelgauge_gpio_i2c_pdata,
};

static int charger_online(void)
{
	struct power_supply *psp = power_supply_get_by_name("charger");
	union power_supply_propval val;
	int ret = 0;
	if (!psp) {
		pr_err("%s: fail to get charger ps\n", __func__);
		return 0;
	}
	ret = psp->get_property(psp, POWER_SUPPLY_PROP_ONLINE, &val);
	if (ret)
		return 0;
	return val.intval;
}

static int charger_enable(void)
{
	struct power_supply *psp = power_supply_get_by_name("charger");
	union power_supply_propval val;
	int ret = 0;
	if (!psp) {
		pr_err("%s: fail to get charger ps\n", __func__);
		return 0;
	}
	ret = psp->get_property(psp, POWER_SUPPLY_PROP_STATUS, &val);
	if (ret)
		return 0;
	return val.intval == POWER_SUPPLY_STATUS_CHARGING;
}

static int max8907c_check_vchg(void)
{
	struct power_supply *psp = power_supply_get_by_name("max8907c-charger");
	union power_supply_propval val;

	if (!psp) {
		pr_err("%s: fail to get charger ps\n", __func__);
		return -ENODEV;
		}

	psp->get_property(psp, POWER_SUPPLY_PROP_STATUS, &val);

	return val.intval;
}
static int max17043_low_batt_cb(void)
{
	struct power_supply *psy = power_supply_get_by_name("battery");
	union power_supply_propval value;

	if (!psy) {
		pr_err("%s: fail to get battery ps\n", __func__);
		return -ENODEV;
	}

	value.intval = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	return psy->set_property(psy, POWER_SUPPLY_PROP_CAPACITY_LEVEL, &value);
}
/*	2011.03.09 1650mAh SDI from MAXIM
 *	Test Condition
 *	Cell : MV504657M, 1650mAh
 *	Charging : 4.2V, 600mA, 60mA cutoff
 *	Discharging : 300mA, 3.4V cutoff
 */
static struct max17043_platform_data max17043_pdata = {
	.alert_flag = 0x1F,			/* 1% fuel alert */
	.charging_rcomp = 0xE7,	/* modified 2011.05.09 by MAXIM */
	.discharging_rcomp = 0xD7,
	.standard_temp = 20,
	.comp_full = 9680,			/* 2011.06.15 modified to 96.8 becase of topoff current changing */
	.comp_empty = 0,			/* 0 */
	.low_batt_cb = max17043_low_batt_cb,
};

static struct i2c_board_info __initdata n1_fuelgauge[] = {
	{
		I2C_BOARD_INFO("max17043", 0x36),
		.platform_data = &max17043_pdata,
		.irq = TEGRA_GPIO_TO_IRQ(GPIO_FUEL_ALERT),
	},
};

static void n1_max17043_gpio_init(void)
{
	tegra_gpio_enable(GPIO_FUEL_ALERT);
	gpio_request(GPIO_FUEL_ALERT, "fuel_alert");
	gpio_direction_input(GPIO_FUEL_ALERT);
}


extern enum cable_type_t set_cable_status;

struct gpioi2c_otg_data otg_data;

void fsa9480_otg_data_init(struct otg_id_open_data *o,
				struct otg_detect_data *d, void (*vbus_en)(int))
{
	otg_data.otg_open = o;
	otg_data.otg_clk_data = d;
	otg_data.vbus_en = vbus_en;
}

static void fsa9480_otg_cb(bool attached)
{
	struct power_supply *psy = power_supply_get_by_name("battery");
	union power_supply_propval value;
	pr_info("%s : attached(%d)\n", __func__, attached);

	if (attached) {
		set_cable_status = CABLE_TYPE_NONE;
		value.intval = POWER_SUPPLY_TYPE_BATTERY;
		if (otg_data.otg_clk_data->check_detection
				&& otg_data.otg_clk_data->clk_cause)
			otg_data.otg_clk_data->check_detection
				(otg_data.otg_clk_data->clk_cause, HOST_CAUSE);
		else
			pr_err("%s: check_detection or clk_cause is null\n",
						 __func__);
	} else {
		set_cable_status = CABLE_TYPE_NONE;
		value.intval = POWER_SUPPLY_TYPE_BATTERY;
	}

	if (!psy) {
		pr_err("%s: fail to get battery ps\n", __func__);
		return ;
	}

	psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);

}

static void fsa9480_usb_cb(bool attached)
{
	struct power_supply *psy = power_supply_get_by_name("battery");
	union power_supply_propval value;
	pr_info("%s : attached(%d)\n", __func__, attached);

	if (attached && !otg_data.otg_open->otg_enabled) {
		set_cable_status = CABLE_TYPE_USB;
		value.intval = POWER_SUPPLY_TYPE_USB;
	} else {
		set_cable_status = CABLE_TYPE_NONE;
		value.intval = POWER_SUPPLY_TYPE_BATTERY;
	}

	if (!psy) {
		pr_err("%s: fail to get battery ps\n", __func__);
		return ;
	}

	psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);

}

static void fsa9480_charger_ovp_cb(bool ovp)
{
	struct power_supply *psy = power_supply_get_by_name("battery");
	union power_supply_propval value;
	pr_info("%s : ovp(%d)\n", __func__, ovp);

	if (ovp == FSA9480_OVP) {
		value.intval = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	} else if (ovp == FSA9480_RECOVER_OVP) {
		value.intval = POWER_SUPPLY_HEALTH_GOOD;
	}

	if (!psy) {
		pr_err("%s: fail to get battery ps\n", __func__);
		return ;
	}

	psy->set_property(psy, POWER_SUPPLY_PROP_HEALTH, &value);
}

static void fsa9480_charger_cb(bool attached)
{
	struct power_supply *psy = power_supply_get_by_name("battery");
	union power_supply_propval value;
	pr_info("%s : attached(%d)\n", __func__, attached);

	if (attached) {
		set_cable_status = CABLE_TYPE_AC;
		value.intval = POWER_SUPPLY_TYPE_MAINS;
	} else {
		set_cable_status = CABLE_TYPE_NONE;
		value.intval = POWER_SUPPLY_TYPE_BATTERY;
	}

	if (!psy) {
		pr_err("%s: fail to get battery ps\n", __func__);
		return ;
	}

	psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
}

static int fsa9480_uart_cb(bool attached)
{
	int check_vcgh;

	check_vcgh = max8907c_check_vchg();

	if (attached == FSA9480_ATTACHED && check_vcgh == POWER_SUPPLY_STATUS_CHARGING)
	{
		fsa9480_charger_cb(FSA9480_ATTACHED);
		return true;
	}else if(attached == FSA9480_DETACHED &&
			check_vcgh == POWER_SUPPLY_STATUS_NOT_CHARGING &&
			set_cable_status == CABLE_TYPE_AC ) {
		fsa9480_charger_cb(FSA9480_DETACHED);
		return true;
	}
	return false;
}
static struct switch_dev switch_dock = {
	.name = "dock",
};

/* Dock and Charger (ta or usb) is attached */
static void fsa9480_dock_charger_cb(bool attached)
{
	struct power_supply *psy = power_supply_get_by_name("battery");
	union power_supply_propval value;
	pr_info("%s : attached(%d)\n", __func__, attached);


	/* When desckdock is attached, ta detach interrupt is not generated
		because of fsa9480 limitation.
		Detach code is used only when dock & ta was detach at the same time*/
	if (attached) {
		set_cable_status = CABLE_TYPE_DOCK;
		value.intval = POWER_SUPPLY_TYPE_DOCK;

		if (!psy) {
			pr_err("%s: fail to get battery ps\n", __func__);
			return ;
		}

		psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	} else {
		if(set_cable_status == CABLE_TYPE_DOCK) {
			set_cable_status = CABLE_TYPE_NONE;
			value.intval = POWER_SUPPLY_TYPE_BATTERY;

			if (!psy) {
				pr_err("%s: fail to get battery ps\n", __func__);
				return ;
			}

			psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
		}
	}
}
EXPORT_SYMBOL(fsa9480_dock_charger_cb);

bool dock_attached;
void tegra_vbus_detect_notify_batt(void)
{
	if(dock_attached == 1)	{
		fsa9480_dock_charger_cb(1);
	}
}

static void fsa9480_deskdock_cb(bool attached)
{
	if (attached) {
		switch_set_state(&switch_dock, 1);
		dock_attached = 1;
	}
	else {
		switch_set_state(&switch_dock, 0);
		dock_attached = 0;
	}
}

static void fsa9480_cardock_cb(bool attached)
{
	if (attached) {
		switch_set_state(&switch_dock, 2);
		dock_attached = 1;
	}
	else {
		switch_set_state(&switch_dock, 0);
		dock_attached = 0;
	}
}

static void fsa9480_reset_cb(void)
{
	int ret;

	/* for CarDock, DeskDock */
	ret = switch_dev_register(&switch_dock);
	if (ret < 0)
		pr_err("Failed to register dock switch. %d\n", ret);
}

static void fsa9480_set_otg_func(void (*otg_id_open)(struct fsa9480_usbsw *),
			struct fsa9480_usbsw *data)
{
	otg_data.otg_open->id_open = otg_id_open;
	otg_data.otg_open->otg_cb = fsa9480_otg_cb;
	otg_data.otg_open->otg_open_data = data;
	otg_data.otg_open->otg_enabled = 0;
	if (data->dev1 & DEV_USB_OTG) {
		if (otg_data.otg_open->otg_state
				&& otg_data.otg_open->host_state) {
			if (*otg_data.otg_open->otg_state != OTG_STATE_A_HOST)
				otg_id_open(data);
			else if (*otg_data.otg_open->host_state
					== TEGRA_USB_OVERCURRENT) {
				if (otg_data.vbus_en) {
					otg_data.vbus_en(0);
					msleep(20);
					otg_data.vbus_en(1);
				} else
					pr_err("%s: vbus_en is null\n"
								, __func__);
			}
		}

	}
}
extern void n1_inform_charger_connection(int mode);
static struct fsa9480_platform_data fsa9480_pdata = {
	.otg_cb = fsa9480_otg_cb,
	.usb_cb = fsa9480_usb_cb,
	.uart_cb = fsa9480_uart_cb,
	.charger_cb = fsa9480_charger_cb,
	.charger_ovp_cb = fsa9480_charger_ovp_cb,
	.dock_charger_cb = fsa9480_dock_charger_cb,
	.deskdock_cb = fsa9480_deskdock_cb,
	.cardock_cb = fsa9480_cardock_cb,
	.reset_cb = fsa9480_reset_cb,
	.set_otg_func = fsa9480_set_otg_func,
	.inform_charger_connection = n1_inform_charger_connection,
};

static struct i2c_gpio_platform_data tegra_gpio_i2c11_pdata = {
	.sda_pin = GPIO_USB_I2C_SDA,
	.scl_pin = GPIO_USB_I2C_SCL,
	.udelay = 1, /* 200 kHz */
	.timeout = 0, /* jiffies */
};

static struct platform_device tegra_gpio_i2c11_device = {
	.name = "i2c-gpio",
	.id = 11,
	.dev = {
		.platform_data = &tegra_gpio_i2c11_pdata,
	}
};

#ifdef CONFIG_MHL_SII9234
/* HDMI - DDC */
static struct i2c_gpio_platform_data tegra_gpio_i2c13_pdata = {
	.sda_pin = GPIO_HDMI_I2C_SDA,
	.scl_pin = GPIO_HDMI_I2C_SCL,
	.udelay = 3,
	.timeout = 0,
};

static struct platform_device tegra_gpio_i2c13_device = {
	.name = "i2c-gpio",
	.id = 13,
	.dev = {
		.platform_data = &tegra_gpio_i2c13_pdata,
	}
};

/* HDMI MHL LOGIC IF */
static struct i2c_gpio_platform_data tegra_gpio_i2c14_pdata = {
	.sda_pin = GPIO_MHL_I2C_SDA,
	.scl_pin = GPIO_MHL_I2C_SCL,
	.udelay = 3,
	.timeout = 0,
};

static struct platform_device tegra_gpio_i2c14_device = {
	.name = "i2c-gpio",
	.id = 14,
	.dev = {
		.platform_data = &tegra_gpio_i2c14_pdata,
	}
};
#endif

static struct i2c_gpio_platform_data tegra_gpio_i2c16_pdata = {
	.sda_pin = GPIO_CAMPMIC_SDA_18V,
	.scl_pin = GPIO_CAMPMIC_SCL_18V,
	.udelay = 1,
	.timeout = 0,
};

static struct platform_device tegra_gpio_i2c16_device = {
	.name = "i2c-gpio",
	.id = 16,
	.dev = {
		.platform_data = &tegra_gpio_i2c16_pdata,
	}
};

static struct i2c_gpio_platform_data tegra_gpio_i2c17_pdata = {
	.sda_pin = GPIO_VIBTONE_I2C_SDA,
	.scl_pin = GPIO_VIBTONE_I2C_SCL,
	.udelay = 1,
	.timeout = 0,
};

static struct platform_device tegra_gpio_i2c17_device = {
	.name = "i2c-gpio",
	.id = 17,
	.dev = {
		.platform_data = &tegra_gpio_i2c17_pdata,
	}
};

static const struct i2c_board_info sec_gpio_i2c6_info[] = {
	{
		I2C_BOARD_INFO("Si4709", 0x20 >> 1),
		.irq = TEGRA_GPIO_TO_IRQ(GPIO_FM_INT),
	},
};

static struct i2c_board_info sec_gpio_i2c9_info[] = {

	{
		I2C_BOARD_INFO("cm3663", 0x11),
		.irq = GPIO_ALC_INT,
	},
};

static struct i2c_board_info sec_gpio_i2c10_info[] = {
	{
		I2C_BOARD_INFO("image_convertor", 0x38),
	},
};

static struct i2c_board_info sec_gpio_i2c11_info[] = {
	{
		I2C_BOARD_INFO("fsa9480", 0x4A>>1),
		.platform_data = &fsa9480_pdata,
		.irq = GPIO_JACK_nINT,
	},
};

static struct i2c_board_info sec_gpio_i2c13_info[] = {
};

static struct i2c_board_info sec_gpio_i2c14_info[] = {
	{
		I2C_BOARD_INFO("SII9234", 0x72>>1),
	},
	{
		I2C_BOARD_INFO("SII9234A", 0x7A>>1),
	},
	{
		I2C_BOARD_INFO("SII9234B", 0x92>>1),
	},
	{
		I2C_BOARD_INFO("SII9234C", 0xC8>>1),
	},
};

static void nct1008_temp_register_callbacks(
		struct nct1008_temp_callbacks *ptr)
{
	callbacks = ptr;
}
static void n1_nct1008_init(void)
{
	tegra_gpio_enable(GPIO_nTHRM_IRQ);
	gpio_request(GPIO_nTHRM_IRQ, "temp_alert");
	gpio_direction_input(GPIO_nTHRM_IRQ);
}
static struct nct1008_temp_data n1_temp = {
	.register_callbacks = &nct1008_temp_register_callbacks,
};

extern void tegra_throttling_enable(bool enable);
static struct nct1008_platform_data n1_nct1008_pdata = {
	.supported_hwrev = true,
	.ext_range = true,
	.conv_rate = 0x04,
	.offset = 0,
	.hysteresis = 0,
	.shutdown_ext_limit = 115,
	.shutdown_local_limit = 120,
	.throttling_ext_limit = 90,
	.alarm_fn = tegra_throttling_enable,
	.temp = &n1_temp,
};


static struct i2c_board_info sec_gpio_i2c15_info[] = {
	{
		I2C_BOARD_INFO("nct1008", 0x4C),
		.platform_data = &n1_nct1008_pdata,
		.irq = TEGRA_GPIO_TO_IRQ(GPIO_nTHRM_IRQ),
	},
};

int __init n1_gpio_i2c_init(int lpm_mode)
{
	/* image converter */
	platform_device_register(&tegra_gpio_i2c10_device);
	i2c_register_board_info(10, sec_gpio_i2c10_info,
			ARRAY_SIZE(sec_gpio_i2c10_info));

	platform_device_register(&tegra_gpio_i2c8_device);
	i2c_register_board_info(8, sec_gpio_i2c8_info,
			ARRAY_SIZE(sec_gpio_i2c8_info));

	platform_device_register(&tegra_gpio_i2c11_device);
	i2c_register_board_info(11, sec_gpio_i2c11_info,
			ARRAY_SIZE(sec_gpio_i2c11_info));



	tegra_gpio_enable(tegra_gpio_i2c8_pdata.sda_pin);
	tegra_gpio_enable(tegra_gpio_i2c8_pdata.scl_pin);


	tegra_gpio_enable(tegra_gpio_i2c11_pdata.sda_pin);
	tegra_gpio_enable(tegra_gpio_i2c11_pdata.scl_pin);


	/* nct1008 Thermal monitor */
	n1_nct1008_init();
	platform_device_register(&tegra_gpio_i2c15_device);
	i2c_register_board_info(15, sec_gpio_i2c15_info, ARRAY_SIZE(sec_gpio_i2c15_info));
	tegra_gpio_enable(tegra_gpio_i2c15_pdata.sda_pin);
	tegra_gpio_enable(tegra_gpio_i2c15_pdata.scl_pin);

	/* max17043 Fuel Gauge */
	n1_max17043_gpio_init();
	platform_device_register(&fuelgauge_gpio_i2c_device);
	i2c_register_board_info(5, n1_fuelgauge, ARRAY_SIZE(n1_fuelgauge));
	tegra_gpio_enable(fuelgauge_gpio_i2c_pdata.sda_pin);
	tegra_gpio_enable(fuelgauge_gpio_i2c_pdata.scl_pin);

	platform_device_register(&tegra_gpio_i2c9_device);
	i2c_register_board_info(9, sec_gpio_i2c9_info,
			ARRAY_SIZE(sec_gpio_i2c9_info));
	tegra_gpio_enable(tegra_gpio_i2c9_pdata.sda_pin);
	tegra_gpio_enable(tegra_gpio_i2c9_pdata.scl_pin);
	tegra_gpio_enable(sec_gpio_i2c9_info[0].irq);

	/* compass */
	tegra_gpio_enable(gpio_i2c7_platdata.sda_pin);
	tegra_gpio_enable(gpio_i2c7_platdata.scl_pin);
	platform_device_register(&n1_device_gpio_i2c7);

	/* fmradio : si4709 */
	if (system_rev >= 4) {
		n1_fmradio_init(lpm_mode);
		platform_device_register(&n1_device_gpio_i2c6);
		i2c_register_board_info(6, sec_gpio_i2c6_info,
			ARRAY_SIZE(sec_gpio_i2c6_info));
		tegra_gpio_enable(gpio_i2c6_platdata.sda_pin);
		tegra_gpio_enable(gpio_i2c6_platdata.scl_pin);
	}

	if (system_rev >= 4) {
		platform_device_register(&tegra_gpio_i2c16_device);
		tegra_gpio_enable(tegra_gpio_i2c16_pdata.sda_pin);
		tegra_gpio_enable(tegra_gpio_i2c16_pdata.scl_pin);
	}

	if(system_rev >= 5) {
		platform_device_register(&tegra_gpio_i2c17_device);
		tegra_gpio_enable(tegra_gpio_i2c17_pdata.sda_pin);
		tegra_gpio_enable(tegra_gpio_i2c17_pdata.scl_pin);
	}


	return 0;
}
