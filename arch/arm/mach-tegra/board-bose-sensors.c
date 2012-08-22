/*
 * arch/arm/mach-tegra/board-n1-sensors.c
 *
 * Copyright (c) 2010, NVIDIA, All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of NVIDIA CORPORATION nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/i2c.h>
#include <mach/gpio.h>
#include <linux/i2c/ak8975.h>

#include "gpio-names.h"
#include <linux/mpu.h>
#if defined(CONFIG_MACH_BOSE_ATT)
#include <mach/gpio-bose.h>
#else
#include <mach/gpio-n1.h>
#endif

/* we use a skeleton to provide some information needed by MPL
 * but we don't use the suspend/resume/read functions so we
 * don't initialize them so that mldl_cfg.c doesn't try to
 * control it directly.  we have a separate mag driver instead.
 */

extern struct class *sec_class;
extern unsigned int system_rev;

#if defined(CONFIG_MACH_BOSE_ATT)

static struct mpu3050_platform_data n1_mpu3050_pdata_bose_rev01 = {
	.int_config  = 0x10,
	/* Orientation for MPU.  Part is mounted rotated
	 * 90 degrees counter-clockwise from natural orientation.
	 * So X & Y are swapped and Y is negated.
	 */
	.orientation = {0,  1,  0,
			-1,  0,  0,
			0,  0,  1 },
	.level_shifter = 0,
	.accel = {
		.get_slave_descr = NULL,
		.irq	     = 0,
		.adapt_num   = 0,
		.bus         = EXT_SLAVE_BUS_SECONDARY,
		.address	 = 0x30>>1,
		/* Orientation for the Acc.  Part is mounted rotated
		 * 180 degrees from natural orientation.
		 * So X & Y are both negated.
		 */
		.orientation = {1,  0,  0,
				0,  1,  0,
				0,  0,  1 },
	},
	.compass = {
		.get_slave_descr = NULL,
		.irq	     = 0,
		.adapt_num   = 7,            /*bus number 7*/
		.bus         = EXT_SLAVE_BUS_PRIMARY,
		.address     = 0x0C,
		/* Orientation for the Mag.  Part is mounted rotated
		 * 90 degrees clockwise from natural orientation.
		 * So X & Y are swapped and Y & Z are negated.
		 */
		.orientation = {1,  0,  0,
				0,  1,  0,
				0,  0,  1 },
	},
};

static struct mpu3050_platform_data n1_mpu3050_pdata_bose_rev05 = {
	.int_config  = 0x10,
	/* Orientation for MPU.  Part is mounted rotated
	 * 90 degrees counter-clockwise from natural orientation.
	 * So X & Y are swapped and Y is negated.
	 */
	.orientation = {0,  1,  0,
			-1,  0,  0,
			0,  0,  1},
	.level_shifter = 0,
	.accel = {
		.get_slave_descr = NULL,
		.irq	     = 0,
		.adapt_num   = 0,
		.bus         = EXT_SLAVE_BUS_SECONDARY,
		/*
		.address	 = 0x30>>1,
		*/
		.address     = 0x0F,

		/* Orientation for the Acc.  Part is mounted rotated
		 * 180 degrees from natural orientation.
		 * So X & Y are both negated.
		 */
		.orientation = {1,  0,  0,
				0,  1,  0,
				0,  0,  1},
	},
	.compass = {
		.get_slave_descr = NULL,
		.irq	     = 0,
		.adapt_num   = 7,            /*bus number 7*/
		.bus         = EXT_SLAVE_BUS_PRIMARY,
		.address     = 0x0C,
		/* Orientation for the Mag.  Part is mounted rotated
		 * 90 degrees clockwise from natural orientation.
		 * So X & Y are swapped and Y & Z are negated.
		 */
		.orientation = {1,  0,  0,
				0,  1,  0,
				0,  0,  1},
	},
};


#else
static struct mpu3050_platform_data n1_mpu3050_pdata = {
	.int_config  = 0x10,
	/* Orientation for MPU.  Part is mounted rotated
	 * 90 degrees counter-clockwise from natural orientation.
	 * So X & Y are swapped and Y is negated.
	 */
	.orientation = {0,  1,  0,
			1,  0,  0,
			0,  0,  -1},
	.level_shifter = 0,
	.accel = {
		.get_slave_descr = NULL,
		.irq	     = 0,
		.adapt_num   = 0,
		.bus         = EXT_SLAVE_BUS_SECONDARY,
		.address     = 0x0F,
		/* Orientation for the Acc.  Part is mounted rotated
		 * 180 degrees from natural orientation.
		 * So X & Y are both negated.
		 */
		.orientation = {0,  1,  0,
				1, 0,  0,
				0,  0,  -1},
	},
	.compass = {
		.get_slave_descr = NULL,
		.irq	     = 0,
		.adapt_num   = 7,            /*bus number 7*/
		.bus         = EXT_SLAVE_BUS_PRIMARY,
		.address     = 0x0C,
		/* Orientation for the Mag.  Part is mounted rotated
		 * 90 degrees clockwise from natural orientation.
		 * So X & Y are swapped and Y & Z are negated.
		 */
		.orientation = {0, -1,  0,
				1,  0,  0,
				0,  0,  1},
	},
};

static struct mpu3050_platform_data n1_mpu3050_pdata_rev05 = {
	.int_config  = 0x10,
	/* Orientation for MPU.  Part is mounted rotated
	 * 90 degrees counter-clockwise from natural orientation.
	 * So X & Y are swapped and Y is negated.
	 */
	.orientation = {0, -1,  0,
			-1, 0,  0,
			0,  0,  -1},
	.level_shifter = 0,
	.accel = {
		.get_slave_descr = NULL,
		.irq	     = 0,
		.adapt_num   = 0,
		.bus         = EXT_SLAVE_BUS_SECONDARY,
		.address     = 0x0F,
		/* Orientation for the Acc.  Part is mounted rotated
		 * 180 degrees from natural orientation.
		 * So X & Y are both negated.
		 */
		.orientation = {1,  0,  0,
				0, -1,  0,
				0,  0,  -1},
	},
	.compass = {
		.get_slave_descr = NULL,
		.irq	     = 0,
		.adapt_num   = 7,            /*bus number 7*/
		.bus         = EXT_SLAVE_BUS_PRIMARY,
		.address     = 0x0C,
		/* Orientation for the Mag.  Part is mounted rotated
		 * 90 degrees clockwise from natural orientation.
		 * So X & Y are swapped and Y & Z are negated.
		 */
		.orientation = {0,  1,  0,
				1,  0, 0,
				0,  0, -1 },
	},
};
#endif

static void n1_mpu3050_init(void)
{
	tegra_gpio_enable(TEGRA_GPIO_PA0);
	gpio_request(TEGRA_GPIO_PA0, "mpu3050_int");
	gpio_direction_input(TEGRA_GPIO_PA0);
#if defined(CONFIG_MACH_BOSE_ATT)
	if (system_rev < 0x08)
		n1_mpu3050_pdata_bose_rev01.sec_class = sec_class;
	else
		n1_mpu3050_pdata_bose_rev05.sec_class = sec_class;
#else
	if (system_rev < 0x05)
		n1_mpu3050_pdata.sec_class = sec_class;
	else
		n1_mpu3050_pdata_rev05.sec_class = sec_class;
#endif
}

#if defined(CONFIG_MACH_BOSE_ATT)
static const struct i2c_board_info n1_i2c_mpu_sensor_board_info_bose_rev01[] = {
	{
		I2C_BOARD_INFO("mpu3050", 0x68),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PA0),
		.platform_data = &n1_mpu3050_pdata_bose_rev01,
	},
	{
		I2C_BOARD_INFO("kxud9", 0x30>>1),
	},
};

static const struct i2c_board_info n1_i2c_mpu_sensor_board_info_bose_rev05[] = {
	{
		I2C_BOARD_INFO("mpu3050", 0x68),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PA0),
		.platform_data = &n1_mpu3050_pdata_bose_rev05,
	},
	{
		I2C_BOARD_INFO("kxtf9", 0x0F),
	},
};

#else
static const struct i2c_board_info n1_i2c_mpu_sensor_board_info[] = {
	{
		I2C_BOARD_INFO("mpu3050", 0x68),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PA0),
		.platform_data = &n1_mpu3050_pdata,
	},
	{
		I2C_BOARD_INFO("kxtf9", 0x0F),
	},
#if 0
	{
		I2C_BOARD_INFO("kxud9", 0x30>>1),
	},
#endif
};
static const struct i2c_board_info n1_i2c_mpu_sensor_board_info_rev05[] = {
	{
		I2C_BOARD_INFO("mpu3050", 0x68),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PA0),
		.platform_data = &n1_mpu3050_pdata_rev05,
	},
	{
		I2C_BOARD_INFO("kxtf9", 0x0F),
	},
};
#endif


static void n1_ak8975_init(void)
{
	tegra_gpio_enable(GPIO_AK8975_INT);
	gpio_request(GPIO_AK8975_INT, "ak8975_int");
	gpio_direction_input(GPIO_AK8975_INT);
}

static struct akm8975_platform_data akm8975_pdata = {
	.gpio_data_ready_int = GPIO_AK8975_INT,
};

static struct i2c_board_info n1_i2c_compass_sensor_board_info[] = {
	{
		I2C_BOARD_INFO("ak8975", 0x0C),
		.irq = TEGRA_GPIO_TO_IRQ(GPIO_AK8975_INT),
		.platform_data = &akm8975_pdata,
	},
};
int __init n1_sensors_init(void)
{
	n1_mpu3050_init();
	n1_ak8975_init();
#if defined(CONFIG_MACH_BOSE_ATT)
	if (system_rev < 0x08) {
		i2c_register_board_info(0,
			n1_i2c_mpu_sensor_board_info_bose_rev01,
			ARRAY_SIZE(n1_i2c_mpu_sensor_board_info_bose_rev01));
	} else {
		i2c_register_board_info(0,
			n1_i2c_mpu_sensor_board_info_bose_rev05,
			ARRAY_SIZE(n1_i2c_mpu_sensor_board_info_bose_rev05));
	}
#else
	if (system_rev < 0x05) {
		i2c_register_board_info(0, n1_i2c_mpu_sensor_board_info,
			ARRAY_SIZE(n1_i2c_mpu_sensor_board_info));
		tegra_gpio_enable(n1_i2c_mpu_sensor_board_info[0].irq);
	} else {
		i2c_register_board_info(0, n1_i2c_mpu_sensor_board_info_rev05,
			ARRAY_SIZE(n1_i2c_mpu_sensor_board_info_rev05));
	}
#endif
	i2c_register_board_info(7, n1_i2c_compass_sensor_board_info,
		ARRAY_SIZE(n1_i2c_compass_sensor_board_info));

	return 0;
}
