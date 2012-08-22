/*
 * drivers/usb/otg/tegra-otg.c
 *
 * OTG transceiver driver for Tegra UTMI phy
 *
 * Copyright (C) 2010 NVIDIA Corp.
 * Copyright (C) 2010 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
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

#include <linux/usb.h>
#include <linux/usb/otg.h>
#include <linux/usb/gadget.h>
#include <linux/usb/hcd.h>
#include <linux/platform_device.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/err.h>
#ifdef CONFIG_MACH_N1
#ifdef CONFIG_USB_HOST_NOTIFY
#include <linux/host_notify.h>
#endif
#include <linux/gpio.h>
#include <linux/wakelock.h>
#endif

#define USB_PHY_WAKEUP		0x408
#define  USB_ID_INT_EN		(1 << 0)
#define  USB_ID_INT_STATUS	(1 << 1)
#define  USB_ID_STATUS		(1 << 2)
#define  USB_ID_PIN_WAKEUP_EN	(1 << 6)
#define  USB_VBUS_WAKEUP_EN	(1 << 30)
#define  USB_VBUS_INT_EN	(1 << 8)
#define  USB_VBUS_INT_STATUS	(1 << 9)
#define  USB_VBUS_STATUS	(1 << 10)
#define  USB_INTS		(USB_VBUS_INT_STATUS | USB_ID_INT_STATUS)

#ifdef CONFIG_MACH_N1
#define TEGRA_HOST_CAUSE	(1 << 0)
#define TEGRA_VBUS_CAUSE	(1 << 1)
#define TEGRA_ACCPWROFF_CAUSE	(1 << 2)
#define TEGRA_RESUME_CAUSE	(1 << 3)
#endif

typedef void (*callback_t)(enum usb_otg_state to,
				enum usb_otg_state from, void *args);

#ifdef DEBUG
#define DBG(stuff...)	pr_info("tegra-otg: " stuff)
#else
#define DBG(stuff...)	do {} while (0)
#endif

struct tegra_otg_data {
	struct otg_transceiver otg;
	unsigned long int_status;
	spinlock_t lock;
	void __iomem *regs;
	struct clk *clk;
	int irq;
	struct platform_device *pdev;
	struct work_struct work;
	unsigned int intr_reg_data;
#ifdef CONFIG_MACH_N1
#ifdef CONFIG_USB_HOST_NOTIFY
	struct host_notify_dev ndev;
#endif
	int currentlimit_irq;
	u32 host_state;
	struct wake_lock wake_lock;
	void (*usb_ldo_en)(int, int);
	struct mutex clk_lock;
	int clone_clk_enabled[CAUSE_NUM];
	unsigned int clk_cause;
#endif
	callback_t	charger_cb;
	void	*charger_cb_data;
	bool interrupt_mode;
};

static inline unsigned long otg_readl(struct tegra_otg_data *tegra,
				      unsigned int offset)
{
	return readl(tegra->regs + offset);
}

static inline void otg_writel(struct tegra_otg_data *tegra, unsigned long val,
			      unsigned int offset)
{
	writel(val, tegra->regs + offset);
}

#ifdef CONFIG_MACH_N1
static void tegra_otg_enable_clk(struct tegra_otg_data *tegra, int cause)
{
	mutex_lock(&tegra->clk_lock);
	dev_info(tegra->otg.dev, "%s: clk cause=%d present enabled=%d\n",
		__func__, cause, tegra->clone_clk_enabled[cause]);

	switch (cause) {
	case HOST_CAUSE:
		tegra->clk_cause |= TEGRA_HOST_CAUSE;
		break;
	case VBUS_CAUSE:
		tegra->clk_cause |= TEGRA_VBUS_CAUSE;
		break;
	case RESUME_CAUSE:
		tegra->clk_cause |= TEGRA_RESUME_CAUSE;
		break;
	default:
		break;
	}
	if (!tegra->clone_clk_enabled[cause])
		clk_enable(tegra->clk);
	tegra->clone_clk_enabled[cause] = true;
	mutex_unlock(&tegra->clk_lock);
}

static void tegra_otg_disable_clk(struct tegra_otg_data *tegra, int cause)
{
	int i = 0;
	int index = 0;
	mutex_lock(&tegra->clk_lock);
	index = cause < FORCE_ALL ?
		tegra->clone_clk_enabled[cause] : FORCE_ALL;
	if (index)
		dev_info(tegra->otg.dev, "%s: clk cause=%d cause all=%d\n",
			__func__, cause, tegra->clk_cause);

	if (cause == FORCE_ALL) {
		for (i = 0; i < CAUSE_NUM; i++) {
			if (tegra->clone_clk_enabled[i])
				clk_disable(tegra->clk);
			tegra->clone_clk_enabled[i] = false;
		}
	} else {
		if (tegra->clone_clk_enabled[cause])
			clk_disable(tegra->clk);
		tegra->clone_clk_enabled[cause] = false;
	}
	mutex_unlock(&tegra->clk_lock);
}
#endif

static const char *tegra_state_name(enum usb_otg_state state)
{
	switch (state) {
		case OTG_STATE_A_HOST:
		return "HOST";
		case OTG_STATE_B_PERIPHERAL:
		return "PERIPHERAL";
		case OTG_STATE_A_SUSPEND:
		return "SUSPEND";
		case OTG_STATE_UNDEFINED:
			return "UNDEFINED";
		default:
			break;
	}
	return "INVALID";
}

#ifdef CONFIG_MACH_N1
static int tegra_check_register(unsigned long val)
{
	if ((val & USB_VBUS_INT_EN) && (val & USB_ID_INT_EN)
		&& (val & USB_VBUS_WAKEUP_EN)
		&& (val & USB_ID_PIN_WAKEUP_EN))
		return 1;
	else
		return 0;
}

static void tegra_recover_register_value(unsigned long *val)
{
	unsigned long reg_val = *val;

	if (!tegra_check_register(reg_val)) {
		reg_val |= (USB_VBUS_INT_EN | USB_VBUS_WAKEUP_EN
		| USB_ID_INT_EN | USB_ID_PIN_WAKEUP_EN);
		*val = reg_val;
	}
}

static void tegra_recover_register(struct tegra_otg_data *tegra)
{
	unsigned long reg_val = 0;
	unsigned long flags = 0;
	int try_times = 5;

	while (try_times--) {
		reg_val = otg_readl(tegra, USB_PHY_WAKEUP);
		if (tegra_check_register(reg_val))
			break;
		msleep(20);
	}

	if (try_times < 0) {
		spin_lock_irqsave(&tegra->lock, flags);
		reg_val = otg_readl(tegra, USB_PHY_WAKEUP);
		tegra_recover_register_value(&reg_val);
		otg_writel(tegra, reg_val, USB_PHY_WAKEUP);
		spin_unlock_irqrestore(&tegra->lock, flags);
		msleep(100);
		reg_val = otg_readl(tegra, USB_PHY_WAKEUP);
	}
	dev_info(tegra->otg.dev, "%s reg_val=%lu\n", __func__, reg_val);
	return;
}
#endif

static unsigned long enable_interrupt(struct tegra_otg_data *tegra, bool en)
{
	unsigned long val;

	clk_enable(tegra->clk);
	val = otg_readl(tegra, USB_PHY_WAKEUP);
	if (en) {
		val |= (USB_VBUS_INT_EN | USB_VBUS_WAKEUP_EN);
		val |= (USB_ID_INT_EN | USB_ID_PIN_WAKEUP_EN);
	} else {
		val &= ~(USB_VBUS_INT_EN | USB_VBUS_WAKEUP_EN);
		val &= ~(USB_ID_INT_EN | USB_ID_PIN_WAKEUP_EN);
	}
	otg_writel(tegra, val, USB_PHY_WAKEUP);
#ifdef CONFIG_MACH_N1
	tegra_recover_register(tegra);
#endif
	/* Add delay to make sure register is updated */
	udelay(1);
	clk_disable(tegra->clk);

	return val;
}

static struct platform_device *
tegra_usb_otg_host_register(struct platform_device *ehci_device,
			    struct tegra_ehci_platform_data *pdata)
{
	struct platform_device *pdev;
	void *platform_data;
	int val;

	pdev = platform_device_alloc(ehci_device->name, ehci_device->id);
	if (!pdev)
		return NULL;

	val = platform_device_add_resources(pdev, ehci_device->resource,
					    ehci_device->num_resources);
	if (val)
		goto error;

	pdev->dev.dma_mask =  ehci_device->dev.dma_mask;
	pdev->dev.coherent_dma_mask = ehci_device->dev.coherent_dma_mask;

	platform_data = kmalloc(sizeof(struct tegra_ehci_platform_data),
		GFP_KERNEL);
	if (!platform_data)
		goto error;

	memcpy(platform_data, pdata, sizeof(struct tegra_ehci_platform_data));
	pdev->dev.platform_data = platform_data;

	val = platform_device_add(pdev);
	if (val)
		goto error_add;

	return pdev;

error_add:
	kfree(platform_data);
error:
	pr_err("%s: failed to add the host controller device\n", __func__);
	platform_device_put(pdev);
	return NULL;
}

static void tegra_usb_otg_host_unregister(struct platform_device *pdev)
{
	platform_device_unregister(pdev);
}

void tegra_start_host(struct tegra_otg_data *tegra)
{
	struct tegra_otg_platform_data *pdata = tegra->otg.dev->platform_data;
	dev_info(tegra->otg.dev, "tegra_start_host+\n");
#ifdef CONFIG_MACH_N1
	wake_lock(&tegra->wake_lock);
#endif
	if (!tegra->pdev) {
		tegra->pdev = tegra_usb_otg_host_register(pdata->ehci_device,
							  pdata->ehci_pdata);
	}
	dev_info(tegra->otg.dev, "tegra_start_host-\n");
}

void tegra_stop_host(struct tegra_otg_data *tegra)
{
	struct tegra_otg_platform_data *pdata = tegra->otg.dev->platform_data;
	dev_info(tegra->otg.dev, "tegra_stop_host+\n");
	if (tegra->pdev) {
		tegra_usb_otg_host_unregister(tegra->pdev);
		tegra->pdev = NULL;
	}
#ifdef CONFIG_MACH_N1
	if (pdata->otg_id_open->id_open)
		pdata->otg_id_open->id_open
			(pdata->otg_id_open->otg_open_data);
	wake_unlock(&tegra->wake_lock);
#endif
	dev_info(tegra->otg.dev, "tegra_stop_host-\n");
}

int register_otg_callback(callback_t cb, void *args)
{
/*
	if (!tegra_clone)
		return -ENODEV;
	tegra_clone->charger_cb = cb;
	tegra_clone->charger_cb_data = args;
*/	
	return 0;
}
EXPORT_SYMBOL_GPL(register_otg_callback);

static void tegra_change_otg_state(struct tegra_otg_data *tegra,
				enum usb_otg_state to)
{
	struct otg_transceiver *otg = &tegra->otg;
	enum usb_otg_state from = otg->state;

	if(!tegra->interrupt_mode){
		DBG("OTG: Vbus detection is disabled");
		return;
	}

	DBG("%s(%d) requested otg state %s-->%s\n", __func__,
		__LINE__, tegra_state_name(from), tegra_state_name(to));

	if (to != OTG_STATE_UNDEFINED && from != to) {
		otg->state = to;
		dev_info(tegra->otg.dev, "%s --> %s\n", tegra_state_name(from),
					      tegra_state_name(to));

		if (tegra->charger_cb)
			tegra->charger_cb(to, from, tegra->charger_cb_data);

		if (from == OTG_STATE_A_SUSPEND) {
			if (to == OTG_STATE_B_PERIPHERAL && otg->gadget)
				usb_gadget_vbus_connect(otg->gadget);
			else if (to == OTG_STATE_A_HOST)
				tegra_start_host(tegra);
		} else if (from == OTG_STATE_A_HOST) {
			if (to == OTG_STATE_A_SUSPEND)
				tegra_stop_host(tegra);
		} else if (from == OTG_STATE_B_PERIPHERAL && otg->gadget) {
			if (to == OTG_STATE_A_SUSPEND)
				usb_gadget_vbus_disconnect(otg->gadget);
		}
	}
}

static void irq_work(struct work_struct *work)
{
	struct tegra_otg_data *tegra =
		container_of(work, struct tegra_otg_data, work);
#ifdef CONFIG_MACH_N1
	struct tegra_otg_platform_data *pdata = tegra->otg.dev->platform_data;
#endif
	struct otg_transceiver *otg = &tegra->otg;
	enum usb_otg_state from = otg->state;
	enum usb_otg_state to = OTG_STATE_UNDEFINED;
	unsigned long flags;
	unsigned long status;

	clk_enable(tegra->clk);

	spin_lock_irqsave(&tegra->lock, flags);

	status = tegra->int_status;

	/* Debug prints */
	DBG("%s(%d) status = 0x%x\n", __func__, __LINE__, status);
	if ((status & USB_ID_INT_STATUS) &&
			(status & USB_VBUS_INT_STATUS))
		DBG("%s(%d) got vbus & id interrupt\n", __func__, __LINE__);
	else {
		if (status & USB_ID_INT_STATUS)
			DBG("%s(%d) got id interrupt\n", __func__, __LINE__);
		if (status & USB_VBUS_INT_STATUS)
			DBG("%s(%d) got vbus interrupt\n", __func__, __LINE__);
	}

	if (tegra->int_status & USB_ID_INT_STATUS) {
		if (status & USB_ID_STATUS) {
			if ((status & USB_VBUS_STATUS) && (from != OTG_STATE_A_HOST))
				to = OTG_STATE_B_PERIPHERAL;
			else
				to = OTG_STATE_A_SUSPEND;

#ifdef CONFIG_MACH_N1
			tegra->host_state = TEGRA_USB_POWEROFF;
#endif
		} else {
			to = OTG_STATE_A_HOST;
#ifdef CONFIG_MACH_N1
			tegra->int_status &= ~USB_VBUS_INT_STATUS;
#endif
		}
	}
	if (from != OTG_STATE_A_HOST) {
		if (tegra->int_status & USB_VBUS_INT_STATUS) {
			if (status & USB_VBUS_STATUS)
				to = OTG_STATE_B_PERIPHERAL;
			else
				to = OTG_STATE_A_SUSPEND;
		}
	}
#ifdef CONFIG_MACH_N1
	else {
		if (tegra->int_status & USB_VBUS_INT_STATUS) {
			if (status & USB_VBUS_STATUS)
				tegra->host_state = TEGRA_USB_POWERON;
			else
				tegra->host_state = TEGRA_USB_OVERCURRENT;
		}
	}
#endif
	spin_unlock_irqrestore(&tegra->lock, flags);

#ifdef CONFIG_MACH_N1
	if (tegra->host_state == TEGRA_USB_POWERON) {
#ifdef CONFIG_USB_HOST_NOTIFY
		tegra->ndev.booster = NOTIFY_POWER_ON;
#endif
		dev_info(tegra->otg.dev, "OTG power on detect\n");
	} else if (tegra->host_state == TEGRA_USB_OVERCURRENT) {
#ifdef CONFIG_USB_HOST_NOTIFY
		tegra->ndev.booster = NOTIFY_POWER_OFF;
		host_state_notify(&tegra->ndev,
			NOTIFY_HOST_OVERCURRENT, false);
#endif
		dev_info(tegra->otg.dev, "OTG overcurrent!!!!!!\n");
	}
#ifdef CONFIG_USB_HOST_NOTIFY
	else
		tegra->ndev.booster = NOTIFY_POWER_OFF;
#endif
#endif

	if (to != OTG_STATE_UNDEFINED) {
		otg->state = to;

		dev_info(tegra->otg.dev, "%s --> %s\n", tegra_state_name(from),
					      tegra_state_name(to));

		if (tegra->charger_cb)
			tegra->charger_cb(to, from, tegra->charger_cb_data);

		if (to == OTG_STATE_A_SUSPEND) {
			if (from == OTG_STATE_A_HOST) {
				tegra_stop_host(tegra);
#ifdef CONFIG_MACH_N1
#ifdef CONFIG_USB_HOST_NOTIFY
				host_state_notify(&tegra->ndev,
					 NOTIFY_HOST_REMOVE, false);
#endif
#endif
			} else if (from == OTG_STATE_B_PERIPHERAL
							&& otg->gadget)
				usb_gadget_vbus_disconnect(otg->gadget);
#ifdef CONFIG_MACH_N1
#ifdef CONFIG_USB_HOST_NOTIFY
			tegra->ndev.mode = NOTIFY_NONE_MODE;
#endif
#endif
		} else if (to == OTG_STATE_B_PERIPHERAL && otg->gadget) {
			if (from != OTG_STATE_B_PERIPHERAL)
				usb_gadget_vbus_connect(otg->gadget);
#ifdef CONFIG_MACH_N1
#ifdef CONFIG_USB_HOST_NOTIFY
			if (tegra->ndev.mode == NOTIFY_TEST_MODE) {
				if (pdata->otg_id_open->otg_cb)
					pdata->otg_id_open->otg_cb(1);
			} else
				tegra->ndev.mode = NOTIFY_PERIPHERAL_MODE;
			tegra->ndev.booster = NOTIFY_POWER_ON;
#endif
#endif
		} else if (to == OTG_STATE_A_HOST) {
			if (from == OTG_STATE_A_SUSPEND)
			tegra_start_host(tegra);
#ifdef CONFIG_MACH_N1
			else if (from == OTG_STATE_B_PERIPHERAL) {
				usb_gadget_vbus_disconnect(otg->gadget);
				tegra_start_host(tegra);
			}
#ifdef CONFIG_USB_HOST_NOTIFY
			tegra->ndev.mode = NOTIFY_HOST_MODE;
			host_state_notify(&tegra->ndev,
				NOTIFY_HOST_ADD, false);
#endif
#endif
		}
	}
	clk_disable(tegra->clk);

#ifdef CONFIG_MACH_N1
	if (tegra->clk_cause & TEGRA_HOST_CAUSE) {
		if (to != OTG_STATE_A_HOST &&
			to != OTG_STATE_UNDEFINED &&
				from == OTG_STATE_A_HOST) {
			tegra->clk_cause &=
				~TEGRA_HOST_CAUSE;
			tegra_otg_disable_clk(tegra, HOST_CAUSE);
		}
	} else if (tegra->clk_cause & TEGRA_VBUS_CAUSE) {
		if ((to != OTG_STATE_B_PERIPHERAL &&
				from == OTG_STATE_B_PERIPHERAL) ||
			(to != OTG_STATE_B_PERIPHERAL &&
				tegra->host_state == TEGRA_USB_POWEROFF)) {
			tegra->clk_cause &=
				~TEGRA_VBUS_CAUSE;
			tegra_otg_disable_clk(tegra, VBUS_CAUSE);
		}
	}

	tegra->clk_cause &=
		~TEGRA_RESUME_CAUSE;
	tegra_otg_disable_clk(tegra, RESUME_CAUSE);
#endif
}

static irqreturn_t tegra_otg_irq(int irq, void *data)
{
	struct tegra_otg_data *tegra = data;
	unsigned long flags;
	unsigned long val;

	spin_lock_irqsave(&tegra->lock, flags);

	val = otg_readl(tegra, USB_PHY_WAKEUP);
	if (val & (USB_VBUS_INT_EN | USB_ID_INT_EN)) {
		otg_writel(tegra, val, USB_PHY_WAKEUP);
		if ((val & USB_ID_INT_STATUS) || (val & USB_VBUS_INT_STATUS)) {
			tegra->int_status = val;
			schedule_work(&tegra->work);
		}
	}

	spin_unlock_irqrestore(&tegra->lock, flags);

	return IRQ_HANDLED;
}

#ifdef CONFIG_MACH_N1
static void tegra_otg_check_detection(int *clk_cause, int cause)
{
	struct tegra_otg_data *tegra =
		container_of(clk_cause, struct tegra_otg_data, clk_cause);
	tegra_otg_enable_clk(tegra, cause);
}
#endif

static int tegra_otg_set_peripheral(struct otg_transceiver *otg,
				struct usb_gadget *gadget)
{
	struct tegra_otg_data *tegra;
	unsigned long val;

	tegra = container_of(otg, struct tegra_otg_data, otg);
	otg->gadget = gadget;

	val = enable_interrupt(tegra, true);

	if ((val & USB_ID_STATUS) && (val & USB_VBUS_STATUS)) {
		val |= USB_VBUS_INT_STATUS;
	} else if (!(val & USB_ID_STATUS)) {
		val |= USB_ID_INT_STATUS;
#ifdef CONFIG_MACH_N1
		val &= ~USB_VBUS_INT_STATUS;
#endif
	} else {
		val &= ~(USB_ID_INT_STATUS | USB_VBUS_INT_STATUS);
	}
#ifndef CONFIG_MACH_N1
	if ((val & USB_ID_INT_STATUS) || (val & USB_VBUS_INT_STATUS)) {
		tegra->int_status = val;
		schedule_work (&tegra->work);
	}
#else
	tegra->int_status = val;
	schedule_work(&tegra->work);
#endif

	return 0;
}

static int tegra_otg_set_host(struct otg_transceiver *otg,
				struct usb_bus *host)
{
	struct tegra_otg_data *tegra;
	unsigned long val;

	tegra = container_of(otg, struct tegra_otg_data, otg);
	otg->host = host;

	clk_enable(tegra->clk);
	val = otg_readl(tegra, USB_PHY_WAKEUP);
	val &= ~(USB_VBUS_INT_STATUS | USB_ID_INT_STATUS);

	val |= (USB_ID_INT_EN | USB_ID_PIN_WAKEUP_EN);
	otg_writel(tegra, val, USB_PHY_WAKEUP);
#ifdef CONFIG_MACH_N1
	tegra_recover_register(tegra);
#endif
	clk_disable(tegra->clk);

	return 0;
}

static int tegra_otg_set_power(struct otg_transceiver *otg, unsigned mA)
{
	return 0;
}

static int tegra_otg_set_suspend(struct otg_transceiver *otg, int suspend)
{
	return 0;
}

#ifdef CONFIG_MACH_N1
static int tegra_get_accpower_level(int irq)
{
	return __gpio_get_value((unsigned)irq_to_gpio(irq));
}

static irqreturn_t tegra_currentlimit_irq_thread(int irq, void *data)
{
	struct tegra_otg_data *tegra = data;

	if (tegra_get_accpower_level(tegra->currentlimit_irq)) {
#ifdef CONFIG_USB_HOST_NOTIFY
		tegra->ndev.booster = NOTIFY_POWER_ON;
#endif
		dev_info(tegra->otg.dev, "Acc power on detect\n");
	} else {
#ifdef CONFIG_USB_HOST_NOTIFY
		if (tegra->ndev.mode == NOTIFY_HOST_MODE) {
			host_state_notify(&tegra->ndev,
				NOTIFY_HOST_OVERCURRENT, false);
			dev_err(tegra->otg.dev, "OTG overcurrent!!!!!!\n");
		}
		tegra->ndev.booster = NOTIFY_POWER_OFF;
#endif
	}
	return IRQ_HANDLED;
}
#endif

static ssize_t show_host_en(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct tegra_otg_data *tegra = platform_get_drvdata(pdev);

	*buf = tegra->interrupt_mode ? '0': '1';
	strcat(buf, "\n");
	return strlen(buf);
}

static ssize_t store_host_en(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct tegra_otg_data *tegra = platform_get_drvdata(pdev);
	unsigned long host;
	int err;

	err = kstrtoul(buf, 10, &host);
	if (err < 0) {
		return err;
	}

	if (host) {
		enable_interrupt(tegra, false);
		tegra_change_otg_state(tegra, OTG_STATE_A_SUSPEND);
		tegra_change_otg_state(tegra, OTG_STATE_A_HOST);
		tegra->interrupt_mode = false;
	} else {
		tegra->interrupt_mode = true;
		tegra_change_otg_state(tegra, OTG_STATE_A_SUSPEND);
		enable_interrupt(tegra, true);
	}

	return count;
}

static DEVICE_ATTR(enable_host, 0644, show_host_en, store_host_en);

static int tegra_otg_probe(struct platform_device *pdev)
{
	struct tegra_otg_data *tegra;
	struct tegra_otg_platform_data *otg_pdata;
	struct tegra_ehci_platform_data *ehci_pdata;
	struct resource *res;
	int err;

	tegra = kzalloc(sizeof(struct tegra_otg_data), GFP_KERNEL);
	if (!tegra)
		return -ENOMEM;

	tegra->otg.dev = &pdev->dev;
	otg_pdata = tegra->otg.dev->platform_data;
	ehci_pdata = otg_pdata->ehci_pdata;
	tegra->otg.label = "tegra-otg";
	tegra->otg.state = OTG_STATE_UNDEFINED;
	tegra->otg.set_host = tegra_otg_set_host;
	tegra->otg.set_peripheral = tegra_otg_set_peripheral;
	tegra->otg.set_suspend = tegra_otg_set_suspend;
	tegra->otg.set_power = tegra_otg_set_power;
	spin_lock_init(&tegra->lock);

	platform_set_drvdata(pdev, tegra);
	tegra->interrupt_mode = true;

	tegra->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(tegra->clk)) {
		dev_err(&pdev->dev, "Can't get otg clock\n");
		err = PTR_ERR(tegra->clk);
		goto err_clk;
	}

	err = clk_enable(tegra->clk);
	if (err)
		goto err_clken;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get I/O memory\n");
		err = -ENXIO;
		goto err_io;
	}
	tegra->regs = ioremap(res->start, resource_size(res));
	if (!tegra->regs) {
		err = -ENOMEM;
		goto err_io;
	}

	tegra->otg.state = OTG_STATE_A_SUSPEND;

	err = otg_set_transceiver(&tegra->otg);
	if (err) {
		dev_err(&pdev->dev, "can't register transceiver (%d)\n", err);
		goto err_otg;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get IRQ\n");
		err = -ENXIO;
		goto err_irq;
	}
	tegra->irq = res->start;
	err = request_threaded_irq(tegra->irq, tegra_otg_irq,
				   NULL,
				   IRQF_SHARED, "tegra-otg", tegra);
	if (err) {
		dev_err(&pdev->dev, "Failed to register IRQ\n");
		goto err_irq;
	}
	INIT_WORK (&tegra->work, irq_work);

#ifdef CONFIG_MACH_N1
	if (otg_pdata->usb_ldo_en)
		tegra->usb_ldo_en = otg_pdata->usb_ldo_en;
	if (otg_pdata->currentlimit_irq) {
		tegra->currentlimit_irq =
			otg_pdata->currentlimit_irq;
		err = request_threaded_irq(tegra->currentlimit_irq,
					NULL,
					tegra_currentlimit_irq_thread,
					(IRQF_TRIGGER_FALLING |
						IRQF_TRIGGER_RISING),
					dev_name(&pdev->dev),
					tegra);
		if (err) {
			dev_err(&pdev->dev, "Failed to register IRQ\n");
			goto err_irq2;
		}
	}
#ifdef CONFIG_USB_HOST_NOTIFY
#define NOTIFY_DRIVER_NAME "usb_otg"
	tegra->ndev.name = NOTIFY_DRIVER_NAME;
	if (otg_pdata->acc_power)
		tegra->ndev.set_booster = otg_pdata->acc_power;
	err = host_notify_dev_register(&tegra->ndev);
	if (err) {
		dev_err(&pdev->dev, "Failed to host_notify_dev_register\n");
		goto err_devreg;
	}
#endif
	wake_lock_init(&tegra->wake_lock, WAKE_LOCK_SUSPEND, "tegra-otg");
	mutex_init(&tegra->clk_lock);
	if (otg_pdata->set_clk_func)
		otg_pdata->set_clk_func(tegra_otg_check_detection,
					&tegra->clk_cause);
	if (otg_pdata->otg_id_open) {
		otg_pdata->otg_id_open->otg_state = (int *)&tegra->otg.state;
		otg_pdata->otg_id_open->host_state = (int *)&tegra->host_state;
	}
	tegra->clone_clk_enabled[0] = false;
	tegra->clone_clk_enabled[1] = false;
	tegra_otg_enable_clk(tegra, RESUME_CAUSE);
#endif

	if (!ehci_pdata->default_enable)
		clk_disable(tegra->clk);
	dev_info(&pdev->dev, "otg transceiver registered\n");

	err = device_create_file(&pdev->dev, &dev_attr_enable_host);
	if (err) {
		dev_warn(&pdev->dev, "Can't register sysfs attribute\n");
		goto err_irq;
	}

	return 0;

#ifdef CONFIG_MACH_N1
#ifdef CONFIG_USB_HOST_NOTIFY
err_devreg:
	if (tegra->currentlimit_irq)
		free_irq(tegra->currentlimit_irq, tegra);
#endif
err_irq2:
	free_irq(tegra->irq, tegra);
#endif
err_irq:
	otg_set_transceiver(NULL);
err_otg:
	iounmap(tegra->regs);
err_io:
	clk_disable(tegra->clk);
err_clken:
	clk_put(tegra->clk);
err_clk:
	platform_set_drvdata(pdev, NULL);
	kfree(tegra);
	return err;
}

static int __exit tegra_otg_remove(struct platform_device *pdev)
{
	struct tegra_otg_data *tegra = platform_get_drvdata(pdev);

#ifdef CONFIG_MACH_N1
#ifdef CONFIG_USB_HOST_NOTIFY
	host_notify_dev_unregister(&tegra->ndev);
#endif
	if (tegra->currentlimit_irq)
		free_irq(tegra->currentlimit_irq, tegra);
#endif
	free_irq(tegra->irq, tegra);
	otg_set_transceiver(NULL);
	iounmap(tegra->regs);
	clk_disable(tegra->clk);
	clk_put(tegra->clk);
	platform_set_drvdata(pdev, NULL);
	kfree(tegra);

	return 0;
}

#ifdef CONFIG_PM
static int tegra_otg_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct tegra_otg_data *tegra_otg = platform_get_drvdata(pdev);
	unsigned int val = 0;
#ifdef CONFIG_MACH_N1
	unsigned long flags = 0;
#endif
	/* store the interupt enable for cable ID and VBUS */
	clk_enable(tegra_otg->clk);
#ifdef CONFIG_MACH_N1
	spin_lock_irqsave(&tegra_otg->lock, flags);
#endif
	tegra_otg->intr_reg_data = readl(tegra_otg->regs + USB_PHY_WAKEUP);
#ifdef CONFIG_MACH_N1
	tegra_recover_register_value
		((unsigned long *)&tegra_otg->intr_reg_data);
#endif
	val = tegra_otg->intr_reg_data & ~(USB_ID_INT_EN | USB_VBUS_INT_EN);
	writel(val, (tegra_otg->regs + USB_PHY_WAKEUP));
#ifdef CONFIG_MACH_N1
	spin_unlock_irqrestore(&tegra_otg->lock, flags);
	msleep(20);
#endif
	clk_disable(tegra_otg->clk);

	tegra_otg->clk_cause = 0;
	tegra_otg_disable_clk(tegra_otg, FORCE_ALL);
	return 0;
}

static void tegra_otg_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct tegra_otg_data *tegra_otg = platform_get_drvdata(pdev);

	tegra_otg_enable_clk(tegra_otg, RESUME_CAUSE);

	/* Following delay is intentional.
	 * It is placed here after observing system hang.
	 * Root cause is not confirmed.
	 */
	msleep(1);
	/* restore the interupt enable for cable ID and VBUS */
	clk_enable(tegra_otg->clk);
	writel(tegra_otg->intr_reg_data, (tegra_otg->regs + USB_PHY_WAKEUP));
#ifdef CONFIG_MACH_N1
	msleep(20);
	tegra_recover_register(tegra_otg);
#endif
	clk_disable(tegra_otg->clk);

	return;
}

static const struct dev_pm_ops tegra_otg_pm_ops = {
	.complete = tegra_otg_resume,
	.suspend = tegra_otg_suspend,
};
#endif

static struct platform_driver tegra_otg_driver = {
	.driver = {
		.name  = "tegra-otg",
#ifdef CONFIG_PM
		.pm    = &tegra_otg_pm_ops,
#endif
	},
	.remove  = __exit_p(tegra_otg_remove),
	.probe   = tegra_otg_probe,
};

static int __init tegra_otg_init(void)
{
	return platform_driver_register(&tegra_otg_driver);
}
subsys_initcall(tegra_otg_init);

static void __exit tegra_otg_exit(void)
{
	platform_driver_unregister(&tegra_otg_driver);
}
module_exit(tegra_otg_exit);
