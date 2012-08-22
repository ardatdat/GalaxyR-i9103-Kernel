/*
*  drivers/input/touchscreen/atmel_mxt1224E_cfg.h
*
*  Copyright (c) 2010 Samsung Electronics Co., LTD.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*/
#ifndef __ATMEL_MXT224E_CFG_H
#define __ATMEL_MXT224E_CFG_H

typedef struct {
	uint8_t reset;       /*  Force chip reset             */
	uint8_t backupnv;    /*  Force backup to eeprom/flash */
	uint8_t calibrate;   /*  Force recalibration          */
	uint8_t reportall;   /*  Force all objects to report  */
	uint8_t reserved;
	uint8_t diagnostic;  /*  Controls the diagnostic object */
} __packed gen_commandprocessor_t6_config_t;

typedef struct {
	uint8_t idleacqint;    /*  Idle power mode sleep length in ms           */
	uint8_t actvacqint;    /*  Active power mode sleep length in ms         */
	uint8_t actv2idleto;   /*  Active to idle power mode delay length in units of 0.2s*/

} __packed gen_powerconfig_t7_config_t;

typedef struct {
	uint8_t chrgtime;          /*  Charge-transfer dwell time             */
	uint8_t reserved;          /*  reserved                               */
	uint8_t tchdrift;          /*  Touch drift compensation period        */
	uint8_t driftst;           /*  Drift suspend time                     */
	uint8_t tchautocal;        /*  Touch automatic calibration delay in units of 0.2s*/
	uint8_t sync;              /*  Measurement synchronisation control    */
	uint8_t atchcalst;         /*  recalibration suspend time after last detection */
	uint8_t atchcalsthr;       /*  Anti-touch calibration suspend threshold */
	uint8_t atchcalfrcthr;
	uint8_t atchcalfrcratio;
} __packed gen_acquisitionconfig_t8_config_t;

typedef struct {
	/* Screen Configuration */
	uint8_t ctrl;            /*  ACENABLE LCENABLE Main configuration field  */

	/* Physical Configuration */
	uint8_t xorigin;         /*  LCMASK ACMASK Object x start position on matrix  */
	uint8_t yorigin;         /*  LCMASK ACMASK Object y start position on matrix  */
	uint8_t xsize;           /*  LCMASK ACMASK Object x size (i.e. width)         */
	uint8_t ysize;           /*  LCMASK ACMASK Object y size (i.e. height)        */

	/* Detection Configuration */
	uint8_t akscfg;          /*  Adjacent key suppression config     */
	uint8_t blen;            /*  Sets the gain of the analog circuits in front of the ADC. The gain should be set in
				  conjunction with the burst length to optimize the signal acquisition. Maximum gain values for
				  a given object/burst length can be obtained following a full calibration of the system. GAIN
				 has a maximum setting of 4; settings above 4 are capped at 4.*/
	uint8_t tchthr;          /*  ACMASK Threshold for all object channels   */
	uint8_t tchdi;           /*  Detect integration config           */

	uint8_t orient;		/*  LCMASK Controls flipping and rotating of touchscreen
				*   object */
	uint8_t mrgtimeout;	/*  Timeout on how long a touch might ever stay
				*   merged - units of 0.2s, used to tradeoff power
				*   consumption against being able to detect a touch
				*   de-merging early */

				  /* Position Filter Configuration */
	uint8_t movhysti;   /*  Movement hysteresis setting used after touchdown */
	uint8_t movhystn;   /*  Movement hysteresis setting used once dragging   */
	uint8_t movfilter;  /*  Position filter setting controlling the rate of  */

			  /* Multitouch Configuration */
	uint8_t numtouch;   /*  The number of touches that the screen will attempt
			  *   to track */
	uint8_t mrghyst;    /*  The hysteresis applied on top of the merge threshold
			  *   to stop oscillation */
	uint8_t mrgthr;     /*  The threshold for the point when two peaks are
			  *   considered one touch */

	uint8_t amphyst;          /*  TBD */

			  /* Resolution Controls */
	uint16_t xrange;       /*  LCMASK */
	uint16_t yrange;       /*  LCMASK */
	uint8_t xloclip;       /*  LCMASK */
	uint8_t xhiclip;       /*  LCMASK */
	uint8_t yloclip;       /*  LCMASK */
	uint8_t yhiclip;       /*  LCMASK */
	/* edge correction controls */
	uint8_t xedgectrl;     /*  LCMASK */
	uint8_t xedgedist;     /*  LCMASK */
	uint8_t yedgectrl;     /*  LCMASK */
	uint8_t yedgedist;     /*  LCMASK */
	uint8_t jumplimit;
	uint8_t tchhyst;
	uint8_t xpitch;
	uint8_t ypitch;
	uint8_t nexttchdi;
} __packed touch_multitouchscreen_t9_config_t;




typedef struct {
	/* Key Array Configuration */
	uint8_t ctrl;               /*  ACENABLE LCENABLE Main configuration field           */

	/* Physical Configuration */
	uint8_t xorigin;           /*  ACMASK LCMASK Object x start position on matrix  */
	uint8_t yorigin;           /*  ACMASK LCMASK Object y start position on matrix  */
	uint8_t xsize;             /*  ACMASK LCMASK Object x size (i.e. width)         */
	uint8_t ysize;             /*  ACMASK LCMASK Object y size (i.e. height)        */

	/* Detection Configuration */
	uint8_t akscfg;             /*  Adjacent key suppression config     */
	uint8_t blen;               /*  ACMASK Burst length for all object channels*/
	uint8_t tchthr;             /*  ACMASK LCMASK Threshold for all object channels   */
	uint8_t tchdi;              /*  Detect integration config           */
	uint8_t reserved[2];        /*  Spare x2 */
} __packed touch_keyarray_t15_config_t;


typedef struct {
	uint8_t  ctrl;
	uint8_t  cmd;
} __packed spt_comcconfig_t18_config_t;


typedef struct {
	/* GPIOPWM Configuration */
	uint8_t ctrl;             /*  Main configuration field           */
	uint8_t reportmask;       /*  Event mask for generating messages to
				*   the host */
	uint8_t dir;              /*  Port DIR register   */
	uint8_t intpullup;        /*  Port pull-up per pin enable register */
	uint8_t out;              /*  Port OUT register*/
	uint8_t wake;             /*  Port wake on change enable register  */
	uint8_t pwm;              /*  Port pwm enable register    */
	uint8_t period;           /*  PWM period (min-max) percentage*/
	uint8_t duty[4];          /*  PWM duty cycles percentage */
	uint8_t trigger[4];       /*  Trigger for GPIO */
} __packed spt_gpiopwm_t19_config_t;


typedef struct {
	uint8_t ctrl;
	uint8_t xlogrip;
	uint8_t xhigrip;
	uint8_t ylogrip;
	uint8_t yhigrip;
	uint8_t maxtchs;
	uint8_t reserved;
	uint8_t szthr1;
	uint8_t szthr2;
	uint8_t shpthr1;
	uint8_t shpthr2;
	uint8_t supextto;
} __packed proci_gripfacesuppression_t20_config_t;


typedef struct {
	uint8_t ctrl;
	uint8_t reserved;
	uint8_t reserved1;
	int16_t gcaful;
	int16_t gcafll;
	uint8_t actvgcafvalid;        /* LCMASK */
	uint8_t noisethr;
	uint8_t reserved2;
	uint8_t freqhopscale;
	uint8_t freq[5u];
	uint8_t idlegcafvalid;        /* LCMASK */
} __packed procg_noisesuppression_t22_config_t;


typedef struct {
	/* Prox Configuration */
	uint8_t ctrl;               /*  ACENABLE LCENABLE Main configuration field           */

	/* Physical Configuration */
	uint8_t xorigin;           /*  ACMASK LCMASK Object x start position on matrix  */
	uint8_t yorigin;           /*  ACMASK LCMASK Object y start position on matrix  */
	uint8_t xsize;             /*  ACMASK LCMASK Object x size (i.e. width)         */
	uint8_t ysize;             /*  ACMASK LCMASK Object y size (i.e. height)        */
	uint8_t reserved;
	/* Detection Configuration */
	uint8_t blen;               /*  ACMASK Burst length for all object channels*/
	uint16_t fxddthr;             /*  Fixed detection threshold   */
	uint8_t fxddi;              /*  Fixed detection integration  */
	uint8_t average;            /*  Acquisition cycles to be averaged */
	uint16_t mvnullrate;               /*  Movement nulling rate */
	uint16_t mvdthr;               /*  Movement detection threshold */
} __packed touch_proximity_t23_config_t;


typedef struct {
	uint8_t ctrl;
	uint8_t numgest;
	uint16_t gesten;
	uint8_t process;
	uint8_t tapto;
	uint8_t flickto;
	uint8_t dragto;
	uint8_t spressto;
	uint8_t lpressto;
	uint8_t reppressto;
	uint16_t flickthr;
	uint16_t dragthr;
	uint16_t tapthr;
	uint16_t throwthr;
} __packed proci_onetouchgestureprocessor_t24_config_t;


typedef struct {
	uint16_t upsiglim;              /* LCMASK */
	uint16_t losiglim;              /* LCMASK */
} siglim_t;

/*! = Config Structure = */

typedef struct {
	uint8_t  ctrl;                 /* LCENABLE */
	uint8_t  cmd;
	siglim_t siglim[3];            /* T9, T15, T23 */
} __packed spt_selftest_t25_config_t;


typedef struct {
	uint8_t ctrl;          /*  Ctrl field reserved for future expansion */
	uint8_t cmd;           /*  Cmd field for sending CTE commands */
	uint8_t mode;          /*  LCMASK CTE mode configuration field */
	uint8_t idlegcafdepth; /*  LCMASK The global gcaf number of averages when idle */
	uint8_t actvgcafdepth; /*  LCMASK The global gcaf number of averages when active */
	int8_t  voltage;
} __packed spt_cteconfig_t28_config_t;

typedef struct {
	uint8_t data[8];
} __packed spt_userdata_t38_t;

/* MXT224E Added */

typedef struct {
	uint8_t ctrl;          /*  Reserved/ GRIPMODE/ Reserved/ ENABLE */
	uint8_t xlogrip;       /*  Grip suppression X low boundary   */
	uint8_t xhigrip;       /*  Grip suppression X high boundary  */
	uint8_t ylogrip;       /*  Grip suppression Y low boundary   */
	uint8_t yhigrip;       /*  Grip suppression Y high boundary  */
} __packed proci_gripsuppression_t40_config_t;



typedef struct {
	uint8_t ctrl;            /*  ctrl field reserved for future expansion */
	uint8_t apprthr;         /*  Approach threshold */
	uint8_t maxapprarea;     /*  Maximum approach area threshold */
	uint8_t maxtcharea;      /*  Maximum touch area threshold */
	uint8_t supstrength;     /*  Suppression aggressiveness */
	uint8_t supextto;        /*  Suppression extension timeout */
	uint8_t maxnumtchs;      /*  Maximum touches */
	uint8_t shapestrength;   /*  Shaped-based aggressiveness */
} __packed proci_touchsuppression_t42_config_t;


typedef struct {
	uint8_t ctrl;          /*  ctrl field reserved for future expansion */
	uint8_t mode;          /*  X line start position   */
	uint8_t idlesyncsperx; /*  Number of sets of ADC conversions per X when idle  */
	uint8_t actvsyncsperx; /*  Number of sets of ADC conversions per X when active*/
	uint8_t adcspersync;   /*  Number of ADC conversions per sync edge            */
	uint8_t pulsesperadc;  /*  Number of pulses for each ADC conversion           */
	uint8_t xslew;         /*  X pulse slew rate                                  */
	uint16_t syncdelay;
} __packed spt_cteconfig_t46_config_t;


typedef struct {
	uint8_t ctrl;          /*  Reserved ENABLE            */
	uint8_t contmin;       /*  Minimum contact diameter   */
	uint8_t contmax;       /*  Maximum contact diameter   */
	uint8_t stability;     /*  Stability                  */
	uint8_t maxtcharea;    /*  Maximum touch are          */
	uint8_t amplthr;       /*  Maximum touch amplitude    */
	uint8_t styshape;      /*  Stylus shape adjustment    */
	uint8_t hoversup;      /*  Hovering finger suppression*/
	uint8_t confthr;       /*  Confidence threshold       */
	uint8_t syncsperx;     /*  ADC sets per X             */
} __packed proci_stylus_t47_config_t;



typedef struct {
	uint8_t ctrl;                /*  Reserved RPTAPX RPTFREQ RPTEN ENABLE             */
	uint8_t cfg;                 /*  Reserved GCMODE                                  */
	uint8_t calcfg;              /*  INCRST INCBIAS Reserved FIXFREQ MFEN NLEN        */
	uint8_t basefreq;            /*  Base sampling frequency                          */
	uint8_t freq_0;              /*  Frequency Hopping frequency 0                    */
	uint8_t freq_1;              /*  Frequency Hopping frequency 1                    */
	uint8_t freq_2;              /*  Frequency Hopping frequency 2                    */
	uint8_t freq_3;              /*  Frequency Hopping frequency 3                    */
	uint8_t mffreq_2;            /*  Median Filter frequency for second filter frame  */
	uint8_t mffreq_3;            /*  Median Filter frequency for third filter frame   */
	uint8_t nlgain;              /*  GAIN Reserved                                    */
	uint8_t nlthr;               /*  Noise line threshold                             */
	uint8_t gclimit;             /*  Grass cut limit                                  */
	uint8_t gcactvinvldadcs;     /*  Grass cut valid ADCs                             */
	uint8_t gcidleinvldadcs;     /*  Grass cut valid threshold                        */
	uint16_t gcinvalidthr;        /*  Grass-cutting source threshold                   */
	uint8_t gcmaxadcsperx;       /*  Max ADCs per X line                              */
	uint8_t gclimitmin ;
	uint8_t gclimitmax ;
	uint16_t gccountmintgt ;
	uint8_t mfinvlddiffthr ;
	uint16_t mfincadcspxthr ;
	uint16_t mferrorthr ;
	uint8_t selfreqmax ;
	uint8_t reserved9 ;
	uint8_t reserved10 ;
	uint8_t reserved11 ;
	uint8_t reserved12 ;
	uint8_t reserved13 ;
	uint8_t reserved14 ;
	uint8_t blen ;
	uint8_t tchthr ;
	uint8_t tchdi ;
	uint8_t movhysti ;
	uint8_t movhystn ;
	uint8_t movfilter ;
	uint8_t numtouch ;
	uint8_t mrghyst ;
	uint8_t mrgthr ;
	uint8_t xloclip ;
	uint8_t xhiclip ;
	uint8_t yloclip ;
	uint8_t yhiclip ;
	uint8_t xedgectrl ;
	uint8_t xedgedist ;
	uint8_t yedgectrl ;
	uint8_t yedgedist ;
	uint8_t jumplimit ;
	uint8_t tchhyst ;
	uint8_t nexttchdi ;
} __packed procg_noisesuppression_t48_config_t;

int mxt_config_settings(struct mxt_data *mxt);
int mxt_get_object_values(struct mxt_data *mxt, int obj_type);
int mxt_copy_object(struct mxt_data *mxt, u8 *buf, int obj_type);
int mxt_load_firmware(struct device *dev, const char *fn);
int mxt_power_config(struct mxt_data *mxt);
int mxt_multitouch_config(struct mxt_data *mxt);
#endif  /* __ATMEL_MXT224E_CFG_H */
