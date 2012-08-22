/*
 * Copyright (C) ST-Ericsson SA 2010
 *
 * License Terms: GNU General Public License, version 2
 * Author: Rabin Vincent <rabin.vincent@stericsson.com> for ST-Ericsson
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/stmpe.h>
#include "stmpe-bose-keypad.h"

#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/debug_locks.h>

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/input/matrix_keypad.h>

#include <mach/gpio-bose.h>

#include <linux/gpio.h>
#include <mach/sec_battery.h>

#define BOARD_REV06 0x09
#define BOARD_REV08 0x0B
#define MAX8922_TOPOFF_INT 0x01
#define EARJACK_DET35_INT  0x02
#define SDCARD_DET_INT 0x80

extern int sec_jack_detect_irq(void);
extern irqreturn_t external_carddetect_irq(void);


struct stmpe *g_stmpe;
EXPORT_SYMBOL(g_stmpe);

/*sec_class sysfs*/
extern struct class *sec_class;
struct device *sec_stmpe_bl;


static struct stmpe_variant_info stmpe1801 = {
	.name		= "stmpe1801",
	.id_val		= 0xc110,                           /* chip_id, version_id */
	.id_mask		= 0xffff,	                     /* at least 0x0210 and 0x0212 */
	.num_gpios	= 18,
	.af_bits		= 2,
//	.regs		= stmpe1801_regs,
	.blocks		= NULL,
	.num_blocks	= NULL,
	.num_irqs	= NULL,
	.enable		= NULL,
	.get_altfunc	= NULL,
	.enable_autosleep	= NULL,
};

static const struct stmpe_keypad_variant stmpe_keypad_variants = {
		.auto_increment	= true,
		.num_data		= 5,
		.num_normal_data	= 3,
		.max_cols		= 10,
		.max_rows		= 8,
		.col_gpios		= 0x000ff,	/* GPIO 0 - 7 */
		.row_gpios		= 0x3ff00,	/* GPIO 8 - 15, GPIO16, GPIO17 */
};

static int __stmpe_reg_read(struct stmpe *stmpe, u8 reg)
{
	int ret;
	ret = i2c_smbus_read_byte_data(stmpe->i2c, reg);
	if (ret < 0)
		dev_err(stmpe->dev, "failed to read reg %#x: %d\n",
			reg, ret);

	dev_vdbg(stmpe->dev, "rd: reg %#x => data %#x\n", reg, ret);

	return ret;
}

static int __stmpe_reg_write(struct stmpe *stmpe, u8 reg, u8 val)
{
	int ret;
	dev_vdbg(stmpe->dev, "wr: reg %#x <= %#x\n", reg, val);

	ret = i2c_smbus_write_byte_data(stmpe->i2c, reg, val);
	if (ret < 0)
		dev_err(stmpe->dev, "failed to write reg %#x: %d\n",
			reg, ret);

	return ret;
}

static int __stmpe_set_bits(struct stmpe *stmpe, u8 reg, u8 mask, u8 val)
{
	int ret;
	ret = __stmpe_reg_read(stmpe, reg);
	if (ret < 0)
		return ret;

	ret &= ~mask;
	ret |= val;

	return __stmpe_reg_write(stmpe, reg, ret);
}

static int __stmpe_block_read(struct stmpe *stmpe, u8 reg, u8 length,
			      u8 *values)
{
	int ret;
	ret = i2c_smbus_read_i2c_block_data(stmpe->i2c, reg, length, values);

	if (ret < 0)
		dev_err(stmpe->dev, "failed to read regs %#x: %d\n",
			reg, ret);

	dev_vdbg(stmpe->dev, "rd: reg %#x (%d) => ret %#x\n", reg, length, ret);
	stmpe_dump_bytes("stmpe rd: ", values, length);

	return ret;
}

static int __stmpe_block_write(struct stmpe *stmpe, u8 reg, u8 length,
			const u8 *values)
{
	int ret;
	dev_vdbg(stmpe->dev, "wr: regs %#x (%d)\n", reg, length);
	stmpe_dump_bytes("stmpe wr: ", values, length);

	ret = i2c_smbus_write_i2c_block_data(stmpe->i2c, reg, length,
					     values);
	if (ret < 0)
		dev_err(stmpe->dev, "failed to write regs %#x: %d\n",
			reg, ret);

	return ret;
}

/**
 * stmpe_reg_read() - read a single STMPE register
 * @stmpe:	Device to read from
 * @reg:	Register to read
 */
int stmpe_reg_read(struct stmpe *stmpe, u8 reg)
{
	int ret;
	mutex_lock(&stmpe->lock);
	ret = __stmpe_reg_read(stmpe, reg);
	mutex_unlock(&stmpe->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(stmpe_reg_read);

/**
 * stmpe_reg_write() - write a single STMPE register
 * @stmpe:	Device to write to
 * @reg:	Register to write
 * @val:	Value to write
 */
int stmpe_reg_write(struct stmpe *stmpe, u8 reg, u8 val)
{
	int ret;
	mutex_lock(&stmpe->lock);
	ret = __stmpe_reg_write(stmpe, reg, val);
	mutex_unlock(&stmpe->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(stmpe_reg_write);

/**
 * stmpe_set_bits() - set the value of a bitfield in a STMPE register
 * @stmpe:	Device to write to
 * @reg:	Register to write
 * @mask:	Mask of bits to set
 * @val:	Value to set
 */
int stmpe_set_bits(struct stmpe *stmpe, u8 reg, u8 mask, u8 val)
{
	int ret;
	mutex_lock(&stmpe->lock);
	ret = __stmpe_set_bits(stmpe, reg, mask, val);
	mutex_unlock(&stmpe->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(stmpe_set_bits);

/**
 * stmpe_block_read() - read multiple STMPE registers
 * @stmpe:	Device to read from
 * @reg:	First register
 * @length:	Number of registers
 * @values:	Buffer to write to
 */
int stmpe_block_read(struct stmpe *stmpe, u8 reg, u8 length, u8 *values)
{
	int ret;
	mutex_lock(&stmpe->lock);
	ret = __stmpe_block_read(stmpe, reg, length, values);
	mutex_unlock(&stmpe->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(stmpe_block_read);

/**
 * stmpe_block_write() - write multiple STMPE registers
 * @stmpe:	Device to write to
 * @reg:	First register
 * @length:	Number of registers
 * @values:	Values to write
 */
int stmpe_block_write(struct stmpe *stmpe, u8 reg, u8 length,
		      const u8 *values)
{
	int ret;
	mutex_lock(&stmpe->lock);
	ret = __stmpe_block_write(stmpe, reg, length, values);
	mutex_unlock(&stmpe->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(stmpe_block_write);


int stmpe_keypad_read_data(struct stmpe_keypad *keypad, u8 *data)
{
	const struct stmpe_keypad_variant *variant = keypad->variant;
	struct stmpe *stmpe = keypad->stmpe;
	int ret;
	int i;

	if (variant->auto_increment)
		return stmpe_block_read(stmpe, STMPE_KPC_DATA_BYTE0,
					variant->num_data, data);

	for (i = 0; i < variant->num_data; i++) {
		ret = stmpe_reg_read(stmpe, STMPE_KPC_DATA_BYTE0 + i);
		if (ret < 0)
			return ret;

		data[i] = ret;
	}

	return 0;
}

#if defined (CONFIG_MACH_BOSE_ATT)
int stmpe_max8922_topoff_irq_read_data(struct stmpe_keypad *keypad, int inq_status_high)
{
	int ta_nconnected;

	ta_nconnected = gpio_get_value(GPIO_TA_nCONNECTED);

	printk("%s [%x][%d]\n", __func__, inq_status_high, ta_nconnected);
	if(inq_status_high == MAX8922_TOPOFF_INT && ta_nconnected ==0){
		max8922_charger_topoff();
	}
	return 0;
}

int stmpe_get_topoff_value(void)
{
	int inq_status;

	inq_status = stmpe_reg_read(g_stmpe, STMPE_GPIO_MP_HIGH);
	if(inq_status == MAX8922_TOPOFF_INT)
		return 1;
	else
		return 0;
}

static void stmpe_max8922_full_comp_work_handler(struct work_struct *work)
{
	struct delayed_work *dw = container_of(work, struct delayed_work, work);
	struct stmpe_keypad *keypad = container_of(dw, struct stmpe_keypad, full_chg_work);

	stmpe_max8922_topoff_irq_read_data(keypad, MAX8922_TOPOFF_INT);
}

#endif

static irqreturn_t stmpe_irq(int irq, void *data)
{
	struct stmpe_keypad *keypad = data;
	struct stmpe *stmpe = keypad->stmpe;
	struct input_dev *input =  keypad->input;

	u8 fifo[keypad->variant->num_data];
	int ret,i;
	int iSta_high, iSta_mid, iStaValue ;

	iStaValue = stmpe_reg_read(stmpe, STMPE1801_REG_ISR_LSB);
	iSta_mid = stmpe_reg_read(stmpe, STMPE_INT_STA_GPIO_MID);
	iSta_high = stmpe_reg_read(stmpe, STMPE_INT_STA_GPIO_HIGH);


/* MAX8922 TOPOFF , EARJACK DET */
if(system_rev >= BOARD_REV06){
		printk("STMPE_INT_STA_GPIO_HIGH %#x: [%x]\n", STMPE_INT_STA_GPIO_HIGH, iSta_high);

		if(iSta_high == MAX8922_TOPOFF_INT){
			cancel_delayed_work(&keypad->full_chg_work);
			schedule_delayed_work(&keypad->full_chg_work, HZ/10);  // after 100 ms
			return IRQ_HANDLED;
		} else if ( iSta_high == EARJACK_DET35_INT ) {
		/* earjack handler */
		sec_jack_detect_irq();
		return IRQ_HANDLED;
	}
}

/* SDCARD DETECTION */
	if(system_rev >= BOARD_REV08){
		printk("STMPE_INT_STA_GPIO_MID %#x: [%x]\n", STMPE_INT_STA_GPIO_MID, iSta_mid);

		if(iSta_mid == SDCARD_DET_INT){
			external_carddetect_irq();
			printk(KERN_ERR "SDcard_detect_interrupt occur\n");
			return IRQ_HANDLED;
		}
	}

/* QWERTY KEYPAD */
	ret = stmpe_keypad_read_data(keypad, fifo);
	if (ret < 0)
		return IRQ_NONE;

	for (i = 0; i < keypad->variant->num_normal_data; i++) {
		u8 data = fifo[i];
		int col = (data & STMPE_KPC_DATA_COL) >> 3;
		int row = data & STMPE_KPC_DATA_ROW;

		int code = keypad->keymap[MATRIX_SCAN_CODE(row, col, STMPE_KEYPAD_COL_SHIFT)];
		bool up = data & STMPE_KPC_DATA_UP;

		if ((data & STMPE_KPC_DATA_NOKEY_MASK)
			== STMPE_KPC_DATA_NOKEY_MASK)
			continue;

		printk("[%s] row=%d, col=%d,  code=%d , ISR=%x \n",__func__ ,row,col,code,iStaValue);
		input_event(input, EV_MSC, MSC_SCAN, code);
		input_report_key(input, code, !up);
		input_sync(input);
	}

	return IRQ_HANDLED;

}


static void stmpe_irq_remove(struct stmpe *stmpe)
{
	int num_irqs = stmpe->variant->num_irqs;
	int base = stmpe->irq_base;
	int irq;

	for (irq = base; irq < base + num_irqs; irq++) {
#ifdef CONFIG_ARM
		set_irq_flags(irq, 0);
#endif
		irq_set_chip_and_handler_name(irq, NULL, NULL, NULL);
		irq_set_chip_data(irq, NULL);
	}
}

static int stmpe_key_suspend(struct i2c_client *client)
{
	printk(KERN_DEBUG "[QwertyKey] Qwertykey_early_suspend\n");

  tegra_gpio_enable(GPIO_QKEY_BL_EN);
  gpio_direction_output(GPIO_QKEY_BL_EN,0);

	return 0;
}


static int stmpe_key_resume(struct i2c_client *client)
{

//	printk(KERN_DEBUG "[QwertyKey] stmpe_key_resume\n");

#if 0
  tegra_gpio_enable(GPIO_QKEY_BL_EN);
  gpio_direction_output(GPIO_QKEY_BL_EN,1);
#endif

	return 0;
}

void stmpe_Register_DUMP(struct stmpe_keypad *keypad)
{
    u8 iread;
    int value;

	   printk("[STMPE_KD] , REGISTER DUMP --------- \n");

	    for(iread= 0x0; iread < 0x3E; iread++)
		{
		    value = stmpe_reg_read(keypad->stmpe, iread);
		     printk("[%x]= %x ", iread, value);
		    if( (iread % 8) == 0 )
			 printk(" \n");
		}

	   printk(" \n ----------------------------------- \n");

 }
EXPORT_SYMBOL(stmpe_Register_DUMP);

static int __devinit stmpe_keypad_chip_init(struct stmpe_keypad *keypad)
{
	const struct stmpe_keypad_platform_data *plat = keypad->plat;
	struct stmpe *stmpe = keypad->stmpe;
	int ret;

	if (plat->debounce_ms > STMPE_KEYPAD_MAX_DEBOUNCE)
		return -EINVAL;

	if (plat->scan_count > STMPE_KEYPAD_MAX_SCAN_COUNT)
		return -EINVAL;

       // KPC columm nums
	ret = stmpe_reg_write(stmpe, STMPE_KPC_COL_LOW, keypad->cols);
	if (ret < 0)
		return ret;

	ret = stmpe_reg_write(stmpe, STMPE_KPC_COL_HIGH, 0x0);
	if (ret < 0)
		return ret;


      // KPC row nums
	ret = stmpe_reg_write(stmpe, STMPE_KPC_ROW, keypad->rows);
	if (ret < 0)
		return ret;


      // KPC control scan count, dedicated keys
	ret = stmpe_set_bits(stmpe, STMPE_KPC_CTL_LOW,
			     STMPE_KPC_CTRL_MSB_SCAN_COUNT,
			    plat->scan_count << 4);
	if (ret < 0)
		return ret;


      // KPC control  debouce time
	ret = stmpe_set_bits(stmpe, STMPE_KPC_CTL_MID,  0xff, plat->debounce_ms);  // deboucnce time
	if (ret < 0)
		return ret;


      // scan frequency
	ret = stmpe_set_bits(stmpe, STMPE_KPC_CTL_HIGH,  0xff,   0x40);

	if (ret < 0)
		return ret;

	/* GPIO direction */
	ret = stmpe_set_bits(stmpe, STMPE_GPIO_DIR_LOW,  0xff,  0x0);
	if (ret < 0)
		return ret;

	if(system_rev >= BOARD_REV06){
		ret = stmpe_set_bits(stmpe, STMPE_GPIO_DIR_HIGH,  0x03,  0x0);
		if (ret < 0)
			return ret;
	}

	if(system_rev >= BOARD_REV08){
		ret = stmpe_set_bits(stmpe, STMPE_GPIO_DIR_MID,  0x80,	0x0);
		if (ret < 0)
			return ret;
	}

	/* Edge detection */
	ret = stmpe_set_bits(stmpe, STMPE_GPIO_FE_LOW,     0xff,  0xff);
		if (ret < 0)
			return ret;

	if(system_rev >= BOARD_REV06){
		ret = stmpe_set_bits(stmpe, STMPE_GPIO_RE_HIGH, 	0x03,  0x03);
	if (ret < 0)
		return ret;

		ret = stmpe_set_bits(stmpe, STMPE_GPIO_FE_HIGH, 	0x03,  0x02);
	if (ret < 0)
		return ret;
	}

	if(system_rev >= BOARD_REV08){
		/* SD_DET GPIO15 rising/falling edge */
		ret = stmpe_set_bits(stmpe, STMPE_GPIO_RE_MID, 	0x80,  0x80);
	if (ret < 0)
		return ret;

		ret = stmpe_set_bits(stmpe, STMPE_GPIO_FE_MID, 	0x80,  0x80);
	if (ret < 0)
		return ret;
	}

	/* GPIO PULLUP/DOWN setting */
	ret = stmpe_set_bits(stmpe, STMPE_GPIO_PULLUP_LOW,     0xff,  0xff);
	if (ret < 0)
		return ret;

	if(system_rev >= BOARD_REV06){
		/* topoff interrupt pin internal pull-up */
		ret = stmpe_set_bits(stmpe, STMPE_GPIO_PULLUP_HIGH,  0x01,  0x01);
	if (ret < 0)
		return ret;
	}


	/* GPIO interrupt enable */
	ret = stmpe_set_bits(stmpe, STMPE_GPIO_INT_EN_MASK_LOW,     0xff,  0xff);
	if (ret < 0)
		return ret;

	if(system_rev >= BOARD_REV08){
		ret = stmpe_set_bits(stmpe, STMPE_GPIO_INT_EN_MASK_MID, 0x80, 0x80);
		if (ret < 0)
			return ret;
	}

	if(system_rev >= BOARD_REV06){
		ret = stmpe_set_bits(stmpe, STMPE_GPIO_INT_EN_MASK_HIGH, 0x03, 0x03);
		if (ret < 0)
			return ret;
	}

	/* SCAN START */
	ret = stmpe_set_bits(stmpe, STMPE_KPC_CMD, 0xff, 0x3);  // scan starting
	if (ret < 0)
		return ret;

	return  0;
}

static int __devinit stmpe_chip_INT_init(struct stmpe *stmpe)
{
//	unsigned int irq_trigger = stmpe->pdata->irq_trigger;
//	int autosleep_timeout = stmpe->pdata->autosleep_timeout;

	struct stmpe_variant_info *variant = stmpe->variant;

	u8 icr = 0;  //  to allow global interrupt to host
	unsigned int id;
	u8 data[2];
	int ret;

	ret = stmpe_block_read(stmpe, STMPE1801_IDX_CHIP_ID,
			       ARRAY_SIZE(data), data);
	if (ret < 0)
		return ret;


	id = ((unsigned int)data[0] << 8) | (unsigned int)data[1];   // chipid, vender id

	if ((id & variant->id_mask) != variant->id_val) {
		dev_err(stmpe->dev, "unknown chip id: %#x\n", id);
		return -EINVAL;
	}

	dev_info(stmpe->dev, "%s detected, chip id: %#x\n", variant->name, id);


       ret = stmpe_reg_write(stmpe, STMPE1801_REG_SYS_CTRL, 0x80);
	if (ret < 0)
		return ret;

	if ( system_rev >= BOARD_REV06 ) {
		ret = stmpe_reg_write(stmpe, STMPE1801_REG_IER_LSB, 0x1E);
	} else {
		ret = stmpe_reg_write(stmpe, STMPE1801_REG_IER_LSB, 0x16);
	}
	if (ret < 0)
		return ret;

#if defined (CONFIG_MACH_BOSE_ATT)
	icr = STMPE_ICR_LSB_LEVEL | STMPE_ICR_LSB_GIM;
#else
	icr = STMPE_ICR_LSB_EDGE | STMPE_ICR_LSB_GIM;
#endif
       ret = stmpe_reg_write(stmpe, STMPE1801_REG_ICR_LSB, icr);
	if (ret < 0)
		return ret;



	return   0;
}


static ssize_t stmpe_backlight_control(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t size)
{
	int data;
	if (sscanf(buf, "%d\n", &data) == 1) {
		if (data == 0) {
//			tegra_gpio_enable(GPIO_QKEY_BL_EN);
			gpio_direction_output(GPIO_QKEY_BL_EN,0);
			printk(KERN_DEBUG "[QWERTY] stmpe_backlight_control: %d \n", data);
		} else if (data == 1) {
//			tegra_gpio_enable(GPIO_QKEY_BL_EN);
			gpio_direction_output(GPIO_QKEY_BL_EN,1);
			printk(KERN_DEBUG "[QWERTY] stmpe_backlight_control: %d \n", data);
		}

	} else {
		printk(KERN_DEBUG "[QWERTY] stmpe_backlight_control Error\n");
	}

	return size;
}

static DEVICE_ATTR(backlight, 0664, NULL, stmpe_backlight_control);


static int __devinit stmpe_probe(struct i2c_client *i2c,
				 const struct i2c_device_id *id)
{
	struct stmpe_keypad_platform_data *pdata = i2c->dev.platform_data;
	struct stmpe_keypad *keypad;
	struct input_dev *input;
	struct stmpe * stmpe;

	int ret;
	int i;

       if(!pdata)
		return -ENOMEM;

	stmpe = kzalloc(sizeof(*stmpe), GFP_KERNEL);
	if (!stmpe)
		return -ENOMEM;

	keypad = kzalloc(sizeof(struct stmpe_keypad), GFP_KERNEL);
	if (!keypad) {
		kfree(stmpe);
		return -ENOMEM;
	}
	mutex_init(&stmpe->irq_lock);
	mutex_init(&stmpe->lock);

	stmpe->dev = &i2c->dev;
	stmpe->i2c = i2c;
	stmpe->variant = &stmpe1801;
	stmpe->num_gpios = stmpe->variant->num_gpios;


	input = input_allocate_device();
	if (!input) {
		ret = -ENOMEM;
		goto out_free;
	}
	input->name = "STMPE_keypad";
	input->id.bustype = BUS_I2C;
	input->dev.parent = &i2c->dev;

	input_set_capability(input, EV_MSC, MSC_SCAN);

	__set_bit(EV_KEY, input->evbit);
	if (!pdata->no_autorepeat)
		__set_bit(EV_REP, input->evbit);

	input->keycode = keypad->keymap;
	input->keycodesize = sizeof(keypad->keymap[0]);
	input->keycodemax = ARRAY_SIZE(keypad->keymap);

	matrix_keypad_build_keymap(pdata->keymap_data, STMPE_KEYPAD_COL_SHIFT,
				   input->keycode, input->keybit);

	for (i = 0; i < pdata->keymap_data->keymap_size; i++) {
		unsigned int key = pdata->keymap_data->keymap[i];

		keypad->cols |= 1 << KEY_COL(key);
		keypad->rows |= 1 << KEY_ROW(key);
	}

	keypad->stmpe = stmpe;
	keypad->plat = pdata;
	keypad->input = input;
	keypad->variant = &stmpe_keypad_variants;

	i2c_set_clientdata(i2c, stmpe);
#ifdef CONFIG_MACH_BOSE_ATT
	g_stmpe = stmpe; // add opk

	/* init delayed work */
	INIT_DELAYED_WORK(&keypad->full_chg_work, stmpe_max8922_full_comp_work_handler);
#endif

	ret = stmpe_chip_INT_init(stmpe);
	if (ret)
		goto out_free;

	ret = stmpe_keypad_chip_init(keypad);
	if (ret)
		goto out_free;

	ret = input_register_device(input);
	if (ret) {
		dev_err(&i2c->dev,
			"unable to register input device: %d\n", ret);
		goto out_freeinput;
	}

/* BOSE_QWERTY_BL */
    sec_stmpe_bl= device_create(sec_class, NULL, 0, NULL, "sec_stmpe_bl");
	if (IS_ERR(sec_stmpe_bl))
	{
		printk(KERN_ERR "Failed to create device(sec_stmpe_bl)!\n");
	}
	if (device_create_file(sec_stmpe_bl, &dev_attr_backlight)< 0)
	{
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_backlight.attr.name);
	}
/* BOSE_QWERTY_BL */

	ret = request_threaded_irq(gpio_to_irq(stmpe->i2c->irq), NULL, stmpe_irq,
				   IRQF_TRIGGER_FALLING  , "stmpe1801", keypad);
	if (ret) {
		dev_err(stmpe->dev, "failed to request IRQ: %d\n", ret);
		goto out_removeirq;
	}

	ret = enable_irq_wake(gpio_to_irq(stmpe->i2c->irq));
	if (ret < 0)
		dev_err(stmpe->dev,
			"failed to enable wakeup src %d\n", ret);

	tegra_gpio_enable(GPIO_QKEY_BL_EN);

	return 0;

out_removedevs:
	mfd_remove_devices(stmpe->dev);
	free_irq(gpio_to_irq(stmpe->i2c->irq), stmpe);

out_removeirq:
	stmpe_irq_remove(stmpe);
out_freeinput:
	input_free_device(input);
out_free:
	kfree(stmpe);
	kfree(keypad);

	return ret;
}

static int __devexit stmpe_remove(struct i2c_client *client)
{
	struct stmpe *stmpe = i2c_get_clientdata(client);

	mfd_remove_devices(stmpe->dev);

	free_irq(stmpe->i2c->irq, stmpe);
	stmpe_irq_remove(stmpe);

	kfree(stmpe);

	return 0;
}


static const struct i2c_device_id stmpe_id[] = {
	{ "stmpe1801", 0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, stmpe_id);

static struct i2c_driver stmpe_driver = {
	.driver.name	= "stmpe1801",
	.driver.owner	= THIS_MODULE,
	.probe		= stmpe_probe,
	.remove		= __devexit_p(stmpe_remove),
	.suspend	= stmpe_key_suspend,
	.resume		= stmpe_key_resume,
	.id_table	= stmpe_id,
};

static int __init stmpe_init(void)
{
	return i2c_add_driver(&stmpe_driver);
}
subsys_initcall(stmpe_init);

static void __exit stmpe_exit(void)
{
	i2c_del_driver(&stmpe_driver);
}
module_exit(stmpe_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("STMPE MFD core driver");
MODULE_AUTHOR("Rabin Vincent <rabin.vincent@stericsson.com>");
