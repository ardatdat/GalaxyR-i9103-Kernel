/*
 *  sec_battery.c
 *  Samsung Mobile Battery Driver
 *
 *  Copyright (C) 2010 Samsung Electronics
 *
 *  <ms925.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <mach/sec_battery.h>
#include <linux/power/max8907c-charger.h>
#include <linux/timer.h>
#include <linux/android_alarm.h>

#define HWREV_FOR_EXTERNEL_CHARGER	7

#define POLLING_INTERVAL	(30 * 1000)

#define FAST_POLL	40	/* 40 sec */
#define SLOW_POLL	(30*60)	/* 30 min */

#if 0
#define FULL_CHARGING_TIME	(6 * 60 *  60 * HZ)	/* 6hr */
#define RECHARGING_TIME		(90 * 60 * HZ)		/* 1.5hr */
#endif
#define FULL_CHARGING_TIME	(6 * 60 *  60)	/* 6hr */
#define RECHARGING_TIME		(90 * 60 )/* 1.5hr */

#define RECHARGING_VOLTAGE	(4130 * 1000)		/* 4.13 V */

#define FG_T_SOC		0
#define FG_T_VCELL		1
#define FG_T_TEMPER		2
#define FG_T_ONLINE		3

#define HIGH_BLOCK_TEMP		600
#define HIGH_RECOVER_TEMP		400
#define LOW_BLOCK_TEMP		(-30)
#define LOW_RECOVER_TEMP		0
#define TEMP_BLOCK_COUNT	3

#define BAT_DET_COUNT		0
#define ADC_SAMPLING_CNT	6

enum batt_full_t {
	BATT_NOT_FULL = 0,
	BATT_1ST_FULL,
	BATT_2ND_FULL,
};

#define __TEST_DEVICE_DRIVER__

#ifdef __TEST_DEVICE_DRIVER__
static ssize_t sec_batt_test_show_property(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf);
static ssize_t sec_batt_test_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count);
static int bat_temp_force_state = 0;
#endif /* __TEST_DEVICE_DRIVER__ */

/* to notify lpm state to other modules */
int lpm_mode_flag;
EXPORT_SYMBOL(lpm_mode_flag);

static ssize_t sec_bat_show_property(struct device *dev,
			struct device_attribute *attr,
			char *buf);

static ssize_t sec_bat_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count);

struct sec_bat_info {
	struct device		*dev;

	char			*fuel_gauge_name;
	char			*charger_name;
	char 		*sub_charger_name;

	unsigned int		adc_arr_size;
	struct sec_bat_adc_table_data *adc_table;
	unsigned int adc_channel;

	struct power_supply	psy_bat;
	struct power_supply	psy_usb;
	struct power_supply	psy_ac;

	struct wake_lock	vbus_wake_lock;
	struct wake_lock	monitor_wake_lock;
	struct wake_lock	cable_wake_lock;

	enum cable_type_t       cable_type;
	enum batt_full_t	batt_full_status;

	int				batt_temp;	/* Battery Temperature (C) */
	int				batt_temp_high_cnt;
	int				batt_temp_low_cnt;
	int				batt_temp_recover_cnt;
	unsigned int		batt_health;
	unsigned int		batt_vcell;
	unsigned int		batt_soc;
	unsigned int		charging_status;
	unsigned int 		batt_lp_charging;
	struct s3c_adc_client	*padc;

	struct workqueue_struct *monitor_wqueue;
	struct work_struct	monitor_work;
	struct work_struct	cable_work;

	struct alarm		wakeup_alarm;

	unsigned long		charging_start_time;
	unsigned int		recharging_status;

	unsigned int (*get_lpcharging_state)(void);
	bool use_sub_charger;

	int present;
	int present_count;
	void (*get_init_cable_state)(struct power_supply *psy);
	int (*get_temperature)(void);
	bool		slow_poll;
	ktime_t		last_poll;
};

static char *supply_list[] = {
	"battery",
};

static enum power_supply_property sec_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TECHNOLOGY,
};

static enum power_supply_property sec_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int sec_bat_get_fuelgauge_data(struct sec_bat_info *info, int type)
{
	struct power_supply *psy
		= power_supply_get_by_name(info->fuel_gauge_name);
	union power_supply_propval value;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get fuel gauge ps\n", __func__);
		return -ENODEV;
	}

	switch (type) {
	case FG_T_VCELL:
		psy->get_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &value);
		break;
	case FG_T_SOC:
		psy->get_property(psy, POWER_SUPPLY_PROP_CAPACITY_LEVEL, &value);
		break;
	case FG_T_TEMPER:
		psy->get_property(psy, POWER_SUPPLY_PROP_TEMP, &value);
		break;
	default:
		return -ENODEV;
	}

	return value.intval;
}

static int sec_bat_set_fuelgauge_data(
	struct sec_bat_info *info, int type, int data)
{
	struct power_supply *psy
		= power_supply_get_by_name(info->fuel_gauge_name);
	union power_supply_propval value;

	value.intval = data;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get fuel gauge ps\n", __func__);
		return -ENODEV;
	}

	switch (type) {
	case FG_T_ONLINE:
		psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
		break;
	default:
		return -ENODEV;
	}

	return value.intval;
}

static int sec_bat_get_property(struct power_supply *ps,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct sec_bat_info *info = container_of(ps, struct sec_bat_info,
			psy_bat);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = info->charging_status;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = info->batt_health;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = info->batt_temp;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		/* battery is always online */
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = info->batt_vcell;
		if (val->intval == -1)
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (info->charging_status == POWER_SUPPLY_STATUS_FULL) {
			val->intval = 100;
			break;
		} else {
			val->intval = info->batt_soc;
			if ((val->intval >= 100) &&
		(info->charging_status == POWER_SUPPLY_STATUS_CHARGING))
				val->intval = 99;
		}
		if (val->intval == -1)
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int sec_usb_get_property(struct power_supply *ps,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct sec_bat_info *info = container_of(ps, struct sec_bat_info,
			psy_usb);

	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	/* Set enable=1 only if the USB charger is connected */
	val->intval = (info->cable_type == CABLE_TYPE_USB);

	return 0;
}

static int sec_ac_get_property(struct power_supply *ps,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct sec_bat_info *info = container_of(ps, struct sec_bat_info,
			psy_ac);

	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	/* Set enable=1 only if the AC charger is connected */
	val->intval = (info->cable_type == CABLE_TYPE_AC) ||
			(info->cable_type == CABLE_TYPE_DOCK);

	return 0;
}

static void sec_bat_alarm(struct alarm *alarm)
{
	struct sec_bat_info *info = container_of(alarm, struct sec_bat_info,
			wakeup_alarm);

	printk(KERN_INFO "%s\n", __func__);
	wake_lock(&info->monitor_wake_lock);
	queue_work(info->monitor_wqueue, &info->monitor_work);
}

static void sec_program_alarm(struct sec_bat_info *info, int seconds)
{
	ktime_t low_interval = ktime_set(seconds - 10, 0);
	ktime_t slack = ktime_set(20, 0);
	ktime_t next;

	printk(KERN_INFO "%s \n", __func__);
	next = ktime_add(info->last_poll, low_interval);
	alarm_start_range(&info->wakeup_alarm, next, ktime_add(next, slack));
}

static int sec_bat_get_adc_data(struct sec_bat_info *info, int adc_ch)
{
	int adc_data;
	int adc_max = 0;
	int adc_min = 0;
	int adc_total = 0;
	int i;

	for (i = 0; i < ADC_SAMPLING_CNT; i++) {
		adc_data = 10*info->get_temperature();

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

	return (adc_total - adc_max - adc_min) / (ADC_SAMPLING_CNT - 2);

}

static inline int sec_bat_read_temp(struct sec_bat_info *info)
{
#ifdef CONFIG_SENSORS_NCT1008
	return (10*info->get_temperature());
#else
	return sec_bat_get_adc_data(info, info->adc_channel);
#endif
}

static int sec_bat_get_temp(struct sec_bat_info *info)
{
	int temp = info->batt_temp;
	int temp_adc = sec_bat_read_temp(info);
	int health = info->batt_health;
	int low = 0;
	int high = info->adc_arr_size - 1;
	int mid = 0;

#ifdef CONFIG_SENSORS_NCT1008
	temp = temp_adc;
#else
	if (!info->adc_table || !info->adc_arr_size) {
		/* using fake temp*/
		info->batt_temp = 250;
		return -ENODATA;
	}

	while (low <= high) {
		mid = (low + high) / 2 ;
		if (info->adc_table[mid].adc > temp_adc)
			high = mid - 1;
		else if (info->adc_table[mid].adc < temp_adc)
			low = mid + 1;
		else
			break;
	}

	temp = info->adc_table[mid].temperature;
#endif


	info->batt_temp = temp;
	if (temp >= HIGH_BLOCK_TEMP) {
		if (health != POWER_SUPPLY_HEALTH_OVERHEAT &&
		    health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			info->batt_temp_high_cnt++;
	} else if (temp <= HIGH_RECOVER_TEMP && temp >= LOW_RECOVER_TEMP) {
		if (health == POWER_SUPPLY_HEALTH_OVERHEAT ||
		    health == POWER_SUPPLY_HEALTH_COLD)
			info->batt_temp_recover_cnt++;
	} else if (temp <= LOW_BLOCK_TEMP) {
		if (health != POWER_SUPPLY_HEALTH_COLD &&
		    health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			info->batt_temp_low_cnt++;
	} else {
		info->batt_temp_high_cnt = 0;
		info->batt_temp_low_cnt = 0;
		info->batt_temp_recover_cnt = 0;
	}

   if (info->batt_temp_high_cnt >= TEMP_BLOCK_COUNT) {
	   info->batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
	   info->batt_temp_high_cnt = 0;
   } else if (info->batt_temp_low_cnt >= TEMP_BLOCK_COUNT) {
	   info->batt_health = POWER_SUPPLY_HEALTH_COLD;
	   info->batt_temp_low_cnt = 0;
   } else if (info->batt_temp_recover_cnt >= TEMP_BLOCK_COUNT) {
	   info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
	   info->batt_temp_recover_cnt = 0;
   }

#ifdef __TEST_DEVICE_DRIVER__
	switch (bat_temp_force_state) {
	case 0:
		break;
	case 1:
		info->batt_health =POWER_SUPPLY_HEALTH_OVERHEAT;
		break;
	case 2:
		info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case 3:
		info->batt_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		break;
	default:
		break;
	}
#endif /* __TEST_DEVICE_DRIVER__ */

	dev_dbg(info->dev, "%s: temp=%d, adc=%d\n", __func__, temp, temp_adc);

	return temp;
}

static void sec_bat_update_info(struct sec_bat_info *info)
{
	struct power_supply *psy_main = power_supply_get_by_name(info->charger_name);
	struct power_supply *psy_sub = power_supply_get_by_name(info->sub_charger_name);
	union power_supply_propval value;

	if (!psy_main &&  !psy_sub) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	info->batt_soc = sec_bat_get_fuelgauge_data(info, FG_T_SOC);
	info->batt_vcell = sec_bat_get_fuelgauge_data(info, FG_T_VCELL);

	/* Remove this */
	if (info->use_sub_charger)
		psy_sub->get_property(psy_sub, POWER_SUPPLY_PROP_STATUS, &value);
	else
		psy_main->get_property(psy_main, POWER_SUPPLY_PROP_ONLINE, &value);
}

static void sec_bat_set_temp_info(struct sec_bat_info *info)
{
	struct power_supply *psy
		= power_supply_get_by_name(info->fuel_gauge_name);
	union power_supply_propval value;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get fuel gauge ps\n", __func__);
		return;
	}

	value.intval = info->batt_temp;
	psy->set_property(psy, POWER_SUPPLY_PROP_TEMP_AMBIENT, &value);
}

static void sec_set_time_for_charging(struct sec_bat_info *info, int mode)
{
	if (mode) {
		ktime_t ktime;
		struct timespec cur_time;

		ktime = alarm_get_elapsed_realtime();
		cur_time = ktime_to_timespec(ktime);

		/* record start time for abs timer */
		info->charging_start_time = cur_time.tv_sec;
	} else {
		/* initialize start time for abs timer */
		info->charging_start_time = 0;
	}
}

static int sec_bat_re_enable_charging_main(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval val_type, val_chg_current;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	/* disable charging */
	val_type.intval = POWER_SUPPLY_STATUS_DISCHARGING;
	ret = psy->set_property(psy, POWER_SUPPLY_PROP_STATUS, &val_type);
	if (ret) {
		dev_err(info->dev, "%s: fail to set charging status(%d)\n",
				__func__, ret);
		return ret;
	}

	/* Set charging current */
	switch (info->cable_type) {
	case CABLE_TYPE_USB:
	case CABLE_TYPE_DOCK:
		val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
		val_chg_current.intval = 460;	/* mA */
		break;
	case CABLE_TYPE_AC:
		val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
		val_chg_current.intval = 600;	/* mA */
		break;
	default:
		dev_err(info->dev, "%s: Invalid func use\n", __func__);
		return -EINVAL;
	}

	ret = psy->set_property(psy, POWER_SUPPLY_PROP_CURRENT_NOW,
				&val_chg_current);
	if (ret) {
		dev_err(info->dev, "%s: fail to set charging cur(%d)\n",
			__func__, ret);
		return ret;
	}

	/* enable charging */
	ret = psy->set_property(psy, POWER_SUPPLY_PROP_STATUS, &val_type);
	if (ret) {
		dev_err(info->dev, "%s: fail to set charging status(%d)\n",
				__func__, ret);
		return ret;
	}

	return 0;
}


static int sec_bat_enable_charging_main(struct sec_bat_info *info, bool enable)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval val_type, val_chg_current, val_topoff;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	if (enable) { /* Enable charging */
		switch (info->cable_type) {
		case CABLE_TYPE_USB:
		case CABLE_TYPE_DOCK:
			val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
			val_chg_current.intval = 460;	/* mA */
			break;
		case CABLE_TYPE_AC:
			val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
			val_chg_current.intval = 600;	/* mA */
			break;
		default:
			dev_err(info->dev, "%s: Invalid func use\n", __func__);
			return -EINVAL;
		}

		/* Set charging current */
		ret = psy->set_property(psy, POWER_SUPPLY_PROP_CURRENT_NOW,
					&val_chg_current);
		if (ret) {
			dev_err(info->dev, "%s: fail to set charging cur(%d)\n",
				__func__, ret);
			return ret;
		}

		/* Set topoff current */
		if(info->batt_full_status == BATT_2ND_FULL || info->batt_full_status == BATT_1ST_FULL) {
			/*set topoff current : 600mA *15%* = 90mA */
			val_topoff.intval = MAX8907C_TOPOFF_10PERCENT;
			ret = psy->set_property(psy, POWER_SUPPLY_PROP_CHARGE_FULL,
					&val_topoff);
		} else {
			/*set topoff current : 600mA *20%* = 120mA */
			val_topoff.intval = MAX8907C_TOPOFF_15PERCENT;
			ret = psy->set_property(psy, POWER_SUPPLY_PROP_CHARGE_FULL,
					&val_topoff);
		}

		if (ret) {
			dev_err(info->dev, "%s: fail to set topoff cur(%d)\n",
					__func__, ret);
			return ret;
		}

		/*Reset charging start time */
		sec_set_time_for_charging(info, 1);

	}
	else {/* Disable charging */
		val_type.intval = POWER_SUPPLY_STATUS_DISCHARGING;
		sec_set_time_for_charging(info, 0);
		info->recharging_status = false;
	}

	ret = psy->set_property(psy, POWER_SUPPLY_PROP_STATUS, &val_type);
	if (ret) {
		dev_err(info->dev, "%s: fail to set charging status(%d)\n",
				__func__, ret);
		return ret;
	}

	return 0;
}

static int sec_bat_enable_charging_sub(struct sec_bat_info *info, bool enable)
{
	struct power_supply *psy_main =
	    power_supply_get_by_name(info->charger_name);
	struct power_supply *psy_sub =
	    power_supply_get_by_name(info->sub_charger_name);
	union power_supply_propval val_type, val_chg_current;
	int ret;

	if (!psy_main || !psy_sub) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}


	if (enable) {		/* Enable charging */
		val_type.intval = POWER_SUPPLY_STATUS_DISCHARGING;
		ret = psy_main->set_property(psy_main, POWER_SUPPLY_PROP_STATUS,
					     &val_type);
		if (ret) {
			dev_err(info->dev, "%s: fail to set charging"
				" status-main(%d)\n", __func__, ret);
			return ret;
		}

		switch (info->cable_type) {
		case CABLE_TYPE_USB:
		case CABLE_TYPE_DOCK:
			val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
			val_chg_current.intval = 400;	/* mA */
			break;
		case CABLE_TYPE_AC:
			val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
			val_chg_current.intval = 660;	/* mA */
			break;
		default:
			dev_err(info->dev, "%s: Invalid func use\n", __func__);
			return -EINVAL;
		}

		/* Set charging current */
		ret = psy_sub->set_property(psy_sub,
					    POWER_SUPPLY_PROP_CURRENT_NOW,
					    &val_chg_current);
		if (ret) {
			dev_err(info->dev, "%s: fail to set charging cur(%d)\n",
				__func__, ret);
			return ret;
		}

		/*Reset charging start time */
		sec_set_time_for_charging(info, 1);
	} else {		/* Disable charging */
		val_type.intval = POWER_SUPPLY_STATUS_DISCHARGING;
		ret = psy_main->set_property(psy_main, POWER_SUPPLY_PROP_STATUS,
					     &val_type);
		if (ret) {
			dev_err(info->dev, "%s: fail to set charging"
				" status-main(%d)\n", __func__, ret);
			return ret;
		}

		sec_set_time_for_charging(info, 0);
	}

	ret = psy_sub->set_property(psy_sub, POWER_SUPPLY_PROP_STATUS,
				    &val_type);
	if (ret) {
		dev_err(info->dev, "%s: fail to set charging status(%d)\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static void sec_bat_check_vf(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	/* first vf check */
	ret = psy->get_property(psy, POWER_SUPPLY_PROP_PRESENT, &value);

	if (ret < 0) {
		dev_err(info->dev, "%s: fail to get status(%d)\n",
			__func__, ret);
		return;
	}

	if (value.intval == BAT_NOT_DETECTED) {
		msleep(5);
		udelay(12);

		/* second vf check */
		ret = psy->get_property(psy, POWER_SUPPLY_PROP_PRESENT, &value);

		if(value.intval == BAT_NOT_DETECTED) {
			msleep(5);
			udelay(15);

			/* third vf check */
			ret = psy->get_property(psy, POWER_SUPPLY_PROP_PRESENT, &value);
		}
	}

	if ((info->cable_type != CABLE_TYPE_NONE) &&
		(value.intval == BAT_NOT_DETECTED)) {
		info->present = BAT_NOT_DETECTED;
		info->batt_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
	} else {
		info->present = BAT_DETECTED;
		info->present_count = 0;
		if(info->batt_health == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
	}

	return;
}

static int sec_bat_enable_charging(struct sec_bat_info *info, bool enable)
{
	sec_bat_check_vf(info);
	if (enable && (info->batt_health != POWER_SUPPLY_HEALTH_GOOD)) {
		/* disable default charging */
		sec_bat_enable_charging_main(info, 0);
		sec_bat_enable_charging_sub(info, 0);
		info->charging_status =
		    POWER_SUPPLY_STATUS_NOT_CHARGING;

		dev_info(info->dev, "%s: Battery is NOT good!!!\n", __func__);
		return -EPERM;
	}

	if (info->use_sub_charger)
		return sec_bat_enable_charging_sub(info, enable);
	else
		return sec_bat_enable_charging_main(info, enable);
}

static int sec_bat_set_property(struct power_supply *ps,
			    enum power_supply_property psp,
			    const union power_supply_propval *val)
{
	struct sec_bat_info *info = container_of(ps, struct sec_bat_info,
			psy_bat);
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		/* TODO: topoff interrupt: called by charger IC */
		dev_info(info->dev, "%s: topoff intr\n", __func__);
		if (val->intval != POWER_SUPPLY_STATUS_FULL)
			return -EINVAL;

		if(info->charging_status != POWER_SUPPLY_STATUS_CHARGING &&
			info->charging_status != POWER_SUPPLY_STATUS_FULL) {
			dev_info(info->dev, "%s: battery is not charging status(%d)\n",
				__func__, info->charging_status);
			return -EINVAL;
		}

		if(info->batt_soc < 90) {
			dev_info(info->dev,
			"%s: battery level(%d) is low! not full charged\n",
				__func__, info->batt_soc);
			return -EINVAL;
		}

		if (info->use_sub_charger) {
			struct power_supply *psy =
				power_supply_get_by_name("fuelgauge");
			union power_supply_propval value;

			info->batt_full_status = BATT_2ND_FULL;
			info->charging_status = POWER_SUPPLY_STATUS_FULL;
			info->recharging_status = false;

			/* disable charging */
			sec_bat_enable_charging(info, 0);
			if (!psy) {
				pr_err("%s: Fail to get fuelgauge ps\n",
					__func__);
				return;
			}
			value.intval = POWER_SUPPLY_STATUS_FULL;
			psy->set_property(psy, POWER_SUPPLY_PROP_STATUS,
				&value);
		}
		else {
			if (info->batt_full_status == BATT_NOT_FULL) {

				info->batt_full_status = BATT_1ST_FULL;
				info->charging_status = POWER_SUPPLY_STATUS_FULL;

				/*set topoff current : 600mA *15%* = 90mA */
				value.intval = MAX8907C_TOPOFF_10PERCENT;
				psy->set_property(psy, POWER_SUPPLY_PROP_CHARGE_FULL, &value);

				/* re-enable charging */
				sec_bat_re_enable_charging_main(info);
			} else {
				info->batt_full_status = BATT_2ND_FULL;
				info->recharging_status = false;

				/* disable charging */
				sec_bat_enable_charging(info, 0);
			}
		}
		dev_info(info->dev, "%s: battery is full charged\n",	__func__);

		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		/* TODO: lowbatt interrupt: called by fuel gauge */
		dev_info(info->dev, "%s: lowbatt intr\n", __func__);
		if (val->intval != POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL)
			return -EINVAL;
		wake_lock(&info->monitor_wake_lock);
		queue_work(info->monitor_wqueue, &info->monitor_work);

		break;
	case POWER_SUPPLY_PROP_ONLINE:
		/* cable is attached or detached. called by USB switch(MUIC) */
		dev_info(info->dev, "%s: cable was changed(%d)\n", __func__,
				val->intval);
		switch (val->intval) {
		case POWER_SUPPLY_TYPE_BATTERY:
			info->cable_type = CABLE_TYPE_NONE;
			break;
		case POWER_SUPPLY_TYPE_MAINS:
			info->cable_type = CABLE_TYPE_AC;
			break;
		case POWER_SUPPLY_TYPE_USB:
			info->cable_type = CABLE_TYPE_USB;
			break;
		case POWER_SUPPLY_TYPE_DOCK:
			info->cable_type = CABLE_TYPE_DOCK;
			break;
		default:
			return -EINVAL;
		}
		wake_lock(&info->cable_wake_lock);
		queue_work(info->monitor_wqueue, &info->cable_work);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		if (val->intval != POWER_SUPPLY_HEALTH_OVERVOLTAGE &&
			val->intval != POWER_SUPPLY_HEALTH_GOOD)
			return -EINVAL;
		info->batt_health = val->intval;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static void sec_bat_cable_work(struct work_struct *work)
{
	struct sec_bat_info *info = container_of(work, struct sec_bat_info,
			cable_work);

	switch (info->cable_type) {
	case CABLE_TYPE_NONE:
		if (info->charging_status == POWER_SUPPLY_STATUS_FULL)
			sec_bat_set_fuelgauge_data(info, FG_T_ONLINE, 0);
		info->batt_full_status = BATT_NOT_FULL;
		info->recharging_status = false;
		info->charging_start_time = 0;
		info->batt_temp_high_cnt = 0;
		info->batt_temp_low_cnt = 0;
		info->batt_temp_recover_cnt = 0;
		info->charging_status = POWER_SUPPLY_STATUS_DISCHARGING;
		sec_bat_enable_charging(info, false);
		if (info->batt_health == POWER_SUPPLY_HEALTH_OVERVOLTAGE)
			info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		wake_lock_timeout(&info->vbus_wake_lock, HZ * 2);
		break;
	case CABLE_TYPE_USB:
		info->charging_status = POWER_SUPPLY_STATUS_CHARGING;
		sec_bat_enable_charging(info, true);
		wake_lock(&info->vbus_wake_lock);
		break;
	case CABLE_TYPE_AC:
	case CABLE_TYPE_DOCK:
		info->charging_status = POWER_SUPPLY_STATUS_CHARGING;
		sec_bat_enable_charging(info, true);
		break;
	default:
		dev_err(info->dev, "%s: Invalid cable type\n", __func__);
		break;;
	}

	power_supply_changed(&info->psy_ac);
	power_supply_changed(&info->psy_usb);

	info->last_poll = alarm_get_elapsed_realtime();
	sec_program_alarm(info, FAST_POLL);

	/* To notify framework layer, remaning 2 sec */
	wake_lock_timeout(&info->cable_wake_lock, HZ * 2);
}

static int sec_is_over_abs_time(struct sec_bat_info *info, unsigned int abs_time)
{
	ktime_t ktime;
	struct timespec cur_time;

	if (!info->charging_start_time)
		return 0;

	ktime = alarm_get_elapsed_realtime();
	cur_time = ktime_to_timespec(ktime);

	if (info->charging_start_time + abs_time < cur_time.tv_sec) {
		pr_info("Charging time out");
		return 1;
	} else
		return 0;
}

static void sec_bat_charging_time_management(struct sec_bat_info *info)
{
	unsigned long charging_time;

	if (info->charging_start_time == 0) {
		dev_dbg(info->dev, "%s: charging_start_time has never\
			 been used since initializing\n", __func__);
		return;
	}

	switch (info->batt_full_status) {
	case BATT_2ND_FULL:
		if (sec_is_over_abs_time(info, (unsigned long)RECHARGING_TIME) &&
				info->recharging_status == true) {
			sec_bat_enable_charging(info, false);
			info->recharging_status = false;
			info->charging_start_time = 0;
			info->batt_full_status = BATT_2ND_FULL;
			dev_info(info->dev, "%s: Recharging timer expired\n"
						, __func__);
		}
		break;
	case BATT_NOT_FULL:
	case BATT_1ST_FULL:
		if (sec_is_over_abs_time(info,
					(unsigned long)FULL_CHARGING_TIME)) {
			sec_bat_enable_charging(info, false);
			info->charging_start_time = 0;
			info->recharging_status = false;
			info->batt_full_status = BATT_2ND_FULL;
			dev_info(info->dev, "%s: Charging timer expired\n",
						 __func__);
		}
		break;
	default:
		dev_info(info->dev, "%s: Undefine Battery Status\n", __func__);
		return;
	}

	dev_dbg(info->dev, "Time past : %u secs\n",
		jiffies_to_msecs(charging_time)/1000);

	return;
}

static void sec_bat_monitor_work(struct work_struct *work)
{
	struct sec_bat_info *info = container_of(work, struct sec_bat_info,
			monitor_work);
	unsigned long flags;

	/* TODO: check charging time */
	sec_bat_charging_time_management(info);
	sec_bat_check_vf(info);
	sec_bat_update_info(info);
	sec_bat_get_temp(info);
	sec_bat_set_temp_info(info);

	switch (info->charging_status) {
	case POWER_SUPPLY_STATUS_FULL:
		if (info->batt_vcell < RECHARGING_VOLTAGE &&
				info->recharging_status == false) {
			info->recharging_status = true;
			sec_bat_enable_charging(info, true);

			dev_err(info->dev, "%s: Start Recharging, Vcell = %d\n"
						, __func__, info->batt_vcell);
		}
		break;
	case POWER_SUPPLY_STATUS_CHARGING:
		switch (info->batt_health) {
		case POWER_SUPPLY_HEALTH_OVERHEAT:
		case POWER_SUPPLY_HEALTH_COLD:
		case POWER_SUPPLY_HEALTH_OVERVOLTAGE:
		case POWER_SUPPLY_HEALTH_DEAD:
		case POWER_SUPPLY_HEALTH_UNSPEC_FAILURE:
			info->charging_status =
			    POWER_SUPPLY_STATUS_NOT_CHARGING;
			info->recharging_status = false;
			sec_bat_enable_charging(info, false);

			dev_info(info->dev, "%s: Not charging\n", __func__);
			break;
		default:
			break;
		}
		break;
	case POWER_SUPPLY_STATUS_DISCHARGING:
		dev_dbg(info->dev, "%s: Discharging\n", __func__);
		break;
	case POWER_SUPPLY_STATUS_NOT_CHARGING:
		if (info->batt_health == POWER_SUPPLY_HEALTH_GOOD) {
			dev_info(info->dev, "%s: recover health state\n",
					__func__);
			if (info->cable_type != CABLE_TYPE_NONE) {
				sec_bat_enable_charging(info, true);
				info->charging_status
					= POWER_SUPPLY_STATUS_CHARGING;
			} else
				info->charging_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
		}
		break;
	default:
		dev_info(info->dev, "%s: Undefined Battery Status\n", __func__);
		return;
	}

	dev_info(info->dev, "soc(%d), vcell(%d), temp(%d), charging(%d) full(%d) recharging(%d)\n",
		info->batt_soc, info->batt_vcell/1000, info->batt_temp/10,
		info->charging_status,info->batt_full_status,info->recharging_status);

	power_supply_changed(&info->psy_bat);

	/* prevent suspend before starting the alarm */
	local_irq_save(flags);
	info->last_poll = alarm_get_elapsed_realtime();
	sec_program_alarm(info, FAST_POLL);
	local_irq_restore(flags);

	wake_unlock(&info->monitor_wake_lock);

	return;
}

#define SEC_BATTERY_ATTR(_name)			\
{						\
	.attr = { .name = #_name,		\
		  .mode = 0664,		   },	\
	.show = sec_bat_show_property,		\
	.store = sec_bat_store,			\
}

static struct device_attribute sec_battery_attrs[] = {
	SEC_BATTERY_ATTR(batt_vol),
	SEC_BATTERY_ATTR(batt_soc),
	SEC_BATTERY_ATTR(batt_reset_soc),
	SEC_BATTERY_ATTR(batt_temp),
	SEC_BATTERY_ATTR(batt_temp_adc),
	SEC_BATTERY_ATTR(batt_charging_source),
	SEC_BATTERY_ATTR(batt_lp_charging),
	SEC_BATTERY_ATTR(video),
	SEC_BATTERY_ATTR(mp3),
	SEC_BATTERY_ATTR(batt_type),
};

enum {
	BATT_VOL = 0,
	BATT_SOC,
	BATT_RESET_SOC,
	BATT_TEMP,
	BATT_TEMP_ADC,
	BATT_CHARGING_SOURCE,
	BATT_LP_CHARGING,
	BATT_VIDEO,
	BATT_MP3,
	BATT_TYPE,
};

static ssize_t sec_bat_show_property(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct sec_bat_info *info = dev_get_drvdata(dev->parent);
	int i = 0, val;
	const ptrdiff_t off = attr - sec_battery_attrs;

	switch (off) {
	case BATT_VOL:
		val = sec_bat_get_fuelgauge_data(info, FG_T_VCELL);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;

	case BATT_SOC:
		val = sec_bat_get_fuelgauge_data(info, FG_T_SOC);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;

	case BATT_TEMP:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_temp);
		break;
	case BATT_TEMP_ADC:
		val = sec_bat_get_adc_data(info, info->adc_channel);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_CHARGING_SOURCE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				info->cable_type);
		break;

	case BATT_LP_CHARGING:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				info->batt_lp_charging);
		break;
	case BATT_VIDEO:
		/* TODO */
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 0);
		break;
	case BATT_MP3:
		/* TODO */
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 0);
		break;
	case BATT_TYPE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n", "SDI_SDI");
		break;
	default:
		i = -EINVAL;
	}

	return i;
}

extern void max17043_reset_soc();

static ssize_t sec_bat_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct sec_bat_info *info = dev_get_drvdata(dev->parent);
	int x = 0;
	int ret = 0;
	const ptrdiff_t off = attr - sec_battery_attrs;

	switch (off) {
	case BATT_RESET_SOC:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if (x == 1)
				max17043_reset_soc();
			ret = count;
		}
		break;
	case BATT_LP_CHARGING:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if (x == 1) {
				info->batt_lp_charging = 1;
				lpm_mode_flag = 1;
			}
			ret = count;
		}
		printk("%s : batt_lp_charging = %d\n",__func__, info->batt_lp_charging);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int sec_bat_create_attrs(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(sec_battery_attrs); i++) {
		rc = device_create_file(dev, &sec_battery_attrs[i]);
		if (rc)
			goto sec_attrs_failed;
	}
	goto succeed;

sec_attrs_failed:
	while (i--)
		device_remove_file(dev, &sec_battery_attrs[i]);
succeed:
	return rc;
}

#ifdef __TEST_DEVICE_DRIVER__
#define SEC_TEST_ATTR(_name)\
{\
	.attr = { .name = #_name, .mode = S_IRUGO | (S_IWUSR | S_IWGRP) },\
	.show = sec_batt_test_show_property,\
	.store = sec_batt_test_store,\
}

static struct device_attribute sec_batt_test_attrs[] = {
	SEC_TEST_ATTR(control_temp),
};

enum {
	CTRL_TEMP = 0,
};

static int sec_batt_test_create_attrs(struct device * dev)
{
        int i, ret;

        for (i = 0; i < ARRAY_SIZE(sec_batt_test_attrs); i++) {
                ret = device_create_file(dev, &sec_batt_test_attrs[i]);
                if (ret)
                        goto sec_attrs_failed;
        }
        goto succeed;

sec_attrs_failed:
        while (i--)
                device_remove_file(dev, &sec_batt_test_attrs[i]);
succeed:
        return ret;
}

static ssize_t sec_batt_test_show_property(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf)
{
	int i = 0;
	const ptrdiff_t off = attr - sec_batt_test_attrs;

	switch (off) {
	default:
		i = -EINVAL;
	}

	return i;
}

static ssize_t sec_batt_test_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	int mode = 0;
	int ret = 0;
	const ptrdiff_t off = attr - sec_batt_test_attrs;

	switch (off) {
	case CTRL_TEMP:
		if (sscanf(buf, "%d\n", &mode) == 1) {
			dev_info(dev, "%s: control temp (%d)\n", __func__, mode);
			bat_temp_force_state = mode;
			ret = count;
		}
		break;

	default:
		ret = -EINVAL;
	}

	return ret;
}
#endif /* __TEST_DEVICE_DRIVER__ */

static __devinit int sec_bat_probe(struct platform_device *pdev)
{
	struct sec_bat_platform_data *pdata = dev_get_platdata(&pdev->dev);
	struct sec_bat_info *info;
	int ret = 0;

	dev_info(&pdev->dev, "%s: SEC Battery Driver Loading\n", __func__);

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	platform_set_drvdata(pdev, info);

	info->dev = &pdev->dev;
	if (!pdata->fuel_gauge_name || !pdata->charger_name) {
		dev_err(info->dev, "%s: no fuel gauge or charger name\n",
				__func__);
		goto err_kfree;
	}
	info->fuel_gauge_name = pdata->fuel_gauge_name;
	info->charger_name = pdata->charger_name;
	if (system_rev >= HWREV_FOR_EXTERNEL_CHARGER) {
		dev_info(&pdev->dev, "%s: use sub-charger\n", __func__);
	} else {
		dev_info(&pdev->dev, "%s: use main charger\n", __func__);
	}

	if (pdata->sub_charger_name) {
		info->sub_charger_name = pdata->sub_charger_name;
		if (system_rev >= HWREV_FOR_EXTERNEL_CHARGER)
			info->use_sub_charger = true;
		else
			info->use_sub_charger = false;
	}

	info->get_init_cable_state = pdata->get_init_cable_state;
	info->get_temperature = pdata->get_temperature;

	info->adc_arr_size = pdata->adc_arr_size;
	info->adc_table = pdata->adc_table;
	info->adc_channel = pdata->adc_channel;

	info->present = BAT_DETECTED;
	info->present_count = 0;

	info->psy_bat.name = "battery",
	info->psy_bat.type = POWER_SUPPLY_TYPE_BATTERY,
	info->psy_bat.properties = sec_battery_props,
	info->psy_bat.num_properties = ARRAY_SIZE(sec_battery_props),
	info->psy_bat.get_property = sec_bat_get_property,
	info->psy_bat.set_property = sec_bat_set_property,

	info->psy_usb.name = "usb",
	info->psy_usb.type = POWER_SUPPLY_TYPE_USB,
	info->psy_usb.supplied_to = supply_list,
	info->psy_usb.num_supplicants = ARRAY_SIZE(supply_list),
	info->psy_usb.properties = sec_power_props,
	info->psy_usb.num_properties = ARRAY_SIZE(sec_power_props),
	info->psy_usb.get_property = sec_usb_get_property,

	info->psy_ac.name = "ac",
	info->psy_ac.type = POWER_SUPPLY_TYPE_MAINS,
	info->psy_ac.supplied_to = supply_list,
	info->psy_ac.num_supplicants = ARRAY_SIZE(supply_list),
	info->psy_ac.properties = sec_power_props,
	info->psy_ac.num_properties = ARRAY_SIZE(sec_power_props),
	info->psy_ac.get_property = sec_ac_get_property;

	wake_lock_init(&info->vbus_wake_lock, WAKE_LOCK_SUSPEND,
			"vbus_present");
	wake_lock_init(&info->monitor_wake_lock, WAKE_LOCK_SUSPEND,
			"sec-battery-monitor");
	wake_lock_init(&info->cable_wake_lock, WAKE_LOCK_SUSPEND,
			"sec-battery-cable");


	info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
	info->charging_start_time = 0;
	info->batt_lp_charging = 0;

	/* init power supplier framework */
	ret = power_supply_register(&pdev->dev, &info->psy_bat);
	if (ret) {
		dev_err(info->dev, "%s: failed to register psy_bat\n",
				__func__);
		goto err_wake_lock;
	}

	ret = power_supply_register(&pdev->dev, &info->psy_usb);
	if (ret) {
		dev_err(info->dev, "%s: failed to register psy_usb\n",
				__func__);
		goto err_supply_unreg_bat;
	}

	ret = power_supply_register(&pdev->dev, &info->psy_ac);
	if (ret) {
		dev_err(info->dev, "%s: failed to register psy_ac\n",
				__func__);
		goto err_supply_unreg_usb;
	}

	/* create sec detail attributes */
	sec_bat_create_attrs(info->psy_bat.dev);


#ifdef __TEST_DEVICE_DRIVER__
	sec_batt_test_create_attrs(info->psy_ac.dev);
#endif /* __TEST_DEVICE_DRIVER__ */

	info->last_poll = alarm_get_elapsed_realtime();
	alarm_init(&info->wakeup_alarm, ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP,
		sec_bat_alarm);

	info->monitor_wqueue =
		create_freezable_workqueue(dev_name(&pdev->dev));
	if (!info->monitor_wqueue) {
		dev_err(info->dev, "%s: fail to create workqueue\n", __func__);
		goto err_supply_unreg_ac;
	}

	INIT_WORK(&info->monitor_work, sec_bat_monitor_work);
	INIT_WORK(&info->cable_work, sec_bat_cable_work);

	/* get initial cable status */
	info->get_init_cable_state(&info->psy_bat);

	if (pdata->get_batt_level)
		pdata->get_batt_level(&info->batt_soc);

	/* for playlpm init value */
	sec_bat_update_info(info);
	printk("BAT :  capacity(%d), health(%d), status(%d), voltage(%d)\n",
		info->batt_soc, info->batt_health, info->charging_status, info->batt_vcell);

	wake_lock(&info->monitor_wake_lock);
	queue_work(info->monitor_wqueue, &info->monitor_work);

	return 0;

err_supply_unreg_ac:
	power_supply_unregister(&info->psy_ac);
err_supply_unreg_usb:
	power_supply_unregister(&info->psy_usb);
err_supply_unreg_bat:
	power_supply_unregister(&info->psy_bat);
err_wake_lock:
	wake_lock_destroy(&info->vbus_wake_lock);
	wake_lock_destroy(&info->monitor_wake_lock);
	wake_lock_destroy(&info->cable_wake_lock);
err_kfree:
	kfree(info);

	return ret;
}

static int __devexit sec_bat_remove(struct platform_device *pdev)
{
	struct sec_bat_info *info = platform_get_drvdata(pdev);

	flush_workqueue(info->monitor_wqueue);
	destroy_workqueue(info->monitor_wqueue);


	power_supply_unregister(&info->psy_bat);
	power_supply_unregister(&info->psy_usb);
	power_supply_unregister(&info->psy_ac);

	wake_lock_destroy(&info->vbus_wake_lock);
	wake_lock_destroy(&info->monitor_wake_lock);
	wake_lock_destroy(&info->cable_wake_lock);

	kfree(info);

	return 0;
}

static int sec_bat_suspend(struct device *dev)
{
	struct sec_bat_info *info = dev_get_drvdata(dev);

	cancel_work_sync(&info->monitor_work);

	if (info->cable_type == CABLE_TYPE_NONE) {
		sec_program_alarm(info, SLOW_POLL);
		info->slow_poll = 1;
	}

	return 0;
}

static int sec_bat_resume(struct device *dev)
{
	struct sec_bat_info *info = dev_get_drvdata(dev);

	if (info->slow_poll) {
		sec_program_alarm(info, FAST_POLL);
		info->slow_poll = 0;
	}

	wake_lock(&info->monitor_wake_lock);
	queue_work(info->monitor_wqueue, &info->monitor_work);

	return 0;
}

static const struct dev_pm_ops sec_bat_pm_ops = {
	.prepare	= sec_bat_suspend,
	.complete	= sec_bat_resume,
};

static struct platform_driver sec_bat_driver = {
	.driver = {
		.name = "sec-battery",
		.owner = THIS_MODULE,
		.pm = &sec_bat_pm_ops,
	},
	.probe = sec_bat_probe,
	.remove = __devexit_p(sec_bat_remove),
};

static int __init sec_bat_init(void)
{
	return platform_driver_register(&sec_bat_driver);
}

static void __exit sec_bat_exit(void)
{
	platform_driver_unregister(&sec_bat_driver);
}

late_initcall(sec_bat_init);
module_exit(sec_bat_exit);

MODULE_DESCRIPTION("SEC battery driver");
MODULE_AUTHOR("<ms925.kim@samsung.com>");
MODULE_AUTHOR("<joshua.chang@samsung.com>");
MODULE_LICENSE("GPL");
