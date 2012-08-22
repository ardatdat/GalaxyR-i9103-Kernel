/*
 * arch/arm/mach-tegra/board-n1.c
 *
 * Copyright (c) 2010 - 2011, NVIDIA Corporation.
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
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/i2c-tegra.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c/mcs.h>
#include <linux/power_supply.h>

#include <linux/atmel_mxt224E.h>
#include <linux/platform_data/tegra_usb.h>

#include <linux/mfd/max8907c.h>
#include <linux/memblock.h>
#include <linux/tegra_uart.h>
#include <linux/i2c/mcs.h>
#include <linux/power_supply.h>
#include <linux/sec_jack.h>
#include <linux/fsa9480.h>
#include <linux/regulator/consumer.h>
#include <linux/reboot.h>
#include <linux/notifier.h>
#include <linux/syscalls.h>
#include <linux/vfs.h>
#include <linux/file.h>
#include <linux/console.h>

#include <mach/clk.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/pinmux.h>
#include <mach/iomap.h>
#include <mach/io.h>
#include <mach/i2s.h>
#include <mach/spdif.h>
#include <mach/audio.h>

#include <media/tegra_camera.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/usb_phy.h>
#include <mach/gpio-n1.h>
#include <mach/sec_battery.h>
#include <mach/otg_def.h>
#include <linux/power/max8922-charger.h>
#include <linux/mfd/max8907c.h>

#include "pm.h"
#include "board.h"
#include "clock.h"
#include "board-n1.h"
#include "devices.h"
#include "gpio-names.h"
#include "fuse.h"
#include "wakeups-t2.h"
#include <media/s5k6aafx.h>
#include <media/s5k4ecgx.h>
#include <linux/nct1008.h>
#include <linux/kernel_sec_common.h>
#ifdef CONFIG_SEC_MODEM
#include <mach/sec_modem.h>
#endif

#ifdef CONFIG_SAMSUNG_LPM_MODE
#include <linux/moduleparam.h>
#endif

#define HWREV_FOR_EXTERNEL_CHARGER	7

/* WLAN STATIC BUF Implementation */
#define WLAN_STATIC_BUF	// this feature for using static buffer on wifi driver /Kernel/drivers/net/wireless/bcm4330
#ifdef WLAN_STATIC_BUF
#include <linux/skbuff.h>
#define USB1_VBUS_GPIO TCA6416_GPIO_BASE
#ifdef CONFIG_USB_ANDROID_ACCESSORY
#include <linux/usb/f_accessory.h>
#endif

#ifdef CONFIG_SAMSUNG_LPM_MODE
int charging_mode_from_boot;

/* Get charging_mode status from kernel CMDLINE parameter. */
__module_param_call("", lpm_boot,  &param_ops_int,
		&charging_mode_from_boot, 0, 0644);
MODULE_PARM_DESC(charging_mode_from_boot, "Charging mode parameter value.");
#else
int charging_mode_from_boot = 0;
#endif


#define PREALLOC_WLAN_SEC_NUM		4
#define PREALLOC_WLAN_BUF_NUM		160
#define PREALLOC_WLAN_SECTION_HEADER	24


#define WLAN_SECTION_SIZE_0	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_1	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_2	(PREALLOC_WLAN_BUF_NUM * 512)
#define WLAN_SECTION_SIZE_3	(PREALLOC_WLAN_BUF_NUM * 1024)

#define WLAN_SKB_BUF_NUM	17

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];
static struct mxt_callbacks *inform_charger_callbacks;

struct wifi_mem_prealloc {
	void *mem_ptr;
	unsigned long size;
};

static struct wifi_mem_prealloc wifi_mem_array[PREALLOC_WLAN_SEC_NUM] = {
	{NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER)}
};

void *wlan_mem_prealloc(int section, unsigned long size)
{
	if (section == PREALLOC_WLAN_SEC_NUM)
		return wlan_static_skb;

	if ((section < 0) || (section > PREALLOC_WLAN_SEC_NUM))
		return NULL;

	if (wifi_mem_array[section].size < size)
		return NULL;

	return wifi_mem_array[section].mem_ptr;
}
EXPORT_SYMBOL(wlan_mem_prealloc);

#define DHD_SKB_HDRSIZE			336
#define DHD_SKB_1PAGE_BUFSIZE	((PAGE_SIZE*1)-DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE	((PAGE_SIZE*2)-DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE	((PAGE_SIZE*4)-DHD_SKB_HDRSIZE)

static int __init init_wifi_mem(void)
{
	int i;
	int j;

	for (i = 0; i < 8; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_1PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	for (; i < 16; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_2PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_4PAGE_BUFSIZE);
	if (!wlan_static_skb[i])
		goto err_skb_alloc;

	for (i = 0 ; i < PREALLOC_WLAN_SEC_NUM ; i++) {
		wifi_mem_array[i].mem_ptr =
				kmalloc(wifi_mem_array[i].size, GFP_KERNEL);

		if (!wifi_mem_array[i].mem_ptr)
			goto err_mem_alloc;
	}
	printk("%s: WIFI MEM Allocated\n", __FUNCTION__);
	return 0;

 err_mem_alloc:
	pr_err("Failed to mem_alloc for WLAN\n");
	for (j = 0 ; j < i ; j++)
		kfree(wifi_mem_array[j].mem_ptr);

	i = WLAN_SKB_BUF_NUM;

 err_skb_alloc:
	pr_err("Failed to skb_alloc for WLAN\n");
	for (j = 0 ; j < i ; j++)
		dev_kfree_skb(wlan_static_skb[j]);

	return -ENOMEM;
}

#endif /* WLAN_STATIC_BUF */

enum cable_type_t set_cable_status;
EXPORT_SYMBOL(set_cable_status);

struct class *sec_class;
EXPORT_SYMBOL(sec_class);

struct board_usb_data {
	struct mutex ldo_en_lock;
	int usb_regulator_on[3];
};

static struct board_usb_data usb_data;
struct otg_id_open_data otg_open;
struct otg_detect_data otg_clk_data;
static unsigned int *sec_batt_level;
extern int __init register_smd_resource(void);


struct bootloader_message {
	char command[32];
	char status[32];
};
/* REBOOT_MODE */
#define REBOOT_MODE_NONE                0
#define REBOOT_MODE_DOWNLOAD            1
#define REBOOT_MODE_NORMAL              2
#define REBOOT_MODE_CHARGING            3
#define REBOOT_MODE_RECOVERY            4
#define REBOOT_MODE_FOTA                5
#define REBOOT_MODE_FASTBOOT            7

#define MISC_DEVICE "/dev/block/mmcblk0p5"

static int write_bootloader_message(char *cmd, int mode)
{
	int fd;
	struct file *filp;
	mm_segment_t oldfs;
	int ret = 0;
	loff_t pos = 2048L;  /* bootloader message offset in MISC.*/

	struct bootloader_message  bootmsg;

	memset(&bootmsg, 0, sizeof(struct bootloader_message));

	if (mode == REBOOT_MODE_RECOVERY) {
		strcpy(bootmsg.command, "boot-recovery");
		kernel_sec_set_path(SEC_PORT_USB, SEC_PORT_PATH_AP);
		kernel_sec_set_path(SEC_PORT_UART, SEC_PORT_PATH_CP);
	}
	else if (mode == REBOOT_MODE_FASTBOOT)
		strcpy(bootmsg.command, "boot-fastboot");
	else if (mode == REBOOT_MODE_NORMAL)
		strcpy(bootmsg.command, "boot-reboot");
	else if (mode == REBOOT_MODE_NONE)
		strcpy(bootmsg.command, "boot-normal");
	else if (mode == REBOOT_MODE_FOTA)
		strcpy(bootmsg.command, "boot-fota");
	else
		strcpy(bootmsg.command, cmd);

	bootmsg.status[0] = (char) mode;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_open(MISC_DEVICE, O_WRONLY, 0);

	if (fd < 0) {
		printk(KERN_INFO "failed to open MISC : '%s'. %d\n", MISC_DEVICE, fd);
		return 0;
	}

	filp = fget(fd);

	ret = vfs_write(filp, (const char *)&bootmsg, sizeof(struct bootloader_message), &pos);

	if (ret < 0)
		printk(KERN_INFO "failed to write on MISC \n");
	else
		printk(KERN_INFO "command : %s written on MISC\n", bootmsg.command);

	fput(filp);
	filp_close(filp, NULL);

	set_fs(oldfs);

	return ret;
}

/* Boot Mode Physical Addresses and Magic Token */
#define BOOT_MODE_P_ADDR	(0x20000000 - 0x0C)
#define BOOT_MAGIC_P_ADDR	(0x20000000 - 0x10)
#define BOOT_MAGIC_TOKEN	0x626F6F74

static void write_bootloader_mode(char boot_mode)
{
	void __iomem *to_io;

	to_io = ioremap(BOOT_MODE_P_ADDR, 4);
	writel((unsigned long)boot_mode, to_io);
	iounmap(to_io);

	/* Write a magic value to a 2nd memory location to distinguish between a
	 * cold boot and a reboot.
	 */
	to_io = ioremap(BOOT_MAGIC_P_ADDR, 4);
	writel(BOOT_MAGIC_TOKEN, to_io);
	iounmap(to_io);
}

static int n1_notifier_call(struct notifier_block *this,
					unsigned long code, void *_cmd)
{
	int mode;

	if (code == SYS_RESTART) {
		mode = REBOOT_MODE_NORMAL;
		if (_cmd) {
			if (!strcmp((char *)_cmd, "recovery"))
				mode = REBOOT_MODE_RECOVERY;
			else if (!strcmp((char *)_cmd, "arm11_fota"))
				mode = REBOOT_MODE_FOTA;
			else if (!strcmp((char *)_cmd, "bootloader"))
				mode = REBOOT_MODE_FASTBOOT;
			else if (!strcmp((char *)_cmd, "download"))
				mode = REBOOT_MODE_DOWNLOAD;
		}
	} else
		mode = REBOOT_MODE_NONE;

	pr_info("%s, Reboot Mode : %d \n", __func__, mode);

	switch( mode )
	{
		case REBOOT_MODE_FOTA:
		case REBOOT_MODE_NORMAL:
		case REBOOT_MODE_RECOVERY:
			write_bootloader_mode(mode);
	}

	write_bootloader_message(_cmd, mode);

	return NOTIFY_DONE;
}

static struct notifier_block n1_reboot_notifier = {
	.notifier_call = n1_notifier_call,
};
static struct plat_serial8250_port debug_uartb_platform_data[] = {
	{
		.membase	= IO_ADDRESS(TEGRA_UARTB_BASE),
		.mapbase	= TEGRA_UARTB_BASE,
		.irq		= INT_UARTB,
		.flags		= UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE,
		.type           = PORT_TEGRA,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= 216000000,
	}, {
		.flags		= 0,
	}
};

static struct platform_device debug_uartb = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = debug_uartb_platform_data,
	},
};

static struct platform_device *n1_uart_devices[] __initdata = {
	&tegra_uarta_device,
	&tegra_uartb_device,
	&tegra_uartc_device,
};

struct uart_clk_parent uart_parent_clk[] = {
	[0] = {.name = "pll_p"},
	[1] = {.name = "pll_m"},
	[2] = {.name = "clk_m"},
};

static struct tegra_uart_platform_data n1_uart_pdata;

static void __init uart_debug_init(void)
{
	unsigned long rate;
	struct clk *debug_uart_clk;
	struct clk *c;

	/* UARTB is the debug port. */
	pr_info("Selecting UARTB as the debug console\n");
	n1_uart_devices[1] = &debug_uartb;
	debug_uart_clk = clk_get_sys("serial8250.0", "uartb");

	/* Clock enable for the debug channel */
	if (!IS_ERR_OR_NULL(debug_uart_clk)) {
		rate = debug_uartb_platform_data[0].uartclk;
		pr_info("The debug console clock name is %s\n",
					debug_uart_clk->name);
		c = tegra_get_clock_by_name("pll_p");
		if (IS_ERR_OR_NULL(c))
			pr_err("Not getting the parent clock pll_p\n");
		else
			clk_set_parent(debug_uart_clk, c);
			clk_enable(debug_uart_clk);
		clk_set_rate(debug_uart_clk, rate);
	} else {
		pr_err("Not getting the clock %s for debug console\n",
					debug_uart_clk->name);
	}
}

#ifdef CONFIG_BT_BCM4330
static struct platform_device bcm4330_bluetooth_device = {
	.name = "bcm4330_bluetooth",
	.id = -1,
};

/* UART Interface for Bluetooth */
static struct tegra_uart_platform_data bt_uart_pdata = {
	.wake_peer = bcm_bt_lpm_exit_lpm_locked,
};
#endif

static void __init n1_uart_init(void)
{
	int i;
	struct clk *c;

	for (i = 0; i < ARRAY_SIZE(uart_parent_clk); ++i) {
		c = tegra_get_clock_by_name(uart_parent_clk[i].name);
		if (IS_ERR_OR_NULL(c)) {
			pr_err("Not able to get the clock for %s\n",
						uart_parent_clk[i].name);
			continue;
		}
		uart_parent_clk[i].parent_clk = c;
		uart_parent_clk[i].fixed_clk_rate = clk_get_rate(c);
	}
	n1_uart_pdata.parent_clk_list = uart_parent_clk;
	n1_uart_pdata.parent_clk_count = ARRAY_SIZE(uart_parent_clk);

	tegra_uarta_device.dev.platform_data = &n1_uart_pdata;
	tegra_uartb_device.dev.platform_data = &n1_uart_pdata;
#ifdef CONFIG_BT_BCM4330
	tegra_uartc_device.dev.platform_data = &bt_uart_pdata;
#else
	tegra_uartc_device.dev.platform_data = &n1_uart_pdata;
#endif

	if (!is_tegra_debug_uartport_hs())
		uart_debug_init();

	platform_add_devices(n1_uart_devices,
				ARRAY_SIZE(n1_uart_devices));
}

#ifdef CONFIG_BCM4330_RFKILL
/* Bluetooth(BCM4330) rfkill */
static struct resource n1_bcm4330_rfkill_resources[] = {
	{
		.name   = "bcm4330_nreset_gpio",
		.start  = GPIO_BT_nRST,
		.end    = GPIO_BT_nRST,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "bcm4330_nshutdown_gpio",
		.start  = GPIO_BT_EN,
		.end    = GPIO_BT_EN,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "bcm4330_btwake_gpio",
		.start  = GPIO_BT_WAKE,
		.end    = GPIO_BT_WAKE,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "bcm4330_hostwake_gpio",
		.start  = TEGRA_GPIO_TO_IRQ(GPIO_BT_HOST_WAKE),
		.end    = TEGRA_GPIO_TO_IRQ(GPIO_BT_HOST_WAKE),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device n1_bcm4330_rfkill_device = {
	.name = "bcm4330_rfkill",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(n1_bcm4330_rfkill_resources),
	.resource       = n1_bcm4330_rfkill_resources,
};

int __init n1_rfkill_init(void)
{
	/*Add Clock Resource*/
	clk_add_alias("bcm4330_32k_clk", n1_bcm4330_rfkill_device.name, \
				"blink", NULL);

	tegra_gpio_enable(GPIO_BT_HOST_WAKE);
	platform_device_register(&n1_bcm4330_rfkill_device);

	return 0;
}
#endif

static struct tegra_utmip_config utmi_phy_config[] = {
	[0] = {
			.hssync_start_delay = 9,
			.idle_wait_delay = 17,
			.elastic_limit = 16,
			.term_range_adj = 6,
			.xcvr_setup = 11,
			.xcvr_setup_offset = 0,
			.xcvr_use_fuses = 1,
			.xcvr_lsfslew = 2,
			.xcvr_lsrslew = 2,
		},
	[1] = {
			.hssync_start_delay = 9,
			.idle_wait_delay = 17,
			.elastic_limit = 16,
			.term_range_adj = 6,
			.xcvr_setup = 8,
			.xcvr_setup_offset = 0,
			.xcvr_use_fuses = 1,
			.xcvr_lsfslew = 2,
			.xcvr_lsrslew = 2,
		},
};

#ifdef CONFIG_LINK_DEVICE_HSIC
static int notify_hsic_host_ready(void)
{
	set_host_states(&tegra_ehci2_device, TEGRA_HOST_ON);
	return 0;
}
static int notify_hsic_host_off(void)
{
	set_host_states(&tegra_ehci2_device, TEGRA_HOST_OFF);
	return 0;
}

static struct tegra_uhsic_config hsic_phy_config = {
	.enable_gpio = GPIO_HSIC_EN,
	.reset_gpio = -1,
	.sync_start_delay = 9,
	.idle_wait_delay = 17,
	.term_range_adj = 0,
	.elastic_underrun_limit = 16,
	.elastic_overrun_limit = 16,
	.usb_phy_ready = notify_hsic_host_ready,
	.post_phy_off = notify_hsic_host_off,
	.device_wake = set_slave_wake,
};
#else
static struct tegra_ulpi_config ulpi_phy_config = {
	.reset_gpio = TEGRA_GPIO_PG2,
	.clk = "cdev2",
};
#endif

static struct resource ram_console_resource[] = {
	{
		.flags = IORESOURCE_MEM,
	}
};

static struct platform_device ram_console_device = {
	.name = "ram_console",
	.id = -1,
	.num_resources = ARRAY_SIZE(ram_console_resource),
	.resource = ram_console_resource,
};
static __initdata struct tegra_clk_init_table n1_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "uartb",	"pll_p",	216000000,	true},
	{ "uartc",	"pll_m",	600000000,	false},

	{ "pll_p_out4",	"pll_p",	24000000,	true},
	{ "pwm",	"clk_32k",	32768,		false},
	{ "kbc",	"clk_32k",	32768,		true},
	{ "sdmmc2",	"pll_p",	25000000,	false},
	{ "i2s1",	"pll_a_out0",	0,		false},
	{ "i2s2",	"pll_a_out0",	0,		false},
	{ "spdif_out",	"pll_a_out0",	0,		false},
	{ NULL,		NULL,		0,		0},
};

static void sec_jack_set_micbias_state(bool on)
{
	printk(KERN_INFO "Board N1 : %s (on %d)\n", __func__, on);
	wm8994_set_sub_mic_bias(on);
}

static struct sec_jack_zone sec_jack_zones[] = {
	{
		/* adc == 0, 3 pole */
		.adc_high = 0,
		.delay_ms = 0,
		.check_count = 0,
		.jack_type = SEC_HEADSET_3POLE,
	},
	{
		/* 0 < adc <= 737, unstable zone, default to 3pole if it stays
		 * in this range for a 200ms (20ms delays, 10 samples)
		 */
		.adc_high = 737,
		.delay_ms = 20,
		.check_count = 10,
		.jack_type = SEC_HEADSET_3POLE,
	},
	{
		/* 737 < adc <= 2000, unstable zone, default to 3pole if it
		 * stays in this range for a second (10ms delays, 100 samples)
		 */
		.adc_high = 2000,
		.delay_ms = 0,
		.check_count = 5,
		.jack_type = SEC_HEADSET_4POLE,
	},
	{
		/* 2000 < adc <= 3700, 4 pole zone */
		.adc_high = 3700,
		.delay_ms = 0,
		.check_count = 10,
		.jack_type = SEC_HEADSET_4POLE,
	},
	{
		/* adc > 3700, unstable zone, default to 3pole if it stays
		 * in this range for a second (10ms delays, 100 samples)
		 */
		.adc_high = 0x7fffffff,
		.delay_ms = 10,
		.check_count = 100,
		.jack_type = SEC_HEADSET_3POLE,
	},
};


/* To support 3-buttons earjack */
static struct sec_jack_buttons_zone sec_jack_buttons_zones[] = {
	{
		/* 0 <= adc <=114, stable zone */
		.code		= KEY_MEDIA,
		.adc_low		= 0,
		.adc_high		= 114,
	},
	{
		/* 115 <= adc <= 254, stable zone */
		.code		= KEY_VOLUMEUP,
		.adc_low		= 115,
		.adc_high		= 254,
	},
	{
		/* 255 <= adc <= 520, stable zone */
		.code		= KEY_VOLUMEDOWN,
		.adc_low		= 255,
		.adc_high		= 520,
	},
};

static int sec_jack_get_adc_value(void)
{
	int mili_volt;
	max8907c_adc_read_aux2(&mili_volt);
	printk(KERN_DEBUG
		"Board N1 : Enterring %s = %d\n", __func__, mili_volt);
	return  mili_volt;
}

struct sec_jack_platform_data sec_jack_pdata = {
	.set_micbias_state = sec_jack_set_micbias_state,
	.get_adc_value = sec_jack_get_adc_value,
	.zones = sec_jack_zones,
	.num_zones = ARRAY_SIZE(sec_jack_zones),
	.buttons_zones = sec_jack_buttons_zones,
	.num_buttons_zones = ARRAY_SIZE(sec_jack_buttons_zones),
	.det_gpio = GPIO_DET_3_5,
	.send_end_gpio = GPIO_EAR_SEND_END,
	.det_active_high = 0,
	.send_end_active_high = 0,
	.jack_status = SEC_JACK_NO_DEVICE,
};

static struct platform_device sec_device_jack = {
	.name			= "sec_jack",
	.id			= 1, /* will be used also for gpio_event id */
	.dev.platform_data	= &sec_jack_pdata,
};


static struct tegra_ulpi_config n1_ehci2_ulpi_phy_config = {
	.reset_gpio = TEGRA_GPIO_PV1,
	.clk = "clk_dev2",
};

static struct tegra_ehci_platform_data n1_ehci2_ulpi_platform_data = {
	.operating_mode = TEGRA_USB_HOST,
	.power_down_on_bus_suspend = 0,
	.phy_config = &n1_ehci2_ulpi_phy_config,
};

static struct tegra_i2c_platform_data n1_i2c1_platform_data = {
	.adapter_nr	= 0,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PC4, 0},
	.sda_gpio		= {TEGRA_GPIO_PC5, 0},
	.arb_recovery = arb_lost_recovery,
};

static const struct tegra_pingroup_config i2c2_ddc = {
	.pingroup	= TEGRA_PINGROUP_DDC,
	.func		= TEGRA_MUX_I2C2,
};

static const struct tegra_pingroup_config i2c2_gen2 = {
	.pingroup	= TEGRA_PINGROUP_PTA,
	.func		= TEGRA_MUX_I2C2,
};

static struct tegra_i2c_platform_data n1_i2c2_platform_data = {
	.adapter_nr	= 1,
	.bus_count	= 2,
	.bus_clk_rate	= { 400000, 400000 },
	.bus_mux	= { &i2c2_ddc, &i2c2_gen2 },
	.bus_mux_len	= { 1, 1 },
	.scl_gpio		= {0, TEGRA_GPIO_PT5},
	.sda_gpio		= {0, TEGRA_GPIO_PT6},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data n1_i2c3_platform_data = {
	.adapter_nr	= 3,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio		= {TEGRA_GPIO_PBB2, 0},
	.sda_gpio		= {TEGRA_GPIO_PBB3, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_i2c_platform_data n1_dvc_platform_data = {
	.adapter_nr	= 4,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.is_dvc		= true,
	.scl_gpio		= {TEGRA_GPIO_PZ6, 0},
	.sda_gpio		= {TEGRA_GPIO_PZ7, 0},
	.arb_recovery = arb_lost_recovery,
};

static struct tegra_audio_platform_data tegra_audio_pdata[] = {
	/* For I2S1 */
	[0] = {
		.i2s_master	= true,
		.dma_on		= true,  /* use dma by default */
		.i2s_master_clk = 44100,
		.i2s_clk_rate	= 2822400,
		.dap_clk	= "clk_dev1",
		.audio_sync_clk = "audio_2x",
		.mode		= I2S_BIT_FORMAT_I2S,
		.fifo_fmt	= I2S_FIFO_PACKED,
		.bit_size	= I2S_BIT_SIZE_16,
		.i2s_bus_width = 32,
		.dsp_bus_width = 16,
	},
	/* For I2S2 */
	[1] = {
		.i2s_master	= true,
		.dma_on		= true,  /* use dma by default */
		.i2s_master_clk = 8000,
		.dsp_master_clk = 8000,
		.i2s_clk_rate	= 2000000,
		.dap_clk	= "clk_dev1",
		.audio_sync_clk = "audio_2x",
		.mode		= I2S_BIT_FORMAT_DSP,
		.fifo_fmt	= I2S_FIFO_16_LSB,
		.bit_size	= I2S_BIT_SIZE_16,
		.i2s_bus_width = 32,
		.dsp_bus_width = 16,
	}
};

#if 0
// N1_ICS
static struct tegra_das_platform_data tegra_das_pdata = {
	.tegra_dap_port_info_table = {
		[0] = {
			.dac_port = tegra_das_port_none,
			.codec_type = tegra_audio_codec_type_none,
			.device_property = {
				.num_channels = 0,
				.bits_per_sample = 0,
				.rate = 0,
				.dac_dap_data_comm_format = 0,
			},
		},
		/* I2S1 <--> DAC1 <--> DAP1 <--> Hifi Codec */
		[1] = {
			.dac_port = tegra_das_port_i2s1,
			.codec_type = tegra_audio_codec_type_hifi,
			.device_property = {
				.num_channels = 2,
				.bits_per_sample = 16,
				.rate = 44100,
				.dac_dap_data_comm_format = dac_dap_data_format_i2s,
			},
		},
		[2] = {
			.dac_port = tegra_das_port_none,
			.codec_type = tegra_audio_codec_type_none,
			.device_property = {
				.num_channels = 0,
				.bits_per_sample = 0,
				.rate = 0,
				.dac_dap_data_comm_format = 0,
			},
		},
		[3] = {
			.dac_port = tegra_das_port_none,
			.codec_type = tegra_audio_codec_type_none,
			.device_property = {
				.num_channels = 0,
				.bits_per_sample = 0,
				.rate = 0,
				.dac_dap_data_comm_format = 0,
			},
		},
		[4] = {
			.dac_port = tegra_das_port_none,
			.codec_type = tegra_audio_codec_type_none,
			.device_property = {
				.num_channels = 0,
				.bits_per_sample = 0,
				.rate = 0,
				.dac_dap_data_comm_format = 0,
			},
		},
	},

	.tegra_das_con_table = {
		[0] = {
			.con_id = tegra_das_port_con_id_hifi,
			.num_entries = 4,
			.con_line = {
				[0] = {tegra_das_port_i2s1, tegra_das_port_dap1, true},
				[1] = {tegra_das_port_dap1, tegra_das_port_i2s1, false},
				[2] = {tegra_das_port_i2s2, tegra_das_port_dap4, true},
				[3] = {tegra_das_port_dap4, tegra_das_port_i2s2, false},
			},
		},
	}
};
#endif
static struct platform_device tegra_audio_device = {
	.name	= "tegra-snd-wm8994",
	.id	= 0,
};

static void n1_i2c_init(void)
{
	tegra_i2c_device1.dev.platform_data = &n1_i2c1_platform_data;
	tegra_i2c_device2.dev.platform_data = &n1_i2c2_platform_data;
	tegra_i2c_device3.dev.platform_data = &n1_i2c3_platform_data;
	tegra_i2c_device4.dev.platform_data = &n1_dvc_platform_data;

	platform_device_register(&tegra_i2c_device1);
	platform_device_register(&tegra_i2c_device2);
	platform_device_register(&tegra_i2c_device3);
	platform_device_register(&tegra_i2c_device4);
}

#define TEGRA_GPIO_NULL		255

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_BIT(x)		((x) & 0x7)

#define GPIO_REG(x)		(IO_TO_VIRT(TEGRA_GPIO_BASE) +	\
				 GPIO_BANK(x) * 0x80 +		\
				 GPIO_PORT(x) * 4)
#define GPIO_IN(x)		(GPIO_REG(x) + 0x30)

static int n1_tegra_gpio_get(unsigned offset)
{
	return (__raw_readl(GPIO_IN(offset)) >> GPIO_BIT(offset)) & 0x1;
}

#ifdef CONFIG_KEYBOARD_GPIO
#define GPIO_KEY(_id, _gpio, _iswake, _act_low)		\
	{					\
		.code = _id,			\
		.gpio = _gpio,	\
		.active_low = _act_low,		\
		.desc = #_id,			\
		.type = EV_KEY,			\
		.wakeup = _iswake,		\
		.debounce_interval = 10,	\
	}

static struct gpio_keys_button n1_keys[] = {
	[0] = GPIO_KEY(KEY_HOME, GPIO_HOME_KEY, 1, 1),
	[1] = GPIO_KEY(KEY_POWER, GPIO_EXT_WAKEUP, 1, 0),
	[2] = GPIO_KEY(KEY_VOLUMEUP, GPIO_VOL_UP, 0, 1),
	[3] = GPIO_KEY(KEY_VOLUMEDOWN, GPIO_VOL_DOWN, 0, 1),
};

static bool gpio_keys_wakeup;
static int gpio_keys_lp_state;
/* index of n1_keys array */
static int gpio_keys_wake_lp2[] = {0, 1, 2, 3};
/* save gpio state from lp2 state */
static int gpio_keys_wake_state[ARRAY_SIZE(gpio_keys_wake_lp2)];
/* save wakeup status register */
static u32 gpio_keys_wakeup_status;
static int gpio_keys_wake_state2[ARRAY_SIZE(gpio_keys_wake_lp2)];

#define PMC_WAKE_STATUS		0x14

void n1_save_wakeup_key(int lp_state)
{
	int state;
	int i;
	int idx;

	gpio_keys_wakeup = true;
	gpio_keys_lp_state = lp_state;


	if (lp_state == TEGRA_SUSPEND_LP2 || lp_state == TEGRA_SUSPEND_LP1) {
		/* save gpio state */
		for (i = 0; i < ARRAY_SIZE(gpio_keys_wake_lp2); i++) {
			idx = gpio_keys_wake_lp2[i];
			state = (n1_tegra_gpio_get(n1_keys[idx].gpio) ? 1 : 0) ^ n1_keys[idx].active_low;
			pr_debug("[%s] %d:%d\n", __func__, n1_keys[idx].code, !!state);
			gpio_keys_wake_state[i] = state;
			gpio_keys_wake_state2[i] = 0;
		}
	} else {
		gpio_keys_wakeup_status =
			readl(IO_ADDRESS(TEGRA_PMC_BASE) + PMC_WAKE_STATUS);
	}
}

/* caution: this function has some hard-coded value */
/* if key environment is modifed, this function has to be re-written */
void n1_check_key_pressed(void)
{
	int i;
	int state;

	if (gpio_keys_wakeup && (gpio_keys_lp_state == TEGRA_SUSPEND_LP2 ||\
			gpio_keys_lp_state == TEGRA_SUSPEND_LP1)) {
		/* volume up key */
		i = 2;
		if (gpio_keys_wake_state[i] && !gpio_keys_wake_state2[i]) {
			state  = !n1_tegra_gpio_get(GPIO_VOL_UP);
			if(!state)
				gpio_keys_wake_state2[i] = 1;
			pr_debug("[%s] %d:%d->%d\n", __func__, KEY_VOLUMEUP, state, gpio_keys_wake_state2[i]);
		}
		/* volume down key */
		i = 3;
		if (gpio_keys_wake_state[i] && !gpio_keys_wake_state2[i]) {
			state  = !n1_tegra_gpio_get(GPIO_VOL_DOWN);
			if(!state)
				gpio_keys_wake_state2[i] = 1;
			pr_debug("[%s] %d:%d->%d\n", __func__, KEY_VOLUMEDOWN, state, gpio_keys_wake_state2[i]);
		}
	}
}

/* Only key resume code can use this function */
/* Because this function return correct key code at one time after wakeup */
static int n1_wakeup_key(void)
{
	int i;
	int idx;
	int ret= KEY_RESERVED;

	if(!gpio_keys_wakeup)
		ret = 0;
	else {
		if (gpio_keys_lp_state == TEGRA_SUSPEND_LP2 ||\
				gpio_keys_lp_state == TEGRA_SUSPEND_LP1) {
			/* this function returns only one key code.
			** if key is missed on wakeup, please review this code.
			*/
			for (i = 0; i < ARRAY_SIZE(gpio_keys_wake_lp2); i++) {
				idx = gpio_keys_wake_lp2[i];
				if (gpio_keys_wake_state[i]) {
					ret = n1_keys[idx].code;
					break;
				}
			}
		} else {
			if (gpio_keys_wakeup_status & TEGRA_WAKE_GPIO_PU5) {
				ret = KEY_POWER;
			} else if (gpio_keys_wakeup_status & TEGRA_WAKE_GPIO_PO5) {
				ret = KEY_HOME;
			} else
				ret = KEY_RESERVED;
		}
		pr_debug("[%s] %d wakeup from lp%d\n", __func__, ret, gpio_keys_lp_state);
	}

	gpio_keys_wakeup = false;

	return ret;
}

/* This can find a volume key pressed twice */
static int n1_wakeup_key_twice(void)
{
	int i;
	int idx;
	int ret= KEY_RESERVED;

	for (i = 0; i < ARRAY_SIZE(gpio_keys_wake_lp2); i++) {
		idx = gpio_keys_wake_lp2[i];
		if (gpio_keys_wake_state2[i]) {
			ret = n1_keys[idx].code;
			break;
		}
	}

	return ret;
}

bool n1_check_lpm(void)
{
	return charging_mode_from_boot ? true : false;
}
EXPORT_SYMBOL(n1_check_lpm);

static struct gpio_keys_platform_data n1_keys_platform_data = {
	.buttons	= n1_keys,
	.nbuttons	= ARRAY_SIZE(n1_keys),
	.wakeup_key	= n1_wakeup_key,
	.wakeup_key_twice	= n1_wakeup_key_twice,
#ifdef CONFIG_SAMSUNG_LPM_MODE
	.check_lpm = n1_check_lpm,
#endif
};

static struct platform_device n1_keys_device = {
	.name	= "sec_key",
	.id	= 0,
	.dev	= {
		.platform_data	= &n1_keys_platform_data,
	},
};

static void n1_keys_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(n1_keys); i++)
		tegra_gpio_enable(n1_keys[i].gpio);

}
#endif /* CONFIG_KEYBOARD_GPIO */

static struct platform_device tegra_camera = {
	.name = "tegra_camera",
	.id = -1,
};



#if defined (CONFIG_MACH_BOSE_ATT)
static int adc_result;
#define ADC_DATA_ARR_SIZE 6
int sec_BAT_VF_get_adc_value(void)
{
	int adc_data;
	int adc_max = 0;
	int adc_min = 0;
	int adc_total = 0;
	int i;


	for (i = 0; i < ADC_DATA_ARR_SIZE; i++)
	{
		max8907c_adc_read_aux1(&adc_data);

		if (i != 0) {
			if (adc_data > adc_max)
				adc_max = adc_data;
			else if (adc_data < adc_min)
				adc_min = adc_data;
		} else {
			adc_max = adc_data;
			adc_min = adc_data;
		}
		adc_total += adc_data;
	}

	        adc_result = (adc_total - adc_max - adc_min) / (ADC_DATA_ARR_SIZE - 2);

	return adc_result;
}
#endif
#ifdef CONFIG_BATTERY_SEC

static void sec_bat_get_init_cable_state(struct power_supply *psy)
{
	union power_supply_propval value;
	/* if there was a cable status change before the charger was
	ready, send this now */
	printk("%s : set_cable_status(%d)\n", __func__, set_cable_status);

	if (!psy) {
		pr_err("%s: fail to get battery ps\n", __func__);
		return;
	}
	if (set_cable_status == CABLE_TYPE_NONE) {
		value.intval = POWER_SUPPLY_TYPE_BATTERY;
		psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	} else if (set_cable_status == CABLE_TYPE_USB) {
		value.intval = POWER_SUPPLY_TYPE_USB;
		psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	} else if (set_cable_status == CABLE_TYPE_AC) {
		value.intval = POWER_SUPPLY_TYPE_MAINS;
		psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	} else if (set_cable_status == CABLE_TYPE_DOCK) { /* Dock and Charger (ta or usb) is attached */
		value.intval = POWER_SUPPLY_TYPE_DOCK;
		psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	}
}

extern struct nct1008_temp_callbacks *callbacks;

static int sec_bat_get_temperature(void)
{
	int batt_temp = 25;

	if (callbacks && callbacks->get_temperature)
		batt_temp = callbacks->get_temperature(callbacks);
	return batt_temp;
}

static void sec_bat_get_level(unsigned int *batt_soc)
{
	sec_batt_level = batt_soc;
}

static struct sec_bat_platform_data sec_bat_pdata = {
	.fuel_gauge_name	= "fuelgauge",
	.charger_name		= "max8907c-charger",
#ifdef CONFIG_MAX8922_CHARGER
	.sub_charger_name	= "max8922-charger",
#endif
	.adc_arr_size		= NULL,
	.adc_table			= NULL,
	.adc_channel		= NULL,
	.get_init_cable_state	= sec_bat_get_init_cable_state,
	.get_temperature	= sec_bat_get_temperature,
	.get_batt_level	        = sec_bat_get_level,
};

static struct platform_device sec_device_battery = {
	.name = "sec-battery",
	.id = -1,
	.dev.platform_data = &sec_bat_pdata,
};
#endif /* CONFIG_BATTERY_SEC */

#ifdef CONFIG_MAX8922_CHARGER
static int max8922_cfg_gpio(struct max8922_platform_data *pdata)
{
	if (system_rev < HWREV_FOR_EXTERNEL_CHARGER)
		return -ENODEV;

	tegra_gpio_enable(pdata->gpio_chg_en);
	gpio_request(pdata->gpio_chg_en, "chg_en");
//20110517_HDLNC_PMIC
#if !defined (CONFIG_MACH_BOSE_ATT)

	gpio_direction_output(pdata->gpio_chg_en, 0);
#endif

	tegra_gpio_enable(pdata->gpio_chg_ing);
	gpio_request(pdata->gpio_chg_ing, "chg_ing");
	gpio_direction_input(pdata->gpio_chg_ing);

	tegra_gpio_enable(pdata->gpio_ta_nconnected);
	gpio_request(pdata->gpio_ta_nconnected, "ta_nconnected");
	gpio_direction_input(pdata->gpio_ta_nconnected);

	return 0;
}

static int max8922_charger_topoff_cb(void)
{
	struct power_supply *psy = power_supply_get_by_name("battery");
	union power_supply_propval value;

	if (!psy) {
		pr_err("%s: Max8907c-Charger : Fail to get battery ps\n", __func__);
		return -ENODEV;
	}

	value.intval = POWER_SUPPLY_STATUS_FULL;
	return psy->set_property(psy, POWER_SUPPLY_PROP_STATUS, &value);
}

static struct max8922_platform_data max8922_pdata = {
	.topoff_cb = max8922_charger_topoff_cb,
	.cfg_gpio = max8922_cfg_gpio,
	.gpio_chg_en = GPIO_CHG_EN,
	.gpio_chg_ing = GPIO_CHG_ING_N,
	.gpio_ta_nconnected = GPIO_TA_nCONNECTED,
};

static struct platform_device max8922_device_charger = {
	.name = "max8922-charger",
	.id = -1,
	.dev.platform_data = &max8922_pdata,
};
#endif /* CONFIG_MAX8922_CHARGER */
static struct platform_device *n1_devices[] __initdata = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	&s3c_device_rndis,
#endif
#ifdef CONFIG_USB_ANDROID
	&s3c_device_android_usb,
	&s3c_device_usb_mass_storage,
#endif
	&tegra_pmu_device,
	&tegra_udc_device,
	&tegra_gart_device,
#ifdef CONFIG_KEYBOARD_GPIO
	&n1_keys_device,
#endif
	/* &tegra_wdt_device, */
	&tegra_avp_device,
	&tegra_camera,
	&sec_device_jack,
	&tegra_i2s_device1,
	&tegra_i2s_device2,
	&tegra_spdif_device,
#ifdef CONFIG_BATTERY_SEC
	&sec_device_battery,
#endif
	&tegra_das_device,
	&spdif_dit_device,
	&bluetooth_dit_device,
#ifdef CONFIG_BT_BCM4330
	&bcm4330_bluetooth_device,
#endif
	&baseband_dit_device,
	&ram_console_device,

	&tegra_pcm_device,
	&tegra_audio_device,
};

/*n1 touch : atmel_mxt224E*/
static void n1_touch_init_hw(struct mxt_platform_data *pdata)
{
	int error;
	pr_info("(%s,%d)\n", __func__, __LINE__);

	/* TSP INT GPIO initialize */
	tegra_gpio_enable(pdata->irq_gpio);
	error = gpio_request(pdata->irq_gpio, "tsp_int");
	if (error != 0) {
		pr_err("tsp_int request FAIL error = %d", error);
	}
	gpio_direction_input(pdata->irq_gpio);

	/* TouchKey LED LDO_EN GPIO initialize */
	if (system_rev <= 9) {
		pdata->key_led_en1 = GPIO_LED_LDO_EN1;	/* Menu */
		pdata->key_led_en2 = GPIO_LED_LDO_EN2;	/* Back */

		error = gpio_request(pdata->key_led_en1, "tsp_key_led1");
		if (error == 0) {
		tegra_gpio_enable(pdata->key_led_en1);
			gpio_direction_output(pdata->key_led_en1, 0);
		} else {
			pr_err("tsp_menu_key_led GPIO(%d) request FAIL error = %d", pdata->key_led_en1, error);
	}

		error = gpio_request(pdata->key_led_en2, "tsp_key_led2");
		if (error == 0) {
		tegra_gpio_enable(pdata->key_led_en2);
			gpio_direction_output(pdata->key_led_en2, 0);
		} else {
			pr_err("tsp_menu_key_led GPIO(%d) request FAIL error = %d", pdata->key_led_en2, error);
	}
	} else if (system_rev == 10) {		/* board rev.10, touch key is 4 key array, 4 LED */
		pdata->key_led_en1 = GPIO_LED_LDO_EN1;	/* Menu */
		pdata->key_led_en2 = GPIO_LED_LDO_EN2;	/* Home */
		pdata->key_led_en3 = GPIO_LED_LDO_EN3;	/* Back */
		pdata->key_led_en4 = GPIO_LED_LDO_EN4;	/* Search */

		error = gpio_request(pdata->key_led_en1, "tsp_key_led1");
		if (error == 0) {
		tegra_gpio_enable(pdata->key_led_en1);
			gpio_direction_output(pdata->key_led_en1, 0);
		} else {
			pr_err("tsp_menu_key_led GPIO(%d) request FAIL error = %d", pdata->key_led_en1, error);
	}

		error = gpio_request(pdata->key_led_en2, "tsp_key_led2");
		if (error == 0) {
			tegra_gpio_enable(pdata->key_led_en2);
			gpio_direction_output(pdata->key_led_en2, 0);
		} else {
			pr_err("tsp_menu_key_led GPIO(%d) request FAIL error = %d", pdata->key_led_en2, error);
		}

		error = gpio_request(pdata->key_led_en3, "tsp_key_led3");
		if (error == 0) {
			tegra_gpio_enable(pdata->key_led_en3);
			gpio_direction_output(pdata->key_led_en3, 0);
		} else {
			pr_err("tsp_menu_key_led GPIO(%d) request FAIL error = %d", pdata->key_led_en3, error);
		}

		error = gpio_request(pdata->key_led_en4, "tsp_key_led4");
		if (error == 0) {
			tegra_gpio_enable(pdata->key_led_en4);
			gpio_direction_output(pdata->key_led_en4, 0);
		} else {
			pr_err("tsp_menu_key_led GPIO(%d) request FAIL error = %d", pdata->key_led_en4, error);
	}
	} else if (system_rev >= 11) {		/* since board rev.11, touch key is 4 key array, 1 LED */
		pdata->key_led_en1 = GPIO_LED_LDO_EN1;	/* Menu */

		error = gpio_request(pdata->key_led_en1, "tsp_key_led1");
		if (error == 0) {
		tegra_gpio_enable(pdata->key_led_en1);
			gpio_direction_output(pdata->key_led_en1, 0);
		} else {
			pr_err("tsp_menu_key_led GPIO(%d) request FAIL error = %d", pdata->key_led_en1, error);
		}
	}
	pdata->board_rev = system_rev;
}

static void n1_touch_exit_hw(struct mxt_platform_data *pdata)
{
	pr_info("(%s,%d)\n", __func__, __LINE__);
	gpio_free(pdata->irq_gpio);
	gpio_free(pdata->key_led_en1);
	if (pdata->key_led_en2 != NULL)
	gpio_free(pdata->key_led_en2);
	if (pdata->key_led_en3 != NULL)
		gpio_free(pdata->key_led_en3);
	if (pdata->key_led_en4 != NULL)
		gpio_free(pdata->key_led_en4);

	tegra_gpio_disable(pdata->irq_gpio);
	tegra_gpio_disable(pdata->key_led_en1);
	if (pdata->key_led_en2 != NULL)
	tegra_gpio_disable(pdata->key_led_en2);
	if (pdata->key_led_en3 != NULL)
		tegra_gpio_disable(pdata->key_led_en3);
	if (pdata->key_led_en4 != NULL)
		tegra_gpio_disable(pdata->key_led_en4);
}

static void n1_touch_suspend_hw(struct mxt_platform_data *pdata)
{
	pr_info("(%s,%d)\n", __func__, __LINE__);
	/* first power off (off sequence: avdd -> vdd) */
	regulator_disable(pdata->reg_avdd);
	regulator_disable(pdata->reg_vdd_lvsio);
	regulator_disable(pdata->reg_vdd);

	/*and then, pin configuration */
	gpio_direction_output(pdata->irq_gpio, 0);
	tegra_pinmux_set_pullupdown(pdata->i2c_pingroup, pdata->i2c_suspend_pupd);
	pr_info("[TSP] Power Off!!");
}

static void n1_touch_resume_hw(struct mxt_platform_data *pdata)
{
	pr_info("(%s,%d)\n", __func__, __LINE__);
	/* first pin configuration */
	gpio_direction_input(pdata->irq_gpio);
	tegra_pinmux_set_pullupdown(pdata->i2c_pingroup, pdata->i2c_resume_pupd);

	/* and then, power on (on sequence: vdd -> avdd) */
	regulator_enable(pdata->reg_vdd);
	regulator_enable(pdata->reg_vdd_lvsio);
	regulator_enable(pdata->reg_avdd);
	msleep(100);
	pr_info("[TSP] Power On!!");

}

void n1_inform_charger_connection(int mode)
{
	if (inform_charger_callbacks &&
				inform_charger_callbacks->inform_charger)
		inform_charger_callbacks->inform_charger(inform_charger_callbacks, mode);
};
EXPORT_SYMBOL(n1_inform_charger_connection);

static void n1_register_touch_callbacks(struct mxt_callbacks *cb)
{
	 inform_charger_callbacks = cb;
}

static struct mxt_platform_data n1_mxt224E_platform_data = {
	.platform_name = "mxt224E TSP",
	.numtouch = 10,
	.max_x = 480,
	.max_y = 800,
	.init_platform_hw = n1_touch_init_hw,
	.exit_platform_hw = n1_touch_exit_hw,
	.suspend_platform_hw = n1_touch_suspend_hw,
	.resume_platform_hw = n1_touch_resume_hw,
	.register_cb = n1_register_touch_callbacks,

	.key_led_en1 = NULL,
	.key_led_en2 = NULL,
	.key_led_en3 = NULL,
	.key_led_en4 = NULL,

	.reg_vdd_name = "TSP_VDD_1V8",
	.reg_vdd = NULL,
	.reg_vdd_level = 1800000,

	.reg_avdd_name = "TSP_AVDD_3V3",
	.reg_avdd = NULL,
	.reg_avdd_level = 3300000,

	.reg_vdd_lvsio_name = "TSP_VDD_LVSIO",
	.reg_vdd_lvsio = NULL,
	.reg_vdd_lvsio_level = 2800000,

	.i2c_pingroup = TEGRA_PINGROUP_DDC,
	.i2c_resume_pupd = TEGRA_PUPD_PULL_UP,
	.i2c_suspend_pupd = TEGRA_PUPD_PULL_DOWN,
	.irq_gpio = GPIO_TSP_INT,
	.board_rev = NULL,
};

static struct i2c_board_info atmel_i2c_touch_info[] = {
	{
		I2C_BOARD_INFO("mxt_touch", 0x4a),
		.irq		= TEGRA_GPIO_TO_IRQ(GPIO_TSP_INT),
		.platform_data = &n1_mxt224E_platform_data,
	},
};

static void n1_camera_init(void)
{
#if defined (CONFIG_MACH_BOSE_ATT)
	tegra_gpio_enable(GPIO_CAM_PMIC_EN2);
	tegra_gpio_enable(GPIO_CAM_MEGA_nRST);
	tegra_gpio_enable(GPIO_CAM_VT_nRST);
	tegra_gpio_enable(GPIO_CAM_VT_nSTBY);
	tegra_gpio_enable(GPIO_CAM_MEGA_INT);
	if (HWREV_FOR_CAMERA <= system_rev) {
		tegra_gpio_enable(GPIO_CAM_MEGA_CORE_1P2_EN);
	}

	gpio_request(GPIO_CAM_PMIC_EN2, "CAMERA_PMIC_EN2");
	gpio_request(GPIO_CAM_MEGA_nRST, "CAMERA_CAM_MEGA_nRST");
	gpio_request(GPIO_CAM_VT_nRST, "CAMERA_CAM_VT_nRST");
	gpio_request(GPIO_CAM_VT_nSTBY, "CAMERA_CAM_VT_nSTBY");
	gpio_request(GPIO_CAM_MEGA_INT, "MEGA_INT");
	if (HWREV_FOR_CAMERA <= system_rev) {
		gpio_request(GPIO_CAM_MEGA_CORE_1P2_EN, "GPIO_CAM_MEGA_CORE_1P2_EN");
	}

	gpio_direction_output(GPIO_CAM_PMIC_EN2, 0);
	gpio_direction_output(GPIO_CAM_MEGA_nRST, 0);
	gpio_direction_output(GPIO_CAM_VT_nRST, 0);
	gpio_direction_output(GPIO_CAM_VT_nSTBY, 0);
	gpio_direction_input(GPIO_CAM_MEGA_INT);
	if (HWREV_FOR_CAMERA <= system_rev) {
		gpio_direction_output(GPIO_CAM_MEGA_CORE_1P2_EN, 0);
	}
#else
	tegra_gpio_enable(GPIO_CAM_PMIC_EN2);	//CAM_SENSOR_A2.8V
	tegra_gpio_enable(GPIO_CAM_MEGA_nRST);	//CAM_MEGA_nRST
	tegra_gpio_enable(GPIO_CAM_VT_nRST);	//GPIO_PT4
	tegra_gpio_enable(GPIO_CAM_VT_nSTBY);	//GPIO_PD5
	tegra_gpio_enable(GPIO_CAM_MEGA_STBY);	//GPIO_PD6
	tegra_gpio_enable(GPIO_CAM_FLASH_EN);	//GPIO_PX3
	tegra_gpio_enable(GPIO_CAM_FLASH_SET);	//GPIO_PX2

	gpio_request(GPIO_CAM_PMIC_EN2, "CAMERA_PMIC_EN2");
	gpio_request(GPIO_CAM_MEGA_nRST, "CAMERA_CAM_MEGA_nRST");
	gpio_request(GPIO_CAM_VT_nRST, "CAMERA_CAM_VT_nRST");
	gpio_request(GPIO_CAM_VT_nSTBY, "CAMERA_CAM_VT_nSTBY");
	gpio_request(GPIO_CAM_MEGA_STBY, "CAMERA_CAM_MEGA_STBY");
	gpio_request(GPIO_CAM_FLASH_EN, "GPIO_CAM_FLASH_EN");
	gpio_request(GPIO_CAM_FLASH_SET, "GPIO_CAM_FLASH_SET");

	gpio_direction_output(GPIO_CAM_PMIC_EN2, 0);
	gpio_direction_output(GPIO_CAM_MEGA_nRST, 0);
	gpio_direction_output(GPIO_CAM_VT_nRST, 0);
	gpio_direction_output(GPIO_CAM_VT_nSTBY, 0);
	gpio_direction_output(GPIO_CAM_MEGA_STBY, 0);
	gpio_direction_output(GPIO_CAM_FLASH_EN, 0);
	gpio_direction_output(GPIO_CAM_FLASH_SET, 0);
#endif
}

struct tegra_pingroup_config mclk = {
	TEGRA_PINGROUP_CSUS,
	TEGRA_MUX_VI_SENSOR_CLK,
	TEGRA_PUPD_PULL_DOWN,
	TEGRA_TRI_TRISTATE
};

static struct s5k4ecgx_reg_8 s5k4ecgx_active_discharge[] = {
	{0x01, 0xFF},	// active discharge enable
	{S5K4ECGX_TABLE_END_8, 0x0},
};

static struct s5k4ecgx_reg_8 s5k4ecgx_VCM_ON[] = {
	{0x08, 0x14},	// 2.8V for VCM - LDO4
	{0x00, 0x05},
	{S5K4ECGX_TABLE_END_8, 0x0},
};

static struct s5k4ecgx_reg_8 s5k4ecgx_power_on_1[] = {
	{0x09, 0x0A},	// 1.8V - LDO5
	{0x00, 0x07},
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x01},

	{0x06, 0x03},	// VT_CORE_1.5V - LDO2
	{0x00, 0x17},
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x01},

	{0x05, 0x02},	// CAM_IO_1.8V - LDO1
	{0x00, 0x37},
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x01},

	{0x07, 0x02},	// VT_CAM_1.8V - LDO3
	{0x00, 0x3F},
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x01},

	{S5K4ECGX_TABLE_END_8, 0x0},
};

static struct s5k4ecgx_reg_8 s5k4ecgx_power_on_2[] = {
	{0x04, 0x04},	// CAM_CORE_1.2V - BUCK
	{0x00, 0xBF},
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x01},
	{S5K4ECGX_TABLE_END_8, 0x0},
};

static const struct s5k6aafx_reg s5k6aafx_stby_reg1[] = {
	{0xFCFC, 0xD000},
	{0x0010, 0x0001},
	{0x1030, 0x0000},
	{0x0014, 0x0001},
	{0xFFFF, 0xFFFF},
};
static const struct s5k6aafx_reg s5k6aafx_stby_reg2[] = {
	{0x0028, 0xD000},
	{0x002A, 0xB0B0},
	{0x0F12, 0x0001},
	{0x0028, 0x7000},
	{0x002A, 0x022E},
	{0x0F12, 0x0001},
	{0x0F12, 0x0001},
	{0xFFFF, 0xFFFF},
};

struct i2c_client *i2c_client_pmic;
struct i2c_client *i2c_client_camera;

static void n1_s5k4ecgx_power_on()
{
	pr_err("%s,\n", __func__);

	s5k4ecgx_write_table_8(i2c_client_pmic, s5k4ecgx_VCM_ON);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_PMIC_EN2, 1);	//CAM_SENSOR_A2.8V
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	s5k4ecgx_write_table_8(i2c_client_pmic, s5k4ecgx_power_on_1);

	gpio_set_value(GPIO_CAM_VT_nSTBY, 1);	//CAM_VT_nSTBY
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	tegra_pinmux_set_func(&mclk);	//CAM_MCLK
	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CSUS, TEGRA_TRI_NORMAL);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_VT_nRST, 1);

	usleep_range(1000, 2000);//1ms

	s5k6aafx_write_table(i2c_client_camera, s5k6aafx_stby_reg1);
	usleep_range(10000, 20000);//10ms
	s5k6aafx_write_table(i2c_client_camera, s5k6aafx_stby_reg2);

	//msleep(150);
	usleep_range(150000, 250000);//150ms

	gpio_set_value(GPIO_CAM_VT_nSTBY, 0);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	s5k4ecgx_write_table_8(i2c_client_pmic, s5k4ecgx_power_on_2);

	gpio_set_value(GPIO_CAM_MEGA_STBY, 1);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_MEGA_nRST, 1);
	//msleep(1);
	usleep_range(1000, 2000);//1ms
}

static struct s5k4ecgx_reg_8 s5k4ecgx_power_off_1[] = {
	{0x00, 0xBB},				// 2.8V for VCM - LDO4
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x02},
	{0x00, 0x9B},				// VT_CAM_1.8V - LDO3
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x02},
	{0x00, 0x93},				// CAM_IO_1.8V - LDO1
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x02},
	{0x00, 0x83},				// VT_CORE_1.5V - LDO2
	{S5K4ECGX_TABLE_WAIT_MS_8, 0x02},
	{0x00, 0x81},				// 1.8V - LDO5

	{S5K4ECGX_TABLE_END, 0x0},
};

static struct s5k4ecgx_reg_8 s5k4ecgx_power_off_2[] = {
	{0x00, 0x01},				// CAM_CORE_1.2V - BUCK

	{S5K4ECGX_TABLE_END, 0x0},
};

static void n1_s5k4ecgx_power_off()
{
	pr_err("%s,\n", __func__);

	gpio_set_value(GPIO_CAM_VT_nSTBY, 0);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_VT_nRST, 0);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_MEGA_nRST, 0);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CSUS, TEGRA_TRI_TRISTATE);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_MEGA_STBY, 0);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	s5k4ecgx_write_table_8(i2c_client_pmic, s5k4ecgx_power_off_1);
	//msleep(1);
	usleep_range(2000, 3000);//1ms

	gpio_set_value(GPIO_CAM_PMIC_EN2, 0);
	//msleep(1);
	usleep_range(2000, 3000);//1ms

	s5k4ecgx_write_table_8(i2c_client_pmic, s5k4ecgx_power_off_2);
	//msleep(1);
	usleep_range(1000, 2000);//1ms
	//mdelay(20);

}

static int n1_s5k4ecgx_flash(int enable)
{
	/* Turn main flash on or off by asserting a value on the EN line. */
	printk(KERN_INFO "========== flash enable = %d \n", enable);
	gpio_set_value(GPIO_CAM_FLASH_EN, enable);

	return 0;
}

static int n1_s5k4ecgx_torch(int enable)
{
	printk(KERN_INFO "========== torch enable = %d \n", enable);
	switch(enable) {
		case 0:
			gpio_set_value(GPIO_CAM_FLASH_EN, 0);
			gpio_set_value(GPIO_CAM_FLASH_SET, 0);
			break;
		case 1:
			gpio_set_value(GPIO_CAM_FLASH_EN, 1);
			gpio_set_value(GPIO_CAM_FLASH_SET, 1);
			break;
	}

	return 0;
}

struct s5k4ecgx_platform_data n1_s5k4ecgx_data = {
	.power_on = n1_s5k4ecgx_power_on,
	.power_off = n1_s5k4ecgx_power_off,
	.flash_onoff = n1_s5k4ecgx_flash,
	.torch_onoff = n1_s5k4ecgx_torch,
};

static const struct i2c_board_info sec_s5k4ecgx_camera[] = {
	{
		I2C_BOARD_INFO("s5k4ecgx", 0xAC>>1),
		.platform_data = &n1_s5k4ecgx_data,
	},
};

struct tegra_pingroup_config s5k6aafx_mclk = {
	TEGRA_PINGROUP_CSUS, TEGRA_MUX_VI_SENSOR_CLK, TEGRA_PUPD_PULL_DOWN, TEGRA_TRI_TRISTATE
};

/*static struct s5k6aafx_reg_8 s5k6aafx_power_on_1[] = {
	{0x04, 0x04},	// CAM_CORE_1.2V - BUCK
	{0x00, 0x81},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},

	{S5K6AAFX_TABLE_END_8, 0x0},
};*/

static struct s5k6aafx_reg_8 s5k6aafx_power_on_12[] = {
	{0x09, 0x0A},	// 1.8V - LDO5
	{0x00, 0x03},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},

	{0x06, 0x03},	// VT_CORE_1.5V - LDO2
	{0x00, 0x13},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},

	{0x07, 0x02},	// VT_CAM_1.8V - LDO3
	{0x00, 0x1B},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},

//	{0x05, 0x02},	// CAM_IO_1.8V - LDO1
//	{0x00, 0x39},
//	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},

	{S5K6AAFX_TABLE_END_8, 0x0},
};

static struct s5k6aafx_reg_8 s5k6aafx_power_on_11[] = {
	{0x06, 0x03},	// VT_CORE_1.5V - LDO2
	{0x00, 0x11},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},

	{0x07, 0x02},	// VT_CAM_1.8V - LDO3
	{0x00, 0x19},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},

	{0x05, 0x02},	// CAM_IO_1.8V - LDO1
	{0x00, 0x39},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},

	{S5K6AAFX_TABLE_END_8, 0x0},
};

static struct s5k6aafx_reg_8 s5k6aafx_power_on_21[] = {
	{0x04, 0x04},	// CAM_CORE_1.2V - BUCK
	{0x00, 0xB9},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},
	{S5K6AAFX_TABLE_END_8, 0x0},
};
static struct s5k6aafx_reg_8 s5k6aafx_power_off_21[] = {
	{0x00, 0x39},				// CAM_CORE_1.2V - BUCK

	{S5K6AAFX_TABLE_END, 0x0},
};

static struct s5k6aafx_reg_8 s5k6aafx_power_on_22[] = {
	{0x04, 0x04},	// CAM_CORE_1.2V - BUCK
	{0x00, 0x9B},
	{S5K6AAFX_TABLE_WAIT_MS_8, 0x01},
	{S5K6AAFX_TABLE_END_8, 0x0},
};
static struct s5k6aafx_reg_8 s5k6aafx_power_off_22[] = {
	{0x00, 0x1B},				// CAM_CORE_1.2V - BUCK

	{S5K6AAFX_TABLE_END, 0x0},
};

static void n1_s5k6aafx_power_on(void)
{
	pr_err("%s,\n", __func__);

	gpio_set_value(GPIO_CAM_PMIC_EN2, 1);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	if (system_rev >= 12) {
		s5k6aafx_write_table_8(i2c_client_pmic, s5k6aafx_power_on_12);
		s5k6aafx_write_table_8(i2c_client_pmic, s5k6aafx_power_on_22);
		usleep_range(1000, 2000);//1ms
		s5k6aafx_write_table_8(i2c_client_pmic, s5k6aafx_power_off_22);
	} else {
		s5k6aafx_write_table_8(i2c_client_pmic, s5k6aafx_power_on_11);
		s5k6aafx_write_table_8(i2c_client_pmic, s5k6aafx_power_on_21);
		usleep_range(1000, 2000);//1ms
		s5k6aafx_write_table_8(i2c_client_pmic, s5k6aafx_power_off_21);
	}

	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_VT_nSTBY, 1);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	tegra_pinmux_set_func(&s5k6aafx_mclk);
	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CSUS, TEGRA_TRI_NORMAL);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_VT_nRST, 1);
	//msleep(1);
	usleep_range(1000, 2000);//1ms
}

static struct s5k6aafx_reg_8 s5k6aafx_power_off_12[] = {
	{0x00, 0x13},	// VT_CAM_1.8V - LDO3
	{0x00, 0x03},	// VT_CORE_1.5V - LDO2
	{0x00, 0x01},	// 1.8V - LDO5

	{S5K6AAFX_TABLE_END_8, 0x0},
};

static struct s5k6aafx_reg_8 s5k6aafx_power_off_11[] = {
	{0x00, 0x31},	// VT_CAM_1.8V - LDO3
	{0x00, 0x11},	// CAM_IO_1.8V - LDO1
	{0x00, 0x01},	// VT_CORE_1.5V - LDO2

	{S5K6AAFX_TABLE_END_8, 0x0},
};

static void n1_s5k6aafx_power_off(void)
{
	pr_err("%s,\n", __func__);

	gpio_set_value(GPIO_CAM_VT_nRST, 0);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_MEGA_nRST, 0);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	tegra_pinmux_set_tristate(TEGRA_PINGROUP_CSUS, TEGRA_TRI_TRISTATE);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_VT_nSTBY, 0);
	//msleep(20);
	usleep_range(20000, 30000);//20ms

	if (system_rev >= 12)
		s5k6aafx_write_table_8(i2c_client_pmic, s5k6aafx_power_off_12);
	else
		s5k6aafx_write_table_8(i2c_client_pmic, s5k6aafx_power_off_11);
	//msleep(1);
	usleep_range(1000, 2000);//1ms

	gpio_set_value(GPIO_CAM_PMIC_EN2, 0);
	//msleep(1);
	usleep_range(1000, 2000);//1ms
}


struct s5k6aafx_platform_data n1_s5k6aafx_data = {
	.power_on = n1_s5k6aafx_power_on,
	.power_off = n1_s5k6aafx_power_off
};

static const struct i2c_board_info sec_s5k6aafx_camera[] = {
	{
		I2C_BOARD_INFO("s5k6aafx", 0x78>>1),
		.platform_data = &n1_s5k6aafx_data,
	},
};

static const struct i2c_board_info sec_pmic_camera[] = {
	{
		I2C_BOARD_INFO("s5k4ecgx_pmic", 0x7c>>1),
	},
};

static int __init camera_init(void)
{
#ifdef CONFIG_TEGRA_CAMERA
	int status;
#endif
	n1_camera_init();

#ifdef CONFIG_TEGRA_CAMERA
#if defined (CONFIG_MACH_BOSE_ATT)
	if(system_rev >= 4) {
		status = i2c_register_board_info(16, sec_pmic_camera,
					ARRAY_SIZE(sec_pmic_camera));
		status = i2c_register_board_info(3, sec_m5mo_camera,
					ARRAY_SIZE(sec_m5mo_camera));
		status = i2c_register_board_info(3, sec_s5k6aafx_camera,
					ARRAY_SIZE(sec_s5k6aafx_camera));
	} else {
		status = i2c_register_board_info(3, sec_pmic_camera,
					ARRAY_SIZE(sec_pmic_camera));
		status = i2c_register_board_info(3, sec_m5mo_camera,
					ARRAY_SIZE(sec_m5mo_camera));
		status = i2c_register_board_info(3, sec_s5k6aafx_camera,
					ARRAY_SIZE(sec_s5k6aafx_camera));
	}
#else
	if(system_rev >= 4) {
		status = i2c_register_board_info(16, sec_pmic_camera,
					ARRAY_SIZE(sec_pmic_camera));
		status = i2c_register_board_info(3, sec_s5k4ecgx_camera,
					ARRAY_SIZE(sec_s5k4ecgx_camera));
		status = i2c_register_board_info(3, sec_s5k6aafx_camera,
					ARRAY_SIZE(sec_s5k6aafx_camera));
	} else {
		status = i2c_register_board_info(3, sec_pmic_camera,
					ARRAY_SIZE(sec_pmic_camera));
		status = i2c_register_board_info(3, sec_s5k4ecgx_camera,
					ARRAY_SIZE(sec_s5k4ecgx_camera));
		status = i2c_register_board_info(3, sec_s5k6aafx_camera,
					ARRAY_SIZE(sec_s5k6aafx_camera));
	}
#endif
#endif

	return 0;
}
static int __init n1_touch_init(void)
{
	n1_touch_init_hw(&n1_mxt224E_platform_data);

	i2c_register_board_info(1, atmel_i2c_touch_info,
			ARRAY_SIZE(atmel_i2c_touch_info));
	return 0;
}

void tegra_usb1_ldo_en(int active, int instance)
{
	struct regulator *reg = regulator_get(NULL, "VAP_USB_3V3");
	int ret = 0;
	int try_times = 3;

	if (IS_ERR_OR_NULL(reg)) {
		pr_err("%s: failed to get VAP_USB_3V3 regulator\n",
							__func__);
		return;
	}
	pr_info("Board N1 : %s=%d instance=%d present regulator=%d\n",
		__func__, active, instance,
			usb_data.usb_regulator_on[instance]);
	mutex_lock(&usb_data.ldo_en_lock);

	if (active) {
		if (!usb_data.usb_regulator_on[instance]) {
			do {
				ret = regulator_enable(reg);
				if (ret == 0)
					break;
				msleep(20);
			} while (try_times--);
			if (ret == 0)
				usb_data.usb_regulator_on[instance] = 1;
			else
				pr_err("%s: failed to turn on regulator\n"
							, __func__);
		}
	} else {
		regulator_disable(reg);
		usb_data.usb_regulator_on[instance] = 0;
	}
	regulator_put(reg);

	mutex_unlock(&usb_data.ldo_en_lock);
}

void tegra_usb1_power(int active)
{
	active = !!active;

	if (active)
		otg_open.otg_enabled = 1;
	else
		otg_open.otg_enabled = 0;

	gpio_direction_output(GPIO_USB_OTG_EN, active);
	pr_info("Board N1 : %s = %d\n", __func__, active);
}

void tegra_set_clk_func(void (*check_detection)(int *, int),
				unsigned int *clk_cause)
{
	otg_clk_data.check_detection = check_detection;
	otg_clk_data.clk_cause = clk_cause;
}

static struct usb_phy_plat_data tegra_usb_phy_pdata[] = {
	[0] = {
			.instance = 0,
			.vbus_irq = MAX8907C_INT_BASE + MAX8907C_IRQ_VCHG_DC_R,
			.vbus_gpio = -1,
			.usb_ldo_en = tegra_usb1_ldo_en,
			.vbus_en = tegra_usb1_power,
			.otg_clk_data = &otg_clk_data,
	},
	[1] = {
			.instance = 1,
			.vbus_gpio = -1,
	},
	[2] = {
			.instance = 2,
			.vbus_gpio = -1,
	},
};

static struct tegra_ehci_platform_data tegra_ehci_pdata[] = {
	[0] = {
			.phy_type = TEGRA_USB_PHY_TYPE_UTMIP,
			.phy_config = &utmi_phy_config[0],
			.operating_mode = TEGRA_USB_OTG,
			.power_down_on_bus_suspend = 0,
			.default_enable = false,
	},
#ifdef CONFIG_LINK_DEVICE_HSIC
	[1] = {
			.phy_config = &hsic_phy_config,
			.operating_mode = TEGRA_USB_HOST,
			.power_down_on_bus_suspend = 1,
			.phy_type = TEGRA_USB_PHY_TYPE_HSIC,
			.default_enable = false,
	},
#else
	[1] = {
			.phy_config = &ulpi_phy_config,
			.operating_mode = TEGRA_USB_HOST,
			.power_down_on_bus_suspend = 1,
			.default_enable = true,
	},
#endif
	[2] = {
			.phy_type = TEGRA_USB_PHY_TYPE_UTMIP,
			.phy_config = &utmi_phy_config[1],
			.operating_mode = TEGRA_USB_HOST,
			.power_down_on_bus_suspend = 1,
			.default_enable = true,
	},
};



static struct tegra_otg_platform_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci_pdata[0],
	.acc_power = tegra_usb1_power,
	.usb_ldo_en = tegra_usb1_ldo_en,
	.set_clk_func = tegra_set_clk_func,
	.otg_id_open = &otg_open,
	.batt_level = &sec_batt_level,
};

static void n1_usb_init(void)
{
	int ret;

	mutex_init(&usb_data.ldo_en_lock);
	usb_data.usb_regulator_on[0] = 0;
	usb_data.usb_regulator_on[1] = 0;
	usb_data.usb_regulator_on[2] = 0;

	tegra_gpio_enable(GPIO_USB_OTG_EN);
	ret = gpio_request(GPIO_USB_OTG_EN, "GPIO_USB_OTG_EN");
	if (ret) {
		pr_err("%s: gpio_request() for USB_OTG_EN failed\n",
			__func__);
		return;
	}
	ret = gpio_direction_output(GPIO_USB_OTG_EN, 0);

	tegra_usb_phy_init(tegra_usb_phy_pdata, ARRAY_SIZE(tegra_usb_phy_pdata));
	fsa9480_otg_data_init(&otg_open, &otg_clk_data, tegra_usb1_power);

	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);
}

#if defined CONFIG_LINK_DEVICE_HSIC
static int __init tegra_ehci2_hsic_init(void)
{
#ifdef CONFIG_SAMSUNG_LPM_MODE
	int ret = 0;
	if (!charging_mode_from_boot) {
		/* Uncomment when fully ported from HC.
		register_smd_resource(); */
		tegra_ehci2_device.dev.platform_data = &tegra_ehci_pdata[1];
		ret = platform_device_register(&tegra_ehci2_device);
	}
	return ret;
#else
	/* Uncomment when fully ported from HC.
	register_smd_resource(); */
	tegra_ehci2_device.dev.platform_data = &tegra_ehci_pdata[1];
	return platform_device_register(&tegra_ehci2_device);
#endif
}
late_initcall(tegra_ehci2_hsic_init);
#endif

static int n1_jack_init(void)
{
	int ret = 0;

	ret = gpio_request(GPIO_MICBIAS1_EN, "micbias_enabl");
	if (ret < 0) {
		gpio_free(GPIO_MICBIAS1_EN);
		return ret;
	}

	ret = gpio_request(GPIO_MICBIAS2_EN, "ear_micbias_enable");
	if (ret < 0) {
		gpio_free(GPIO_MICBIAS2_EN);
		return ret;
	}

	ret = gpio_direction_output(GPIO_MICBIAS1_EN, 0);
	if (ret < 0)
		goto cleanup;

	ret = gpio_direction_output(GPIO_MICBIAS2_EN, 0);
	if (ret < 0)
		goto cleanup;

	tegra_gpio_enable(GPIO_MICBIAS1_EN);
	tegra_gpio_enable(GPIO_MICBIAS2_EN);

	tegra_gpio_enable(GPIO_DET_3_5);
	tegra_gpio_enable(GPIO_EAR_SEND_END);

	return ret;
cleanup:
	gpio_free(GPIO_MICBIAS1_EN);
	gpio_free(GPIO_MICBIAS2_EN);
	return ret;

}

static void tegra_n1_codec_init(void)
{
	gpio_request(GPIO_CODEC_LDO_EN, "codec_ldo_en");
	tegra_gpio_enable(GPIO_CODEC_LDO_EN);
	gpio_direction_output(GPIO_CODEC_LDO_EN, 0);
	msleep(50);
	gpio_set_value(GPIO_CODEC_LDO_EN, 1);

	/* Earmic sel disable */
	gpio_request(GPIO_EAR_SEL, "ear_sel");
	tegra_gpio_enable(GPIO_EAR_SEL);
	gpio_direction_output(GPIO_EAR_SEL, 0);
}

static void n1_fsa9480_init(void)
{
	gpio_request(GPIO_UART_SEL, "uart_sel");
	gpio_request(GPIO_JACK_nINT, "jack_nint");

	gpio_direction_input(GPIO_JACK_nINT);

	tegra_gpio_enable(GPIO_UART_SEL);
	tegra_gpio_enable(GPIO_JACK_nINT);
	if((system_rev >= 0x05) && (system_rev < 0x07))
	{
		gpio_request(GPIO_UART_SEL_EN, "uart_sel_en");
		gpio_direction_output(GPIO_UART_SEL_EN, 0);
		tegra_gpio_enable(GPIO_UART_SEL_EN);
	}
}

#ifdef CONFIG_PN544
static void n1_nfc_init(void)
{
	gpio_request(GPIO_NFC_IRQ, "nfc_irq");
	gpio_request(GPIO_NFC_EN, "nfc_en");
	gpio_request(GPIO_NFC_FIRMWARE, "nfc_firm");

	gpio_direction_input(GPIO_NFC_IRQ);
	gpio_direction_output(GPIO_NFC_EN, 0);
	gpio_direction_output(GPIO_NFC_FIRMWARE, 0);

	tegra_gpio_enable(GPIO_NFC_IRQ);
	tegra_gpio_enable(GPIO_NFC_EN);
	tegra_gpio_enable(GPIO_NFC_FIRMWARE);
}
#endif

static int __init n1_gps_init(void)
{
	int error;
	struct device *gps_dev;
	struct clk *clk32 = clk_get_sys(NULL, "blink");
	if (!IS_ERR(clk32)) {
		clk_set_rate(clk32, clk32->parent->rate);
		clk_enable(clk32);
	}

	gps_dev = device_create(sec_class, NULL, 0, NULL, "gps");
	if (IS_ERR(gps_dev)) {
		pr_err("Failed to create gps device!\n");
		error = PTR_ERR(gps_dev);
		goto err;
	}

	gpio_request(GPIO_GPS_NRST, "GPS_NRST");
	gpio_request(GPIO_GPS_EN, "GPS_EN");

	gpio_direction_output(GPIO_GPS_NRST, 1);
	gpio_direction_output(GPIO_GPS_EN, 0);

	tegra_gpio_enable(GPIO_GPS_NRST);
	tegra_gpio_enable(GPIO_GPS_EN);

	gpio_export(GPIO_GPS_NRST, 1);
	gpio_export(GPIO_GPS_EN, 1);

	gpio_export_link(gps_dev, "GPS_nRST", GPIO_GPS_NRST);
	gpio_export_link(gps_dev, "GPS_PWR_EN", GPIO_GPS_EN);
	return 0;

err:
	return error;
}


static struct i2c_board_info isa1200_i2c_vibrator_info[] = {
	{
		I2C_BOARD_INFO("isa1200",  0x48),
	},
};

static int __init n1_vibrator_init(void)
{
	i2c_register_board_info(17, isa1200_i2c_vibrator_info,
			ARRAY_SIZE(isa1200_i2c_vibrator_info));

	return 0;

}

static void n1_power_off(void)
{
	int ret;

	ret = max8907c_power_off();
	if (ret)
		pr_err("n1: failed to power off\n");

	while (1);
}

static void __init n1_power_off_init(void)
{
	pm_power_off = n1_power_off;
}

#ifdef CONFIG_KERNEL_DEBUG_SEC
/* Debug level control */
static ssize_t show_sec_debug_level(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	int sec_debug_level = kernel_sec_get_debug_level();
	char buffer[5];

	printk("%s %x\n", __func__, sec_debug_level);

	memcpy(buffer, &sec_debug_level, 4);
	buffer[4] = '\0';
	return sprintf(buf, "%s\n", buffer);
}

static ssize_t store_sec_debug_level(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	int sec_debug_level = 0;
	memcpy(&sec_debug_level, buf, 4);

	printk("%s %x\n", __func__, sec_debug_level);

	if (!(sec_debug_level == KERNEL_SEC_DEBUG_LEVEL_LOW
			|| sec_debug_level == KERNEL_SEC_DEBUG_LEVEL_MID
			|| sec_debug_level == KERNEL_SEC_DEBUG_LEVEL_HIGH))
		return -EINVAL;

	kernel_sec_set_debug_level(sec_debug_level);

	return count;
}

static DEVICE_ATTR(sec_debug_level, 0644, show_sec_debug_level, store_sec_debug_level);
/* -- Debug level control */
#endif

static void __init tegra_n1_init(void)
{
	char serial[20];
#ifdef CONFIG_KERNEL_DEBUG_SEC
	int ret = 0;
#endif
	tegra_clk_init_from_table(n1_clk_init_table);
	n1_pinmux_init();
	pr_info("Board HW Revision: system_rev = %d\n", system_rev);
	n1_i2c_init();
	n1_uart_init();

	snprintf(serial, sizeof(serial), "%llx", tegra_chip_uid());
	tegra_i2s_device1.dev.platform_data = &tegra_audio_pdata[0];
	tegra_i2s_device2.dev.platform_data = &tegra_audio_pdata[1];
#if !defined CONFIG_LINK_DEVICE_HSIC
	tegra_ehci2_device.dev.platform_data = &n1_ehci2_ulpi_platform_data;
#endif
	platform_add_devices(n1_devices, ARRAY_SIZE(n1_devices));

#ifdef CONFIG_MAX8922_CHARGER
	if (system_rev >= HWREV_FOR_EXTERNEL_CHARGER)
		platform_device_register(&max8922_device_charger);
#endif

	sec_class = class_create(THIS_MODULE, "sec");
	if (IS_ERR(sec_class))
		pr_err("Failed to create sec class!\n");

	n1_jack_init();
	n1_sdhci_init();
#ifdef CONFIG_MHL_SII9234
	n1_mhl_gpio_init();
#endif
	n1_gpio_i2c_init(charging_mode_from_boot);
	n1_regulator_init();

	n1_touch_init();
#ifdef CONFIG_KEYBOARD_GPIO
	n1_keys_init();
#endif
	n1_usb_init();
	n1_gps_init();
    n1_panel_init();
	n1_power_off_init();
	n1_emc_init();
#ifdef CONFIG_BCM4330_RFKILL
	n1_rfkill_init();
#endif
	n1_sensors_init();
	tegra_n1_codec_init();


	n1_fsa9480_init();
	camera_init();
	init_wifi_mem();		// for wifi static buf
	n1_vibrator_init();
	tegra_release_bootloader_fb();
	register_reboot_notifier(&n1_reboot_notifier);
	/* console suspend disable */
	console_suspend_enabled = 0;
#ifdef CONFIG_KERNEL_DEBUG_SEC
	/* Add debug level node */
	struct device *platform = n1_devices[0]->dev.parent;
	ret = device_create_file(platform, &dev_attr_sec_debug_level);
	if (ret)
		printk("Fail to create sec_debug_level file\n");
#endif
}

int __init tegra_n1_protected_aperture_init(void)
{
	tegra_protected_aperture_init(tegra_grhost_aperture);
	return 0;
}
late_initcall(tegra_n1_protected_aperture_init);

void __init tegra_n1_reserve(void)
{
	u64 ram_console_start;
	int ret;

	if (memblock_reserve(0x0, 4096) < 0)
		pr_warn("Cannot reserve first 4K of memory for safety\n");

	if (system_rev < 2) /* 512 MB */
		tegra_reserve(SZ_128M, SZ_8M, SZ_16M);
	else	/* 1GB */
		tegra_reserve(SZ_256M, SZ_8M, SZ_16M);

	/* Reserve memory for the ram console. */
	ram_console_start = memblock_end_of_DRAM() - SZ_1M;

	ret = memblock_remove(ram_console_start, SZ_1M);
	if (ret < 0) {
		pr_err("Failed to reserve 0x%x bytes for ram_console at "
			"0x%llx, err = %d.\n",
			SZ_1M, ram_console_start, ret);
	} else {
		ram_console_resource[0].start = ram_console_start;
		ram_console_resource[0].end = ram_console_start + SZ_1M - 1;
	}
}

MACHINE_START(SAMSUNG_N1, "n1")
	.boot_params    = 0x00000100,
	.map_io         = tegra_map_common_io,
	.reserve        = tegra_n1_reserve,
	.init_early		= tegra_init_early,
	.init_irq       = tegra_init_irq,
	.timer          = &tegra_timer,
	.init_machine   = tegra_n1_init,
MACHINE_END

