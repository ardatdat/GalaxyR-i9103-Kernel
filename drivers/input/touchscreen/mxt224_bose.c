/*
 *  Copyright (C) 2010, Samsung Electronics Co. Ltd. All Rights Reserved.
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
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c/mxt224_bose.h>
#include <asm/unaligned.h>
#include <linux/firmware.h>

#include <linux/regulator/consumer.h>

#define OBJECT_TABLE_START_ADDRESS	7
#define OBJECT_TABLE_ELEMENT_SIZE	6

#define CMD_RESET_OFFSET		0
#define CMD_BACKUP_OFFSET		1
#define CMD_CALIBRATE_OFFSET    2
#define CMD_REPORTATLL_OFFSET   3
#define CMD_DEBUG_CTRL_OFFSET   4
#define CMD_DIAGNOSTIC_OFFSET   5


#define DETECT_MSG_MASK			0x80
#define PRESS_MSG_MASK			0x40
#define RELEASE_MSG_MASK		0x20
#define MOVE_MSG_MASK			0x10
#define SUPPRESS_MSG_MASK		0x02

/* Version */
#define MXT224_VER_20			20
#define MXT224_VER_21			21
#define MXT224_VER_22			22

/* Slave addresses */
#define MXT224_APP_LOW		0x4a
#define MXT224_APP_HIGH		0x4b
#define MXT224_BOOT_LOW		0x24
#define MXT224_BOOT_HIGH		0x25

/* FIRMWARE NAME */
#define MXT224_ECHO_FW_NAME	    "mXT224E.fw"
#define MXT224_FW_NAME		    "mXT224.fw"
u8 firmware_latest_ver[] = {
#include "atmel_mxt224e_fw_ver10_aa.h"
};

#define MXT224_FWRESET_TIME		175	/* msec */
#define MXT224_RESET_TIME		65	/* msec */

#define MXT224_BOOT_VALUE		0xa5
#define MXT224_BACKUP_VALUE		0x55

/* Bootloader mode status */
#define MXT224_WAITING_BOOTLOAD_CMD	0xc0	/* valid 7 6 bit only */
#define MXT224_WAITING_FRAME_DATA	0x80	/* valid 7 6 bit only */
#define MXT224_FRAME_CRC_CHECK	0x02
#define MXT224_FRAME_CRC_FAIL		0x03
#define MXT224_FRAME_CRC_PASS		0x04
#define MXT224_APP_CRC_FAIL		0x40	/* valid 7 8 bit only */
#define MXT224_BOOT_STATUS_MASK	0x3f

/* Command to unlock bootloader */
#define MXT224_UNLOCK_CMD_MSB		0xaa
#define MXT224_UNLOCK_CMD_LSB		0xdc

#define ID_BLOCK_SIZE			7

#define DRIVER_FILTER

#define MXT224_STATE_INACTIVE		-1
#define MXT224_STATE_RELEASE		0
#define MXT224_STATE_PRESS		1
#define MXT224_STATE_MOVE		2

#define MAX_USING_FINGER_NUM 10

#define READ_FW_FROM_HEADER	1

struct mxt224_data *copy_data;
extern struct class *sec_class;
int touch_is_pressed;
EXPORT_SYMBOL(touch_is_pressed);
static int mxt224_enabled;
static bool g_debug_switch;
static u8 threshold;
static int firm_status_data;
bool tsp_deepsleep;
EXPORT_SYMBOL(tsp_deepsleep);
static bool auto_cal_flag;
static bool boot_or_resume = 1;		/*1: boot_or_resume,0: others*/
static bool suspend_ta_status;
static u8 Doing_calibration_flag;

#if 1 /* eplus andy add 20111028 */
/* add for AT&T TA noise protection */
#define CLEAR_MEDIAN_FILTER_ERROR

typedef struct
{
	bool median_on_flag;
	bool mferr_setting;
	uint8_t mferr_count;
	uint8_t t46_actvsyncsperx_for_mferr;
	uint8_t t48_mfinvlddiffthr_for_mferr;
	uint8_t t48_mferrorthr_for_mferr;
	uint8_t t48_thr_for_mferr;
	uint8_t t48_movfilter_for_mferr;
}__packed t48_median_config_t;

static t48_median_config_t noise_median = {0};
#endif

void Mxt224_force_released(void);

/* for debug log. use debug level */
extern int checkTSPKEYdebuglevel;
#define KERNEL_SEC_DEBUG_LEVEL_LOW	(0x574F4C44)
#define KERNEL_SEC_DEBUG_LEVEL_MID	(0x44494D44)
#define KERNEL_SEC_DEBUG_LEVEL_HIGH	(0x47494844)

static int read_mem(struct mxt224_data *data, u16 reg, u8 len, u8 *buf)
{
	int ret;
	u16 le_reg = cpu_to_le16(reg);
	struct i2c_msg msg[2] = {
		{
			.addr = data->client->addr,
			.flags = 0,
			.len = 2,
			.buf = (u8 *)&le_reg,
		},
		{
			.addr = data->client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};

	ret = i2c_transfer(data->client->adapter, msg, 2);
	if (ret < 0)
		return ret;

	return ret == 2 ? 0 : -EIO;
}

static int write_mem(struct mxt224_data *data, u16 reg, u8 len, const u8 *buf)
{
	int ret;
	u8 tmp[len + 2];

	put_unaligned_le16(cpu_to_le16(reg), tmp);
	memcpy(tmp + 2, buf, len);

	ret = i2c_master_send(data->client, tmp, sizeof(tmp));

	if (ret < 0)
		return ret;
	/*
	if (reg==(data->cmd_proc + CMD_CALIBRATE_OFFSET))
		printk(KERN_ERR"[TSP] write calibrate_command ret is %d, size is %d\n",ret,sizeof(tmp));
	*/

	return ret == sizeof(tmp) ? 0 : -EIO;
}

static void mxt224_hardware_reset(struct mxt224_data *data)
{
	if (!mxt224_enabled) {
			printk(KERN_ERR"[TSP] mxt224_enabled is 0\n");
			return;
	}

	data->power_off(data);
	msleep(30);
	data->power_on(data);
}

static int __devinit mxt224_reset(struct mxt224_data *data)
{
	u8 buf = 1u;
	return write_mem(data, data->cmd_proc + CMD_RESET_OFFSET, 1, &buf);
}

static int __devinit mxt224_backup(struct mxt224_data *data)
{
	u8 buf = 0x55u;
	return write_mem(data, data->cmd_proc + CMD_BACKUP_OFFSET, 1, &buf);
}

static int get_object_info(struct mxt224_data *data, u8 object_type, u16 *size,
				u16 *address)
{
	int i;

	for (i = 0; i < data->objects_len; i++) {
		if (data->objects[i].object_type == object_type) {
			*size = data->objects[i].size + 1;
			*address = data->objects[i].i2c_address;
			return 0;
		}
	}

	return -ENODEV;
}

static int write_config(struct mxt224_data *data, u8 type, const u8 *cfg)
{
	int ret;
	u16 address = 0;
	u16 size = 0;

	ret = get_object_info(data, type, &size, &address);

	if(size ==0 && address == 0) return 0;
	else return write_mem(data, address, size, cfg);
}


static u32 __devinit crc24(u32 crc, u8 byte1, u8 byte2)
{
	static const u32 crcpoly = 0x80001B;
	u32 res;
	u16 data_word;

	data_word = (((u16)byte2) << 8) | byte1;
	res = (crc << 1) ^ (u32)data_word;

	if (res & 0x1000000)
		res ^= crcpoly;

	return res;
}

static int __devinit calculate_infoblock_crc(struct mxt224_data *data,
							u32 *crc_pointer)
{
	u32 crc = 0;
	u8 mem[7 + data->objects_len * 6];
	int status;
	int i;

	status = read_mem(data, 0, sizeof(mem), mem);

	if (status)
		return status;

	for (i = 0; i < sizeof(mem) - 1; i += 2)
		crc = crc24(crc, mem[i], mem[i + 1]);

	*crc_pointer = crc24(crc, mem[i], 0) & 0x00FFFFFF;

	return 0;
}

/* mxt224E reconfigration */
static void mxt_reconfigration_normal(struct work_struct *work)
{
	int error, id;
	u16 size;

	struct mxt224_data *data = container_of(work, struct mxt224_data, config_dwork.work);
	u16 obj_address = 0;
	disable_irq(data->client->irq);

	for (id = 0 ; id < MAX_USING_FINGER_NUM ; ++id) {
		if ( data->fingers[id].state == MXT224_STATE_INACTIVE )
			continue;
		schedule_delayed_work(&data->config_dwork, HZ*5);
		printk("[TSP] touch pressed!! %s didn't execute!!\n", __func__);
		enable_irq(data->client->irq);
		return;
	}

	get_object_info(data, GEN_ACQUISITIONCONFIG_T8, &size, &obj_address);
	error = write_mem(data, obj_address+8, 1, &data->atchfrccalthr_e);
	if (error < 0) printk(KERN_ERR "[TSP] %s, %d Error!!\n", __func__, __LINE__);
	error = write_mem(data, obj_address+9, 1, &data->atchfrccalratio_e);
	if (error < 0) printk(KERN_ERR "[TSP] %s, %d Error!!\n", __func__, __LINE__);
	enable_irq(data->client->irq);
	return;
}

static unsigned int mxt_time_point;
static unsigned int mxt_time_diff;
static unsigned int mxt_timer_state;
static unsigned int good_check_flag;
static unsigned int not_yet_count;
static u8 cal_check_flag;

uint8_t calibrate_chip(void)
{
	u8 cal_data = 1;
	int ret = 0;
	int ret1 = 0;
	u8 tchautocal_tmp, atchcalst_tmp, atchcalsthr_tmp;
	u16 obj_address = 0;
	u16 size = 1;

/*	printk(KERN_ERR"[TSP]ret is %d,ret1 is %d\n",ret,ret1); */

	if (cal_check_flag == 0) {
		cal_check_flag = 1u;

		ret = get_object_info(copy_data, GEN_ACQUISITIONCONFIG_T8, &size, &obj_address);
		size = 1;

		/* change calibration suspend settings to zero until calibration confirmed good */
		/* store normal settings */
		/* read_mem(copy_data, obj_address+6, (u8)size, &atchcalst); */
		/* read_mem(copy_data, obj_address+7, (u8)size, &atchcalsthr); */

		/* resume calibration must be performed with zero settings */
		tchautocal_tmp = 0;
		atchcalst_tmp = 0;
		atchcalsthr_tmp = 0;

		ret |= write_mem(copy_data, obj_address+6, size, &atchcalst_tmp);
		ret1 = write_mem(copy_data, obj_address+7, size, &atchcalsthr_tmp);
		if (copy_data->family_id == 0x81) { /* mxT224E */
			ret |= write_mem(copy_data, obj_address+8, 1, &atchcalst_tmp);    /* forced cal thr  */
			ret1 |= write_mem(copy_data, obj_address+9, 1, &atchcalsthr_tmp); /* forced cal thr ratio */
		}
	}

	/* send calibration command to the chip */
	if (!ret && !ret1 && !Doing_calibration_flag) {
		/* change calibration suspend settings to zero until calibration confirmed good */
		ret = write_mem(copy_data, copy_data->cmd_proc + CMD_CALIBRATE_OFFSET, 1, &cal_data);
		/* msleep(5); */

		/* set flag for calibration lockup recovery if cal command was successful */
		if (!ret) {
			/* set flag to show we must still confirm if calibration was good or bad */
			Doing_calibration_flag = 1;
			printk(KERN_ERR "[TSP] calibration success!!!\n");
		}
	}
	return ret;
}

static int check_abs_time(void)
{
	if (!mxt_time_point)
		return 0;

	mxt_time_diff = jiffies_to_msecs(jiffies) - mxt_time_point;

	if (mxt_time_diff > 0)
		return 1;
	else
		return 0;
}

static void mxt224_ta_probe(int ta_status)
{
	u16 obj_address = 0;
	u16 size;
	int ret;
	u8 value;
	u8 val = 0;
	u8 noise_threshold;
	u8 movfilter;
	u8 calcfg_dis;
	u8 calcfg_en;
	u8 active_depth;
	u8 charge_time;

	printk(KERN_DEBUG"%s(%d)\n", __func__, ta_status);

	if (!mxt224_enabled) {
		printk(KERN_ERR"[TSP] mxt224_enabled is 0\n");
		return;
	}

	if (copy_data->family_id == 0x81) {
		if (ta_status) {
			threshold = copy_data->tchthr_charging;
			calcfg_dis = copy_data->calcfg_charging_e;
			calcfg_en = copy_data->calcfg_charging_e | 0x20;
			active_depth = 24;
			charge_time = 22;
#ifdef CLEAR_MEDIAN_FILTER_ERROR
			noise_median.mferr_setting = false;
#endif
		} else {
			threshold = copy_data->tchthr_batt;
			calcfg_dis = copy_data->calcfg_batt_e;
			calcfg_en = copy_data->calcfg_batt_e | 0x20;
			active_depth = 20;
			charge_time = 24;
#ifdef CLEAR_MEDIAN_FILTER_ERROR
			noise_median.mferr_count = 0;
			noise_median.mferr_setting = false;
			noise_median.median_on_flag = false;
#endif
		}

		ret = get_object_info(copy_data, SPT_CTECONFIG_T46, &size, &obj_address);
		write_mem(copy_data, obj_address+3, 1, &active_depth);

		/* GEN_ACQUISITIONCONFIG_T8 */
		ret = get_object_info(copy_data, GEN_ACQUISITIONCONFIG_T8, &size, &obj_address);
		write_mem(copy_data, obj_address+0, 1, &charge_time);

		/* PROCG_NOISESUPPRESSION_T48 */
		ret = get_object_info(copy_data, PROCG_NOISESUPPRESSION_T48, &size, &obj_address);
		if (ta_status)
			write_config(copy_data, copy_data->t48_config_chrg_e[0], copy_data->t48_config_chrg_e + 1);
		else
			write_config(copy_data, copy_data->t48_config_batt_e[0], copy_data->t48_config_batt_e + 1);
		read_mem(copy_data, obj_address+2, 1, &val);
		printk(KERN_ERR"[TSP]TA_probe MXT224E T48 Byte2 is %d\n", val);

	} else if (copy_data->family_id == 0x80) {

		if (ta_status) {
			threshold = copy_data->tchthr_charging;
			noise_threshold = copy_data->noisethr_charging;
			movfilter = copy_data->movfilter_charging;
		} else {
			if (boot_or_resume == 1)
				threshold = copy_data->tchthr_batt_init;
			else
				threshold = copy_data->tchthr_batt;
			noise_threshold = copy_data->noisethr_batt;
			movfilter = copy_data->movfilter_batt;
		}
		get_object_info(copy_data, TOUCH_MULTITOUCHSCREEN_T9, &size, &obj_address);
		write_mem(copy_data, obj_address+7, 1, &threshold);
		write_mem(copy_data, obj_address+13, 1, &movfilter);

		get_object_info(copy_data, PROCG_NOISESUPPRESSION_T22, &size, &obj_address);
		write_mem(copy_data, obj_address+8, 1, &noise_threshold);
	}
	printk("[TSP] threshold : %d\n", threshold);
}

void check_chip_calibration(void)
{
	u8 data_buffer[100] = { 0 };
	u8 try_ctr = 0;
	u8 data_byte = 0xF3; /* dianostic command to get touch flags */
	u8 tch_ch = 0, atch_ch = 0;
	u8 check_mask;
	u8 i, j = 0;
	u8 x_line_limit;
	int ret;
	u16 size;
	u16 object_address = 0;
	bool ta_status;

	/* we have had the first touchscreen or face suppression message
	* after a calibration - check the sensor state and try to confirm if
	* cal was good or bad */

	/* get touch flags from the chip using the diagnostic object */
	/* write command to command processor to get touch flags - 0xF3 Command required to do this */
	/* write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte); */
	write_mem(copy_data, copy_data->cmd_proc + CMD_DIAGNOSTIC_OFFSET, 1, &data_byte);


	/* get the address of the diagnostic object so we can get the data we need */
	/* diag_address = get_object_address(DEBUG_DIAGNOSTIC_T37,0); */
	ret = get_object_info(copy_data, DEBUG_DIAGNOSTIC_T37, &size, &object_address);

	msleep(10);

	/* read touch flags from the diagnostic object - clear buffer so the while loop can run first time */
	memset(data_buffer , 0xFF, sizeof(data_buffer));

	/* wait for diagnostic object to update */
	while (!((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00))) {
		/* wait for data to be valid  */
		if (try_ctr > 10) {

			/* Failed! */
			printk(KERN_ERR"[TSP] Diagnostic Data did not update!!\n");
			mxt_timer_state = 0;
			break;
		}

		mdelay(2);
		try_ctr++; /* timeout counter */
		/* read_mem(diag_address, 2,data_buffer); */

		read_mem(copy_data, object_address, 2, data_buffer);
	}


	/* data is ready - read the detection flags */
	/* read_mem(diag_address, 82,data_buffer); */
	read_mem(copy_data, object_address, 82, data_buffer);


	/* data array is 20 x 16 bits for each set of flags, 2 byte header, 40 bytes for touch flags 40 bytes for antitouch flags*/

	/* count up the channels/bits if we recived the data properly */
	if ((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00)) {

		/* mode 0 : 16 x line, mode 1 : 17 etc etc upto mode 4.*/
		/* x_line_limit = 16 + cte_config.mode; */
		x_line_limit = 16 + 3;

		if (x_line_limit > 20) {
			/* hard limit at 20 so we don't over-index the array */
			x_line_limit = 20;
		}

		/* double the limit as the array is in bytes not words */
		x_line_limit = x_line_limit << 1;

		/* count the channels and print the flags to the log */
		for (i = 0; i < x_line_limit; i += 2) { /* check X lines - data is in words so increment 2 at a time */

			/* print the flags to the log - only really needed for debugging */

			/* count how many bits set for this row */
			for (j = 0; j < 8; j++) {
				/* create a bit mask to check against */
				check_mask = 1 << j;

				/* check detect flags */
				if (data_buffer[2+i] & check_mask)
					tch_ch++;

				if (data_buffer[3+i] & check_mask)
					tch_ch++;

				/* check anti-detect flags */
				if (data_buffer[42+i] & check_mask)
					atch_ch++;

				if (data_buffer[43+i] & check_mask)
					atch_ch++;

			}
		}

		//printk(KERN_DEBUG"[TSP] t: %d, a: %d\n", tch_ch, atch_ch);

		/* send page up command so we can detect when data updates next time,
		* page byte will sit at 1 until we next send F3 command */
		data_byte = 0x01;

		/* write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte); */
		write_mem(copy_data, copy_data->cmd_proc + CMD_DIAGNOSTIC_OFFSET, 1, &data_byte);


		/* process counters and decide if we must re-calibrate or if cal was good */
		if ((tch_ch + atch_ch) > 21) {
			printk(KERN_ERR "[TSP]touch ch + anti ch > 21\n");
			calibrate_chip();
			mxt_timer_state = 0;
			mxt_time_point = jiffies_to_msecs(jiffies);
			not_yet_count = 0;
		} else if (tch_ch > 17) {
			printk(KERN_ERR "[TSP]touch ch > 17\n");
			calibrate_chip();
			mxt_timer_state = 0;
			mxt_time_point = jiffies_to_msecs(jiffies);
			not_yet_count = 0;
		} else if ((tch_ch > 0) && (atch_ch == 0)) {
			/* cal was good - don't need to check any more */
			not_yet_count = 0;

			if ((auto_cal_flag == 0)&&(copy_data->family_id == 0x80)) {
				data_byte = 5;
				ret = get_object_info(copy_data, GEN_ACQUISITIONCONFIG_T8, &size, &object_address);
				write_mem(copy_data, object_address+4, 1, &data_byte);	/* TCHAUTOCAL 1sec */
				printk(KERN_DEBUG"[TSP] auto calibration enable state is %d\n",data_byte);
				auto_cal_flag = 1;
			}
			if (!check_abs_time())
				mxt_time_diff = 301;

			if (mxt_timer_state == 1) {
				if (mxt_time_diff > 300) {
					printk("[TSP] calibration was good\n");
					cal_check_flag = 0;
					good_check_flag = 0;
					mxt_timer_state = 0;
					auto_cal_flag = 0;
					mxt_time_point = jiffies_to_msecs(jiffies);

					ret = get_object_info(copy_data, GEN_ACQUISITIONCONFIG_T8, &size, &object_address);

					/* change calibration suspend settings to zero until calibration confirmed good */
					/* store normal settings */
					write_mem(copy_data, object_address+6, 1, &copy_data->atchcalst);
					write_mem(copy_data, object_address+7, 1, &copy_data->atchcalsthr);

					if(copy_data->family_id == 0x81) {
						write_mem(copy_data, object_address+8, 1, &copy_data->atchfrccalthr_e);
						write_mem(copy_data, object_address+9, 1, &copy_data->atchfrccalratio_e);
					}

					if ((copy_data->read_ta_status)&&(boot_or_resume == 1)&&(copy_data->family_id == 0x80)) {
						boot_or_resume = 0;
						copy_data->read_ta_status(&ta_status);
						printk(KERN_DEBUG "[TSP] ta_status is %d\n", ta_status);

						if (ta_status == 0)
							mxt224_ta_probe(ta_status);
					}
				} else  {
					cal_check_flag = 1;
				}
			} else {
				mxt_timer_state = 1;
				mxt_time_point = jiffies_to_msecs(jiffies);
				cal_check_flag = 1;
			}

		} else if (atch_ch >= 5) {
			/* cal was bad - must recalibrate and check afterwards */
			printk(KERN_ERR"[TSP] calibration was bad\n");
			calibrate_chip();
			mxt_timer_state = 0;
			not_yet_count = 0;
			mxt_time_point = jiffies_to_msecs(jiffies);
		} else {
			/* we cannot confirm if good or bad - we must wait for next touch  message to confirm */
			printk(KERN_DEBUG"[TSP] calibration was not decided yet\n");
			cal_check_flag = 1u;
			mxt_timer_state = 0;
			mxt_time_point = jiffies_to_msecs(jiffies);
			not_yet_count++;
			if (not_yet_count > 10) {
				not_yet_count = 0;
				calibrate_chip();
			}
		}
	}
}

#if defined(DRIVER_FILTER)
static void equalize_coordinate(bool detect, u8 id, u16 *px, u16 *py)
{
	static int tcount[MAX_USING_FINGER_NUM] = { 0, };
	static u16 pre_x[MAX_USING_FINGER_NUM][4] = {{0}, };
	static u16 pre_y[MAX_USING_FINGER_NUM][4] = {{0}, };
	int coff[4] = {0,};
	int distance = 0;

	if (detect)
	{
		tcount[id] = 0;
	}

	pre_x[id][tcount[id]%4] = *px;
	pre_y[id][tcount[id]%4] = *py;

	if (tcount[id] > 3)
	{
		{
			distance = abs(pre_x[id][(tcount[id]-1)%4] - *px) + abs(pre_y[id][(tcount[id]-1)%4] - *py);

			coff[0] = (u8)(2 + distance/5);
			if (coff[0] < 8) {
				coff[0] = max(2, coff[0]);
				coff[1] = min((8 - coff[0]), (coff[0]>>1)+1);
				coff[2] = min((8 - coff[0] - coff[1]), (coff[1]>>1)+1);
				coff[3] = 8 - coff[0] - coff[1] - coff[2];

				/* printk(KERN_DEBUG "[TSP] %d, %d, %d, %d", coff[0], coff[1], coff[2], coff[3]); */

				*px = (u16)((*px*(coff[0]) + pre_x[id][(tcount[id]-1)%4]*(coff[1])
					+ pre_x[id][(tcount[id]-2)%4]*(coff[2]) + pre_x[id][(tcount[id]-3)%4]*(coff[3]))/8);
				*py = (u16)((*py*(coff[0]) + pre_y[id][(tcount[id]-1)%4]*(coff[1])
					+ pre_y[id][(tcount[id]-2)%4]*(coff[2]) + pre_y[id][(tcount[id]-3)%4]*(coff[3]))/8);
			} else {
				*px = (u16)((*px*4 + pre_x[id][(tcount[id]-1)%4])/5);
				*py = (u16)((*py*4 + pre_y[id][(tcount[id]-1)%4])/5);
			}
		}
	}
	tcount[id]++;
}
#endif  /* DRIVER_FILTER */

static int __devinit mxt224_init_touch_driver(struct mxt224_data *data)
{
	struct object_t *object_table;
	u32 read_crc = 0;
	u32 calc_crc;
	u16 crc_address;
	u16 dummy;
	int i;
	u8 id[ID_BLOCK_SIZE];
	int ret;
	u8 type_count = 0;
	u8 tmp;

	ret = read_mem(data, 0, sizeof(id), id);
	if (ret)
		return ret;

	dev_info(&data->client->dev, "family = %#02x, variant = %#02x, version "
			"= %#02x, build = %d\n", id[0], id[1], id[2], id[3]);
	printk(KERN_ERR"family = %#02x, variant = %#02x, version "
			"= %#02x, build = %d\n", id[0], id[1], id[2], id[3]);
	dev_dbg(&data->client->dev, "matrix X size = %d\n", id[4]);
	dev_dbg(&data->client->dev, "matrix Y size = %d\n", id[5]);

	data->family_id = id[0];
	data->tsp_version = id[2];
	data->tsp_build = id[3];
	data->objects_len = id[6];

	object_table = kmalloc(data->objects_len * sizeof(*object_table),
				GFP_KERNEL);
	if (!object_table)
		return -ENOMEM;

	ret = read_mem(data, OBJECT_TABLE_START_ADDRESS,
			data->objects_len * sizeof(*object_table),
			(u8 *)object_table);
	if (ret)
		goto err;

	for (i = 0; i < data->objects_len; i++) {
		object_table[i].i2c_address = le16_to_cpu(object_table[i].i2c_address);
		tmp = 0;
		if (object_table[i].num_report_ids) {
			tmp = type_count + 1;
			type_count += object_table[i].num_report_ids *
						(object_table[i].instances + 1);
		}
		switch (object_table[i].object_type) {
		case TOUCH_MULTITOUCHSCREEN_T9:
			data->finger_type = tmp;
			dev_dbg(&data->client->dev, "Finger type = %d\n",
						data->finger_type);
			break;
		case GEN_MESSAGEPROCESSOR_T5:
			data->msg_object_size = object_table[i].size + 1;
			dev_dbg(&data->client->dev, "Message object size = "
						"%d\n", data->msg_object_size);
			break;
		}
	}

	data->objects = object_table;

	/* Verify CRC */
	crc_address = OBJECT_TABLE_START_ADDRESS +
			data->objects_len * OBJECT_TABLE_ELEMENT_SIZE;

#ifdef __BIG_ENDIAN
#error The following code will likely break on a big endian machine
#endif
	ret = read_mem(data, crc_address, 3, (u8 *)&read_crc);
	if (ret)
		goto err;

	read_crc = le32_to_cpu(read_crc);

	ret = calculate_infoblock_crc(data, &calc_crc);
	if (ret)
		goto err;

	if (read_crc != calc_crc) {
		dev_err(&data->client->dev, "CRC error\n");
		ret = -EFAULT;
		goto err;
	}

	ret = get_object_info(data, GEN_MESSAGEPROCESSOR_T5, &dummy,
					&data->msg_proc);
	if (ret)
		goto err;

	ret = get_object_info(data, GEN_COMMANDPROCESSOR_T6, &dummy,
					&data->cmd_proc);
	if (ret)
		goto err;

	return 0;

err:
	kfree(object_table);
	return ret;
}

static void report_input_data(struct mxt224_data *data)
{
	int i;
	int count = 0;

	for (i = 0; i < data->num_fingers; i++) {
		if (data->fingers[i].state == MXT224_STATE_INACTIVE)
			continue;

#if 0
		input_report_abs(data->input_dev, ABS_MT_POSITION_X,
				data->fingers[i].x);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
				data->fingers[i].y);
		input_report_abs(data->input_dev, ABS_MT_PRESSURE,
				data->fingers[i].z);
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
				data->fingers[i].w);
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, i);

		input_mt_sync(data->input_dev);
#else
// N1_ICS
		if (data->fingers[i].state == MXT224_STATE_RELEASE) {
			input_mt_slot(data->input_dev, i);
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, false);
		}
		else {
			input_mt_slot(data->input_dev, i);
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, true);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					data->fingers[i].x);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					data->fingers[i].y);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE,
					data->fingers[i].z);
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
					data->fingers[i].w);
			count++;
		}
#endif

#if 0
		if ((checkTSPKEYdebuglevel == KERNEL_SEC_DEBUG_LEVEL_MID) ||
			(checkTSPKEYdebuglevel == KERNEL_SEC_DEBUG_LEVEL_HIGH)) {
			if (data->fingers[i].state == MXT224_STATE_PRESS) {
				printk(KERN_DEBUG "[TSP][P] id[%d],x=%d,y=%d,z=%d,w=%d\n",
					i , data->fingers[i].x, data->fingers[i].y,
					data->fingers[i].z, data->fingers[i].w);
			} else if (data->fingers[i].state == MXT224_STATE_RELEASE) {
				printk(KERN_DEBUG "[TSP][U] id[%d],x=%d,y=%d,z=%d,w=%d\n",
					i , data->fingers[i].x, data->fingers[i].y,
					data->fingers[i].z, data->fingers[i].w);
			}
		}
#else
		if (data->fingers[i].state == MXT224_STATE_PRESS) {
			printk(KERN_DEBUG "[TSP][%d] press\n", i);
		} else if (data->fingers[i].state == MXT224_STATE_RELEASE) {
			printk(KERN_DEBUG "[TSP][%d] release\n", i);
		}
#endif

	if (data->fingers[i].state == MXT224_STATE_RELEASE) {
		data->fingers[i].state = MXT224_STATE_INACTIVE;
	} else {
		data->fingers[i].state = MXT224_STATE_MOVE;
		count++;
	}

	}
	input_sync(data->input_dev);

	if (count)
		touch_is_pressed = 1;
	else
		touch_is_pressed = 0;

	data->finger_mask = 0;
}

#if 1 /* eplus andy add 20111028 */
/* 110926 zero charger & palne noise apply */
static void median_err_setting(void)
{
	u16 obj_address;
	u16 size;
	u8 value, state;
	bool ta_status_check;
	int ret = 0;

	copy_data->read_ta_status(&ta_status_check);

	if (ta_status_check == 0) {
		ret = get_object_info(copy_data, SPT_CTECONFIG_T46, &size, &obj_address);
		value = 32;
		write_mem(copy_data, obj_address+3, 1, &value);
		ret = get_object_info(copy_data, PROCG_NOISESUPPRESSION_T48, &size, &obj_address);
		value = 40;
		write_mem(copy_data, obj_address+35, 1, &value);
	} else {

		value = 1;
		if (noise_median.mferr_count < 3)
			noise_median.mferr_count++;

		if (!(noise_median.mferr_count%value)&&(noise_median.mferr_count < 3)) {
			printk(KERN_DEBUG"[TSP] median thr noise level too high. %d\n", noise_median.mferr_count/value);
			state = noise_median.mferr_count/value;

			get_object_info(copy_data, PROCG_NOISESUPPRESSION_T48, &size, &obj_address);
			if (state == 1) {
				value = noise_median.t48_mfinvlddiffthr_for_mferr;
				write_mem(copy_data, obj_address+22, 1, &value);
				value = noise_median.t48_mferrorthr_for_mferr;
				write_mem(copy_data, obj_address+25, 1, &value);
				value = noise_median.t48_thr_for_mferr;
				write_mem(copy_data, obj_address+35, 1, &value);
				value = noise_median.t48_movfilter_for_mferr;
				write_mem(copy_data, obj_address+39, 1, &value);
				get_object_info(copy_data, SPT_CTECONFIG_T46, &size, &obj_address);
				value = noise_median.t46_actvsyncsperx_for_mferr;
				write_mem(copy_data, obj_address+3, 1, &value);
			} else if (state >= 2) {
				value = 7; /* basefreq */
				write_mem(copy_data, obj_address+3, 1, &value);
				value = 1; /* secondmf */
				write_mem(copy_data, obj_address+8, 1, &value);
				value = 2; /* thirdmf */
				write_mem(copy_data, obj_address+9, 1, &value);
				value = 20; /* mfinvlddiffthr */
				write_mem(copy_data, obj_address+22, 1, &value);
				value = 38; /* mferrorthr */
				write_mem(copy_data, obj_address+25, 1, &value);
				value = 45; /* thr */
				write_mem(copy_data, obj_address+35, 1, &value);
				value = 64; /* movfilter */
				write_mem(copy_data, obj_address+39, 1, &value);
				get_object_info(copy_data, SPT_CTECONFIG_T46, &size, &obj_address);
				value = 63; /* actvsyncsperx */
				write_mem(copy_data, obj_address+3, 1, &value);
			}
		}
	}
	noise_median.mferr_setting = true;
}
#endif

static irqreturn_t mxt224_irq_thread(int irq, void *ptr)
{
	struct mxt224_data *data = ptr;
	int id;
	u8 msg[data->msg_object_size];
	u8 touch_message_flag = 0;
	u16 obj_address = 0;
	u16 size;
	u8 value;
	int error;
	int reset = 0;
	bool ta_status = 0;

	if (mxt224_enabled == 0) {
		printk("[TSP] mxt224_enabled == 0. do not excute i2c read\n");
		return IRQ_HANDLED;
	}

	touch_message_flag = 0;
	if (read_mem(data, data->msg_proc, sizeof(msg), msg)) {
		Mxt224_force_released();
		printk(KERN_ERR"[TSP]%s read_mem fail. hardware reset\n", __func__);
		mxt224_hardware_reset(data);
		msleep(80);

#ifdef CLEAR_MEDIAN_FILTER_ERROR
		noise_median.mferr_count = 0;
		noise_median.mferr_setting = false;
		noise_median.median_on_flag = false;
#endif
		if (data->read_ta_status) {
			data->read_ta_status(&ta_status);
			printk("[TSP] %s, ta_status = %d\n", __func__, ta_status);
			if (ta_status)
				mxt224_ta_probe(ta_status);
		}
		calibrate_chip();
		return IRQ_HANDLED;
	}
	if (msg[0] == 0x1) {
		if (msg[1] == 0x00) { /* normal mode */
			Doing_calibration_flag = 0;
			printk("[TSP] normal mode\n");
			if (cal_check_flag == 1) {
				mxt_timer_state = 0;
				mxt_time_point = jiffies_to_msecs(jiffies);
			}
		}
		if ((msg[1]&0x04) == 0x04) /* I2C checksum error */
			printk("[TSP] I2C checksum error\n");

		if ((msg[1]&0x08) == 0x08) { /* config error */
			printk("[TSP] config error\n");
			reset = 1;
		}
		if ((msg[1]&0x10) == 0x10) { /* calibration */
			printk("[TSP] calibration is on going\n");
			Doing_calibration_flag = 1;
		}
		if ((msg[1]&0x20) == 0x20) {/* signal error */
			printk("[TSP] signal error\n");
			reset = 1;
		}
		if ((msg[1]&0x40) == 0x40) /* overflow */
			printk("[TSP] overflow detected\n");

		if ((msg[1]&0x80) == 0x80) /* reset */
			printk("[TSP] reset is ongoing\n");

		if (reset) {
			Mxt224_force_released();
			error = mxt224_reset(data);
			if (error) {
				printk(KERN_ERR"[TSP]mxt224_reset fail. hardware reset\n");
				mxt224_hardware_reset(data);
				msleep(80);
			} else
				msleep(120);

#ifdef CLEAR_MEDIAN_FILTER_ERROR
			noise_median.mferr_count = 0;
			noise_median.mferr_setting = false;
			noise_median.median_on_flag = false;
#endif

			if (data->read_ta_status) {
				data->read_ta_status(&ta_status);
				printk("[TSP] %s, ta_status = %d\n", __func__, ta_status);
				if (ta_status)
					mxt224_ta_probe(ta_status);
			}
			calibrate_chip();
			printk(KERN_ERR"[TSP] touch error reset done\n");
			return IRQ_HANDLED;
		}
	}

	if ( msg[0] == 14) {
		if ((msg[1] & 0x01) == 0x00) { /* Palm release */
			printk("[TSP] palm touch released\n");
			touch_is_pressed = 0;
		} else if ((msg[1] & 0x01) == 0x01) { /* Palm Press */
			printk("[TSP] palm touch detected\n");
			touch_is_pressed = 1;
			touch_message_flag = 1;
		}
	}

	if ((data->family_id == 0x80) && (msg[0] == 0xf)) { /* freq error release */
		printk(KERN_ERR"[TSP] Starting irq with 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x",
			msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7]);
		if ((msg[1]&0x08) == 0x08)
			calibrate_chip();
	}

	if ((data->family_id == 0x81) && (msg[0] == 18)) {
		printk(KERN_ERR"[TSP] median noise msg : 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x",
			msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7]);

		if ((msg[4] & 0x5) == 0x5) {
			printk(KERN_ERR"[TSP] median filter state error!!!\n");
			median_err_setting();
		} else if ((msg[4] & 0x4) == 0x4) {
			if (data->read_ta_status)
				data->read_ta_status(&ta_status);
			if ((!ta_status) && (noise_median.mferr_setting == false) &&
					(noise_median.median_on_flag == false)) {
				printk(KERN_ERR"[TSP] median filter ON!!!\n");
				error = get_object_info(data, TOUCH_MULTITOUCHSCREEN_T9, &size, &obj_address);
				value = 0;
				write_mem(data, obj_address+34, 1, &value);
				noise_median.median_on_flag = true;
			}
		}
	}

	if (msg[0] > 1 && msg[0] < 12) {

		id = msg[0] - data->finger_type;

		if (data->family_id == 0x80) {	/*  : MXT-224 */
			if ( (data->fingers[id].state >= MXT224_STATE_PRESS)
					&& msg[1] & PRESS_MSG_MASK ) {
				printk("[TSP] calibrate on ghost touch\n");
				calibrate_chip();
			}
		}

		if (msg[1] & RELEASE_MSG_MASK) {
			data->fingers[id].z = 0;
			data->fingers[id].w = msg[5];
			data->finger_mask |= 1U << id;
			data->fingers[id].state = MXT224_STATE_RELEASE;
		} else if ((msg[1] & DETECT_MSG_MASK) && (msg[1] &
				(PRESS_MSG_MASK | MOVE_MSG_MASK))) {
			touch_message_flag = 1;
			data->fingers[id].z = msg[6];
			data->fingers[id].w = msg[5];
			data->fingers[id].x = ((msg[2] << 4) | (msg[4] >> 4)) >>
							data->x_dropbits;
			data->fingers[id].y = ((msg[3] << 4) |
					(msg[4] & 0xF)) >> data->y_dropbits;
			data->finger_mask |= 1U << id;
#if defined(DRIVER_FILTER)
			if (msg[1] & PRESS_MSG_MASK) {
				equalize_coordinate(1, id, &data->fingers[id].x, &data->fingers[id].y);
				data->fingers[id].state = MXT224_STATE_PRESS;
			}
			else if (msg[1] & MOVE_MSG_MASK) {
				equalize_coordinate(0, id, &data->fingers[id].x, &data->fingers[id].y);
			}
#endif
		} else if ((msg[1] & SUPPRESS_MSG_MASK)
				&& (data->fingers[id].state != MXT224_STATE_INACTIVE)) {
			data->fingers[id].z = 0;
			data->fingers[id].w = msg[5];
			data->fingers[id].state = MXT224_STATE_RELEASE;
			data->finger_mask |= 1U << id;
		} else {
			dev_dbg(&data->client->dev, "Unknown state %#02x %#02x\n", msg[0], msg[1]);
		}
	}

	if (data->finger_mask)
		report_input_data(data);

	if (touch_message_flag && (cal_check_flag) &&
		!Doing_calibration_flag)
		check_chip_calibration();

	return IRQ_HANDLED;
}

static void mxt224_deepsleep(struct mxt224_data *data)
{
	u8 power_cfg[3] = {0, };
	write_config(data, GEN_POWERCONFIG_T7, power_cfg);
	tsp_deepsleep = 1;
}

static void mxt224_wakeup(struct mxt224_data *data)
{
	write_config(data, GEN_POWERCONFIG_T7, data->power_cfg);
}

static int mxt224_internal_suspend(struct mxt224_data *data)
{
	int i;
	int count = 0;

	if (data->family_id == 0x81) {	/*  : MXT-224E */
		cancel_delayed_work_sync(&data->config_dwork);
	}

	for (i = 0; i < data->num_fingers; i++) {
		if (data->fingers[i].state == MXT224_STATE_INACTIVE)
			continue;
		data->fingers[i].z = 0;
		data->fingers[i].state = MXT224_STATE_RELEASE;
		count++;
	}
	if (count)
		report_input_data(data);

	if (!tsp_deepsleep)
		data->power_off(data);

	return 0;
}

static int mxt224_internal_resume(struct mxt224_data *data)
{
	if (tsp_deepsleep)
		mxt224_wakeup(data);
	else
		data->power_on(data);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
#define mxt224_suspend	NULL
#define mxt224_resume	NULL

static void mxt224_early_suspend(struct early_suspend *h)
{
	struct mxt224_data *data = container_of(h, struct mxt224_data,
								early_suspend);

	printk(KERN_DEBUG"[TSP] %s\n", __func__);
	if (data->read_ta_status)
		data->read_ta_status(&suspend_ta_status);

	if (suspend_ta_status) {
		mxt224_deepsleep(data);
	} else {
		msleep(10);
		mxt224_enabled = 0;
	}
	disable_irq(data->client->irq);
	mxt224_internal_suspend(data);
	touch_is_pressed = 0;
	Doing_calibration_flag = 0;
	mxt_timer_state = 0;
}

static void mxt224_late_resume(struct early_suspend *h)
{
	struct mxt224_data *data = container_of(h, struct mxt224_data,
								early_suspend);
	bool ta_status = 0;

	printk(KERN_DEBUG"[TSP] %s\n", __func__);
	mxt224_internal_resume(data);
	mxt224_enabled = 1;

	if (data->read_ta_status) {
		data->read_ta_status(&ta_status);
		printk(KERN_DEBUG"[TSP] ta_status is %d\n", ta_status);
		mxt224_ta_probe(ta_status);
	}
	suspend_ta_status = ta_status;
	if (data->family_id == 0x80)
		boot_or_resume = 1;
	calibrate_chip();
	enable_irq(data->client->irq);

#ifdef CLEAR_MEDIAN_FILTER_ERROR
	noise_median.mferr_count = 0;
	noise_median.mferr_setting = false;
	noise_median.median_on_flag = false;
#endif

	tsp_deepsleep = 0;
}
#else
static int mxt224_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt224_data *data = i2c_get_clientdata(client);

	mxt224_enabled = 0;
	touch_is_pressed = 0;
	return mxt224_internal_suspend(data);
}

static int mxt224_resume(struct device *dev)
{
	int ret = 0;
	bool ta_status = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt224_data *data = i2c_get_clientdata(client);

	ret = mxt224_internal_resume(data);

	mxt224_enabled = 1;

	if (data->read_ta_status) {
		data->read_ta_status(&ta_status);
		printk("[TSP] ta_status is %d\n", ta_status);
		mxt224_ta_probe(ta_status);
	}
	return ret;
}
#endif

void Mxt224_force_released(void)
{
	struct mxt224_data *data = copy_data;
	int i;

	if (!mxt224_enabled) {
		printk(KERN_ERR"[TSP] mxt224_enabled is 0\n");
		return;
	}

	for (i = 0; i < data->num_fingers; i++) {
		if (data->fingers[i].state == MXT224_STATE_INACTIVE)
			continue;
		data->fingers[i].z = 0;
		data->fingers[i].state = MXT224_STATE_RELEASE;
	}
	printk("[TSP] %s\n", __func__);
	report_input_data(data);
}
/* EXPORT_SYMBOL(Mxt224_force_released); */

static ssize_t mxt224_debug_setting(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	g_debug_switch = !g_debug_switch;
	return 0;
}

static ssize_t mxt224_object_setting(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct mxt224_data *data = dev_get_drvdata(dev);
	unsigned int object_type;
	unsigned int object_register;
	unsigned int register_value;
	u8 value;
	u8 val;
	int ret;
	u16 address;
	u16 size;
	sscanf(buf, "%u%u%u", &object_type, &object_register, &register_value);
	printk(KERN_ERR "[TSP] object type T%d", object_type);
	printk(KERN_ERR "[TSP] object register ->Byte%d\n", object_register);
	printk(KERN_ERR "[TSP] register value %d\n", register_value);
	ret = get_object_info(data, (u8)object_type, &size, &address);
	if (ret) {
		printk(KERN_ERR "[TSP] fail to get object_info\n");
		return count;
	}

	size = 1;
	value = (u8)register_value;
	write_mem(data, address+(u16)object_register, size, &value);
	read_mem(data, address+(u16)object_register, (u8)size, &val);

	printk(KERN_ERR "[TSP] T%d Byte%d is %d\n", object_type, object_register, val);
	return count;
}

static ssize_t mxt224_object_show(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct mxt224_data *data = dev_get_drvdata(dev);
	unsigned int object_type;
	u8 val;
	int ret;
	u16 address;
	u16 size;
	u16 i;
	sscanf(buf, "%u", &object_type);
	printk("[TSP] object type T%d\n", object_type);
	ret = get_object_info(data, (u8)object_type, &size, &address);
	if (ret) {
		printk(KERN_ERR "[TSP] fail to get object_info\n");
		return count;
	}
	for (i = 0; i < size; i++) {
		read_mem(data, address+i, 1, &val);
		printk("[TSP] Byte %u --> %u\n", i, val);
	}
	return count;
}

struct device *sec_touchscreen;
static u8 firmware_latest[] ={0x16, 0x10}; /* mxt224 : 0x16, mxt224E : 0x10 */
static u8 build_latest[] = {0xAB, 0xAA};

struct device *mxt224_noise_test;
/*
	botton_right, botton_left, center, top_right, top_left
*/
u16 test_node[5] = { 196, 188, 104, 20, 12};


void diagnostic_chip(u8 mode)
{
	int error;
	u16 t6_address = 0;
	u16 size_one;
	int ret;
	u8 value;
	u16 t37_address = 0;

	ret = get_object_info(copy_data, GEN_COMMANDPROCESSOR_T6, &size_one, &t6_address);

	size_one = 1;
	error = write_mem(copy_data, t6_address+5, (u8)size_one, &mode);
		/* qt602240_write_object(p_qt602240_data, QT602240_GEN_COMMAND, */
		/* QT602240_COMMAND_DIAGNOSTIC, mode); */
	if (error < 0) {
		printk(KERN_ERR "[TSP] error %s: write_object\n", __func__);
	} else {
		get_object_info(copy_data, DEBUG_DIAGNOSTIC_T37, &size_one, &t37_address);
		size_one = 1;
		/* printk(KERN_ERR"diagnostic_chip setting success\n"); */
		read_mem(copy_data, t37_address, (u8)size_one, &value);
		/* printk(KERN_ERR"dianostic_chip mode is %d\n",value); */
	}
}

uint8_t read_uint16_t(struct mxt224_data *data, uint16_t address, uint16_t *buf )
{
	uint8_t status;
	uint8_t temp[2];

	status = read_mem(data, address, 2, temp);
	*buf= ((uint16_t)temp[1]<<8)+ (uint16_t)temp[0];

	return (status);
}

void read_dbg_data(uint8_t dbg_mode , uint8_t node, uint16_t *dbg_data)
{
	u8 read_page, read_point;
	uint8_t mode,page;
	u16 size;
	u16 diagnostic_addr = 0;

	get_object_info(copy_data, DEBUG_DIAGNOSTIC_T37, &size, &diagnostic_addr);

	read_page = node / 64;
	node %= 64;
	read_point = (node * 2) + 2;

	/* Page Num Clear */
	diagnostic_chip(MXT_CTE_MODE);
	msleep(10);

	do {
		if(read_mem(copy_data, diagnostic_addr, 1, &mode))
		{
			printk(KERN_INFO "[TSP] READ_MEM_FAILED \n");
			return;
		}
	} while(mode != MXT_CTE_MODE);

	diagnostic_chip(dbg_mode);
	msleep(10);

	do {
		if(read_mem(copy_data, diagnostic_addr, 1, &mode))
		{
			printk(KERN_INFO "[TSP] READ_MEM_FAILED \n");
			return;
		}
	} while(mode != dbg_mode);

    for(page = 1; page <= read_page;page++)
	{
		diagnostic_chip(MXT_PAGE_UP);
		msleep(10);
		do {
			if(read_mem(copy_data, diagnostic_addr + 1, 1, &mode))
			{
				printk(KERN_INFO "[TSP] READ_MEM_FAILED \n");
				return;
			}
		} while(mode != page);
	}

	if(read_uint16_t(copy_data, diagnostic_addr + read_point, dbg_data))
	{
		printk(KERN_INFO "[TSP] READ_MEM_FAILED \n");
		return;
	}
}

static int mxt224_check_bootloader(struct i2c_client *client,
					unsigned int state)
{
	u8 val;
	msleep(10);  /* recommendation from ATMEL */
recheck:
	if (i2c_master_recv(client, &val, 1) != 1) {
		dev_err(&client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	switch (state) {
	case MXT224_WAITING_BOOTLOAD_CMD:
	case MXT224_WAITING_FRAME_DATA:
		val &= ~MXT224_BOOT_STATUS_MASK;
		break;
	case MXT224_FRAME_CRC_PASS:
		if (val == MXT224_FRAME_CRC_CHECK)
			goto recheck;
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		dev_err(&client->dev, "Unvalid bootloader mode state\n");
		printk(KERN_ERR "[TSP] Unvalid bootloader mode state\n");
		return -EINVAL;
	}

	return 0;
}

static int mxt224_unlock_bootloader(struct i2c_client *client)
{
	u8 buf[2];

	buf[0] = MXT224_UNLOCK_CMD_LSB;
	buf[1] = MXT224_UNLOCK_CMD_MSB;

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int mxt224_fw_write(struct i2c_client *client,
				const u8 *data, unsigned int frame_size)
{
	if (i2c_master_send(client, data, frame_size) != frame_size) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int mxt224_load_fw(struct device *dev, const char *fn)
{
	struct mxt224_data *data = copy_data;
	struct i2c_client *client = copy_data->client;
	unsigned int frame_size;
	unsigned int pos = 0;
	int ret;
	u16 obj_address=0;
	u16 size_one;
	u8 value;
	unsigned int object_register;

#if READ_FW_FROM_HEADER
	struct firmware *fw = NULL;
	printk("[TSP] mxt224_load_fw start!!!\n");

	fw = kzalloc(sizeof(struct firmware), GFP_KERNEL);
	fw->data = firmware_latest_ver;
	fw->size = sizeof(firmware_latest_ver);
#else
	const struct firmware *fw = NULL;
	printk("[TSP] mxt224_load_fw start!!!\n");

	ret = request_firmware(&fw, fn, &client->dev);
	if (ret) {
		dev_err(dev, "Unable to open firmware %s\n", fn);
		printk(KERN_ERR "[TSP] Unable to open firmware %s\n", fn);
		return ret;
	}
#endif
	/* Change to the bootloader mode */
	/* mxt224_write_object(data, MXT224_GEN_COMMAND, MXT224_COMMAND_RESET, MXT224_BOOT_VALUE); */
	object_register = 0;
	value = (u8)MXT224_BOOT_VALUE;
	ret = get_object_info(data, GEN_COMMANDPROCESSOR_T6, &size_one, &obj_address);
	if (ret)
	{
		printk(KERN_ERR"[TSP] fail to get object_info\n");
		return ret;
	}
	size_one = 1;
	write_mem(data, obj_address+(u16)object_register, (u8)size_one, &value);
	msleep(MXT224_RESET_TIME);

	/* Change to slave address of bootloader */
	if (client->addr == MXT224_APP_LOW)
		client->addr = MXT224_BOOT_LOW;
	else
		client->addr = MXT224_BOOT_HIGH;
	printk(KERN_ERR "[TSP]change mxt224 address = 0x%x\n", client->addr);

	ret = mxt224_check_bootloader(client, MXT224_WAITING_BOOTLOAD_CMD);
	if (ret)
		goto out;

	/* Unlock bootloader */
	mxt224_unlock_bootloader(client);

	pr_info("Updating progress: ");
	pos += 2;
	while (pos < fw->size) {
		ret = mxt224_check_bootloader(client,
						MXT224_WAITING_FRAME_DATA);
		if (ret)
			goto out;

		frame_size = ((*(fw->data + pos) << 8) | *(fw->data + pos + 1));

		/* We should add 2 at frame size as the the firmware data is not
		* included the CRC bytes.
		*/
		frame_size += 2;

		/* Write one frame to device */
		/* mxt224_fw_write(client, fw->data + pos, frame_size); */
		mxt224_fw_write(client, fw->data + pos, frame_size);

		ret = mxt224_check_bootloader(client,
						MXT224_FRAME_CRC_PASS);
		if (ret)
			goto out;
		printk(KERN_ERR "[TSP]mxt224_check_bootloader MXT224_FRAME_CRC_PASS\n");
		pos += frame_size;

		dev_dbg(dev, "Updated %d bytes / %zd bytes\n", pos, fw->size);
		printk("[TSP] Updated %d bytes / %zd bytes\n", pos, fw->size);
	}

out:
#if READ_FW_FROM_HEADER
	kfree(fw);
#else
	release_firmware(fw);
#endif

	/* Change to slave address of application */
	if (client->addr == MXT224_BOOT_LOW)
		client->addr = MXT224_APP_LOW;
	else
		client->addr = MXT224_APP_HIGH;

	return ret;
}

static ssize_t set_refer0_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    uint16_t mxt_reference=0;
    read_dbg_data(MXT_REFERENCE_MODE, test_node[0],&mxt_reference);
    return sprintf(buf, "%u\n", mxt_reference);
}

static ssize_t set_refer1_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_reference=0;
	read_dbg_data(MXT_REFERENCE_MODE, test_node[1], &mxt_reference);
	return sprintf(buf, "%u\n", mxt_reference);
}

static ssize_t set_refer2_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_reference=0;
	read_dbg_data(MXT_REFERENCE_MODE, test_node[2], &mxt_reference);
	return sprintf(buf, "%u\n", mxt_reference);
}

static ssize_t set_refer3_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_reference=0;
	read_dbg_data(MXT_REFERENCE_MODE, test_node[3], &mxt_reference);
	return sprintf(buf, "%u\n", mxt_reference);
}

static ssize_t set_refer4_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_reference=0;
	read_dbg_data(MXT_REFERENCE_MODE, test_node[4], &mxt_reference);
	return sprintf(buf, "%u\n", mxt_reference);
}

static ssize_t set_delta0_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_delta=0;
	read_dbg_data(MXT_DELTA_MODE, test_node[0], &mxt_delta);
	if (mxt_delta < 32767)
		return sprintf(buf, "%u\n", mxt_delta);
	else
		mxt_delta = 65535 - mxt_delta;
	return sprintf(buf, "-%u\n", mxt_delta);
}

static ssize_t set_delta1_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_delta=0;
	read_dbg_data(MXT_DELTA_MODE, test_node[1], &mxt_delta);
	if (mxt_delta < 32767)
		return sprintf(buf, "%u\n", mxt_delta);
	else
		mxt_delta = 65535 - mxt_delta;
	return sprintf(buf, "-%u\n", mxt_delta);

}

static ssize_t set_delta2_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_delta=0;
	read_dbg_data(MXT_DELTA_MODE, test_node[2], &mxt_delta);
	if (mxt_delta < 32767)
		return sprintf(buf, "%u\n", mxt_delta);
	else
		mxt_delta = 65535 - mxt_delta;
	return sprintf(buf, "-%u\n", mxt_delta);
}

static ssize_t set_delta3_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_delta=0;
	read_dbg_data(MXT_DELTA_MODE, test_node[3], &mxt_delta);
	if (mxt_delta < 32767)
		return sprintf(buf, "%u\n", mxt_delta);
	else
		mxt_delta = 65535 - mxt_delta;
	return sprintf(buf, "-%u\n", mxt_delta);
}

static ssize_t set_delta4_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t mxt_delta=0;
	read_dbg_data(MXT_DELTA_MODE, test_node[4], &mxt_delta);
	if (mxt_delta < 32767)
		return sprintf(buf, "%u\n", mxt_delta);
	else
		mxt_delta = 65535 - mxt_delta;
	return sprintf(buf, "-%u\n", mxt_delta);
}

static ssize_t set_threshold_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", threshold);
}

static ssize_t set_mxt_firm_update_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct mxt224_data *data = dev_get_drvdata(dev);
	int error = 0;
	printk("[TSP] set_mxt_update_show start!!\n");
	if (*buf != 'S' && *buf != 'F') {
		printk(KERN_ERR"Invalid values\n");
		dev_err(dev, "Invalid values\n");
		return -EINVAL;
	}

	disable_irq(data->client->irq);
	firm_status_data = 1;
	if (data->family_id == 0x80) {	/*  : MXT-224 */
		if (*buf != 'F' && data->tsp_version >= firmware_latest[0] && data->tsp_build >= build_latest[0]) {
			printk(KERN_ERR"[TSP] mxt224 has latest firmware\n");
			firm_status_data =2;
			enable_irq(data->client->irq);
			return size;
		}
		printk("[TSP] mxt224_fm_update\n");
		error = mxt224_load_fw(dev, MXT224_FW_NAME);
	}
	else if (data->family_id == 0x81)  {	/* tsp_family_id - 0x81 : MXT-224E */
		if (*buf != 'F' && data->tsp_version >= firmware_latest[1] && data->tsp_build >= build_latest[1]) {
			printk(KERN_ERR"[TSP] mxt224E has latest firmware\n");
			firm_status_data =2;
			enable_irq(data->client->irq);
			return size;
		}
		printk("[TSP] mxt224E_fm_update\n");
		error = mxt224_load_fw(dev, MXT224_ECHO_FW_NAME);
	}

	if (error) {
		dev_err(dev, "The firmware update failed(%d)\n", error);
		firm_status_data =3;
		printk(KERN_ERR"[TSP]The firmware update failed(%d)\n", error);
		return error;
	} else {
		dev_dbg(dev, "The firmware update succeeded\n");
		firm_status_data =2;
		printk("[TSP] The firmware update succeeded\n");

		/* Wait for reset */
		msleep(MXT224_FWRESET_TIME);

		mxt224_init_touch_driver(data);
		/* mxt224_initialize(data); */
	}

	enable_irq(data->client->irq);
	error = mxt224_backup(data);
	if (error)
	{
		printk(KERN_ERR"[TSP]mxt224_backup fail!!!\n");
		return error;
	}

	/* reset the touch IC. */
	error = mxt224_reset(data);
	if (error)
	{
		printk(KERN_ERR"[TSP]mxt224_reset fail!!!\n");
		return error;
	}

	msleep(MXT224_RESET_TIME);
	return size;
}

static ssize_t set_mxt_firm_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	int count;
	printk("Enter firmware_status_show by Factory command \n");

	if (firm_status_data == 1) {
		count = sprintf(buf, "DOWNLOADING\n");
	} else if (firm_status_data == 2) {
		count = sprintf(buf, "PASS\n");
	} else if (firm_status_data == 3) {
		count = sprintf(buf, "FAIL\n");
	} else
		count = sprintf(buf, "PASS\n");

	return count;
}

static ssize_t key_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", threshold);
}

static ssize_t key_threshold_store(struct device *dev, struct device_attribute *attr,
								   const char *buf, size_t size)
{
	/*TO DO IT*/
	unsigned int object_register=7;
	u8 value;
	u8 val;
	int ret;
	u16 address = 0;
	u16 size_one;
	int num;
	if (sscanf(buf, "%d", &num) == 1)
	{
		threshold = num;
		printk("threshold value %d\n",threshold);
		ret = get_object_info(copy_data, TOUCH_MULTITOUCHSCREEN_T9, &size_one, &address);
		size_one = 1;
		value = (u8)threshold;
		write_mem(copy_data, address+(u16)object_register, size_one, &value);
		read_mem(copy_data, address+(u16)object_register, (u8)size_one, &val);
		printk(KERN_ERR"T9 Byte%d is %d\n", object_register, val);
	}
	return size;
}

static ssize_t set_mxt_firm_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u8 fw_latest_version = 0;
	struct mxt224_data *data = dev_get_drvdata(dev);
	if (data->family_id == 0x80) fw_latest_version = firmware_latest[0];
	else if (data->family_id == 0x81) fw_latest_version = firmware_latest[1];

	pr_info("Atmel Last firmware version is %d\n", fw_latest_version);
	return sprintf(buf, "%#02x\n", fw_latest_version);
}

static ssize_t set_mxt_firm_version_read_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct mxt224_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%#02x\n", data->tsp_version);
}

static DEVICE_ATTR(set_refer0, S_IRUGO, set_refer0_mode_show, NULL);
static DEVICE_ATTR(set_delta0, S_IRUGO, set_delta0_mode_show, NULL);
static DEVICE_ATTR(set_refer1, S_IRUGO, set_refer1_mode_show, NULL);
static DEVICE_ATTR(set_delta1, S_IRUGO, set_delta1_mode_show, NULL);
static DEVICE_ATTR(set_refer2, S_IRUGO, set_refer2_mode_show, NULL);
static DEVICE_ATTR(set_delta2, S_IRUGO, set_delta2_mode_show, NULL);
static DEVICE_ATTR(set_refer3, S_IRUGO, set_refer3_mode_show, NULL);
static DEVICE_ATTR(set_delta3, S_IRUGO, set_delta3_mode_show, NULL);
static DEVICE_ATTR(set_refer4, S_IRUGO, set_refer4_mode_show, NULL);
static DEVICE_ATTR(set_delta4, S_IRUGO, set_delta4_mode_show, NULL);
static DEVICE_ATTR(set_threshould, S_IRUGO, set_threshold_mode_show, NULL);

static DEVICE_ATTR(tsp_firm_update, S_IWUSR | S_IWGRP, NULL, set_mxt_firm_update_store);		/* firmware update */
static DEVICE_ATTR(tsp_firm_update_status, S_IRUGO, set_mxt_firm_status_show, NULL);	/* firmware update status return */
static DEVICE_ATTR(tsp_threshold, S_IRUGO | S_IWUSR | S_IWGRP, key_threshold_show, key_threshold_store);	/* touch threshold return, store */
static DEVICE_ATTR(tsp_firm_version_phone, S_IRUGO, set_mxt_firm_version_show, NULL);/* PHONE*/	/* firmware version resturn in phone driver version */
static DEVICE_ATTR(tsp_firm_version_panel, S_IRUGO, set_mxt_firm_version_read_show, NULL);/*PART*/	/* firmware version resturn in TSP panel version */
static DEVICE_ATTR(object_show, S_IWUSR | S_IWGRP, NULL, mxt224_object_show);
static DEVICE_ATTR(object_write, S_IWUSR | S_IWGRP, NULL, mxt224_object_setting);
static DEVICE_ATTR(dbg_switch, S_IWUSR | S_IWGRP, NULL, mxt224_debug_setting);


static struct attribute *mxt224_attrs[] = {
	&dev_attr_object_show.attr,
	&dev_attr_object_write.attr,
	&dev_attr_dbg_switch.attr,
	NULL
};

static const struct attribute_group mxt224_attr_group = {
	.attrs = mxt224_attrs,
};

static int __devinit mxt224_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct mxt224_platform_data *pdata = client->dev.platform_data;
	struct mxt224_data *data;
	struct input_dev *input_dev;
	int ret;
	int i;
	bool ta_status = 0;
	u8 **tsp_config;

	touch_is_pressed = 0;

	if (!pdata) {
		dev_err(&client->dev, "missing platform data\n");
		return -ENODEV;
	}

	if (pdata->max_finger_touches <= 0)
		return -EINVAL;

	data = kzalloc(sizeof(*data) + pdata->max_finger_touches *
					sizeof(*data->fingers), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->num_fingers = pdata->max_finger_touches;
	data->power_on = pdata->power_on;
	data->power_off = pdata->power_off;
	data->register_cb = pdata->register_cb;
	data->read_ta_status = pdata->read_ta_status;

	data->client = client;
	i2c_set_clientdata(client, data);

	input_dev = input_allocate_device();
	if (!input_dev) {
		ret = -ENOMEM;
		dev_err(&client->dev, "input device allocation failed\n");
		goto err_alloc_dev;
	}
	data->input_dev = input_dev;
	input_set_drvdata(input_dev, data);
	input_dev->name = "sec_touchscreen";

	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(MT_TOOL_FINGER, input_dev->keybit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	input_mt_init_slots(input_dev, MAX_USING_FINGER_NUM);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, pdata->min_x,
			pdata->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, pdata->min_y,
			pdata->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, pdata->min_z,
			pdata->max_z, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, pdata->min_w,
			pdata->max_w, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0,
			data->num_fingers - 1, 0, 0);

	ret = input_register_device(input_dev);
	if (ret) {
		input_free_device(input_dev);
		goto err_reg_dev;
	}

	data->gpio_read_done = pdata->gpio_read_done;

	data->reg_vdd_name = pdata->reg_vdd_name;
	data->reg_avdd_name = pdata->reg_avdd_name;

	data->reg_vdd = pdata->reg_vdd;
	data->reg_avdd = pdata->reg_avdd;

	data->i2c_pingroup = pdata->i2c_pingroup;
	data->i2c_suspend_pupd = pdata->i2c_suspend_pupd;
	data->i2c_resume_pupd = pdata->i2c_resume_pupd;

	data->platform_name = pdata->platform_name;

	/* mxt224E PMIC regulator get */
	data->reg_vdd = regulator_get(NULL, data->reg_vdd_name);
	if (IS_ERR(data->reg_vdd)) {
		ret = PTR_ERR(data->reg_vdd);
		pr_err("[TSP] [%s: %s]unable to get regulator %s: %d\n",
			__func__,
			data->platform_name,
			data->reg_vdd_name,
			ret);
	}

	data->reg_avdd = regulator_get(NULL,data->reg_avdd_name);
	if (IS_ERR(data->reg_avdd)) {
		ret = PTR_ERR(data->reg_avdd);
		pr_err("[TSP] [%s: %s]unable to get regulator %s: %d\n",
			__func__,
			data->platform_name,
			data->reg_avdd_name,
			ret);
	}

	data->power_on(data);

	ret = mxt224_init_touch_driver(data);

	copy_data = data;

	noise_median.median_on_flag = false;
	noise_median.mferr_setting = false;
	noise_median.mferr_count = 0;
	noise_median.t46_actvsyncsperx_for_mferr = 38;
	noise_median.t48_mfinvlddiffthr_for_mferr = 10;
	noise_median.t48_mferrorthr_for_mferr = 19;
	noise_median.t48_thr_for_mferr = 40;
	noise_median.t48_movfilter_for_mferr = 0;

	data->register_cb(mxt224_ta_probe);

	if (ret) {
		dev_err(&client->dev, "chip initialization failed\n");
		goto err_init_drv;
	}

	if (data->family_id == 0x80) {	/*  : MXT-224 */
		tsp_config = (u8 **)pdata->config;
		data->atchcalst = pdata->atchcalst;
		data->atchcalsthr = pdata->atchcalsthr;
		data->tchthr_batt = pdata->tchthr_batt;
		data->tchthr_batt_init = pdata->tchthr_batt_init;
		data->tchthr_charging = pdata->tchthr_charging;
		data->noisethr_batt = pdata->noisethr_batt;
		data->noisethr_charging = pdata->noisethr_charging;
		data->movfilter_batt = pdata->movfilter_batt;
		data->movfilter_charging = pdata->movfilter_charging;
		printk("[TSP] TSP chip is MXT224\n");
	}
	else if (data->family_id == 0x81)  {	/* tsp_family_id - 0x81 : MXT-224E */
		tsp_config = (u8 **)pdata->config_e;
		data->t48_config_batt_e = pdata->t48_config_batt_e;
		data->t48_config_chrg_e = pdata->t48_config_chrg_e;
		data->tchthr_batt = pdata->tchthr_batt_e;
		data->tchthr_charging = pdata->tchthr_charging_e;
		data->calcfg_batt_e = pdata->calcfg_batt_e;
		data->calcfg_charging_e = pdata->calcfg_charging_e;
		data->atchcalst = pdata->atchcalst_e;
		data->atchcalsthr = pdata->atchcalsthr_e;
		data->atchfrccalthr_e = pdata->atchfrccalthr_e;
		data->atchfrccalratio_e = pdata->atchfrccalratio_e;

		printk("[TSP] TSP chip is MXT224-E\n");
		if (data->family_id == 0x81) {
			if (!(data->tsp_version >= firmware_latest[1] && data->tsp_build >= build_latest[1])) {
				printk("[TSP] mxt224E force firmware update\n");
				if (mxt224_load_fw(NULL, MXT224_ECHO_FW_NAME)) goto err_config;
				else {
					msleep(MXT224_FWRESET_TIME);
					mxt224_init_touch_driver(data);
				}
			}
		}
		INIT_DELAYED_WORK(&data->config_dwork, mxt_reconfigration_normal);
	}
	else  {
		printk(KERN_ERR"ERROR : There is no valid TSP ID\n");
		goto err_config;
	}

	for (i = 0; tsp_config[i][0] != RESERVED_T255; i++) {
		ret = write_config(data, tsp_config[i][0],
							tsp_config[i] + 1);
		if (ret)
			goto err_config;

		if (tsp_config[i][0] == GEN_POWERCONFIG_T7)
			data->power_cfg = tsp_config[i] + 1;

		if (tsp_config[i][0] == TOUCH_MULTITOUCHSCREEN_T9) {
			/* Are x and y inverted? */
			if (tsp_config[i][10] & 0x1) {
				data->x_dropbits = (!(tsp_config[i][22] & 0xC)) << 1;
				data->y_dropbits = (!(tsp_config[i][20] & 0xC)) << 1;
			} else {
				data->x_dropbits = (!(tsp_config[i][20] & 0xC)) << 1;
				data->y_dropbits = (!(tsp_config[i][22] & 0xC)) << 1;
			}
		}
	}

	ret = mxt224_backup(data);
	if (ret)
		goto err_backup;

	/* reset the touch IC. */
	ret = mxt224_reset(data);
	if (ret)
		goto err_reset;

	msleep(MXT224_RESET_TIME);

/*	if (data->family_id == 0x81) {
		schedule_delayed_work(&data->config_dwork, HZ*30);
	}
*/
	if (data->read_ta_status) {
		data->read_ta_status(&ta_status);
		printk(KERN_DEBUG "[TSP] ta_status is %d\n", ta_status);
		mxt224_ta_probe(ta_status);
	}

	calibrate_chip();

	for (i = 0; i < data->num_fingers; i++)
		data->fingers[i].state = MXT224_STATE_INACTIVE;

	mxt224_enabled = 1;
	ret = request_threaded_irq(client->irq, NULL, mxt224_irq_thread,
		IRQF_TRIGGER_LOW | IRQF_ONESHOT, "mxt224_ts", data);
	if (ret < 0)
		goto err_irq;

	ret = sysfs_create_group(&client->dev.kobj, &mxt224_attr_group);
	if (ret)
		printk(KERN_ERR"[TSP] sysfs_create_group()is falled\n");

	sec_touchscreen = device_create(sec_class, NULL, 0, NULL, "sec_touchscreen");
	dev_set_drvdata(sec_touchscreen, data);
	if (IS_ERR(sec_touchscreen))
		printk(KERN_ERR "[TSP] Failed to create device(sec_touchscreen)!\n");

	if (device_create_file(sec_touchscreen, &dev_attr_tsp_firm_update) < 0)
		printk(KERN_ERR "[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_firm_update.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_tsp_firm_update_status) < 0)
		printk(KERN_ERR "[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_firm_update_status.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_tsp_threshold) < 0)
		printk(KERN_ERR "[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_threshold.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_tsp_firm_version_phone) < 0)
		printk(KERN_ERR "[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_firm_version_phone.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_tsp_firm_version_panel) < 0)
		printk(KERN_ERR "[TSP] Failed to create device file(%s)!\n", dev_attr_tsp_firm_version_panel.attr.name);

	/* mxt224_noise_test = device_create(sec_class, NULL, 0, NULL, "tsp_noise_test"); */

	if (device_create_file(sec_touchscreen, &dev_attr_set_refer0) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_refer0.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_delta0) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_delta0.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_refer1) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_refer1.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_delta1) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_delta1.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_refer2) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_refer2.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_delta2) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_delta2.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_refer3) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_refer3.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_delta3) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_delta3.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_refer4) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_refer4.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_delta4) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_delta4.attr.name);

	if (device_create_file(sec_touchscreen, &dev_attr_set_threshould) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_set_threshould.attr.name);

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING + 1;
	data->early_suspend.suspend = mxt224_early_suspend;
	data->early_suspend.resume = mxt224_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	return 0;

err_irq:
err_reset:
err_backup:
err_config:
	mxt224_enabled = 0;
	kfree(data->objects);
err_init_drv:
	gpio_free(data->gpio_read_done);
/* err_gpio_req:
	data->power_off();
	input_unregister_device(input_dev); */
err_reg_dev:
err_alloc_dev:
	kfree(data);
	return ret;
}

static int __devexit mxt224_remove(struct i2c_client *client)
{
	struct mxt224_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(client->irq, data);
	kfree(data->objects);
	gpio_free(data->gpio_read_done);
	data->power_off(data);
	input_unregister_device(data->input_dev);
	kfree(data);

	return 0;
}

static struct i2c_device_id mxt224_idtable[] = {
	{MXT224_DEV_NAME, 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, mxt224_idtable);

static const struct dev_pm_ops mxt224_pm_ops = {
	.suspend = mxt224_suspend,
	.resume = mxt224_resume,
};

static struct i2c_driver mxt224_i2c_driver = {
	.id_table = mxt224_idtable,
	.probe = mxt224_probe,
	.remove = __devexit_p(mxt224_remove),
	.driver = {
		.owner	= THIS_MODULE,
		.name	= MXT224_DEV_NAME,
		.pm	= &mxt224_pm_ops,
	},
};

static int __init mxt224_init(void)
{
	return i2c_add_driver(&mxt224_i2c_driver);
}

static void __exit mxt224_exit(void)
{
	i2c_del_driver(&mxt224_i2c_driver);
}
module_init(mxt224_init);
module_exit(mxt224_exit);

MODULE_DESCRIPTION("Atmel MaXTouch 224E driver");
MODULE_AUTHOR("Heetae Ahn <heetae82.ahn@samsung.com>");
MODULE_LICENSE("GPL");
