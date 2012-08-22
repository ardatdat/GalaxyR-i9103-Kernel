/*
 * driver/misc/fsa9480.c - FSA9480 micro USB switch device driver
 *
 * Copyright (C) 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Wonguk Jeong <wonguk.jeong@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/fsa9480.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/kernel_sec_common.h>
#if defined (CONFIG_MACH_BOSE_ATT)
#include <mach/gpio-bose.h>
#else
#include <mach/gpio-n1.h>
#endif
#include <mach/gpio.h>
#if (defined(CONFIG_MHL_SWITCH) && defined(CONFIG_MHL_SII9234))
#include <linux/power_supply.h>
#endif

/* FSA9480 I2C registers */
#define FSA9480_REG_DEVID		0x01
#define FSA9480_REG_CTRL		0x02
#define FSA9480_REG_INT1		0x03
#define FSA9480_REG_INT2		0x04
#define FSA9480_REG_INT1_MASK		0x05
#define FSA9480_REG_INT2_MASK		0x06
#define FSA9480_REG_ADC			0x07
#define FSA9480_REG_TIMING1		0x08
#define FSA9480_REG_TIMING2		0x09
#define FSA9480_REG_DEV_T1		0x0a
#define FSA9480_REG_DEV_T2		0x0b
#define FSA9480_REG_BTN1		0x0c
#define FSA9480_REG_BTN2		0x0d
#define FSA9480_REG_CK			0x0e
#define FSA9480_REG_CK_INT1		0x0f
#define FSA9480_REG_CK_INT2		0x10
#define FSA9480_REG_CK_INTMASK1		0x11
#define FSA9480_REG_CK_INTMASK2		0x12
#define FSA9480_REG_MANSW1		0x13
#define FSA9480_REG_MANSW2		0x14
#define FSA9480_REG_HIDDEN		0x1B
#define FSA9480_REG_HIDDEN2		0x1D

/* Control */
#define CON_SWITCH_OPEN		(1 << 4)
#define CON_RAW_DATA		(1 << 3)
#define CON_MANUAL_SW		(1 << 2)
#define CON_WAIT		(1 << 1)
#define CON_INT_MASK		(1 << 0)
#define CON_MASK		(CON_SWITCH_OPEN | CON_RAW_DATA | \
				CON_MANUAL_SW | CON_WAIT)

/* Interrupt 1 */
#define INT1_OVP_OCP_DIS	(1 << 7)
#define INT1_OVP_EN		(1 << 5)
#define INT1_DETACH		(1 << 1)
#define INT1_ATTACH		(1 << 0)

/* Interrupt 2*/
#define INT2_AV_CHARGING	(1 << 0)

/* Device Type 1 */
#define DEV_USB_OTG		(1 << 7)
#define DEV_DEDICATED_CHG	(1 << 6)
#define DEV_USB_CHG		(1 << 5)
#define DEV_CAR_KIT		(1 << 4)
#define DEV_UART		(1 << 3)
#define DEV_USB			(1 << 2)
#define DEV_AUDIO_2		(1 << 1)
#define DEV_AUDIO_1		(1 << 0)

#define DEV_T1_USB_OTG_MASK	(DEV_USB_OTG)
#define DEV_T1_USB_MASK		(DEV_USB)
#define DEV_T1_UART_MASK	(DEV_UART)
#define DEV_T1_CHARGER_MASK	(DEV_DEDICATED_CHG | DEV_USB_CHG | DEV_CAR_KIT)

/* Device Type 2 */
#define DEV_AV			(1 << 6)
#define DEV_TTY			(1 << 5)
#define DEV_PPD			(1 << 4)
#define DEV_JIG_UART_OFF	(1 << 3)
#define DEV_JIG_UART_ON		(1 << 2)
#define DEV_JIG_USB_OFF		(1 << 1)
#define DEV_JIG_USB_ON		(1 << 0)

#define DEV_T2_USB_MASK		(DEV_JIG_USB_OFF | DEV_JIG_USB_ON)
#define DEV_T2_UART_MASK	DEV_JIG_UART_OFF
#define DEV_T2_JIG_MASK		(DEV_JIG_USB_OFF | DEV_JIG_USB_ON | \
				DEV_JIG_UART_OFF)

/*
 * Manual Switch
 * D- [7:5] / D+ [4:2]
 * 000: Open all / 001: USB / 010: AUDIO / 011: UART / 100: V_AUDIO
 */
#define SW_VAUDIO		((4 << 5) | (4 << 2))
#define SW_UART			((3 << 5) | (3 << 2))
#define SW_AUDIO		((2 << 5) | (2 << 2))
#define SW_DHOST		((1 << 5) | (1 << 2))
#define SW_AUTO			((0 << 5) | (0 << 2))

#define SW_ID_OPEN		((0 << 1) | (0 << 0))
#define SW_ID_VIDEO		((0 << 1) | (1 << 0))
#define SW_ID_BYPASS	        ((1 << 1) | (0 << 0))

/* Interrupt 1 */
#define INT_DETACH		(1 << 1)
#define INT_ATTACH		(1 << 0)

/* Hidden */
#define HD_RESET		(1 << 0)

/* Hidden2 */
#define HD2_VBUS_VALID		(1 << 1)

static int usb_path = 1; /* Initial value is AP */
static int usb_state = 0;

enum
{
	SEL_CP = 0,
	SEL_AP
};

enum
{
	USB_NOT_CONFIGURED = 0,
	USB_CONFIGURED
};

#if (defined(CONFIG_MHL_SWITCH) && defined(CONFIG_MHL_SII9234))
bool gv_mhl_sw_state = 0;
extern u8 mhl_cable_status;
extern bool mhl_vbus; //3355

static struct fsa9480_usbsw global_usbsw;
static struct fsa9480_usbsw *local_usbsw=&global_usbsw;
static u8 FSA9480_AudioDockConnectionStatus=0x00;
static struct mutex FSA9480_MhlSwitchSel_lock;
void FSA9480_MhlSwitchSel(bool sw);
extern void sii9234_cfg_power(bool on);
extern void mhl_hw_reset(void);
extern bool SiI9234_init(void);
extern void mhl_hpd_handler(bool);
extern bool tegra_dc_hdmi_hpd(void);
extern  int sii9234_switch_onoff(bool );
int gv_intr2 = 0,mhl_power_supply=0;

struct fsa9480_platform_data *local_pdata;

void DisableFSA9480Interrupts(void)
{
	struct i2c_client *client = local_usbsw->client;
	int value,ret;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_CTRL);

	msleep(5);
       value |= 0x01;

	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

} // DisableFSA9480Interrupts()

void EnableFSA9480Interrupts(void)
{
	struct i2c_client *client = local_usbsw->client;
	int value,ret;
	value = i2c_smbus_read_byte_data(client, FSA9480_REG_CTRL);
	msleep(5);
	value &= 0xFE;
	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);
} //EnableFSA9480Interrupts()


//This function set the fsa audio dock status and update the audio driver state variable
static void FSA9480_AudioDockEnable(void)
{
	//if (pdata->deskdock_cb)
		//pdata->deskdock_cb(FSA9480_ATTACHED);
}

//This function will reset the fsa audio dock status and update the audio driver state variable
static void FSA9480_AudioDockDisable(void)
{
	//if (pdata->deskdock_cb)
		//pdata->deskdock_cb(FSA9480_DETACHED);
	FSA9480_AudioDockConnectionStatus = 0;
}
/* This function is implimented to clear unwanted/garbage interrupts and hook audio dock functionality.
 *  Note:- Minimise Audio connection time delay as much as possible.
 *  Recomonded to minimize debug prints at audio_dock connection path
 */
void FSA9480_CheckAndHookAudioDock(void)
{
	FSA9480_AudioDockConnectionStatus++;
	FSA9480_MhlSwitchSel(0);
	FSA9480_AudioDockEnable();
}

void FSA9480_MhlSwitchSel(bool sw)
{
	struct i2c_client *client = local_usbsw->client;
	int hidden_reg, vbus_in;

	//struct power_supply *mhl_power_supply = power_supply_get_by_name("battery");
	//	union power_supply_propval value;
    printk("[FSA] mutex_lock\n");
	//mutex_lock(&FSA9480_MhlSwitchSel_lock);//avoid race condtion between mhl and hpd
	if(sw)
	{
               sii9234_cfg_power(1);
		if (gv_intr2&0x1) //cts: fix for 3355
		{
			mhl_vbus = true;
			mhl_power_supply=1;
			local_pdata->charger_cb(FSA9480_ATTACHED);
			gv_intr2=0;
			printk("vbus_power is detected\n");
		}

		SiI9234_init();

		//charging with MHL dock
		hidden_reg = i2c_smbus_read_word_data(client, FSA9480_REG_HIDDEN2);
		vbus_in = ((hidden_reg & HD2_VBUS_VALID) ? 1:0);
		dev_info(&client->dev, "[FSA9480] Deskdock Attach, hidden_reg2 = %x, vbus_in = %d \n", hidden_reg, vbus_in );
		if (local_usbsw->pdata->mhldock_cb)
			local_usbsw->pdata->mhldock_cb(FSA9480_ATTACHED);
		if (vbus_in == 1 && local_usbsw->pdata->dock_charger_cb)
			local_usbsw->pdata->dock_charger_cb(FSA9480_ATTACHED);
	}

	else
	{
		if (tegra_dc_hdmi_hpd())
		{
			printk("[FSA] Turn off hdmi\n");
			mhl_hpd_handler(false);
		}
                      if(mhl_power_supply==1)
			{
				mhl_power_supply=0;
				local_pdata->charger_cb(FSA9480_DETACHED);
			}
		mhl_vbus = false;
		sii9234_cfg_power(0);	//Turn Off power to SiI9234
		if (local_usbsw->pdata->mhldock_cb)
			local_usbsw->pdata->mhldock_cb(FSA9480_DETACHED);
		if (local_usbsw->pdata->dock_charger_cb)
			local_usbsw->pdata->dock_charger_cb(FSA9480_DETACHED);

	}

	/********************FSA-MHL-FSA Switching start***************************/
	DisableFSA9480Interrupts();
	{
		//gpio_set_value_cansleep(GPIO_MHL_SEL, sw);
		sii9234_switch_onoff(sw);

		i2c_smbus_read_byte_data(client, FSA9480_REG_INT1);
	}
	EnableFSA9480Interrupts();
	/******************FSA-MHL-FSA Switching End*******************************/
	//mutex_unlock(&FSA9480_MhlSwitchSel_lock);

    printk("[FSA] mutex_unlock\n");
}
EXPORT_SYMBOL(FSA9480_MhlSwitchSel);

void FSA9480_MhlTvOff(void)
{
	struct i2c_client *client =  local_usbsw->client;
	int intr1;
	DisableFSA9480Interrupts();
	//printk(KERN_ERR "%s: started######\n", __func__);
	intr1 = i2c_smbus_read_word_data(client, FSA9480_REG_INT1);
	gpio_set_value(GPIO_MHL_SEL, 0);
	do
	{
		msleep(5);
		intr1 = i2c_smbus_read_byte_data(client, FSA9480_REG_INT1);
	}while(!intr1);

	mhl_cable_status =0x08;//MHL_TV_OFF_CABLE_CONNECT;
	EnableFSA9480Interrupts();

	//charging with MHL dock
	if (local_usbsw->pdata->mhldock_cb)
		local_usbsw->pdata->mhldock_cb(FSA9480_DETACHED);
	if (local_usbsw->pdata->dock_charger_cb)
		local_usbsw->pdata->dock_charger_cb(FSA9480_DETACHED);

	//printk(KERN_ERR "%s: End######\n",__func__);
	printk("%s:  interrupt1= %d\n", __func__, intr1);
}
EXPORT_SYMBOL(FSA9480_MhlTvOff);
#endif
#if defined (CONFIG_MACH_BOSE_ATT)
static struct fsa9480_usbsw *local_usbsw;

int FSA9480_Get_I2C_USB_Status(void)
{
	struct i2c_client *client = local_usbsw->client;
	unsigned char dev1, dev2;
	int result;

	dev1 = i2c_smbus_read_byte_data(client,FSA9480_REG_DEV_T1);
	dev2 = i2c_smbus_read_byte_data(client,FSA9480_REG_DEV_T2);

	   result = dev2 << 8 | dev1;
	return result;
}
EXPORT_SYMBOL(FSA9480_Get_I2C_USB_Status);
#endif

static ssize_t fsa9480_show_control(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_CTRL);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	return sprintf(buf, "CONTROL: %02x\n", value);
}

static ssize_t fsa9480_show_device_type(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_DEV_T1);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	return sprintf(buf, "DEVICE_TYPE: %02x\n", value);
}

static ssize_t fsa9480_show_manualsw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_MANSW1);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if (value == SW_VAUDIO)
		return sprintf(buf, "VAUDIO\n");
	else if (value == SW_UART)
		return sprintf(buf, "UART\n");
	else if (value == SW_AUDIO)
		return sprintf(buf, "AUDIO\n");
	else if (value == SW_DHOST)
		return sprintf(buf, "DHOST\n");
	else if (value == SW_AUTO)
		return sprintf(buf, "AUTO\n");
	else
		return sprintf(buf, "%x", value);
}

static ssize_t fsa9480_set_manualsw(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;
	unsigned int path = 0;
	int ret;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_CTRL);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if ((value & ~CON_MANUAL_SW) !=
		(CON_SWITCH_OPEN | CON_RAW_DATA | CON_WAIT))
		return 0;

	if (!strncmp(buf, "VAUDIO", 6)) {
		path = SW_VAUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "UART", 4)) {
		path = SW_UART;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "AUDIO", 5)) {
		path = SW_AUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "DHOST", 5)) {
		path = SW_DHOST;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "AUTO", 4)) {
		path = SW_AUTO;
		value |= CON_MANUAL_SW;
	} else {
		dev_err(dev, "Wrong command\n");
		return 0;
	}

	usbsw->mansw = path;

	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_MANSW1, path);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return count;
}

static ssize_t usb_sel_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	ssize_t	ret;

	if(usb_path == SEL_CP)
		ret = sprintf(buf, "USB path : Phone\n");
	else if (usb_path == SEL_AP)
		ret = sprintf(buf, "USB path : PDA\n");
	else
		ret = sprintf(buf, "not avaiable\n");

	return ret;
}

static ssize_t usb_sel_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;
	unsigned int path = 0;
	int ret;

	int state;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_CTRL);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if ((value & ~CON_MANUAL_SW) !=
		(CON_SWITCH_OPEN | CON_RAW_DATA | CON_WAIT))
		return 0;

	if (sscanf(buf, "%i", &state) != 1 || (state < 0 || state > 2))
		return -EINVAL;

	if (state == SEL_CP)
	{
		path = SW_AUDIO;
		value &= ~CON_MANUAL_SW;

		usbsw->mansw = path;

		ret = i2c_smbus_write_byte_data(client, FSA9480_REG_MANSW1, path);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, value);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		usb_path = SEL_CP;
		kernel_sec_set_path(SEC_PORT_USB, SEC_PORT_PATH_CP);
	}
	else if(state == SEL_AP)
	{
		usbsw->mansw = SW_AUTO;
		ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, CON_MASK);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		usb_path = SEL_AP;
		kernel_sec_set_path(SEC_PORT_USB, SEC_PORT_PATH_AP);
	}

	return count;
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
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;

	int state;
	int ret;

	if (sscanf(buf, "%i", &state) != 1 || (state < 0 || state > 1))
		return -EINVAL;

	if(state == SEL_AP) {
		gpio_direction_output(GPIO_UART_SEL, (int)SEL_AP);
		kernel_sec_set_path(SEC_PORT_UART, SEC_PORT_PATH_AP);
	}
	else if(state == SEL_CP) {
		gpio_direction_output(GPIO_UART_SEL, (int)SEL_CP);
		kernel_sec_set_path(SEC_PORT_UART, SEC_PORT_PATH_CP);
	}

	//Need to check.
	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, CON_MASK);
	if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return size;
}

static ssize_t usb_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t	ret;

	if(usb_state == USB_CONFIGURED)
		ret = sprintf(buf, "USB_STATE_CONFIGURED\n");
	else
		ret = sprintf(buf, "USB_STATE_NOTCONFIGURED\n");

	return ret;
}

static ssize_t usb_state_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t size)
{
	int state;

	if (sscanf(buf, "%i", &state) != 1 || (state < 0 || state > 1))
		return -EINVAL;

	usb_state = state;

	return size;
}

static ssize_t fsa9480_show_adc(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int val;

	pr_info("fsa9480_show_adc+\n");

	val = i2c_smbus_read_byte_data(client, FSA9480_REG_ADC);
	val &= 0x1f;

	return sprintf(buf, "%x\n", (val & 0x1f));
}


static DEVICE_ATTR(control, S_IRUGO, fsa9480_show_control, NULL);
static DEVICE_ATTR(device_type, S_IRUGO, fsa9480_show_device_type, NULL);
static DEVICE_ATTR(switch, S_IRUGO | S_IWUSR,	fsa9480_show_manualsw, fsa9480_set_manualsw);
static DEVICE_ATTR(usb_sel, 0664, usb_sel_show, usb_sel_store);
static DEVICE_ATTR(uartsel, 0664, uart_sel_show, uart_sel_store);
static DEVICE_ATTR(usbstate, 0664, usb_state_show, usb_state_store);
static DEVICE_ATTR(adc, 0664, fsa9480_show_adc, NULL);

static struct attribute *fsa9480_attributes[] = {
	&dev_attr_control.attr,
	&dev_attr_device_type.attr,
	&dev_attr_switch.attr,
	&dev_attr_usb_sel.attr,
	&dev_attr_uartsel.attr,
	&dev_attr_usbstate.attr,
	NULL
};

static const struct attribute_group fsa9480_group = {
	.attrs = fsa9480_attributes,
};

static void fsa9480_otg_id_open(struct fsa9480_usbsw *usbsw)
{
	int i, ret;
	struct i2c_client *client = usbsw->client;

	for(i=0; i < 5; i++) {
		ret =i2c_smbus_write_byte_data(client,
				FSA9480_REG_HIDDEN, HD_RESET);
		if (!ret)
			return;
		msleep(10);
	}
	dev_err(&client->dev, "%s: err %d\n", __func__, ret);
}

static void fsa9480_ovp_dev(struct fsa9480_usbsw *usbsw, unsigned char int1)
{
	struct fsa9480_platform_data *pdata = usbsw->pdata;

	if (int1 & INT1_OVP_OCP_DIS) {
		if (pdata->charger_ovp_cb)
			pdata->charger_ovp_cb(FSA9480_RECOVER_OVP);
	}
	else if (int1 & INT1_OVP_EN) {
		if (pdata->charger_ovp_cb)
			pdata->charger_ovp_cb(FSA9480_OVP);
	}
}

static void fsa9480_detect_dev(struct fsa9480_usbsw *usbsw)
{
	int device_type, hidden_reg, vbus_in;
	int ret = 0;
	int vchg = 0;
	unsigned char val1, val2;
	struct fsa9480_platform_data *pdata = usbsw->pdata;
#if (defined(CONFIG_MHL_SWITCH) && defined(CONFIG_MHL_SII9234))

        local_pdata = usbsw->pdata;
#endif
	struct i2c_client *client = usbsw->client;

	mutex_lock(&usbsw->lock);
	device_type = i2c_smbus_read_word_data(client, FSA9480_REG_DEV_T1);
	if (device_type < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, device_type);

	val1 = device_type & 0xff;
	val2 = device_type >> 8;

	dev_info(&client->dev, "dev1: 0x%x, dev2: 0x%x\n", val1, val2);

	/* Attached */
	if (val1 || val2) {
		/* USB OTG*/
		if (val1 & DEV_T1_USB_OTG_MASK) {
			if (pdata->otg_cb)
				pdata->otg_cb(FSA9480_ATTACHED);
			if (usbsw->mansw)
				ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_CTRL, CON_MASK);
				if (ret < 0)
					dev_err(&client->dev, "%s: err %d\n", __func__, ret);
		/* USB */
		} else if (val1 & DEV_USB_CHG ||
				val1 & DEV_T1_USB_MASK ||
				val2 & DEV_T2_USB_MASK) {
			if (pdata->usb_cb) {
				usb_state = USB_CONFIGURED;
				pdata->usb_cb(FSA9480_ATTACHED);
			}
			if (pdata->inform_charger_connection)
				pdata->inform_charger_connection(true);

			if (usbsw->mansw) {
				ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_MANSW1, usbsw->mansw);
				if (ret < 0)
					dev_err(&client->dev,
						"%s: err %d\n", __func__, ret);
			}
		/* UART */
		} else if (val1 & DEV_T1_UART_MASK || val2 & DEV_T2_UART_MASK) {
			if (pdata->uart_cb)
				vchg = pdata->uart_cb(FSA9480_ATTACHED);

			dev_info(&client->dev, "[FSA9480] Attached UART, vchg = %d \n", vchg );
			if (vchg && pdata->inform_charger_connection)
				pdata->inform_charger_connection(true);
			if (usbsw->mansw) {
				ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_MANSW1, SW_UART);
				if (ret < 0)
					dev_err(&client->dev,
						"%s: err %d\n", __func__, ret);
			}
		/* CHARGER */
		} else if (val1 & DEV_T1_CHARGER_MASK) {
			if (pdata->charger_cb)
				pdata->charger_cb(FSA9480_ATTACHED);
			if (pdata->inform_charger_connection)
				pdata->inform_charger_connection(true);
		/* JIG */
		} else if (val2 & DEV_T2_JIG_MASK) {
			if (pdata->jig_cb)
				pdata->jig_cb(FSA9480_ATTACHED);
		/* Desk Dock */
		} else if (val2 & DEV_AV) {
#if (defined(CONFIG_MHL_SWITCH) && defined(CONFIG_MHL_SII9234))
			printk("FSA MHL Attach \n");
			printk("mhl_cable_status = %d \n", mhl_cable_status);

			FSA9480_MhlSwitchSel(1);
#else
			hidden_reg = i2c_smbus_read_word_data(client, FSA9480_REG_HIDDEN2);
			vbus_in = ((hidden_reg & HD2_VBUS_VALID) ? 1:0);
			dev_info(&client->dev, "[FSA9480] Deskdock Attach, hidden_reg2 = %x, vbus_in = %d \n", hidden_reg, vbus_in );
			if (pdata->deskdock_cb)
				pdata->deskdock_cb(FSA9480_ATTACHED);
			if (vbus_in == 1 && pdata->dock_charger_cb)
				pdata->dock_charger_cb(FSA9480_ATTACHED);
#endif
		/* Car Dock */
		} else if (val2 & DEV_JIG_UART_ON) {
			hidden_reg = i2c_smbus_read_word_data(client, FSA9480_REG_HIDDEN2);
			vbus_in = ((hidden_reg & HD2_VBUS_VALID) ? 1:0);
			dev_info(&client->dev, "[FSA9480] Cardock Attach, hidden_reg2 = %x, vbus_in = %d \n", hidden_reg, vbus_in );
			if (pdata->cardock_cb)
				pdata->cardock_cb(FSA9480_ATTACHED);
			if (vbus_in == 1 && pdata->dock_charger_cb)
				pdata->dock_charger_cb(FSA9480_ATTACHED);
		}
	/* Detached */
	} else {
		/* USB OTG*/
		if (usbsw->dev1 & DEV_T1_USB_OTG_MASK) {
			if (pdata->otg_cb)
				pdata->otg_cb(FSA9480_DETACHED);
		/* USB */
		} else if (usbsw->dev1 & DEV_USB_CHG ||
				usbsw->dev1 & DEV_T1_USB_MASK ||
				usbsw->dev2 & DEV_T2_USB_MASK) {
			if (pdata->usb_cb) {
				usb_state = USB_NOT_CONFIGURED;
				pdata->usb_cb(FSA9480_DETACHED);
			}
			if (pdata->inform_charger_connection)
				pdata->inform_charger_connection(false);
		/* UART */
		} else if (usbsw->dev1 & DEV_T1_UART_MASK ||
				usbsw->dev2 & DEV_T2_UART_MASK) {
			if (pdata->uart_cb)
				vchg = pdata->uart_cb(FSA9480_DETACHED);

			dev_info(&client->dev, "[FSA9480] Detached UART , vchg = %d\n", vchg);
			if (vchg && pdata->inform_charger_connection)
				pdata->inform_charger_connection(false);
		/* CHARGER */
		} else if (usbsw->dev1 & DEV_T1_CHARGER_MASK) {
			if (pdata->charger_cb)
				pdata->charger_cb(FSA9480_DETACHED);
			if (pdata->inform_charger_connection)
				pdata->inform_charger_connection(false);
		/* JIG */
		} else if (usbsw->dev2 & DEV_T2_JIG_MASK) {
			if (pdata->jig_cb)
				pdata->jig_cb(FSA9480_DETACHED);
		/* Desk Dock */
		} else if (usbsw->dev2 & DEV_AV) {
#if (defined(CONFIG_MHL_SWITCH) && defined(CONFIG_MHL_SII9234))

			printk("FSA MHL Detach\n");
			FSA9480_MhlSwitchSel(0);

#else
			if (pdata->deskdock_cb)
				pdata->deskdock_cb(FSA9480_DETACHED);
			if (pdata->dock_charger_cb)
				pdata->dock_charger_cb(FSA9480_DETACHED);
#endif
		/* Car Dock */
		} else if (usbsw->dev2 & DEV_JIG_UART_ON) {
			if (pdata->cardock_cb)
				pdata->cardock_cb(FSA9480_DETACHED);
			if (pdata->dock_charger_cb)
				pdata->dock_charger_cb(FSA9480_DETACHED);
		}
	}

	usbsw->dev1 = val1;
	usbsw->dev2 = val2;
	mutex_unlock(&usbsw->lock);
}

static void fsa9480_reg_init(struct fsa9480_usbsw *usbsw, int mansw_set)
{
	struct i2c_client *client = usbsw->client;
	unsigned int ctrl = CON_MASK;
	int ret;

	/* mask interrupts (unmask attach/detach/ovp_dcp_dis/ ovp_en only) */
	ret = i2c_smbus_write_word_data(client, FSA9480_REG_INT1_MASK, 0xFF5C);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	/* mask all car kit interrupts */
	ret = i2c_smbus_write_word_data(client,
			FSA9480_REG_CK_INTMASK1, 0x07ff);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	/* ADC Detect Time: 500ms */
	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_TIMING1, 0x4);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	if (mansw_set) {
		if (usbsw->mansw) {
			ret = i2c_smbus_write_byte_data(client,
				FSA9480_REG_MANSW1, usbsw->mansw);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);
			ctrl &= ~CON_MANUAL_SW;	/* Manual Switching Mode */
		}

		ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, ctrl);
	} else {
		usbsw->mansw = i2c_smbus_read_byte_data(client,
						FSA9480_REG_MANSW1);
		if (usbsw->mansw < 0)
			dev_err(&client->dev,
				"%s: err %d\n", __func__, usbsw->mansw);
	}

#if (defined(CONFIG_MHL_SWITCH) && defined(CONFIG_MHL_SII9234))
	i2c_smbus_write_byte_data(client, FSA9480_REG_INT2_MASK, 0xFE & i2c_smbus_read_byte_data(client, 0x06));
#endif
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);
}

static irqreturn_t fsa9480_irq_thread(int irq, void *data)
{
	struct fsa9480_usbsw *usbsw = data;
	struct i2c_client *client = usbsw->client;
	int intr;
	unsigned char val1, val2;

	/* read and clear interrupt status bits */
	intr = i2c_smbus_read_word_data(client, FSA9480_REG_INT1);
	val1 = intr & 0xff;
	val2 = intr >> 8;
	dev_info(&client->dev, "int1: 0x%x, int2: 0x%x\n", val1, val2);
#if (defined(CONFIG_MHL_SWITCH) && defined(CONFIG_MHL_SII9234))
	gv_intr2 = intr >> 8;
#endif
	if (intr < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, intr);

		/* read and clear interrupt status bits one more time*/
		mdelay(5);
		intr = i2c_smbus_read_word_data(client, FSA9480_REG_INT1);
		val1 = intr & 0xff;
		val2 = intr >> 8;
		dev_info(&client->dev, "try to read one more time! int1:\
				0x%x, int2: 0x%x\n", val1, val2);
	}
	if (val1 == 0) {
		/* interrupt was fired, but no status bits were set,
		so device was reset. In this case, the registers were
		reset to defaults so they need to be reinitialised. */
		pr_info("fsa9480 reset\n");
		fsa9480_reg_init(usbsw, 1);
		fsa9480_detect_dev(usbsw);
		return IRQ_HANDLED;
	}

	/* device detection */

	if ((val1 & (INT1_ATTACH | INT1_DETACH)) || val2&0x01)
		fsa9480_detect_dev(usbsw);
	else
		fsa9480_ovp_dev(usbsw, val1);

	return IRQ_HANDLED;
}

static void fsa9480_recheck_irq(struct fsa9480_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	int intr;
	unsigned char val1, val2;

	/* read and clear interrupt status bits */
	intr = i2c_smbus_read_word_data(client, FSA9480_REG_INT1);
	val1 = intr & 0xff;
	val2 = intr >> 8;
	dev_info(&client->dev, "%s int1: 0x%x, int2: 0x%x\n",\
		__func__, val1, val2);

	if (intr < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, intr);
	else if (intr == 0) {
		/* interrupt was fired, but no status bits were set,
		so device was reset. In this case, the registers were
		reset to defaults so they need to be reinitialised. */
		pr_info("fsa9480 reset\n");
		fsa9480_reg_init(usbsw, 1);
		fsa9480_detect_dev(usbsw);
		return;
	}

	/* device detection */
	if (val1 & (INT1_ATTACH | INT1_DETACH))
		fsa9480_detect_dev(usbsw);
	else
		fsa9480_ovp_dev(usbsw, val1);

	return;
}

static int fsa9480_irq_init(struct fsa9480_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	int ret;

	if (client->irq) {
		ret = request_threaded_irq(client->irq, NULL,
			fsa9480_irq_thread, IRQF_TRIGGER_FALLING,
			"fsa9480 micro USB", usbsw);
		if (ret) {
			dev_err(&client->dev, "failed to reqeust IRQ\n");
			return ret;
		}

		ret = enable_irq_wake(client->irq);
		if (ret < 0)
			dev_err(&client->dev,
				"failed to enable wakeup src %d\n", ret);
		/* read and clear interrupt status bits */
		ret = i2c_smbus_read_word_data(client, FSA9480_REG_INT1);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);
	}

	return 0;
}

extern struct class *sec_class;

static int __devinit fsa9480_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct fsa9480_usbsw *usbsw;
	struct device *switch_dev;
	int ret = 0;

	pr_info("fsa9480_probe+\n");

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	usbsw = kzalloc(sizeof(struct fsa9480_usbsw), GFP_KERNEL);
	if (!usbsw) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	usbsw->client = client;
	usbsw->pdata = client->dev.platform_data;
	if (!usbsw->pdata)
		goto fail1;

	client->irq = gpio_to_irq(usbsw->client->irq);
	pr_info("%s :now IRQ num %d\n", __func__, client->irq);

	i2c_set_clientdata(client, usbsw);
#if defined(CONFIG_MACH_BOSE_ATT)
	local_usbsw = usbsw;
#endif
	if (usbsw->pdata->cfg_gpio)
		usbsw->pdata->cfg_gpio();

	mutex_init(&usbsw->lock);

	fsa9480_reg_init(usbsw, 0);

	ret = fsa9480_irq_init(usbsw);
	if (ret)
		goto fail1;

	ret = sysfs_create_group(&client->dev.kobj, &fsa9480_group);
	if (ret) {
		dev_err(&client->dev,
				"failed to create fsa9480 attribute group\n");
		goto fail2;
	}

	switch_dev = device_create(sec_class, NULL, 0, NULL, "switch");
	ret = device_create_file(switch_dev, &dev_attr_adc);
	dev_set_drvdata(switch_dev, usbsw);

	ret = device_create_file(switch_dev, &dev_attr_usb_sel);

	gpio_export(GPIO_UART_SEL, 1);
	gpio_export_link(switch_dev, "uart_sel", GPIO_UART_SEL);

	if (usbsw->pdata->reset_cb)
		usbsw->pdata->reset_cb();

	/* device detection */
	fsa9480_detect_dev(usbsw);

	if (usbsw->pdata->set_otg_func)
		usbsw->pdata->set_otg_func(fsa9480_otg_id_open, usbsw);

	return 0;

fail2:
	if (client->irq)
		free_irq(client->irq, usbsw);
fail1:
	i2c_set_clientdata(client, NULL);
	kfree(usbsw);
	return ret;
}

static int __devexit fsa9480_remove(struct i2c_client *client)
{
	struct fsa9480_usbsw *usbsw = i2c_get_clientdata(client);

	if (client->irq) {
		disable_irq_wake(client->irq);
		free_irq(client->irq, usbsw);
	}
	i2c_set_clientdata(client, NULL);

	sysfs_remove_group(&client->dev.kobj, &fsa9480_group);
	kfree(usbsw);
	return 0;
}

#ifdef CONFIG_PM
static int fsa9480_resume(struct i2c_client *client)
{
	struct fsa9480_usbsw *usbsw = i2c_get_clientdata(client);
	int jack_nint = gpio_get_value(irq_to_gpio(usbsw->client->irq));

	/* If interrupt is not handled, re-check */
	if (jack_nint == 0)
		fsa9480_recheck_irq(usbsw);

	return 0;
}

#else

#define fsa9480_suspend NULL
#define fsa9480_resume NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id fsa9480_id[] = {
	{"fsa9480", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, fsa9480_id);

static struct i2c_driver fsa9480_i2c_driver = {
	.driver = {
		.name = "fsa9480",
	},
	.probe = fsa9480_probe,
	.remove = __devexit_p(fsa9480_remove),
	.resume = fsa9480_resume,
	.id_table = fsa9480_id,
};

static int __init fsa9480_init(void)
{
	return i2c_add_driver(&fsa9480_i2c_driver);
}
late_initcall(fsa9480_init);

static void __exit fsa9480_exit(void)
{
	i2c_del_driver(&fsa9480_i2c_driver);
}
module_exit(fsa9480_exit);

MODULE_AUTHOR("Minkyu Kang <mk7.kang@samsung.com>");
MODULE_DESCRIPTION("FSA9480 USB Switch driver");
MODULE_LICENSE("GPL");
