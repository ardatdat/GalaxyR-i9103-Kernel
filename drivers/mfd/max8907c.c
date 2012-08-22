/*
 * max8907c.c - mfd driver for MAX8907c
 *
 * Copyright (C) 2010 Gyungoh Yoo <jack.yoo@maxim-ic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/max8907c.h>
#ifdef CONFIG_MACH_N1
#include <linux/irq.h>
#include <linux/interrupt.h>
#endif

static struct mfd_cell cells[] = {
	{.name = "max8907-regulator",},
	{.name = "max8907c-rtc",},
#ifdef CONFIG_MACH_N1
    {.name = "max8907c-adc",},
#endif
};

static int max8907c_i2c_read(struct i2c_client *i2c, u8 reg, u8 count, u8 *dest)
{
	struct i2c_msg xfer[2];
	int ret = 0;

	xfer[0].addr = i2c->addr;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = &reg;

	xfer[1].addr = i2c->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = count;
	xfer[1].buf = dest;

	ret = i2c_transfer(i2c->adapter, xfer, 2);
	if (ret < 0)
		return ret;
	if (ret != 2)
		return -EIO;

	return 0;
}

static int max8907c_i2c_write(struct i2c_client *i2c, u8 reg, u8 count, const u8 *src)
{
	u8 msg[0x100 + 1];
	int ret = 0;

	msg[0] = reg;
	memcpy(&msg[1], src, count);

	ret = i2c_master_send(i2c, msg, count + 1);
	if (ret < 0)
		return ret;
	if (ret != count + 1)
		return -EIO;

	return 0;
}

#ifdef CONFIG_MACH_N1
int max8907c_send_cmd(struct i2c_client *i2c, u8 cmd)
{
    int ret = 0;

    ret = i2c_master_send(i2c, &cmd, 1);
    if (ret < 0)
        return ret;
    if (ret != 1)
        return -EIO;

    return 0;
}
#endif

int max8907c_reg_read(struct i2c_client *i2c, u8 reg)
{
	int ret;
	u8 val;

	ret = max8907c_i2c_read(i2c, reg, 1, &val);

	pr_debug("max8907c: reg read  reg=%x, val=%x\n",
		 (unsigned int)reg, (unsigned int)val);

	if (ret != 0)
		pr_err("Failed to read max8907c I2C driver: %d\n", ret);
	return val;
}
EXPORT_SYMBOL_GPL(max8907c_reg_read);

int max8907c_reg_bulk_read(struct i2c_client *i2c, u8 reg, u8 count, u8 *val)
{
	int ret;

	ret = max8907c_i2c_read(i2c, reg, count, val);

	pr_debug("max8907c: reg read  reg=%x, val=%x\n",
		 (unsigned int)reg, (unsigned int)*val);

	if (ret != 0)
		pr_err("Failed to read max8907c I2C driver: %d\n", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(max8907c_reg_bulk_read);

int max8907c_reg_write(struct i2c_client *i2c, u8 reg, u8 val)
{
	struct max8907c *max8907c = i2c_get_clientdata(i2c);
	int ret;

	pr_debug("max8907c: reg write  reg=%x, val=%x\n",
		 (unsigned int)reg, (unsigned int)val);

	mutex_lock(&max8907c->io_lock);
	ret = max8907c_i2c_write(i2c, reg, 1, &val);
	mutex_unlock(&max8907c->io_lock);

	if (ret != 0)
		pr_err("Failed to write max8907c I2C driver: %d\n", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(max8907c_reg_write);

int max8907c_reg_bulk_write(struct i2c_client *i2c, u8 reg, u8 count, u8 *val)
{
	struct max8907c *max8907c = i2c_get_clientdata(i2c);
	int ret;

	pr_debug("max8907c: reg write  reg=%x, val=%x\n",
		 (unsigned int)reg, (unsigned int)*val);

	mutex_lock(&max8907c->io_lock);
	ret = max8907c_i2c_write(i2c, reg, count, val);
	mutex_unlock(&max8907c->io_lock);

	if (ret != 0)
		pr_err("Failed to write max8907c I2C driver: %d\n", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(max8907c_reg_bulk_write);

int max8907c_set_bits(struct i2c_client *i2c, u8 reg, u8 mask, u8 val)
{
	struct max8907c *max8907c = i2c_get_clientdata(i2c);
	u8 tmp;
	int ret;

	pr_debug("max8907c: reg write  reg=%02X, val=%02X, mask=%02X\n",
		 (unsigned int)reg, (unsigned int)val, (unsigned int)mask);

	mutex_lock(&max8907c->io_lock);
	ret = max8907c_i2c_read(i2c, reg, 1, &tmp);
	if (ret == 0) {
		val = (tmp & ~mask) | (val & mask);
		ret = max8907c_i2c_write(i2c, reg, 1, &val);
	}
	mutex_unlock(&max8907c->io_lock);

	if (ret != 0)
		pr_err("Failed to write max8907c I2C driver: %d\n", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(max8907c_set_bits);

static struct i2c_client *max8907c_client = NULL;
#ifdef	CONFIG_MACH_N1
static struct i2c_client *max8907c_i2c_client = NULL;
static struct i2c_client *max8907c_rtc_client = NULL;
#endif
int max8907c_power_off(void)
{
#ifdef	CONFIG_MACH_N1
    int ret = -EINVAL;

    if (!max8907c_i2c_client)
        return ret;

    /* Clear ON OFF IRQ1 */
    max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_ON_OFF_IRQ1);

    /* Set up LDO2 after resume, attach to SEQ01 and max power down count to 0x07 */
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL2, MAX8907C_MASK_LDO_SEQ, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOSEQCNT2, MAX8907C_MASK_LDO_OFF_CNT, 0x07);

    /* Attach SD2, LDO 4, 10, 11, 17 to original sequence. */
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_SDCTL2, MAX8907C_MASK_LDO_SEQ, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL4, MAX8907C_MASK_LDO_SEQ, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL10, MAX8907C_MASK_LDO_SEQ, 0x1c);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL11, MAX8907C_MASK_LDO_SEQ, 0x1c);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL17, MAX8907C_MASK_LDO_SEQ, 0x1c);

    max8907c_reg_write(max8907c_i2c_client, MAX8907C_REG_RESET_CNFG, 0x12);

    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_SYSENSEL, 0x10, 0x00);

    /* Attach LDO3, 5, 10 to SEQ1 */
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL3, MAX8907C_MASK_LDO_SEQ, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL5, MAX8907C_MASK_LDO_SEQ, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL10, MAX8907C_MASK_LDO_SEQ, 0x00);

    /* Trun off non SEQ1 LDOs */
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL6, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL7, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL8, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL9, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL11, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL12, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL13, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL14, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL15, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL16, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL17, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL18, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL19, MAX8907C_MASK_LDO_EN, 0x00);
    max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_LDOCTL20, MAX8907C_MASK_LDO_EN, 0x00);

#if DEBUG_PMIC_REG
    max8907c_print_regs();
#endif

    ret = max8907c_set_bits(max8907c_i2c_client, MAX8907C_REG_RESET_CNFG, 0x40, 0x40);
    if (ret)
        return ret;
    return 0;
#else
	if (!max8907c_client)
		return -EINVAL;

	return max8907c_set_bits(max8907c_client, MAX8907C_REG_RESET_CNFG,
						MAX8907C_MASK_POWER_OFF, 0x40);
#endif	/* CONFIG_MACH_N1 */
}

void max8907c_deep_sleep(int enter)
{
	if (!max8907c_client)
		return;

	if (enter) {
		max8907c_reg_write(max8907c_client, MAX8907C_REG_SDSEQCNT1,
						MAX8907C_POWER_UP_DELAY_CNT12);
		max8907c_reg_write(max8907c_client, MAX8907C_REG_SDSEQCNT2,
							MAX8907C_DELAY_CNT0);
		max8907c_reg_write(max8907c_client, MAX8907C_REG_SDCTL2,
							MAX8907C_SD_SEQ2);
	} else {
		max8907c_reg_write(max8907c_client, MAX8907C_REG_SDSEQCNT1,
							MAX8907C_DELAY_CNT0);
		max8907c_reg_write(max8907c_client, MAX8907C_REG_SDCTL2,
							MAX8907C_SD_SEQ1);
		max8907c_reg_write(max8907c_client, MAX8907C_REG_SDSEQCNT2,
				MAX8907C_POWER_UP_DELAY_CNT1 | MAX8907C_POWER_DOWN_DELAY_CNT12);
	}
}

static int max8907c_remove_subdev(struct device *dev, void *unused)
{
	platform_device_unregister(to_platform_device(dev));
	return 0;
}

static int max8907c_remove_subdevs(struct max8907c *max8907c)
{
	return device_for_each_child(max8907c->dev, NULL,
				     max8907c_remove_subdev);
}

static int max8097c_add_subdevs(struct max8907c *max8907c,
				struct max8907c_platform_data *pdata)
{
	struct platform_device *pdev;
	int ret;
	int i;

	for (i = 0; i < pdata->num_subdevs; i++) {
		pdev = platform_device_alloc(pdata->subdevs[i]->name,
					     pdata->subdevs[i]->id);

		pdev->dev.parent = max8907c->dev;
		pdev->dev.platform_data = pdata->subdevs[i]->dev.platform_data;

		ret = platform_device_add(pdev);
		if (ret)
			goto error;
	}
	return 0;

error:
	max8907c_remove_subdevs(max8907c);
	return ret;
}

#if DEBUG_PMIC_REG
#define PRINT_RGULATOR_REGS(_read, _dev, _reg_id)			\
	_read = max8907c_reg_read(_dev, _reg_id);		\
	pr_info("Control:  %s\n", byte_to_binary(_read));	\
	_read = max8907c_reg_read(_dev, _reg_id+1);		\
	pr_info("Sequence: %s\n", byte_to_binary(_read));	\
	_read = max8907c_reg_read(_dev, _reg_id+2);		\
	pr_info("Output:   %s\n", byte_to_binary(_read));

static const char *byte_to_binary(int x)
{
	static char b[9];
	int z;

	b[0] = '\0';
	for (z = 0x80; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}

static int max8907c_print_onoff_regs(void)
{
	int read;

	pr_info(" ****** ON/OFF Controller Registers ******\n");
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SYSENSEL);
	pr_info("%s :MAX8907C_REG_SYSENSEL\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_ON_OFF_IRQ1);
	pr_info("%s :MAX8907C_REG_ON_OFF_IRQ1\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_ON_OFF_IRQ1_MASK);
	pr_info("%s :MAX8907C_REG_ON_OFF_IRQ1_MASK\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_ON_OFF_STAT);
	pr_info("%s :MAX8907C_REG_ON_OFF_STAT\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_ON_OFF_IRQ2);
	pr_info("%s :MAX8907C_REG_ON_OFF_IRQ2\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_ON_OFF_IRQ2_MASK);
	pr_info("%s :MAX8907C_REG_ON_OFF_IRQ2_MASK\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_RESET_CNFG);
	pr_info("%s :MAX8907C_REG_RESET_CNFG\n", byte_to_binary(read));

	return 0;
}
static int max8907c_print_fseq_regs(void)
{
	int read;

	pr_info(" ****** Flexible Sequencer Registers ******\n");
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SEQ1CNFG);
	pr_info("%s :MAX8907C_REG_SEQ1CNFG\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SEQ2CNFG);
	pr_info("%s :MAX8907C_REG_SEQ2CNFG\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SEQ3CNFG);
	pr_info("%s :MAX8907C_REG_SEQ3CNFG\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SEQ4CNFG);
	pr_info("%s :MAX8907C_REG_SEQ4CNFG\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SEQ5CNFG);
	pr_info("%s :MAX8907C_REG_SEQ5CNFG\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SEQ6CNFG);
	pr_info("%s :MAX8907C_REG_SEQ6CNFG\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SEQ7CNFG);
	pr_info("%s :MAX8907C_REG_SEQ7CNFG\n", byte_to_binary(read));

	return 0;
}

static int max8907c_print_sd_regs(void)
{
	int read;

	pr_info(" ****** Step-Down Regulator Registers ******\n");
	pr_info(" *********    Step-Down 1    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_SDCTL1);
	pr_info(" *********    Step-Down 2    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_SDCTL2);
	pr_info(" *********    Step-Down 3    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_SDCTL3);
	return 0;
}
static int max8907c_print_ldo_regs(void)
{
	int read;

	pr_info(" ****** Linear Regulator Registers ******\n");
	pr_info(" *********    LDO 1    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL1);
	pr_info(" *********    LDO 2    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL2);
	pr_info(" *********    LDO 3    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL3);
	pr_info(" *********    LDO 4    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL4);
	pr_info(" *********    LDO 5    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL5);
	pr_info(" *********    LDO 6    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL6);
	pr_info(" *********    LDO 7    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL7);
	pr_info(" *********    LDO 8    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL8);
	pr_info(" *********    LDO 9    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL9);
	pr_info(" *********    LDO 10    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL10);
	pr_info(" *********    LDO 11    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL11);
	pr_info(" *********    LDO 12    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL12);
	pr_info(" *********    LDO 13    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL13);
	pr_info(" *********    LDO 14    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL14);
	pr_info(" *********    LDO 15    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL15);
	pr_info(" *********    LDO 16    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL16);
	pr_info(" *********    LDO 17    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL17);
	pr_info(" *********    LDO 18    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL18);
	pr_info(" *********    LDO 19    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL19);
	pr_info(" *********    LDO 20    *********\n");
	PRINT_RGULATOR_REGS(read, max8907c_i2c_client, MAX8907C_REG_LDOCTL20);

	return 0;
}

static int max8907c_print_etcreg_regs(void)
{
	int read;

	pr_info(" ****** Back-Up Batt & STBY LDO Registers ******\n");
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_SDBYSEQCNT);
	pr_info("%s :MAX8907C_REG_SDBYSEQCNT\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_BBAT_CNFG);
	pr_info("%s :MAX8907C_REG_BBAT_CNFG\n", byte_to_binary(read));

	pr_info(" ****** OU5V & OUT3.3V Registers ******\n");
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_OUT5VEN);
	pr_info("%s :MAX8907C_REG_OUT5VEN\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_OUT5VSEQ);
	pr_info("%s :MAX8907C_REG_OUT5VSEQ\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_OUT33VEN);
	pr_info("%s :MAX8907C_REG_OUT33VEN\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_i2c_client, MAX8907C_REG_OUT33VSEQ);
	pr_info("%s :MAX8907C_REG_OUT33VSEQ\n", byte_to_binary(read));

	return 0;
}

static int max8907c_print_rtc_regs(void)
{
	int read, ret;
	u8 buf[8];

	pr_info(" ****** RTC Registers ******\n");
	read = max8907c_reg_read(max8907c_rtc_client, MAX8907C_REG_RTC_STATUS);
	pr_info("%s :MAX8907C_REG_RTC_STATUS\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_rtc_client, MAX8907C_REG_RTC_CNTL);
	pr_info("%s :MAX8907C_REG_RTC_CNTL\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_rtc_client, MAX8907C_REG_RTC_IRQ);
	pr_info("%s :MAX8907C_REG_RTC_IRQ\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_rtc_client, MAX8907C_REG_RTC_IRQ_MASK);
	pr_info("%s :MAX8907C_REG_RTC_IRQ_MASK\n", byte_to_binary(read));
	read = max8907c_reg_read(max8907c_rtc_client, MAX8907C_REG_MPL_CNTL);
	pr_info("%s :MAX8907C_REG_MPL_CNTL\n", byte_to_binary(read));

	ret = max8907c_reg_bulk_read(max8907c_rtc_client, MAX8907C_REG_RTC_SEC, 8, buf);
	if (ret < 0)
		return ret;
	pr_info("MAX8907C-RTC: RTC: %2x%2x/%2x/%2x %x %2x:%2x:%2x\n",
		buf[7], buf[6], buf[5]+1, buf[4], buf[3], buf[2], buf[1], buf[0]);

	read = max8907c_reg_read(max8907c_rtc_client, MAX8907C_REG_ALARM0_CNTL);
	pr_info("%s :MAX8907C_REG_ALARM0_CNTL\n", byte_to_binary(read));

	ret = max8907c_reg_bulk_read(max8907c_rtc_client, MAX8907C_REG_ALARM0_SEC, 8, buf);
	if (ret < 0)
		return ret;
	pr_info("MAX8907C-RTC: ALARM0: %2x%2x/%2x/%2x %x %2x:%2x:%2x\n",
		buf[7], buf[6], buf[5]+1, buf[4], buf[3], buf[2], buf[1], buf[0]);
	return 0;
}

int max8907c_print_regs(void)
{
	u8 printMask;
 #if !defined(CONFIG_MACH_BOSE_ATT)
	printMask = PRINT_PMIC_REG_MASK &
		( PRINT_PMIC_REG_ONOFF |
		PRINT_PMIC_REG_RTC);

	if (printMask & PRINT_PMIC_REG_ONOFF) {
		max8907c_print_onoff_regs();
	}
	if (printMask & PRINT_PMIC_REG_SEQ) {
		max8907c_print_fseq_regs();
	}
	if (printMask & PRINT_PMIC_REG_SD) {
		max8907c_print_sd_regs();
	}
	if (printMask & PRINT_PMIC_REG_LDO) {
		max8907c_print_ldo_regs();
	}
	if (printMask & PRINT_PMIC_REG_ETC) {
		max8907c_print_etcreg_regs();
	}
	if (printMask & PRINT_PMIC_REG_RTC) {
		max8907c_print_rtc_regs();
	}
#else
{
    int read=0;
    int i;


    printk("\n\n=================MAX8907C register map=========================\n");
    for(i=MAX8907C_REG_SYSENSEL; i < MAX8907C_REG_LDO20VOUT + 1; i++){
        read = max8907c_reg_read(max8907c_i2c_client, i);
        printk("0x%2x:0x%2x |", i, read);         // byte_to_binary(read)
        read=0;
        if(((i+1)%10)==0) printk("\n");
    }
    printk("\n");
    max8907c_print_rtc_regs();
}
#endif
	return 0;
}
EXPORT_SYMBOL_GPL(max8907c_print_regs);
#endif

int max8907c_pwr_en_config(void)
{
	int ret;
	u8 data;

	if (!max8907c_client)
		return -EINVAL;

	/*
	 * Enable/disable PWREN h/w control mechanism (PWREN signal must be
	 * inactive = high at this time)
	 */
	ret = max8907c_set_bits(max8907c_client, MAX8907C_REG_RESET_CNFG,
					MAX8907C_MASK_PWR_EN, MAX8907C_PWR_EN);
	if (ret != 0)
		return ret;

	/*
	 * When enabled, connect PWREN to SEQ2 by clearing SEQ2 configuration
	 * settings for silicon revision that requires s/w WAR. On other
	 * MAX8907B revisions PWREN is always connected to SEQ2.
	 */
	data = max8907c_reg_read(max8907c_client, MAX8907C_REG_II2RR);

	if (data == MAX8907B_II2RR_PWREN_WAR) {
		data = 0x00;
		ret = max8907c_reg_write(max8907c_client, MAX8907C_REG_SEQ2CNFG, data);
	}
	return ret;
}

int max8907c_pwr_en_attach(void)
{
	int ret;

	if (!max8907c_client)
		return -EINVAL;

	/* No sequencer delay for CPU rail when it is attached */
	ret = max8907c_reg_write(max8907c_client, MAX8907C_REG_SDSEQCNT1,
							MAX8907C_DELAY_CNT0);
	if (ret != 0)
		return ret;

	return max8907c_set_bits(max8907c_client, MAX8907C_REG_SDCTL1,
					MAX8907C_MASK_CTL_SEQ, MAX8907C_CTL_SEQ);
}

#ifdef CONFIG_MACH_N1
static int max8907c_init_regs(struct i2c_client *i2c)
{
	max8907c_reg_write(i2c, MAX8907C_REG_RESET_CNFG, 0x93);

	/* Attach SD2, LDO 4, 10, 11, 17 to SEQ02. */
	max8907c_set_bits(i2c, MAX8907C_REG_SDCTL2,
		MAX8907C_MASK_LDO_SEQ, 0x04);
	max8907c_set_bits(i2c, MAX8907C_REG_LDOCTL3,
		MAX8907C_MASK_LDO_EN | MAX8907C_MASK_LDO_SEQ,
		MAX8907C_MASK_LDO_EN | MAX8907C_MASK_LDO_SEQ);
	max8907c_set_bits(i2c, MAX8907C_REG_LDOCTL4,
		MAX8907C_MASK_LDO_SEQ, MAX8907C_MASK_LDO_SEQ);
	max8907c_set_bits(i2c, MAX8907C_REG_LDOCTL5,
		MAX8907C_MASK_LDO_SEQ, MAX8907C_MASK_LDO_SEQ);
	max8907c_set_bits(i2c, MAX8907C_REG_LDOCTL10,
		MAX8907C_MASK_LDO_SEQ, 0x04);
	max8907c_set_bits(i2c, MAX8907C_REG_LDOCTL17,
		MAX8907C_MASK_LDO_SEQ, 0x04);

    /* Set up LDO2 for suspend, attach to SEQ02 and max power down count to 0x0F */
    max8907c_set_bits(i2c, MAX8907C_REG_LDOCTL2, MAX8907C_MASK_LDO_SEQ, 0x04);
    max8907c_set_bits(i2c, MAX8907C_REG_LDOSEQCNT2, MAX8907C_MASK_LDO_OFF_CNT, 0x0F);
}

static int max8907c_suspend_regs(struct i2c_client *i2c)
{
    /* Register LDO11 to SEQ2 in suspend because of voltage drop problem */
    max8907c_set_bits(i2c, MAX8907C_REG_LDOCTL11, MAX8907C_MASK_LDO_SEQ, 0x04);
}
#endif /* CONFIG_MACH_N1 */

static int max8907c_i2c_probe(struct i2c_client *i2c,
			      const struct i2c_device_id *id)
{
	struct max8907c *max8907c;
	struct max8907c_platform_data *pdata = i2c->dev.platform_data;
	int ret;
	int i;
	u8 tmp;

	max8907c = kzalloc(sizeof(struct max8907c), GFP_KERNEL);
	if (max8907c == NULL)
		return -ENOMEM;

	max8907c->dev = &i2c->dev;
	dev_set_drvdata(max8907c->dev, max8907c);

	max8907c->i2c_power = i2c;
	i2c_set_clientdata(i2c, max8907c);

	max8907c->i2c_rtc = i2c_new_dummy(i2c->adapter, RTC_I2C_ADDR);
	i2c_set_clientdata(max8907c->i2c_rtc, max8907c);

#ifdef CONFIG_MACH_N1
    max8907c->i2c_adc = i2c_new_dummy(i2c->adapter, ADC_I2C_ADDR);
    i2c_set_clientdata(max8907c->i2c_adc, max8907c);

    max8907c_i2c_client = i2c;
    max8907c_rtc_client = max8907c->i2c_rtc;
#endif
	mutex_init(&max8907c->io_lock);

	for (i = 0; i < ARRAY_SIZE(cells); i++) {
		cells[i].platform_data = max8907c;
		cells[i].pdata_size = sizeof(*max8907c);
	}
	ret = mfd_add_devices(max8907c->dev, -1, cells, ARRAY_SIZE(cells),
			      NULL, 0);
	if (ret != 0) {
	  	i2c_unregister_device(max8907c->i2c_rtc);
#ifdef CONFIG_MACH_N1
        i2c_unregister_device(max8907c->i2c_adc);
#endif
		kfree(max8907c);
		pr_debug("max8907c: failed to add MFD devices   %X\n", ret);
		return ret;
	}

	max8907c_client = i2c;

	max8907c_irq_init(max8907c, i2c->irq, pdata->irq_base);

#ifdef CONFIG_MACH_N1
    ret = max8907c_reg_write(i2c, MAX8907C_REG_SYSENSEL,
        MAX8907C_MASK_HRDRSTEN);
    if (ret != 0)
        return ret;

#if !defined(CONFIG_MACH_BOSE_ATT)
    /* Set HRDRSTEN bit for HW Rev 15 or greater. */
    if(system_rev > 14) {
        ret = max8907c_reg_write(i2c, MAX8907C_REG_SYSENSEL,
            0xbf);
        if (ret != 0)
            return ret;
    }
#endif
    ret = max8907c_set_bits(i2c, MAX8907C_REG_SYSENSEL,
        MAX8907C_MASK_ALARM0_WAKE | MAX8907C_MASK_ALARM1_WAKE, 0x00);
    if (ret != 0)
        return ret;
#endif

	ret = max8097c_add_subdevs(max8907c, pdata);

#ifdef CONFIG_MACH_N1
    /* Initialize max8907c registers */
    max8907c_init_regs(i2c);
#endif

	ret = max8907c_i2c_read(i2c, MAX8907C_REG_SYSENSEL, 1, &tmp);
	/*Mask HARD RESET, if enabled */
	if (ret == 0) {
		tmp &= ~(BIT(7));
		ret = max8907c_i2c_write(i2c, MAX8907C_REG_SYSENSEL, 1, &tmp);
	}

	if (ret != 0) {
		pr_err("Failed to write max8907c I2C driver: %d\n", ret);
		return ret;
	}

	if (pdata->max8907c_setup)
		return pdata->max8907c_setup();

	return ret;
}

#ifdef CONFIG_MACH_N1
static int max8907c_suspend(struct i2c_client *i2c, pm_message_t state)
{
    max8907c_suspend_regs(i2c);

#if DEBUG_PMIC_REG
    max8907c_print_regs();
#endif
    max8907c_irq_suspend(i2c, state);
    return 0;
}

static int max8907c_resume(struct i2c_client *i2c)
{
    max8907c_irq_resume(i2c);
    return 0;
}
#endif

static int max8907c_i2c_remove(struct i2c_client *i2c)
{
	struct max8907c *max8907c = i2c_get_clientdata(i2c);

	max8907c_remove_subdevs(max8907c);
	i2c_unregister_device(max8907c->i2c_rtc);
#ifdef CONFIG_MACH_N1
    i2c_unregister_device(max8907c->i2c_adc);
#endif
	mfd_remove_devices(max8907c->dev);
	max8907c_irq_free(max8907c);
	kfree(max8907c);

	return 0;
}

static const struct i2c_device_id max8907c_i2c_id[] = {
	{"max8907c", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, max8907c_i2c_id);

static struct i2c_driver max8907c_i2c_driver = {
	.driver = {
		   .name = "max8907c",
		   .owner = THIS_MODULE,
		   },
	.probe = max8907c_i2c_probe,
	.remove = max8907c_i2c_remove,
	.suspend = max8907c_suspend,
	.resume = max8907c_resume,
	.id_table = max8907c_i2c_id,
};

static int __init max8907c_i2c_init(void)
{
	int ret = -ENODEV;

	ret = i2c_add_driver(&max8907c_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register I2C driver: %d\n", ret);

	return ret;
}

subsys_initcall(max8907c_i2c_init);

static void __exit max8907c_i2c_exit(void)
{
	i2c_del_driver(&max8907c_i2c_driver);
}

module_exit(max8907c_i2c_exit);

MODULE_DESCRIPTION("MAX8907C multi-function core driver");
MODULE_AUTHOR("Gyungoh Yoo <jack.yoo@maxim-ic.com>");
MODULE_LICENSE("GPL");
