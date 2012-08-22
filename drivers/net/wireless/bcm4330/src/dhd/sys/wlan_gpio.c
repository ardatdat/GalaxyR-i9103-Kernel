#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd.h>

extern void tegra_sdhci_force_presence_change();
extern int n1_wifi_power(int on);

//this is called by exit()
void n1_device_wifi_power(int on,int flag)
{
	if (flag != 1)
	{
		if (on)
	        n1_wifi_power(1);
		else
	        n1_wifi_power(0);
	        //tegra_sdhci_force_presence_change();

	    return;
	}

	if (on)
	{
        n1_wifi_power(1);
        tegra_sdhci_force_presence_change();
	}
	else
	{
        n1_wifi_power(0);
        //tegra_sdhci_force_presence_change();
	}

    return;
}
EXPORT_SYMBOL(n1_device_wifi_power);
