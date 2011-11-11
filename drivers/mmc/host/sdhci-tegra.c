/*
 * drivers/mmc/host/sdhci-tegra.c
 *
 * Copyright (C) 2009 Palm, Inc.
 * Author: Yvonne Yip <y@palm.com>
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

#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/mmc/card.h>

#include <mach/sdhci.h>
#ifdef CONFIG_MACH_N1
#include <linux/regulator/consumer.h>
#endif

#include "sdhci.h"

#define DRIVER_NAME    "sdhci-tegra"

#define SDHCI_VENDOR_CLOCK_CNTRL       0x100
#define SDHCI_TEGRA_MIN_CONTROLLER_CLOCK	12000000

#ifdef CONFIG_MACH_BOSE_ATT
#include <linux/mfd/stmpe.h>
#endif
#ifdef CONFIG_MACH_BOSE_ATT
extern struct stmpe *g_stmpe;
extern int stmpe_reg_read(struct stmpe *stmpe, u8 reg);
#endif

#ifdef CONFIG_MACH_N1
/* FIXME N1 Device Specific */
extern struct platform_device *tegra_sdhci_device0_ptr;
#endif

#if defined CONFIG_MACH_BOSE_ATT

struct sdhci_host *card_sdhost = NULL;

#endif



struct tegra_sdhci_host {
	struct sdhci_host *sdhci;
	struct clk *clk;
	int clk_enabled;
	bool card_always_on;
	u32 sdhci_ints;
#if defined CONFIG_MACH_BOSE_ATT
	int cd_gpio;
	int cd_gpio_polarity;
#endif	
	int wp_gpio;
#if defined CONFIG_MACH_BOSE_ATT
	int card_present;
	unsigned int acquire_spinlock;
#endif
};

#if defined CONFIG_MACH_BOSE_ATT
irqreturn_t external_carddetect_irq()
{
	struct tegra_sdhci_host *host = sdhci_priv(card_sdhost);
	if (card_sdhost) {
		if (stmpe_reg_read(g_stmpe,0x17) & 0x80) {
			printk(KERN_ERR "sdhci-tegra: card removed.\n");
			mmc_host_sd_clear_present(card_sdhost->mmc);
			host->card_present = 0;
		} else {
			printk(KERN_ERR "sdhci-tegra: card inserted.\n");
			mmc_host_sd_set_present(card_sdhost->mmc);
			host->card_present = 1;
		}
		printk(KERN_DEBUG "sdhci-tegra: card present state=0x%x.\n",
				mmc_host_sd_present(card_sdhost->mmc));

		tasklet_schedule(&card_sdhost->card_tasklet);
	}
	return IRQ_HANDLED;
}
#endif 

static irqreturn_t carddetect_irq(int irq, void *data)
{
	struct sdhci_host *sdhost = (struct sdhci_host *)data;

#if defined CONFIG_MACH_BOSE_ATT
	struct tegra_sdhci_host *host = sdhci_priv(sdhost);
	unsigned int present;

	present = (gpio_get_value(host->cd_gpio) == host->cd_gpio_polarity);

	if( present != host->card_present)
	{
//		printk(KERN_ERR"%s, card present is %d\n",__func__,present);
		host->card_present = present;
		if (present) {
			printk(KERN_ERR "sdhci-tegra: card inserted.\n");
			mmc_host_sd_set_present(sdhost->mmc);
		} else {
			printk(KERN_ERR "sdhci-tegra: card removed.\n");
			mmc_host_sd_clear_present(sdhost->mmc);
		}
	}	
	else
		return IRQ_HANDLED;
#endif

	tasklet_schedule(&sdhost->card_tasklet);
	return IRQ_HANDLED;
};

#if defined CONFIG_MACH_BOSE_ATT
static void tegra_sdhci_update_card_detection(struct sdhci_host *sdhci)
{
	struct tegra_sdhci_host *host = sdhci_priv(sdhci);
	unsigned int present;

	if(host->cd_gpio != -1 )
	{
		if (card_sdhost) {
			if (stmpe_reg_read(g_stmpe,0x17) & 0x80)
				host->card_present = 0;
			else
				host->card_present = 1;

			tasklet_schedule(&card_sdhost->card_tasklet);
		} else {
			present = (gpio_get_value(host->cd_gpio) == host->cd_gpio_polarity);

			if( present != host->card_present )
			{
				host->card_present = present;
				tasklet_schedule(&sdhci->card_tasklet);
			}
		}
	}
}
#endif

static void tegra_sdhci_status_notify_cb(int card_present, void *dev_id)
{
	struct sdhci_host *sdhci = (struct sdhci_host *)dev_id;
	pr_debug("%s: card_present %d\n",
		mmc_hostname(sdhci->mmc), card_present);
	sdhci_card_detect_callback(sdhci);
}

static int tegra_sdhci_enable_dma(struct sdhci_host *host)
{
	return 0;
}

static void tegra_sdhci_enable_clock(struct tegra_sdhci_host *host, int clock)
{
	u8 val;

#if defined CONFIG_MACH_BOSE_ATT
	if( spin_is_locked(&host->sdhci->lock)){
		spin_unlock_irqrestore(&host->sdhci->lock,host->sdhci->spinlock_flags);
		host->acquire_spinlock = 1;
	}
#endif
	
	if (clock) {
		if (!host->clk_enabled) {
			clk_enable(host->clk);
			val = sdhci_readb(host->sdhci, SDHCI_VENDOR_CLOCK_CNTRL);
			val |= 1;
			sdhci_writeb(host->sdhci, val, SDHCI_VENDOR_CLOCK_CNTRL);
			host->clk_enabled = 1;
		}
		if (clock < SDHCI_TEGRA_MIN_CONTROLLER_CLOCK)
			clk_set_rate(host->clk, SDHCI_TEGRA_MIN_CONTROLLER_CLOCK);
		else
			clk_set_rate(host->clk, clock);
	} else if (host->clk_enabled) {
		val = sdhci_readb(host->sdhci, SDHCI_VENDOR_CLOCK_CNTRL);
		val &= ~(0x1);
		sdhci_writeb(host->sdhci, val, SDHCI_VENDOR_CLOCK_CNTRL);

#if defined CONFIG_MACH_BOSE_ATT
/*
 *	Read back the register to ensure all writes on AHB are flushed prior 
 *	to switching OFF the clock
 */
 		val = sdhci_readb(host->sdhci, SDHCI_VENDOR_CLOCK_CNTRL);
#endif
		clk_disable(host->clk);
		host->clk_enabled = 0;
	}

#if defined CONFIG_MACH_BOSE_ATT
	if( host->acquire_spinlock){
		spin_lock_irqsave(&host->sdhci->lock, host->sdhci->spinlock_flags);
		host->acquire_spinlock = 0;
	}
#endif

	
	if (host->clk_enabled)
		host->sdhci->max_clk = clk_get_rate(host->clk);
	else
		host->sdhci->max_clk = 0;

}

static void tegra_sdhci_set_clock(struct sdhci_host *sdhci, unsigned int clock)
{
	struct tegra_sdhci_host *host = sdhci_priv(sdhci);
	pr_debug("tegra sdhci clock %s %u enabled=%d\n",
		mmc_hostname(sdhci->mmc), clock, host->clk_enabled);

	tegra_sdhci_enable_clock(host, clock);
}

#if defined CONFIG_MACH_BOSE_ATT
static int tegra_sdhci_card_detect(struct sdhci_host *sdhost)
{
	struct tegra_sdhci_host *host = sdhci_priv(sdhost);

	return host->card_present;
}
#endif

#ifdef CONFIG_MACH_N1
void tegra_sdhci_force_presence_change()
{
	struct tegra_sdhci_host *host = platform_get_drvdata (tegra_sdhci_device0_ptr);
	printk ("*************INto function %s*****************\n", __func__);

//	struct tegra_sdio_platform_data *pdata = tegra_wlan_pdevice->dev.platform_data;
	//printk("%s:tegra_wlan_pdevice->name %s  tegra_wlan_pdevice->id %d\n",__func__, tegra_sdhci_device0_ptr->name, tegra_sdhci_device0_ptr->id);
//	printk("host->mmc->bus_ops=%x\n",(unsigned int)host->mmc->bus_ops);
	//sdhci_card_detect_callback(sdhost);
	if (host->sdhci && host->sdhci->mmc){
		host->sdhci->mmc->pm_flags |= MMC_PM_KEEP_POWER;
		mmc_detect_change(host->sdhci->mmc,msecs_to_jiffies(0));
		msleep(200);
	}
	else
		printk ("Invalid pointers %x\n",host);
}
EXPORT_SYMBOL(tegra_sdhci_force_presence_change);
#endif

static int tegra_sdhci_get_ro(struct sdhci_host *sdhci)
{
	struct tegra_sdhci_host *host;
	host = sdhci_priv(sdhci);
	if (gpio_is_valid(host->wp_gpio))
		return gpio_get_value(host->wp_gpio);
	return 0;
}

static struct sdhci_ops tegra_sdhci_ops = {
	.enable_dma = tegra_sdhci_enable_dma,
	.set_clock = tegra_sdhci_set_clock,
	.get_ro = tegra_sdhci_get_ro,
#if defined CONFIG_MACH_BOSE_ATT
	.card_detect = tegra_sdhci_card_detect,
#endif
};

static int __devinit tegra_sdhci_probe(struct platform_device *pdev)
{
	int rc;
	struct tegra_sdhci_platform_data *plat;
	struct sdhci_host *sdhci;
	struct tegra_sdhci_host *host;
	struct resource *res;
	int irq;
	void __iomem *ioaddr;

	plat = pdev->dev.platform_data;
	if (plat == NULL)
		return -ENXIO;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL)
		return -ENODEV;

	irq = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL)
		return -ENODEV;

	ioaddr = ioremap(res->start, res->end - res->start);

	sdhci = sdhci_alloc_host(&pdev->dev, sizeof(struct tegra_sdhci_host));
	if (IS_ERR(sdhci)) {
		rc = PTR_ERR(sdhci);
		goto err_unmap;
	}

	host = sdhci_priv(sdhci);
	host->sdhci = sdhci;
	host->card_always_on = (plat->power_gpio == -1) ? 1 : 0;
#if defined CONFIG_MACH_BOSE_ATT
	host->cd_gpio = plat->cd_gpio;
	host->cd_gpio_polarity = plat->cd_gpio_polarity;
#endif
	host->wp_gpio = plat->wp_gpio;

#if defined CONFIG_MACH_BOSE_ATT
	host->acquire_spinlock = 0;
#endif

	host->clk = clk_get(&pdev->dev, plat->clk_id);
	if (IS_ERR(host->clk)) {
		rc = PTR_ERR(host->clk);
		goto err_free_host;
	}

	rc = clk_enable(host->clk);
	if (rc != 0)
		goto err_clkput;

	host->clk_enabled = 1;
	sdhci->hw_name = "tegra";
	sdhci->ops = &tegra_sdhci_ops;
	sdhci->irq = irq;
	sdhci->ioaddr = ioaddr;
	sdhci->version = SDHCI_SPEC_200;
	sdhci->quirks = SDHCI_QUIRK_BROKEN_TIMEOUT_VAL |
			SDHCI_QUIRK_SINGLE_POWER_WRITE |
			SDHCI_QUIRK_ENABLE_INTERRUPT_AT_BLOCK_GAP |
			SDHCI_QUIRK_BROKEN_WRITE_PROTECT |
			SDHCI_QUIRK_BROKEN_CTRL_HISPD |
			SDHCI_QUIRK_NO_HISPD_BIT |
			SDHCI_QUIRK_8_BIT_DATA |
			SDHCI_QUIRK_NO_VERSION_REG |
			SDHCI_QUIRK_BROKEN_ADMA_ZEROLEN_DESC |
#if defined CONFIG_MACH_BOSE_ATT
			SDHCI_QUIRK_RUNTIME_DISABLE|
			SDHCI_QUIRK_BROKEN_CARD_DETECTION;
#else
			SDHCI_QUIRK_RUNTIME_DISABLE;
#endif

	if (plat->force_hs != 0)
		sdhci->quirks |= SDHCI_QUIRK_FORCE_HIGH_SPEED_MODE;
#ifdef CONFIG_MMC_EMBEDDED_SDIO
	mmc_set_embedded_sdio_data(sdhci->mmc,
			&plat->cis,
			&plat->cccr,
			plat->funcs,
			plat->num_funcs);
#endif


	platform_set_drvdata(pdev, host);

#if defined CONFIG_MACH_BOSE_ATT
	/*
	  *	If the card detect gpio is not present, treat the card as non-removable
	  */
	  if( plat->cd_gpio == -1 )
	  	host->card_present = 1;

#endif

	if (plat->cd_gpio != -1) {
#ifdef CONFIG_MACH_BOSE_ATT
		mmc_host_sd_set_present(sdhci->mmc);
#endif

#if defined CONFIG_MACH_BOSE_ATT
		if (plat->cd_gpio ==0xffff) {
			card_sdhost = sdhci;		
			if (stmpe_reg_read(g_stmpe,0x17) & 0x80)
				host->card_present = 0;
			else
				host->card_present = 1;
		}
		else {
#endif
			rc = request_irq(gpio_to_irq(plat->cd_gpio), carddetect_irq,
				IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				mmc_hostname(sdhci->mmc), sdhci);

			if (rc)
				goto err_remove_host;
#if defined CONFIG_MACH_BOSE_ATT
			host->card_present = ( gpio_get_value(plat->cd_gpio) == host->cd_gpio_polarity);
		}	
#endif
	} else if (plat->register_status_notify) {
			plat->register_status_notify(
				tegra_sdhci_status_notify_cb, sdhci);
	}

	rc = sdhci_add_host(sdhci);
	if (rc)
		goto err_clk_disable;

	if (plat->board_probe)
		plat->board_probe(pdev->id, sdhci->mmc);

	printk(KERN_INFO "sdhci%d: initialized irq %d ioaddr %p\n", pdev->id,
			sdhci->irq, sdhci->ioaddr);

	return 0;

err_remove_host:
	sdhci_remove_host(sdhci, 1);
err_clk_disable:
	clk_disable(host->clk);
	host->clk_enabled = 0;
err_clkput:
	clk_put(host->clk);
err_free_host:
	if (sdhci)
		sdhci_free_host(sdhci);
err_unmap:
	iounmap(sdhci->ioaddr);

	return rc;
}

static int tegra_sdhci_remove(struct platform_device *pdev)
{
	struct tegra_sdhci_host *host = platform_get_drvdata(pdev);
	if (host) {
		struct tegra_sdhci_platform_data *plat;
		plat = pdev->dev.platform_data;
		if (plat && plat->board_probe)
			plat->board_probe(pdev->id, host->sdhci->mmc);

		sdhci_remove_host(host->sdhci, 0);
		sdhci_free_host(host->sdhci);
	}
	return 0;
}


#define is_card_sdio(_card) \
((_card) && ((_card)->type == MMC_TYPE_SDIO))

#ifdef CONFIG_PM


static void tegra_sdhci_restore_interrupts(struct sdhci_host *sdhost)
{
	u32 ierr;
	u32 clear = SDHCI_INT_ALL_MASK;
	struct tegra_sdhci_host *host = sdhci_priv(sdhost);

	/* enable required interrupts */
	ierr = sdhci_readl(sdhost, SDHCI_INT_ENABLE);
	ierr &= ~clear;
	ierr |= host->sdhci_ints;
	sdhci_writel(sdhost, ierr, SDHCI_INT_ENABLE);
	sdhci_writel(sdhost, ierr, SDHCI_SIGNAL_ENABLE);

	if ((host->sdhci_ints & SDHCI_INT_CARD_INT) &&
		(sdhost->quirks & SDHCI_QUIRK_ENABLE_INTERRUPT_AT_BLOCK_GAP)) {
		u8 gap_ctrl = sdhci_readb(sdhost, SDHCI_BLOCK_GAP_CONTROL);
		gap_ctrl |= 0x8;
		sdhci_writeb(sdhost, gap_ctrl, SDHCI_BLOCK_GAP_CONTROL);
	}
}

static int tegra_sdhci_restore(struct sdhci_host *sdhost)
{
	unsigned long timeout;
	u8 mask = SDHCI_RESET_ALL;
	u8 pwr;

	sdhci_writeb(sdhost, mask, SDHCI_SOFTWARE_RESET);

	sdhost->clock = 0;

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (sdhci_readb(sdhost, SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			printk(KERN_ERR "%s: Reset 0x%x never completed.\n",
				mmc_hostname(sdhost->mmc), (int)mask);
			return -EIO;
		}
		timeout--;
		mdelay(1);
	}

	tegra_sdhci_restore_interrupts(sdhost);

	pwr = SDHCI_POWER_ON;
	sdhci_writeb(sdhost, pwr, SDHCI_POWER_CONTROL);
	sdhost->pwr = 0;

	return 0;
}

static int tegra_sdhci_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct tegra_sdhci_host *host = platform_get_drvdata(pdev);
	int ret = 0;

	if (host->card_always_on && is_card_sdio(host->sdhci->mmc->card)) {
		int div = 0;
		u16 clk;
		unsigned int clock = 100000;

		if (device_may_wakeup(&pdev->dev)) {
		        enable_irq_wake(host->sdhci->irq);
		}

		/* save interrupt status before suspending */
		host->sdhci_ints = sdhci_readl(host->sdhci, SDHCI_INT_ENABLE);

		/* reduce host controller clk and card clk to 100 KHz */
		tegra_sdhci_set_clock(host->sdhci, clock);
		sdhci_writew(host->sdhci, 0, SDHCI_CLOCK_CONTROL);

		if (host->sdhci->max_clk > clock) {
			div =  1 << (fls(host->sdhci->max_clk / clock) - 2);
			if (div > 128)
				div = 128;
		}

		clk = div << SDHCI_DIVIDER_SHIFT;
		clk |= SDHCI_CLOCK_INT_EN | SDHCI_CLOCK_CARD_EN;
		sdhci_writew(host->sdhci, clk, SDHCI_CLOCK_CONTROL);

		return ret;
	}


	ret = sdhci_suspend_host(host->sdhci, state);
	if (ret)
		pr_err("%s: failed, error = %d\n", __func__, ret);

	tegra_sdhci_enable_clock(host, 0);
	return ret;
}

static int tegra_sdhci_resume(struct platform_device *pdev)
{
	struct tegra_sdhci_host *host = platform_get_drvdata(pdev);
	int ret;
#ifdef CONFIG_MACH_N1	
	int i, present;
#endif

	if (host->card_always_on && is_card_sdio(host->sdhci->mmc->card)) {
		int ret = 0;

		if (device_may_wakeup(&pdev->dev)) {
		        disable_irq_wake(host->sdhci->irq);
		}

		/* soft reset SD host controller and enable interrupts */
		ret = tegra_sdhci_restore(host->sdhci);
		if (ret) {
			pr_err("%s: failed, error = %d\n", __func__, ret);
			return ret;
		}

		mmiowb();
#ifdef CONFIG_MACH_N1
		for(i=0;i<20;i++){
			present = sdhci_readl(host->sdhci, SDHCI_PRESENT_STATE);
			if((present & SDHCI_CARD_PRESENT) == SDHCI_CARD_PRESENT)
				break;
			mdelay(5);
//			printk(KERN_ERR "MMC : %s : 6(Card Presnet %x) : %d \n",mmc_hostname(host->sdhci->mmc),present,i);
		}
#endif
		host->sdhci->mmc->ops->set_ios(host->sdhci->mmc,
			&host->sdhci->mmc->ios);
		return 0;
	}


	tegra_sdhci_enable_clock(host, SDHCI_TEGRA_MIN_CONTROLLER_CLOCK);
	mdelay(10);
	ret = sdhci_resume_host(host->sdhci);
	if (ret)
		pr_err("%s: failed, error = %d\n", __func__, ret);

#if defined CONFIG_MACH_BOSE_ATT
	tegra_sdhci_update_card_detection(host->sdhci);
#endif
	return ret;
}
#else
#define tegra_sdhci_suspend    NULL
#define tegra_sdhci_resume     NULL
#endif

static struct platform_driver tegra_sdhci_driver = {
	.probe = tegra_sdhci_probe,
	.remove = tegra_sdhci_remove,
	.suspend = tegra_sdhci_suspend,
	.resume = tegra_sdhci_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init tegra_sdhci_init(void)
{
	return platform_driver_register(&tegra_sdhci_driver);
}

static void __exit tegra_sdhci_exit(void)
{
	platform_driver_unregister(&tegra_sdhci_driver);
}

module_init(tegra_sdhci_init);
module_exit(tegra_sdhci_exit);

MODULE_DESCRIPTION("Tegra SDHCI controller driver");
MODULE_LICENSE("GPL");
