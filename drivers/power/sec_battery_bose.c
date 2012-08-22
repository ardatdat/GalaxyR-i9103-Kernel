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

#if defined (CONFIG_MACH_BOSE_ATT)
#define chg_cutoff_count_index 7
#define recharging_count_index 1

int chg_cutoff_count = chg_cutoff_count_index;
int chg_cutoff = 0;
bool chg_cutoff_interrupt = false;
int recharging_count = recharging_count_index;
bool is_bat_exist = false;
#endif

#define HWREV_FOR_EXTERNEL_CHARGER	7

#define POLLING_INTERVAL	(30 * 1000)

#define FAST_POLL	50	/* 40 sec */
#define SLOW_POLL	(30*60)	/* 30 min */
//#define TOTAL_EVENT_TIME 	(40*60)
#define TOTAL_EVENT_TIME 	(40*60*1000 )

#if 0
#define FULL_CHARGING_TIME	(6 * 60 *  60 * HZ)	/* 6hr */
#define RECHARGING_TIME		(90 * 60 * HZ)		/* 1.5hr */
#endif
#define FULL_CHARGING_TIME	(6 * 60 *  60)	/* 6hr */
#define RECHARGING_TIME		(90 * 60 )/* 1.5hr */

#define RECHARGING_VOLTAGE	(4130 * 1000)		/* 4.15 V */

#define FG_T_SOC		0
#define FG_T_VCELL		1
#define FG_T_TEMPER		2
#define FG_T_ONLINE		3

#if 0
#define HIGH_BLOCK_TEMP		    520  //  46 _C
#define HIGH_RECOVER_TEMP	          460  //  40 _C
#define LOW_BLOCK_TEMP		          50  //  0 _C
#define LOW_RECOVER_TEMP		   80   //  3 _C

#define EVENT_HIGH_BLOCK_TEMP		680  // 63 _C
#define EVENT_HIGH_RECOVER_TEMP	640  // 58 _C
#else	// Changed by HW 2011.07.25
#define HIGH_BLOCK_TEMP		    520  //  46 _C
#define HIGH_RECOVER_TEMP	          410  //  40 _C
#define LOW_BLOCK_TEMP		          (-40)  //  -5 _C
#define LOW_RECOVER_TEMP		   10   //  0 _C
#define LOW_BLOCK_TEMP_LPM		          (-50)  //  -5 _C
#define LOW_RECOVER_TEMP_LPM		   30   //  0 _C

#define EVENT_HIGH_BLOCK_TEMP		660  // 65 _C
#define EVENT_HIGH_RECOVER_TEMP	440  // 43 _C

#endif

#define BAT_DET_COUNT		0
#define ADC_SAMPLING_CNT	6

// edit seojw event charging
#define OFFSET_VIBRATOR_ON			(0x1 << 0)
#define OFFSET_CAMERA_ON			(0x1 << 1)
#define OFFSET_MP3_PLAY			(0x1 << 2)
#define OFFSET_VIDEO_PLAY			(0x1 << 3)
#define OFFSET_VOICE_CALL_2G		(0x1 << 4)
#define OFFSET_VOICE_CALL_3G		(0x1 << 5)
#define OFFSET_DATA_CALL			(0x1 << 6)
#define OFFSET_LCD_ON				(0x1 << 7)
#define OFFSET_TA_ATTACHED		(0x1 << 8)
#define OFFSET_CAM_FLASH			(0x1 << 9)
#define OFFSET_BOOTING				(0x1 << 10)
#define OFFSET_WIFI					(0x1 << 11)
#define OFFSET_GPS					(0x1 << 12)

#define COMPENSATE_VIBRATOR		19
#define COMPENSATE_CAMERA			25
#define COMPENSATE_MP3				17
#define COMPENSATE_VIDEO			28
#define COMPENSATE_VOICE_CALL_2G	13
#define COMPENSATE_VOICE_CALL_3G	14
#define COMPENSATE_DATA_CALL		25
#define COMPENSATE_LCD				0
#define COMPENSATE_TA				0
#define COMPENSATE_CAM_FALSH		0
#define COMPENSATE_BOOTING		52
#define COMPENSATE_WIFI			0
#define COMPENSATE_GPS				0
enum batt_full_t {
	BATT_NOT_FULL = 0,
	BATT_1ST_FULL,
	BATT_2ND_FULL,
};

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
	unsigned int		batt_health;
	unsigned int		batt_vcell;
	unsigned int		batt_soc;
	unsigned int           batt_vf;

	unsigned int		charging_status;
	unsigned int 		batt_lp_charging;
	unsigned int 		event_dev_state;		// eidt seojw event charging
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
    int (*get_BAT_VF_adcvalue) (void);
	bool		slow_poll;
	ktime_t		last_poll;
};

unsigned int gv_backup_batt_soc=0;
unsigned int gv_backup_batt_vcell=0;
int gv_backup_batt_temp=0;
unsigned int gv_backup_charging_status=0;
enum batt_full_t gv_backup_batt_full_status=0;
unsigned int gv_backup_recharging_status=0;
enum cable_type_t  gv_backup_cable_type=0;
unsigned int gv_backup_batt_health=0;
unsigned int gv_backup_batt_lp_charging=0;
unsigned int gv_backup_event_dev_state=0;
bool gv_backup_use_sub_charger=0;
unsigned int gv_backup_present = 0;
/* gv variable */
struct sec_bat_info *gv_info;  // edit seojw event charging
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
	POWER_SUPPLY_PROP_CURRENT_AVG,
};

static enum power_supply_property sec_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int event_occur = 0;
static unsigned int event_start_time_msec =0;
static unsigned int event_total_time_msec =0;
static backup_batt_info_data(struct sec_bat_info *info)
{
	printk("[BAT] %s \n", __func__);

	gv_backup_batt_soc = info->batt_soc;
	gv_backup_batt_vcell= info->batt_vcell;
	gv_backup_batt_temp=info->batt_temp;
	gv_backup_charging_status=info->charging_status;
	gv_backup_batt_full_status=info->batt_full_status;
	gv_backup_recharging_status=info->recharging_status;
	gv_backup_cable_type=info->cable_type;
	gv_backup_batt_health=info->batt_health;
	gv_backup_batt_lp_charging=info->batt_lp_charging;
	gv_backup_event_dev_state=info->event_dev_state;
	gv_backup_use_sub_charger=info->use_sub_charger;
	gv_backup_present=info->present;
};
static void sec_set_time_for_event(int mode)
{
	if (mode)
	{
		/* record start time for abs timer */
		event_start_time_msec = jiffies_to_msecs(jiffies);
	}
	else
	{
		/* initialize start time for abs timer */
		event_start_time_msec = 0;
		event_total_time_msec = 0;
		//pr_info("[BAT]:%s: start_time_msec(%u)\n", __func__, event_start_time_msec);
	}

}
static int is_over_event_time(struct sec_bat_info *info)
{
	unsigned int total_time=0;

	if(!event_start_time_msec){
		return true;
	}
#if 0

      total_time = TOTAL_EVENT_TIME;

	if(jiffies_to_msecs(jiffies) >= event_start_time_msec){
		event_total_time_msec = jiffies_to_msecs(jiffies) - event_start_time_msec;
	}else{
		event_total_time_msec = 0xFFFFFFFF - event_start_time_msec + jiffies_to_msecs(jiffies);
	}

	if (event_total_time_msec > total_time && event_start_time_msec){
		printk("[BAT]:%s:abs time is over.:event_start_time_msec=%u, event_total_time_msec=%u\n", __func__, event_start_time_msec, event_total_time_msec);
		return true;
	}else{
		return false;
	}
//      total_time = TOTAL_EVENT_TIME_HZ;
      total_time = (3 * 60 * HZ);

	if(time_after((unsigned long)jiffies, (unsigned long)(event_start_time_msec + total_time)))
	{
		printk("%s abs time over (abs time: %u, start time: %u)\n",__func__, total_time, event_start_time_msec);
		sec_set_time_for_event(0);
		return true;
	}
	else {
		printk("%s abs time NOT over (abs time: %u, start time: %u)\n",__func__, total_time, event_start_time_msec);
		return false;

	}
#else

      total_time = TOTAL_EVENT_TIME;

	if(jiffies_to_msecs(jiffies) >= event_start_time_msec){
		event_total_time_msec = jiffies_to_msecs(jiffies) - event_start_time_msec;
	}else{
		event_total_time_msec = 0xFFFFFFFF - event_start_time_msec + jiffies_to_msecs(jiffies);
	}

	if (event_total_time_msec > total_time && event_start_time_msec){
//		printk("[BAT]:%s:abs time is over.:event_start_time_msec=%u, event_total_time_msec=%u\n", __func__, event_start_time_msec, event_total_time_msec);
		sec_set_time_for_event(0);
		return true;
	}else{
		dev_info(info->dev, "%s : NOT over! start_time(%u), total_time(%u)\n", __func__, event_start_time_msec, event_total_time_msec);
		return false;
	}



#endif

}
static void sec_bat_set_compesation(int mode, int offset, int compensate_value)
{
	if (mode){
		if (!(gv_info->event_dev_state & offset)){
				gv_info->event_dev_state |= offset;
		}
	}else{
		if (gv_info->event_dev_state & offset){
				gv_info->event_dev_state &= ~offset;
		}
	}

	printk("%s : event_dev_state(0x%04x) \n", __func__, gv_info->event_dev_state);
}

static int sec_bat_get_compensation(int offset)
{
	if (gv_info->event_dev_state & offset)
		return 1;
	else
		return 0;
}

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
		}
		val->intval = info->batt_soc;
		if (val->intval == -1)
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		val->intval = 1;
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

	printk("%s\n", __func__);
	wake_lock(&info->monitor_wake_lock);
	queue_work(info->monitor_wqueue, &info->monitor_work);
}

static void sec_program_alarm(struct sec_bat_info *info, int seconds)
{
	ktime_t low_interval = ktime_set(seconds - 10, 0);
	ktime_t slack = ktime_set(20, 0);
	ktime_t next;

	printk("%s \n", __func__);
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
	int is_over_time = 0;
	int update = 0;
	int old_health = 0;
	unsigned int event_temper_case = 0;

	extern int charging_mode_from_boot;

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

	if(info->cable_type == CABLE_TYPE_NONE) {
		info->batt_health= POWER_SUPPLY_HEALTH_GOOD;
		return temp;
	}

	event_temper_case = OFFSET_MP3_PLAY | OFFSET_VOICE_CALL_2G | OFFSET_VOICE_CALL_3G | OFFSET_DATA_CALL | OFFSET_VIDEO_PLAY |
                       OFFSET_CAMERA_ON | OFFSET_WIFI | OFFSET_GPS;


#if 0
//edit seojw event charging
	if (info->event_dev_state & event_temper_case) {
		if (temp >= EVENT_HIGH_BLOCK_TEMP){
			if ( health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			info->batt_health= POWER_SUPPLY_HEALTH_OVERHEAT;

		}else if (temp < EVENT_HIGH_BLOCK_TEMP && temp > LOW_BLOCK_TEMP){
			if ( (health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) && (health != POWER_SUPPLY_HEALTH_COLD) &&  (health != POWER_SUPPLY_HEALTH_OVERHEAT) ){
				info->batt_health= POWER_SUPPLY_HEALTH_GOOD;

			}else if( (temp <= EVENT_HIGH_RECOVER_TEMP) &&  (temp >= LOW_RECOVER_TEMP) ){
				if (  (health  == POWER_SUPPLY_HEALTH_COLD) || (health == POWER_SUPPLY_HEALTH_OVERHEAT) ){
					info->batt_health= POWER_SUPPLY_HEALTH_GOOD;
				}
		}

		}else if (temp <= LOW_BLOCK_TEMP){
			if ( health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE){
				info->batt_health= POWER_SUPPLY_HEALTH_COLD;
			}
		}
		 if(event_occur == false)
			event_occur = true;
//  nomal
     }else{
		if(event_occur == true){
			sec_set_time_for_event(1);
			event_occur = false;
		}

		if (temp >= HIGH_BLOCK_TEMP){
			if ( health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE  &&  is_over_event_time() ){
				info->batt_health= POWER_SUPPLY_HEALTH_OVERHEAT;
				sec_set_time_for_event(0);
			}
		}else if(temp < HIGH_BLOCK_TEMP && temp > LOW_BLOCK_TEMP){
			if ( (health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) && (health != POWER_SUPPLY_HEALTH_COLD) &&  (health != POWER_SUPPLY_HEALTH_OVERHEAT) ){
				info->batt_health= POWER_SUPPLY_HEALTH_GOOD;
			}else if( (temp <= HIGH_RECOVER_TEMP) &&  (temp >= LOW_RECOVER_TEMP) ){
				if ( (health  == POWER_SUPPLY_HEALTH_COLD) || (health == POWER_SUPPLY_HEALTH_OVERHEAT) ){
					info->batt_health= POWER_SUPPLY_HEALTH_GOOD;
				}
                     }
		}else if (temp <= LOW_BLOCK_TEMP){
			if ( health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE){
				info->batt_health=POWER_SUPPLY_HEALTH_COLD;
			}
		}
     }

	if(info->batt_health != POWER_SUPPLY_HEALTH_GOOD) {
		dev_info(info->dev, "event_occur(%d), event case(0x%04x), temp(%d)\n",
			event_occur, info->event_dev_state, temp);
	}

	dev_dbg(info->dev, "%s: temp=%d, adc=%d\n", __func__, temp, temp_adc);

#else
	if(info->event_dev_state & event_temper_case) {
		sec_set_time_for_event(1);
		is_over_time = false;
	} else {
		is_over_time =  is_over_event_time(info);
	}
	old_health = info->batt_health;

	if(charging_mode_from_boot)
	{
		if (info->event_dev_state & event_temper_case || is_over_time == false ) {
			if (temp >= EVENT_HIGH_BLOCK_TEMP) {
				if (info->batt_health != POWER_SUPPLY_HEALTH_OVERHEAT &&
						info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
					update=2;
				}
			} else if (temp <= EVENT_HIGH_RECOVER_TEMP &&
					//temp >= LOW_RECOVER_TEMP) {
					temp >= LOW_RECOVER_TEMP_LPM) {
				if (info->batt_health == POWER_SUPPLY_HEALTH_OVERHEAT ||
						info->batt_health == POWER_SUPPLY_HEALTH_COLD)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
					update=2;
				}
			//} else if (temp <= LOW_BLOCK_TEMP) {
			} else if (temp <= LOW_BLOCK_TEMP_LPM) {
				if (info->batt_health != POWER_SUPPLY_HEALTH_COLD &&
						info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_COLD;
					update=2;
				}
			}
		} else {
			if (temp >= HIGH_BLOCK_TEMP) {
				if (info->batt_health != POWER_SUPPLY_HEALTH_OVERHEAT &&
						info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
					update=1;
				}
			} else if (temp <= HIGH_RECOVER_TEMP &&
					//temp >= LOW_RECOVER_TEMP) {
					temp >= LOW_RECOVER_TEMP_LPM) {
				if (info->batt_health == POWER_SUPPLY_HEALTH_OVERHEAT ||
						info->batt_health == POWER_SUPPLY_HEALTH_COLD)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
					update=1;
				}
			//} else if (temp <= LOW_BLOCK_TEMP) {
			} else if (temp <= LOW_BLOCK_TEMP_LPM) {
				if (info->batt_health != POWER_SUPPLY_HEALTH_COLD &&
						info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_COLD;
					update=1;
				}
			}
		}

	}
	else
	{
		if (info->event_dev_state & event_temper_case || is_over_time == false ) {
			if (temp >= EVENT_HIGH_BLOCK_TEMP) {
				if (info->batt_health != POWER_SUPPLY_HEALTH_OVERHEAT &&
						info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
					update=2;
				}
			} else if (temp <= EVENT_HIGH_RECOVER_TEMP &&
					temp >= LOW_RECOVER_TEMP) {
				if (info->batt_health == POWER_SUPPLY_HEALTH_OVERHEAT ||
						info->batt_health == POWER_SUPPLY_HEALTH_COLD)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
					update=2;
				}
			} else if (temp <= LOW_BLOCK_TEMP) {
				if (info->batt_health != POWER_SUPPLY_HEALTH_COLD &&
						info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_COLD;
					update=2;
				}
			}
		} else {
			if (temp >= HIGH_BLOCK_TEMP) {
				if (info->batt_health != POWER_SUPPLY_HEALTH_OVERHEAT &&
						info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
					update=1;
				}
			} else if (temp <= HIGH_RECOVER_TEMP &&
					temp >= LOW_RECOVER_TEMP) {
				if (info->batt_health == POWER_SUPPLY_HEALTH_OVERHEAT ||
						info->batt_health == POWER_SUPPLY_HEALTH_COLD)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
					update=1;
				}
			} else if (temp <= LOW_BLOCK_TEMP) {
				if (info->batt_health != POWER_SUPPLY_HEALTH_COLD &&
						info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				{
					info->batt_health = POWER_SUPPLY_HEALTH_COLD;
					update=1;
				}
			}
		}
	}

	if (update != 0 || info->event_dev_state & event_temper_case) {
		dev_info(info->dev, "health(%d->%d), temp(%d), update(%d), event_state(0x%04x), regarding_event(%d), event_total_time(%d))\n",
			old_health, info->batt_health, temp, update, info->event_dev_state, (!is_over_time), event_total_time_msec/1000);
	}

#endif

	return temp;
}

static void sec_bat_update_info(struct sec_bat_info *info)
{
	struct power_supply *psy_main = power_supply_get_by_name(info->charger_name);
	struct power_supply *psy_sub = power_supply_get_by_name(info->sub_charger_name);
	union power_supply_propval value;

	if (!psy_main &&  !psy_sub) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return;
	}

	info->batt_soc = sec_bat_get_fuelgauge_data(info, FG_T_SOC);
	info->batt_vcell = sec_bat_get_fuelgauge_data(info, FG_T_VCELL);

	/* Remove this */
	if (info->use_sub_charger)
		psy_sub->get_property(psy_sub, POWER_SUPPLY_PROP_STATUS, &value);
	else
		psy_main->get_property(psy_main, POWER_SUPPLY_PROP_ONLINE, &value);

	return;
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
			val_chg_current.intval = 640;	/* mA */
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

#if defined (CONFIG_MACH_BOSE_ATT)
#define BATT_VF_MAX   1100
#define BATT_VF_MIN      600

static void sec_bat_check_vf(struct sec_bat_info *info)
{
	int ret  = 0;

	if (info->get_BAT_VF_adcvalue)
		info->batt_vf = info->get_BAT_VF_adcvalue();
       else
	       dev_err(info->dev, "%s: fail to set charging status(%d)\n",__func__, ret);

       if(info->cable_type != CABLE_TYPE_NONE){
		if((info->batt_vf < BATT_VF_MIN) ||  ( info->batt_vf > BATT_VF_MAX)){
			if (info->present_count < BAT_DET_COUNT){
				info->present_count++;
				is_bat_exist = false;
			}
			else
			{
				info->present = BAT_NOT_DETECTED;
				info->batt_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
				is_bat_exist = false;
				dev_info(info->dev, "%s: batt not detected batt_vf(%d) min(%d), max(%d)\n",
					__func__, info->batt_vf, BATT_VF_MIN, BATT_VF_MAX);
			}
		}
		else
		{
			info->present = BAT_DETECTED;
			info->present_count = 0;
			if(info->batt_health == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) {
				info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
				dev_info(info->dev, "%s: batt detected! recover health! batt_vf(%d) min(%d), max(%d)\n",
					__func__, info->batt_vf, BATT_VF_MIN, BATT_VF_MAX);

			}
			is_bat_exist = true;
		}
       }
       else
       {
		info->present = BAT_DETECTED;
		info->present_count = 0;
		if(info->batt_health == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		is_bat_exist = true;
	}

	return;
}

#else
static void sec_bat_check_vf(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	ret = psy->get_property(psy, POWER_SUPPLY_PROP_PRESENT, &value);

	if (ret < 0) {
		dev_err(info->dev, "%s: fail to get status(%d)\n",
			__func__, ret);
		return;
	}

	if ((info->cable_type != CABLE_TYPE_NONE) &&
		(value.intval == BAT_NOT_DETECTED)) {
		if (info->present_count < BAT_DET_COUNT)
			info->present_count++;
		else {
			info->present = BAT_NOT_DETECTED;
			info->batt_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		}
	} else {
		info->present = BAT_DETECTED;
		info->present_count = 0;
		if(info->batt_health == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
	}

	return;
}
#endif

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
			dev_info(info->dev, "%s: battery level(%d) is low! not full charged\n",
				__func__, info->batt_soc);
			return -EINVAL;
		}

		if (info->use_sub_charger) {
			info->batt_full_status = BATT_2ND_FULL;
#if 0
			info->charging_status = POWER_SUPPLY_STATUS_FULL;
			info->recharging_status = false;

			/* disable charging */
			sec_bat_enable_charging(info, 0);
#else
			chg_cutoff_count = chg_cutoff_count_index;
			chg_cutoff_interrupt = true;
			chg_cutoff = 0;
#endif
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
		info->charging_status = POWER_SUPPLY_STATUS_DISCHARGING;
		sec_bat_enable_charging(info, false);
//		if (info->batt_health == POWER_SUPPLY_HEALTH_OVERVOLTAGE)
//			info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		wake_lock_timeout(&info->vbus_wake_lock, HZ * 2);
		break;
	case CABLE_TYPE_USB:
		info->charging_status = POWER_SUPPLY_STATUS_CHARGING;
		sec_bat_get_temp(info);
		sec_bat_enable_charging(info, true);
		wake_lock(&info->vbus_wake_lock);
		break;
	case CABLE_TYPE_AC:
	case CABLE_TYPE_DOCK:
		info->charging_status = POWER_SUPPLY_STATUS_CHARGING;
		sec_bat_get_temp(info);
		sec_bat_enable_charging(info, true);
	//20110517_HDLNC_PMIC_shinjh
#if !defined (CONFIG_MACH_BOSE_ATT)
	 wake_lock(&info->vbus_wake_lock);
#endif
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
			info->charging_status = POWER_SUPPLY_STATUS_FULL;
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

#if defined (CONFIG_MACH_BOSE_ATT)
int max8922_charger_topoff(void)
{
	struct power_supply *psy = power_supply_get_by_name("battery");
	union power_supply_propval value;

	if (!psy) {
		pr_err("%s: Max8907c-Charger : Fail to get battery ps\n", __func__);
		return -ENODEV;
	}

	sec_bat_check_vf(gv_info);
	if(is_bat_exist == false){
		printk("[SJH]BATTREY NOT DETECTED\n");
	return 0;
	}

	value.intval = POWER_SUPPLY_STATUS_FULL;
	return psy->set_property(psy, POWER_SUPPLY_PROP_STATUS, &value);
}
#endif

static void sec_bat_monitor_work(struct work_struct *work)
{
	struct sec_bat_info *info = container_of(work, struct sec_bat_info,
			monitor_work);
	unsigned long flags,i;
	unsigned int batt_vcell = 0, batt_vcell_temp = 0;

	for (i = 0; i < 10; i++) {
	batt_vcell_temp = sec_bat_get_fuelgauge_data(info, FG_T_VCELL);
	batt_vcell = batt_vcell + batt_vcell_temp;
	}

	if(info->batt_full_status == BATT_2ND_FULL && chg_cutoff_interrupt == true){
		if (chg_cutoff_count > 0)
			chg_cutoff_count--;
		else
			info->charging_status = POWER_SUPPLY_STATUS_FULL;
	}

	/* TODO: check charging time */
	sec_bat_charging_time_management(info);
	sec_bat_check_vf(info);
	sec_bat_update_info(info);
	sec_bat_get_temp(info);
	sec_bat_set_temp_info(info);
	info->batt_vcell = batt_vcell/10;
//	printk("[SJH]chg_cutoff_count[%d], recharging_count[%d]\n",chg_cutoff_count,recharging_count);
	switch (info->charging_status) {
	case POWER_SUPPLY_STATUS_FULL:
		if(info->batt_vcell < RECHARGING_VOLTAGE){
			if(recharging_count != 0){
				recharging_count--;
			}
		}else{
			recharging_count = recharging_count_index;
		}

		if (info->batt_vcell < RECHARGING_VOLTAGE &&
				info->recharging_status == false && recharging_count == 0) {
			info->recharging_status = true;
			sec_bat_enable_charging(info, true);
			recharging_count = recharging_count_index;
			dev_info(info->dev, "%s: Start Recharging, Vcell = %d\n"
					, __func__, info->batt_vcell);
		}

		if(info->batt_full_status == BATT_2ND_FULL && chg_cutoff_count == 0){
			//printk("[SJH]chg_cutoff[%d]\n",chg_cutoff);
			if (chg_cutoff < 3) {
				struct power_supply *psy =
					power_supply_get_by_name("fuelgauge");
				union power_supply_propval value;

				printk("[SJH]disablecharging\n");
				info->recharging_status = false;
				/* disable charging */
				sec_bat_enable_charging(info, false);
				chg_cutoff_interrupt = false;
				chg_cutoff++;
				if (!psy) {
					pr_err("%s: Fail to get fuelgauge ps\n",
						__func__);
					return -ENODEV;
				}
				value.intval = POWER_SUPPLY_STATUS_FULL;
				psy->set_property(psy, POWER_SUPPLY_PROP_STATUS,
					&value);
			}
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

	dev_info(info->dev, "soc(%d), vcell(%d), health(%d), temp(%d), charging(%d) full(%d) recharging(%d)\n",
		info->batt_soc, info->batt_vcell/1000, info->batt_health, info->batt_temp/10,
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
	SEC_BATTERY_ATTR(batt_vol_adc),
	SEC_BATTERY_ATTR(batt_vol_adc_cal),
	SEC_BATTERY_ATTR(batt_vol_adc_aver),
	SEC_BATTERY_ATTR(batt_vol_aver),
	SEC_BATTERY_ATTR(batt_temp),
	SEC_BATTERY_ATTR(batt_temp_adc),
	SEC_BATTERY_ATTR(batt_temp_adc_cal),
	SEC_BATTERY_ATTR(batt_temp_adc_aver),
	SEC_BATTERY_ATTR(batt_temp_aver),
	SEC_BATTERY_ATTR(batt_soc),
	SEC_BATTERY_ATTR(batt_reset_soc),
	SEC_BATTERY_ATTR(batt_fg_soc),
	SEC_BATTERY_ATTR(batt_test_mode),
	SEC_BATTERY_ATTR(batt_vibrator),
	SEC_BATTERY_ATTR(batt_camera),
	SEC_BATTERY_ATTR(batt_mp3),
	SEC_BATTERY_ATTR(batt_video),
	SEC_BATTERY_ATTR(batt_voice_call_2g),
	SEC_BATTERY_ATTR(batt_voice_call_3g),
	SEC_BATTERY_ATTR(batt_data_call),
	SEC_BATTERY_ATTR(batt_wifi),
	SEC_BATTERY_ATTR(batt_gps),
	SEC_BATTERY_ATTR(batt_temp_check),
	SEC_BATTERY_ATTR(batt_full_check),
	SEC_BATTERY_ATTR(batt_v_f_adc),
	SEC_BATTERY_ATTR(batt_chg_current),
	SEC_BATTERY_ATTR(batt_type),
	SEC_BATTERY_ATTR(batt_dev_state),
	SEC_BATTERY_ATTR(batt_compensation),
	SEC_BATTERY_ATTR(batt_booting),
	SEC_BATTERY_ATTR(batt_charging_source),
	SEC_BATTERY_ATTR(batt_lp_charging),
};

enum {
	BATT_VOL = 0,
       BATT_VOL_ADC,
       BATT_VOL_ADC_CAL,
       BATT_VOL_ADC_AVER,
       BATT_VOL_AVER,
       BATT_TEMP,
	BATT_TEMP_ADC,
	BATT_TEMP_ADC_CAL,
	BATT_TEMP_ADC_AVER,
	BATT_TEMP_AVER,
	BATT_SOC,
	BATT_RESET_SOC,
	BATT_FG_SOC,
	BATT_TEST_MODE,
	BATT_VIBRATOR,
	BATT_CAMERA,
	BATT_MP3,
	BATT_VIDEO,
	BATT_VOICE_CALL_2G,
	BATT_VOICE_CALL_3G,
	BATT_DATA_CALL,
	BATT_WIFI,
	BATT_GPS,
	BATT_TEMP_CHECK,
	BATT_FULL_CHECK,
	BATT_V_F_ADC,
	BATT_CHG_CURRENT,
	BATT_TYPE,
	BATT_DEV_STATE,
	BATT_COMPENSATION,
	BATT_BOOTING,
	BATT_CHARGING_SOURCE,
	BATT_LP_CHARGING,
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
	case BATT_CAMERA:
		val = sec_bat_get_compensation(OFFSET_CAMERA_ON);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_MP3:
		val = sec_bat_get_compensation(OFFSET_MP3_PLAY);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_VIDEO:
		val = sec_bat_get_compensation(OFFSET_VIDEO_PLAY);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_VOICE_CALL_2G:
		val = sec_bat_get_compensation(OFFSET_VOICE_CALL_2G);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_VOICE_CALL_3G:
		val = sec_bat_get_compensation(OFFSET_VOICE_CALL_3G);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_DATA_CALL:
		val = sec_bat_get_compensation(OFFSET_DATA_CALL);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_WIFI:
		val = sec_bat_get_compensation(OFFSET_WIFI);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_GPS:
		val = sec_bat_get_compensation(OFFSET_GPS);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_TYPE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n", "SDI_SDI");
		break;
	case BATT_VOL_ADC:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 0);
		break;
	case BATT_FG_SOC:
/*		if (chg->pdata &&
		    chg->pdata->psy_fuelgauge &&
		    chg->pdata->psy_fuelgauge->get_property) {
			chg->pdata->psy_fuelgauge->get_property(chg->pdata->psy_fuelgauge, POWER_SUPPLY_PROP_CAPACITY, &value);
			chg->bat_info.batt_soc = value.intval;
		}
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", chg->bat_info.batt_soc);
		*/break;
	case BATT_TEMP_CHECK:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_health);
		break;
	case BATT_FULL_CHECK:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_full_status);
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
	case BATT_VIBRATOR:
		if (sscanf(buf, "%d\n", &x) == 1){
			sec_bat_set_compesation(x, OFFSET_VIBRATOR_ON,	COMPENSATE_VIBRATOR);
			ret = count;
		}
		printk("[BAT]:%s: vibrator = %d\n", __func__, x);
		break;
	case BATT_CAMERA:
		if (sscanf(buf, "%d\n", &x) == 1)	{
			sec_bat_set_compesation(x, OFFSET_CAMERA_ON, COMPENSATE_CAMERA);
			ret = count;
		}
		printk("[BAT]:%s: camera = %d\n", __func__, x);
		break;
	case BATT_MP3:
		if (sscanf(buf, "%d\n", &x) == 1)	{
			sec_bat_set_compesation(x, OFFSET_MP3_PLAY,	COMPENSATE_MP3);
			ret = count;
		}
		printk("[BAT]:%s: mp3 = %d\n", __func__, x);
		break;
	case BATT_VIDEO:
		if (sscanf(buf, "%d\n", &x) == 1){
			sec_bat_set_compesation(x, OFFSET_VIDEO_PLAY, COMPENSATE_VIDEO);
			ret = count;
		}
		printk("[BAT]:%s: video = %d\n", __func__, x);
		break;
	case BATT_VOICE_CALL_2G:
		if (sscanf(buf, "%d\n", &x) == 1){
			sec_bat_set_compesation(x, OFFSET_VOICE_CALL_2G, COMPENSATE_VOICE_CALL_2G);
			ret = count;
		}
		printk("[BAT]:%s: voice call 2G = %d\n", __func__, x);
		break;
	case BATT_VOICE_CALL_3G:
		if (sscanf(buf, "%d\n", &x) == 1){
			sec_bat_set_compesation(x, OFFSET_VOICE_CALL_3G, COMPENSATE_VOICE_CALL_3G);
			ret = count;
		}
		printk("[BAT]:%s: voice call 3G = %d\n", __func__, x);
		break;
	case BATT_DATA_CALL:
		if (sscanf(buf, "%d\n", &x) == 1){
			sec_bat_set_compesation(x, OFFSET_DATA_CALL, COMPENSATE_DATA_CALL);
			ret = count;
		}
		printk("[BAT]:%s: data call = %d\n", __func__, x);
		break;
	case BATT_WIFI:
		if (sscanf(buf, "%d\n", &x) == 1){
			sec_bat_set_compesation(x, OFFSET_WIFI, COMPENSATE_WIFI);
			ret = count;
		}
		printk("[BAT]:%s: wifi = %d\n", __func__, x);
		break;
	case BATT_GPS:
		if (sscanf(buf, "%d\n", &x) == 1){
			sec_bat_set_compesation(x, OFFSET_GPS, COMPENSATE_GPS);
			ret = count;
		}
		printk("[BAT]:%s: gps = %d\n", __func__, x);
		break;

	case BATT_BOOTING:
		if (sscanf(buf, "%d\n", &x) == 1){
			sec_bat_set_compesation(x, OFFSET_BOOTING, COMPENSATE_BOOTING);
			ret = count;
		}
		printk("[BAT]:%s: boot complete = %d\n", __func__, x);
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

    info->get_BAT_VF_adcvalue = pdata->get_BAT_VF_adcvalue;

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
	info->event_dev_state = 0;             	// edit seojw event charging

	event_start_time_msec = 0;
	event_total_time_msec = 0;
	event_occur=0;

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

	gv_info = info;			// edit seojw event charging
	if (pdata->get_batt_level)
		pdata->get_batt_level(&info->batt_soc);

	/* for playlpm init value */
	sec_bat_update_info(info);
	dev_info(&pdev->dev, "capacity(%d), health(%d), status(%d), voltage(%d)\n",
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


