#ifndef _COMMON_H
#define _COMMON_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <mach/gpio.h>

/* #define Si4709_DEBUG */

#define error(fmt, arg...) printk(KERN_CRIT fmt "\n", ##arg)

#define Si4709_DEBUG
#ifdef Si4709_DEBUG
#define debug(fmt, arg...) printk(KERN_CRIT "--------" fmt "\n", ##arg)
#else
#define debug(fmt, arg...)
#endif

#ifdef CONFIG_MACH_N1
#define FM_RESET_04 GPIO_FM_RST_04
#define FM_RESET_05 GPIO_FM_RST_05
#define FM_PORT		"fm_rst"
#else
#define FM_RESET	GPIO_FM_RST
#define FM_PORT		"GPB"
#endif // CONFIG_MACH_N1

/* VNVS:28-OCT'09 : For testing FM tune and seek operation status */
#define TEST_FM

/* VNVS:7-JUNE'10 : RDS Interrupt ON Always */
/* (Enabling interrupt when RDS is enabled) */
#define RDS_INTERRUPT_ON_ALWAYS

/* VNVS:18-JUN'10 : For testing RDS */
/* Enable only for debugging RDS */
/* #define RDS_TESTING */
#ifdef RDS_TESTING
#define debug_rds(fmt, arg...) printk(KERN_CRIT "--------" fmt "\n", ##arg)
#define GROUP_TYPE_2A     (2 * 2 + 0)
#define GROUP_TYPE_2B     (2 * 2 + 1)
#else
#define debug_rds(fmt, arg...)
#endif

#define YES  1
#define NO  0

#endif
