/*
 * Copyright (C) ST-Ericsson SA 2010
 *
 * License Terms: GNU General Public License, version 2
 * Author: Rabin Vincent <rabin.vincent@stericsson.com> for ST-Ericsson
 */

#ifndef __STMPE_H
#define __STMPE_H

#ifdef STMPE_DUMP_BYTES
static inline void stmpe_dump_bytes(const char *str, const void *buf,
				    size_t len)
{
	print_hex_dump_bytes(str, DUMP_PREFIX_OFFSET, buf, len);
}
#else
static inline void stmpe_dump_bytes(const char *str, const void *buf,
				    size_t len)
{
}
#endif

/**
 * struct stmpe_variant_block - information about block
 * @cell:	base mfd cell
 * @irq:	interrupt number to be added to each IORESOURCE_IRQ
 *		in the cell
 * @block:	block id; used for identification with platform data and for
 *		enable and altfunc callbacks
 */
struct stmpe_variant_block {
	struct mfd_cell		*cell;
	int			irq;
	enum stmpe_block	block;
};

/**
 * struct stmpe_variant_info - variant-specific information
 * @name:	part name
 * @id_val:	content of CHIPID register
 * @id_mask:	bits valid in CHIPID register for comparison with id_val
 * @num_gpios:	number of GPIOS
 * @af_bits:	number of bits used to specify the alternate function
 * @blocks:	list of blocks present on this device
 * @num_blocks:	number of blocks present on this device
 * @num_irqs:	number of internal IRQs available on this device
 * @enable:	callback to enable the specified blocks.
 *		Called with the I/O lock held.
 * @get_altfunc: callback to get the alternate function number for the
 *		 specific block
 * @enable_autosleep: callback to configure autosleep with specified timeout
 */
struct stmpe_variant_info {
	const char *name;
	u16 id_val;
	u16 id_mask;
	int num_gpios;
	int af_bits;
	const u8 *regs;
	struct stmpe_variant_block *blocks;
	int num_blocks;
	int num_irqs;
	int (*enable)(struct stmpe *stmpe, unsigned int blocks, bool enable);
	int (*get_altfunc)(struct stmpe *stmpe, enum stmpe_block block);
	int (*enable_autosleep)(struct stmpe *stmpe, int autosleep_timeout);
};


#define STMPE_KEYPAD_MAX_ROWS		8
#define STMPE_KEYPAD_MAX_COLS		       10
#define STMPE_KEYPAD_KEYMAP_SIZE	\
	(STMPE_KEYPAD_MAX_ROWS * STMPE_KEYPAD_MAX_COLS)

struct stmpe_keypad {
	struct stmpe *stmpe;
	struct input_dev *input;
	const struct stmpe_keypad_variant *variant;
	const struct stmpe_keypad_platform_data *plat;
#ifdef CONFIG_MACH_BOSE_ATT
	struct delayed_work	full_chg_work;
#endif
	unsigned int rows;
	unsigned int cols;

	unsigned short keymap[STMPE_KEYPAD_KEYMAP_SIZE];
};

struct stmpe_keypad_variant {
	bool		auto_increment;
	int		num_data;
	int		num_normal_data;
	int		max_cols;
	int		max_rows;
	unsigned int	col_gpios;
	unsigned int	row_gpios;
};


#define STMPE_ICR_LSB_AciveLow	(1 << 2)       // falling / rising edge interrupt
#define STMPE_ICR_LSB_EDGE	(1 << 1)       // level / edge interrupt
#define STMPE_ICR_LSB_LEVEL 	(0 << 1)	   // level / edge interrupt
#define STMPE_ICR_LSB_GIM	(1 << 0)       // Global interrupt mask bit


/*
 * STMPE1801
 */

#define STMPE_KPC_DATA_UP		(0x1 << 7)
#define STMPE_KPC_DATA_COL		(0xf << 3)
#define STMPE_KPC_DATA_ROW		(0x7 << 0)
#define STMPE_KPC_CTRL_MSB_SCAN_COUNT	(0xf << 4)

#define STMPE_KEYPAD_COL_SHIFT		3
#define STMPE_KPC_DATA_NOKEY_MASK	0x78

#define STMPE_KPC_DATA_NOKEY_MASK	0x78

#define STMPE_KEYPAD_MAX_DEBOUNCE	127
#define STMPE_KEYPAD_MAX_SCAN_COUNT	15

#define STMPE_KPC_CMD_SCAN		               (0x1 << 0)
#define STMPE_KPC_CMD_KPC_LOCK	               (0x1 << 1)
#define STMPE_KPC_CTRL_SCAN_FREQ_60hz	 (0x0 << 0)
#define STMPE_KPC_CTRL_SCAN_FREQ_30hz	 (0x1 << 0)
#define STMPE_KPC_CTRL_SCAN_FREQ_15hz	 (0x2 << 0)
#define STMPE_KPC_CTRL_SCAN_FREQ_275hz	 (0x3 << 0)
#define STMPE_KPC_CTRL_CMD_KEY

//-----------------------
#define STMPE1801_IDX_CHIP_ID                0x0
#define STMPE1801_REG_SYS_CTRL		0x02
#define STMPE1801_REG_SYS_CTRL2		0x03
#define STMPE1801_REG_ICR_LSB			0x04
#define STMPE1801_REG_INT_CTRL_HIGH     0x05

#define STMPE1801_REG_IER_LSB			0x06
#define STMPE1801_REG_INT_EN_MASK_HIGH 0x07

#define STMPE1801_REG_ISR_LSB			0x08
#define STMPE1801_REG_ISR_MSB               0x09

#define STMPE1801_REG_CHIP_ID			0x00
#define STMPE1801_REG_VERSION_ID          0x01

#define STMPE1801_REG_INT_EN_GPIO_MASK_LSB	0x0A
#define STMPE1801_REG_INT_STA_GPIO_LSB		0x0D
#define STMPE1801_REG_INT_STA_GPIO_MSB           0x0E

#define STMPE1801_REG_GPIO_MP_LSB		0x16
#define STMPE1801_REG_GPIO_SET_LSB		0x10
#define STMPE1801_REG_GPIO_CLR_LSB		0x13
#define STMPE1801_REG_GPIO_SET_DIR_LSB	0x19

#define STMPE1801_REG_GPIO_ED_MSB		0xFF

#define STMPE1801_REG_GPIO_RE_LSB		0x1C
#define STMPE1801_REG_GPIO_FE_LSB		0x1F
#define STMPE1801_REG_GPIO_AF_U_MSB	0xFF

#define STMPE_GPIO_DIR_LOW	 0x19
#define STMPE_GPIO_DIR_MID  0x1A
#define STMPE_GPIO_DIR_HIGH	 0x1B
#define STMPE_GPIO_PULLUP_LOW	 0x22
#define STMPE_GPIO_PULLUP_MID	 0x23
#define STMPE_GPIO_PULLUP_HIGH	 0x24
#define STMPE_GPIO_FE_LOW         0x1f
#define STMPE_GPIO_FE_MID		  0x20
#define STMPE_GPIO_FE_HIGH		  0x21
#define STMPE_GPIO_RE_MID		  0x1D
#define STMPE_GPIO_RE_HIGH		  0x1E
#define STMPE_GPIO_INT_EN_MASK_LOW  0xA
#define STMPE_GPIO_INT_EN_MASK_MID	0xB
#define STMPE_GPIO_INT_EN_MASK_HIGH	0xC
#define STMPE_INT_STA_GPIO_MID  0xE
#define STMPE_INT_STA_GPIO_HIGH	 0xF

#define STMPE_GPIO_MP_HIGH	 0x18


#define STMPE_KPC_ROW			0x30
#define STMPE_KPC_COL_LOW		0x31
#define STMPE_KPC_COL_HIGH		0x32

#define STMPE_KPC_CTL_LOW		0x33
#define STMPE_KPC_CTL_MID		0x34
#define STMPE_KPC_CTL_HIGH		0x35
#define STMPE_KPC_CMD		       0x36

#define STMPE_KPC_COMBI_KEY0    0x37
#define STMPE_KPC_COMBI_KEY1    0x38
#define STMPE_KPC_COMBI_KEY2    0x39

#define STMPE_KPC_DATA_BYTE0		0x3A
#define STMPE_KPC_DATA_BYTE1		0x3B
#define STMPE_KPC_DATA_BYTE2		0x3C
#define STMPE_KPC_DATA_BYTE3		0x3D
#define STMPE_KPC_DATA_BYTE4		0x3E

#endif
