/* linux/power/max8907c-charger.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_MAX8907C_CHARGER_H
#define __LINUX_MAX8907C_CHARGER_H

/* interrupt */
#define MAX8907C_VCHG_OVP		(1 << 0)
#define MAX8907C_VCHG_F			(1 << 1)
#define MAX8907C_VCHG_R			(1 << 2)
#define MAX8907C_THM_OK_R		(1 << 8)
#define MAX8907C_THM_OK_F		(1 << 9)
#define MAX8907C_MBATTLOW_F		(1 << 10)
#define MAX8907C_MBATTLOW_R		(1 << 11)
#define MAX8907C_CHG_RST		(1 << 12)
#define MAX8907C_CHG_DONE		(1 << 13)
#define MAX8907C_CHG_TOPOFF		(1 << 14)
#define MAX8907C_CHK_TMR_FAULT		(1 << 15)

/* MAX8907C_REG_CHGCNTL1 */
#define CHGCNTL1_FCHG_SHIFT         0
#define CHGCNTL1_CHG_TOPFF_SHIFT    5
#define CHGCNTL1_NCHGEN_SHIFT	    7
#define CHGCNTL1_FCHG_MASK		(0x7 << CHGCNTL1_FCHG_SHIFT)
#define CHGCNTL1_NCHGEN_MASK	(0x1 << CHGCNTL1_NCHGEN_SHIFT)
#define CHGCNTL1_CHG_TOPFF_MASK	(0x3 << CHGCNTL1_CHG_TOPFF_SHIFT)

/* MAX8907C_REG_CHGCNTL2 */
#define CHGCNTL2_FCHG_TMR_SHIFT		4
#define CHGCNTL2_MBAT_REG_SHIFT		3
#define CHGCNTL2_FCHG_TMR_MASK		(3 << CHGCNTL2_FCHG_TMR_SHIFT)
#define CHGCNTL2_MBAT_REG_MASK		(0x1 << CHGCNTL2_MBAT_REG_SHIFT)

/* MAX8907C_CHG_STAT */
#define CHGSTAT_MBDET_SHIFT			1
#define CHGSTAT_CHG_MODE_SHIFT		2
#define CHGSTAT_VCHG_OK_SHIFT		7

#define CHGSTAT_MBDET_MASK			(1 << CHGSTAT_MBDET_SHIFT)
#define CHGSTAT_CHG_MODE_MASK		(3 << CHGSTAT_CHG_MODE_SHIFT)
#define CHGSTAT_VCHG_OK_MASK		(1 << CHGSTAT_VCHG_OK_SHIFT)

enum {
	BAT_NOT_DETECTED,
	BAT_DETECTED
};

enum max8907c_chager_mode {
	MAX8907C_PREQUALIFICATION	= 0x00,
	max8907c_FAST_CHARGE			= 0x01,
	MAX8907C_TOP_OFF				= 0x02,
	MAX8907C_CHARGE_DONE		= 0x03,
};

enum max8907c_charger_topoff_threshold {
	MAX8907C_TOPOFF_5PERCENT	= 0x00,
	MAX8907C_TOPOFF_10PERCENT	= 0x01,
	MAX8907C_TOPOFF_15PERCENT	= 0x02,
	MAX8907C_TOPOFF_20PERCENT	= 0x03,
};

enum max8907c_charger_restart_hysteresis {
	MAX8907C_RESTART_100MV	= 0x00,
	MAX8907C_RESTART_150MV	= 0x01,
	MAX8907C_RESTART_200MV	= 0x02,
	MAX8907C_RESTART_FLOAT	= 0x03,
};

enum max8907c_fast_charging_current {
	MAX8907C_FASTCHARGE_90MA	= 0x00,
	MAX8907C_FASTCHARGE_300MA	= 0x01,
	MAX8907C_FASTCHARGE_460MA	= 0x02,
	MAX8907C_FASTCHARGE_600MA	= 0x03,
	MAX8907C_FASTCHARGE_700MA	= 0x04,
	MAX8907C_FASTCHARGE_800MA	= 0x05,
	MAX8907C_FASTCHARGE_900MA	= 0x06,
	MAX8907C_FASTCHARGE_1000MA	= 0x07,
};

enum max8907c_fast_charger_time {
	MAX8907C_FCHARGE_TM_8H		= 0x00,
	MAX8907C_FCHARGE_TM_12H		= 0x01,
	MAX8907C_FCHARGE_TM_16H 	= 0x02,
	MAX8907C_FCHARGE_TM_OFF 	= 0x03,
};

struct max8907c_charger_pdata {
	int irq;
#ifdef CONFIG_MACH_N1
    int (*topoff_cb) (void);
    int (*vchg_f_cb) (int);
    int (*vchg_r_f_cb) (int); /* FACTORY TEST BINARY */
#endif
	enum max8907c_charger_topoff_threshold topoff_threshold;
	enum max8907c_charger_restart_hysteresis restart_hysteresis;
	enum max8907c_charger_restart_hysteresis fast_charging_current;
	enum max8907c_fast_charger_time fast_charger_time;
};

#endif
