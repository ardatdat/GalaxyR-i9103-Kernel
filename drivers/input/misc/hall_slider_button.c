/********************************************************************************
*																*
* File Name : hall_slider_button.c								*
* Description: 													*
*	Slider IC Implementation - Polling Mode/Interrupt Mode		*
*																*
* Author    : Aakash Manik 										*
* 			aakash.manik@samsung.com							*
*			Samsung Electronics 								*
*																*
********************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <linux/io.h>
#include <mach/hardware.h>
#include <asm/delay.h>
#include <asm/irq.h>

//#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#if defined (CONFIG_MACH_BOSE_ATT)
#include <mach/gpio-bose.h>
#else
#include <mach/gpio-n1.h>
#endif
//#include <plat/gpio-cfg.h>

#include <linux/wakelock.h>

#define HALL_DEBUG 1
//#define HALL_INT TEGRA_GPIO_PW3

#define DEVICE_NAME "HALL"

#define HALL_INTERRUPT_MODE	1		//Currently configured for interrupt mode, for polling mode make it 0
#define HALL_POLLING_MODE	!(HALL_INTERRUPT_MODE)

#define HALL_INT_IRQ		TEGRA_GPIO_TO_IRQ(GPIO_HALL_INT)

#define LID_CLOSED 1
#define LID_OPEN 0

struct wake_lock hallIC_wake_lock;

/* Hall driver data */
struct hall_data {
	struct input_dev *input_dev;
	struct work_struct work;
	struct hrtimer timer;
	int irq;

};

struct class *hall_class;
struct device *hall_dev;


static int prev_val = -1;
static int cur_val = -1;
int current_hall_ic_state = 0;



static ssize_t hall_show(struct device *dev, struct device_attribute *attr, char *sysfsbuf)
{

	//Get the value
	return sprintf(sysfsbuf, "%d\n", prev_val);

}

static ssize_t hall_store(struct device *dev, struct device_attribute *attr,const char *sysfsbuf, size_t size)
{

	//do nothing
	return size;
}

static DEVICE_ATTR(hallevtcntrl, S_IRUGO | S_IWUSR, hall_show, hall_store);

#if HALL_POLLING_MODE

struct workqueue_struct *hall_workqueue;


/*
Workqueue Function
*/
static void hall_func(struct work_struct *work)
{

	struct hall_data *hall = container_of(work,struct hall_data,work);
	static int check_count = 0;

#if HALL_DEBUG
	printk("Capture GPIO state and report event!!!\n");
#endif
	cur_val = gpio_get_value(GPIO_HALL_INT);
	current_hall_ic_state = cur_val;

	if(cur_val == 0)
		cur_val = LID_CLOSED;
	else
		cur_val = LID_OPEN;

	if(cur_val != prev_val)
	{
		if(check_count > 2){
			check_count = 0;
		input_report_switch(hall->input_dev,SW_LID,cur_val);
		prev_val =	cur_val;

		if(cur_val == 1)
		{
		///Acquire wakelock for 10 sec
		}
		else
		{
		///Release wakelock in 2 sec
		}
			printk("Change HALL IC state\n",prev_val,cur_val);
		}else
			check_count++;
	}

	//printk("HALL GPIO state prev_val %d cur_val %d\n",prev_val,cur_val);
	return;
}

/*
Workqueue Timer Function
*/

static enum hrtimer_restart hall_timer_func(struct hrtimer *timer)
{
	struct hall_data *hall = container_of(timer, struct hall_data, timer);
	ktime_t hall_polling_time;

	queue_work(hall_workqueue, &hall->work);
	hall_polling_time = ktime_set(0,0);
	hall_polling_time = ktime_add_us(hall_polling_time,100000);
	hrtimer_start(&hall->timer,hall_polling_time,HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

#endif //HALL_POLLING_MODE

#if HALL_INTERRUPT_MODE
struct workqueue_struct *hall_workqueue;

/*
IRQ Sched Func
*/
struct hall_data *hall_int_data;

static void hall_int_func(struct work_struct *work)
{
	int check_hall_ic_state = 0;
	int check_count = 0;

    #define COUNT_MAX	3
    #define DEBOUNCE	10

#if HALL_DEBUG
//	printk("Capture GPIO state and report event!!!\n");
#endif
	cur_val = gpio_get_value(GPIO_HALL_INT);

	// check Hall IC state 3 times for 300ms to confirm the state is correct
	for(check_count = 0; check_count <COUNT_MAX; check_count++)
	{
		//msleep(50);
		//mdelay(100);

		check_hall_ic_state = gpio_get_value(GPIO_HALL_INT);

		if(cur_val == check_hall_ic_state){
//			printk("HALL GPIO state is changed. cur_val = %d, check_hall_ic_state = %d\n",cur_val,check_hall_ic_state);
			continue;
		}else{
			return;
		}
	}

	if(cur_val == 0)
		cur_val = LID_CLOSED;
	else
		cur_val = LID_OPEN;

	if(cur_val != prev_val)
	{
#if 0 // BL controlled by framework
		if (cur_val==1)
		{
			tegra_gpio_enable(GPIO_QKEY_BL_EN);
			gpio_direction_output(GPIO_QKEY_BL_EN,0);
		}
		else
		{
			tegra_gpio_enable(GPIO_QKEY_BL_EN);
			gpio_direction_output(GPIO_QKEY_BL_EN,1);
		}
#endif
		input_report_switch(hall_int_data->input_dev,SW_LID,cur_val);
		input_sync(hall_int_data->input_dev);

		printk("[HALL] GPIO state is changed. input_report_switch.. prev_val %d cur_val %d\n",prev_val,cur_val);

		if(cur_val == 0)
			current_hall_ic_state = 1;
		else
			current_hall_ic_state = 0;

		prev_val =	cur_val;

		if(cur_val == 1)
		{
		///Acquire wakelock for 10 sec
		}
		else
		{
		///Release wakelock in 2 sec
		}
	}else
		printk("HALL GPIO state is not changed. prev_val %d cur_val %d\n",prev_val,cur_val);

	return;
}


/*
IRQ Handler
*/
static irqreturn_t hall_func(int irq, void* dev_id)
{

	struct hall_data *hall = dev_id;

	printk("[%s] - Hall IC interrupt occurs\n", __func__);

	wake_lock(&hallIC_wake_lock);

	hall_int_data = hall;

	queue_work(hall_workqueue, &hall->work);

	wake_lock_timeout(&hallIC_wake_lock, 2 * HZ);

	return IRQ_HANDLED;
}

#endif //HALL_INTERRUPT_MODE


static int __init hall_probe(struct platform_device *pdev)
{

	struct hall_data *hall;
	//ktime_t hall_polling_time;


	int ret;


	hall = kzalloc(sizeof(struct hall_data), GFP_KERNEL);
	if (!hall)
	{
		return -ENOMEM;

	}
	hall->input_dev = input_allocate_device();
	if(!hall->input_dev)
	{
		return -ENOMEM;
	}

	set_bit(SW_LID, hall->input_dev->swbit);		//NAGSM_Android_SEL_Kernel_Aakash_20100913
	set_bit(EV_SW, hall->input_dev->evbit);		//NAGSM_Android_SEL_Kernel_Aakash_20100913


	platform_set_drvdata(pdev, hall);


	/* create and register the input driver */



	hall->input_dev->name = DEVICE_NAME;
	hall->input_dev->phys = "hall/input0";

	hall->input_dev->id.bustype = BUS_HOST;
	hall->input_dev->id.vendor = 0x0000;
	hall->input_dev->id.product = 0x0000;
	hall->input_dev->id.version = 0x0000;

	ret = input_register_device(hall->input_dev);
	if (ret) {
		printk("Unable to register s3c-keypad input device!!!\n");
		return ret;
	}


#if HALL_POLLING_MODE

	ktime_t hall_polling_time;
	s3c_gpio_cfgpin(GPIO_HALL_INT, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_HALL_INT, S3C_GPIO_PULL_NONE);

	hall_workqueue = create_singlethread_workqueue("hall_workqueue");
    if (!hall_workqueue)
	    return -ENOMEM;
    INIT_WORK(&hall->work, hall_func);



	hrtimer_init(&hall->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hall->timer.function = hall_timer_func;


	hall_polling_time = ktime_set(0,0);
	hall_polling_time = ktime_add_us(hall_polling_time,100000);
    hrtimer_start(&hall->timer,hall_polling_time,HRTIMER_MODE_REL);


#endif


#if HALL_INTERRUPT_MODE

	hall_workqueue = create_singlethread_workqueue("hall_workqueue");
    if (!hall_workqueue)
	    return -ENOMEM;
    INIT_WORK(&hall->work, hall_int_func);


//	s3c_gpio_cfgpin(GPIO_HALL_INT, S3C_GPIO_SFN(3));
//	s3c_gpio_setpull(GPIO_HALL_INT, S3C_GPIO_PULL_NONE);
	irq_set_irq_type(HALL_INT_IRQ, IRQ_TYPE_EDGE_BOTH);
	//set_irq_type(GPIO_HALL_INT, IRQ_TYPE_EDGE_BOTH);

	ret = request_irq(HALL_INT_IRQ, hall_func, IRQF_DISABLED, DEVICE_NAME, hall);
	if(ret){
	printk("Unable to request IRQ\n");
	return ret;
	}
	hall->irq = HALL_INT_IRQ;

	disable_irq(hall->irq);
	mdelay(100);
	enable_irq(hall->irq);
#endif

	ret = enable_irq_wake(HALL_INT_IRQ);
	if (ret < 0)
		pr_err("[HALL] failed to enable wakeup src %d\n", ret);

	hall_class= class_create(THIS_MODULE, "hallevtcntrl");
	if (IS_ERR(hall_class))
		pr_err("Failed to create class(hallevtcntrl)!\n");

	hall_dev= device_create(hall_class, NULL, 0, NULL, "hall_control");
	if (IS_ERR(hall_dev))
		pr_err("Failed to create device(hall_control)!\n");
	if (device_create_file(hall_dev, &dev_attr_hallevtcntrl) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_hallevtcntrl.attr.name);

	//Initialize wakelock

	cur_val = gpio_get_value(GPIO_HALL_INT);
	current_hall_ic_state = cur_val;

	if(cur_val == 0)
		cur_val = LID_CLOSED;
	else
		cur_val = LID_OPEN;

	input_report_switch(hall->input_dev,SW_LID,cur_val);
	prev_val =	cur_val;

	return 0;
}

static int hall_remove(struct platform_device *pdev)
{
	struct input_dev *input_dev = platform_get_drvdata(pdev);

	input_unregister_device(input_dev);
	kfree(pdev->dev.platform_data);
	free_irq(0, (void *) pdev);

	printk(DEVICE_NAME " Removed.\n");
	return 0;
}


static int hall_suspend(struct platform_device *pdev, pm_message_t state)
{
	//struct input_dev *input_dev = platform_get_drvdata(pdev);

	//Release wakelock in 2 secs

	printk(DEVICE_NAME " Suspend.\n");
	return 0;
}


static int hall_resume(struct platform_device *pdev)
{
	//struct input_dev *input_dev = platform_get_drvdata(pdev);

	//Acquire wakelock for 10 sec

	printk(DEVICE_NAME " Resume.\n");
	return 0;
}

static struct platform_driver hall_driver = {
	.probe		= hall_probe,
	.remove		= hall_remove,
	.suspend	= hall_suspend,
	.resume		= hall_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "hall",
	},
};

static int __init hall_init(void)
{
	int ret;

	wake_lock_init(&hallIC_wake_lock, WAKE_LOCK_SUSPEND, "hallIC_wakelock");

	ret = platform_driver_register(&hall_driver);

	if(!ret)
	   printk(KERN_INFO "Hall Driver\n");

	return ret;
}

static void __exit hall_exit(void)
{
	platform_driver_unregister(&hall_driver);
}

module_init(hall_init);
module_exit(hall_exit);

MODULE_AUTHOR("Aakash Manik");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Slider interface for Hall IC");
