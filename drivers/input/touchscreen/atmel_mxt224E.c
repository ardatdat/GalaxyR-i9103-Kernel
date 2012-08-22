/*
*  atmel_mxt224E.c - Atmel maXTouch Touchscreen Controller
*
*  Version 0.2a
*
*  An early alpha version of the maXTouch Linux driver.
*
*
*  Copyright (C) 2010 Iiro Valkonen <iiro.valkonen@atmel.com>
*  Copyright (C) 2009 Ulf Samuelsson <ulf.samuelsson@atmel.com>
*  Copyright (C) 2009 Raphael Derosso Pereira <raphaelpereira@gmail.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define	DEBUG_INFO      1
#define DEBUG_VERBOSE   2
#define	DEBUG_MESSAGES  5
#define	DEBUG_RAW       8
#define	DEBUG_TRACE     10
//#define	TSP_BOOST
#define	TS_100S_TIMER_INTERVAL 1

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/input/mt.h>

#include <linux/init.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/major.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/smp_lock.h>

#include <linux/delay.h>
#include <linux/atmel_mxt224E.h>
#include "atmel_mxt224E_cfg.h"
#include "device_config.h"

#define TSP_INFO_LOG //touch message out

#include <linux/reboot.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>

/*
* This is a driver for the Atmel maXTouch Object Protocol
*
* When the driver is loaded, mxt_init is called.
* mxt_driver registers the "mxt_driver" structure in the i2c subsystem
* The mxt_idtable.name string allows the board support to associate
* the driver with its own data.
*
* The i2c subsystem will call the mxt_driver.probe == mxt_probe
* to detect the device.
* mxt_probe will reset the maXTouch device, and then
* determine the capabilities of the I2C peripheral in the
* host processor (needs to support BYTE transfers)
*
* If OK; mxt_probe will try to identify which maXTouch device it is
* by calling mxt_identify.
*
* If a known device is found, a linux input device is initialized
* the "mxt" device data structure is allocated
* as well as an input device structure "mxt->input"
* "mxt->client" is provided as a parameter to mxt_probe.
*
* mxt_read_object_table is called to determine which objects
* are present in the device, and to determine their adresses
*
*
* Addressing an object:
*
* The object is located at a 16 address in the object address space
*
* The object address can vary between revisions of the firmware
*
* The address is provided through an object descriptor table in the beginning
* of the object address space.
* It is assumed that an object type is only listed once in this table,
* Each object type can have several instances, and the number of
* instances is available in the object table
*
* The base address of the first instance of an object is stored in
* "mxt->object_table[object_type].chip_addr",
* This is indexed by the object type and allows direct access to the
* first instance of an object.
*
* Each instance of an object is assigned a "Report Id" uniquely identifying
* this instance. Information about this instance is available in the
* "mxt->report_id" variable, which is a table indexed by the "Report Id".
*
* The maXTouch object protocol supports adding a checksum to messages.
* By setting the most significant bit of the maXTouch address
* an 8 bit checksum is added to all writes.
*
*
* How to use driver.
* -----------------
* Example:
* In arch/avr32/boards/atstk1000/atstk1002.c
* an "i2c_board_info" descriptor is declared.
* This contains info about which driver ("mXT224"),
* which i2c address and which pin for CHG interrupts are used.
*
* In the "atstk1002_init" routine, "i2c_register_board_info" is invoked
* with this information. Also, the I/O pins are configured, and the I2C
* controller registered is on the application processor.
*
*/


int tsp_keycodes[NUMOFKEYS] = {
	KEY_MENU,
	KEY_BACK,
};

char *tsp_2keyname[NUMOFKEYS] = {
	"Menu",
	"Back",
};

int tsp_2key_led_ctrl[NUMOFKEYS] = {
	MENU_LED_2KEY,
	BACK_LED_2KEY,
};

int tsp_4keycodes[NUMOF4KEYS] = {
	KEY_MENU,
	KEY_HOME,
	KEY_BACK,
	KEY_SEARCH
};

char *tsp_4keyname[NUMOF4KEYS] ={
        "Menu",
        "Home",
        "Back",
        "Search"
};

int tsp_4key_led_ctrl[NUMOF4KEYS] = {
	MENU_LED_4KEY,
	HOME_LED_4KEY,
	BACK_LED_4KEY,
	SEARCH_LED_4KEY
};

static u16 tsp_keystatus;
#ifdef KEY_LED_CONTROL
static u32 key_led_status = false;
#endif

/*#define MEDIAN_FILTER_ERROR_SET*/
#ifdef MEDIAN_FILTER_ERROR_SET
u8 median_error_flag;
#endif
u8 first_palm_chk;

bool mxt_reconfig_flag;
EXPORT_SYMBOL(mxt_reconfig_flag);
/* 0404 work */
enum {
	DISABLE,
	ENABLE
};
static bool cal_check_flag;
static u8 facesup_message_flag ;
static u8 facesup_message_flag_T9 ;
static bool timer_flag = DISABLE;
static uint8_t timer_ticks = 0;
static unsigned int mxt_time_point;
static unsigned int time_after_autocal_enable = 0;
static unsigned int time_after_autocal_enable_key;
static bool coin_check_flag = 0;
static u8 coin_check_count = 0;
static bool metal_suppression_chk_flag = true;

static u8 chk_touch_cnt, chk_antitouch_cnt;
static u8 caling_check = 0;

#define ABS(x,y)		( (x < y) ? (y - x) : (x - y))

//#define TSP_DEBUG_MESSAGE
#ifdef TSP_DEBUG_MESSAGE
#define MAX_MSG_LOG_SIZE	512

struct  {
	u8  id[MAX_MSG_LOG_SIZE];
	u8  status[MAX_MSG_LOG_SIZE];
	u16  xpos[MAX_MSG_LOG_SIZE];
	u16  ypos[MAX_MSG_LOG_SIZE];
	u8   area[MAX_MSG_LOG_SIZE];
	u8   amp[MAX_MSG_LOG_SIZE];
	u16 cnt;
}msg_log;
#endif

struct  tch_msg_t{
	u8  id;
	u8  status[10];
	u16  xpos[10];
	u16  ypos[10];
	u8   area[10];
	u8   amp[10];
};

struct tch_msg_t new_touch;
struct tch_msg_t old_touch;

struct  {
	s16 length[5];
	u8 angle[5];
	u8 cnt;
}tch_vct[10];


#if defined(TSP_BOOST)
static bool clk_lock_state = 0;
extern void tegra_cpu_lock_speed(int min_rate, int timeout_ms, bool force);
extern void tegra_cpu_unlock_speed(bool force);
#endif

static void check_chip_calibration(struct mxt_data *mxt);
static void check_chip_channel(struct mxt_data *mxt);
static void cal_maybe_good(struct mxt_data *mxt);
static int calibrate_chip(struct mxt_data *mxt);
static void mxt_palm_recovery(struct work_struct *work);
static void check_chip_palm(struct mxt_data *mxt);

#if 1/*for debugging, enable DEBUG_INFO */
static int debug = DEBUG_MESSAGES;
#else
static int debug = DEBUG_TRACE;  /* for debugging,  enable DEBUG_TRACE */
#endif

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Activate debugging output");

#if ENABLE_NOISE_TEST_MODE
/*botton_right, botton_left, center, top_right, top_left */
u16 test_node[5] = { 12, 20, 104, 188, 196};;
#endif


#ifdef CONFIG_HAS_EARLYSUSPEND
static void mxt_early_suspend(struct early_suspend *h);
static void mxt_late_resume(struct early_suspend *h);
#endif

#ifdef MXT_FACTORY_TEST
extern struct class *sec_class;
struct device *tsp_factory_mode;
#endif

#if	TS_100S_TIMER_INTERVAL
static struct workqueue_struct *ts_100s_tmr_workqueue;
static void ts_100ms_timeout_handler(unsigned long data);
static void ts_100ms_timer_start(struct mxt_data *mxt);
static void ts_100ms_timer_stop(struct mxt_data *mxt);
static void ts_100ms_timer_init(struct mxt_data *mxt);
static void ts_100ms_tmr_work(struct work_struct *work);
#endif

static int  mxt_identify(struct i2c_client *client, struct mxt_data *mxt);
static int  mxt_read_object_table(struct i2c_client *client, struct mxt_data *mxt);

#define I2C_RETRY_COUNT 5

const u8 *maxtouch_family = "maXTouch";
const u8 *mxt224_variant  = "mXT224E";

u8 *object_type_name[MXT_MAX_OBJECT_TYPES] = {
	[5] = "GEN_MESSAGEPROCESSOR_T5",
	[6] = "GEN_COMMANDPROCESSOR_T6",
	[7] = "GEN_POWERCONFIG_T7",
	[8] = "GEN_ACQUIRECONFIG_T8",
	[9] = "TOUCH_MULTITOUCHSCREEN_T9",
	[15] = "TOUCH_KEYARRAY_T15",
	[18] = "SPT_COMMSCONFIG_T18",
	[24] = "PROCI_ONETOUCHGESTUREPROCESSOR_T24",
	[25] = "SPT_SELFTEST_T25",
	[37] = "DEBUG_DIAGNOSTICS_T37",
	[38] = "USER_DATA_T38",
	[40] = "PROCI_GRIPSUPPRESSION_T40",
	[42] = "PROCI_TOUCHSUPPRESSION_T42",
	[46] = "SPT_CTECONFIG_T46",
	[47] = "PROCI_STYLUS_T47",
	[48] = "PROCG_NOISESUPPRESSION_T48",
};

#if 1/* _SUPPORT_MULTITOUCH_ */
struct multi_touch_info {
	uint16_t size;
	int16_t pressure;
	int16_t x;
	int16_t y;
	int16_t component;
};

static struct multi_touch_info mtouch_info[MXT_MAX_NUM_TOUCHES];
#endif

static bool palm_check_timer_flag = false;
static bool palm_release_flag = true;

#define TOUCH_LOCKUP_PATTERN_RELEASE

#ifdef TOUCH_LOCKUP_PATTERN_RELEASE

#define MAX_GHOSTCHECK_FINGER		10
#define MAX_GHOSTTOUCH_COUNT		200
#define MAX_COUNT_TOUCHSYSREBOOT	4
#define MAX_GHOSTTOUCH_BY_PATTERNTRACKING	5

unsigned int touch_is_pressed_arr[MXT_MAX_NUM_TOUCHES];

static int cghost_clear;  /* ghost touch clear count  by Xtopher */
static int ftouch_reboot;
static int tcount_finger[MAX_GHOSTCHECK_FINGER] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int touchbx[MAX_GHOSTCHECK_FINGER] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int touchby[MAX_GHOSTCHECK_FINGER] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int ghosttouchcount;
static int tsp_reboot_count;
static int cFailbyPattenTracking;
#endif

/*
* declaration of external functions
*/
u16 get_object_address(uint8_t object_type,
		       uint8_t instance,
		       struct mxt_object *object_table,
		       int max_objs);

int backup_to_nv(struct mxt_data *mxt)
{
	/* backs up settings to the non-volatile memory */
	return mxt_write_byte(mxt->client,
		MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) +
		MXT_ADR_T6_BACKUPNV,
		0x55);
}

int reset_chip(struct mxt_data *mxt, u8 mode)
{
	u8 data;
	if (debug >= DEBUG_MESSAGES)
		pr_info("[TSP] Reset chip Reset mode (%d)", mode);
	if (mode == RESET_TO_NORMAL)
		data = 0x1;  /* non-zero value */
	else if (mode == RESET_TO_BOOTLOADER)
		data = 0xA5;
	else {
		pr_err("[TSP] Invalid reset mode(%d)", mode);
		return -1;
	}

	/* Any non-zero value written to reset reg will reset the chip */
#if	0	/* defined MXT_FIRMUP_ENABLE */
	/*start firmware updating : not yet finished*/
	/*There are no MXT_BASE_ADDR , before get object table*/
	/*MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) +MXT_ADR_T6_RESET is 0x102*/
	if (mxt->firm_status_data == 1) {
		return mxt_write_byte(mxt->client,
			0x102,
			data);
	} else {
		return mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) +
			MXT_ADR_T6_RESET,
			data);
	}
#else
	return mxt_write_byte(mxt->client,
		MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) +
		MXT_ADR_T6_RESET,
		data);
#endif
}

#ifdef MXT_ESD_WORKAROUND
static void mxt_force_reset(struct mxt_data *mxt)
{
	int cnt;

	if (debug >= DEBUG_MESSAGES)
		pr_info("[TSP] %s has been called!", __func__);

#ifndef MXT_THREADED_IRQ
	wake_lock(&mxt->wakelock);	/* prevents the system from entering suspend during updating */
	disable_irq(mxt->client->irq);	/* disable interrupt */
#endif
	if (mxt->pdata->suspend_platform_hw != NULL)
		mxt->pdata->suspend_platform_hw(mxt->pdata);
	msleep(100);
	if (mxt->pdata->resume_platform_hw != NULL)
		mxt->pdata->resume_platform_hw(mxt->pdata);

	for (cnt = 10; cnt > 0; cnt--) {
		if (reset_chip(mxt, RESET_TO_NORMAL) == 0)	/* soft reset */
			break;
	}
	if (cnt == 0) {
		pr_err("[TSP] mxt_force_reset failed!!!");
		return;
	}
	msleep(250);  /* 200ms */
#ifndef MXT_THREADED_IRQ
	enable_irq(mxt->client->irq);	/* enable interrupt */
	wake_unlock(&mxt->wakelock);
#endif
	if (debug >= DEBUG_MESSAGES)
		pr_info("[TSP] %s success!!!", __func__);
}
#endif

#if defined(MXT_DRIVER_FILTER)
static void equalize_coordinate(bool detect, u8 id, u16 *px, u16 *py)
{
	static int tcount[MXT_MAX_NUM_TOUCHES] = {  0, };
	static u16 pre_x[MXT_MAX_NUM_TOUCHES][4] = { { 0}, };
	static u16 pre_y[MXT_MAX_NUM_TOUCHES][4] = { { 0}, };
	int coff[4] = { 0,};
	int distance = 0;

	if (detect) {
		tcount[id] = 0;
	}

	pre_x[id][tcount[id]%4] = *px;
	pre_y[id][tcount[id]%4] = *py;

	if (tcount[id] > 3)	{
		distance = abs(pre_x[id][(tcount[id]-1)%4] - *px) + abs(pre_y[id][(tcount[id]-1)%4] - *py);

		coff[0] = (u8)(4 + distance/5);
		if (coff[0] < 8) {
			coff[0] = max(4, coff[0]);
			coff[1] = min((10 - coff[0]), (coff[0]>>1)+1);
			coff[2] = min((10 - coff[0] - coff[1]), (coff[1]>>1)+1);
			coff[3] = 10 - coff[0] - coff[1] - coff[2];

			pr_debug("[TSP] %d, %d, %d, %d \n",
				coff[0], coff[1], coff[2], coff[3]);
			*px = (u16)((*px*(coff[0])
				+ pre_x[id][(tcount[id]-1)%4]*(coff[1])
				+ pre_x[id][(tcount[id]-2)%4]*(coff[2])
				+ pre_x[id][(tcount[id]-3)%4]*(coff[3]))/10);
			*py = (u16)((*py*(coff[0])
				+ pre_y[id][(tcount[id]-1)%4]*(coff[1])
				+ pre_y[id][(tcount[id]-2)%4]*(coff[2])
				+ pre_y[id][(tcount[id]-3)%4]*(coff[3]))/10);
		} else {
			*px = (u16)((*px*4 + pre_x[id][(tcount[id]-1)%4])/5);
			*py = (u16)((*py*4 + pre_y[id][(tcount[id]-1)%4])/5);
		}
	}
	tcount[id]++;
}
#endif  /* MXT_DRIVER_FILTER */

static void mxt_release_all_fingers(struct mxt_data *mxt)
{
	int id;
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] %s \n", __func__);

	for (id = 0 ; id < MXT_MAX_NUM_TOUCHES ; ++id) {
		if ( mtouch_info[id].pressure == -1 )
			continue;
#ifdef TOUCH_LOCKUP_PATTERN_RELEASE
		touch_is_pressed_arr[id] = 0;
#endif
		/* force release check*/
		mtouch_info[id].pressure = 0;

		/* ADD TRACKING_ID*/
		input_mt_slot(mxt->input, id);
		input_mt_report_slot_state(mxt->input, MT_TOOL_FINGER, false);
#ifdef TSP_INFO_LOG
		if (debug >= DEBUG_MESSAGES)
			pr_info("[TSP] r (%d,%d) %d\n", mtouch_info[id].x, mtouch_info[id].y, id);
#endif
		if (mtouch_info[id].pressure == 0)
			mtouch_info[id].pressure = -1;
	}

	input_sync(mxt->input);

#if defined(TSP_BOOST)
		if (clk_lock_state) {
			tegra_cpu_unlock_speed(false);
			clk_lock_state = false;
		}
#endif
}

static void mxt_release_all_keys(struct mxt_data *mxt)
{
	if (debug >= DEBUG_INFO)
			pr_info("[TSP] %s, tsp_keystatus = %d \n", __func__, tsp_keystatus);
	if (tsp_keystatus != TOUCH_KEY_NULL) {
		if ((mxt->pdata->board_rev <= 9) || (mxt->pdata->board_rev >= 13)) {
			switch (tsp_keystatus) {
			case TOUCH_2KEY_MENU:
				input_report_key(mxt->input, KEY_MENU, KEY_RELEASE);
				break;
			case TOUCH_2KEY_BACK:
				input_report_key(mxt->input, KEY_BACK, KEY_RELEASE);
				break;
			default:
				break;
			}
#ifdef TSP_INFO_LOG
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP_KEY] r %s\n", tsp_2keyname[tsp_keystatus - 1]);
#endif
		} else {	/* board rev1.0~1.2, touch key is 4 key array */
			switch (tsp_keystatus) {
			case TOUCH_4KEY_MENU:
				input_report_key(mxt->input, KEY_MENU, KEY_RELEASE);
				break;
			case TOUCH_4KEY_HOME:
				input_report_key(mxt->input, KEY_HOME, KEY_RELEASE);
				break;
			case TOUCH_4KEY_BACK:
				input_report_key(mxt->input, KEY_BACK, KEY_RELEASE);
				break;
			case TOUCH_4KEY_SEARCH:
				input_report_key(mxt->input, KEY_SEARCH, KEY_RELEASE);
				break;
			default:
				break;
			}
#ifdef TSP_INFO_LOG
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP_KEY] r %s\n", tsp_4keyname[tsp_keystatus - 1]);
#endif
			tsp_keystatus = TOUCH_KEY_NULL;
		}
	}
}

/*mode 1 = Charger connected */
/*mode 0 = Charger disconnected*/

static void mxt_inform_charger_connection(struct mxt_callbacks *cb, int mode)
{
	struct mxt_data *mxt = container_of(cb, struct mxt_data, callbacks);

	mxt->set_mode_for_ta = !!mode;
	if (mxt->mxt_status && !work_pending(&mxt->ta_work))
		schedule_work(&mxt->ta_work);
}

extern int mxt_noisesuppression_t48_config(struct mxt_data *mxt);
extern int mxt_noisesuppression_t48_config_for_TA(struct mxt_data *mxt);

static void mxt_ta_worker(struct work_struct *work)
{
	struct mxt_data *mxt = container_of(work, struct mxt_data, ta_work);
	int error = 0;

	if (debug >= DEBUG_INFO)
		pr_info("[TSP] %s\n", __func__);
	disable_irq(mxt->client->irq);

	if (mxt->set_mode_for_ta) {
		/* CHRGON disable */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) + MXT_ADR_T48_CALCFG,
			T48_CALCFG_TA);

		/* T48 config all wirte (TA) */
		mxt_noisesuppression_t48_config_for_TA(mxt);

		/* T8 charge time 31 */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_CHRGTIME,
			T8_CHRGTIME_TA);

		/* T46 Depth 48 */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_SPT_CTECONFIG_T46) + MXT_ADR_T46_IDLESYNCSPERX,
			T46_IDLESYNCSPERX_TA);
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_SPT_CTECONFIG_T46) + MXT_ADR_T46_ACTVSYNCSPERX,
			T46_ACTVSYNCSPERX_TA);

		/* CHRGON enable */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) + MXT_ADR_T48_CALCFG,
			T48_CALCFG_TA | T48_CHGON_BIT);
	} else {
		/* CHRGON disable */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) + MXT_ADR_T48_CALCFG,
			T48_CALCFG);

		/* T48 config all wirte (BATT) */
		mxt_noisesuppression_t48_config(mxt);

		/* T8 charge time 45 */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_CHRGTIME,
			T8_CHRGTIME);

		/* T46 Depth 26 */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_SPT_CTECONFIG_T46) + MXT_ADR_T46_IDLESYNCSPERX,
			T46_IDLESYNCSPERX);
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_SPT_CTECONFIG_T46) + MXT_ADR_T46_ACTVSYNCSPERX,
			T46_ACTVSYNCSPERX);
#ifdef MEDIAN_FILTER_ERROR_SET
		if (median_error_flag == 1) {
			mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) +
				MXT_ADR_T9_TCHTHR, T9_TCHTHR);
			mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) +
				MXT_ADR_T9_MOVFILTER, T9_MOVFILTER);
			mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) +
				MXT_ADR_T9_NEXTTCHDI, T9_NEXTTCHDI);
			median_error_flag = 0;
		}
#endif
		/* CHRGON enable */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) + MXT_ADR_T48_CALCFG,
			T48_CALCFG | T48_CHGON_BIT);
	}
	if (error < 0) pr_err("[TSP] mxt TA/USB mxt_noise_suppression_config Error!!\n");
	else {
		if (debug >= DEBUG_INFO) {
			if (mxt->set_mode_for_ta) {
				pr_info("[TSP] mxt TA/USB mxt_noise_suppression_config Success!!\n");
			} else {
				pr_info("[TSP] mxt BATTERY mxt_noise_suppression_config Success!!\n");
			}
		}
		calibrate_chip(mxt);
	}

	enable_irq(mxt->client->irq);
	return;
}

/* metal suppress enable timer function */
static void mxt_metal_suppression_off(struct work_struct *work)
{
	int error, i;
	struct	mxt_data *mxt;
	mxt = container_of(work, struct mxt_data, timer_dwork.work);

	metal_suppression_chk_flag = false;
	if (debug >= DEBUG_INFO)
		pr_info("[TSP]%s, metal_suppression_chk_flag = %d \n", __func__, metal_suppression_chk_flag);
	disable_irq(mxt->client->irq);
	mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) + MXT_ADR_T9_AMPHYST, 30);

	if (time_after_autocal_enable != 0) {
		if ((jiffies_to_msecs(jiffies) - time_after_autocal_enable) > 1500) {
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP] Floating metal Suppressed time out!! Autocal = 0, (%d), coin_check_count = %d \n",
					(jiffies_to_msecs(jiffies) - time_after_autocal_enable),
					coin_check_count);

			/* T8_TCHAUTOCAL  */
			error = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_TCHAUTOCAL, 0);
			if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

			time_after_autocal_enable = 0;

			for(i=0;i < 10; i++) {
				tch_vct[i].cnt = 0;
			}
		}
	}
	enable_irq(mxt->client->irq);
	return;
}

/* mxt224E reconfigration */
static void mxt_reconfigration_normal(struct work_struct *work)
{
	struct	mxt_data *mxt;
	int error, id;
	mxt = container_of(work, struct mxt_data, config_dwork.work);

	if (debug >= DEBUG_INFO)
		pr_info("[TSP]%s \n", __func__);

	disable_irq(mxt->client->irq);
	for (id = 0 ; id < MXT_MAX_NUM_TOUCHES ; ++id) {
		if ( mtouch_info[id].pressure == -1 )
			continue;
		schedule_delayed_work(&mxt->config_dwork, 300);
		pr_warning("[TSP] touch pressed!! %s didn't execute!!\n", __func__);
		enable_irq(mxt->client->irq);
		return;
	}

	/* 0331 Added for Palm recovery */

	/* T8_ATCHFRCCALTHR */
	error = mxt_write_byte(mxt->client,
		MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALTHR,
		T8_ATCHFRCCALTHR);

	if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

	/* T8_ATCHFRCCALRATIO */
	error = mxt_write_byte(mxt->client,
		MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALRATIO,
		0);

	if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

	/* T42_MAXNUMTCHS */
	error = mxt_write_byte(mxt->client,
		MXT_BASE_ADDR(MXT_PROCI_TOUCHSUPPRESSION_T42) + MXT_ADR_T42_MAXNUMTCHS,
		T42_MAXNUMTCHS);

	if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);


	mxt_reconfig_flag = true;
	enable_irq(mxt->client->irq);
	return;
}

/* Calculates the 24-bit CRC sum. */
static u32 mxt_CRC_24(u32 crc, u8 byte1, u8 byte2)
{
	static const u32 crcpoly = 0x80001B;
	u32 result;
	u16 data_word;

	data_word = (u16) ((u16) (byte2 << 8u) | byte1);
	result = ((crc << 1u) ^ (u32) data_word);
	if (result & 0x1000000)
		result ^= crcpoly;
	return result;
}

/* Returns object address in mXT chip, or zero if object is not found */
u16 get_object_address(uint8_t object_type,
		       uint8_t instance,
		       struct mxt_object *object_table,
		       int max_objs)
{
	uint8_t object_table_index = 0;
	uint8_t address_found = 0;
	uint16_t address = 0;

	struct mxt_object obj;

	while ((object_table_index < max_objs) && !address_found) {
		obj = object_table[object_table_index];
		if (obj.type == object_type) {
			address_found = 1;
			/* Are there enough instances defined in the FW? */
			if (obj.instances >= instance) {
				address = obj.chip_addr +
					(obj.size + 1) * instance;
			} else {
				return 0;
			}
		}
		object_table_index++;
	}

	return address;
}

/* Returns object size in mXT chip, or zero if object is not found */
u16 get_object_size(uint8_t object_type, struct mxt_object *object_table, int max_objs)
{
	uint8_t object_table_index = 0;
	struct mxt_object obj;

	while (object_table_index < max_objs) {
		obj = object_table[object_table_index];
		if (obj.type == object_type) {
			return obj.size;
		}
		object_table_index++;
	}
	return 0;
}

/*
* Reads one byte from given address from mXT chip (which requires
* writing the 16-bit address pointer first).
*/

int mxt_read_byte(struct i2c_client *client, u16 addr, u8 *value)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg[2];
	__le16 le_addr = cpu_to_le16(addr);
	struct mxt_data *mxt;

	mxt = i2c_get_clientdata(client);


	msg[0].addr  = client->addr;
	msg[0].flags = 0x00;
	msg[0].len   = 2;
	msg[0].buf   = (u8 *) &le_addr;

	msg[1].addr  = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len   = 1;
	msg[1].buf   = (u8 *) value;
	if  (i2c_transfer(adapter, msg, 2) == 2) {
		mxt->last_read_addr = addr;
		return 0;
	} else {
	/*
	* In case the transfer failed, set last read addr to invalid
	* address, so that the next reads won't get confused.
		*/
		mxt->last_read_addr = -1;
		return -EIO;
	}
}

/*
* Reads a block of bytes from given address from mXT chip. If we are
* reading from message window, and previous read was from message window,
* there's no need to write the address pointer: the mXT chip will
* automatically set the address pointer back to message window start.
*/

int mxt_read_block(struct i2c_client *client,
		   u16 addr,
		   u16 length,
		   u8 *value)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg[2];
	__le16	le_addr;
	struct mxt_data *mxt;

	mxt = i2c_get_clientdata(client);

	if (mxt != NULL) {
		if ((mxt->last_read_addr == addr) &&
			(addr == mxt->msg_proc_addr)) {
			if  (i2c_master_recv(client, value, length) == length)
				return 0;
			else
				return -EIO;
		} else {
			mxt->last_read_addr = addr;
		}
	}

	le_addr = cpu_to_le16(addr);
	msg[0].addr  = client->addr;
	msg[0].flags = 0x00;
	msg[0].len   = 2;
	msg[0].buf   = (u8 *) &le_addr;

	msg[1].addr  = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len   = length;
	msg[1].buf   = (u8 *) value;
	if  (i2c_transfer(adapter, msg, 2) == 2)
		return 0;
	else
		return -EIO;

}

/* Reads a block of bytes from current address from mXT chip. */

int mxt_read_block_wo_addr(struct i2c_client *client,
			   u16 length,
			   u8 *value)
{


	if  (i2c_master_recv(client, value, length) == length) {
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] read ok\n");
		return length;
	} else {
		pr_err("[TSP] read failed\n");
		return -EIO;
	}

}


/* Writes one byte to given address in mXT chip. */

int mxt_write_byte(struct i2c_client *client, u16 addr, u8 value)
{
	struct {
		__le16 le_addr;
		u8 data;

	} i2c_byte_transfer;

	struct mxt_data *mxt;

	mxt = i2c_get_clientdata(client);
	if (mxt != NULL)
		mxt->last_read_addr = -1;

	i2c_byte_transfer.le_addr = cpu_to_le16(addr);
	i2c_byte_transfer.data = value;


	if  (i2c_master_send(client, (u8 *) &i2c_byte_transfer, 3) == 3)
		return 0;
	else
		return -EIO;
}


/* Writes a block of bytes (max 256) to given address in mXT chip. */

int mxt_write_block(struct i2c_client *client,
		    u16 addr,
		    u16 length,
		    u8 *value)
{
	int i;
	struct {
		__le16	le_addr;
		u8	data[256];

	} i2c_block_transfer;

	struct mxt_data *mxt;

	if (length > 256)
		return -EINVAL;

	mxt = i2c_get_clientdata(client);
	if (mxt != NULL)
		mxt->last_read_addr = -1;



	for (i = 0; i < length; i++)
		i2c_block_transfer.data[i] = *value++;


	i2c_block_transfer.le_addr = cpu_to_le16(addr);

	i = i2c_master_send(client, (u8 *) &i2c_block_transfer, length + 2);

	if (i == (length + 2))
		return length;
	else
		return -EIO;
}

/* TODO: make all other access block until the read has been done? Otherwise
an arriving message for example could set the ap to message window, and then
the read would be done from wrong address! */

/* Writes the address pointer (to set up following reads). */

int mxt_write_ap(struct i2c_client *client, u16 ap)
{

	__le16	le_ap = cpu_to_le16(ap);
	struct mxt_data *mxt;

	mxt = i2c_get_clientdata(client);
	if (mxt != NULL)
		mxt->last_read_addr = -1;

	if (debug >= DEBUG_INFO)
		pr_info("[TSP] Address pointer set to %d\n", ap);

	if (i2c_master_send(client, (u8 *) &le_ap, 2) == 2)
		return 0;
	else
		return -EIO;
}

/* Calculates the CRC value for mXT infoblock. */
int calculate_infoblock_crc(u32 *crc_result, struct mxt_data *mxt)
{
	u32 crc = 0;
	u16 crc_area_size;
	u8 *mem;
	int i;

	int error;
	struct i2c_client *client;

	client = mxt->client;

	crc_area_size = MXT_ID_BLOCK_SIZE +
		mxt->device_info.num_objs * MXT_OBJECT_TABLE_ELEMENT_SIZE;

	mem = kmalloc(crc_area_size, GFP_KERNEL);

	if (mem == NULL) {
		dev_err(&client->dev, "[TSP] Error allocating memory\n");
		return -ENOMEM;
	}

	error = mxt_read_block(client, 0, crc_area_size, mem);
	if (error < 0) {
		kfree(mem);
		return error;
	}

	for (i = 0; i < (crc_area_size - 1); i = i + 2)
		crc = mxt_CRC_24(crc, *(mem + i), *(mem + i + 1));

	/* If uneven size, pad with zero */
	if (crc_area_size & 0x0001)
		crc = mxt_CRC_24(crc, *(mem + i), 0);

	kfree(mem);

	/* Return only 24 bits of CRC. */
	*crc_result = (crc & 0x00FFFFFF);
	return 1;

}

#ifdef TOUCH_LOCKUP_PATTERN_RELEASE

static void clear_tcount(void)
{
	int i;

	for (i = 0; i < MAX_GHOSTCHECK_FINGER; i++) {
		tcount_finger[i] = 0;
		touchbx[i] = 0;
		touchby[i] = 0;
	}
}

static int diff_two_point(s16 x, s16 y, s16 oldx, s16 oldy)
{
	s16 diffx, diffy;
	s16 distance;

	diffx = x - oldx;
	diffy = y - oldy;
	distance = abs(diffx) + abs(diffy);

	if (distance < 4)
		return 1;
	else
		return 0;
}

static void TSP_forced_release_for_call(struct mxt_data *mxt)
{
	mxt_release_all_fingers(mxt);
	mxt_release_all_keys(mxt);

	calibrate_chip(mxt);
}

static ssize_t TSP_Clibration_Power_Off(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mxt_data *mxt = dev_get_drvdata(dev);

	pr_info("[TSP] %s : force calibration\n", __func__);
	TSP_forced_release_for_call(mxt);
	return sprintf(buf, "1\n");
}

/*
Forced reboot sequence.
Don't use arbitraily.
if you meet special case that this routine has to be used, ask Xtopher's advice.
*/
static void TSP_forced_reboot(struct mxt_data *mxt)
{
	if (ftouch_reboot == 1)
		return;
	ftouch_reboot = 1;
	cghost_clear = 0;

	mxt_release_all_fingers(mxt);
	mxt_release_all_keys(mxt);
	mxt_force_reset(mxt);

	calibrate_chip(mxt);

	ftouch_reboot  = 0;
}

/* To do forced calibration when ghost touch occured at the same point
    for several second.   Xtopher */
static int tsp_pattern_tracking(int fingerindex,
	s16 x, s16 y, struct mxt_data *mxt)
{
	int i;
	int ghosttouch = 0;

	for (i = 0; i < MXT_MAX_NUM_TOUCHES; i++) {
		if (i == fingerindex) {
			if (diff_two_point(x, y, touchbx[i], touchby[i]))
				tcount_finger[i] = tcount_finger[i] + 1;
			else
				tcount_finger[i] = 0;

			touchbx[i] = x;
			touchby[i] = y;

			if (tcount_finger[i] > MAX_GHOSTTOUCH_COUNT) {
				ghosttouch = 1;
				ghosttouchcount++;
				printk(KERN_DEBUG "[TSP] SUNFLOWER (PATTERN TRACKING) %d\n",
					ghosttouchcount);
				clear_tcount();

				cFailbyPattenTracking++;
				if (cFailbyPattenTracking >
					MAX_GHOSTTOUCH_BY_PATTERNTRACKING) {
					cFailbyPattenTracking = 0;
					TSP_forced_reboot(mxt);
				} else {
					TSP_forced_release_for_call(mxt);
				}
			}
		}
	}
	return ghosttouch;
}
#endif


void process_T9_message(u8 *message, struct mxt_data *mxt)
{
	struct	input_dev *input;
	u8  status;
	u16 xpos = 0xFFFF;
	u16 ypos = 0xFFFF;
	u8 report_id;
	u8 touch_id;  /* to identify each touches. starts from 0 to 15 */
	u8 pressed_or_released = 0;
	static int prev_touch_id = -1;
	int i, error;
	u16 chkpress = 0;
	u16 finger_channel_cnt = 0;
	u8 touch_message_flag = 0;
	static int cal_move =0;
#ifdef TOUCH_LOCKUP_PATTERN_RELEASE
	int movecount = 0;
	int presscount = 0;
#endif
	input = mxt->input;
	status = message[MXT_MSG_T9_STATUS];
	report_id = message[0];
	touch_id = report_id - 2;

	if (touch_id >= MXT_MAX_NUM_TOUCHES) {
		pr_err("[TSP] Invalid touch_id (toud_id=%d)", touch_id);
		return;
	}

	/* calculate positions of x, y */
	xpos = message[2];
	xpos = xpos << 4;
	xpos = xpos | (message[4] >> 4);
	xpos >>= 2;

	ypos = message[3];
	ypos = ypos << 4;
	ypos = ypos | (message[4] & 0x0F);
	ypos >>= 2;

	/************************************************************************
							defence coin lock-up added
	************************************************************************/
	if ((coin_check_count <= 2)/* && (cal_check_flag == 0)*/ && metal_suppression_chk_flag) {
		new_touch.id		= report_id;
		new_touch.status[new_touch.id]	= status;
		new_touch.xpos[new_touch.id]	= xpos;
		new_touch.ypos[new_touch.id]	= ypos;
		new_touch.area[new_touch.id]	= message[MXT_MSG_T9_TCHAREA];
		new_touch.amp[new_touch.id] 	= message[MXT_MSG_T9_TCHAMPLITUDE];

#ifdef TSP_DEBUG_MESSAGE
		msg_log.id[msg_log.cnt] 	= report_id;
		msg_log.status[msg_log.cnt] 	= status;
		msg_log.xpos[msg_log.cnt] 	= xpos;
		msg_log.ypos[msg_log.cnt] 	= ypos;
		msg_log.area[msg_log.cnt] 	= message[MXT_MSG_T9_TCHAREA];
		msg_log.amp[msg_log.cnt] 	= message[MXT_MSG_T9_TCHAMPLITUDE];
#endif

#if 0
		tch_vct[new_touch.id].length[tch_vct[new_touch.id].cnt] = ABS(old_touch.xpos[new_touch.id],  new_touch.xpos[new_touch.id]);
		tch_vct[new_touch.id].length[tch_vct[new_touch.id].cnt] += ABS(old_touch.ypos[new_touch.id],new_touch.ypos[new_touch.id]);

		if( new_touch.area[new_touch.id] != 0) {
			tch_vct[new_touch.id].angle[tch_vct[new_touch.id].cnt] = new_touch.amp[new_touch.id] / new_touch.area[new_touch.id];
		} else {
			tch_vct[new_touch.id].angle[tch_vct[new_touch.id].cnt] = 255;
		}
#else
		tch_vct[new_touch.id].length[new_touch.id] = ABS(old_touch.xpos[new_touch.id],  new_touch.xpos[new_touch.id]);
		tch_vct[new_touch.id].length[new_touch.id] += ABS(old_touch.ypos[new_touch.id],new_touch.ypos[new_touch.id]);

		if( new_touch.area[new_touch.id] != 0) {
			tch_vct[new_touch.id].angle[new_touch.id] = new_touch.amp[new_touch.id] / new_touch.area[new_touch.id];
		} else {
			tch_vct[new_touch.id].angle[new_touch.id] = 255;
		}
#endif
		/* Software auto calibration */
#if 1
		 if(new_touch.area[new_touch.id] > 15) {
			tch_vct[new_touch.id].cnt = 0;
		 /* 20120518 */
		} else if (new_touch.area[new_touch.id] < 4) {
			tch_vct[new_touch.id].cnt = 0;
		/* 20120518 */
		} else if (new_touch.amp[new_touch.id] > 45) {
			tch_vct[new_touch.id].cnt = 0;
		} else if (new_touch.amp[new_touch.id] < 10) {
			tch_vct[new_touch.id].cnt = 0;
		#if 0
		} else if( tch_vct[new_touch.id].length[tch_vct[new_touch.id].cnt] > 20 ){
			tch_vct[new_touch.id].cnt = 0;
		} else if( tch_vct[new_touch.id].angle[tch_vct[new_touch.id].cnt] > 12 ) {
			tch_vct[new_touch.id].cnt = 0;
		#else
		} else if( tch_vct[new_touch.id].length[new_touch.id] > 20 ){
			tch_vct[new_touch.id].cnt = 0;
		} else if( tch_vct[new_touch.id].angle[new_touch.id] > 15 ) {
			tch_vct[new_touch.id].cnt = 0;
		#endif
		} else{
			tch_vct[new_touch.id].cnt ++;
		}
#else
		 if (/* for metal coin floating */
			(((new_touch.area[new_touch.id] > 3) && (new_touch.area[new_touch.id] < 14))
			&& ((new_touch.amp[new_touch.id] > 9) && (new_touch.amp[new_touch.id] < 44))
			&& ( tch_vct[new_touch.id].length[tch_vct[new_touch.id].cnt] < 18 )
			&& ( tch_vct[new_touch.id].angle[tch_vct[new_touch.id].cnt] < 10 ))
#if 0
			/* for finger floating */
			|| ((new_touch.area[new_touch.id] < 2)
			&& ((new_touch.amp[new_touch.id] > 1) && (new_touch.amp[new_touch.id] < 5))
			&& ( tch_vct[new_touch.id].length[tch_vct[new_touch.id].cnt] == 0 ))
#endif
			)
		 {
			tch_vct[new_touch.id].cnt ++;
		 } else {
			tch_vct[new_touch.id].cnt = 0;
		 }
#endif
		if (debug >= DEBUG_TRACE) {
			pr_info("[TSP] TCH_MSG :  %4d, 0x%2x, %4d, %4d , area=%d,  amp=%d, \n",
				report_id,
				status,
				new_touch.xpos[new_touch.id], //xpos,
				new_touch.ypos[new_touch.id], //ypos,
				new_touch.area[new_touch.id], //message[MXT_MSG_T9_TCHAREA],
				new_touch.amp[new_touch.id]);	//message[MXT_MSG_T9_TCHAMPLITUDE]);

			pr_info("[TSP] TCH_VCT :  %4d, length=%d, angle=%d, cnt=%d  \n",
				new_touch.id,
			#if 0
				tch_vct[new_touch.id].length[tch_vct[new_touch.id].cnt],
				tch_vct[new_touch.id].angle[tch_vct[new_touch.id].cnt],
			#else
				tch_vct[new_touch.id].length[new_touch.id],
				tch_vct[new_touch.id].angle[new_touch.id],
			#endif
				tch_vct[new_touch.id].cnt);
		}

		if((tch_vct[new_touch.id].cnt >= 3) && (time_after_autocal_enable == 0)){
			check_chip_channel(mxt);
#if 0
			if (status & MXT_MSGB_T9_DETECT) {
				/* touch amplitude */
				mtouch_info[touch_id].pressure =
					message[MXT_MSG_T9_TCHAMPLITUDE];
			}
			for (i = 0; i < MXT_MAX_NUM_TOUCHES; i++) {
				if (mtouch_info[i].pressure == -1)
					continue;
				finger_channel_cnt++;
			}

			if (chk_touch_cnt < /* 120521 */
				((finger_channel_cnt * 2) + 1)) {
				klogi_if("[TSP] Floating metal Supressed"
					"(finger_channel_cnt : %d, chk_touch_cnt : %d)\n",
					finger_channel_cnt, chk_touch_cnt);
				calibrate_chip(mxt);

			} else if (chk_antitouch_cnt > 14) {
				pr_info("[TSP] Floating metal Suppressed"
					"(chk_antitouch_cnt = %d\n",
					chk_antitouch_cnt);
				calibrate_chip(mxt);

			} else if (((chk_touch_cnt < 7) ||
				(chk_touch_cnt > 19)) ||
				((chk_touch_cnt >= 7) &&
				((chk_antitouch_cnt >= 4) &&
				(chk_antitouch_cnt < (chk_touch_cnt + 3))))) {
				for (i = 0; i < 10; i++)
					tch_vct[i].cnt = 0;
#else
			if (((chk_touch_cnt < 7) || (chk_touch_cnt > 15)) ||
				((chk_touch_cnt >= 7) &&
				((chk_antitouch_cnt >= 4) &&
				(chk_antitouch_cnt < (chk_touch_cnt + 3))))) {
				for (i = 0; i < 10; i++)
					tch_vct[i].cnt = 0;
#endif
			} else {
#ifdef TSP_DEBUG_MESSAGE
				if (debug >= DEBUG_MESSAGES) {
					pr_info("[TSP] Floating metal Suppressed (ID = %d, msg_log.cnt = %d)!!! \n",new_touch.id,msg_log.cnt);
				}
#endif

				for(i=0;i < 10; i++) {
					tch_vct[i].cnt = 0;
				}

				/* T8_TCHAUTOCAL  */
				if (debug >= DEBUG_MESSAGES)
					pr_info("[TSP] Floating metal Suppressed!! Autocal = 3\n");
				error = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_TCHAUTOCAL, 3);
				if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

				time_after_autocal_enable = jiffies_to_msecs(jiffies);
			}
		} else if ((tch_vct[new_touch.id].cnt >= 4) && (time_after_autocal_enable == 1)){
			for(i=0;i < 10; i++) {
				tch_vct[i].cnt = 0;
			}
		}

		if (time_after_autocal_enable != 0) {
			if ((jiffies_to_msecs(jiffies) - time_after_autocal_enable) > 1500) {
				coin_check_count++;
				if (debug >= DEBUG_MESSAGES)
					pr_info("[TSP] Floating metal Suppressed time out!! Autocal = 0, (%d), coin_check_count = %d \n",
						(jiffies_to_msecs(jiffies) - time_after_autocal_enable),
						coin_check_count);

				/* T8_TCHAUTOCAL  */
				error = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_TCHAUTOCAL, 0);
				if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

				time_after_autocal_enable = 0;

				for(i=0;i < 10; i++) {
					tch_vct[i].cnt = 0;
				}
			}
		}
#ifdef TSP_DEBUG_MESSAGE
		msg_log.cnt++;
		msg_log.cnt &= (MAX_MSG_LOG_SIZE-1);
#endif

		old_touch.id	=  new_touch.id ;
		old_touch.status[new_touch.id]	=  new_touch.status[new_touch.id] ;
		old_touch.xpos[new_touch.id]	=  new_touch.xpos[new_touch.id] ;
		old_touch.ypos[new_touch.id]	=  new_touch.ypos[new_touch.id] ;
		old_touch.area[new_touch.id]	=  new_touch.area[new_touch.id] ;
		old_touch.amp[new_touch.id]	=  new_touch.amp[new_touch.id] ;
	}
	  /************************************************************************
							  end
	  ************************************************************************/


	if (status & MXT_MSGB_T9_DETECT) {   /* case 1: detected */


		mtouch_info[touch_id].pressure = message[MXT_MSG_T9_TCHAMPLITUDE];  /* touch amplitude */
		mtouch_info[touch_id].x = (int16_t)xpos;
		mtouch_info[touch_id].y = (int16_t)ypos;

		if (status & MXT_MSGB_T9_PRESS) {
			pressed_or_released = 1;  /* pressed */
#if defined(MXT_DRIVER_FILTER)
			equalize_coordinate(1, touch_id, &mtouch_info[touch_id].x, &mtouch_info[touch_id].y);
#endif
			touch_message_flag = 1;
			cal_move=0;

#ifdef TOUCH_LOCKUP_PATTERN_RELEASE
			touch_is_pressed_arr[touch_id] = 1;
#endif
		} else if (status & MXT_MSGB_T9_MOVE) {
#if defined(MXT_DRIVER_FILTER)
			equalize_coordinate(0, touch_id, &mtouch_info[touch_id].x, &mtouch_info[touch_id].y);
#endif
			if(cal_move++>=3){
				touch_message_flag = 1;
				cal_move=0;
			}
#ifdef TOUCH_LOCKUP_PATTERN_RELEASE
			touch_is_pressed_arr[touch_id] = 2;
#endif
		}
	} else if (status & MXT_MSGB_T9_RELEASE) {   /* case 2: released */
		pressed_or_released = 1;
		mtouch_info[touch_id].pressure = 0;
	} else if (status & MXT_MSGB_T9_SUPPRESS) {   /* case 3: suppressed */
	     /*
	     * Atmel's recommendation:
	     * In the case of supression,
	     * mxt224E chip doesn't make a release event.
	     * So we need to release them forcibly.
		*/
		if (debug >= DEBUG_MESSAGES)
			pr_info("[TSP] Palm(T9) Suppressed !!! \n");
		facesup_message_flag_T9 = 1;
		pressed_or_released = 1;
		mtouch_info[touch_id].pressure = 0;
	} else {
		pr_err("[TSP] Unknown status (0x%x)", status);

		if(facesup_message_flag_T9 == 1) {
			facesup_message_flag_T9 = 0;
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP] Palm(T92) Suppressed !!! \n");
		}

	}

	/*only get size , id would use TRACKING_ID*/
	mtouch_info[touch_id].size = message[MXT_MSG_T9_TCHAREA];

	if (prev_touch_id >= touch_id || pressed_or_released) {
		for (i = 0; i < MXT_MAX_NUM_TOUCHES; ++i) {
			if (mtouch_info[i].pressure == -1)
				continue;

			/* ADD TRACKING_ID*/
// N1_ICS
#ifdef TOUCH_LOCKUP_PATTERN_RELEASE
			tsp_pattern_tracking(i,
				mtouch_info[i].x, mtouch_info[i].y, mxt);

			if (touch_is_pressed_arr[i] == 1)
				presscount++;
			else if (touch_is_pressed_arr[i] == 2)
				movecount++;
#endif
			if (mtouch_info[i].pressure == 0) {
				mtouch_info[i].pressure = -1;
				input_mt_slot(mxt->input, i);
				input_mt_report_slot_state(mxt->input, MT_TOOL_FINGER, false);
			} else {
				input_mt_slot(mxt->input, i);
				input_mt_report_slot_state(mxt->input, MT_TOOL_FINGER, true);
				input_report_abs(mxt->input,
					ABS_MT_POSITION_X, mtouch_info[i].x);
				input_report_abs(mxt->input,
					ABS_MT_POSITION_Y, mtouch_info[i].y);
				input_report_abs(mxt->input,
					ABS_MT_PRESSURE,
					mtouch_info[i].pressure);
				input_report_abs(mxt->input,
					ABS_MT_TOUCH_MAJOR,
					mtouch_info[i].size);

				chkpress++;
			}
#ifdef TOUCH_LOCKUP_PATTERN_RELEASE
/*
Forced-calibration or Touch kernel reboot at power on or system wake-up.
This routine must be escaped from recursive calling by external ghost touch
that should be occured continuously.
Press event during move event by other finger is considered as ghost at resume.
Xtopher
*/
			if ((cal_check_flag == 1) && (ftouch_reboot == 0) &&
				(presscount >= 1) && (movecount >= 1)) {
				cghost_clear++;
				if (cghost_clear > MAX_COUNT_TOUCHSYSREBOOT) {
					ftouch_reboot = 1;
				/*printk(KERN_DEBUG "[TSP] Reboot-Touch S\n");*/
					cghost_clear = 0;
					TSP_forced_reboot(mxt);
				} else {
				/*printk(KERN_DEBUG
				"[TSP] Clear G-Touch at Boot/Wake-up\n");*/
					TSP_forced_release_for_call(mxt);
				}
			}
#endif
		}
		input_sync(input);  /* TO_CHK: correct position? */
	}
	prev_touch_id = touch_id;

#if defined(TSP_BOOST)
	if ((status & MXT_MSGB_T9_PRESS) && (!clk_lock_state)) {
		tegra_cpu_unlock_speed(false);
		tegra_cpu_lock_speed(608000, 0, false);
		clk_lock_state = true;
	} else if (clk_lock_state && (chkpress == 0) && pressed_or_released) {
		tegra_cpu_lock_speed(608000, 50, false);
		clk_lock_state = false;
	}
#endif

#ifdef TSP_INFO_LOG
	/* simple touch log */
	if (debug >= DEBUG_MESSAGES) {
		if (status & MXT_MSGB_T9_SUPPRESS) {
			pr_info("[TSP] Suppress\n");
			pr_info("[TSP] r (%d,%d) %d No.%d\n", xpos, ypos, touch_id, chkpress);

		} else {
			if (status & MXT_MSGB_T9_PRESS) {
				pr_info("[TSP] P (%d,%d) %d No.%d amp=%d area=%d\n", xpos, ypos, touch_id, chkpress, message[MXT_MSG_T9_TCHAMPLITUDE], message[MXT_MSG_T9_TCHAREA]);
			} else if (status & MXT_MSGB_T9_RELEASE) {
				pr_info("[TSP] r (%d,%d) %d No.%d\n", xpos, ypos, touch_id, chkpress);
			}
		}
	}

	/* detail touch log */
	if (debug >= DEBUG_TRACE) {
		char msg[64] = { 0};
		char info[64] = { 0};
		if (status & MXT_MSGB_T9_SUPPRESS) {
			strcpy(msg, "[TSP] Suppress: ");
		} else {
			if (status & MXT_MSGB_T9_DETECT) {
				strcpy(msg, "Detect(");
				if (status & MXT_MSGB_T9_PRESS)
					strcat(msg, "P");
				if (status & MXT_MSGB_T9_MOVE)
					strcat(msg, "M");
				if (status & MXT_MSGB_T9_AMP)
					strcat(msg, "A");
				if (status & MXT_MSGB_T9_VECTOR)
					strcat(msg, "V");
				strcat(msg, "): ");
			} else if (status & MXT_MSGB_T9_RELEASE) {
				strcpy(msg, "Release: ");
			} else {
				strcpy(msg, "[!] Unknown status: ");
			}
		}
		sprintf(info, "[TSP] (%d,%d) amp=%d, size=%d, id=%d", xpos, ypos,
			message[MXT_MSG_T9_TCHAMPLITUDE], message[MXT_MSG_T9_TCHAREA], touch_id);
		strcat(msg, info);
		pr_info("%s\n", msg);
	}
#endif


	if(cal_check_flag == 1) {
		/* If chip has recently calibrated and there are any touch or face suppression
		* messages, we must confirm if the calibration is good. */
		if(touch_message_flag) {
			if(timer_flag == DISABLE) {
				timer_flag = ENABLE;
				timer_ticks = 0u;
				ts_100ms_timer_start(mxt);
			}
			if (mxt_time_point == 0)
				mxt_time_point = jiffies_to_msecs(jiffies);

			// noise : cal problem
			if (!caling_check){
				check_chip_calibration(mxt);
			}else{
				cal_move=3;		// next try cal check
		   }
		}
	}
#if 0
	if(mxt->check_auto_cal == 5) {
		if (debug >= DEBUG_MESSAGES)
			pr_info("[TSP] Autocal = 0 \n");
		mxt->check_auto_cal = 0;
		error = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_TCHAUTOCAL, 0);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);
	}
#endif
	return;
}

void process_T15_message(u8 *message, struct mxt_data *mxt)
{
	struct	input_dev *input;
	u8  status = false;
	int error = 0;

	input = mxt->input;
	status = message[MXT_MSG_T15_STATUS];
#if 0
	check_chip_channel(mxt);

	if (chk_antitouch_cnt > 15) {

		/* T8_TCHAUTOCAL */
		if (debug >= DEBUG_MESSAGES)
			pr_info("[TSP] Key AntiTouch Check! Autocal = 3sec\n");
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) +
			MXT_ADR_T8_TCHAUTOCAL, 10); /*10x 200msec = 2sec*/
		if (error < 0)
			pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		time_after_autocal_enable_key = jiffies_to_msecs(jiffies);
	} else {
		time_after_autocal_enable_key = 0;
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) +
			MXT_ADR_T8_TCHAUTOCAL, 0);
		pr_info("[TSP] Key AntiTouch Check_Disable!! Autocal = 0, (%d)\n",
			(jiffies_to_msecs(jiffies) -
			time_after_autocal_enable_key));
	}

	if (time_after_autocal_enable_key != 0) {
		if ((jiffies_to_msecs(jiffies) -
			time_after_autocal_enable_key) > 2000) {
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP] Key AntiTouch Check!! Autocal = 0, (%d)\n",
					(jiffies_to_msecs(jiffies) -
					time_after_autocal_enable_key));

			/* T8_TCHAUTOCAL  */
			error = mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) +
				MXT_ADR_T8_TCHAUTOCAL, 0);
			if (error < 0)
				pr_err("[TSP] %s, %d Error!!\n",
					__func__, __LINE__);

				time_after_autocal_enable_key = 0;
		}
	}
#endif
	/* whether reportid is thing of atmel_mxt224E_TOUCH_KEYARRAY */
	/* single key configuration*/
	if ((mxt->pdata->board_rev <= 9) || (mxt->pdata->board_rev >= 13)) {
		if (message[MXT_MSG_T15_STATUS] & MXT_MSGB_T15_DETECT) {
			if (tsp_keystatus != TOUCH_KEY_NULL) {	/* defence code, if there is any Pressed key, force release!! */
				switch (tsp_keystatus) {
				case TOUCH_4KEY_MENU:
					input_report_key(mxt->input, KEY_MENU, KEY_RELEASE);
					break;
				case TOUCH_4KEY_BACK:
					input_report_key(mxt->input, KEY_BACK, KEY_RELEASE);
					break;
				default:
					break;
				}
#ifdef TSP_INFO_LOG
				if (debug >= DEBUG_MESSAGES)
					pr_info("[TSP_KEY] r %s\n", tsp_2keyname[tsp_keystatus - 1]);
#endif
				tsp_keystatus = TOUCH_KEY_NULL;
			}

			switch (message[MXT_MSG_T15_KEYSTATE]) {
			case TOUCH_2KEY_MENU:
				input_report_key(mxt->input, KEY_MENU, KEY_PRESS);
				tsp_keystatus = TOUCH_2KEY_MENU;
				break;
			case TOUCH_2KEY_BACK:
				input_report_key(mxt->input, KEY_BACK, KEY_PRESS);
				tsp_keystatus = TOUCH_2KEY_BACK;
				break;
			default:
				break;
			}

#ifdef TSP_INFO_LOG
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP_KEY] P %s\n", tsp_2keyname[tsp_keystatus - 1]);
#endif
#ifdef KEY_LED_CONTROL
		if (mxt->pdata->board_rev <= 9) {
			if (key_led_status) {
				key_led_on(mxt, tsp_2key_led_ctrl[tsp_keystatus-1]);
			}
		}
#endif
		} else {
			switch (tsp_keystatus) {
			case TOUCH_2KEY_MENU:
				input_report_key(mxt->input, KEY_MENU, KEY_RELEASE);
				break;
			case TOUCH_2KEY_BACK:
				input_report_key(mxt->input, KEY_BACK, KEY_RELEASE);
				break;
			default:
				break;
			}
#ifdef TSP_INFO_LOG
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP_KEY] r %s\n", tsp_2keyname[tsp_keystatus - 1]);
#endif
			tsp_keystatus = TOUCH_KEY_NULL;
#ifdef KEY_LED_CONTROL
			if (mxt->pdata->board_rev <= 9) {
				if (key_led_status) {
					key_led_on(mxt, 0xFF);
				}
			}
#endif

		}
	}		else {	/* board rev1.0~1.2, touch key is 4 key array, No LED blinking */
		if (message[MXT_MSG_T15_STATUS] & MXT_MSGB_T15_DETECT) { /* TOUCH KEY PRESS */
			if (tsp_keystatus != TOUCH_KEY_NULL) {	/* defence code, if there is any Pressed key, force release!! */
				switch (tsp_keystatus) {
				case TOUCH_4KEY_MENU:
					input_report_key(mxt->input, KEY_MENU, KEY_RELEASE);
					break;
				case TOUCH_4KEY_HOME:
					input_report_key(mxt->input, KEY_HOME, KEY_RELEASE);
					break;
				case TOUCH_4KEY_BACK:
					input_report_key(mxt->input, KEY_BACK, KEY_RELEASE);
					break;
				case TOUCH_4KEY_SEARCH:
					input_report_key(mxt->input, KEY_SEARCH, KEY_RELEASE);
					break;
				default:
					break;
				}
#ifdef TSP_INFO_LOG
				if (debug >= DEBUG_MESSAGES)
					pr_info("[TSP_KEY] r %s\n", tsp_4keyname[tsp_keystatus - 1]);
#endif
				tsp_keystatus = TOUCH_KEY_NULL;
			}

			switch (message[MXT_MSG_T15_KEYSTATE]) {
			case 0x01:
				input_report_key(mxt->input, KEY_MENU, KEY_PRESS);
				tsp_keystatus = TOUCH_4KEY_MENU;
				break;
			case 0x02:
				input_report_key(mxt->input, KEY_HOME, KEY_PRESS);
				tsp_keystatus = TOUCH_4KEY_HOME;
				break;
			case 0x04:
				input_report_key(mxt->input, KEY_BACK, KEY_PRESS);
				tsp_keystatus = TOUCH_4KEY_BACK;
				break;
			case 0x08:
				input_report_key(mxt->input, KEY_SEARCH, KEY_PRESS);
				tsp_keystatus = TOUCH_4KEY_SEARCH;
				break;
			default:
				break;
			}

#ifdef TSP_INFO_LOG
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP_KEY] P %s\n", tsp_4keyname[tsp_keystatus - 1]);
#endif
		} else { /* TOUCH KEY RELEASE */
			switch (tsp_keystatus) {
			case TOUCH_4KEY_MENU:
				input_report_key(mxt->input, KEY_MENU, KEY_RELEASE);
				break;
			case TOUCH_4KEY_HOME:
				input_report_key(mxt->input, KEY_HOME, KEY_RELEASE);
				break;
			case TOUCH_4KEY_BACK:
				input_report_key(mxt->input, KEY_BACK, KEY_RELEASE);
				break;
			case TOUCH_4KEY_SEARCH:
				input_report_key(mxt->input, KEY_SEARCH, KEY_RELEASE);
				break;
			default:
				break;
			}
#ifdef TSP_INFO_LOG
			if (debug >= DEBUG_MESSAGES)
				pr_info("[TSP_KEY] r %s\n", tsp_4keyname[tsp_keystatus - 1]);
#endif
			tsp_keystatus = TOUCH_KEY_NULL;
		}
	}
	input_sync(mxt->input);
	return;
}

static void mxt_palm_recovery(struct work_struct *work)
{
	struct	mxt_data *mxt;
	int error = 0;
	mxt = container_of(work, struct mxt_data, config_dwork.work);
	/*client = mxt->client;*/
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] %s \n", __func__);

	if( mxt->check_auto_cal == 1)
	{
		mxt->check_auto_cal = 2;
		/* T8_ATCHCALST */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHCALST,
			T8_ATCHCALST);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		/* T8_ATCHCALSTHR */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHCALSTHR,
			T8_ATCHCALSTHR);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		/* T8_ATCHFRCCALTHR */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALTHR,
			T8_ATCHFRCCALTHR);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		/* T8_ATCHFRCCALRATIO */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALRATIO,
			T8_ATCHFRCCALRATIO);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		/* T42_MAXNUMTCHS */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_PROCI_TOUCHSUPPRESSION_T42) + MXT_ADR_T42_MAXNUMTCHS,
			T42_MAXNUMTCHS);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

	} else if(mxt->check_auto_cal == 2) {
		mxt->check_auto_cal = 1;
		/* T8_ATCHCALST */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHCALST,
			T8_ATCHCALST);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		/* T8_ATCHCALSTHR */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHCALSTHR,
			T8_ATCHCALSTHR);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		/* T8_ATCHFRCCALTHR */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALTHR,
			0);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		/* T8_ATCHFRCCALRATIO */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALRATIO,
			0);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

		/* T42_MAXNUMTCHS */
		error = mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_PROCI_TOUCHSUPPRESSION_T42) + MXT_ADR_T42_MAXNUMTCHS,
			0);
		if (error < 0) pr_err("[TSP] %s, %d Error!!\n", __func__, __LINE__);

	/* Case 3 : Autocalibration enabled */
	} else if(mxt->check_auto_cal == 3) {
		mxt->check_auto_cal = 5;

	/* Case 4 : Calibrated from none Calibraton command*/
	} else if(mxt->check_auto_cal == 4) {
	        mxt->check_auto_cal = 0;
		facesup_message_flag  = 0;
	}
}

void process_T42_message(u8 *message, struct mxt_data *mxt)
{
	struct	input_dev *input;
	u8  status = false;

	input = mxt->input;
	status = message[MXT_MSG_T42_STATUS];
#if 0
	/* Read Charging Time */
	uint8_t charg_time;
	mxt_read_byte(mxt->client,
		MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + 0, &charg_time);
	klogi_if("[TSP] T8 CHRGTIME =  %d\n", charg_time);
#endif
	/* whether reportid is thing of atmel_mxt224E_TOUCH_KEYARRAY */
	/* single key configuration*/

	if (message[MXT_MSG_T42_STATUS] == MXT_MSGB_T42_TCHSUP) {
		palm_release_flag = false;
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] Palm(T42) Suppressed !!!\n");

		// 0506 LBK
		if (facesup_message_flag && timer_flag)
			return;

		check_chip_palm(mxt);

		if(facesup_message_flag)
		{
			/* 100ms timer Enable */
			timer_flag = ENABLE;
			timer_ticks = 0;
			ts_100ms_timer_start(mxt);
			klogi_if("[TSP] Palm(T42) Timer start !!!\n");
		}
	} else {
		if (first_palm_chk == true)
			first_palm_chk = false;

		if (debug >= DEBUG_INFO)
			pr_info("[TSP] Palm(T42) Released !!!\n");

		palm_release_flag = true;
		facesup_message_flag = 0;
		/* 100ms timer disable */
		timer_flag = DISABLE;
		timer_ticks = 0;
		ts_100ms_timer_stop(mxt);
	}
	return;
}


int process_message(u8 *message, u8 object, struct mxt_data *mxt)
{

	struct i2c_client *client;

	u8  status, state;
	u16 xpos = 0xFFFF;
	u16 ypos = 0xFFFF;
	u8  event;
	u8  length;
	u8  report_id;

	client = mxt->client;
	length = mxt->message_size;
	report_id = message[0];

	if (debug >= DEBUG_TRACE)
		pr_info("process_message 0: (0x%x) 1:(0x%x) object:(%d)", message[0], message[1], object);

	switch (object) {
	case MXT_PROCG_NOISESUPPRESSION_T48:
		state = message[4];
		if (state == 0x05) {	/* error state */
			if (debug >= DEBUG_MESSAGES)
				dev_info(&client->dev, "[TSP] NOISESUPPRESSION_T48 error\n");
#ifdef MEDIAN_FILTER_ERROR_SET
			if ((!(mxt->set_mode_for_ta)) &&
				(median_error_flag == 0)) {
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) +
				MXT_ADR_T9_BLEN, 16);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) +
				MXT_ADR_T9_TCHTHR, 40);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) +
				MXT_ADR_T9_MOVFILTER, 80);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) +
				MXT_ADR_T48_BASEFREQ, 29);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) +
				MXT_ADR_T48_MFFREQ_2, 1);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) +
				MXT_ADR_T48_MFFREQ_3, 2);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) +
				MXT_ADR_T48_GCMAXADCSPERX, 100);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) +
				MXT_ADR_T48_GCLIMITMAX, 64);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) +
				MXT_ADR_T48_MFINVLDDIFFTHR, 20);
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) +
				MXT_ADR_T48_MFERRORTHR, 38);
				median_error_flag = 1;
			}
		}
		if ((state == 0x04) &&
			(!(mxt->set_mode_for_ta))) { /* error state */
			mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9)
				+ MXT_ADR_T9_NEXTTCHDI, 0);
		}
#else
		}
#endif
		break;
	case MXT_GEN_COMMANDPROCESSOR_T6:
		status = message[1];
		if (status & MXT_MSGB_T6_COMSERR) {
			dev_err(&client->dev, "[TSP] maXTouch checksum error\n");
		}
		if (status & MXT_MSGB_T6_CFGERR) {
			dev_err(&client->dev, "[TSP] maXTouch configuration error\n");
			reset_chip(mxt, RESET_TO_NORMAL);
			msleep(250);
			/* re-configurate */
			mxt_config_settings(mxt);
			/* backup to nv memory */
			backup_to_nv(mxt);
			/* forces a reset of the chipset */
			reset_chip(mxt, RESET_TO_NORMAL);
			msleep(250); /*need 250ms*/
		}
		if (status & MXT_MSGB_T6_CAL) {
			if (debug >= DEBUG_MESSAGES)
				dev_info(&client->dev, "[TSP] maXTouch calibration in progress(cal_check_flag = %d)\n",cal_check_flag);

			if(cal_check_flag == 0)
			{
				/* ATMEL_DEBUG 0406 */
				mxt->check_auto_cal = 4;
				/* mxt_palm_recovery(mxt); */
				cancel_delayed_work(&mxt->config_dwork);
				schedule_delayed_work(&mxt->config_dwork, 0);
				cal_check_flag = 1;
			}
		}
		if (status & MXT_MSGB_T6_SIGERR) {
			dev_err(&client->dev,
				"[TSP] maXTouch acquisition error\n");
#ifdef MXT_ESD_WORKAROUND
			mxt_release_all_fingers(mxt);
			mxt_release_all_keys(mxt);
			mxt_force_reset(mxt);
#endif
		}
		if (status & MXT_MSGB_T6_OFL) {
			dev_err(&client->dev, "[TSP] maXTouch cycle overflow\n");
		}
		if (status & MXT_MSGB_T6_RESET) {
			if (debug >= DEBUG_MESSAGES)
				dev_info(&client->dev, "[TSP] maXTouch chip reset\n");
		}
		if (status == MXT_MSG_T6_STATUS_NORMAL) {
			if (debug >= DEBUG_MESSAGES)
				dev_info(&client->dev, "[TSP] maXTouch status normal\n");
#if defined(MXT_FACTORY_TEST) || defined(MXT_FIRMUP_ENABLE)
						/*check if firmware started*/
						if (mxt->firm_status_data == 1) {
							if (debug >= DEBUG_MESSAGES)
								dev_info(&client->dev,
									"maXTouch mxt->firm_normal_status_ack after firm up\n");
							/*got normal status ack*/
							mxt->firm_normal_status_ack = 1;
						}
#endif

		}
		break;

	case MXT_TOUCH_MULTITOUCHSCREEN_T9:
		process_T9_message(message, mxt);
		break;

	case MXT_TOUCH_KEYARRAY_T15:
		process_T15_message(message, mxt);
		break;

	case MXT_PROCI_TOUCHSUPPRESSION_T42:
		process_T42_message(message, mxt);
		if (debug >= DEBUG_TRACE)
			dev_info(&client->dev, "[TSP] Receiving Touch suppression msg\n");
		break;

	case MXT_PROCI_ONETOUCHGESTUREPROCESSOR_T24:
		if (debug >= DEBUG_TRACE)
			dev_info(&client->dev,
			"[TSP] Receiving one-touch gesture msg\n");

		event = message[MXT_MSG_T24_STATUS] & 0x0F;
		xpos = message[MXT_MSG_T24_XPOSMSB] * 16 +
			((message[MXT_MSG_T24_XYPOSLSB] >> 4) & 0x0F);
		ypos = message[MXT_MSG_T24_YPOSMSB] * 16 +
			((message[MXT_MSG_T24_XYPOSLSB] >> 0) & 0x0F);
		xpos >>= 2;
		ypos >>= 2;

		switch (event) {
		case	MT_GESTURE_RESERVED:
			break;
		case	MT_GESTURE_PRESS:
			break;
		case	MT_GESTURE_RELEASE:
			break;
		case	MT_GESTURE_TAP:
			break;
		case	MT_GESTURE_DOUBLE_TAP:
			break;
		case	MT_GESTURE_FLICK:
			break;
		case	MT_GESTURE_DRAG:
			break;
		case	MT_GESTURE_SHORT_PRESS:
			break;
		case	MT_GESTURE_LONG_PRESS:
			break;
		case	MT_GESTURE_REPEAT_PRESS:
			break;
		case	MT_GESTURE_TAP_AND_PRESS:
			break;
		case	MT_GESTURE_THROW:
			break;
		default:
			break;
		}
		break;

		case MXT_SPT_SELFTEST_T25:
			if (debug >= DEBUG_TRACE) {
				dev_info(&client->dev,"[TSP] Receiving Self-Test msg\n");
			}

			if (message[MXT_MSG_T25_STATUS] == MXT_MSGR_T25_OK) {
				if (debug >= DEBUG_TRACE)
					dev_info(&client->dev,
					"[TSP] maXTouch: Self-Test OK\n");

			} else  {
				dev_err(&client->dev,
					"[TSP] maXTouch: Self-Test Failed [%02x]:"
					"{ %02x,%02x,%02x,%02x,%02x}\n",
					message[MXT_MSG_T25_STATUS],
					message[MXT_MSG_T25_STATUS + 0],
					message[MXT_MSG_T25_STATUS + 1],
					message[MXT_MSG_T25_STATUS + 2],
					message[MXT_MSG_T25_STATUS + 3],
					message[MXT_MSG_T25_STATUS + 4]
					);
			}
			break;

		case MXT_SPT_CTECONFIG_T46:
			if (debug >= DEBUG_TRACE)
				dev_info(&client->dev,
				"[TSP] Receiving CTE message...\n");
			status = message[MXT_MSG_T46_STATUS];
			if (status & MXT_MSGB_T46_CHKERR)
				dev_err(&client->dev,
				"[TSP] maXTouch: Power-Up CRC failure\n");

			break;
		default:
			if (debug >= DEBUG_TRACE)
				dev_info(&client->dev,"[TSP] maXTouch: Unknown message!\n");

			break;
	}

	return 0;
}


#ifdef MXT_THREADED_IRQ
/* Processes messages when the interrupt line (CHG) is asserted. */

static void mxt_threaded_irq_handler(struct mxt_data *mxt)
{
	struct	i2c_client *client;

	u8 message[mxt->message_size];
	u16	message_length;
	u16	message_addr;
	u8	report_id = 0;
	u8	object;
	int	error;
	bool need_reset = false;
	int	i;

	client = mxt->client;
	message_addr = 	mxt->msg_proc_addr;
	message_length = mxt->message_size;
	if (debug >= DEBUG_TRACE)
		dev_info(&mxt->client->dev, "[TSP] maXTouch worker active:\n");

	/* Read next message */
	mxt->message_counter++;
	/* Reread on failure! */
	for (i = I2C_RETRY_COUNT; i > 0; i--) {
		/* note: changed message_length to 8 in ver0.9*/
		error = mxt_read_block(client, message_addr,
			8/*message_length*/, message);
		if (error >= 0)
			break;
		mxt->read_fail_counter++;
		pr_alert("[TSP] mXT: message read failed!\n");
		/* Register read failed */
		dev_err(&client->dev, "[TSP] Failure reading maxTouch device\n");
	}
	if (i == 0) {
		need_reset = true;

		/* TSP reset */
		mxt_release_all_fingers(mxt);
		mxt_release_all_keys(mxt);
		mxt_force_reset(mxt);
		return;
	}
	report_id = message[0];
	if (debug >= DEBUG_RAW) {
		pr_info("[TSP] %s message [%08x]:",
			REPORT_ID_TO_OBJECT_NAME(report_id),
			mxt->message_counter);
		for (i = 0; i < message_length; i++)
			pr_info("0x%02x ", message[i]);
		pr_info("\n");
	}

	if ((report_id != MXT_END_OF_MESSAGES) && (report_id != 0)) {

		for (i = 0; i < message_length; i++)
			mxt->last_message[i] = message[i];
/*
		if (down_interruptible(&mxt->msg_sem)) {
			pr_warning("[TSP] mxt_worker Interrupted "
				"while waiting for msg_sem!\n");
			return;
		}
		mxt->new_msgs = 1;
		up(&mxt->msg_sem);
		wake_up_interruptible(&mxt->msg_queue);
*/
		/* Get type of object and process the message */
		object = mxt->rid_map[report_id].object;
		process_message(message, object, mxt);
	} else if (report_id == MXT_END_OF_MESSAGES) {
		dev_err(&client->dev, "[TSP] MXT_END_OF_MESSAGES\n");
	}
}

static irqreturn_t mxt_threaded_irq(int irq, void *_mxt)
{
	struct	mxt_data *mxt = _mxt;
	mxt->irq_counter++;
	mxt_threaded_irq_handler(mxt);
	return IRQ_HANDLED;
}


/* boot initial delayed work */
static void mxt_boot_delayed_initial(struct work_struct *work)
{
	int error, i;
	struct	mxt_data *mxt;
	mxt = container_of(work, struct mxt_data, initial_dwork.work);

	if (debug >= DEBUG_MESSAGES)
				dev_info(&mxt->client->dev, "[TSP] %s\n", __func__);

	calibrate_chip(mxt);

	if (mxt->irq) {
	/* Try to request IRQ with falling edge first. This is
		* not always supported. If it fails, try with any edge. */
#ifdef MXT_THREADED_IRQ
		error = request_threaded_irq(mxt->irq,
			NULL,
			mxt_threaded_irq,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT,
			mxt->client->dev.driver->name,
			mxt);
		if (error < 0) {
			error = request_threaded_irq(mxt->irq,
				NULL,
				mxt_threaded_irq,
				IRQF_DISABLED,
				mxt->client->dev.driver->name,
				mxt);
		}
#else
		error = request_irq(mxt->irq,
			mxt_irq_handler,
			IRQF_TRIGGER_FALLING,
			mxt->client->dev.driver->name,
			mxt);
		if (error < 0) {
			error = request_irq(mxt->irq,
				mxt_irq_handler,
				0,
				mxt->client->dev.driver->name,
				mxt);
		}
#endif
		if (error < 0) {
			dev_err(&mxt->client->dev,
				"[TSP] failed to allocate irq %d\n", mxt->irq);
		}
	}

	if (debug > DEBUG_INFO)
		dev_info(&mxt->client->dev, "[TSP] touchscreen, irq %d\n", mxt->irq);

	return;
}

#else
/* Processes messages when the interrupt line (CHG) is asserted. */
static void mxt_worker(struct work_struct *work)
{
	struct	mxt_data *mxt;
	struct	i2c_client *client;

	u8	*message;
	u16	message_length;
	u16	message_addr;
	u8	report_id;
	u8	object;
	int	error;
	int	i;

	message = NULL;
	mxt = container_of(work, struct mxt_data, dwork.work);
	client = mxt->client;
	message_addr = mxt->msg_proc_addr;
	message_length = mxt->message_size;

	if (message_length < 256) {
		message = kmalloc(message_length, GFP_KERNEL);
		if (message == NULL) {
			dev_err(&client->dev, "[TSP] Error allocating memory\n");
			return;
		}
	} else {
		dev_err(&client->dev,
			"[TSP] Message length larger than 256 bytes not supported\n");
	}

	if (debug >= DEBUG_TRACE)
		dev_info(&mxt->client->dev, "[TSP] maXTouch worker active: \n");

	do {
		/* Read next message */
		mxt->message_counter++;
		/* Reread on failure! */
		for (i = 1; i < I2C_RETRY_COUNT; i++) {
			/* note: changed message_length to 8 in ver0.9 */
			error = mxt_read_block(client, message_addr, 8/*message_length*/, message);
			if (error >= 0)
				break;
			mxt->read_fail_counter++;
			pr_alert("[TSP] mXT: message read failed!\n");
			/* Register read failed */
			dev_err(&client->dev,
				"[TSP] Failure reading maxTouch device\n");
		}

		report_id = message[0];
		if (debug >= DEBUG_RAW) {
			pr_info("[TSP] %s message [%08x]:",
				REPORT_ID_TO_OBJECT_NAME(report_id),
				mxt->message_counter
				);
			for (i = 0; i < message_length; i++) {
				pr_info("0x%02x ", message[i]);;
			}
			pr_info("\n");
		}

		if ((report_id != MXT_END_OF_MESSAGES) && (report_id != 0)) {

			for (i = 0; i < message_length; i++)
				mxt->last_message[i] = message[i];

			if (down_interruptible(&mxt->msg_sem)) {
				pr_warning("[TSP] mxt_worker Interrupted "
					"while waiting for msg_sem!\n");
				kfree(message);
				return;
			}
			mxt->new_msgs = 1;
			up(&mxt->msg_sem);
			wake_up_interruptible(&mxt->msg_queue);
			/* Get type of object and process the message */
			object = mxt->rid_map[report_id].object;
			process_message(message, object, mxt);
		}

	} while ((report_id != MXT_END_OF_MESSAGES) && (report_id != 0));

	kfree(message);
}


/*
* The maXTouch device will signal the host about a new message by asserting
* the CHG line. This ISR schedules a worker routine to read the message when
* that happens.
*/

static irqreturn_t mxt_irq_handler(int irq, void *_mxt)
{
	struct	mxt_data *mxt = _mxt;
	unsigned long	flags;
	mxt->irq_counter++;
	spin_lock_irqsave(&mxt->lock, flags);

	if (mxt_valid_interrupt()) {
		/* Send the signal only if falling edge generated the irq. */
		cancel_delayed_work(&mxt->dwork);
		schedule_delayed_work(&mxt->dwork, 0);
		mxt->valid_irq_counter++;
	} else {
		mxt->invalid_irq_counter++;
	}
	spin_unlock_irqrestore(&mxt->lock, flags);

	return IRQ_HANDLED;
}
#endif


/* Function to write a block of data to any address on touch chip. */

#define I2C_PAYLOAD_SIZE 254

static ssize_t set_config(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf,
			  size_t count)
{
	int i;

	u16 address;
	int whole_blocks;
	int last_block_size;

	struct i2c_client *client  = to_i2c_client(dev);

	address = *((u16 *) buf);
	address = cpu_to_be16(address);
	buf += 2;

	whole_blocks = (count - 2) / I2C_PAYLOAD_SIZE;
	last_block_size = (count - 2) % I2C_PAYLOAD_SIZE;

	for (i = 0; i < whole_blocks; i++) {
		mxt_write_block(client, address, I2C_PAYLOAD_SIZE, (u8 *) buf);
		address += I2C_PAYLOAD_SIZE;
		buf += I2C_PAYLOAD_SIZE;
	}

	mxt_write_block(client, address, last_block_size, (u8 *) buf);

	return count;

}

static ssize_t get_config(struct device *dev,
			  struct device_attribute *attr,
			  char *buf)
{
	int i;
	struct i2c_client *client  = to_i2c_client(dev);
	struct mxt_data *mxt = i2c_get_clientdata(client);

	pr_warning("[TSP] Reading %d bytes from current ap\n",
		mxt->bytes_to_read);

	i = mxt_read_block_wo_addr(client, mxt->bytes_to_read, (u8 *) buf);

	return (ssize_t) i;

}

/*
* Sets up a read from mXT chip. If we want to read config data from user space
* we need to use this first to tell the address and byte count, then use
* get_config to read the data.
*/

static ssize_t set_ap(struct device *dev,
		      struct device_attribute *attr,
		      const char *buf,
		      size_t count)
{

	int i;
	struct i2c_client *client;
	struct mxt_data *mxt;
	u16 ap;

	client = to_i2c_client(dev);
	mxt = i2c_get_clientdata(client);

	if (count < 3) {
		/* Error, ap needs to be two bytes, plus 1 for size! */
		pr_err("[TSP] set_ap needs to arguments: address pointer "
			"and data size");
		return -EIO;
	}

	ap = (u16) *((u16 *)buf);
	i = mxt_write_ap(client, ap);
	mxt->bytes_to_read = (u16) *(buf + 2);
	return count;

}


static ssize_t show_deltas(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	struct i2c_client *client;
	struct mxt_data *mxt;
	s16     *delta;
	s16     size, read_size;
	u16     diagnostics;
	u16     debug_diagnostics;
	char    *bufp;
	int     x, y;
	int     error;
	u16     *val;

	client = to_i2c_client(dev);
	mxt = i2c_get_clientdata(client);

	/* Allocate buffer for delta's */
	size = mxt->device_info.num_nodes * sizeof(__u16);
	if (mxt->delta == NULL) {
		mxt->delta = kzalloc(size, GFP_KERNEL);
		if (!mxt->delta) {
			sprintf(buf, "insufficient memory\n");
			return strlen(buf);
		}
	}

	if (mxt->object_table[MXT_GEN_COMMANDPROCESSOR_T6].type == 0) {
		dev_err(&client->dev, "[TSP] maXTouch: Object T6 not found\n");
		return 0;
	}
	diagnostics =  T6_REG(MXT_ADR_T6_DIAGNOSTICS);
	if (mxt->object_table[MXT_DEBUG_DIAGNOSTICS_T37].type == 0) {
		dev_err(&client->dev, "[TSP] maXTouch: Object T37 not found\n");
		return 0;
	}
	debug_diagnostics = T37_REG(2);

	/* Configure T37 to show deltas */
	error = mxt_write_byte(client, diagnostics, MXT_CMD_T6_DELTAS_MODE);
	if (error)
		return error;

	delta = mxt->delta;

	while (size > 0) {
		read_size = size > 128 ? 128 : size;
		error = mxt_read_block(client,
			debug_diagnostics,
			read_size,
			(__u8 *) delta);
		if (error < 0) {
			mxt->read_fail_counter++;
			dev_err(&client->dev,
				"[TSP] maXTouch: Error reading delta object\n");
		}
		delta += (read_size / 2);
		size -= read_size;
		/* Select next page */
		mxt_write_byte(client, diagnostics, MXT_CMD_T6_PAGE_UP);
	}

	bufp = buf;
	val  = (s16 *) mxt->delta;
	for (x = 0; x < mxt->device_info.x_size; x++) {
		for (y = 0; y < mxt->device_info.y_size; y++)
			bufp += sprintf(bufp, "%05d  ",
			(s16) le16_to_cpu(*val++));
		bufp -= 2;	/* No spaces at the end */
		bufp += sprintf(bufp, "\n");
	}
	bufp += sprintf(bufp, "\n");
	return strlen(buf);
}


static ssize_t show_references(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct i2c_client *client;
	struct mxt_data *mxt;
	s16   *reference;
	s16   size, read_size;
	u16   diagnostics;
	u16   debug_diagnostics;
	char  *bufp;
	int   x, y;
	int   error;
	u16   *val;

	client = to_i2c_client(dev);
	mxt = i2c_get_clientdata(client);
	/* Allocate buffer for reference's */
	size = mxt->device_info.num_nodes * sizeof(u16);
	if (mxt->reference == NULL) {
		mxt->reference = kzalloc(size, GFP_KERNEL);
		if (!mxt->reference) {
			sprintf(buf, "insufficient memory\n");
			return strlen(buf);
		}
	}

	if (mxt->object_table[MXT_GEN_COMMANDPROCESSOR_T6].type == 0) {
		dev_err(&client->dev, "[TSP] maXTouch: Object T6 not found\n");
		return 0;
	}
	diagnostics =  T6_REG(MXT_ADR_T6_DIAGNOSTICS);
	if (mxt->object_table[MXT_DEBUG_DIAGNOSTICS_T37].type == 0) {
		dev_err(&client->dev, "[TSP] maXTouch: Object T37 not found\n");
		return 0;
	}
	debug_diagnostics = T37_REG(2);

	/* Configure T37 to show references */
	mxt_write_byte(client, diagnostics, MXT_CMD_T6_REFERENCES_MODE);
	/* Should check for error */
	reference = mxt->reference;
	while (size > 0) {
		read_size = size > 128 ? 128 : size;
		error = mxt_read_block(client,
			debug_diagnostics,
			read_size,
			(__u8 *) reference);
		if (error < 0) {
			mxt->read_fail_counter++;
			dev_err(&client->dev,
				"[TSP] maXTouch: Error reading reference object\n");
		}
		reference += (read_size / 2);
		size -= read_size;
		/* Select next page */
		mxt_write_byte(client, diagnostics, MXT_CMD_T6_PAGE_UP);
	}

	bufp = buf;
	val  = (u16 *) mxt->reference;

	for (x = 0; x < mxt->device_info.x_size; x++) {
		for (y = 0; y < mxt->device_info.y_size; y++)
			bufp += sprintf(bufp, "%05d  ", le16_to_cpu(*val++));
		bufp -= 2; /* No spaces at the end */
		bufp += sprintf(bufp, "\n");
	}
	bufp += sprintf(bufp, "\n");
	return strlen(buf);
}

static ssize_t show_device_info(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct i2c_client *client;
	struct mxt_data *mxt;
	char *bufp;

	client = to_i2c_client(dev);
	mxt = i2c_get_clientdata(client);

	bufp = buf;
	bufp += sprintf(bufp,
		"Family:\t\t\t[0x%02x] %s\n",
		mxt->device_info.family_id,
		mxt->device_info.family
		);
	bufp += sprintf(bufp,
		"Variant:\t\t[0x%02x] %s\n",
		mxt->device_info.variant_id,
		mxt->device_info.variant
		);
	bufp += sprintf(bufp,
		"Firmware version:\t[%d.%d], build 0x%02X\n",
		mxt->device_info.major,
		mxt->device_info.minor,
		mxt->device_info.build
		);
	bufp += sprintf(bufp,
		"%d Sensor nodes:\t[X=%d, Y=%d]\n",
		mxt->device_info.num_nodes,
		mxt->device_info.x_size,
		mxt->device_info.y_size
		);
	bufp += sprintf(bufp,
		"Reported resolution:\t[X=%d, Y=%d]\n",
		mxt->pdata->max_x,
		mxt->pdata->max_y
		);
	return strlen(buf);
}

static ssize_t show_stat(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	struct i2c_client *client;
	struct mxt_data *mxt;
	char *bufp;

	client = to_i2c_client(dev);
	mxt = i2c_get_clientdata(client);

	bufp = buf;
	bufp += sprintf(bufp,
		"Interrupts:\t[VALID=%d ; INVALID=%d]\n",
		mxt->valid_irq_counter,
		mxt->invalid_irq_counter
		);
	bufp += sprintf(bufp, "Messages:\t[%d]\n", mxt->message_counter);
	bufp += sprintf(bufp, "Read Failures:\t[%d]\n", mxt->read_fail_counter);
	return strlen(buf);
}

static ssize_t show_object_info(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct i2c_client	*client;
	struct mxt_data		*mxt;
	char			*bufp;
	struct mxt_object	*object_table;
	int			i;

	client = to_i2c_client(dev);
	mxt = i2c_get_clientdata(client);
	object_table = mxt->object_table;

	bufp = buf;

	bufp += sprintf(bufp, "maXTouch: %d Objects\n",
		mxt->device_info.num_objs);

	for (i = 0; i < mxt->device_info.num_objs; i++) {
		if (object_table[i].type != 0) {
			bufp += sprintf(bufp,
				"Type:\t\t[%d]: %s\n",
				object_table[i].type,
				object_type_name[object_table[i].type]);
			bufp += sprintf(bufp,
				"Address:\t0x%04X\n",
				object_table[i].chip_addr);
			bufp += sprintf(bufp,
				"Size:\t\t%d Bytes\n",
				object_table[i].size);
			bufp += sprintf(bufp,
				"Instances:\t%d\n",
				object_table[i].instances
				);
			bufp += sprintf(bufp,
				"Report Id's:\t%d\n\n",
				object_table[i].num_report_ids);
		}
	}
	return strlen(buf);
}

static ssize_t show_messages(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct i2c_client *client;
	struct mxt_data   *mxt;
	struct mxt_object *object_table;
	int   i;
	__u8  *message;
	__u16 message_len;
	__u16 message_addr;

	char  *bufp;

	client = to_i2c_client(dev);
	mxt = i2c_get_clientdata(client);
	object_table = mxt->object_table;

	bufp = buf;

	message = kmalloc(mxt->message_size, GFP_KERNEL);
	if (message == NULL) {
		pr_warning("Error allocating memory!\n");
		return -ENOMEM;
	}

	message_addr = mxt->msg_proc_addr;
	message_len = mxt->message_size;
	bufp += sprintf(bufp,
		"Reading Message Window [0x%04x]\n",
		message_addr);

	/* Acquire the lock. */
	if (down_interruptible(&mxt->msg_sem)) {
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] mxt: Interrupted while waiting for mutex!\n");
		kfree(message);
		return -ERESTARTSYS;
	}

	while (mxt->new_msgs == 0) {
		/* Release the lock. */
		up(&mxt->msg_sem);
		if (wait_event_interruptible(mxt->msg_queue, mxt->new_msgs)) {
			if (debug >= DEBUG_INFO)
				pr_info(	"[TSP] mxt: Interrupted while waiting for new msg!\n");
			kfree(message);
			return -ERESTARTSYS;
		}

		/* Acquire the lock. */
		if (down_interruptible(&mxt->msg_sem)) {
			if (debug >= DEBUG_INFO)
				pr_info(	"[TSP] mxt: Interrupted while waiting for mutex!\n");
			kfree(message);
			return -ERESTARTSYS;
		}

	}

	for (i = 0; i < mxt->message_size; i++)
		message[i] = mxt->last_message[i];

	mxt->new_msgs = 0;

	/* Release the lock. */
	up(&mxt->msg_sem);

	for (i = 0; i < message_len; i++)
		bufp += sprintf(bufp, "0x%02x ", message[i]);
	bufp--;
	bufp += sprintf(bufp, "\t%s\n", REPORT_ID_TO_OBJECT_NAME(message[0]));

	kfree(message);
	return strlen(buf);
}


static ssize_t show_report_id(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	struct i2c_client    *client;
	struct mxt_data      *mxt;
	struct report_id_map *report_id;
	int                  i;
	int                  object;
	char                 *bufp;

	client    = to_i2c_client(dev);
	mxt       = i2c_get_clientdata(client);
	report_id = mxt->rid_map;

	bufp = buf;
	for (i = 0 ; i < mxt->report_id_count ; i++) {
		object = report_id[i].object;
		bufp += sprintf(bufp, "Report Id [%03d], object [%03d], "
			"instance [%03d]:\t%s\n",
			i,
			object,
			report_id[i].instance,
			object_type_name[object]);
	}
	return strlen(buf);
}

static ssize_t set_debug(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int state;

	sscanf(buf, "%d", &state);
	if (state == 0 || state == 1) {
		if (state) {
			debug = DEBUG_TRACE;
			pr_info("[TSP] touch info enabled");
		} else {
			debug = DEBUG_INFO;
			pr_info("[TSP] touch info disabled");
		}
	} else return -EINVAL;

	return count;
}

static ssize_t show_firmware(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct mxt_data *mxt = dev_get_drvdata(dev);
	u8 val[7];

	mxt_read_block(mxt->client, MXT_ADDR_INFO_BLOCK, 7, (u8 *)val);
	mxt->device_info.major = ((val[2] >> 4) & 0x0F);
	mxt->device_info.minor = (val[2] & 0x0F);
	mxt->device_info.build	= val[3];

	return snprintf(buf, PAGE_SIZE,
		"Atmel %s Firmware version [%d.%d] Build %d\n",
		mxt224_variant,
		mxt->device_info.major,
		mxt->device_info.minor,
		mxt->device_info.build);
}

static ssize_t store_firmware(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int state, ret;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	if (sscanf(buf, "%i", &state) != 1 || (state < 0 || state > 1))
		return -EINVAL;

	/* prevents the system from entering suspend during updating */
	wake_lock(&mxt->wakelock);
	disable_irq(mxt->client->irq);

	mxt_load_firmware(dev, MXT224E_FIRMWARE);
	mdelay(100);

	/* chip reset and re-initialize */
	mxt->pdata->suspend_platform_hw(mxt->pdata);
	mdelay(50);
	mxt->pdata->resume_platform_hw(mxt->pdata);

	ret = mxt_identify(mxt->client, mxt);
	if (ret >= 0) {
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] mxt_identify Sucess ");
	} else pr_err("[TSP] mxt_identify Fail ");

	ret = mxt_read_object_table(mxt->client, mxt);
	if (ret >= 0) {
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] mxt_read_object_table Sucess ");
	} else pr_err("[TSP] mxt_read_object_table Fail ");

	enable_irq(mxt->client->irq);
	wake_unlock(&mxt->wakelock);

	return count;
}


#ifdef MXT_FIRMUP_ENABLE
static int set_mxt_auto_update_exe(struct device *dev)
{
	int error;
	struct mxt_data *mxt = dev_get_drvdata(dev);
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] set_mxt_auto_update_exe \n");

	error = mxt_load_firmware(&mxt->client->dev, MXT224E_FIRMWARE);

	if (error >= 0) {
		mxt->firm_status_data = 2;	/*firmware update success */
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] Reprogram done : Firmware update Success~~~~~~~~~~\n");
		/* for stable-time */
		mdelay(100);
	} else {
		mxt->firm_status_data = 3;	/* firmware update Fail */
		pr_err("[TSP] Reprogram done : Firmware update Fail~~~~~~~~~~\n");
	}

	/* chip reset and re-initialize */
	mxt->pdata->suspend_platform_hw(mxt->pdata);
	mdelay(50);
	mxt->pdata->resume_platform_hw(mxt->pdata);

	error = mxt_identify(mxt->client, mxt);
	if (error >= 0) {
		if (debug >= DEBUG_INFO)
			pr_info("[TSP]  mxt_identify Sucess ");
	} else pr_err("[TSP]  mxt_identify Fail ");

	mxt_read_object_table(mxt->client, mxt);

	return error;
}
#endif

#ifdef MXT_FACTORY_TEST
static void set_mxt_update_exe(struct work_struct *work)
{
	struct	mxt_data *mxt;
	int ret, cnt;;
	mxt = container_of(work, struct mxt_data, firmup_dwork.work);
	/*client = mxt->client;*/
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] set_mxt_update_exe \n");


	/*wake_lock(&mxt->wakelock);*/  /* prevents the system from entering suspend during updating */
	disable_irq(mxt->client->irq);
	ret = mxt_load_firmware(&mxt->client->dev, MXT224E_FIRMWARE);
	mdelay(100);

	/* chip reset and re-initialize */
	mxt->pdata->suspend_platform_hw(mxt->pdata);
	mdelay(50);
	mxt->pdata->resume_platform_hw(mxt->pdata);

	ret = mxt_identify(mxt->client, mxt);
	if (ret >= 0) {
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] mxt_identify Sucess ");
	} else pr_err("[TSP] mxt_identify Fail ");


	ret = mxt_read_object_table(mxt->client, mxt);
	if (ret >= 0) {
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] mxt_read_object_table Sucess ");
	}else pr_err("[TSP] mxt_read_object_table Fail ");

	enable_irq(mxt->client->irq);
	/*wake_unlock(&mxt->wakelock);*/

	if (ret >= 0) {
		for (cnt = 10; cnt > 0; cnt--) {
			if (mxt->firm_normal_status_ack == 1) {
				mxt->firm_status_data = 2;	/* firmware update success */
				if (debug >= DEBUG_INFO)
					pr_info("[TSP] Reprogram done : Firmware update Success~~~~~~~~~~\n");
				break;
			} else {
				if (debug >= DEBUG_INFO)
					pr_info("[TSP] Reprogram done , but not yet normal status : 3s delay needed \n");
				msleep(500);/* 3s delay */
			}

		}
		if (cnt == 0) {
			mxt->firm_status_data = 3;	/* firmware update Fail */
			pr_err("[TSP] Reprogram done : Firmware update Fail ~~~~~~~~~~\n");
		}
	} else {
		mxt->firm_status_data = 3;	/* firmware update Fail */
		pr_err("[TSP] Reprogram done : Firmware update Fail~~~~~~~~~~\n");
	}
	mxt->firm_normal_status_ack = 0;
}

static ssize_t set_mxt_update_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	if (debug >= DEBUG_INFO)
		pr_info("[TSP] touch firmware update \n");
	mxt->firm_status_data = 1;	/* start firmware updating */
	cancel_delayed_work(&mxt->firmup_dwork);
	schedule_delayed_work(&mxt->firmup_dwork, 0);

	if (mxt->firm_status_data == 3) {
		count = sprintf(buf, "FAIL\n");
	} else
		count = sprintf(buf, "OK\n");
	return count;
}

/*Current(Panel) Version*/
static ssize_t set_mxt_firm_version_read_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct mxt_data *mxt = dev_get_drvdata(dev);
	int error, cnt;
	u8 val[7];
	u8 fw_current_version;

	for (cnt = 10; cnt > 0; cnt--) {
		error = mxt_read_block(mxt->client, MXT_ADDR_INFO_BLOCK, 7, (u8 *)val);
		if (error < 0) {
			pr_err("[TSP] Atmel touch version read fail it will try 2s later\n");
			msleep(2000);
		} else {
			break;
		}
	}
	if (cnt == 0) {
		pr_err("[TSP] set_mxt_firm_version_show failed!!!\n");
		fw_current_version = 0;
	}

	mxt->device_info.major = ((val[2] >> 4) & 0x0F);
	mxt->device_info.minor = (val[2] & 0x0F);
	mxt->device_info.build	= val[3];
	fw_current_version = val[2];
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] Atmel %s Firmware version [%d.%d](%d) Build %d\n",
			mxt224_variant,
			mxt->device_info.major,
			mxt->device_info.minor,
			fw_current_version,
			mxt->device_info.build);

	return sprintf(buf, "Ver %d.%d Build 0x%x\n", mxt->device_info.major, mxt->device_info.minor, mxt->device_info.build);
}

/* Last(Phone) Version */
extern u8 firmware_latest[];
static ssize_t set_mxt_firm_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u8 fw_latest_version;
	fw_latest_version = firmware_latest[0];
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] Atmel Last firmware version is 0x%02x\n", fw_latest_version);
	return sprintf(buf, "Ver %d.%d Build 0x%x\n", (firmware_latest[0]>>4)&0x0f, firmware_latest[0]&0x0f, firmware_latest[1]);
}
static ssize_t set_mxt_firm_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	int count;
	struct mxt_data *mxt = dev_get_drvdata(dev);
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] Enter firmware_status_show by Factory command \n");

	if (mxt->firm_status_data == 1) {
		count = sprintf(buf, "Downloading\n");
	} else if (mxt->firm_status_data == 2) {
		count = sprintf(buf, "PASS\n");
	} else if (mxt->firm_status_data == 3) {
		count = sprintf(buf, "FAIL\n");
	} else
		count = sprintf(buf, "PASS\n");

	return count;

}

static ssize_t key_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u8 val;
	struct mxt_data *mxt = dev_get_drvdata(dev);
	if (mxt->set_mode_for_ta) {
		mxt_read_byte(mxt->client,
			MXT_BASE_ADDR(MXT_PROCG_NOISESUPPRESSION_T48) + MXT_ADR_T48_TCHTHR,
			&val);
	} else {
	mxt_read_byte(mxt->client,
		MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) + MXT_ADR_T9_TCHTHR,
		&val);
	}
	return sprintf(buf, "%d\n", val);
}

static ssize_t key_threshold_store(struct device *dev, struct device_attribute *attr,
				   const char *buf, size_t size)
{
	struct mxt_data *mxt = dev_get_drvdata(dev);
	int i;
	if (sscanf(buf, "%d", &i) == 1) {
		wake_lock(&mxt->wakelock);  /* prevents the system from entering suspend during updating */
		disable_irq(mxt->client->irq);   /* disable interrupt */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) + MXT_ADR_T9_TCHTHR,
			i);
		/* backup to nv memory */
		backup_to_nv(mxt);
		/* forces a reset of the chipset */
		reset_chip(mxt, RESET_TO_NORMAL);
		msleep(250);  /* 250ms */

		enable_irq(mxt->client->irq);    /* enable interrupt */
		wake_unlock(&mxt->wakelock);
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] threshold is changed to %d\n", i);
	} else
		pr_err("[TSP] threshold write error\n");

	return size;
}
#endif

#if ENABLE_NOISE_TEST_MODE
uint8_t read_uint16_t(u16 Address, u16 *Data, struct mxt_data *mxt)
{
	uint8_t status;
	uint8_t temp[2];

	status = mxt_read_block(mxt->client, Address, 2, temp);
	*Data = ((uint16_t)temp[1] << 8) + (uint16_t)temp[0];

	return status;
}
int  read_dbg_data(u8 dbg_mode , u8 node, u16 *dbg_data, struct mxt_data *mxt)
{
	int  status;
	u8 mode, page, i;
	u8 read_page;
	u16 read_point;
	u16	diagnostics;
	u16 diagnostic_addr;

	diagnostic_addr = MXT_BASE_ADDR(MXT_DEBUG_DIAGNOSTICS_T37);
	diagnostics =  T6_REG(MXT_ADR_T6_DIAGNOSTICS);

	read_page = node / 64;
	node %= 64;
	read_point = (node * 2) + 2;

	/* Page Num Clear */
	mxt_write_byte(mxt->client, diagnostics, MXT_CMD_T6_CTE_MODE);
	msleep(20);
	mxt_write_byte(mxt->client, diagnostics, dbg_mode);
	msleep(20);

	for (i = 0; i < 5; i++) {
		msleep(20);
		status = mxt_read_byte(mxt->client, diagnostic_addr, &mode);
		if (status == 0) {
			if (mode == dbg_mode) {
				break;
			}
		} else {
			pr_err("[TSP] read mode fail \n");
			return status;
		}
	}



	for (page = 0; page < read_page; page++) {
		mxt_write_byte(mxt->client, diagnostics, MXT_CMD_T6_PAGE_UP);
		msleep(10);
	}

	status = read_uint16_t(diagnostic_addr + read_point, dbg_data, mxt);

	msleep(10);

	return status;
}
static ssize_t set_refer0_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_refrence = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_REFERENCES_MODE, test_node[0], &qt_refrence, mxt);
	return sprintf(buf, "%u\n", qt_refrence);
}

static ssize_t set_refer1_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_refrence = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_REFERENCES_MODE, test_node[1], &qt_refrence, mxt);
	return sprintf(buf, "%u\n", qt_refrence);
}

static ssize_t set_refer2_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_refrence = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_REFERENCES_MODE, test_node[2], &qt_refrence, mxt);
	return sprintf(buf, "%u\n", qt_refrence);
}


static ssize_t set_refer3_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_refrence = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_REFERENCES_MODE, test_node[3], &qt_refrence, mxt);
	return sprintf(buf, "%u\n", qt_refrence);
}


static ssize_t set_refer4_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_refrence = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_REFERENCES_MODE, test_node[4], &qt_refrence, mxt);
	return sprintf(buf, "%u\n", qt_refrence);
}

static ssize_t set_delta0_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_delta = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_DELTAS_MODE, test_node[0], &qt_delta, mxt);
	if (qt_delta < 32767) {
		return sprintf(buf, "%u\n", qt_delta);
	} else	{
		qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);
	}
}

static ssize_t set_delta1_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_delta = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_DELTAS_MODE, test_node[1], &qt_delta, mxt);
	if (qt_delta < 32767) {
		return sprintf(buf, "%u\n", qt_delta);
	} else	{
		qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);
	}
}

static ssize_t set_delta2_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_delta = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_DELTAS_MODE, test_node[2], &qt_delta, mxt);
	if (qt_delta < 32767) {
		return sprintf(buf, "%u\n", qt_delta);
	} else {
		qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);
	}
}

static ssize_t set_delta3_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_delta = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_DELTAS_MODE, test_node[3], &qt_delta, mxt);
	if (qt_delta < 32767) {
		return sprintf(buf, "%u\n", qt_delta);
	} else {
		qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);
	}
}

static ssize_t set_delta4_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int  status;
	u16 qt_delta = 0;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	status = read_dbg_data(MXT_CMD_T6_DELTAS_MODE, test_node[4], &qt_delta, mxt);
	if (qt_delta < 32767) {
		return sprintf(buf, "%u\n", qt_delta);
	} else {
		qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);
	}
}

static ssize_t set_threshold_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u8 val;
	struct mxt_data *mxt = dev_get_drvdata(dev);

	mxt_read_byte(mxt->client,
		MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) + MXT_ADR_T9_TCHTHR,
		&val);
	return sprintf(buf, "%d\n", val);
}

#endif


static int chk_obj(u8 type)
{
	switch (type) {
		/*	case	MXT_GEN_MESSAGEPROCESSOR_T5:*/
		/*	case	MXT_GEN_COMMANDPROCESSOR_T6:*/
	case	MXT_GEN_POWERCONFIG_T7:
	case	MXT_GEN_ACQUIRECONFIG_T8:
	case	MXT_TOUCH_MULTITOUCHSCREEN_T9:
	case	MXT_TOUCH_KEYARRAY_T15:
		/*	case	MXT_SPT_COMMSCONFIG_T18:*/
		/*	case	MXT_PROCG_NOISESUPPRESSION_T22:*/
		/*	case	MXT_PROCI_ONETOUCHGESTUREPROCESSOR_T24:*/
		/*	case	MXT_SPT_SELFTEST_T25:*/
		/*	case	MXT_PROCI_TWOTOUCHGESTUREPROCESSOR_T27:*/
		/*	case	MXT_SPT_CTECONFIG_T28:*/
		/*	case	MXT_DEBUG_DIAGNOSTICS_T37:*/
	case	MXT_USER_INFO_T38:
		/*	case	MXT_GEN_EXTENSION_T39:*/
	case	MXT_PROCI_GRIPSUPPRESSION_T40:
	case	MXT_PROCI_TOUCHSUPPRESSION_T42:
	case	MXT_SPT_CTECONFIG_T46:
	case	MXT_PROCI_STYLUS_T47:
	case	MXT_PROCG_NOISESUPPRESSION_T48:
		/*	case	MXT_SPT_DIGITIZER_T43: */
		/*	case	MXT_MESSAGECOUNT_T44:*/
		return 0;
	default:
		return -1;
	}
}

static ssize_t show_message(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
#ifdef TSP_DEBUG_MESSAGE
	struct mxt_data *mxt;
	char *bufp;
	unsigned short msg_tail_cnt;
	mxt = dev_get_drvdata(dev);

	bufp = buf;
	bufp += sprintf(bufp,
		"Show recently touch message, msg_log.cnt = %d\n", msg_log.cnt
		);
	msg_tail_cnt = msg_log.cnt + 1;
	msg_tail_cnt &= (MAX_MSG_LOG_SIZE - 1);

	do {
		bufp += sprintf(bufp, "%d,\t",msg_log.id[msg_tail_cnt]);
		bufp += sprintf(bufp, "%x,\t",msg_log.status[msg_tail_cnt]);
		bufp += sprintf(bufp, "%d,\t",msg_log.xpos[msg_tail_cnt]);
		bufp += sprintf(bufp, "%d,\t",msg_log.ypos[msg_tail_cnt]);
		bufp += sprintf(bufp, "%d,\t",msg_log.area[msg_tail_cnt]);
		bufp += sprintf(bufp, "%d\n",msg_log.amp[msg_tail_cnt]);

		msg_tail_cnt++;
	}while(msg_tail_cnt != msg_log.cnt);

	for(msg_log.cnt = 0; msg_log.cnt < 128; msg_log.cnt++){
		msg_log.id[msg_log.cnt] = 0;
		msg_log.status[msg_log.cnt] = 0;
		msg_log.xpos[msg_log.cnt] = 0;
		msg_log.ypos[msg_log.cnt] = 0;
		msg_log.area[msg_log.cnt] = 0;
		msg_log.amp[msg_log.cnt] = 0;
	}
	msg_log.cnt = 0;
#endif
	return strlen(buf);

}


static ssize_t show_object(struct device *dev, struct device_attribute *attr, char *buf)
{
	/*	struct qt602240_data *data = dev_get_drvdata(dev); */
	/*	struct qt602240_object *object; */
	struct mxt_data *mxt;
	struct mxt_object	 *object_table;

	int count = 0;
	int i, j;
	u8 val;

	mxt = dev_get_drvdata(dev);
	object_table = mxt->object_table;

	for (i = 0; i < mxt->device_info.num_objs; i++) {
		u8 obj_type = object_table[i].type;

		if (chk_obj(obj_type))
			continue;

		count += sprintf(buf + count, "%s: %d bytes\n",
			object_type_name[obj_type], object_table[i].size);

		for (j = 0; j < object_table[i].size; j++) {
			mxt_read_byte(mxt->client, MXT_BASE_ADDR(obj_type)+(u16)j, &val);
			count += sprintf(buf + count,
				"  Byte %2d: 0x%02x (%d)\n", j, val, val);
		}

		count += sprintf(buf + count, "\n");
	}

	/* debug only */
	/*
	count += sprintf(buf + count, "%s: %d bytes\n", "debug_config_T0", 32);

	for (j = 0; j < 32; j++) {
		count += sprintf(buf + count,
			"  Byte %2d: 0x%02x (%d)\n", j, mxt->debug_config[j], mxt->debug_config[j]);
	}
	* */


#ifdef MXT_TUNNING_ENABLE
	backup_to_nv(mxt);
#endif

	return count;
}

static ssize_t store_object(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/*	struct qt602240_data *data = dev_get_drvdata(dev); */
	/*	struct qt602240_object *object; */
	struct mxt_data *mxt;
	/*	struct mxt_object	*object_table;//TO_CHK: not used now */

	unsigned int type, offset;
	u8 val;
	u16	chip_addr;
	int ret;

	mxt = dev_get_drvdata(dev);

	if ((sscanf(buf, "%u %u %u", &type, &offset, &val) != 3) || (type >= MXT_MAX_OBJECT_TYPES)) {
		pr_err("Invalid values");
		return -EINVAL;
	}

	if (debug >= DEBUG_INFO)
		pr_info("[TSP] Object type: %u, Offset: %u, Value: %u\n", type, offset, val);


	/* debug only */
	/*
	count += sprintf(buf + count, "%s: %d bytes\n", "debug_config_T0", 32);
	*/


	if(type == 0)
	{
		/*
		mxt->debug_config[offset] = (u8)val;
		*/
	} else {

	chip_addr = get_object_address(type, 0, mxt->object_table,
		mxt->device_info.num_objs);
	if (chip_addr == 0) {
		pr_err("[TSP] Invalid object type(%d)!", type);
		return -EIO;
	}

	ret = mxt_write_byte(mxt->client, chip_addr+(u16)offset, (u8)val);
	pr_err("[TSP] store_object result: (%d)\n", ret);
	if (ret < 0) {
		return ret;
	}
	mxt_read_byte(mxt->client,
		MXT_BASE_ADDR(MXT_USER_INFO_T38)+
		MXT_ADR_T38_CFG_CTRL,
		&val);

	if (val == MXT_CFG_DEBUG) {
		backup_to_nv(mxt);
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] backup_to_nv\n");
	}

	}

	return count;
}

#if 0  /* FOR_TEST */
static ssize_t test_suspend(struct device *dev, struct device_attribute *attr, char *buf)
{
	char *bufp;
	struct early_suspend  *fake;

	bufp = buf;
	bufp += sprintf(bufp, "Running early_suspend function...\n");

	fake = kzalloc(sizeof(struct early_suspend), GFP_KERNEL);
	mxt_early_suspend(fake);
	kfree(fake);

	return strlen(buf);
}

static ssize_t test_resume(struct device *dev, struct device_attribute *attr, char *buf)
{
	char *bufp;
	struct early_suspend  *fake;

	bufp = buf;
	bufp += sprintf(bufp, "Running late_resume function...\n");

	fake = kzalloc(sizeof(struct early_suspend), GFP_KERNEL);
	mxt_late_resume(fake);
	kfree(fake);

	return strlen(buf);
}
#endif

#ifdef KEY_LED_CONTROL
static void key_led_on(struct mxt_data *mxt, u32 val)
{
	if (mxt->pdata->key_led_en1 != NULL)
		gpio_direction_output(mxt->pdata->key_led_en1, (val & 0x01) ? true : false);
	if (mxt->pdata->key_led_en2 != NULL)
		gpio_direction_output(mxt->pdata->key_led_en2, (val & 0x02) ? true : false);
	if (mxt->pdata->key_led_en3 != NULL)
		gpio_direction_output(mxt->pdata->key_led_en3, (val & 0x04) ? true : false);
	if (mxt->pdata->key_led_en4 != NULL)
		gpio_direction_output(mxt->pdata->key_led_en4, (val & 0x08) ? true : false);
}

static ssize_t key_led_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t size)
{
	struct mxt_data *mxt = dev_get_drvdata(dev);
	u32 i = 0;
	if (sscanf(buf, "%d", &i) != 1) {
		pr_err("[TSP] keyled write error\n");
	}
	if (mxt->mxt_status)
		key_led_on(mxt, i);

	if (debug >= DEBUG_INFO)
		pr_info("[TSP] Called value by HAL = %d\n", i);
	if (i) key_led_status = true;
	else key_led_status = false;

	return size;
}

static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR, NULL, key_led_store);
#endif


/* Register sysfs files */

static DEVICE_ATTR(deltas,      S_IRUGO, show_deltas,      NULL);
static DEVICE_ATTR(references,  S_IRUGO, show_references,  NULL);
static DEVICE_ATTR(device_info, S_IRUGO, show_device_info, NULL);
static DEVICE_ATTR(object_info, S_IRUGO, show_object_info, NULL);
static DEVICE_ATTR(messages,    S_IRUGO, show_messages,    NULL);
static DEVICE_ATTR(report_id,   S_IRUGO, show_report_id,   NULL);
static DEVICE_ATTR(stat,        S_IRUGO, show_stat,        NULL);
static DEVICE_ATTR(config,      S_IWUSR|S_IRUGO, get_config, set_config);
static DEVICE_ATTR(ap,          S_IWUSR, NULL,             set_ap);
static DEVICE_ATTR(debug, S_IWUSR, NULL, set_debug);
static DEVICE_ATTR(firmware, S_IWUSR|S_IRUGO, show_firmware, store_firmware);
static DEVICE_ATTR(object, S_IWUSR|S_IRUGO, show_object, store_object);
static DEVICE_ATTR(message,     S_IRUGO, show_message,      NULL);

/* static DEVICE_ATTR(suspend, S_IRUGO, test_suspend, NULL); */
/* static DEVICE_ATTR(resume, S_IRUGO, test_resume, NULL);  */
#ifdef MXT_FACTORY_TEST
static DEVICE_ATTR(tsp_firm_update, S_IRUGO, set_mxt_update_show, NULL);		/* firmware update */
static DEVICE_ATTR(tsp_firm_update_status, S_IRUGO, set_mxt_firm_status_show, NULL);	/* firmware update status return */
static DEVICE_ATTR(tsp_threshold, S_IRUGO | S_IWUSR, key_threshold_show, key_threshold_store);	/* touch threshold return, store */
static DEVICE_ATTR(tsp_firm_version_phone, S_IRUGO, set_mxt_firm_version_show, NULL);	/* firmware version resturn in phone driver version */
static DEVICE_ATTR(tsp_firm_version_panel, S_IRUGO, set_mxt_firm_version_read_show, NULL);		/* firmware version resturn in TSP panel version */
#endif
#if ENABLE_NOISE_TEST_MODE
static DEVICE_ATTR(set_refer0, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_refer0_mode_show, NULL);
static DEVICE_ATTR(set_delta0, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_delta0_mode_show, NULL);
static DEVICE_ATTR(set_refer1, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_refer1_mode_show, NULL);
static DEVICE_ATTR(set_delta1, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_delta1_mode_show, NULL);
static DEVICE_ATTR(set_refer2, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_refer2_mode_show, NULL);
static DEVICE_ATTR(set_delta2, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_delta2_mode_show, NULL);
static DEVICE_ATTR(set_refer3, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_refer3_mode_show, NULL);
static DEVICE_ATTR(set_delta3, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_delta3_mode_show, NULL);
static DEVICE_ATTR(set_refer4, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_refer4_mode_show, NULL);
static DEVICE_ATTR(set_delta4, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_delta4_mode_show, NULL);
static DEVICE_ATTR(set_threshould, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, set_threshold_mode_show, NULL);
#endif


static struct attribute *maxTouch_attributes[] = {
	&dev_attr_deltas.attr,
		&dev_attr_references.attr,
		&dev_attr_device_info.attr,
		&dev_attr_object_info.attr,
		&dev_attr_messages.attr,
		&dev_attr_report_id.attr,
		&dev_attr_stat.attr,
		&dev_attr_config.attr,
		&dev_attr_ap.attr,
		&dev_attr_debug.attr,
		&dev_attr_firmware.attr,
		&dev_attr_object.attr,
		&dev_attr_message.attr,
		/*	&dev_attr_suspend.attr, */
		/*	&dev_attr_resume.attr, */
		NULL,
};

static struct attribute_group maxtouch_attr_group = {
	.attrs = maxTouch_attributes,
};

static struct attribute *maxTouch_facotry_attributes[] = {
#ifdef MXT_FACTORY_TEST
	&dev_attr_tsp_firm_update.attr,
		&dev_attr_tsp_firm_update_status.attr,
		&dev_attr_tsp_threshold.attr,
		&dev_attr_tsp_firm_version_phone.attr,
		&dev_attr_tsp_firm_version_panel.attr,
#endif

#if ENABLE_NOISE_TEST_MODE
		&dev_attr_set_refer0.attr,
		&dev_attr_set_delta0.attr,
		&dev_attr_set_refer1.attr,
		&dev_attr_set_delta1.attr,
		&dev_attr_set_refer2.attr,
		&dev_attr_set_delta2.attr,
		&dev_attr_set_refer3.attr,
		&dev_attr_set_delta3.attr,
		&dev_attr_set_refer4.attr,
		&dev_attr_set_delta4.attr,
		&dev_attr_set_threshould.attr,
#endif

		NULL,
};

static struct attribute_group maxtouch_factory_attr_group = {
	.attrs = maxTouch_facotry_attributes,
};

/* This function sends a calibrate command to the maXTouch chip.
* While calibration has not been confirmed as good, this function sets
* the ATCHCALST and ATCHCALSTHR to zero to allow a bad cal to always recover
* Returns WRITE_MEM_OK if writing the command to touch chip was successful.
*/
unsigned char not_yet_count = 0;
extern gen_acquisitionconfig_t8_config_t			acquisition_config;
extern int mxt_acquisition_config(struct mxt_data *mxt);


static int calibrate_chip(struct mxt_data *mxt)
{
	u8 data = 1u;
	int ret ;
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] %s\n", __func__);

	caling_check = 1;
	facesup_message_flag  = 0;
	not_yet_count = 0;
	mxt_time_point = 0;

	/* change calibration suspend settings to zero until calibration confirmed good */
	ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHCALST, 0);
	ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHCALSTHR, 0);
	ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALTHR, 0);
	ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALRATIO, 0);

	ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) + MXT_ADR_T9_NUMTOUCH, T9_NUMTOUCH);

	/* TSP, Touchscreen threshold */
	//ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) + MXT_ADR_T9_TCHTHR, 50);

	/* TSP, TouchKEY threshold */
	ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_TOUCH_KEYARRAY_T15) + MXT_ADR_T15_TCHTHR, T15_TCHTHR);

	klogi_if("[TSP] reset acq atchcalst=%d, atchcalsthr=%d\n", 0, 0 );
	/* restore settings to the local structure so that when we confirm the */
	/* cal is good we can correct them in the chip */
	/* this must be done before returning */
	/* send calibration command to the chip */
	/* change calibration suspend settings to zero until calibration confirmed good */
	ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_CALIBRATE, data);
	if ( ret < 0 ) {
		pr_err("[TSP][ERROR] line : %d\n", __LINE__);
		caling_check = 0;
		return -1;
	} else {
		/* set flag for calibration lockup recovery if cal command was successful */
		/* set flag to show we must still confirm if calibration was good or bad */
		cal_check_flag = 1u;
	}

	caling_check = 0;
	//msleep(120);
	msleep(60);

	return ret;
}

static void check_chip_palm(struct mxt_data *mxt)
{
	uint8_t data_buffer[100] = { 0 };
	uint8_t try_ctr = 0;
	uint8_t data_byte = 0xF3; /* dianostic command to get touch flags */
	uint16_t diag_address;
	uint8_t tch_ch = 0, atch_ch = 0;
	uint8_t check_mask;
	uint8_t i;
	uint8_t j;
	uint8_t x_line_limit;
	uint8_t max_touch_ch;
	struct i2c_client *client;

	client = mxt->client;
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] %s\n", __func__);

	/* we have had the first touchscreen or face suppression message
	 * after a calibration - check the sensor state and try to confirm if
	 * cal was good or bad */

	/* get touch flags from the chip using the diagnostic object */
	/* write command to command processor to get touch flags - 0xF3 Command required to do this */
	mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_DIAGNOSTICS, 0xf3);


	/* get the address of the diagnostic object so we can get the data we need */
	diag_address = MXT_BASE_ADDR(MXT_DEBUG_DIAGNOSTICS_T37);

	/* SAP_Sleep(5); */
	msleep(5);


	/* read touch flags from the diagnostic object - clear buffer so the while loop can run first time */
	memset( data_buffer , 0xFF, sizeof( data_buffer ) );

	/* wait for diagnostic object to update */
	while(!((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00))) {
		/* wait for data to be valid  */
		if(try_ctr > 50) {
			/* Failed! */
			pr_err("[TSP] Diagnostic Data did not update!!\n");
			break;
		}
		msleep(2);
		try_ctr++; /* timeout counter */
		mxt_read_block(client, diag_address, 2, data_buffer);

		klogi_if("[TSP] Waiting for diagnostic data to update, try %d\n", try_ctr);
	}

	if(try_ctr > 50){
		pr_err("[TSP] %s, Diagnostic Data did not update over 50, force reset!! %d\n", __func__, try_ctr);
		/* forces a reset of the chipset */
		mxt_release_all_fingers(mxt);
		mxt_release_all_keys(mxt);
		reset_chip(mxt, RESET_TO_NORMAL);
		msleep(150); /*need 250ms*/
		return;
	}

	/* data is ready - read the detection flags */
#if 1 /* 20120517 */
	mxt_read_block(client, diag_address, (22*2*2 + 2), data_buffer);
#else
	mxt_read_block(client, diag_address, 82, data_buffer);
#endif
	/* data array is 20 x 16 bits for each set of flags, 2 byte header, 40 bytes for touch flags 40 bytes for antitouch flags*/

	/* count up the channels/bits if we recived the data properly */
	if((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00)) {

		/* mode 0 : 16 x line, mode 1 : 17 etc etc upto mode 4.*/
		//x_line_limit = 16 + cte_config.mode;
		x_line_limit = 16 + T46_MODE;
#if 1 /* 20120517 */
		if (x_line_limit > 22)
			x_line_limit = 22;
#else
		if(x_line_limit > 20) {
			/* hard limit at 20 so we don't over-index the array */
			x_line_limit = 20;
		}
#endif
		/* double the limit as the array is in bytes not words */
		x_line_limit = x_line_limit << 1;

		/* count the channels and print the flags to the log */
		for(i = 0; i < x_line_limit; i+=2) {  /*check X lines - data is in words so increment 2 at a time */

			/* print the flags to the log - only really needed for debugging */
			//printk("[TSP] Detect Flags X%d, %x%x, %x%x \n", i>>1,data_buffer[3+i],data_buffer[2+i],data_buffer[43+i],data_buffer[42+i]);

			/* count how many bits set for this row */
			for(j = 0; j < 8; j++) {
				/* create a bit mask to check against */
				check_mask = 1 << j;

				/* check detect flags */
				if(data_buffer[2+i] & check_mask) {
					tch_ch++;
				}
				if(data_buffer[3+i] & check_mask) {
					tch_ch++;
				}

				/* check anti-detect flags */
#if 1
				if (data_buffer[46+i] & check_mask)
					atch_ch++;

				if (data_buffer[47+i] & check_mask)
					atch_ch++;
#else
				if(data_buffer[42+i] & check_mask) {
					atch_ch++;
				}
				if(data_buffer[43+i] & check_mask) {
					atch_ch++;
				}
#endif
			}
		}


		/* print how many channels we counted */
		klogi_if("[TSP] Flags Counted channels: t:%d a:%d \n", tch_ch, atch_ch);

		/* send page up command so we can detect when data updates next time,
		 * page byte will sit at 1 until we next send F3 command */
		data_byte = 0x01;
		/* write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte); */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_DIAGNOSTICS, data_byte);


		/* process counters and decide if we must re-calibrate or if cal was good */
		mxt_read_byte(mxt->client,  MXT_BASE_ADDR(MXT_USER_INFO_T38)+ MXT_ADR_T38_USER1, &max_touch_ch);


		if ((tch_ch > 0 ) && ( atch_ch  > 0)) {
			facesup_message_flag = 1;
		} else if ((tch_ch > 0 ) && ( atch_ch  == 0)) {
			facesup_message_flag = 2;
			if (tch_ch < 20)
				facesup_message_flag = 5;
		} else if ((tch_ch == 0 ) && ( atch_ch > 0)) {
			facesup_message_flag = 3;
		}else {
			facesup_message_flag = 4;
		}

		if ((tch_ch < 70) || ((tch_ch >= 70) && ((tch_ch - atch_ch) > 25))) palm_check_timer_flag = true;

		klogi_if("[TSP] Touch suppression State: %d \n", facesup_message_flag);
	}
}

static void check_chip_channel(struct mxt_data *mxt)
{
	uint8_t data_buffer[100] = { 0 };
	uint8_t try_ctr = 0;
	uint8_t data_byte = 0xF3; /* dianostic command to get touch flags */
	uint16_t diag_address;
	uint8_t tch_ch = 0, atch_ch = 0;
	uint8_t check_mask;
	uint8_t i;
	uint8_t j;
	uint8_t x_line_limit;
	uint8_t CAL_THR;
	uint8_t num_of_antitouch;
	struct i2c_client *client;

	client = mxt->client;
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] %s\n", __func__);

	/* we have had the first touchscreen or face suppression message
	 * after a calibration - check the sensor state and try to confirm if
	 * cal was good or bad */

	/* get touch flags from the chip using the diagnostic object */
	/* write command to command processor to get touch flags - 0xF3 Command required to do this */
	mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_DIAGNOSTICS, 0xf3);


	/* get the address of the diagnostic object so we can get the data we need */
	diag_address = MXT_BASE_ADDR(MXT_DEBUG_DIAGNOSTICS_T37);

	/* SAP_Sleep(5); */
	msleep(5);


	/* read touch flags from the diagnostic object - clear buffer so the while loop can run first time */
	memset( data_buffer , 0xFF, sizeof( data_buffer ) );

	/* wait for diagnostic object to update */
	while(!((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00))) {
		/* wait for data to be valid  */
		if(try_ctr > 50) {
			/* Failed! */
			pr_err("[TSP] Diagnostic Data did not update!!\n");
				break;
		}
		msleep(2);
		try_ctr++; /* timeout counter */
		/* read_mem(diag_address, 2,data_buffer); */
		mxt_read_block(client, diag_address, 2, data_buffer);

		klogi_if("[TSP] Waiting for diagnostic data to update, try %d\n", try_ctr);
	}

	if(try_ctr > 50){
		pr_err("[TSP] %s, Diagnostic Data did not update over 50, force reset!! %d\n", __func__, try_ctr);
		/* forces a reset of the chipset */
		mxt_release_all_fingers(mxt);
		mxt_release_all_keys(mxt);
		reset_chip(mxt, RESET_TO_NORMAL);
		msleep(150); /*need 250ms*/
		return;
	}

	/* data is ready - read the detection flags */
	/* read_mem(diag_address, 82,data_buffer); */
#if 1 /* 20120517 */
	mxt_read_block(client, diag_address, (22*2*2 + 2), data_buffer);
#else
	mxt_read_block(client, diag_address, 82, data_buffer);
#endif
	/* data array is 20 x 16 bits for each set of flags, 2 byte header, 40 bytes for touch flags 40 bytes for antitouch flags*/

	/* count up the channels/bits if we recived the data properly */
	if((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00)) {

		/* mode 0 : 16 x line, mode 1 : 17 etc etc upto mode 4.*/
		x_line_limit = 16 + T46_MODE;
#if 1 /* 20120517 */
		if (x_line_limit > 22)
			x_line_limit = 22;
#else
		if(x_line_limit > 20) {
			/* hard limit at 20 so we don't over-index the array */
			x_line_limit = 20;
		}
#endif
		/* double the limit as the array is in bytes not words */
		x_line_limit = x_line_limit << 1;

		/* count the channels and print the flags to the log */
		for(i = 0; i < x_line_limit; i+=2) {  /*check X lines - data is in words so increment 2 at a time */
			/* count how many bits set for this row */
			for(j = 0; j < 8; j++) {
				/* create a bit mask to check against */
				check_mask = 1 << j;

				/* check detect flags */
				if(data_buffer[2+i] & check_mask) {
					tch_ch++;
				}
				if(data_buffer[3+i] & check_mask) {
					tch_ch++;
				}

				/* check anti-detect flags */
#if 1 /* 20120517 */
				if (data_buffer[46+i] & check_mask)
					atch_ch++;

				if (data_buffer[47+i] & check_mask)
					atch_ch++;
#else
				if(data_buffer[42+i] & check_mask) {
					atch_ch++;
				}
				if(data_buffer[43+i] & check_mask) {
					atch_ch++;
				}
#endif
			}
		}


		/* print how many channels we counted */
		klogi_if("[TSP] Flags Counted channels: t:%d a:%d \n", tch_ch, atch_ch);

		chk_touch_cnt = tch_ch;
		chk_antitouch_cnt = atch_ch;
		/* send page up command so we can detect when data updates next time,
		 * page byte will sit at 1 until we next send F3 command */
		data_byte = 0x01;
		/* write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte); */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_DIAGNOSTICS, data_byte);
	}
	return;
}

static void check_chip_calibration(struct mxt_data *mxt)
{
	uint8_t data_buffer[100] = { 0 };
	uint8_t try_ctr = 0;
	uint8_t data_byte = 0xF3; /* dianostic command to get touch flags */
	uint16_t diag_address;
	uint8_t tch_ch = 0, atch_ch = 0;
	uint8_t check_mask;
	uint8_t i;
	uint8_t j;
	uint8_t x_line_limit;
	uint8_t CAL_THR;
	uint8_t num_of_antitouch;
	struct i2c_client *client;
	uint8_t finger_cnt = 0;

	client = mxt->client;
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] %s\n", __func__);

	mxt_read_byte(mxt->client,  MXT_BASE_ADDR(MXT_USER_INFO_T38)+ MXT_ADR_T38_USER1, &CAL_THR);
	mxt_read_byte(mxt->client,  MXT_BASE_ADDR(MXT_USER_INFO_T38)+ MXT_ADR_T38_USER2, &num_of_antitouch);

	/* we have had the first touchscreen or face suppression message
	 * after a calibration - check the sensor state and try to confirm if
	 * cal was good or bad */

	/* get touch flags from the chip using the diagnostic object */
	/* write command to command processor to get touch flags - 0xF3 Command required to do this */
	mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_DIAGNOSTICS, 0xf3);


	/* get the address of the diagnostic object so we can get the data we need */
	diag_address = MXT_BASE_ADDR(MXT_DEBUG_DIAGNOSTICS_T37);

	/* SAP_Sleep(5); */
	msleep(10);


	/* read touch flags from the diagnostic object - clear buffer so the while loop can run first time */
	memset( data_buffer , 0xFF, sizeof( data_buffer ) );


	/* read_mem(diag_address, 2,data_buffer); */
	mxt_read_block(client, diag_address, 3, data_buffer);


	/* wait for diagnostic object to update */
	while(!((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00))) {

		if(data_buffer[0] == 0xF3) {
			if( data_buffer[1] == 0x01) {
				/* Page down */
				data_byte = 0x02;
				mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_DIAGNOSTICS, data_byte);
				/* SAP_Sleep(5); */
				msleep(10);

			}
		}
		else {
			msleep(10);
		}
		/* wait for data to be valid  */
		if(try_ctr > 3) {
			/* Failed! */
			pr_err("[TSP] Diagnostic Data did not update!!\n");
				break;
		}

		try_ctr++; /* timeout counter */

		/* read_mem(diag_address, 2,data_buffer); */
		mxt_read_block(client, diag_address, 3, data_buffer);

		klogi_if("[TSP] Waiting for diagnostic data to update, try %d\n", try_ctr);
	}

	if(try_ctr > 3){
		//pr_err("[TSP] %s, Diagnostic Data did not update over 3, force reset!! %d\n", __func__, try_ctr);
		pr_err("[TSP] %s, Diagnostic Data did not update over 3 !! %d\n", __func__, try_ctr);
#if 0
		/* forces a reset of the chipset */
		mxt_release_all_fingers(mxt);
		mxt_release_all_keys(mxt);
		reset_chip(mxt, RESET_TO_NORMAL);
		msleep(150); /*need 250ms*/
#endif
		/* send page up command so we can detect when data updates next time,
		 * page byte will sit at 1 until we next send F3 command */
		data_byte = 0x01;
		/* write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte); */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_DIAGNOSTICS, data_byte);

		return;
	}

	/* data is ready - read the detection flags */
	/* read_mem(diag_address, 82,data_buffer); */

#if 1	/* 20120516 add */
	mxt_read_block(client, diag_address, (22*2*2 + 2), data_buffer);
#else
	mxt_read_block(client, diag_address, 82, data_buffer);
#endif

	/* data array is 20 x 16 bits for each set of flags, 2 byte header, 40 bytes for touch flags 40 bytes for antitouch flags*/

	/* count up the channels/bits if we recived the data properly */
	if((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00)) {

		/* mode 0 : 16 x line, mode 1 : 17 etc etc upto mode 4.*/
		x_line_limit = 16 + T46_MODE;


#if 1 /* 20120516 add */
		if (x_line_limit > 22) {
			/* hard limit at 20 so we don't over-index the array */
			x_line_limit = 22;
		}
#else
		if(x_line_limit > 20) {
			/* hard limit at 20 so we don't over-index the array */
			x_line_limit = 20;
		}
#endif

		/* double the limit as the array is in bytes not words */
		x_line_limit = x_line_limit << 1;

		/* count the channels and print the flags to the log */
		for(i = 0; i < x_line_limit; i+=2) {  /*check X lines - data is in words so increment 2 at a time */
			/* count how many bits set for this row */
			for(j = 0; j < 8; j++) {
				/* create a bit mask to check against */
				check_mask = 1 << j;

				/* check detect flags */
				if(data_buffer[2+i] & check_mask) {
					tch_ch++;
				}
				if(data_buffer[3+i] & check_mask) {
					tch_ch++;
				}

#if 1 /* 20120516 add */
				if (data_buffer[46+i] & check_mask)
					atch_ch++;

				if (data_buffer[47+i] & check_mask)
					atch_ch++;
#else
				if (data_buffer[42+i] & check_mask)
					atch_ch++;

				if (data_buffer[43+i] & check_mask)
					atch_ch++;
#endif
			}
		}


		/* print how many channels we counted */
		klogi_if("[TSP] Flags Counted channels: t:%d a:%d \n", tch_ch, atch_ch);

		/* send page up command so we can detect when data updates next time,
		 * page byte will sit at 1 until we next send F3 command */
		data_byte = 0x01;
		/* write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte); */
		mxt_write_byte(mxt->client,
			MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + MXT_ADR_T6_DIAGNOSTICS, data_byte);

		for (i = 0 ; i < MXT_MAX_NUM_TOUCHES ; ++i) {
			if ( mtouch_info[i].pressure == -1 )
				continue;
			finger_cnt++;
		}

		if (cal_check_flag != 1) {
			klogi_if("[TSP] check_chip_calibration just return!! finger_cnt = %d\n", finger_cnt);
			return;
		}

		/* process counters and decide if we must re-calibrate or if cal was good */
		if ((tch_ch > 30) || (atch_ch > 20) || ((tch_ch + atch_ch) > 30)) {
			klogi_if("[TSP] maybe palm, re-calibrate!! \n");
			calibrate_chip(mxt);
			/* 100ms timer disable */
			timer_flag = DISABLE;
			timer_ticks = 0;
			ts_100ms_timer_stop(mxt);
		} else if((tch_ch <= 30) && (atch_ch == 0)) {
			klogi_if("[TSP] calibration maybe good\n");
			/* 20120521 */
#if 0
			if ((finger_cnt >= 2) && (tch_ch <= 3) &&
				(tch_ch != 0) && (atch_ch != 0)) {
#else
			if ((finger_cnt >= 2) && (tch_ch <= 3)) {
#endif
				printk(KERN_INFO"[TSP]finger_cnt = %d,"
					"re-calibrate!!\n", finger_cnt);
				calibrate_chip(mxt);
				/* 100ms timer disable */
				timer_flag = DISABLE;
				timer_ticks = 0;
				ts_100ms_timer_stop(mxt);
			} else {
				cal_maybe_good(mxt);
				not_yet_count = 0;
			}
		} else if (atch_ch > ((finger_cnt * num_of_antitouch) + 2)) {
			klogi_if("[TSP] calibration was bad (finger : %d, max_antitouch_num : %d)\n", finger_cnt, finger_cnt*num_of_antitouch);
			calibrate_chip(mxt);
			/* 100ms timer disable */
			timer_flag = DISABLE;
			timer_ticks = 0;
			ts_100ms_timer_stop(mxt);
		} else if((tch_ch + CAL_THR /*10*/ ) <= atch_ch) {
			klogi_if("[TSP] calibration was bad (CAL_THR : %d)\n",CAL_THR);
			/* cal was bad - must recalibrate and check afterwards */
			calibrate_chip(mxt);
			/* 100ms timer disable */
			timer_flag = DISABLE;
			timer_ticks = 0;
			ts_100ms_timer_stop(mxt);

		} else if((tch_ch == 0 ) && (atch_ch >= 2)) {
			klogi_if("[TSP] calibration was bad, tch_ch = %d, atch_ch = %d)\n", tch_ch, atch_ch);
			/* cal was bad - must recalibrate and check afterwards */
			calibrate_chip(mxt);
			/* 100ms timer disable */
			timer_flag = DISABLE;
			timer_ticks = 0;
			ts_100ms_timer_stop(mxt);
		} else {
			cal_check_flag = 1u;
			if (timer_flag == DISABLE) {
				/* 100ms timer enable */

				timer_flag = ENABLE;
				timer_ticks = 0;
				ts_100ms_timer_start(mxt);
			}
			not_yet_count++;
			klogi_if("[TSP] calibration was not decided yet, not_yet_count = %d\n", not_yet_count);
			if((tch_ch == 0) && (atch_ch == 0)) {
				not_yet_count = 0;
			} else if (not_yet_count >= 5) {
				printk(KERN_INFO"[TSP] not_yet_count over 5,"
					"re-calibrate!!\n");
				not_yet_count =0;
				calibrate_chip(mxt);
				/* 100ms timer disable */
				timer_flag = DISABLE;
				timer_ticks = 0;
				ts_100ms_timer_stop(mxt);
			}
		}
	}
}

static void cal_maybe_good(struct mxt_data *mxt)
{
	int ret;
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] %s\n", __func__);
	/* Check if the timer is enabled */
	if (mxt_time_point != 0) {
		/* Check if the timer timedout of 0.3seconds */
		if ((jiffies_to_msecs(jiffies) - mxt_time_point) >= 300) {
			pr_info("[TSP] time from touch press after calibration started = %d\n", (jiffies_to_msecs(jiffies) - mxt_time_point));
			/* Cal was good - don't need to check any more */
			mxt_time_point = 0;
			cal_check_flag = 0;
			/* Disable the timer */
			timer_flag = DISABLE;
			timer_ticks = 0;
			ts_100ms_timer_stop(mxt);
			/* Write back the normal acquisition config to chip. */
			ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHCALST, T8_ATCHCALST);
			ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHCALSTHR, T8_ATCHCALSTHR);
			ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALTHR, T8_ATCHFRCCALTHR);
			ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_GEN_ACQUIRECONFIG_T8) + MXT_ADR_T8_ATCHFRCCALRATIO, T8_ATCHFRCCALRATIO);

			ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) + MXT_ADR_T9_NUMTOUCH, 5);
			//ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_PROCI_TOUCHSUPPRESSION_T42) + MXT_ADR_T42_MAXNUMTCHS, 5);

			/* TSP, Touchscreen threshold */
			//ret = mxt_write_byte(mxt->client, MXT_BASE_ADDR(MXT_TOUCH_MULTITOUCHSCREEN_T9) + MXT_ADR_T9_TCHTHR, 32);

			/* TSP, TouchKEY threshold */
			ret = mxt_write_byte(mxt->client,
				MXT_BASE_ADDR(MXT_TOUCH_KEYARRAY_T15) +
				MXT_ADR_T15_TCHTHR, 42);

			mxt->check_auto_cal = 5;
			klogi_if("[TSP] Calibration success!!\n");

			first_palm_chk = true;

			if (metal_suppression_chk_flag == true) {
				/* after 20 seconds, metal coin checking disable */
				cancel_delayed_work(&mxt->timer_dwork);
				/* 20120518 */
				schedule_delayed_work(&mxt->timer_dwork, 2000);
			}
		}
		else {
			cal_check_flag = 1u;
		}
	}
	else {
		cal_check_flag = 1u;
	}
}

#if TS_100S_TIMER_INTERVAL
/******************************************************************************/
/* 0512 Timer Rework by LBK                            */
/******************************************************************************/
static void ts_100ms_timeout_handler(unsigned long data)
{
	struct mxt_data *mxt = (struct mxt_data*)data;
	mxt->p_ts_timeout_tmr=NULL;
	queue_work(ts_100s_tmr_workqueue, &mxt->tmr_work);
}

static void ts_100ms_timer_start(struct mxt_data *mxt)
{
	if(mxt->p_ts_timeout_tmr != NULL)	del_timer(mxt->p_ts_timeout_tmr);
	mxt->p_ts_timeout_tmr = NULL;

	mxt->ts_timeout_tmr.expires = jiffies + HZ/10;	/* 100ms */
	mxt->p_ts_timeout_tmr = &mxt->ts_timeout_tmr;
	add_timer(&mxt->ts_timeout_tmr);
}

static void ts_100ms_timer_stop(struct mxt_data *mxt)
{
	if(mxt->p_ts_timeout_tmr) del_timer(mxt->p_ts_timeout_tmr);
		mxt->p_ts_timeout_tmr = NULL;
}

static void ts_100ms_timer_init(struct mxt_data *mxt)
{
	init_timer(&(mxt->ts_timeout_tmr));
	mxt->ts_timeout_tmr.data = (unsigned long)(mxt);
	mxt->ts_timeout_tmr.function = ts_100ms_timeout_handler;
	mxt->p_ts_timeout_tmr=NULL;
}

static void ts_100ms_tmr_work(struct work_struct *work)
{
	struct mxt_data *mxt = container_of(work, struct mxt_data, tmr_work);

	uint8_t cal_time = 10;
	timer_ticks++;

	klogi_if("[TSP] 100ms T %d\n", timer_ticks);

	disable_irq(mxt->client->irq);
	/* Palm but Not touch message */
	if(facesup_message_flag ){
		klogi_if("[TSP] facesup_message_flag = %d\n", facesup_message_flag);
		check_chip_palm(mxt);
#if 0
		if (facesup_message_flag == 5)
			cal_time = 1;
#endif
	}

	if ((timer_flag == ENABLE) && (timer_ticks < 20)) {
		ts_100ms_timer_start(mxt);
		palm_check_timer_flag = false;
	} else {
#if 0
		if (facesup_message_flag &&
			((first_palm_chk == true) || cal_check_flag)) {
			klogi_if("[TSP] calibrate_chip_1\n");
			calibrate_chip(mxt);
			palm_check_timer_flag = false;
			timer_flag = DISABLE;
		} else if (palm_check_timer_flag
			&& ((facesup_message_flag == 1) ||
			(facesup_message_flag == 2) ||
			(facesup_message_flag == 5)) &&
			(palm_release_flag == false)) {
			klogi_if("[TSP] calibrate_chip_2\n");
			calibrate_chip(mxt);
			palm_check_timer_flag = false;
			timer_flag = DISABLE;
		}
#else
		if (palm_check_timer_flag &&
			((facesup_message_flag == 1) ||
			(facesup_message_flag == 2))
			&& (palm_release_flag == false)) {
			klogi_if("[TSP] calibrate_chip\n");
			calibrate_chip(mxt);
			palm_check_timer_flag = false;
		}
		timer_flag = DISABLE;
#endif
		timer_ticks = 0;
	}
	enable_irq(mxt->client->irq);
}

#endif

/******************************************************************************/
/* Initialization of driver                                                   */
/******************************************************************************/

static int  mxt_identify(struct i2c_client *client,
			 struct mxt_data *mxt)
{
	u8 buf[7];
	int error;
	int identified;

	identified = 0;

retry_i2c:
	/* Read Device info to check if chip is valid */
	error = mxt_read_block(client, MXT_ADDR_INFO_BLOCK, 7, (u8 *)buf);

	if (error < 0) {
		mxt->read_fail_counter++;
		if (mxt->read_fail_counter == 1) {
			if (debug >= DEBUG_INFO)
				pr_info("[TSP] Warning: To wake up touch-ic in deep sleep, retry i2c communication!");
			msleep(30);  /* delay 25ms */
			goto retry_i2c;
		}
		dev_err(&client->dev, "[TSP] Failure accessing maXTouch device\n");
		return -EIO;
	}

	mxt->device_info.family_id  = buf[0];
	mxt->device_info.variant_id = buf[1];
	mxt->device_info.major	    = ((buf[2] >> 4) & 0x0F);
	mxt->device_info.minor      = (buf[2] & 0x0F);
	mxt->device_info.build	    = buf[3];
	mxt->device_info.x_size	    = buf[4];
	mxt->device_info.y_size	    = buf[5];
	mxt->device_info.num_objs   = buf[6];
	mxt->device_info.num_nodes  = mxt->device_info.x_size *
		mxt->device_info.y_size;

	/* Check Family Info */
	if (mxt->device_info.family_id == MAXTOUCH_FAMILYID) {
		strcpy(mxt->device_info.family, maxtouch_family);
	} else {
		dev_err(&client->dev,
			"[TSP] maXTouch Family ID [0x%x] not supported\n",
			mxt->device_info.family_id);
		identified = -ENXIO;
	}

	/* Check Variant Info */
	if ((mxt->device_info.variant_id == MXT224_CAL_VARIANTID) ||
		(mxt->device_info.variant_id == MXT224_UNCAL_VARIANTID)) {
		strcpy(mxt->device_info.variant, mxt224_variant);
	} else {
		dev_err(&client->dev,
			"[TSP] maXTouch Variant ID [0x%x] not supported\n",
			mxt->device_info.variant_id);
		identified = -ENXIO;
	}

	if (debug >= DEBUG_MESSAGES)
		dev_info(
			&client->dev,
			"[TSP] Atmel %s.%s Firmware version [%d.%d] Build [%d]\n",
			mxt->device_info.family,
			mxt->device_info.variant,
			mxt->device_info.major,
			mxt->device_info.minor,
			mxt->device_info.build
			);
	if (debug >= DEBUG_MESSAGES)
		dev_info(
			&client->dev,
			"[TSP] Atmel %s.%s Configuration [X: %d] x [Y: %d]\n",
			mxt->device_info.family,
			mxt->device_info.variant,
			mxt->device_info.x_size,
			mxt->device_info.y_size
			);
	if (debug >= DEBUG_MESSAGES)
		dev_info(
			&client->dev,
			"[TSP] number of objects: %d\n",
			mxt->device_info.num_objs
			);

	return identified;
}

/*
* Reads the object table from maXTouch chip to get object data like
* address, size, report id.
*/
static int mxt_read_object_table(struct i2c_client *client,
				 struct mxt_data *mxt)
{
	u16	report_id_count;
	u8	buf[MXT_OBJECT_TABLE_ELEMENT_SIZE];
	u8	object_type;
	u16	object_address;
	u16	object_size;
	u8	object_instances;
	u8	object_report_ids;
	u16	object_info_address;
	u32	crc;
	u32     crc_calculated;
	int	i;
	int	error;

	u8	object_instance;
	u8	object_report_id;
	u8	report_id;
	int     first_report_id;

	struct mxt_object *object_table;

	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver get configuration\n");

	object_table = kzalloc(sizeof(struct mxt_object) *
		mxt->device_info.num_objs,
		GFP_KERNEL);
	if (object_table == NULL) {
		pr_warning("[TSP] maXTouch: Memory allocation failed!\n");
		return -ENOMEM;
	}

	mxt->object_table = object_table;

	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver Memory allocated\n");

	object_info_address = MXT_ADDR_OBJECT_TABLE;

	report_id_count = 0;
	for (i = 0; i < mxt->device_info.num_objs; i++) {
		if (debug >= DEBUG_TRACE)
			pr_info("[TSP] Reading maXTouch at [0x%04x]: ",
			object_info_address);

		error = mxt_read_block(client, object_info_address, MXT_OBJECT_TABLE_ELEMENT_SIZE, (u8 *)buf);

		if (error < 0) {
			mxt->read_fail_counter++;
			dev_err(&client->dev,
				"[TSP] maXTouch Object %d could not be read\n", i);
			return -EIO;
		}
		object_type		=  buf[0];
		object_address		= (buf[2] << 8) + buf[1];
		object_size		=  buf[3] + 1;
		object_instances	=  buf[4] + 1;
		object_report_ids	=  buf[5];
		if (debug >= DEBUG_TRACE)
			pr_info("[TSP] Type=%03d, Address=0x%04x, "
			"Size=0x%02x, %d instances, %d report id's\n",
			object_type,
			object_address,
			object_size,
			object_instances,
			object_report_ids
			);

		if (object_type > MXT_MAX_OBJECT_TYPES) {
			/* Unknown object type */
			dev_err(&client->dev,
				"[TSP] maXTouch object type [%d] not recognized\n",
				object_type);
			return -ENXIO;

		}

		/* Save frequently needed info. */
		if (object_type == MXT_GEN_MESSAGEPROCESSOR_T5) {
			mxt->msg_proc_addr = object_address;
			mxt->message_size = object_size;
		}

		object_table[i].type            = object_type;
		object_table[i].chip_addr       = object_address;
		object_table[i].size            = object_size;
		object_table[i].instances       = object_instances;
		object_table[i].num_report_ids  = object_report_ids;
		report_id_count += object_instances * object_report_ids;

		object_info_address += MXT_OBJECT_TABLE_ELEMENT_SIZE;
	}

	mxt->rid_map =
		kzalloc(sizeof(struct report_id_map) * (report_id_count + 1),
		/* allocate for report_id 0, even if not used */
		GFP_KERNEL);
	if (mxt->rid_map == NULL) {
		pr_warning("[TSP] maXTouch: Can't allocate memory!\n");
		return -ENOMEM;
	}

	mxt->last_message = kzalloc(mxt->message_size, GFP_KERNEL);
	if (mxt->last_message == NULL) {
		pr_warning("[TSP] maXTouch: Can't allocate memory!\n");
		return -ENOMEM;
	}


	mxt->report_id_count = report_id_count;
	if (report_id_count > 254) { 	/* 0 & 255 are reserved */
		dev_err(&client->dev,
			"[TSP] Too many maXTouch report id's [%d]\n",
			report_id_count);
		return -ENXIO;
	}

	/* Create a mapping from report id to object type */
	report_id = 1; /* Start from 1, 0 is reserved. */

	/* Create table associating report id's with objects & instances */
	for (i = 0; i < mxt->device_info.num_objs; i++) {
		for (object_instance = 0;
		object_instance < object_table[i].instances;
		object_instance++) {
			first_report_id = report_id;
			for (object_report_id = 0;
			object_report_id < object_table[i].num_report_ids;
			object_report_id++) {
				mxt->rid_map[report_id].object =
					object_table[i].type;
				mxt->rid_map[report_id].instance =
					object_instance;
				mxt->rid_map[report_id].first_rid =
					first_report_id;
				report_id++;
			}
		}
	}

	/* Read 3 byte CRC */
	error = mxt_read_block(client, object_info_address, 3, buf);
	if (error < 0) {
		mxt->read_fail_counter++;
		dev_err(&client->dev, "[TSP] Error reading CRC\n");
	}

	crc = (buf[2] << 16) | (buf[1] << 8) | buf[0];

	calculate_infoblock_crc(&crc_calculated, mxt);

	if (debug >= DEBUG_TRACE) {
		pr_info("[TSP] Reported info block CRC = 0x%6X\n\n", crc);
		pr_info("[TSP] Calculated info block CRC = 0x%6X\n\n",
			crc_calculated);
	}

	if (crc == crc_calculated) {
		mxt->info_block_crc = crc;
	} else {
		mxt->info_block_crc = 0;
		pr_warning("[TSP] maXTouch: info block CRC invalid!\n");
	}


	mxt->delta	= NULL;
	mxt->reference	= NULL;
	mxt->cte	= NULL;

	if (debug >= DEBUG_VERBOSE) {

		dev_info(&client->dev, "[TSP] maXTouch: %d Objects\n",
			mxt->device_info.num_objs);

		for (i = 0; i < mxt->device_info.num_objs; i++) {
			dev_info(&client->dev, "[TSP] Type:\t\t\t[%d]: %s\n",
				object_table[i].type,
				object_type_name[object_table[i].type]);
			dev_info(&client->dev, "\tAddress:\t0x%04X\n",
				object_table[i].chip_addr);
			dev_info(&client->dev, "\tSize:\t\t%d Bytes\n",
				object_table[i].size);
			dev_info(&client->dev, "\tInstances:\t%d\n",
				object_table[i].instances);
			dev_info(&client->dev, "\tReport Id's:\t%d\n",
				object_table[i].num_report_ids);
		}
	}
	return 0;
}

u8 mxt_valid_interrupt(void)
{
	/* TO_CHK: how to implement this function? */
	return 1;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static bool tsp_sleep_mode_flag = false;
static void mxt_early_suspend(struct early_suspend *h)
{
	struct	mxt_data *mxt = container_of(h, struct mxt_data, early_suspend);
	u8 cmd_sleep[2] = { 0};
	u16 addr;
	u8 i;

	if (debug >= DEBUG_INFO)
		pr_info("[TSP] mxt_early_suspend has been called!");
#if defined(MXT_FACTORY_TEST) || defined(MXT_FIRMUP_ENABLE)
	/*start firmware updating : not yet finished*/
	while (mxt->firm_status_data == 1) {
		if (debug >= DEBUG_INFO)
			pr_info("[TSP] mxt firmware is Downloading : mxt suspend must be delayed!");
		msleep(1000);
	}
#endif

	cancel_delayed_work(&mxt->config_dwork);
	metal_suppression_chk_flag = false;
	cancel_delayed_work(&mxt->timer_dwork);
	cancel_delayed_work(&mxt->initial_dwork);
	disable_irq(mxt->client->irq);
	ts_100ms_timer_stop(mxt);
	mxt_release_all_fingers(mxt);
	mxt_release_all_keys(mxt);

	/* global variable initialize */
	timer_flag = DISABLE;
	timer_ticks = 0;
	mxt_time_point = 0;
	coin_check_flag = false;
	coin_check_count = 0;
	time_after_autocal_enable = 0;
	time_after_autocal_enable_key = 0;
	for(i=0;i < 10; i++) {
		tch_vct[i].cnt = 0;
	}
	mxt->mxt_status = false;
	key_led_on(mxt, 0);
#if 0
#ifdef MXT_SLEEP_POWEROFF
	if (mxt->pdata->suspend_platform_hw != NULL)
		mxt->pdata->suspend_platform_hw(mxt->pdata);
#else
		/*
		* a setting of zeros to IDLEACQINT and ACTVACQINT
		* forces the chip set to enter Deep Sleep mode.
	*/
	addr = get_object_address(MXT_GEN_POWERCONFIG_T7, 0, mxt->object_table, mxt->device_info.num_objs);
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] addr: 0x%02x, buf[0]=0x%x, buf[1]=0x%x", addr, cmd_sleep[0], cmd_sleep[1]);
	mxt_write_block(mxt->client, addr, 2, (u8 *)cmd_sleep);
#endif
#else
if (mxt->set_mode_for_ta) {	/* if TA -> TSP go to sleep mode */
	/*
	* a setting of zeros to IDLEACQINT and ACTVACQINT
	* forces the chip set to enter Deep Sleep mode.
	*/
	addr = get_object_address(MXT_GEN_POWERCONFIG_T7, 0, mxt->object_table, mxt->device_info.num_objs);
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] addr: 0x%02x, buf[0]=0x%x, buf[1]=0x%x", addr, cmd_sleep[0], cmd_sleep[1]);
	mxt_write_block(mxt->client, addr, 2, (u8 *)cmd_sleep);
	tsp_sleep_mode_flag = true;
} else {
	if (mxt->pdata->suspend_platform_hw != NULL)
		mxt->pdata->suspend_platform_hw(mxt->pdata);
	tsp_sleep_mode_flag = false;
}
#endif
}

static void mxt_late_resume(struct early_suspend *h)
{
	u8 cnt;
	struct	mxt_data *mxt = container_of(h, struct mxt_data, early_suspend);
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] mxt_late_resumehas been called!");

	if (key_led_status)
		key_led_on(mxt, 0xff);
#if 0
#ifdef MXT_SLEEP_POWEROFF
	if (mxt->pdata->resume_platform_hw != NULL)
		mxt->pdata->resume_platform_hw(mxt->pdata);
#else
	for (cnt = 10; cnt > 0; cnt--) {
		if (mxt_power_config(mxt) < 0)
			continue;
		if (reset_chip(mxt, RESET_TO_NORMAL) == 0)  /* soft reset */
			break;
	}
	if (cnt == 0) {
		pr_err("[TSP] mxt_late_resume failed!!!");
		return;
	}
#endif
#else
	if (tsp_sleep_mode_flag == true) {
		for (cnt = 10; cnt > 0; cnt--) {
#ifdef TOUCH_LOCKUP_PATTERN_RELEASE
			touch_is_pressed_arr[cnt] = 0;
#endif
			if (mxt_power_config(mxt) >= 0)
				break;
		}

		if (cnt == 0) {
			pr_err("[TSP] mxt_power_config failed, reset IC!!!");
			reset_chip(mxt, RESET_TO_NORMAL);
			msleep(100);
			if(mxt->set_mode_for_ta && !work_pending(&mxt->ta_work))
				schedule_work(&mxt->ta_work);
		} else {
			msleep(30);
		}

		/* when sleep mode resume, Is TA detached? */
		if (mxt->set_mode_for_ta == 0 && !work_pending(&mxt->ta_work)) {
			schedule_work(&mxt->ta_work);
		} else {
			calibrate_chip(mxt);
		}
	} else {
		if (mxt->pdata->resume_platform_hw != NULL)
			mxt->pdata->resume_platform_hw(mxt->pdata);

		msleep(100);  /*typical value is 200ms*/

		if (mxt->set_mode_for_ta && !work_pending(&mxt->ta_work)) {
			schedule_work(&mxt->ta_work);
		} else {
			calibrate_chip(mxt);
		}
	}
#endif
#ifndef MXT_SLEEP_POWEROFF
	calibrate_chip(mxt);
#endif
	metal_suppression_chk_flag = true;
	mxt->mxt_status = true;
	enable_irq(mxt->client->irq);
}
#endif


static int __devinit mxt_probe(struct i2c_client *client,
			       const struct i2c_device_id *id)
{
	struct mxt_data          *mxt;
	struct mxt_platform_data *pdata;
	struct input_dev         *input;
	int error;
	int i;
	u8 unverified = 0;
	u8  udata[8];

#ifdef MXT_FIRMUP_ENABLE
	/* mXT224E Latest Firmware version [1.0] Build [0xaa]*/
	u8 last_major = 0x01;
	u8 last_minor = 0x00;
	u8 last_build = 0xaa;
#endif

#ifdef KEY_LED_CONTROL
	struct class *leds_class;
	struct device *led_dev;
#endif

	if (debug >= DEBUG_INFO)
		pr_info("[TSP] mXT224E: mxt_probe\n");

	if (client == NULL)
		pr_err("[TSP] maXTouch: client == NULL\n");
	else if (client->adapter == NULL)
		pr_err("[TSP] maXTouch: client->adapter == NULL\n");
	else if (&client->dev == NULL)
		pr_err("[TSP] maXTouch: client->dev == NULL\n");
	else if (&client->adapter->dev == NULL)
		pr_err("[TSP] maXTouch: client->adapter->dev == NULL\n");
	else if (id == NULL)
		pr_err("[TSP] maXTouch: id == NULL\n");
	else
		goto param_check_ok;
	return	-EINVAL;

param_check_ok:
	if (debug >= DEBUG_INFO) {
		pr_info("[TSP] maXTouch driver\n");
		pr_info("\t \"%s\"\n",		client->name);
		pr_info("\taddr:\t0x%04x\n",	client->addr);
		pr_info("\tirq:\t%d\n",	client->irq);
		pr_info("\tflags:\t0x%04x\n",	client->flags);
		pr_info("\tadapter:\"%s\"\n",	client->adapter->name);
		pr_info("\tdevice:\t\"%s\"\n",	client->dev.init_name);
	}
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] Parameters OK\n");;

	/* Allocate structure - we need it to identify device */
	mxt = kzalloc(sizeof(struct mxt_data), GFP_KERNEL);
	input = input_allocate_device();
	if (!mxt || !input) {
		dev_err(&client->dev, "[TSP] insufficient memory\n");
		error = -ENOMEM;
		goto err_after_kmalloc;
	}

	/* Initialize Platform data */
	pdata = client->dev.platform_data;
	if (pdata == NULL) {
		dev_err(&client->dev, "[TSP] platform data is required!\n");
		error = -EINVAL;
		goto err_after_kmalloc;
	}
	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] Platform OK: pdata = 0x%08x\n", (unsigned int) pdata);
	mxt->pdata = pdata;

	mxt->read_fail_counter = 0;
	mxt->message_counter   = 0;

	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver identifying chip\n");

	if (debug >= DEBUG_INFO)
		pr_info("\tboard-revision:\t\"%d\"\n",	mxt->pdata->board_rev);

	mxt->client = client;
	mxt->input  = input;

	/* mxt224E regulator config */
	mxt->pdata->reg_vdd = regulator_get(NULL, mxt->pdata->reg_vdd_name);

	if (IS_ERR(mxt->pdata->reg_vdd)) {
		error = PTR_ERR(mxt->pdata->reg_vdd);
		pr_err("[TSP] [%s: %s]unable to get regulator %s: %d\n",
			__func__,
			mxt->pdata->platform_name,
			mxt->pdata->reg_vdd_name,
			error);
	}

	mxt->pdata->reg_avdd = regulator_get(NULL, mxt->pdata->reg_avdd_name);

	if (IS_ERR(mxt->pdata->reg_avdd)) {
		error = PTR_ERR(mxt->pdata->reg_avdd);
		pr_err("[TSP] [%s: %s]unable to get regulator %s: %d\n",
			__func__,
			mxt->pdata->platform_name,
			mxt->pdata->reg_avdd_name,
			error);
	}

	mxt->pdata->reg_vdd_lvsio = regulator_get(NULL, mxt->pdata->reg_vdd_lvsio_name);

	if (IS_ERR(mxt->pdata->reg_vdd_lvsio)) {
		error = PTR_ERR(mxt->pdata->reg_vdd_lvsio);
		pr_err("[TSP] [%s: %s]unable to get regulator %s: %d\n",
			__func__,
			mxt->pdata->platform_name,
			mxt->pdata->reg_vdd_lvsio_name,
			error);
	}

	/* TSP Power on */
	error = regulator_enable(mxt->pdata->reg_vdd);
	regulator_set_voltage(mxt->pdata->reg_vdd,
		mxt->pdata->reg_vdd_level,
		mxt->pdata->reg_vdd_level);

	error = regulator_enable(mxt->pdata->reg_avdd);
	regulator_set_voltage(mxt->pdata->reg_avdd,
		mxt->pdata->reg_avdd_level,
		mxt->pdata->reg_avdd_level);

	error = regulator_enable(mxt->pdata->reg_vdd_lvsio);
	regulator_set_voltage(mxt->pdata->reg_vdd_lvsio,
		mxt->pdata->reg_vdd_lvsio_level,
		mxt->pdata->reg_vdd_lvsio_level);
	msleep(250);

	error = mxt_identify(client, mxt);
	if (error < 0) {
		dev_err(&client->dev, "[TSP] ATMEL Chip could not be identified. error = %d\n", error);
#ifdef MXT_FIRMUP_ENABLE
		unverified = 1;
#else
		goto err_after_get_regulator;
#endif
	}

	error = mxt_read_object_table(client, mxt);
	if (error < 0){
		dev_err(&client->dev, "[TSP] ATMEL Chip could not read obkect table. error = %d\n", error);
#ifdef MXT_FIRMUP_ENABLE
		unverified = 1;
#else
		goto err_after_get_regulator;
#endif
	}

	i2c_set_clientdata(client, mxt);
	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver setting drv data\n");

#ifdef MXT_FIRMUP_ENABLE /*auto firmware upgrade check */
	if ((mxt->device_info.major < last_major) || (mxt->device_info.minor < last_minor) || (mxt->device_info.build < last_build) || unverified) {
		pr_warning("[TSP] Touch firm up is needed to last version :[%d.%d] , build : [%d] ", last_major, last_minor, last_build);
		mxt->firm_status_data = 1;		/* start firmware updating */
		error = set_mxt_auto_update_exe(&client->dev);
		if (error < 0)
			goto err_after_get_regulator;
	}
#endif

	/* Chip is valid and active. */
	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver allocating input device\n");

#ifndef MXT_THREADED_IRQ
	INIT_DELAYED_WORK(&mxt->dwork, mxt_worker);
#endif
	INIT_WORK(&mxt->ta_work, mxt_ta_worker);

	INIT_DELAYED_WORK(&mxt->config_dwork, mxt_palm_recovery);

	INIT_DELAYED_WORK(&mxt->timer_dwork, mxt_metal_suppression_off);

	INIT_DELAYED_WORK(&mxt->initial_dwork, mxt_boot_delayed_initial);

#ifdef MXT_FACTORY_TEST
	INIT_DELAYED_WORK(&mxt->firmup_dwork, set_mxt_update_exe);
#endif

	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver init spinlock\n");

	/* Register callbacks */
	/* To inform tsp , charger connection status*/
	mxt->callbacks.inform_charger = mxt_inform_charger_connection;
	if (mxt->pdata->register_cb)
		mxt->pdata->register_cb(&mxt->callbacks);

	init_waitqueue_head(&mxt->msg_queue);
	init_MUTEX(&mxt->msg_sem);

	spin_lock_init(&mxt->lock);


	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver creating device name\n");

	snprintf(
		mxt->phys_name,
		sizeof(mxt->phys_name),
		"%s/input0",
		dev_name(&client->dev)
		);

	input->name = "sec_touchscreen";
	input->phys = mxt->phys_name;
	input->id.bustype = BUS_I2C;
	input->dev.parent = &client->dev;

	if (debug >= DEBUG_INFO) {
		pr_info("[TSP] maXTouch name: \"%s\"\n", input->name);
		pr_info("[TSP] maXTouch phys: \"%s\"\n", input->phys);
		pr_info("[TSP] maXTouch driver setting abs parameters\n");
	}
//	__set_bit(BTN_TOUCH, input->keybit);
	__set_bit(EV_ABS, input->evbit);
	/* single touch */
#if 0
	input_set_abs_params(input, ABS_X, 0, mxt->pdata->max_x, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, mxt->pdata->max_y, 0, 0);
	input_set_abs_params(input, ABS_PRESSURE, 0, MXT_MAX_REPORTED_PRESSURE, 0, 0);
	input_set_abs_params(input, ABS_TOOL_WIDTH, 0, MXT_MAX_REPORTED_WIDTH, 0, 0);
#endif
#if 1
// N1_ICS
	__set_bit(MT_TOOL_FINGER, input->keybit);
	__set_bit(INPUT_PROP_DIRECT, input->propbit);
	input_mt_init_slots(input, MXT_MAX_NUM_TOUCHES);
#endif
	/* multi touch */
	input_set_abs_params(input, ABS_MT_POSITION_X,
		0, mxt->pdata->max_x, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y,
		0, mxt->pdata->max_y, 0, 0);
	input_set_abs_params(input, ABS_MT_PRESSURE,
		0, 255, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR,
		0, 30, 0, 0);
	__set_bit(EV_SYN, input->evbit);
	__set_bit(EV_KEY, input->evbit);
	__set_bit(EV_LED, input->evbit);
	__set_bit(LED_MISC, input->ledbit);

	/* touch key */
	if ((mxt->pdata->board_rev <= 9) || (mxt->pdata->board_rev >= 13)) {
		for (i = 0; i < NUMOFKEYS; i++) {
			__set_bit(tsp_keycodes[i], input->keybit);
		}
	} else {	/* board rev1.0~1.2, touch key is 4 key array */
		for (i = 0; i < NUMOF4KEYS; i++) {
			__set_bit(tsp_4keycodes[i], input->keybit);
		}
	}

	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver setting client data\n");

	input_set_drvdata(input, mxt);

	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver input register device\n");

	error = input_register_device(mxt->input);
	if (error < 0) {
		dev_err(&client->dev,
			"[TSP] Failed to register input device\n");
		goto err_after_get_regulator;
	}
	if (debug >= DEBUG_TRACE)
		pr_info("[TSP] maXTouch driver allocate interrupt\n");

#ifndef MXT_TUNNING_ENABLE
	/* pre-set configuration before soft reset */

	mxt_read_byte(mxt->client,  MXT_BASE_ADDR(MXT_USER_INFO_T38)+ MXT_ADR_T38_CFG_CTRL, &udata[0]);

	if (debug >= DEBUG_MESSAGES)
		pr_info("\t[TSP] udata[0] = :\t\"%d\"\n", udata[0]); /* temporary */
	if (udata[0] == MXT_CFG_OVERWRITE) {  /* for manual tuning */
		error = mxt_config_settings(mxt);
		if (error < 0)
			goto err_after_interrupt_register;
	}

	/* backup to nv memory */
	backup_to_nv(mxt);
	/* forces a reset of the chipset */
	//reset_chip(mxt, RESET_TO_NORMAL);
	//msleep(250); /*need 250ms*/
	//calibrate_chip(mxt);

#endif
	for (i = 0; i < MXT_MAX_NUM_TOUCHES ; i++)	/* _SUPPORT_MULTITOUCH_ */
		mtouch_info[i].pressure = -1;

#ifndef MXT_THREADED_IRQ
		/* Schedule a worker routine to read any messages that might have
	* been sent before interrupts were enabled. */
	cancel_delayed_work(&mxt->dwork);
	schedule_delayed_work(&mxt->dwork, 0);
#endif


	/* Allocate the interrupt */
	mxt->irq = client->irq;
	mxt->valid_irq_counter = 0;
	mxt->invalid_irq_counter = 0;
	mxt->irq_counter = 0;
#if 0
	if (mxt->irq) {
	/* Try to request IRQ with falling edge first. This is
		* not always supported. If it fails, try with any edge. */
#ifdef MXT_THREADED_IRQ
		error = request_threaded_irq(mxt->irq,
			NULL,
			mxt_threaded_irq,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT,
			client->dev.driver->name,
			mxt);
		if (error < 0) {
			error = request_threaded_irq(mxt->irq,
				NULL,
				mxt_threaded_irq,
				IRQF_DISABLED,
				client->dev.driver->name,
				mxt);
		}
#else
		error = request_irq(mxt->irq,
			mxt_irq_handler,
			IRQF_TRIGGER_FALLING,
			client->dev.driver->name,
			mxt);
		if (error < 0) {
			error = request_irq(mxt->irq,
				mxt_irq_handler,
				0,
				client->dev.driver->name,
				mxt);
		}
#endif
		if (error < 0) {
			dev_err(&client->dev,
				"[TSP] failed to allocate irq %d\n", mxt->irq);
			goto err_after_input_register;
		}
	}

	if (debug > DEBUG_INFO)
		dev_info(&client->dev, "[TSP] touchscreen, irq %d\n", mxt->irq);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	mxt->early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 10;
	mxt->early_suspend.suspend = mxt_early_suspend;
	mxt->early_suspend.resume = mxt_late_resume;
	register_early_suspend(&mxt->early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

#ifdef KEY_LED_CONTROL
	/* create sysfs */
	leds_class = class_create(THIS_MODULE, "leds");
	if (IS_ERR(leds_class)) {
		return PTR_ERR(leds_class);
	}
	led_dev = device_create(leds_class, NULL, 0, mxt, "button-backlight");
	if (device_create_file(led_dev, &dev_attr_brightness) < 0) {
		pr_err("[TSP] Failed to create device file(%s)!\n", dev_attr_brightness.attr.name);
	}
#endif

	error = sysfs_create_group(&client->dev.kobj, &maxtouch_attr_group);
	if (error) {
		unregister_early_suspend(&mxt->early_suspend);
		pr_err("[TSP] fail sysfs_create_group\n");
		goto err_after_interrupt_register;
	}

#ifdef MXT_FACTORY_TEST
	tsp_factory_mode = device_create(sec_class, NULL, 0, mxt, "sec_touchscreen");
	if (IS_ERR(tsp_factory_mode)) {
		pr_err("[TSP] Failed to create device!\n");
		error = -ENODEV;
		goto err_after_attr_group;
	}

	error = sysfs_create_group(&tsp_factory_mode->kobj, &maxtouch_factory_attr_group);
	if (error) {
		if (unverified) {
			pr_err("[TSP] fail sysfs_create_group 1\n");
			goto err_after_attr_group;
		} else {
			unregister_early_suspend(&mxt->early_suspend);
			pr_err("[TSP] fail sysfs_create_group 2\n");
			goto err_after_attr_group;
		}
	}
#endif

#if	TS_100S_TIMER_INTERVAL
		INIT_WORK(&mxt->tmr_work, ts_100ms_tmr_work);

		ts_100s_tmr_workqueue = create_singlethread_workqueue("ts_100_tmr_workqueue");
		if (!ts_100s_tmr_workqueue)
		{
			printk(KERN_ERR "unabled to create touch tmr work queue \r\n");
			error = -1;
			goto err_after_attr_group;
		}
		ts_100ms_timer_init(mxt);
#endif

	wake_lock_init(&mxt->wakelock, WAKE_LOCK_SUSPEND, "touch");
	mxt->mxt_status = true;

	/* after 15sec, start touch working */
	schedule_delayed_work(&mxt->initial_dwork, 1500);
	if (debug >= DEBUG_INFO)
		pr_info("[TSP] mxt probe ok\n");
	return 0;

err_after_attr_group:
	sysfs_remove_group(&client->dev.kobj, &maxtouch_attr_group);

err_after_interrupt_register:
	if (mxt->irq)
		free_irq(mxt->irq, mxt);
err_after_input_register:
	input_free_device(input);

err_after_get_regulator:
	regulator_disable(mxt->pdata->reg_vdd);
	regulator_disable(mxt->pdata->reg_avdd);
	regulator_disable(mxt->pdata->reg_vdd_lvsio);

	regulator_put(mxt->pdata->reg_vdd);
	regulator_put(mxt->pdata->reg_avdd);
	regulator_put(mxt->pdata->reg_vdd_lvsio);
err_after_kmalloc:
	if (mxt != NULL) {
		kfree(mxt->rid_map);
		kfree(mxt->delta);
		kfree(mxt->reference);
		kfree(mxt->cte);
		kfree(mxt->object_table);
		kfree(mxt->last_message);
		/* if (mxt->pdata->exit_platform_hw != NULL) */
		/*	mxt->pdata->exit_platform_hw(); */
	}
	kfree(mxt);

	return error;
}

static int __devexit mxt_remove(struct i2c_client *client)
{
	struct mxt_data *mxt;

	mxt = i2c_get_clientdata(client);
#ifdef CONFIG_HAS_EARLYSUSPEND
	wake_lock_destroy(&mxt->wakelock);
	unregister_early_suspend(&mxt->early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */
	/* Close down sysfs entries */
	sysfs_remove_group(&client->dev.kobj, &maxtouch_attr_group);

	/* Release IRQ so no queue will be scheduled */
	if (mxt->irq)
		free_irq(mxt->irq, mxt);
#ifndef MXT_THREADED_IRQ
	cancel_delayed_work_sync(&mxt->dwork);
#endif
	input_unregister_device(mxt->input);
	/* Should dealloc deltas, references, CTE structures, if allocated */

	if (mxt != NULL) {
		kfree(mxt->rid_map);
		kfree(mxt->delta);
		kfree(mxt->reference);
		kfree(mxt->cte);
		kfree(mxt->object_table);
		kfree(mxt->last_message);
	}
	kfree(mxt);

	i2c_set_clientdata(client, NULL);
	if (debug >= DEBUG_TRACE)
		dev_info(&client->dev, "[TSP] Touchscreen unregistered\n");

	return 0;
}

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
static int mxt_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct mxt_data *mxt = i2c_get_clientdata(client);

	if (device_may_wakeup(&client->dev))
		enable_irq_wake(mxt->irq);

	return 0;
}

static int mxt_resume(struct i2c_client *client)
{
	struct mxt_data *mxt = i2c_get_clientdata(client);

	if (device_may_wakeup(&client->dev))
		disable_irq_wake(mxt->irq);

	return 0;
}
#else
#define mxt_suspend NULL
#define mxt_resume NULL
#endif

static const struct i2c_device_id mxt_idtable[] = {
	{ "mxt_touch", 0,},
	{  }
};

MODULE_DEVICE_TABLE(i2c, mxt_idtable);

static struct i2c_driver mxt_driver = {
	.driver = {
		.name	= "mxt_touch",
			.owner  = THIS_MODULE,
	},

	.id_table	= mxt_idtable,
	.probe		= mxt_probe,
	.remove		= __devexit_p(mxt_remove),
	.suspend	= mxt_suspend,
	.resume		= mxt_resume,

};

static int __init mxt_init(void)
{
	int err;
	err = i2c_add_driver(&mxt_driver);
	/*	if (err) {
	*		pr_warning("Adding mXT224E driver failed "
	*			"(errno = %d)\n", err);
	*	} else {
	*		pr_info("Successfully added driver %s\n",
	*			mxt_driver.driver.name);
	*	}
	*/
	return err;
}

static void __exit mxt_cleanup(void)
{
	i2c_del_driver(&mxt_driver);
}


module_init(mxt_init);
module_exit(mxt_cleanup);

MODULE_AUTHOR("Samsung");
MODULE_DESCRIPTION("Driver for Atmel mXT224E Touchscreen Controller");

MODULE_LICENSE("GPL");
