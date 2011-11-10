/*
 * drivers/media/video/tegra/tegra_camera.c
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <mach/iomap.h>
#include <mach/clk.h>
#include <mach/tegra_cpufreq.h>

#include <media/tegra_camera.h>

/* Eventually this should handle all clock and reset calls for the isp, vi,
 * vi_sensor, and csi modules, replacing nvrm and nvos completely for camera
 */
#define TEGRA_CAMERA_NAME "tegra_camera"
DEFINE_MUTEX(tegra_camera_lock);

struct tegra_camera_block {
	int (*enable) (void);
	int (*disable) (void);
	bool is_enabled;
};


static struct clk *isp_clk;
static struct clk *vi_clk;
static struct clk *vi_sensor_clk;
static struct clk *csus_clk;
#if defined CONFIG_HAS_EARLYSUSPEND && defined CONFIG_CPU_FREQ
#define SET_CONSERVATIVE_GOVERNOR_DELAY 5*HZ
#define SET_CONSERVATIVE_GOVERNOR_UP_THRESHOLD 90
#define SET_CONSERVATIVE_GOVERNOR_DOWN_THRESHOLD 50
static int work_q_set_conservative;
static struct delayed_work scaling_gov_work;
#endif
static struct clk *csi_clk;
#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
static int set_graphics_clk;
static int current_usecase;
static int current_resolution[4];
static unsigned long *current_rates;
static struct clk **graphics_clks;
static struct clk **graphics_restore_clks;
static unsigned long  *graphics_restore_clk_rates;
static struct tegra_camera_clk_config *graphics_clk_cfg;
#endif
static struct regulator *tegra_camera_regulator_csi;

static int tegra_camera_enable_isp(void)
{
	return clk_enable(isp_clk);
}

static int tegra_camera_disable_isp(void)
{
	clk_disable(isp_clk);
	return 0;
}

#ifdef CONFIG_MACH_N1
int tegra_camera_enable_vi(void)
#else
static int tegra_camera_enable_vi(void)
#endif
{
	clk_enable(vi_clk);
	clk_enable(vi_sensor_clk);
	clk_enable(csus_clk);
	return 0;
}

#ifdef CONFIG_MACH_N1
int tegra_camera_disable_vi(void)
#else
static int tegra_camera_disable_vi(void)
#endif
{
	clk_disable(vi_clk);
	clk_disable(vi_sensor_clk);
	clk_disable(csus_clk);
	return 0;
}

#ifdef CONFIG_MACH_N1
int tegra_camera_enable_csi(void)
#else
static int tegra_camera_enable_csi(void)
#endif
{
#ifndef CONFIG_MACH_N1
	int ret;

	ret = regulator_enable(tegra_camera_regulator_csi);
	if (ret)
		return ret;
#endif
	clk_enable(csi_clk);
	return 0;
}
#ifdef CONFIG_MACH_N1
int tegra_camera_disable_csi(void)
#else
static int tegra_camera_disable_csi(void)
#endif
{
#ifndef CONFIG_MACH_N1
	int ret;

	ret = regulator_disable(tegra_camera_regulator_csi);
	if (ret)
		return ret;
#endif
	clk_disable(csi_clk);
	return 0;
}

struct tegra_camera_block tegra_camera_block[] = {
	[TEGRA_CAMERA_MODULE_ISP] = {tegra_camera_enable_isp,
		tegra_camera_disable_isp, false},
	[TEGRA_CAMERA_MODULE_VI] = {tegra_camera_enable_vi,
		tegra_camera_disable_vi, false},
	[TEGRA_CAMERA_MODULE_CSI] = {tegra_camera_enable_csi,
		tegra_camera_disable_csi, false},
};

#define TEGRA_CAMERA_VI_CLK_SEL_INTERNAL 0
#define TEGRA_CAMERA_VI_CLK_SEL_EXTERNAL (1<<24)
#define TEGRA_CAMERA_PD2VI_CLK_SEL_VI_SENSOR_CLK (1<<25)
#define TEGRA_CAMERA_PD2VI_CLK_SEL_PD2VI_CLK 0
#ifdef CONFIG_MACH_N1
int tegra_camera_clk_set_rate(struct tegra_camera_clk_info *info)
#else
static int tegra_camera_clk_set_rate(struct tegra_camera_clk_info *info)
#endif
{
	u32 offset;
	struct clk *clk;

	if (info->id != TEGRA_CAMERA_MODULE_VI) {
		pr_err("%s: Set rate only aplies to vi module %d\n", __func__,
		       info->id);
		return -EINVAL;
	}

	switch (info->clk_id) {
	case TEGRA_CAMERA_VI_CLK:
		clk = vi_clk;
		offset = 0x148;
		break;
	case TEGRA_CAMERA_VI_SENSOR_CLK:
		clk = vi_sensor_clk;
		offset = 0x1a8;
		break;
	default:
		pr_err("%s: invalid clk id for set rate %d\n", __func__,
		       info->clk_id);
		return -EINVAL;
	}

#ifdef CONFIG_MACH_N1
	if (info->clk_id == TEGRA_CAMERA_VI_SENSOR_CLK)
		clk_set_rate(clk, 24000000);
	else
#endif
	clk_set_rate(clk, info->rate);

	if (info->clk_id == TEGRA_CAMERA_VI_CLK) {
		u32 val;
		void __iomem *car = IO_ADDRESS(TEGRA_CLK_RESET_BASE);
		void __iomem *apb_misc = IO_ADDRESS(TEGRA_APB_MISC_BASE);

		writel(0x2, car + offset);

		val = readl(apb_misc + 0x42c);
		writel(val | 0x1, apb_misc + 0x42c);
	}

	info->rate = clk_get_rate(clk);
	return 0;

}

#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
static int tegra_camera_graphic_clk_set_rate(unsigned long *rates)
{
	int i;
	int size;

	if (graphics_clk_cfg == NULL || rates == NULL)
		return 0;

	size = graphics_clk_cfg->n_clks;

	for (i = 0; i < size && set_graphics_clk; i++) {
		if (graphics_clks[i] == NULL) {
			pr_err("%s. clk %i is NULL\n",__func__, i);
			continue;
		} else {
			clk_set_rate(graphics_clks[i], rates[i]);
			msleep(1);
		}
	}
	return 0;

}

static void tegra_camera_save_graphic_clk(void)
{
	int size = graphics_clk_cfg->n_restore_clks;
	int i =0;

	for (i = 0; i < size; i++) {
		graphics_restore_clk_rates[i] =
			clk_get_rate(graphics_restore_clks[i]);
	}
}

static void tegra_camera_restore_graphic_clk(void)
{
	int size = graphics_clk_cfg->n_restore_clks;
	int i =0;

	for (i = 0; i < size; i++) {
			clk_set_rate(graphics_restore_clks[i],
					graphics_restore_clk_rates[i]);
	}
}
#endif

static int tegra_camera_reset(uint id)
{
	struct clk *clk;

	switch (id) {
	case TEGRA_CAMERA_MODULE_VI:
		clk = vi_clk;
		break;
	case TEGRA_CAMERA_MODULE_ISP:
		clk = isp_clk;
		break;
	case TEGRA_CAMERA_MODULE_CSI:
		clk = csi_clk;
		break;
	default:
		return -EINVAL;
	}
	tegra_periph_reset_assert(clk);
	udelay(10);
	tegra_periph_reset_deassert(clk);

	return 0;
}

static long tegra_camera_ioctl(struct file *file,
			       unsigned int cmd, unsigned long arg)
{
	uint id;

	/* first element of arg must be u32 with id of module to talk to */
	if (copy_from_user(&id, (const void __user *)arg, sizeof(uint))) {
		pr_err("%s: Failed to copy arg from user", __func__);
		return -EFAULT;
	}

	if (id >= ARRAY_SIZE(tegra_camera_block)) {
		pr_err("%s: Invalid id to tegra isp ioctl%d\n", __func__, id);
		return -EINVAL;
	}

	switch (cmd) {
	case TEGRA_CAMERA_IOCTL_ENABLE:
	{
		int ret = 0;

		mutex_lock(&tegra_camera_lock);
		if (!tegra_camera_block[id].is_enabled) {
			ret = tegra_camera_block[id].enable();
			tegra_camera_block[id].is_enabled = true;
		}
#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
		tegra_camera_save_graphic_clk();
		current_rates = NULL;
#endif
#if defined CONFIG_HAS_EARLYSUSPEND && defined CONFIG_CPU_FREQ
		if (work_q_set_conservative == 0) {
			work_q_set_conservative = 1;
			schedule_delayed_work(&scaling_gov_work, SET_CONSERVATIVE_GOVERNOR_DELAY);
		}
#endif
		mutex_unlock(&tegra_camera_lock);
		return ret;
	}
	case TEGRA_CAMERA_IOCTL_DISABLE:
	{
		int ret = 0;

		mutex_lock(&tegra_camera_lock);
		if (tegra_camera_block[id].is_enabled) {
			ret = tegra_camera_block[id].disable();
			tegra_camera_block[id].is_enabled = false;
		}
#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
		tegra_camera_restore_graphic_clk();
		current_rates = NULL;
#endif
#if defined CONFIG_HAS_EARLYSUSPEND && defined CONFIG_CPU_FREQ
		if (work_q_set_conservative == 1) {
			work_q_set_conservative = 0;
			schedule_delayed_work(&scaling_gov_work, 0);
		}
#endif
		mutex_unlock(&tegra_camera_lock);
		return ret;
	}
	case TEGRA_CAMERA_IOCTL_CLK_SET_RATE:
	{
		struct tegra_camera_clk_info info;
		int ret;

		if (copy_from_user(&info, (const void __user *)arg,
				   sizeof(struct tegra_camera_clk_info))) {
			pr_err("%s: Failed to copy arg from user\n", __func__);
			return -EFAULT;
		}
		ret = tegra_camera_clk_set_rate(&info);
		if (ret)
			return ret;
		if (copy_to_user((void __user *)arg, &info,
				 sizeof(struct tegra_camera_clk_info))) {
			pr_err("%s: Failed to copy arg to user\n", __func__);
			return -EFAULT;
		}
		return 0;
	}
	case TEGRA_CAMERA_IOCTL_RESET:
		return tegra_camera_reset(id);
#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
	case TEGRA_CAMERA_IOCTL_CAMERA_USECASE:
	{
		int buf[2];
		unsigned long *rates;

		if (copy_from_user(buf, (const void __user *)arg, sizeof(buf))){
			pr_err("%s: Failed to copy arg from user\n", __func__);
			return -EFAULT;
		}
		mutex_lock(&tegra_camera_lock);
		current_usecase = buf[1];
		rates = graphics_clk_cfg->get_graphics_clk_freqs(
			current_usecase,
			current_resolution[1],
			current_resolution[2],
			current_resolution[3]);
		if (rates != NULL && rates != current_rates) {
			current_rates = rates;
			tegra_camera_graphic_clk_set_rate(rates);
		}

		mutex_unlock(&tegra_camera_lock);
		break;
	}
	case TEGRA_CAMERA_IOCTL_CAMERA_MODE:
	{
		unsigned long *rates;
		if (graphics_clk_cfg == 0)
			break;

		mutex_lock(&tegra_camera_lock);
		if (copy_from_user(current_resolution, (const void __user *)arg,
					sizeof(current_resolution))){
			pr_err("%s: Failed to copy arg from user\n", __func__);
			mutex_unlock(&tegra_camera_lock);
			return -EFAULT;
		}

		rates = graphics_clk_cfg->get_graphics_clk_freqs(
					current_usecase,
					current_resolution[1],
					current_resolution[2],
					current_resolution[3]);
		if (rates != NULL && rates != current_rates) {
			current_rates = rates;
			tegra_camera_graphic_clk_set_rate(rates);
		}

		mutex_unlock(&tegra_camera_lock);
		break;
	}
#endif
	default:
		pr_err("%s: Unknown tegra_camera ioctl.\n", TEGRA_CAMERA_NAME);
		return -EINVAL;
	}
	return 0;
}

#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
static ssize_t tegra_camera_graphics_clk_write(struct file *f,
			const char __user *user_buf, size_t count, loff_t *off)
{
	char buf[32];
	int clk_index = -1;
	int rate = -1;

	if (count < 3) {
		set_graphics_clk = 0;
		return -EINVAL;
	}

	if (copy_from_user(buf, user_buf, count)) {
		pr_err(KERN_ERR "%s. failed copy from user\n", __func__);
		return -EFAULT;
	}
	buf[count] = 0;

	set_graphics_clk = 1;
	sscanf(buf, "%i %i", &clk_index, &rate);
	if (clk_index >= 0 && clk_index < graphics_clk_cfg->n_clks) {
		clk_set_rate(graphics_clks[clk_index], rate*1000000);
	}
	return count;
}
#endif

static int tegra_camera_release(struct inode *inode, struct file *file)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(tegra_camera_block); i++)
		if (tegra_camera_block[i].is_enabled) {
			tegra_camera_block[i].disable();
			tegra_camera_block[i].is_enabled = false;
		}

	return 0;
}

static const struct file_operations tegra_camera_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = tegra_camera_ioctl,
	.release = tegra_camera_release,
};

static struct miscdevice tegra_camera_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = TEGRA_CAMERA_NAME,
	.fops = &tegra_camera_fops,
};

#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
static const struct file_operations tegra_camera_clk_fops = {
	.owner = THIS_MODULE,
	.write = tegra_camera_graphics_clk_write,
};

static struct miscdevice tegra_camera_clk_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tegra_camera_clk",
	.fops = &tegra_camera_clk_fops,
};
#endif

static int tegra_camera_clk_get(struct platform_device *pdev, const char *name,
				struct clk **clk)
{
	*clk = clk_get(&pdev->dev, name);
	if (IS_ERR_OR_NULL(*clk)) {
		pr_err("%s: unable to get clock for %s\n", __func__, name);
		*clk = NULL;
		return PTR_ERR(*clk);
	}
	return 0;
}

#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
static struct clk **tegra_camera_get_clks(int size, char **clk_devs, char **clk_cons)
{
	int i;
	struct clk **clks = NULL;
	if (size > 0) {
		clks = kmalloc(size * sizeof(struct clk*), GFP_KERNEL);
		if (clks == NULL) {
			pr_err("%s. Failed to allocate array.\n", __func__);
			return NULL;
		}
	}
	for (i = 0; i <size; i++) {
		clks[i] = NULL;
		clks[i] = clk_get_sys(clk_devs[i], clk_cons[i]);
		if (clks[i] == NULL) {
			pr_err("%s: Unable to get clk %s-%s\n",  __func__,
				clk_cons[i], clk_devs[i]);
			return NULL;
		}
	}
	return clks;
}
#endif

#if defined CONFIG_HAS_EARLYSUSPEND && defined CONFIG_CPU_FREQ
static void set_scaling_gov_work(struct work_struct *work)
{
	if (work_q_set_conservative == 1) {
		cpufreq_save_default_governor();
		cpufreq_set_conservative_governor(CPUFREQ_CAM_MODE);
		cpufreq_set_conservative_governor_param(
			SET_CONSERVATIVE_GOVERNOR_UP_THRESHOLD,
			SET_CONSERVATIVE_GOVERNOR_DOWN_THRESHOLD);
	} else {
		cpufreq_restore_default_governor(CPUFREQ_CAM_MODE);
	}
}
#endif

static int tegra_camera_probe(struct platform_device *pdev)
{
	int err;
#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
	int i;

	graphics_clk_cfg =
			(struct tegra_camera_clk_config*) (pdev->dev.platform_data);
	graphics_clks = NULL;
	graphics_restore_clks = NULL;
	set_graphics_clk = 1;
	current_usecase = CAMERA_USECASE_PREVIEW;
#endif
	pr_info("%s: probe\n", TEGRA_CAMERA_NAME);
#ifndef CONFIG_MACH_N1
	tegra_camera_regulator_csi = regulator_get(&pdev->dev, "vcsi");
	if (IS_ERR_OR_NULL(tegra_camera_regulator_csi)) {
		pr_err("%s: Couldn't get regulator vcsi\n", TEGRA_CAMERA_NAME);
		return PTR_ERR(tegra_camera_regulator_csi);
	}
#endif

	err = misc_register(&tegra_camera_device);
	if (err) {
		pr_err("%s: Unable to register misc device!\n",
		       TEGRA_CAMERA_NAME);
		goto misc_register_err;
	}

#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
	err = misc_register(&tegra_camera_clk_device);
	if (err) {
		pr_err("%s: Unable to register misc clk device!\n",
			__func__);
		goto misc_register_clk_err;
	}
#endif

	err = tegra_camera_clk_get(pdev, "isp", &isp_clk);
	if (err)
		goto misc_register_err;
	err = tegra_camera_clk_get(pdev, "vi", &vi_clk);
	if (err)
		goto vi_clk_get_err;
	err = tegra_camera_clk_get(pdev, "vi_sensor", &vi_sensor_clk);
	if (err)
		goto vi_sensor_clk_get_err;
	err = tegra_camera_clk_get(pdev, "csus", &csus_clk);
	if (err)
		goto csus_clk_get_err;
	err = tegra_camera_clk_get(pdev, "csi", &csi_clk);
	if (err)
		goto csi_clk_get_err;

#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
	graphics_clks = tegra_camera_get_clks(
			graphics_clk_cfg->n_clks,
			graphics_clk_cfg->clk_devs,
			graphics_clk_cfg->clk_cons);

	if (graphics_clks == NULL)
		goto graphic_clk_get_err;

	graphics_restore_clks = tegra_camera_get_clks(
			graphics_clk_cfg->n_restore_clks,
			graphics_clk_cfg->clk_restore_devs,
			graphics_clk_cfg->clk_restore_cons);
	if (graphics_clks == NULL)
		goto graphic_restore_clk_get_err;

	graphics_restore_clk_rates = kmalloc(sizeof(unsigned long) *
				graphics_clk_cfg->n_restore_clks, GFP_KERNEL);

	if (graphics_restore_clk_rates == NULL)
		goto graphic_restore_clk_get_err;
#endif
#if defined CONFIG_HAS_EARLYSUSPEND && defined CONFIG_CPU_FREQ
	work_q_set_conservative = 0;
	INIT_DELAYED_WORK(&scaling_gov_work, set_scaling_gov_work);
#endif

	return 0;

#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
graphic_restore_clk_get_err:
	for (i = 0; i <graphics_clk_cfg->n_restore_clks
			 && graphics_restore_clks[i] != NULL; i++)
		clk_put(graphics_restore_clks[i]);
	if (graphics_restore_clks != NULL)
		kfree(graphics_restore_clks);
	if (graphics_restore_clk_rates != NULL)
		kfree(graphics_restore_clk_rates);
graphic_clk_get_err:
	for (i = 0; i <graphics_clk_cfg->n_clks
			 && graphics_clks[i] != NULL; i++)
		clk_put(graphics_clks[i]);
	if (graphics_clks != NULL)
		kfree(graphics_clks);
#endif
csi_clk_get_err:
	clk_put(csus_clk);
csus_clk_get_err:
	clk_put(vi_sensor_clk);
vi_sensor_clk_get_err:
	clk_put(vi_clk);
vi_clk_get_err:
	clk_put(isp_clk);
#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
misc_register_clk_err:
	misc_deregister(&tegra_camera_device);
#endif
misc_register_err:
	regulator_put(tegra_camera_regulator_csi);
	return err;
}

static int tegra_camera_remove(struct platform_device *pdev)
{
#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
	int size;
	int i;
#endif
	clk_put(isp_clk);
	clk_put(vi_clk);
	clk_put(vi_sensor_clk);
	clk_put(csus_clk);
	clk_put(csi_clk);

	regulator_put(tegra_camera_regulator_csi);
	misc_deregister(&tegra_camera_device);
#ifdef CONFIG_TEGRA_DYNAMIC_CAMERA_CLK_RATE
	size = 0;
	if (graphics_clk_cfg != NULL) {
		size = graphics_clk_cfg->n_clks;

		for (i = 0; i <size && graphics_clks[i] != NULL; i++)
			clk_put(graphics_clks[i]);
		misc_deregister(&tegra_camera_clk_device);
		kfree(graphics_clks);
	}
	if (graphics_clk_cfg != NULL) {
		for (i = 0; i <graphics_clk_cfg->n_restore_clks
				 && graphics_restore_clks[i] != NULL; i++)
			clk_put(graphics_restore_clks[i]);
		if (graphics_restore_clks != NULL)
			kfree(graphics_restore_clks);
		if (graphics_restore_clk_rates != NULL)
			kfree(graphics_restore_clk_rates);
	}
#endif
	return 0;
}

static struct platform_driver tegra_camera_driver = {
	.probe = tegra_camera_probe,
	.remove = tegra_camera_remove,
	.driver = { .name = TEGRA_CAMERA_NAME }
};

static int __init tegra_camera_init(void)
{
	return platform_driver_register(&tegra_camera_driver);
}

static void __exit tegra_camera_exit(void)
{
	platform_driver_unregister(&tegra_camera_driver);
}

module_init(tegra_camera_init);
module_exit(tegra_camera_exit);

