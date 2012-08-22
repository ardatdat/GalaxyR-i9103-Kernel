/*
*  Atmel maXTouch header file
*
*  Copyright (c) 2010 Iiro Valkonen <iiro.valkonen@atmel.com>
*  Copyright (c) 2010 Ulf Samuelsson <ulf.samuelsson@atmel.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 or 3 as
*  published by the Free Software Foundation.
*  See the file "COPYING" in the main directory of this archive
*  for more details.
*
*/
#include <linux/earlysuspend.h>
#include <linux/wakelock.h>
#include <mach/pinmux.h>


/*For Special Test*/
#if !defined (CONFIG_MACH_BOSE_ATT)
#define MXT_FIRMUP_ENABLE	/* for auto firmware upgrade */
#endif
#if 0
#define MXT_TUNNING_ENABLE  /* for TS tunning using console  */
#endif

/*For Factory*/
#define MXT_FACTORY_TEST  /*for TS factory test */
#define ENABLE_NOISE_TEST_MODE	0

/*For Performance*/
#define MXT_DRIVER_FILTER
#define MXT_THREADED_IRQ

/*Normal Feature*/
#define MXT_SLEEP_POWEROFF
#define MXT_ESD_WORKAROUND
#define KEY_LED_CONTROL

/* TSP Key Feature*/
#define NUMOFKEYS       2
#define NUMOF4KEYS       4
#define KEY_PRESS       1
#define KEY_RELEASE     0

/* 2 Touch key array */
#define 	TOUCH_KEY_NULL		0x00
#define	TOUCH_2KEY_MENU	0x01
#define	TOUCH_2KEY_BACK		0x02

/* 4 Touch key array */
#define	TOUCH_4KEY_MENU	0x01
#define	TOUCH_4KEY_HOME	0x02
#define	TOUCH_4KEY_BACK		0x03
#define	TOUCH_4KEY_SEARCH	0x04

#define MXT_I2C_APP_ADDR        0x4A
#define MXT_I2C_BOOTLOADER_ADDR 0x24

#define MXT_I2C_SPEED_KHZ  400
#define MXT_I2C_MAX_LENGTH 300   /* TO_CHK: Is this a correct value? */

#define MAXTOUCH_FAMILYID                               0x81/*0x80*/
#define MXT224_CAL_VARIANTID                            0x01
#define MXT224_UNCAL_VARIANTID                          0x00

#define MXT_MAX_X_VAL_12_BIT                            4095
#define MXT_MAX_Y_VAL_12_BIT                            4095
#define MXT_MAX_X_VAL_10_BIT                            1023
#define MXT_MAX_Y_VAL_10_BIT                            1023


#define MXT_MAX_REPORTED_WIDTH                          255
#define MXT_MAX_REPORTED_PRESSURE                       255


#define MXT_MAX_TOUCH_SIZE                              255
#define MXT_MAX_NUM_TOUCHES                             10  /* 10 finger */

/* Fixed addresses inside maXTouch device */
#define MXT_ADDR_INFO_BLOCK                             0
#define MXT_ADDR_OBJECT_TABLE                           7
#define MXT_ID_BLOCK_SIZE                               7
#define MXT_OBJECT_TABLE_ELEMENT_SIZE                   6
/* Object types */
#define MXT_GEN_MESSAGEPROCESSOR_T5                     5u
#define MXT_GEN_COMMANDPROCESSOR_T6                     6u
#define MXT_GEN_POWERCONFIG_T7                          7u
#define MXT_GEN_ACQUIRECONFIG_T8                        8u
#define MXT_TOUCH_MULTITOUCHSCREEN_T9                   9u
#define MXT_TOUCH_KEYARRAY_T15                          15u
#define MXT_SPT_COMMSCONFIG_T18                         18u
#define MXT_SPT_GPIOPWM_T19                             19u
#define MXT_TOUCH_PROXIMITY_T23                         23u
#define MXT_PROCI_ONETOUCHGESTUREPROCESSOR_T24          24u
#define MXT_SPT_SELFTEST_T25                            25u
#define MXT_DEBUG_DIAGNOSTICS_T37                       37u
#define MXT_USER_INFO_T38                               38u

#define MXT_SPT_COMMSCONFIG_T18                         18u
#define MXT_PROCI_GRIPSUPPRESSION_T40                   40u  /* added MXT224E */
#define MXT_PROCI_TOUCHSUPPRESSION_T42                  42u  /* added MXT224E */
#define MXT_SPT_CTECONFIG_T46                           46u  /* added MXT224E */
#define MXT_PROCI_STYLUS_T47                            47u  /* added MXT224E */
#define MXT_PROCG_NOISESUPPRESSION_T48                  48u  /* added MXT224E */
#define MXT_GEN_INFOBLOCK16BIT_T254                     254u  /*added MXT224E */
#define	MXT_MAX_OBJECT_TYPES				50u	/* temporary */


#define MXT_END_OF_MESSAGES                             255u

/* Configuration Object Adress Fields */
/* GEN_MESSAGEPROCESSOR_T5  Address Definitions         */
/* T5 does not have any configuration */

/* GEN_COMMANDPROCESSOR_T6  Address Definitions         */
#define MXT_ADR_T6_RESET                                0x00
#define MXT_ADR_T6_BACKUPNV                             0x01
#define MXT_ADR_T6_CALIBRATE                            0x02
#define MXT_ADR_T6_REPORTALL                            0x03
#define MXT_ADR_T6_RESERVED                             0x04
#define MXT_ADR_T6_DIAGNOSTICS                          0x05
/* T6 Diagnostics Debug Command */
#define MXT_CMD_T6_PAGE_UP                              0x01
#define MXT_CMD_T6_PAGE_DOWN                            0x02
#define MXT_CMD_T6_DELTAS_MODE                          0x10
#define MXT_CMD_T6_REFERENCES_MODE                      0x11
#define MXT_CMD_T6_CTE_MODE                             0x31

/* GEN_POWERCONFIG_T7 Address Definitions               */
#define MXT_ADR_T7_IDLEACQINT                           0x00
#define MXT_ADR_T7_ACTVACQINT                           0x01
#define MXT_ADR_T7_ACTV2IDLETO                          0x02

/* GEN_ACQUIRECONFIG_T8 Address Definitions             */
#define MXT_ADR_T8_CHRGTIME                             0x00
#define MXT_ADR_T8_RESERVED                             0x01
#define MXT_ADR_T8_TCHDRIFT                             0x02
#define MXT_ADR_T8_DRIFTSTS                             0x03
#define MXT_ADR_T8_TCHAUTOCAL                           0x04
#define MXT_ADR_T8_SYNC                                 0x05
#define MXT_ADR_T8_ATCHCALST                            0x06
#define MXT_ADR_T8_ATCHCALSTHR                          0x07
#define MXT_ADR_T8_ATCHFRCCALTHR                        0x08         /* V2.0 added */
#define MXT_ADR_T8_ATCHFRCCALRATIO                      0x09         /* V2.0 added */

/* TOUCH_MULTITOUCHSCREEN_T9 Address Definitions        */
#define MXT_ADR_T9_CTRL                                 0x00
#define MXT_T9_CFGB_ENABLE(x)                           (((x) >> 0) & 0x01)
#define MXT_T9_CFGB_RPRTEN(x)                           (((x) >> 1) & 0x01)
#define MXT_T9_CFGB_DISAMP(x)                           (((x) >> 2) & 0x01)
#define MXT_T9_CFGB_DISVECT(x)                          (((x) >> 3) & 0x01)
#define MXT_T9_CFGB_DISMOVE(x)                          (((x) >> 4) & 0x01)
#define MXT_T9_CFGB_DISREL(x)                           (((x) >> 5) & 0x01)
#define MXT_T9_CFGB_DISPRSS(x)                          (((x) >> 6) & 0x01)
#define MXT_T9_CFGB_SCANEN(x)                           (((x) >> 7) & 0x01)


#define MXT_T9_ENABLE                                   (0x01)
#define MXT_T9_RPRTEN                                   (0x02)
#define MXT_T9_DISAMP                                   (0x04)
#define MXT_T9_DISVECT                                  (0x08)
#define MXT_T9_DISMOVE                                  (0x10)
#define MXT_T9_DISREL                                   (0x20)
#define MXT_T9_DISPRSS                                  (0x40)
#define MXT_T9_SCANEN                                   (0x80)

#define MXT_ADR_T9_XORIGIN                              0x01
#define MXT_ADR_T9_YORIGIN                              0x02
#define MXT_ADR_T9_XSIZE                                0x03
#define MXT_ADR_T9_YSIZE                                0x04
#define MXT_ADR_T9_AKSCFG                               0x05
#define MXT_ADR_T9_BLEN                                 0x06
#define MXT_T9_CFGBF_BL(x)                              (x & 0x0F)
#define MXT_T9_CFGBF_GAIN(x)                            ((x >> 4) & 0x0F)
#define MXT_ADR_T9_TCHTHR                               0x07
#define MXT_ADR_T9_TCHDI                                0x08
#define MXT_ADR_T9_ORIENT                               0x09
#define MXT_T9_CFGB_SWITCH(x)                           (((x) >> 0) & 0x01)
#define MXT_T9_CFGB_INVERTX(x)                          (((x) >> 1) & 0x01)
#define MXT_T9_CFGB_INVERTY(x)                          (((x) >> 2) & 0x01)
#define MXT_ADR_T9_MRGTIMEOUT                           0x0a
#define MXT_ADR_T9_MOVHYSTI                             0x0b
#define MXT_ADR_T9_MOVHYSTN                             0x0c
#define MXT_ADR_T9_MOVFILTER                            0x0d
#define MXT_T9_CFGBF_ADAPTTHR(x)                        (((x) >> 0) & 0xF)
#define MXT_T9_CFGB_DISABLE(x)                          (((x) >> 7) & 0x01)
#define MXT_ADR_T9_NUMTOUCH                             0x0e
#define MXT_ADR_T9_MRGHYST                              0x0f
#define MXT_ADR_T9_MRGTHR                               0x10
#define MXT_ADR_T9_AMPHYST                              0x11
/* 16 bit */
#define MXT_ADR_T9_XRANGE                               0x12
/* 16 bit */
#define MXT_ADR_T9_YRANGE                               0x14
#define MXT_ADR_T9_XLOCLIP                              0x16
#define MXT_ADR_T9_XHICLIP                              0x17
#define MXT_ADR_T9_YLOCLIP                              0x18
#define MXT_ADR_T9_YHICLIP                              0x19
#define MXT_ADR_T9_XEDGECTRL                            0x1a
#define MXT_ADR_T9_XEDGEDIST                            0x1b
#define MXT_ADR_T9_YEDGECTRL                            0x1c
#define MXT_ADR_T9_YEDGEDIST                            0x1d

/* TOUCH_KEYARRAY_T15 Address Definitions               */

#define MXT_ADR_T15_CTRL                                0x00
#define MXT_T15_CFGB_ENABLE(x)                         (((x) >> 0) & 0x01)
#define MXT_T15_CFGB_RPRTEN(x)                         (((x) >> 1) & 0x01)
#define MXT_T15_CFGB_INTAKSEN(x)                       (((x) >> 7) & 0x01)
#define MXT_ADR_T15_XORIGIN                             0x01
#define MXT_ADR_T15_YORIGIN                             0x02
#define MXT_ADR_T15_XSIZE                               0x03
#define MXT_ADR_T15_YSIZE                               0x04
#define MXT_ADR_T15_AKSCFG                              0x05
#define MXT_ADR_T15_BLEN                                0x06
#define MXT_T15_CFGBF_BL(x)                             (x & 0x0F)
#define MXT_T15_CFGBF_GAIN(x)                           ((x >> 4) & 0x0F)
#define MXT_ADR_T15_TCHTHR                              0x07
#define MXT_ADR_T15_TCHDI                               0x08
#define MXT_ADR_T15_RESERVED1                           0x09
#define MXT_ADR_T15_RESERVED2                           0x0a


/* Adress Definitions for SPT_GPIOPWM_T19 Address Definitions           */
#define MXT_ADR_T19_CTRL                                0x00
#define MXT_ADR_T19_REPORTMASK                          0x01
#define MXT_ADR_T19_DIR                                 0x02
#define MXT_ADR_T19_INTPULLUP                           0x03
#define MXT_ADR_T19_OUT                                 0x04
#define MXT_ADR_T19_WAKE                                0x05
#define MXT_ADR_T19_PWM                                 0x06
#define MXT_ADR_T19_PERIOD                              0x07
/* 32 bit */
#define MXT_ADR_T19_DUTY                                0x08

/* PROCI_GRIPFACESUPPRESSION_T20 Address Definitions            */
#define MXT_ADR_T20_CTRL                                0x00
#define MXT_ADR_T20_XLOGRIP                             0x01
#define MXT_ADR_T20_XHIGRIP                             0x02
#define MXT_ADR_T20_YLOGRIP                             0x03
#define MXT_ADR_T20_YHIGRIP                             0x04
#define MXT_ADR_T20_MAXTCHS                             0x05
#define MXT_ADR_T20_RESERVED                            0x06
#define MXT_ADR_T20_SZTHR1                              0x07
#define MXT_ADR_T20_SZTHR2                              0x08
#define MXT_ADR_T20_SHPTHR1                             0x09
#define MXT_ADR_T20_SHPTHR2                             0x0a
#define MXT_ADR_T20_SUPEXTTO                            0x0b

/* PROCG_NOISESUPPRESSION_T22 Address Definitions               */
#define MXT_ADR_T22_CTRL                                0x00
/* 16 bit */
#define MXT_ADR_T22_RESERVED1_2                         0x01
/* 16 bit */
#define MXT_ADR_T22_GCAFUL                              0x03
/* 16 bit */
#define MXT_ADR_T22_GCAFLL                              0x05
#define MXT_ADR_T22_ACTVGCAFVALID                       0x07
#define MXT_ADR_T22_NOISETHR                            0x08
#define MXT_ADR_T22_RESERVED9                           0x09
#define MXT_ADR_T22_FREQHOPSCALE                        0x0A
/* 5 bytes */
#define MXT_ADR_T22_FREQ                                0x0B
#define MXT_ADR_T22_IDLEGCAFVALID                       0x10

/* TOUCH_PROXIMITY_T23 Address Definitions              */
#define MXT_ADR_T23_CTRL                                0x00
#define MXT_ADR_T23_XORIGIN                             0x01
#define MXT_ADR_T23_YORIGIN                             0x02
#define MXT_ADR_T23_XSIZE                               0x03
#define MXT_ADR_T23_YSIZE                               0x04
#define MXT_ADR_T23_RESERVED                            0x05
#define MXT_ADR_T23_BLEN                                0x06
#define MXT_ADR_T23_TCHTHR                              0x07
#define MXT_ADR_T23_TCHDI                               0x09
#define MXT_ADR_T23_AVERAGE                             0x0a
/* 16 bit */
#define MXT_ADR_T23_RATE                                0x0b

/* PROCI_ONETOUCHGESTUREPROCESSOR_T24 Address Definitions               */
#define MXT_ADR_T24_CTRL                                0x00
#define MXT_ADR_T24_NUMGEST                             0x01
/* 16 bit */
#define MXT_ADR_T24_GESTEN                              0x02
#define MXT_ADR_T24_PRESSPROC                           0x04
#define MXT_ADR_T24_TAPTO                               0x05
#define MXT_ADR_T24_FLICKTO                             0x06
#define MXT_ADR_T24_DRAGTO                              0x07
#define MXT_ADR_T24_SPRESSTO                            0x08
#define MXT_ADR_T24_LPRESSTO                            0x09
#define MXT_ADR_T24_REPPRESSTO                          0x0a
/* 16 bit */
#define MXT_ADR_T24_FLICKTHR                            0x0b
/* 16 bit */
#define MXT_ADR_T24_DRAGTHR                             0x0d
/* 16 bit */
#define MXT_ADR_T24_TAPTHR                              0x0f
/* 16 bit */
#define MXT_ADR_T24_THROWTHR                            0x11

/* SPT_SELFTEST_T25 Address Definitions         */
#define MXT_ADR_T25_CTRL                                0x00
#define MXT_ADR_T25_CMD                                 0x01
/* 16 bit */
#define MXT_ADR_T25_HISIGLIM0                           0x02
/* 16 bit */
#define MXT_ADR_T25_LOSIGLIM0                           0x04

/* PROCI_TWOTOUCHGESTUREPROCESSOR_T27 Address Definitions               */
#define MXT_ADR_T27_CTRL                                0x00
#define MXT_ADR_T27_NUMGEST                             0x01
#define MXT_ADR_T27_RESERVED2                           0x02
#define MXT_ADR_T27_GESTEN                              0x03
#define MXT_ADR_T27_ROTATETHR                           0x04

/* 16 bit */
#define MXT_ADR_T27_ZOOMTHR                             0x05

/* SPT_CTECONFIG_T28 Address Definitions                */
#define MXT_ADR_T28_CTRL                                0x00
#define MXT_ADR_T28_CMD                                 0x01
#define MXT_ADR_T28_MODE                                0x02
#define MXT_ADR_T28_IDLEGCAFDEPTH                       0x03
#define MXT_ADR_T28_ACTVGCAFDEPTH                       0x04

/* DEBUG_DIAGNOSTICS_T37 Address Definitions            */
#define MXT_ADR_T37_MODE                                0x00
#define MXT_ADR_T37_PAGE                                0x01
#define MXT_ADR_T37_DATA                                0x02


#define MXT_ADR_T38_USER0                               0x00
#define MXT_ADR_T38_USER1                               0x01
#define MXT_ADR_T38_USER2                               0x02
#define MXT_ADR_T38_USER3                               0x03
#define MXT_ADR_T38_USER4                               0x04
#define MXT_ADR_T38_USER5                               0x05
#define MXT_ADR_T38_USER6                               0x06
#define MXT_ADR_T38_USER7                               0x07


/* PROCI_GRIPSUPPRESSION_T40 */

#define MXT_ADR_T40_CTRL                                0x00
#define MXT_ADR_T40_XLOGRIP                             0x01
#define MXT_ADR_T40_XHIGRIP                             0x02
#define MXT_ADR_T40_YLOGRIP                             0x03
#define MXT_ADR_T40_YHIGRIP                             0x04

/* PROCI_TOUCHSUPPRESSION_T42 */
#define MXT_ADR_T42_CTRL                                0x00
#define MXT_ADR_T42_APPRTHR                             0x01   /* 0 (TCHTHR/4), 1 to 255 */
#define MXT_ADR_T42_MAXAPPRAREA                         0x02   /* 0 (40ch), 1 to 255 */
#define MXT_ADR_T42_MAXTCHAREA                          0x03   /* 0 (35ch), 1 to 255 */
#define MXT_ADR_T42_SUPSTRENGTH                         0x04   /* 0 (128), 1 to 255 */
#define MXT_ADR_T42_SUPEXTTO                            0x05   /* 0 (never expires), 1 to 255 (timeout in cycles) */
#define MXT_ADR_T42_MAXNUMTCHS                          0x06   /* 0 to 9 (maximum number of touches minus 1) */
#define MXT_ADR_T42_SHAPESTRENGTH                       0x07   /* 0 (10), 1 to 31 */


/* SPT_CTECONFIG_T46  */
#define MXT_ADR_T46_CTRL                                0x00  /* Reserved */
#define MXT_T46_CFGB_ENABLE(x)                         (((x) >> 0) & 0x01)
#define MXT_ADR_T46_MODE                                0x01  /* 0: 16X14Y, 1: 17X13Y, 2: 18X12Y, 3: 19X11Y, 4: 20X10Y, 5: 21X15Y, 6: 22X8Y, */
#define MXT_ADR_T46_IDLESYNCSPERX                       0x02
#define MXT_ADR_T46_ACTVSYNCSPERX                       0x03
#define MXT_ADR_T46_ADCSPERSYNC                         0x04
#define MXT_ADR_T46_PULSESPERADC                        0x05  /* 0:1  1:2   2:3   3:4 pulses */
#define MXT_ADR_T46_XSLEW                               0x06  /* 1:500nsec,  0:350nsec */


/* PROCI_STYLUS_T47 */
#define MXT_ADR_T47_CTRL                                0x00
#define MXT_ADR_T47_CONTMIN                             0x01
#define MXT_ADR_T47_CONTMAX                             0x02
#define MXT_ADR_T47_STABILITY                           0x03
#define MXT_ADR_T47_MAXTCHAREA                          0x04
#define MXT_ADR_T47_AMPLTHR                             0x05
#define MXT_ADR_T47_STYSHAPE                            0x06
#define MXT_ADR_T47_HOVERSUP                            0x07
#define MXT_ADR_T47_CONFTHR                             0x08
#define MXT_ADR_T47_SYNCSPERX                           0x09


/* MXT_PROCG_NOISESUPPRESSION_T48 Message address fifinitions */
/* add definition for firmware ver0.5  */

#define MXT_ADR_T48_CTRL                                0x00
#define MXT_T48_CFGB_ENABLE(x)                         (((x) >> 0) & 0x01)
#define MXT_T48_CFGB_RPRTEN(x)                         (((x) >> 1) & 0x01)
#define MXT_T48_CFGB_RPTFREQ(x)                        (((x) >> 2) & 0x01)
#define MXT_T48_CFGB_RPTAPX(x)                         (((x) >> 3) & 0x01)

#define MXT_ADR_T48_CFG					0x01

#define MXT_ADR_T48_CALCFG                              0x02
#define MXT_T48_CFGB_MFEN(x)                            (((x) >> 1) & 0x01)
#define MXT_T48_CFGB_MEANEN(x)                          (((x) >> 3) & 0x01)
#define MXT_T48_CFGB_DUALXEN(x)                         (((x) >> 4) & 0x01)
#define MXT_T48_CFGB_CHRGON(x)                          (((x) >> 5) & 0x01)
#define MXT_T48_CFGB_INCBIAS(x)                         (((x) >> 6) & 0x01)
#define MXT_T48_CFGB_INCRST(x)                          (((x) >> 7) & 0x01)


#define MXT_ADR_T48_BASEFREQ				0x03
#define MXT_ADR_T48_RESERVED0				0x04
#define MXT_ADR_T48_RESERVED1				0x05
#define MXT_ADR_T48_RESERVED2				0x06
#define MXT_ADR_T48_RESERVED3				0x07
#define MXT_ADR_T48_MFFREQ_2				0x08
#define MXT_ADR_T48_MFFREQ_3				0x09
#define MXT_ADR_T48_RESERVED4				0x0A
#define MXT_ADR_T48_RESERVED5				0x0B
#define MXT_ADR_T48_RESERVED6				0x0C
#define MXT_ADR_T48_GCACTVINVLDADCS			0x0D
#define MXT_ADR_T48_GCIDLEINVLDADCS			0x0E
#define MXT_ADR_T48_RESERVED7				0x0F
#define MXT_ADR_T48_RESERVED8				0x10
#define MXT_ADR_T48_GCMAXADCSPERX			0x11
#define MXT_ADR_T48_GCLIMITMIN				0x12
#define MXT_ADR_T48_GCLIMITMAX				0x13
#define MXT_ADR_T48_GCCOUNTMINTGT			0x14
#define MXT_ADR_T48_MFINVLDDIFFTHR			0x16
#define MXT_ADR_T48_MFINCADCSPXTHR			0x17
#define MXT_ADR_T48_MFERRORTHR				0x19
#define MXT_ADR_T48_SELFREQMAX				0x1B
#define MXT_ADR_T48_RESERVED9				0x1C
#define MXT_ADR_T48_RESERVED10				0x1D
#define MXT_ADR_T48_RESERVED11				0x1E
#define MXT_ADR_T48_RESERVED12				0x1F
#define MXT_ADR_T48_RESERVED13				0x20
#define MXT_ADR_T48_RESERVED14				0x21
#define MXT_ADR_T48_BLEN				0x22
#define MXT_ADR_T48_TCHTHR				0x23
#define MXT_ADR_T48_TCHDI				0x24
#define MXT_ADR_T48_MOVHYSTI				0x25
#define MXT_ADR_T48_MOVHYSTN				0x26
#define MXT_ADR_T48_MOVFILTER				0x27
#define MXT_ADR_T48_NUMTOUCH				0x28
#define MXT_ADR_T48_MRGHYST				0x29
#define MXT_ADR_T48_MRGTHR				0x2A
#define MXT_ADR_T48_XLOCLIP				0x2B
#define MXT_ADR_T48_XHICLIP				0x2C
#define MXT_ADR_T48_YLOCLIP				0x2D
#define MXT_ADR_T48_YHICLIP				0x2E
#define MXT_ADR_T48_XEDGECTRL				0x2F
#define MXT_ADR_T48_XEDGEDIST				0x30
#define MXT_ADR_T48_YEDGECTRL				0x31
#define MXT_ADR_T48_YEDGEDIST				0x32
#define MXT_ADR_T48_JUMPLIMIT				0x33
#define MXT_ADR_T48_TCHHYST				0x34
#define MXT_ADR_T48_NEXTTCHDI				0x35



/************************************************************************
* MESSAGE OBJECTS ADDRESS FIELDS
*
************************************************************************/
#define MXT_MSG_REPORTID                                0x00


/* MXT_GEN_MESSAGEPROCESSOR_T5 Message address definitions              */
#define MXT_MSG_T5_REPORTID                             0x00
#define MXT_MSG_T5_MESSAGE                              0x01
#define MXT_MSG_T5_CHECKSUM                             0x08

/* MXT_GEN_COMMANDPROCESSOR_T6 Message address definitions              */
#define MXT_MSG_T6_STATUS_NORMAL                        0x00
#define MXT_MSG_T6_STATUS                               0x01
#define MXT_MSGB_T6_COMSERR                             0x04
#define MXT_MSGB_T6_CFGERR                              0x08
#define MXT_MSGB_T6_CAL                                 0x10
#define MXT_MSGB_T6_SIGERR                              0x20
#define MXT_MSGB_T6_OFL                                 0x40
#define MXT_MSGB_T6_RESET                               0x80
/* Three bytes */
#define MXT_MSG_T6_CHECKSUM                             0x02

/* MXT_GEN_POWERCONFIG_T7 NO Message address definitions                */
/* MXT_GEN_ACQUIRECONFIG_T8 Message address definitions                 */
/* MXT_TOUCH_MULTITOUCHSCREEN_T9 Message address definitions            */

#define MXT_MSG_T9_STATUS                               0x01
/* Status bit field */
#define MXT_MSGB_T9_SUPPRESS                            0x02
#define MXT_MSGB_T9_AMP                                 0x04
#define MXT_MSGB_T9_VECTOR                              0x08
#define MXT_MSGB_T9_MOVE                                0x10
#define MXT_MSGB_T9_RELEASE                             0x20
#define MXT_MSGB_T9_PRESS                               0x40
#define MXT_MSGB_T9_DETECT                              0x80

#define MXT_MSG_T9_XPOSMSB                              0x02
#define MXT_MSG_T9_YPOSMSB                              0x03
#define MXT_MSG_T9_XYPOSLSB                             0x04
#define MXT_MSG_T9_TCHAREA                              0x05
#define MXT_MSG_T9_TCHAMPLITUDE                         0x06
#define MXT_MSG_T9_TCHVECTOR                            0x07

/* MXT_TOUCH_KEYARRAY_T15 Message address definitions                   */
#define MXT_MSG_T15_STATUS                              0x01
#define MXT_MSGB_T15_DETECT                             0x80
/* 4 bytes */
#define MXT_MSG_T15_KEYSTATE                            0x02

/* MXT_SPT_GPIOPWM_T19 Message address definitions                      */
#define MXT_MSG_T19_STATUS                              0x01

/* MXT_PROCI_GRIPFACESUPPRESSION_T20 Message address definitions        */
#define MXT_MSG_T20_STATUS                              0x01
#define MXT_MSGB_T20_FACE_SUPPRESS                      0x01
/* MXT_PROCG_NOISESUPPRESSION_T22 Message address definitions           */
#define MXT_MSG_T22_STATUS                              0x01
#define MXT_MSGB_T22_FHCHG                              0x01
#define MXT_MSGB_T22_GCAFERR                            0x04
#define MXT_MSGB_T22_FHERR                              0x08
#define MXT_MSG_T22_GCAFDEPTH                           0x02

/* MXT_TOUCH_PROXIMITY_T23 Message address definitions                  */
#define MXT_MSG_T23_STATUS                              0x01
#define MXT_MSGB_T23_FALL                               0x20
#define MXT_MSGB_T23_RISE                               0x40
#define MXT_MSGB_T23_DETECT                             0x80
/* 16 bit */
#define MXT_MSG_T23_PROXDELTA                           0x02

/* MXT_PROCI_ONETOUCHGESTUREPROCESSOR_T24 Message address definitions   */
#define MXT_MSG_T24_STATUS                              0x01
#define MXT_MSG_T24_XPOSMSB                             0x02
#define MXT_MSG_T24_YPOSMSB                             0x03
#define MXT_MSG_T24_XYPOSLSB                            0x04
#define MXT_MSG_T24_DIR                                 0x05
/* 16 bit */
#define MXT_MSG_T24_DIST                                0x06

/* MXT_SPT_SELFTEST_T25 Message address definitions                     */
#define MXT_MSG_T25_STATUS                              0x01
/* 5 Bytes */
#define MXT_MSGR_T25_OK                                 0xFE
#define MXT_MSGR_T25_INVALID_TEST                       0xFD
#define MXT_MSGR_T25_PIN_FAULT                          0x11
#define MXT_MSGR_T25_SIGNAL_LIMIT_FAULT                 0x17
#define MXT_MSGR_T25_GAIN_ERROR                         0x20
#define MXT_MSG_T25_INFO                                0x02

/* MXT_PROCI_TWOTOUCHGESTUREPROCESSOR_T27 Message address definitions   */
#define MXT_MSG_T27_STATUS                              0x01
#define MXT_MSGB_T27_ROTATEDIR                          0x10
#define MXT_MSGB_T27_PINCH                              0x20
#define MXT_MSGB_T27_ROTATE                             0x40
#define MXT_MSGB_T27_STRETCH                            0x80
#define MXT_MSG_T27_XPOSMSB                             0x02
#define MXT_MSG_T27_YPOSMSB                             0x03
#define MXT_MSG_T27_XYPOSLSB                            0x04
#define MXT_MSG_T27_ANGLE                               0x05

/* 16 bit */
#define MXT_MSG_T27_SEPARATION                          0x06

/* MXT_SPT_CTECONFIG_T28 Message address definitions                    */
#define MXT_MSG_T28_STATUS                              0x01
#define MXT_MSGB_T28_CHKERR                             0x01


/* MXT_PROCI_TOUCHSUPPRESSION_T42 Message address fifinitions */
#define MXT_MSG_T42_STATUS                               0x01
#define MXT_MSGB_T42_TCHSUP				 0x01


/* MXT_SPT_CTECONFIG_T46 Message address fifinitions */
#define MXT_MSG_T46_STATUS                               0x01
#define MXT_MSGB_T46_CHKERR                              0x01



/* MXT_DEBUG_DIAGNOSTICS_T37 NO Message address definitions             */

/* One Touch Events */
#define MT_GESTURE_RESERVED                             0x00
#define MT_GESTURE_PRESS                                0x01
#define MT_GESTURE_RELEASE                              0x02
#define MT_GESTURE_TAP                                  0x03
#define MT_GESTURE_DOUBLE_TAP                           0x04
#define MT_GESTURE_FLICK                                0x05
#define MT_GESTURE_DRAG                                 0x06
#define MT_GESTURE_SHORT_PRESS                          0x07
#define MT_GESTURE_LONG_PRESS                           0x08
#define MT_GESTURE_REPEAT_PRESS                         0x09
#define MT_GESTURE_TAP_AND_PRESS                        0x0a
#define MT_GESTURE_THROW                                0x0b

/* reset mode */
#define RESET_TO_NORMAL                                 0
#define RESET_TO_BOOTLOADER                             1

/* Bootloader states */
#define WAITING_BOOTLOAD_COMMAND                        0xC0
#define WAITING_FRAME_DATA                              0x80
#define FRAME_CRC_CHECK                                 0x02
#define FRAME_CRC_PASS                                  0x04
#define FRAME_CRC_FAIL                                  0x03
#define APP_CRC_FAIL                                    0x40
#define BOOTLOAD_STATUS_MASK                            0x3f

#define MXT_MAX_FRAME_SIZE                              532


#define MXT_ADR_T38_CFG_CTRL 				0x00
#define MXT_CFG_OVERWRITE				0x00
#define MXT_CFG_DEBUG					0xA5


/* Firmware */
#define MXT224E_FIRMWARE "mxt224e.fw"

/* level of debugging messages */
#define DEBUG_INFO      1
#define DEBUG_VERBOSE   2
#define DEBUG_MESSAGES  5
#define DEBUG_RAW       8
#define DEBUG_TRACE     10

#define klogi(fmt, arg...)  printk(KERN_INFO "%s: " fmt "\n" , __func__, ## arg)
#define kloge(fmt, arg...)  printk(KERN_ERR "%s(%d): " fmt "\n" , __func__, __LINE__, ## arg)
#define klogi_if(fmt, arg...) if (1) printk(KERN_INFO "%s: " fmt "\n" , __func__, ## arg)
#define kloge_if(fmt, arg...) if (mxt_debug) printk(KERN_ERR "%s(%d): " fmt "\n" , __func__, __LINE__, ## arg)

#define MIGRATION_TO_NVODM

/* Device Info descriptor */
/* Parsed from maXTouch "Id information" inside device */
struct mxt_device_info {
	u8      family_id;
	u8      variant_id;
	u8      major;
	u8      minor;
	u8      build;
	u8      num_objs;
	u8      x_size;
	u8      y_size;
	u8      family[16];     /* Family name */
	u8      variant[16];    /* Variant name */
	u16     num_nodes;      /* Number of sensor nodes */
};

/* object descriptor table, parsed from maXTouch "object table" */
struct mxt_object {
	u8      type;
	u16     chip_addr;
	u8      size;
	u8      instances;
	u8      num_report_ids;
};

/* Mapping from report id to object type and instance */
struct report_id_map {
	u8  object;
	u8  instance;
	/*
	* This is the first report ID belonging to object. It enables us to
	* find out easily the touch number: each touch has different report
	* ID (which are assigned to touches in increasing order). By
	* subtracting the first report ID from current, we get the touch
	* number.
	*/
	u8  first_rid;
};

struct mxt_callbacks {
	void (*inform_charger)(struct mxt_callbacks *, int mode);
};

struct mxt_platform_data {
	const char *platform_name;
	u8    numtouch; /* Number of touches to report  */
	void  (*init_platform_hw)(void *);
	void  (*exit_platform_hw)(void *);
	void  (*suspend_platform_hw)(void *);
	void  (*resume_platform_hw)(void *);

	u32	key_led_en1;
	u32	key_led_en2;
	u32	key_led_en3;
	u32	key_led_en4;

	int   max_x;    /* The default reported X range   */
	int   max_y;    /* The default reported Y range   */
	u8    (*valid_interrupt) (void);
	void	(*register_cb)(struct mxt_callbacks *);

	const char *reg_vdd_name;
	const char *reg_avdd_name;
	const char *reg_vdd_lvsio_name;

	struct regulator *reg_vdd;  /* TSP_VDD_1V8 */
	struct regulator *reg_avdd;  /* TSP_AVDD_3V3 */
	struct regulator *reg_vdd_lvsio;	/* TSP_VDD_LVSIO_2V8 (for level shifter) */

	int	reg_vdd_level;	/* uV range */
	int	reg_avdd_level;	/* uV range */
	int	reg_vdd_lvsio_level;	/* uV range */

	enum tegra_pingroup i2c_pingroup;
	enum tegra_pullupdown i2c_suspend_pupd;
	enum tegra_pullupdown i2c_resume_pupd;
	u32	irq_gpio;
	u32 board_rev;	/* for set revision check */
};

/* Driver datastructure */
struct mxt_data {
	struct i2c_client *client;
	struct input_dev  *input;
	struct mxt_platform_data *pdata;
	char              phys_name[32];
	int               irq;

	u16               last_read_addr;
	bool              new_msgs;
	u8                *last_message;

	wait_queue_head_t msg_queue;
	struct semaphore  msg_sem;

	int               valid_irq_counter;
	int               invalid_irq_counter;
	int               irq_counter;
	int               message_counter;
	int               read_fail_counter;


	int               bytes_to_read;

	/* Protect canceling/rescheduling of dwork */
	spinlock_t           lock;
	struct delayed_work  dwork;
	struct work_struct ta_work;
	struct work_struct tmr_work;
	struct timer_list ts_timeout_tmr;
	struct timer_list *p_ts_timeout_tmr;
	struct delayed_work  config_dwork;
	struct delayed_work  timer_dwork;
	struct delayed_work  initial_dwork;

	struct delayed_work  timer_get_delta;

#ifdef MXT_FACTORY_TEST
	struct delayed_work  firmup_dwork;
#endif
	u8                   xpos_format;
	u8                   ypos_format;

	struct mxt_device_info  device_info;

	u32                  info_block_crc;
	u32                  configuration_crc;
	u16                  report_id_count;
	struct report_id_map *rid_map;
	struct mxt_object    *object_table;

	u16                  msg_proc_addr;
	u8                   message_size;


	/* u8 debug_config[32]; */

	/* Pointers to diagnostic objects, not allocated, unless needed */
	s16                  *delta;
	u16                  *reference;
	u16                  *cte;

	struct wake_lock wakelock;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend    early_suspend;
#endif
#if defined(MXT_FACTORY_TEST) || defined(MXT_FIRMUP_ENABLE)
	u8			firm_status_data;
	u8			firm_normal_status_ack;
#endif
	u8 check_auto_cal;
	u16			set_mode_for_ta;
	u16			mxt_status;
	struct mxt_callbacks callbacks;
};

/* Returns the start address of object in mXT memory. */
#define MXT_BASE_ADDR(object_type) \
get_object_address(object_type, 0, mxt->object_table, mxt->device_info.num_objs)

/* Returns the size of object in mXT memory. */
#define MXT_GET_SIZE(object_type) \
get_object_size(object_type, mxt->object_table, mxt->device_info.num_objs)

/* If report_id (rid) == 0, then "mxt->report_id[rid].object" will be 0. */
#define	REPORT_ID_TO_OBJECT(rid) \
(((rid) == 0xff) ? 0 : mxt->rid_map[rid].object)

#define	REPORT_ID_TO_OBJECT_NAME(rid) \
object_type_name[REPORT_ID_TO_OBJECT(rid)]

#define	T6_REG(x) (MXT_BASE_ADDR(MXT_GEN_COMMANDPROCESSOR_T6) + (x))
#define	T37_REG(x) (MXT_BASE_ADDR(MXT_DEBUG_DIAGNOSTICS_T37) +  (x))

/* ADD TRACKING_ID */
#define REPORT_MT(touch_number, x, y, amplitude, size)		\
	do {														\
	input_report_abs(mxt->input, ABS_MT_POSITION_X, x);             \
	input_report_abs(mxt->input, ABS_MT_POSITION_Y, y);             \
	input_report_abs(mxt->input, ABS_MT_TOUCH_MAJOR, amplitude);    \
	input_report_abs(mxt->input, ABS_MT_WIDTH_MAJOR, size);         \
	} while (0)

/* Routines for memory access within a 16 bit address space */
int mxt_read_byte(
		  struct i2c_client *client,
		  __u16 addr,
		  __u8 *value
		  );
int mxt_read_block(
		   struct i2c_client *client,
		   __u16 addr,
		   __u16 length,
		   __u8 *value
		   );
int mxt_write_byte(
		   struct i2c_client *client,
		   __u16 addr,
		   __u8 value
		   );
int mxt_write_block(
		    struct i2c_client *client,
		    __u16 addr,
		    __u16 length,
		    __u8 *value
		    );


int mxt_read_byte_bl(
		     struct i2c_client *client,
		     __u8 *value
		     );
int mxt_read_block_bl(
		      struct i2c_client *client,
		      __u16 length,
		      __u8 *value
		      );
int mxt_write_byte_bl(
		      struct i2c_client *client,
		      __u8 value
		      );
int mxt_write_block_bl(
		       struct i2c_client *client,
		       __u16 length,
		       __u8 *value
		       );
int mxt_write_block(struct i2c_client *client, u16 addr, u16 length, u8 *value);
int mxt_read_block(struct i2c_client *client, u16 addr, u16 length, u8 *value);
u16 get_object_address(uint8_t object_type,
		       uint8_t instance,
		       struct mxt_object *object_table,
		       int max_objs);
u16 get_object_size(uint8_t object_type,
		    struct mxt_object *object_table,
		    int max_objs);
int backup_to_nv(struct mxt_data *mxt);
int reset_chip(struct mxt_data *mxt, u8 mode);

#if     1
/* Should be implemented in board support */
u8 mxt_valid_interrupt(void);
#else
#define mxt_valid_interrupt()    1
#endif


void    mxt_hw_reset(void);

#ifdef KEY_LED_CONTROL
#define MENU_LED_2KEY		0xFE
#define BACK_LED_2KEY			0xFD

#define MENU_LED_4KEY		0xFE
#define HOME_LED_4KEY		0xFD
#define BACK_LED_4KEY			0xFB
#define SEARCH_LED_4KEY		0xF7

static void key_led_on(struct mxt_data *mxt, u32 val);
#endif
