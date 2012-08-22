/*
 * arch/arm/mach-tegra/gpio.c
 *
 * Copyright (c) 2010 Google, Inc
 *
 * Author:
 *	Erik Gilling <konkers@google.com>
 *
 * Copyright (c) 2011 NVIDIA Corporation.
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

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/syscore_ops.h>

#include <asm/mach/irq.h>

#include <mach/iomap.h>
#include <mach/pinmux.h>
#include "../../../arch/arm/mach-tegra/gpio-names.h"
#include "../../../arch/arm/mach-tegra/pm-irq.h"
#include "../../../arch/arm/mach-tegra/pm.h"

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_BIT(x)		((x) & 0x7)

#define GPIO_CNF(x)		(GPIO_REG(x) + 0x00)
#define GPIO_OE(x)		(GPIO_REG(x) + 0x10)
#define GPIO_OUT(x)		(GPIO_REG(x) + 0X20)
#define GPIO_IN(x)		(GPIO_REG(x) + 0x30)
#define GPIO_INT_STA(x)		(GPIO_REG(x) + 0x40)
#define GPIO_INT_ENB(x)		(GPIO_REG(x) + 0x50)
#define GPIO_INT_LVL(x)		(GPIO_REG(x) + 0x60)
#define GPIO_INT_CLR(x)		(GPIO_REG(x) + 0x70)

#if defined(CONFIG_ARCH_TEGRA_2x_SOC)
#define GPIO_REG(x)		(IO_TO_VIRT(TEGRA_GPIO_BASE) +	\
				 GPIO_BANK(x) * 0x80 +		\
				 GPIO_PORT(x) * 4)

#define GPIO_MSK_CNF(x)		(GPIO_REG(x) + 0x800)
#define GPIO_MSK_OE(x)		(GPIO_REG(x) + 0x810)
#define GPIO_MSK_OUT(x)		(GPIO_REG(x) + 0X820)
#define GPIO_MSK_INT_STA(x)	(GPIO_REG(x) + 0x840)
#define GPIO_MSK_INT_ENB(x)	(GPIO_REG(x) + 0x850)
#define GPIO_MSK_INT_LVL(x)	(GPIO_REG(x) + 0x860)
#else
#define GPIO_REG(x)		(IO_TO_VIRT(TEGRA_GPIO_BASE) +	\
				 GPIO_BANK(x) * 0x100 +		\
				 GPIO_PORT(x) * 4)

#define GPIO_MSK_CNF(x)		(GPIO_REG(x) + 0x80)
#define GPIO_MSK_OE(x)		(GPIO_REG(x) + 0x90)
#define GPIO_MSK_OUT(x)		(GPIO_REG(x) + 0XA0)
#define GPIO_MSK_INT_STA(x)	(GPIO_REG(x) + 0xC0)
#define GPIO_MSK_INT_ENB(x)	(GPIO_REG(x) + 0xD0)
#define GPIO_MSK_INT_LVL(x)	(GPIO_REG(x) + 0xE0)
#endif

#define GPIO_INT_LVL_MASK		0x010101
#define GPIO_INT_LVL_EDGE_RISING	0x000101
#define GPIO_INT_LVL_EDGE_FALLING	0x000100
#define GPIO_INT_LVL_EDGE_BOTH		0x010100
#define GPIO_INT_LVL_LEVEL_HIGH		0x000001
#define GPIO_INT_LVL_LEVEL_LOW		0x000000

extern unsigned int system_rev;

struct tegra_gpio_bank {
	int bank;
	int irq;
	spinlock_t lvl_lock[4];
#ifdef CONFIG_PM_SLEEP
	u32 cnf[4];
	u32 out[4];
	u32 oe[4];
	u32 int_enb[4];
	u32 int_lvl[4];
#endif
};

static struct tegra_gpio_bank tegra_gpio_banks[] = {
	{.bank = 0, .irq = INT_GPIO1},
	{.bank = 1, .irq = INT_GPIO2},
	{.bank = 2, .irq = INT_GPIO3},
	{.bank = 3, .irq = INT_GPIO4},
	{.bank = 4, .irq = INT_GPIO5},
	{.bank = 5, .irq = INT_GPIO6},
	{.bank = 6, .irq = INT_GPIO7},
#ifndef CONFIG_ARCH_TEGRA_2x_SOC
	{.bank = 7, .irq = INT_GPIO8},
#endif
};

static int tegra_gpio_compose(int bank, int port, int bit)
{
	return (bank << 5) | ((port & 0x3) << 3) | (bit & 0x7);
}

void tegra_gpio_set_tristate(int gpio_nr, enum tegra_tristate ts)
{
	int pin_group  =  tegra_pinmux_get_pingroup(gpio_nr);
	tegra_pinmux_set_tristate(pin_group, ts);
}

static void tegra_gpio_mask_write(u32 reg, int gpio, int value)
{
	u32 val;

	val = 0x100 << GPIO_BIT(gpio);
	if (value)
		val |= 1 << GPIO_BIT(gpio);
	__raw_writel(val, reg);
}

int tegra_gpio_get_bank_int_nr(int gpio)
{
	int bank;
	int irq;
	if (gpio >= TEGRA_NR_GPIOS) {
		pr_warn("%s : Invalid gpio ID - %d\n", __func__, gpio);
		return -EINVAL;
	}
	bank = gpio >> 5;
	irq = tegra_gpio_banks[bank].irq;
	return irq;
}

void tegra_gpio_enable(int gpio)
{
	if (gpio >= TEGRA_NR_GPIOS) {
		pr_warn("%s : Invalid gpio ID - %d\n", __func__, gpio);
		return;
	}
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 1);
}
EXPORT_SYMBOL_GPL(tegra_gpio_enable);

void tegra_gpio_disable(int gpio)
{
	if (gpio >= TEGRA_NR_GPIOS) {
		pr_warn("%s : Invalid gpio ID - %d\n", __func__, gpio);
		return;
	}
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 0);
}
EXPORT_SYMBOL_GPL(tegra_gpio_disable);

void tegra_gpio_init_configure(unsigned gpio, bool is_input, int value)
{
	if (gpio >= TEGRA_NR_GPIOS) {
		pr_warn("%s : Invalid gpio ID - %d\n", __func__, gpio);
		return;
	}
	if (is_input) {
		tegra_gpio_mask_write(GPIO_MSK_OE(gpio), gpio, 0);
	} else {
		tegra_gpio_mask_write(GPIO_MSK_OUT(gpio), gpio, value);
		tegra_gpio_mask_write(GPIO_MSK_OE(gpio), gpio, 1);
	}
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 1);
}

static void tegra_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	tegra_gpio_mask_write(GPIO_MSK_OUT(offset), offset, value);
}

static int tegra_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	if ((__raw_readl(GPIO_OE(offset)) >> GPIO_BIT(offset)) & 0x1)
		return (__raw_readl(GPIO_OUT(offset)) >>
			GPIO_BIT(offset)) & 0x1;
	return (__raw_readl(GPIO_IN(offset)) >> GPIO_BIT(offset)) & 0x1;
}

static int tegra_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	tegra_gpio_mask_write(GPIO_MSK_OE(offset), offset, 0);
	return 0;
}

static int tegra_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
					int value)
{
	tegra_gpio_set(chip, offset, value);
	tegra_gpio_mask_write(GPIO_MSK_OE(offset), offset, 1);
	return 0;
}

static int tegra_gpio_set_debounce(struct gpio_chip *chip, unsigned offset,
				unsigned debounce)
{
	return -ENOSYS;
}

static int tegra_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	return TEGRA_GPIO_TO_IRQ(offset);
}

static struct gpio_chip tegra_gpio_chip = {
	.label			= "tegra-gpio",
	.direction_input	= tegra_gpio_direction_input,
	.get			= tegra_gpio_get,
	.direction_output	= tegra_gpio_direction_output,
	.set			= tegra_gpio_set,
	.set_debounce		= tegra_gpio_set_debounce,
	.to_irq			= tegra_gpio_to_irq,
	.base			= 0,
	.ngpio			= TEGRA_NR_GPIOS,
};

static void tegra_gpio_irq_ack(struct irq_data *d)
{
	int gpio = d->irq - INT_GPIO_BASE;

	__raw_writel(1 << GPIO_BIT(gpio), GPIO_INT_CLR(gpio));

#ifdef CONFIG_TEGRA_FPGA_PLATFORM
	/* FPGA platforms have a serializer between the GPIO
	   block and interrupt controller. Allow time for
	   clearing of the GPIO interrupt to propagate to the
	   interrupt controller before re-enabling the IRQ
	   to prevent double interrupts. */
	udelay(15);
#endif
}

static void tegra_gpio_irq_mask(struct irq_data *d)
{
	int gpio = d->irq - INT_GPIO_BASE;

	tegra_gpio_mask_write(GPIO_MSK_INT_ENB(gpio), gpio, 0);
}

static void tegra_gpio_irq_unmask(struct irq_data *d)
{
	int gpio = d->irq - INT_GPIO_BASE;

	tegra_gpio_mask_write(GPIO_MSK_INT_ENB(gpio), gpio, 1);
}

static int tegra_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	int gpio = d->irq - INT_GPIO_BASE;
	struct tegra_gpio_bank *bank = irq_data_get_irq_chip_data(d);
	int port = GPIO_PORT(gpio);
	int lvl_type;
	int val;
	unsigned long flags;

	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_RISING:
		lvl_type = GPIO_INT_LVL_EDGE_RISING;
		break;

	case IRQ_TYPE_EDGE_FALLING:
		lvl_type = GPIO_INT_LVL_EDGE_FALLING;
		break;

	case IRQ_TYPE_EDGE_BOTH:
		lvl_type = GPIO_INT_LVL_EDGE_BOTH;
		break;

	case IRQ_TYPE_LEVEL_HIGH:
		lvl_type = GPIO_INT_LVL_LEVEL_HIGH;
		break;

	case IRQ_TYPE_LEVEL_LOW:
		lvl_type = GPIO_INT_LVL_LEVEL_LOW;
		break;

	default:
		return -EINVAL;
	}

	spin_lock_irqsave(&bank->lvl_lock[port], flags);

	val = __raw_readl(GPIO_INT_LVL(gpio));
	val &= ~(GPIO_INT_LVL_MASK << GPIO_BIT(gpio));
	val |= lvl_type << GPIO_BIT(gpio);
	__raw_writel(val, GPIO_INT_LVL(gpio));

	spin_unlock_irqrestore(&bank->lvl_lock[port], flags);

	if (type & (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH))
		__irq_set_handler_locked(d->irq, handle_level_irq);
	else if (type & (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
		__irq_set_handler_locked(d->irq, handle_edge_irq);

	tegra_pm_irq_set_wake_type(d->irq, type);

	return 0;
}

static void tegra_gpio_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	struct tegra_gpio_bank *bank;
	int port;
	int pin;
	struct irq_chip *chip = irq_desc_get_chip(desc);

	chained_irq_enter(chip, desc);

	bank = irq_get_handler_data(irq);

	for (port = 0; port < 4; port++) {
		int gpio = tegra_gpio_compose(bank->bank, port, 0);
		unsigned long sta = __raw_readl(GPIO_INT_STA(gpio)) &
			__raw_readl(GPIO_INT_ENB(gpio));

		for_each_set_bit(pin, &sta, 8)
			generic_handle_irq(gpio_to_irq(gpio + pin));
	}

	chained_irq_exit(chip, desc);

}

#ifdef CONFIG_PM_SLEEP
#ifdef CONFIG_MACH_N1
#if defined (CONFIG_MACH_BOSE_ATT)
#include <mach/gpio-bose.h>
#else
#include <mach/gpio-n1.h>
#endif

#define NO		0
#define YES		1

#define GPIO_OUTPUT	1
#define GPIO_INPUT	0

#define GPIO_LEVEL_HIGH		1
#define GPIO_LEVEL_LOW		0
#define GPIO_LEVEL_NONE		(-1)

typedef struct sec_gpio_cfg_st {
	int slp_ctrl;
	unsigned int gpio;
	int dir;
	int val;
	char *name;
};

static struct sec_gpio_cfg_st n1_sleep_gpio_table[] = {
	{NO,	GPIO_ALC_INT,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_ALC_INT"},
	{YES,	GPIO_MAG_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MAG_I2C_SCL"},
	{NO,	GPIO_CP_ON,					GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CP_ON"},
	{YES,	GPIO_MAG_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MAG_I2C_SDA"},
	{NO,	GPIO_IF_CON_SENSE,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_IF_CON_SENSE"},
	{YES,	GPIO_FUEL_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FUEL_I2C_SCL"},
	{YES,	GPIO_FUEL_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FUEL_I2C_SDA"},
	{NO,	GPIO_ALC_I2C_SCL,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_ALC_I2C_SCL"},
	{YES,	GPIO_THERMAL_I2C_SCL,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_THERMAL_I2C_SCL"},
	{NO,	GPIO_ALC_I2C_SDA,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_ALC_I2C_SDA"},
	{YES,	GPIO_THERMAL_I2C_SDA,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_THERMAL_I2C_SDA"},
	{NO,	GPIO_HSIC_EN,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HSIC_EN"},
	{YES,	GPIO_PDA_ACTIVE,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_PDA_ACTIVE"},
	{NO,	GPIO_PHONE_ACTIVE,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_PHONE_ACTIVE"},
	{NO,	GPIO_EAR_SEND_END,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_EAR_SEND_END"},
	{NO,	GPIO_GPS_NRST,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GPS_NRST"},
	{YES,	GPIO_MLCD_RST,				GPIO_OUTPUT,		GPIO_LEVEL_LOW,	"GPIO_MLCD_RST"},
#if !defined(CONFIG_MACH_BOSE_ATT)
	{YES,	GPIO_CMC_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_I2C_SCL"},
	{NO,	GPIO_GPS_EN,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GPS_EN"},
	{NO,	GPIO_CMC_SLEEP,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_SLEEP"},
	{NO,	GPIO_GPS_CNTL,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GPS_CNTL"},
	{YES,	GPIO_CMC_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_I2C_SDA"},
#endif
	{NO,	GPIO_BT_nRST,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_BT_nRST"},
	{NO,	GPIO_CAM_VT_nRST,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_VT_nRST"},
	{NO,	GPIO_CAM_VT_nSTBY,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_VT_nSTBY"},
	{NO,	GPIO_JACK_nINT,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_JACK_nINT"},
#if !defined(CONFIG_MACH_BOSE_ATT)
	{YES,	GPIO_CMC_LDO_EN1,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_CMC_LDO_EN1"},
	{NO,	GPIO_NFC_FIRMWARE,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_NFC_FIRMWARE"},
#endif
	{NO,	GPIO_CP_DUMP_INT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CP_DUMP_INT"},
	{YES,	GPIO_CAM_PMIC_EN2,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_CAM_PMIC_EN2"},
	{NO,	GPIO_CAM_MEGA_STBY,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_MEGA_STBY"},
	{NO,	GPIO_TSP_INT,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_TSP_INT"},
	{YES,	GPIO_USB_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_USB_I2C_SCL"},
/* SFIO sleep setting */
//	{YES,	GPIO_CAM_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_I2C_SCL"},
//	{YES,	GPIO_CAM_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_I2C_SDA"},
	{YES,	GPIO_USB_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_USB_I2C_SDA"},
#if !defined(CONFIG_MACH_BOSE_ATT)
	{NO,	GPIO_CMC_BYPASS,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_BYPASS"},
#endif
	{NO,	GPIO_CAM_MEGA_nRST,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_MEGA_nRST"},
	{NO,	GPIO_GYRO_FIFO_INT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GYRO_FIFO_INT"},
#if !defined(CONFIG_MACH_BOSE_ATT)
	{NO,	GPIO_CMC_RST,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_RST"},
#endif
/* BT malfunction occurred, Do NOT control GPIO_BT_EN here. */
	{NO,	GPIO_BT_EN,					GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_BT_EN"},
	{NO,	GPIO_EXT_WAKEUP,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_EXT_WAKEUP"},
	{NO,	GPIO_nTHRM_IRQ,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_nTHRM_IRQ"},
/* SFIO sleep setting */
//	{YES,	GPIO_GYRO_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GYRO_I2C_SCL"},
//	{YES,	GPIO_GYRO_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GYRO_I2C_SDA"},
#if !defined(CONFIG_MACH_BOSE_ATT)
	{NO,	GPIO_FM_INT,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FM_INT"},
	{NO,	GPIO_FM_RST_04,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FM_RST_04"},
	{NO,	GPIO_FM_RST_05,				GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_FM_RST_05"},
	{YES,	GPIO_FM_SCL_18V,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FM_SCL_18V"},
#if defined (CONFIG_MACH_BOSE_ATT)
	{YES,	GPIO_CAM_MEGA_CORE_1P2_EN	GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_MEGA_CORE_1P2_EN"},
#else
	{YES,	GPIO_FM_SDA_18V,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FM_SDA_18V"},
#endif
	{NO,	GPIO_LCD_LED_EN_SET,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LCD_LED_EN_SET"},
#endif
	{NO,	GPIO_IPC_SLAVE_WAKEUP,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_IPC_SLAVE_WAKEUP"},
	{NO,	GPIO_VIBTONE_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_VIBTONE_EN"},
	{NO,	GPIO_VIBTONE_PWM,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_VIBTONE_PWM"},
	{NO,	GPIO_USB_OTG_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_USB_OTG_EN"},
/* SFIO sleep setting */
/*	{YES,	GPIO_WLAN_HOST_WAKE,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_WLAN_HOST_WAKE"},*/
	{NO,	GPIO_BT_WAKE,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_BT_WAKE"},
	{NO,	GPIO_BT_HOST_WAKE,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_BT_HOST_WAKE"},
	{NO,	GPIO_IPC_HOST_WAKEUP,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_IPC_HOST_WAKEUP"},
	{NO,	GPIO_WLAN_EN,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_WLAN_EN"},
	{NO,	GPIO_VOL_UP,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_VOL_UP"},
	{NO,	GPIO_VOL_DOWN,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_VOL_DOWN"},
#if !defined(CONFIG_MACH_BOSE_ATT)
	{NO,	GPIO_CMC_SHDN,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_SHDN"},
#endif
	{NO,	GPIO_ACTIVE_STATE_HSIC,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_ACTIVE_STATE_HSIC"},
	{NO,	GPIO_AK8975_INT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_AK8975_INT"},
	{NO,	GPIO_TA_CURRENT_SEL,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_TA_CURRENT_SEL"},
	{NO,	GPIO_CHG_EN,				GPIO_OUTPUT,	GPIO_LEVEL_NONE,		"GPIO_CHG_EN"},
// 20110524_HDLNC_PMIC_sleep current edit
//	{YES,	GPIO_CAM_FLASH_SET,			GPIO_OUTPUT,		GPIO_LEVEL_LOW,	"GPIO_CAM_FLASH_SET"},
//	{YES,	GPIO_CAM_FLASH_EN,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,	    "GPIO_CAM_FLASH_EN"},
#if !defined(CONFIG_MACH_BOSE_ATT)
	{NO,	GPIO_HOME_KEY,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HOME_KEY"},
#endif
	{YES,	GPIO_CODEC_LDO_EN,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_CODEC_LDO_EN"},
	{NO,	GPIO_MICBIAS1_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MICBIAS1_EN"},
	{YES,	GPIO_UART_SEL,				GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_UART_SEL"},
	{NO,	GPIO_UART_SEL_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_UART_SEL_EN"},
	{NO,	GPIO_SUSPEND_REQUEST_HSIC,	GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_SUSPEND_REQUEST_HSIC"},
	{NO,	GPIO_DET_3_5,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_DET_3_5"},
	{YES,	GPIO_HDMI_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HDMI_I2C_SCL"},
	{YES,	GPIO_HDMI_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HDMI_I2C_SDA "},
#if defined (CONFIG_MACH_BOSE_ATT)
	{YES,	GPIO_T_FLASH_DET,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_T_FLASH_DET"},
#endif
	{NO,	GPIO_FUEL_ALERT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FUEL_ALERT "},
	{NO,	GPIO_LED_LDO_EN1,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LED_LDO_EN1 "},
	{YES,	GPIO_MHL_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MHL_I2C_SCL "},
	{YES,	GPIO_MHL_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MHL_I2C_SDA "},
	{NO,	GPIO_COMPASS_INT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_COMPASS_INT "},
	{NO,	GPIO_LED_LDO_EN2,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LED_LDO_EN2 "},
	{YES,	GPIO_LCD_ID1,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LCD_ID1"},
	{YES,	GPIO_CODEC_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CODEC_I2C_SDA "},
#if defined (CONFIG_MACH_BOSE_ATT)
	{NO,	GPIO_MHL_INT,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MHL_INT "},
/* MHL did not work when GPIO_MHL_RST controlled here. */
	{NO,	GPIO_MHL_RST,				GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_MHL_RST"},
	{NO,	GPIO_HDMI_LDO_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HDMI_LDO_EN "},
#endif
	{NO,	GPIO_MICBIAS2_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MICBIAS2_EN "},
	{YES,	GPIO_CODEC_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CODEC_I2C_SCL "},
	{YES,	GPIO_EAR_SEL,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_EAR_SEL "},
	{YES,	GPIO_VIBTONE_I2C_SCL,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_VIBTONE_I2C_SCL "},
	{YES,	GPIO_VIBTONE_I2C_SDA,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_VIBTONE_I2C_SDA "},
	{YES,	GPIO_CAMPMIC_SCL_18V,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAMPMIC_SCL_18V "},
	{YES,	GPIO_CAMPMIC_SDA_18V,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAMPMIC_SDA_18V "},
};
#if !defined(CONFIG_MACH_BOSE_ATT)
static struct sec_gpio_cfg_st n1_sleep_gpio_table_07[] = {
	{YES,	GPIO_FM_RST_05,				GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_FM_RST_05"},
};
#endif
static struct sec_gpio_cfg_st n1_sfio_sleep_config_table[] = {
	/* Configure Non SFIO to GPIO to enable high impedence while deep-sleep.
	 * Must reconfigure these pins back to SFIO when resume happens. 		*/
	{YES,	TEGRA_GPIO_PA2,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"SFIO_SPEECH_PCM_SYNC"},
	{YES,	TEGRA_GPIO_PC4,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"SFIO_GYRO_SCL"},
	{YES,	TEGRA_GPIO_PC5,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"SFIO_GYRO_SDA"},
/* Move to panel early suspend. */
/*	{YES,	TEGRA_GPIO_PN4,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"SFIO_LCD_NCS"}, */
	{YES,	TEGRA_GPIO_PN5,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"SFIO_LCD_SDI"},
	{YES,	TEGRA_GPIO_PP0,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"SFIO_CP_PCM_SYNC"},
	{YES,	TEGRA_GPIO_PS0,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_WLAN_HOST_WAKE"},
	{YES,	TEGRA_GPIO_PW4,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"SFIO_CODEC_MCLK_AP"},
	{YES,	TEGRA_GPIO_PZ2,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"SFIO_LCD_SDO"},
/* Move to panel early suspend. */
/*	{YES,	TEGRA_GPIO_PZ4,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"SFIO_LCD_SCLK"}, */
	{YES,	TEGRA_GPIO_PBB2,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"SFIO_CAM_SCL"},
	{YES,	TEGRA_GPIO_PBB3,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"SFIO_CAM_SDA"},
};

void tegra_set_sleep_gpio_table(void)
{
	int cnt;

	for (cnt = 0; cnt < ARRAY_SIZE(n1_sleep_gpio_table); cnt++) {
		if (n1_sleep_gpio_table[cnt].slp_ctrl == YES) {
#if 1
// N1_ICS
			if (!gpio_is_requested(n1_sleep_gpio_table[cnt].gpio))
				gpio_request(n1_sleep_gpio_table[cnt].gpio,
							n1_sleep_gpio_table[cnt].name);
#endif
			tegra_gpio_enable(n1_sleep_gpio_table[cnt].gpio);
			if (n1_sleep_gpio_table[cnt].dir == GPIO_OUTPUT) {
				gpio_direction_output(n1_sleep_gpio_table[cnt].gpio, n1_sleep_gpio_table[cnt].val);
			} else if (n1_sleep_gpio_table[cnt].dir == GPIO_INPUT) {
				gpio_direction_input(n1_sleep_gpio_table[cnt].gpio);
			}
		}
	}
#if !defined(CONFIG_MACH_BOSE_ATT)
	if(system_rev >= 7) {
		for (cnt = 0; cnt < ARRAY_SIZE(n1_sleep_gpio_table_07); cnt++) {
			if (n1_sleep_gpio_table_07[cnt].slp_ctrl == YES) {
#if 1
// N1_ICS
				if (!gpio_is_requested(n1_sleep_gpio_table_07[cnt].gpio))
					gpio_request(n1_sleep_gpio_table_07[cnt].gpio,
								n1_sleep_gpio_table_07[cnt].name);
#endif
				tegra_gpio_enable(n1_sleep_gpio_table_07[cnt].gpio);
				if (n1_sleep_gpio_table_07[cnt].dir == GPIO_OUTPUT) {
					gpio_direction_output(n1_sleep_gpio_table_07[cnt].gpio, n1_sleep_gpio_table_07[cnt].val);
				} else if (n1_sleep_gpio_table_07[cnt].dir == GPIO_INPUT) {
					gpio_direction_input(n1_sleep_gpio_table_07[cnt].gpio);
				}
			}
		}
	}
#endif
}

void tegra_config_sfio_sleep_table(void)
{
	int cnt;

	for (cnt = 0; cnt < ARRAY_SIZE(n1_sfio_sleep_config_table); cnt++) {
		if (n1_sfio_sleep_config_table[cnt].slp_ctrl == YES) {
			tegra_gpio_enable(n1_sfio_sleep_config_table[cnt].gpio);
			gpio_request(n1_sfio_sleep_config_table[cnt].gpio, n1_sfio_sleep_config_table[cnt].name);
			if (n1_sfio_sleep_config_table[cnt].dir == GPIO_OUTPUT)
				gpio_direction_output(n1_sfio_sleep_config_table[cnt].gpio, n1_sfio_sleep_config_table[cnt].val);
			if (n1_sfio_sleep_config_table[cnt].dir == GPIO_INPUT)
				gpio_direction_input(n1_sfio_sleep_config_table[cnt].gpio);
		}
	}
}

void tegra_reconfig_sfio_sleep(void)
{
	int cnt;

	for (cnt = 0; cnt < ARRAY_SIZE(n1_sfio_sleep_config_table); cnt++) {
		if (n1_sfio_sleep_config_table[cnt].slp_ctrl == YES) {
			tegra_gpio_disable(n1_sfio_sleep_config_table[cnt].gpio);
			gpio_free(n1_sfio_sleep_config_table[cnt].gpio);
		}
	}
}

typedef struct sec_init_gpio_cfg_st {
	int attr;
	unsigned int gpio;
	int dir;
	int val;
	char *name;
};

#define SPIO 0
#define GPIO 1
static struct sec_init_gpio_cfg_st n1_gpio_table[] = {
	/* set NC Pin to input or output when no pull */
	{GPIO,	GPIO_HWREV0,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HWREV0 "},
	{GPIO,	GPIO_HWREV1,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HWREV1 "},
	{GPIO,	GPIO_HWREV2,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HWREV2 "},
	{GPIO,	GPIO_HWREV3,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HWREV3 "},
	{GPIO,	TEGRA_GPIO_PD0, 		GPIO_INPUT, 	GPIO_LEVEL_NONE,	"TEGRA_GPIO_PD0" },
	{GPIO,	TEGRA_GPIO_PD1,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"TEGRA_GPIO_PD1" },
	{GPIO,	TEGRA_GPIO_PG1, 		GPIO_INPUT, 	GPIO_LEVEL_NONE,	"TEGRA_GPIO_PG1" },
	{GPIO,	TEGRA_GPIO_PL4,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"TEGRA_GPIO_PL4" },
	{GPIO,	TEGRA_GPIO_PN7,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PN7" },
	{GPIO,	TEGRA_GPIO_PR5,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"TEGRA_GPIO_PR5" },
	{GPIO,	TEGRA_GPIO_PU2,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"TEGRA_GPIO_PU2" },
	{GPIO,	TEGRA_GPIO_PU3,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"TEGRA_GPIO_PU3" },
	{GPIO,	TEGRA_GPIO_PBB0, 		GPIO_OUTPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PBB0" },
#if defined (CONFIG_MACH_BOSE_ATT)
	{GPIO,	TEGRA_GPIO_PG2, 		GPIO_INPUT, 	GPIO_LEVEL_NONE,	"TEGRA_GPIO_PG2" },
	{GPIO,	TEGRA_GPIO_PI7, 		GPIO_INPUT, 	GPIO_LEVEL_NONE,	"TEGRA_GPIO_PI7" },
	{GPIO,	TEGRA_GPIO_PJ6, 		GPIO_INPUT, 	GPIO_LEVEL_NONE,	"TEGRA_GPIO_PJ6" },
	{GPIO,	TEGRA_GPIO_PR2,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"TEGRA_GPIO_PR2" },
	{GPIO,	TEGRA_GPIO_PU4,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"TEGRA_GPIO_PU4" },
	{GPIO,	TEGRA_GPIO_PW0,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PW0" },
	{GPIO,	TEGRA_GPIO_PX3,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"TEGRA_GPIO_PX3" },
	{GPIO,	TEGRA_GPIO_PZ2,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PZ2" },
	{GPIO,	TEGRA_GPIO_PBB5,		GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PBB5" },
#endif
};

void tegra_set_gpio_init_table(void)
{
	int cnt;

	if (system_rev<0x05) return;

	for (cnt = 0; cnt < ARRAY_SIZE(n1_gpio_table); cnt++) {
		if (n1_gpio_table[cnt].attr == GPIO) {
			tegra_gpio_enable(n1_gpio_table[cnt].gpio);
			gpio_request(n1_gpio_table[cnt].gpio, n1_gpio_table[cnt].name);
			if (n1_gpio_table[cnt].dir == GPIO_OUTPUT) {
				gpio_direction_output(n1_gpio_table[cnt].gpio, n1_gpio_table[cnt].val);
			} else if (n1_gpio_table[cnt].dir == GPIO_INPUT) {
				gpio_direction_input(n1_gpio_table[cnt].gpio);
			} else {
				tegra_gpio_disable(n1_gpio_table[cnt].gpio);
			}
		} else {
			tegra_gpio_disable(n1_gpio_table[cnt].gpio);
		}
		gpio_free(n1_gpio_table[cnt].gpio);
	}
}

#endif
static void tegra_gpio_resume(void)
{
	unsigned long flags;
	int b;
	int p;

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
	if (tegra_get_current_suspend_mode() != TEGRA_SUSPEND_LP0)
			return;
#endif

#ifdef CONFIG_MACH_N1
	tegra_reconfig_sfio_sleep();
#endif

	local_irq_save(flags);

	for (b = 0; b < ARRAY_SIZE(tegra_gpio_banks); b++) {
		struct tegra_gpio_bank *bank = &tegra_gpio_banks[b];

		for (p = 0; p < ARRAY_SIZE(bank->oe); p++) {
			unsigned int gpio = (b<<5) | (p<<3);
			__raw_writel(bank->cnf[p], GPIO_CNF(gpio));
			__raw_writel(bank->out[p], GPIO_OUT(gpio));
			__raw_writel(bank->oe[p], GPIO_OE(gpio));
			__raw_writel(bank->int_lvl[p], GPIO_INT_LVL(gpio));
			__raw_writel(bank->int_enb[p], GPIO_INT_ENB(gpio));
		}
	}

	local_irq_restore(flags);
}

static int tegra_gpio_suspend(void)
{
	unsigned long flags;
	int b;
	int p;

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
	if (tegra_get_current_suspend_mode() != TEGRA_SUSPEND_LP0)
			return 0;
#endif

	local_irq_save(flags);
	for (b = 0; b < ARRAY_SIZE(tegra_gpio_banks); b++) {
		struct tegra_gpio_bank *bank = &tegra_gpio_banks[b];

		for (p = 0; p < ARRAY_SIZE(bank->oe); p++) {
			unsigned int gpio = (b<<5) | (p<<3);
			bank->cnf[p] = __raw_readl(GPIO_CNF(gpio));
			bank->out[p] = __raw_readl(GPIO_OUT(gpio));
			bank->oe[p] = __raw_readl(GPIO_OE(gpio));
			bank->int_enb[p] = __raw_readl(GPIO_INT_ENB(gpio));
			bank->int_lvl[p] = __raw_readl(GPIO_INT_LVL(gpio));
		}
	}
	local_irq_restore(flags);

#ifdef CONFIG_MACH_N1
	tegra_set_sleep_gpio_table();
	tegra_config_sfio_sleep_table();
#endif

	return 0;
}

static int tegra_gpio_irq_set_wake(struct irq_data *d, unsigned int enable)
{
	struct tegra_gpio_bank *bank = irq_data_get_irq_chip_data(d);
	int ret = 0;

	ret = tegra_pm_irq_set_wake(d->irq, enable);

	if (ret)
		return ret;

	ret = irq_set_irq_wake(bank->irq, enable);

	if (ret)
		tegra_pm_irq_set_wake(d->irq, !enable);

	return ret;
}
#else
#define tegra_gpio_irq_set_wake NULL
#define tegra_gpio_suspend NULL
#define tegra_gpio_resume NULL
#endif

static struct syscore_ops tegra_gpio_syscore_ops = {
	.suspend = tegra_gpio_suspend,
	.resume = tegra_gpio_resume,
};

int tegra_gpio_resume_init(void)
{
	register_syscore_ops(&tegra_gpio_syscore_ops);

	return 0;
}

static struct irq_chip tegra_gpio_irq_chip = {
	.name		= "GPIO",
	.irq_ack	= tegra_gpio_irq_ack,
	.irq_mask	= tegra_gpio_irq_mask,
	.irq_unmask	= tegra_gpio_irq_unmask,
	.irq_set_type	= tegra_gpio_irq_set_type,
	.irq_set_wake	= tegra_gpio_irq_set_wake,
	.flags		= IRQCHIP_MASK_ON_SUSPEND,
};


/* This lock class tells lockdep that GPIO irqs are in a different
 * category than their parents, so it won't report false recursion.
 */
static struct lock_class_key gpio_lock_class;

static int __init tegra_gpio_init(void)
{
	struct tegra_gpio_bank *bank;
	int gpio;
	int i;
	int j;

	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		for (j = 0; j < 4; j++) {
			int gpio = tegra_gpio_compose(i, j, 0);
			__raw_writel(0x00, GPIO_INT_ENB(gpio));
			__raw_writel(0x00, GPIO_INT_STA(gpio));
		}
	}

#ifdef CONFIG_OF_GPIO
	/*
	 * This isn't ideal, but it gets things hooked up until this
	 * driver is converted into a platform_device
	 */
	tegra_gpio_chip.of_node = of_find_compatible_node(NULL, NULL,
						"nvidia,tegra20-gpio");
#endif /* CONFIG_OF_GPIO */

	gpiochip_add(&tegra_gpio_chip);

	for (gpio = 0; gpio < TEGRA_NR_GPIOS; gpio++) {
		int irq = TEGRA_GPIO_TO_IRQ(gpio);
		/* No validity check; all Tegra GPIOs are valid IRQs */

		bank = &tegra_gpio_banks[GPIO_BANK(gpio)];

		irq_set_lockdep_class(irq, &gpio_lock_class);
		irq_set_chip_data(irq, bank);
		irq_set_chip_and_handler(irq, &tegra_gpio_irq_chip,
					 handle_simple_irq);
		set_irq_flags(irq, IRQF_VALID);
	}

	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		bank = &tegra_gpio_banks[i];

		for (j = 0; j < 4; j++)
			spin_lock_init(&bank->lvl_lock[j]);

		irq_set_handler_data(bank->irq, bank);
		irq_set_chained_handler(bank->irq, tegra_gpio_irq_handler);

	}

#ifdef CONFIG_MACH_N1
	/* init the gpio when kernel booting. */
	tegra_set_gpio_init_table();
#endif
	return 0;
}

postcore_initcall(tegra_gpio_init);

void __init tegra_gpio_config(struct tegra_gpio_table *table, int num)
{
	int i;

	for (i = 0; i < num; i++) {
		int gpio = table[i].gpio;

		if (table[i].enable)
			tegra_gpio_enable(gpio);
		else
			tegra_gpio_disable(gpio);
	}
}

#ifdef	CONFIG_DEBUG_FS

#include <linux/debugfs.h>
#include <linux/seq_file.h>

static int dbg_gpio_show(struct seq_file *s, void *unused)
{
	int i;
	int j;

	seq_printf(s, "Bank:Port CNF OE OUT IN INT_STA INT_ENB INT_LVL\n");
	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		for (j = 0; j < 4; j++) {
			int gpio = tegra_gpio_compose(i, j, 0);
			seq_printf(s,
				"%d:%d %02x %02x %02x %02x %02x %02x %06x\n",
				i, j,
				__raw_readl(GPIO_CNF(gpio)),
				__raw_readl(GPIO_OE(gpio)),
				__raw_readl(GPIO_OUT(gpio)),
				__raw_readl(GPIO_IN(gpio)),
				__raw_readl(GPIO_INT_STA(gpio)),
				__raw_readl(GPIO_INT_ENB(gpio)),
				__raw_readl(GPIO_INT_LVL(gpio)));
		}
	}
	return 0;
}

static int dbg_gpio_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_gpio_show, &inode->i_private);
}

static const struct file_operations debug_fops = {
	.open		= dbg_gpio_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init tegra_gpio_debuginit(void)
{
	(void) debugfs_create_file("tegra_gpio", S_IRUGO,
					NULL, NULL, &debug_fops);
	return 0;
}
late_initcall(tegra_gpio_debuginit);
#endif
