/*
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

#ifndef _FSA9480_H_
#define _FSA9480_H_
#include <linux/wakelock.h>

struct fsa9480_usbsw {
	struct i2c_client		*client;
	struct fsa9480_platform_data	*pdata;
	int				dev1;
	int				dev2;
	int				mansw;
	struct mutex			lock;
#if defined(CONFIG_MACH_BOSE_ATT)
	struct wake_lock mhl_wake_lock;
#endif
};

struct otg_id_open_data{
	void (*id_open)(struct fsa9480_usbsw *);
	void (*otg_cb)(bool);
	struct fsa9480_usbsw *otg_open_data;
	int otg_enabled;
	int *otg_state;
	int *host_state;
};

enum {
	FSA9480_DETACHED,
	FSA9480_ATTACHED
};

enum {
	FSA9480_RECOVER_OVP,
	FSA9480_OVP,
};

struct fsa9480_platform_data {
	void (*cfg_gpio) (void);
	void (*otg_cb) (bool attached);
	void (*usb_cb) (bool attached);
	int (*uart_cb) (bool attached);
	void (*charger_cb) (bool attached);
	void (*charger_ovp_cb) (bool attached);
	void (*jig_cb) (bool attached);
	void (*dock_charger_cb) (bool attached);
	void (*deskdock_cb) (bool attached);
#if defined(CONFIG_MACH_BOSE_ATT)
	 void (*mhldock_cb) (bool attached);
#endif
	void (*cardock_cb) (bool attached);
	void (*reset_cb) (void);
	void (*set_otg_func)(void(*)(struct fsa9480_usbsw *),
				struct fsa9480_usbsw *);
	void (*inform_charger_connection) (int);
};

enum {
	AUTO_SWITCH = 0,
	SWITCH_USB_Port,
	SWITCH_Audio_Port,
	SWITCH_UART_Port,
	SWITCH_V_Audio_Port
};

void fsa9480_manual_switching(int path);

#endif /* _FSA9480_H_ */
