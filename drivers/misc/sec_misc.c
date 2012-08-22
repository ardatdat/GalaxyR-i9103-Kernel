/*
 * driver/misc/sec_misc.c
 *
 * driver supporting miscellaneous functions for Samsung boards
 *
 * COPYRIGHT(C) Samsung Electronics Co., Ltd. 2006-2010 All Right Reserved.
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include <linux/wakelock.h>
#include <linux/blkdev.h>
#if defined (CONFIG_MACH_BOSE_ATT)
#include <mach/gpio-bose.h>
#else
#include <mach/gpio-n1.h>
#endif
#include <linux/kernel_sec_common.h>
#include <mach/gpio.h>

static struct wake_lock sec_misc_wake_lock;

enum sec_uart_sel {
	UART_SEL_CP = 0,
	UART_SEL_AP
};

enum sec_usb_sel {
	USB_SEL_CP = 0,
	USB_SEL_AP
};

unsigned char emmc_checksum_done;
unsigned char emmc_checksum_pass;

static struct file_operations sec_misc_fops =
{
	.owner = THIS_MODULE,
	//.read = sec_misc_read,
	//.ioctl = sec_misc_ioctl,
	//.open = sec_misc_open,
	//.release = sec_misc_release,
};

static struct miscdevice sec_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sec_misc",
	.fops = &sec_misc_fops,
};

static void sec_set_usbpath(enum sec_usb_sel usb_sel)
{
#if 0
	if(usb_sel == USB_SEL_AP_USB) {
		gpio_set_value(GPIO_USB_SEL1, 1);
		gpio_set_value(GPIO_USB_SEL2, 0);
		usb_sel_status = USB_SEL_AP_USB;
	}
	else if(usb_sel == USB_SEL_CP_USB) {
		gpio_set_value(GPIO_USB_SEL1, 0);
		gpio_set_value(GPIO_USB_SEL2, 0);
		usb_sel_status = USB_SEL_CP_USB;
	}
	else if(usb_sel == USB_SEL_ADC) {
		gpio_set_value(GPIO_USB_SEL1, 0);
		gpio_set_value(GPIO_USB_SEL2, 1);
		usb_sel_status = USB_SEL_ADC;
	}
#endif
}

void sec_init_usbpath(void)
{
#if 0
	gpio_request(GPIO_USB_SEL1, "GPIO_USB_SEL1");
	gpio_direction_output(GPIO_USB_SEL1, 0);
	tegra_gpio_enable(GPIO_USB_SEL1);

	gpio_request(GPIO_USB_SEL2, "GPIO_USB_SEL2");
	gpio_direction_output(GPIO_USB_SEL2, 0);
	tegra_gpio_enable(GPIO_USB_SEL2);

	sec_set_usbpath(USB_SEL_AP_USB);
#endif
}

static void sec_set_uartpath(enum sec_uart_sel uart_sel)
{
	pr_info("UART path switched to %s\n", (uart_sel)?"AP":"CP");
	gpio_direction_output(GPIO_UART_SEL, (int)uart_sel);
}

void sec_init_uartpath(void)
{
	//tegra_gpio_enable(GPIO_UART_SEL);
	//gpio_request(GPIO_UART_SEL, "GPIO_UART_SEL");

	//sec_set_uartpath(UART_SEL_CP);

	//kernel_sec_path_type type = kernel_sec_get_path(SEC_PORT_UART);
	//if (type == SEC_PORT_PATH_AP)
	//	gpio_set_value(GPIO_UART_SEL, 1);       // Set UART path to AP
	//else
	//	gpio_set_value(GPIO_UART_SEL, 0);       // Set UART path to CP

}

static ssize_t uart_sel_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	ssize_t	ret;
	int PinValue=5;

	PinValue = gpio_get_value(GPIO_UART_SEL);

	if(PinValue == 0)
		ret = sprintf(buf, "Current UART path is switched to CP\n");
	else if(PinValue == 1)
		ret = sprintf(buf, "Current UART path is switched to AP\n");
	else
		ret = sprintf(buf, "%s\n", __func__);
	return ret;
}

static ssize_t uart_sel_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t size)
{
	int state;

	if (sscanf(buf, "%i", &state) != 1 || (state < 0 || state > 1))
		return -EINVAL;

	/* prevents the system from entering suspend */
	wake_lock(&sec_misc_wake_lock);

	if(state == 1) {
		sec_set_uartpath(UART_SEL_AP);
		kernel_sec_set_path(SEC_PORT_UART, SEC_PORT_PATH_AP);
	}
	else if(state == 0) {
		sec_set_uartpath(UART_SEL_CP);
		kernel_sec_set_path(SEC_PORT_UART, SEC_PORT_PATH_CP);
	}
	else
		pr_info("Enter 1(AP uart) or 0(CP uart)...\n");

	wake_unlock(&sec_misc_wake_lock);
	return size;
}

static DEVICE_ATTR(uartsel, 0664, uart_sel_show, uart_sel_store);


static ssize_t usb_sel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
#if 0
	ssize_t	ret;

	if(usb_sel_status == USB_SEL_ADC)
		ret = sprintf(buf, "USB path => ADC\n");
	else if(usb_sel_status == USB_SEL_AP_USB)
		ret = sprintf(buf, "USB path => AP\n");
	else if (usb_sel_status==USB_SEL_CP_USB)
		ret = sprintf(buf, "USB path => CP\n");
	else
		ret = sprintf(buf, "usb_sel_show\n");

	return ret;
#endif
	return sprintf(buf, "not avaiable\n");
}

static ssize_t usb_sel_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t size)
{
#if 0
	int state;

	if (sscanf(buf, "%i", &state) != 1 || (state < 0 || state > 2))
		return -EINVAL;

	// prevents the system from entering suspend
	wake_lock(&sec_misc_wake_lock);

	if(state == 2)	{
		sec_set_usbpath(USB_SEL_ADC);	// Set USB path to CP
		klogi("Set USB path to ADC\n");
	}
	else if(state == 1)	{
		sec_set_usbpath(USB_SEL_AP_USB);	// Set USB path to AP
		klogi("Set USB path to AP\n");
	}
	else if(state == 0)	{
		sec_set_usbpath(USB_SEL_CP_USB);	// Set USB path to CP
		klogi("Set USB path to CP\n");
	}

	else
		klogi("Enter 2(ADC usb) or 1(AP usb) or 0(CP usb)...\n");

	wake_unlock(&sec_misc_wake_lock);

	return size;
#endif
	return -EINVAL;
}

static DEVICE_ATTR(usbsel, 0664, usb_sel_show, usb_sel_store);

static ssize_t emmc_checksum_done_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", emmc_checksum_done);
}

static ssize_t emmc_checksum_done_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int state;

	if (sscanf(buf, "%i", &state) != 1 || (state < 0 || state > 1))
		return -EINVAL;

	emmc_checksum_done = (unsigned char)state;
	return size;
}

static DEVICE_ATTR(emmc_checksum_done, 0664, emmc_checksum_done_show, emmc_checksum_done_store);

static ssize_t emmc_checksum_pass_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", emmc_checksum_pass);
}

static ssize_t emmc_checksum_pass_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int state;

	if (sscanf(buf, "%i", &state) != 1 || (state < 0 || state > 1))
		return -EINVAL;

	emmc_checksum_pass = (unsigned char)state;
	return size;
}

static DEVICE_ATTR(emmc_checksum_pass, 0664, emmc_checksum_pass_show, emmc_checksum_pass_store);

static void sec_init_ifconsense(void)
{
	gpio_request(GPIO_IF_CON_SENSE, "if_con_sense");
	gpio_direction_input(GPIO_IF_CON_SENSE);
	tegra_gpio_enable(GPIO_IF_CON_SENSE);
}

extern struct class *sec_class;
struct device *sec_misc_dev;

static int __init sec_misc_init(void)
{
	int ret=0;

	ret = misc_register(&sec_misc_device);
	if (ret<0) {
		pr_err("misc_register failed!\n");
		return ret;
	}

	sec_misc_dev = device_create(sec_class, NULL, 0, NULL, "sec_misc");
	if (IS_ERR(sec_misc_dev)) {
		pr_err("failed to create device!\n");
		return -1;
	}

	if (device_create_file(sec_misc_dev, &dev_attr_uartsel) < 0)
		pr_err("failed to create device file - %s\n", dev_attr_uartsel.attr.name);

	if (device_create_file(sec_misc_dev, &dev_attr_usbsel) < 0)
		pr_err("failed to create device file - %s\n", dev_attr_usbsel.attr.name);

	if (device_create_file(sec_misc_dev, &dev_attr_emmc_checksum_done) < 0)
		pr_err("failed to create device file - %s\n", dev_attr_emmc_checksum_done.attr.name);

	if (device_create_file(sec_misc_dev, &dev_attr_emmc_checksum_pass) < 0)
		pr_err("failed to create device file - %s\n", dev_attr_emmc_checksum_pass.attr.name);

	sec_init_uartpath();
	sec_init_usbpath();
	sec_init_ifconsense();

	wake_lock_init(&sec_misc_wake_lock, WAKE_LOCK_SUSPEND, "sec_misc");

	return 0;
}

static void __exit sec_misc_exit(void)
{
	wake_lock_destroy(&sec_misc_wake_lock);

	device_remove_file(sec_misc_dev, &dev_attr_uartsel);
	device_remove_file(sec_misc_dev, &dev_attr_usbsel);
}

module_init(sec_misc_init);
module_exit(sec_misc_exit);

/* Module information */
MODULE_AUTHOR("Samsung");
MODULE_DESCRIPTION("Samsung misc. driver");
MODULE_LICENSE("GPL");
