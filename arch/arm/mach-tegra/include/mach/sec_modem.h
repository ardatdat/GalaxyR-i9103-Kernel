#ifndef _SEC_MODEM_H_
#define _SEC_MODEM_H_

#ifdef CONFIG_LINK_DEVICE_HSIC
void set_host_states(struct platform_device *pdev, int type);
void set_slave_wake(void);
#else
#define set_host_states(pdev, type) do { } while (0);
#define set_slave_wake() do { } while (0);
#endif

#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int charging_mode_from_boot;
#endif

#endif /*_SEC_MODEM_H_*/
