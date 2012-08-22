#ifndef __BOSE_CYPRESS_GPIO_H__
#define __BOSE_CYPRESS_GPIO_H__



//#define _3_GPIO_TOUCH_EN		-1
#define _3_GPIO_TOUCH_INT		GPIO_TKEY_INT
//#define _3_GPIO_TOUCH_INT_AF	S3C_GPIO_SFN(0xf)
#define _3_TOUCH_SDA_28V		GPIO_TKEY_I2C_SDA
#define _3_TOUCH_SCL_28V		GPIO_TKEY_I2C_SCL



#define IRQ_TOUCH_INT			gpio_to_irq(GPIO_TKEY_INT)







#endif
