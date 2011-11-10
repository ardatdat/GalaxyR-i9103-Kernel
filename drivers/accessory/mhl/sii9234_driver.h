/***************************************************************************

* 

*   SiI9244 - MHL Transmitter Driver

*

* Copyright (C) (2011, Silicon Image Inc)

*

* This program is free software; you can redistribute it and/or modify

* it under the terms of the GNU General Public License as published by

* the Free Software Foundation version 2.

*

* This program is distributed as is WITHOUT ANY WARRANTY of any

* kind, whether express or implied; without even the implied warranty

* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

* GNU General Public License for more details.

*

*****************************************************************************/


/*===========================================================================

                      EDIT HISTORY FOR FILE

when              who                         what, where, why
--------        ---                        ----------------------------------------------------------
2010/11/06    Daniel Lee(Philju)      Initial version of file, SIMG Korea
===========================================================================*/

/*===========================================================================
                     INCLUDE FILES FOR MODULE
===========================================================================*/


/*===========================================================================
                   FUNCTION DEFINITIONS
===========================================================================*/
 
struct mhl_dev {
		struct device					*process_dev;
        unsigned int                    irq_gpio;
        unsigned int                    wake_up_gpio;                                            
        struct work_struct              sii9234_int_work;                                        
        struct workqueue_struct         *sii9234_wq;
        struct mhl_platform_data        *pdata;
        struct miscdevice               mdev;
};

void sii9234_interrupt_event(struct mhl_dev *mhl_dev);
bool sii9234_init(void);

#define MHL_READ_RCP_DATA 0x1

