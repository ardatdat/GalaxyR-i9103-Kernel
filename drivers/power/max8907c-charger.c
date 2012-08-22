/*
 * Battery driver for Maxim MAX8907C
 *
 * Copyright (c) 2010, NVIDIA Corporation.
 * Copyright (C) 2010 Gyungoh Yoo <jack.yoo@maxim-ic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/mfd/max8907c.h>
#include <linux/power/max8907c-charger.h>
#include <linux/slab.h>

#define HWREV_FOR_EXTERNEL_CHARGER	7

struct max8907c_charger {
	struct max8907c_charger_pdata *pdata;
	struct max8907c *chip;
	struct i2c_client *i2c;
	int online;
};

static enum power_supply_property max8907c_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
};

static void max8907c_charger_init_register(struct max8907c_charger *charger)
{
	max8907c_reg_write(charger->i2c, MAX8907C_REG_LBCNFG, 0x97);
	max8907c_reg_write(charger->i2c, MAX8907C_REG_CHG_CNTL2, 0x34);
	max8907c_reg_write(charger->i2c, MAX8907C_REG_BBAT_CNFG, 0x3);
	/* Following are not needed for now
	max8907c_reg_write(charger->i2c, MAX8907C_REG_CHG_IRQ1_MASK, 0x07);
	max8907c_reg_write(charger->i2c, MAX8907C_REG_CHG_IRQ2_MASK, 0xBF);
	*/
}

static bool max8907c_check_detbat(struct max8907c_charger *charger)
{
	u8 chg_stat = 0;

	chg_stat = max8907c_reg_read(charger->i2c, MAX8907C_REG_CHG_STAT);

	if (chg_stat & CHGSTAT_MBDET_MASK) {
		pr_err("%s: batt not detected(0x%x)\n", __func__, chg_stat);
		return true;
	}

	return false;
}

static bool max8907c_check_vdcin(struct max8907c_charger *charger)
{
	u8 chg_stat = 0;

	chg_stat = max8907c_reg_read(charger->i2c, MAX8907C_REG_CHG_STAT);

	if (chg_stat & CHGSTAT_VCHG_OK_MASK)
		return true;

	return false;
}

static int max8907c_charger_topoff_irq(struct max8907c_charger *charger)
{
	int ret = 0;

	if (charger->pdata->topoff_cb)
		ret = charger->pdata->topoff_cb();

	return ret;
}

static int max8907c_charger_removal_irq(struct max8907c_charger *charger,
								int vdcin)
{
	int ret = 0;

	if (charger->pdata->vchg_f_cb)
		ret = charger->pdata->vchg_f_cb(vdcin);

	return ret;
}

/* FACTORY TEST BINARY */
static irqreturn_t max8907c_charger_vchg_r_f_isr(int irq, void *dev_id)
{
	int ret = 0;
	struct max8907c_charger *charger = dev_id;
	struct max8907c *chip = charger->chip;

	switch (irq - chip->irq_base) {
	case MAX8907C_IRQ_VCHG_DC_R:
		pr_info("max8907c-charger : MAX8907C_IRQ_VCHG_DC_R.\n");

		if (charger->pdata->vchg_r_f_cb)
				ret = charger->pdata->vchg_r_f_cb(1);

		if (unlikely(ret != 0))
			pr_err("max8907c-charger :"
			" Error max8907c_charger_vchg_r_f_isr %d\n", ret);
		break;
	case MAX8907C_IRQ_VCHG_DC_F:
		pr_info("max8907c-charger : MAX8907C_IRQ_VCHG_DC_F.\n");

		if (charger->pdata->vchg_r_f_cb)
				ret = charger->pdata->vchg_r_f_cb(0);

		if (unlikely(ret != 0))
			pr_err("max8907c-charger :"
			" Error max8907c_charger_vchg_r_f_isr %d\n", ret);
		break;
	}

	return IRQ_HANDLED;
}

static irqreturn_t max8907c_charger_removal_isr(int irq, void *dev_id)
{
	int vdcin, ret;

	struct max8907c_charger *charger = dev_id;

	vdcin = max8907c_check_vdcin(charger);
	dev_info(charger->chip->dev, "max8907c-charger :"
			" Occurred charger removal IRQ (%d)", vdcin);

	ret = max8907c_charger_removal_irq(charger, vdcin);

	if (unlikely(ret != 0))
		pr_err("max8907c-charger :"
			" Error from charger_removal_irq %d\n", ret);

	return IRQ_HANDLED;
}

static irqreturn_t max8907c_charger_isr(int irq, void *dev_id)
{
	int cable_state, ret;

	struct max8907c_charger *charger = dev_id;

	cable_state = max8907c_check_vdcin(charger);

	if (cable_state) {
		dev_info(charger->chip->dev, "Max8907c-Charger :"
				" Occurred Topoff IRQ");
		ret = max8907c_charger_topoff_irq(charger);

		if (unlikely(ret != 0))
			pr_err("max8907c-charger : "
					"Error from topoff_irq %d\n", ret);
	}

	return IRQ_HANDLED;
}

static int max8907c_charger_get_property(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct max8907c_charger *charger = dev_get_drvdata(psy->dev->parent);
	int ret;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = 1;
		if (max8907c_check_vdcin(charger))
			val->intval = POWER_SUPPLY_STATUS_CHARGING;
		else
			val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		if (max8907c_check_detbat(charger))
			val->intval = BAT_NOT_DETECTED;
		else
			val->intval = BAT_DETECTED;
		break;

	case POWER_SUPPLY_PROP_ONLINE:
		/* Remove this */
		ret = max8907c_reg_read(charger->i2c, MAX8907C_REG_CHG_STAT);
		pr_info("MAX8907C_REG_CHG_STAT = 0x%x\n", ret);
		/* battery is always online */
		val->intval = 1;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int max8907c_disable_charging(struct max8907c_charger *charger)
{
	int ret;

	dev_info(charger->chip->dev, "MAX8907c-charger : disable charging\n");
	ret = max8907c_set_bits(charger->i2c, MAX8907C_REG_CHG_CNTL1,
		CHGCNTL1_NCHGEN_MASK, (1 << CHGCNTL1_NCHGEN_SHIFT));
	if (ret < 0)
		dev_err(charger->chip->dev, "max8907c-charger : fail update reg!!!\n");

	return ret;
}

static int max8907c_enable_charging(struct max8907c_charger *charger)
{
	int ret = 0;
	int chg_stat;
	int chg_mode = 0;

	/* set charging enable */
	dev_info(charger->chip->dev, "MAX8907c-charger : enable charging\n");
	ret = max8907c_set_bits(charger->i2c, MAX8907C_REG_CHG_CNTL1,
		CHGCNTL1_NCHGEN_MASK, (0 << CHGCNTL1_NCHGEN_SHIFT));
	if (ret)
		goto err;

	chg_stat = max8907c_reg_read(charger->i2c, MAX8907C_REG_CHG_STAT);
	chg_mode =
		((chg_stat & CHGSTAT_CHG_MODE_MASK) >> CHGSTAT_CHG_MODE_SHIFT);

	if (chg_mode == MAX8907C_TOP_OFF) {
		pr_info("%s : "
		"MAX8907C_REG_CHG_STAT = 0x%x , TOPOFFed. "
		"disable ane re-enable charging\n", __func__, chg_stat);

		ret = max8907c_set_bits(charger->i2c, MAX8907C_REG_CHG_CNTL1,
			CHGCNTL1_NCHGEN_MASK, (1 << CHGCNTL1_NCHGEN_SHIFT));

		ret = max8907c_set_bits(charger->i2c, MAX8907C_REG_CHG_CNTL1,
			CHGCNTL1_NCHGEN_MASK, (0 << CHGCNTL1_NCHGEN_SHIFT));

		chg_stat = max8907c_reg_read(charger->i2c,
			MAX8907C_REG_CHG_STAT);
		pr_info("%s : MAX8907C_REG_CHG_STAT"
				" = 0x%x.\n", __func__, chg_stat);
	}

	return 0;
err:
	dev_err(charger->chip->dev, "%s: max8907c update reg error!(%d)\n",
			__func__, ret);
	return ret;
}


static int max8907c_set_charging_current(struct max8907c_charger *charger,
								int chg_current)
{
	int ret = 0;

	if (chg_current == 600) {
		/* ac */
		dev_info(charger->chip->dev, "%s: TA charging\n", __func__);
		/* set fast charging current : 600mA */
		ret = max8907c_set_bits(charger->i2c, MAX8907C_REG_CHG_CNTL1,
			    CHGCNTL1_FCHG_MASK, (3 << CHGCNTL1_FCHG_SHIFT));
		if (ret)
			goto err;

	} else if (chg_current == 460) {
		/* usb */
		dev_info(charger->chip->dev, "%s: USB charging\n", __func__);
		/* set fast charging current : 460mA */
		ret = max8907c_set_bits(charger->i2c, MAX8907C_REG_CHG_CNTL1,
				CHGCNTL1_FCHG_MASK, (2 << CHGCNTL1_FCHG_SHIFT));
		if (ret)
			goto err;
	} else {
		dev_err(charger->chip->dev, "%s: invalid arg\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	return 0;
err:
	dev_err(charger->chip->dev,
	"%s: max8907c update reg error!(%d)\n", __func__, ret);
	return ret;
}

static int max8907c_charger_set_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    const union power_supply_propval *val)
{
	int ret;

	struct max8907c_charger *charger = dev_get_drvdata(psy->dev->parent);

	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_NOW: /* Set charging current */
		ret = max8907c_set_charging_current(charger, val->intval);
		break;
	case POWER_SUPPLY_PROP_STATUS:	/* Enable/Disable charging */
		if (val->intval == POWER_SUPPLY_STATUS_CHARGING)
			ret = max8907c_enable_charging(charger);
		else
			ret = max8907c_disable_charging(charger);
		break;

	case POWER_SUPPLY_PROP_CHARGE_FULL: /* Set recharging current */
		if (val->intval < MAX8907C_TOPOFF_5PERCENT ||
			val->intval > MAX8907C_TOPOFF_20PERCENT) {
			dev_err(charger->chip->dev, "%s: invalid topoff current(%d)\n",
					__func__, val->intval);
			return -EINVAL;
		}

		dev_info(charger->chip->dev, "%s: Set topoff current to 0x%x\n",
				__func__, val->intval);

		ret = max8907c_set_bits(charger->i2c, MAX8907C_REG_CHG_CNTL1,
			    CHGCNTL1_CHG_TOPFF_MASK,
				(val->intval << CHGCNTL1_CHG_TOPFF_SHIFT));

		if (ret) {
			dev_err(charger->chip->dev,
			"%s: max8997 update reg error(%d)\n",
					__func__, ret);
			return ret;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static struct power_supply max8907c_charger_ps = {
	.name = "max8907c-charger",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties =  max8907c_battery_props,
	.num_properties = ARRAY_SIZE(max8907c_battery_props),
	.get_property = max8907c_charger_get_property,
	.set_property = max8907c_charger_set_property,
};

static __devinit int max8907c_charger_probe(struct platform_device *pdev)
{
	struct max8907c_charger_pdata *pdata = pdev->dev.platform_data;
	struct max8907c_charger *charger = 0;
	struct max8907c *chip = dev_get_drvdata(pdev->dev.parent);
	int ret;

	pr_info("%s : MAX8907c Charger Driver Loading\n", __func__);

	charger = kzalloc(sizeof(*charger), GFP_KERNEL);
	if (!charger)
		return -ENOMEM;

	charger->pdata = pdata;
	charger->online = 0;
	charger->chip = chip;
	charger->i2c = chip->i2c_power;

	max8907c_charger_init_register(charger);

	ret = max8907c_reg_read(charger->i2c, MAX8907C_REG_CHG_CNTL2);
	pr_info("max8907c-charger : MAX8907C_REG_CHG_CNTL2 = 0x%x.\n", ret);

	ret = max8907c_reg_read(charger->i2c, MAX8907C_REG_BBAT_CNFG);
	pr_info("max8907c-charger : REG_BBAT_CNFG = 0x%x.\n", ret);

	ret = max8907c_reg_read(charger->i2c, MAX8907C_REG_CHG_IRQ1_MASK);
	pr_info("max8907c-charger : CHG_IRQ1_MASK = 0x%x.\n", ret);

	ret = max8907c_reg_read(charger->i2c, MAX8907C_REG_CHG_IRQ2_MASK);
	pr_info("max8907c-charger : CHG_IRQ2_MASK = 0x%x.\n", ret);

	platform_set_drvdata(pdev, charger);

	if (system_rev < HWREV_FOR_EXTERNEL_CHARGER) {
		ret = request_threaded_irq((chip->irq_base +
				MAX8907C_IRQ_VCHG_TOPOFF),
					NULL,
					max8907c_charger_isr,
					IRQF_TRIGGER_RISING,
					"max8907c-charger", charger);

		if (unlikely(ret < 0)) {
			pr_debug("max8907c-Charger:"
				" failed to request IRQ %x\n", ret);
			goto out1;
		}
	}

#if 0 /* FACTORY TEST BINARY */
	ret = request_threaded_irq(chip->irq_base + MAX8907C_IRQ_VCHG_DC_F,
						NULL,
						max8907c_charger_vchg_r_f_isr,
						IRQF_TRIGGER_FALLING,
						"max8907c-vchg_f", charger);

	ret = request_threaded_irq(chip->irq_base + MAX8907C_IRQ_VCHG_DC_R,
						NULL,
						max8907c_charger_vchg_r_f_isr,
						IRQF_TRIGGER_RISING,
						"max8907c-vchg_r", charger);
#else
	ret = request_threaded_irq(chip->irq_base + MAX8907C_IRQ_VCHG_DC_F,
					NULL,
					max8907c_charger_removal_isr,
					IRQF_TRIGGER_FALLING,
					"max8907c-vchg_f", charger);

	if (unlikely(ret < 0)) {
		pr_debug("max8907c-Charger: failed to request IRQ %x\n", ret);
		goto out2;
	}
#endif

	ret = power_supply_register(&pdev->dev, &max8907c_charger_ps);
	if (unlikely(ret != 0)) {
		pr_err("Failed to register max8907c_charger driver: %d\n", ret);
		goto err_supply_unreg_charger;
	}

	return 0;
out1:
	free_irq(chip->irq_base + MAX8907C_IRQ_VCHG_TOPOFF, charger);
out2:
	free_irq(chip->irq_base + MAX8907C_IRQ_VCHG_DC_F, charger);
err_supply_unreg_charger:
	power_supply_unregister(&max8907c_charger_ps);
	kfree(charger);
	return ret;
}

static __devexit int max8907c_charger_remove(struct platform_device *pdev)
{
	struct max8907c_charger *charger = platform_get_drvdata(pdev);
	struct max8907c *chip = charger->chip;
	int ret;

	ret = max8907c_reg_write(charger->i2c,
			MAX8907C_REG_CHG_IRQ1_MASK, 0xFF);
	if (unlikely(ret != 0)) {
		pr_err("Failed to set IRQ1_MASK: %d\n", ret);
		goto out;
	}

	free_irq(chip->irq_base + MAX8907C_IRQ_VCHG_TOPOFF, charger);
	power_supply_unregister(&max8907c_charger_ps);
out:
	kfree(charger);
	return 0;
}

static struct platform_driver max8907c_charger_driver = {
	.probe		= max8907c_charger_probe,
	.remove		= __devexit_p(max8907c_charger_remove),
	.driver		= {
		.name	= "max8907c-charger",
	},
};

static int __init max8907c_charger_init(void)
{
	return platform_driver_register(&max8907c_charger_driver);
}
module_init(max8907c_charger_init);

static void __exit max8907c_charger_exit(void)
{
	platform_driver_unregister(&max8907c_charger_driver);
}
module_exit(max8907c_charger_exit);

MODULE_DESCRIPTION("Charger driver for MAX8907C");
MODULE_AUTHOR("Gyungoh Yoo <jack.yoo@maxim-ic.com>");
MODULE_LICENSE("GPL");

