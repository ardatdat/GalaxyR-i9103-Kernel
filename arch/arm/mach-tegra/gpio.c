/*
 * arch/arm/mach-tegra/gpio.c
 *
 * Copyright (c) 2010 Google, Inc
 *
 * Author:
 *	Erik Gilling <konkers@google.com>
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

#include <linux/io.h>
#include <linux/gpio.h>

#include <mach/iomap.h>
#include <mach/suspend.h>

#include "gpio-names.h"

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_BIT(x)		((x) & 0x7)

#define GPIO_REG(x)		(IO_TO_VIRT(TEGRA_GPIO_BASE) +	\
				 GPIO_BANK(x) * 0x80 +		\
				 GPIO_PORT(x) * 4)

#define GPIO_CNF(x)		(GPIO_REG(x) + 0x00)
#define GPIO_OE(x)		(GPIO_REG(x) + 0x10)
#define GPIO_OUT(x)		(GPIO_REG(x) + 0X20)
#define GPIO_IN(x)		(GPIO_REG(x) + 0x30)
#define GPIO_INT_STA(x)		(GPIO_REG(x) + 0x40)
#define GPIO_INT_ENB(x)		(GPIO_REG(x) + 0x50)
#define GPIO_INT_LVL(x)		(GPIO_REG(x) + 0x60)
#define GPIO_INT_CLR(x)		(GPIO_REG(x) + 0x70)

#define GPIO_MSK_CNF(x)		(GPIO_REG(x) + 0x800)
#define GPIO_MSK_OE(x)		(GPIO_REG(x) + 0x810)
#define GPIO_MSK_OUT(x)		(GPIO_REG(x) + 0X820)
#define GPIO_MSK_INT_STA(x)	(GPIO_REG(x) + 0x840)
#define GPIO_MSK_INT_ENB(x)	(GPIO_REG(x) + 0x850)
#define GPIO_MSK_INT_LVL(x)	(GPIO_REG(x) + 0x860)

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
#ifdef CONFIG_PM
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
};

static int tegra_gpio_compose(int bank, int port, int bit)
{
	return (bank << 5) | ((port & 0x3) << 3) | (bit & 0x7);
}

static void tegra_gpio_mask_write(u32 reg, int gpio, int value)
{
	u32 val;

	val = 0x100 << GPIO_BIT(gpio);
	if (value)
		val |= 1 << GPIO_BIT(gpio);
	__raw_writel(val, reg);
}

void tegra_gpio_enable(int gpio)
{
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 1);
}

void tegra_gpio_disable(int gpio)
{
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 0);
}

static void tegra_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	tegra_gpio_mask_write(GPIO_MSK_OUT(offset), offset, value);
}

static int tegra_gpio_get(struct gpio_chip *chip, unsigned offset)
{
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



static struct gpio_chip tegra_gpio_chip = {
	.label			= "tegra-gpio",
	.direction_input	= tegra_gpio_direction_input,
	.get			= tegra_gpio_get,
	.direction_output	= tegra_gpio_direction_output,
	.set			= tegra_gpio_set,
	.base			= 0,
	.ngpio			= TEGRA_NR_GPIOS,
};

static void tegra_gpio_irq_ack(unsigned int irq)
{
	int gpio = irq - INT_GPIO_BASE;

	__raw_writel(1 << GPIO_BIT(gpio), GPIO_INT_CLR(gpio));
}

static void tegra_gpio_irq_mask(unsigned int irq)
{
	int gpio = irq - INT_GPIO_BASE;

	tegra_gpio_mask_write(GPIO_MSK_INT_ENB(gpio), gpio, 0);
}

static void tegra_gpio_irq_unmask(unsigned int irq)
{
	int gpio = irq - INT_GPIO_BASE;

	tegra_gpio_mask_write(GPIO_MSK_INT_ENB(gpio), gpio, 1);
}

static int tegra_gpio_irq_set_type(unsigned int irq, unsigned int type)
{
	int gpio = irq - INT_GPIO_BASE;
	struct tegra_gpio_bank *bank = get_irq_chip_data(irq);
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
		__set_irq_handler_unlocked(irq, handle_level_irq);
	else if (type & (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
		__set_irq_handler_unlocked(irq, handle_edge_irq);

	if (tegra_get_suspend_mode() == TEGRA_SUSPEND_LP0)
		tegra_set_lp0_wake_type(irq, type);

	return 0;
}

static void tegra_gpio_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	struct tegra_gpio_bank *bank;
	int port;
	int pin;
	int unmasked = 0;

	desc->chip->ack(irq);

	bank = get_irq_data(irq);
	if (!bank)
		printk(KERN_ERR "%s: irq(%d/0x%x)\n", __func__, irq, irq);

	for (port = 0; port < 4; port++) {
		int gpio = tegra_gpio_compose(bank->bank, port, 0);
		unsigned long sta = __raw_readl(GPIO_INT_STA(gpio)) &
			__raw_readl(GPIO_INT_ENB(gpio));
		u32 lvl = __raw_readl(GPIO_INT_LVL(gpio));

		for_each_set_bit(pin, &sta, 8) {
			__raw_writel(1 << pin, GPIO_INT_CLR(gpio));

			/* if gpio is edge triggered, clear condition
			 * before executing the hander so that we don't
			 * miss edges
			 */
			if (lvl & (0x100 << pin)) {
				unmasked = 1;
				desc->chip->unmask(irq);
			}

			generic_handle_irq(gpio_to_irq(gpio + pin));
		}
	}

	if (!unmasked)
		desc->chip->unmask(irq);

}

#ifdef CONFIG_PM
#ifdef CONFIG_MACH_N1
#include <mach/gpio-n1.h>

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
	{YES,	GPIO_ALC_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_ALC_I2C_SCL"},
	{YES,	GPIO_THERMAL_I2C_SCL,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_THERMAL_I2C_SCL"},
	{YES,	GPIO_ALC_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_ALC_I2C_SDA"},
	{YES,	GPIO_THERMAL_I2C_SDA,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_THERMAL_I2C_SDA"},
	{NO,	GPIO_HSIC_EN,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HSIC_EN"},
	{YES,	GPIO_PDA_ACTIVE,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_PDA_ACTIVE"},
	{NO,	GPIO_PHONE_ACTIVE,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_PHONE_ACTIVE"},
	{NO,	GPIO_EAR_SEND_END,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_EAR_SEND_END"},
	{NO,	GPIO_GPS_NRST,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GPS_NRST"},
	{YES,	GPIO_MLCD_RST,				GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_MLCD_RST"},
	{YES,	GPIO_CMC_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_I2C_SCL"},
	{NO,	GPIO_GPS_EN,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GPS_EN"},
	{NO,	GPIO_CMC_SLEEP,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_SLEEP"},
	{NO,	GPIO_GPS_CNTL,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GPS_CNTL"},
	{YES,	GPIO_CMC_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_I2C_SDA"},
	{NO,	GPIO_BT_nRST,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_BT_nRST"},
	{NO,	GPIO_CAM_VT_nRST,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_VT_nRST"},
	{NO,	GPIO_CAM_VT_nSTBY,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_VT_nSTBY"},
	{NO,	GPIO_JACK_nINT,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_JACK_nINT"},
	{YES,	GPIO_CMC_LDO_EN1,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_CMC_LDO_EN1"},
	{NO,	GPIO_NFC_FIRMWARE,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_NFC_FIRMWARE"},
	{NO,	GPIO_CP_DUMP_INT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CP_DUMP_INT"},
	{YES,	GPIO_CAM_PMIC_EN2,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_CAM_PMIC_EN2"},
	{NO,	GPIO_CAM_MEGA_STBY,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_MEGA_STBY"},
	{NO,	GPIO_TSP_INT,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_TSP_INT"},
	{YES,	GPIO_USB_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_USB_I2C_SCL"},
/* SFIO sleep setting */
//	{YES,	GPIO_CAM_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_I2C_SCL"},
//	{YES,	GPIO_CAM_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_I2C_SDA"},
	{YES,	GPIO_USB_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_USB_I2C_SDA"},
	{NO,	GPIO_CMC_BYPASS,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_BYPASS"},
	{NO,	GPIO_CAM_MEGA_nRST,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAM_MEGA_nRST"},
	{NO,	GPIO_GYRO_FIFO_INT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GYRO_FIFO_INT"},
	{NO,	GPIO_CMC_RST,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_RST"},
	{NO,	GPIO_BT_EN,					GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_BT_EN"},
	{NO,	GPIO_HWREV2,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HWREV2"},
	{NO,	GPIO_EXT_WAKEUP,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_EXT_WAKEUP"},
	{NO,	GPIO_nTHRM_IRQ,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_nTHRM_IRQ"},
/* SFIO sleep setting */
//	{YES,	GPIO_GYRO_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GYRO_I2C_SCL"},
//	{YES,	GPIO_GYRO_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_GYRO_I2C_SDA"},
	{NO,	GPIO_FM_INT,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FM_INT"},
	{NO,	GPIO_FM_RST_04,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FM_RST_04"},
	{NO,	GPIO_FM_RST_05,				GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_FM_RST_05"},
	{YES,	GPIO_FM_SCL_18V,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FM_SCL_18V"},
	{YES,	GPIO_FM_SDA_18V,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FM_SDA_18V"},
	{NO,	GPIO_LCD_LED_EN_SET,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LCD_LED_EN_SET"},
	{NO,	GPIO_IPC_SLAVE_WAKEUP,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_IPC_SLAVE_WAKEUP"},
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
	{NO,	GPIO_CMC_SHDN,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CMC_SHDN"},
	{NO,	GPIO_ACTIVE_STATE_HSIC,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_ACTIVE_STATE_HSIC"},
	{NO,	GPIO_AK8975_INT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_AK8975_INT"},
	{NO,	GPIO_TA_CURRENT_SEL,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_TA_CURRENT_SEL"},
	{NO,	GPIO_CHG_EN,				GPIO_OUTPUT,	GPIO_LEVEL_NONE,		"GPIO_CHG_EN"},
	{YES,	GPIO_CAM_FLASH_SET,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,	"GPIO_CAM_FLASH_SET"},
	{YES,	GPIO_CAM_FLASH_EN,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,	    "GPIO_CAM_FLASH_EN"},	
	{NO,	GPIO_HOME_KEY,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HOME_KEY"},
	{YES,	GPIO_CODEC_LDO_EN,			GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_CODEC_LDO_EN"},
	{YES,	GPIO_MICBIAS1_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MICBIAS1_EN"},
	{YES,	GPIO_UART_SEL,				GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_UART_SEL"},
	{NO,	GPIO_UART_SEL_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_UART_SEL_EN"},
	{NO,	GPIO_SUSPEND_REQUEST_HSIC,	GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_SUSPEND_REQUEST_HSIC"},
	{NO,	GPIO_DET_3_5,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_DET_3_5"},
//	{YES,	GPIO_HDMI_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HDMI_I2C_SCL"},
//	{YES,	GPIO_HDMI_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HDMI_I2C_SDA "},
	{NO,	GPIO_FUEL_ALERT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_FUEL_ALERT "},
	{NO,	GPIO_LED_LDO_EN1,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LED_LDO_EN1 "},
//	{YES,	GPIO_MHL_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MHL_I2C_SCL "},
//	{YES,	GPIO_MHL_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MHL_I2C_SDA "},
	{NO,	GPIO_COMPASS_INT,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_COMPASS_INT "},
	{NO,	GPIO_LED_LDO_EN2,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LED_LDO_EN2 "},
//	{YES,	GPIO_LED_LDO_EN3,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LED_LDO_EN3 "},
//	{YES,	GPIO_LED_LDO_EN4,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LED_LDO_EN4 "},
	{NO,	GPIO_HWREV3,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HWREV3 "},
	{NO,	GPIO_LCD_ID1,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_LCD_ID1"},
	{YES,	GPIO_CODEC_I2C_SDA,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CODEC_I2C_SDA "},
	{NO,	GPIO_MICBIAS2_EN,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_MICBIAS2_EN "},
	{YES,	GPIO_HWREV0,				GPIO_INPUT,	    GPIO_LEVEL_NONE,	"GPIO_HWREV0 "},
	{YES,	GPIO_HWREV1,				GPIO_INPUT,	    GPIO_LEVEL_NONE,	"GPIO_HWREV1 "},
	{YES,	GPIO_HWREV2,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HWREV2 "},
	{YES,	GPIO_HWREV3,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_HWREV3 "},
	{YES,	GPIO_CODEC_I2C_SCL,			GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CODEC_I2C_SCL "},
	{NO,	GPIO_EAR_SEL,				GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_EAR_SEL "},
	{YES,	GPIO_VIBTONE_I2C_SCL,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_VIBTONE_I2C_SCL "},
	{YES,	GPIO_VIBTONE_I2C_SDA,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_VIBTONE_I2C_SDA "},
	{YES,	GPIO_CAMPMIC_SCL_18V,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAMPMIC_SCL_18V "},
	{YES,	GPIO_CAMPMIC_SDA_18V,		GPIO_INPUT,		GPIO_LEVEL_NONE,	"GPIO_CAMPMIC_SDA_18V "},
};

static struct sec_gpio_cfg_st n1_sleep_gpio_table_07[] = {
	{YES,	GPIO_FM_RST_05,				GPIO_OUTPUT,	GPIO_LEVEL_LOW,		"GPIO_FM_RST_05"},
};

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
			tegra_gpio_enable(n1_sleep_gpio_table[cnt].gpio);
			if (n1_sleep_gpio_table[cnt].dir == GPIO_OUTPUT) {
				gpio_direction_output(n1_sleep_gpio_table[cnt].gpio, n1_sleep_gpio_table[cnt].val);
			} else if (n1_sleep_gpio_table[cnt].dir == GPIO_INPUT) {
				gpio_direction_input(n1_sleep_gpio_table[cnt].gpio);
			}
		}
	}

	if(system_rev >= 7) {
		for (cnt = 0; cnt < ARRAY_SIZE(n1_sleep_gpio_table_07); cnt++) {
			if (n1_sleep_gpio_table_07[cnt].slp_ctrl == YES) {
				tegra_gpio_enable(n1_sleep_gpio_table_07[cnt].gpio);
				if (n1_sleep_gpio_table_07[cnt].dir == GPIO_OUTPUT) {
					gpio_direction_output(n1_sleep_gpio_table_07[cnt].gpio, n1_sleep_gpio_table_07[cnt].val);
				} else if (n1_sleep_gpio_table_07[cnt].dir == GPIO_INPUT) {
					gpio_direction_input(n1_sleep_gpio_table_07[cnt].gpio);
				}
			}
		}
	}
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
/* dedicated to N1 Main Rev02 */
//	{GPIO,	GPIO_USB_SEL2,			GPIO_OUTPUT,	GPIO_LEVEL_NONE },

	/* set NC Pin to input or output when no pull */
	{GPIO,	TEGRA_GPIO_PD0, 		GPIO_INPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PD0" },
	{GPIO,	TEGRA_GPIO_PD1,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PD1" },
	{GPIO,	TEGRA_GPIO_PD3, 		GPIO_INPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PD3" },
	{GPIO,	TEGRA_GPIO_PD4,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PD4" },
	{GPIO,	TEGRA_GPIO_PG1, 		GPIO_INPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PG1" },
	{GPIO,	TEGRA_GPIO_PG2, 		GPIO_INPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PG2" },
	{GPIO,	TEGRA_GPIO_PH0, 		GPIO_INPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PH0" },
	{GPIO,	TEGRA_GPIO_PH1,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PH1" },
	{GPIO,	TEGRA_GPIO_PH2, 		GPIO_INPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PH2" },
	{GPIO,	TEGRA_GPIO_PJ0, 		GPIO_OUTPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PJ0" },
	{GPIO,	TEGRA_GPIO_PK3,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PK3" },
	{GPIO,	TEGRA_GPIO_PL2,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PL2" },
	{GPIO,	TEGRA_GPIO_PL3, 		GPIO_INPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PL3" },
	{GPIO,	TEGRA_GPIO_PL4, 		GPIO_INPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PL4" },
	{GPIO,	TEGRA_GPIO_PN7, 		GPIO_OUTPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PN7" },
	{GPIO,	TEGRA_GPIO_PR5,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PR5" },
	{GPIO,	TEGRA_GPIO_PU2,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PU2" },
	{GPIO,	TEGRA_GPIO_PU3,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PU3" },
	{GPIO,	TEGRA_GPIO_PV6,			GPIO_INPUT,		GPIO_LEVEL_LOW,		"TEGRA_GPIO_PV6" },
	{GPIO,	TEGRA_GPIO_PBB0, 		GPIO_OUTPUT, 	GPIO_LEVEL_LOW,		"TEGRA_GPIO_PBB0" },
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
				gpio_free(n1_gpio_table[cnt].gpio);
			}
		} else {
			tegra_gpio_disable(n1_gpio_table[cnt].gpio);
			gpio_free(n1_gpio_table[cnt].gpio);
		}
	}
}

#endif

void tegra_gpio_resume(void)
{
	unsigned long flags;
	int b, p, i;

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

	for (i = INT_GPIO_BASE; i < (INT_GPIO_BASE + TEGRA_NR_GPIOS); i++) {
		struct irq_desc *desc = irq_to_desc(i);
		if (!desc || (desc->status & IRQ_WAKEUP))
			continue;
		enable_irq(i);
	}
}

void tegra_gpio_suspend(void)
{
	unsigned long flags;
	int b, p, i;

	for (i = INT_GPIO_BASE; i < (INT_GPIO_BASE + TEGRA_NR_GPIOS); i++) {
		struct irq_desc *desc = irq_to_desc(i);
		if (!desc)
			continue;
		if (desc->status & IRQ_WAKEUP) {
			int gpio = i - INT_GPIO_BASE;
			pr_debug("gpio %d.%d is wakeup\n", gpio/8, gpio&7);
			continue;
		}
		disable_irq(i);
	}

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
}

static int tegra_gpio_wake_enable(unsigned int irq, unsigned int enable)
{
	int ret;
	struct tegra_gpio_bank *bank = get_irq_chip_data(irq);

	ret = tegra_set_lp1_wake(bank->irq, enable);
	if (ret)
		return ret;

	if (tegra_get_suspend_mode() == TEGRA_SUSPEND_LP0)
		return tegra_set_lp0_wake(irq, enable);

	return 0;
}
#endif

static struct irq_chip tegra_gpio_irq_chip = {
	.name		= "GPIO",
	.ack		= tegra_gpio_irq_ack,
	.mask		= tegra_gpio_irq_mask,
	.unmask		= tegra_gpio_irq_unmask,
	.set_type	= tegra_gpio_irq_set_type,
#ifdef CONFIG_PM
	.set_wake	= tegra_gpio_wake_enable,
#endif
};


/* This lock class tells lockdep that GPIO irqs are in a different
 * category than their parents, so it won't report false recursion.
 */
static struct lock_class_key gpio_lock_class;

static int __init tegra_gpio_init(void)
{
	struct tegra_gpio_bank *bank;
	int i;
	int j;

	for (i = 0; i < 7; i++) {
		for (j = 0; j < 4; j++) {
			int gpio = tegra_gpio_compose(i, j, 0);
			__raw_writel(0x00, GPIO_INT_ENB(gpio));
			__raw_writel(0x00, GPIO_INT_STA(gpio));
		}
	}

	gpiochip_add(&tegra_gpio_chip);

	for (i = INT_GPIO_BASE; i < (INT_GPIO_BASE + TEGRA_NR_GPIOS); i++) {
		bank = &tegra_gpio_banks[GPIO_BANK(irq_to_gpio(i))];

		lockdep_set_class(&irq_desc[i].lock, &gpio_lock_class);
		set_irq_chip_data(i, bank);
		set_irq_chip(i, &tegra_gpio_irq_chip);
		set_irq_handler(i, handle_simple_irq);
		set_irq_flags(i, IRQF_VALID);
	}

	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		bank = &tegra_gpio_banks[i];

		for (j = 0; j < 4; j++)
			spin_lock_init(&bank->lvl_lock[j]);

		set_irq_data(bank->irq, bank);
		set_irq_chained_handler(bank->irq, tegra_gpio_irq_handler);

	}

#ifdef CONFIG_MACH_N1
	/* init the gpio when kernel booting. */
	tegra_set_gpio_init_table();
#endif

	return 0;
}

postcore_initcall(tegra_gpio_init);

#ifdef	CONFIG_DEBUG_FS

#include <linux/debugfs.h>
#include <linux/seq_file.h>

static int dbg_gpio_show(struct seq_file *s, void *unused)
{
	int i;
	int j;

	for (i = 0; i < 7; i++) {
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
