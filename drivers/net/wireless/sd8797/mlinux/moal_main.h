/** @file moal_main.h
  *
  * @brief This file contains wlan driver specific defines etc.
  * 
  * Copyright (C) 2008-2011, Marvell International Ltd.  
  *
  * This software file (the "File") is distributed by Marvell International 
  * Ltd. under the terms of the GNU General Public License Version 2, June 1991 
  * (the "License").  You may use, redistribute and/or modify this File in 
  * accordance with the terms and conditions of the License, a copy of which 
  * is available by writing to the Free Software Foundation, Inc.,
  * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
  * worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
  *
  * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE 
  * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about 
  * this warranty disclaimer.
  *
  */

/********************************************************
Change log:
    10/21/2008: initial version
********************************************************/

#ifndef _MOAL_MAIN_H
#define _MOAL_MAIN_H

/* warnfix for FS redefination if any? */
#ifdef FS
#undef FS
#endif

/* Linux header files */
#include        <linux/kernel.h>
#include        <linux/module.h>
#include        <linux/init.h>
#include        <linux/version.h>
#include        <linux/param.h>
#include        <linux/delay.h>
#include        <linux/slab.h>
#include        <linux/mm.h>
#include        <linux/types.h>
#include        <linux/sched.h>
#include        <linux/timer.h>
#include        <linux/ioport.h>
#include        <linux/pci.h>
#include        <linux/ctype.h>
#include        <linux/proc_fs.h>
#include        <linux/vmalloc.h>
#include	<linux/ptrace.h>
#include	<linux/string.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#include       <linux/config.h>
#endif

/* ASM files */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include        <linux/semaphore.h>
#else
#include        <asm/semaphore.h>
#endif
#include        <asm/byteorder.h>
#include        <asm/irq.h>
#include        <asm/uaccess.h>
#include        <asm/io.h>
#include        <asm/system.h>

/* Net header files */
#include        <linux/netdevice.h>
#include        <linux/net.h>
#include        <linux/inet.h>
#include        <linux/ip.h>
#include        <linux/skbuff.h>
#include        <linux/if_arp.h>
#include        <linux/if_ether.h>
#include        <linux/etherdevice.h>
#include        <net/sock.h>
#include        <net/arp.h>
#include        <linux/rtnetlink.h>

#include	<linux/firmware.h>

#include        "mlan.h"
#include        "moal_shim.h"
/* Wireless header */
#include        <linux/wireless.h>
#if defined(STA_CFG80211) || defined(UAP_CFG80211)
#include        <net/lib80211.h>
#include        <net/cfg80211.h>
#include        <net/ieee80211_radiotap.h>
#endif
#if defined(STA_WEXT) || defined(UAP_WEXT)
#include        <net/iw_handler.h>
#include        "moal_wext.h"
#endif
#ifdef STA_WEXT
#include        "moal_priv.h"
#endif

/** Define BOOLEAN */
typedef t_u8 BOOLEAN;

/** Driver version */
extern char driver_version[];

/** Private structure for MOAL */
typedef struct _moal_private moal_private;
/** Handle data structure for MOAL  */
typedef struct _moal_handle moal_handle;

/** Hardware status codes */
typedef enum _MOAL_HARDWARE_STATUS
{
    HardwareStatusReady,
    HardwareStatusInitializing,
    HardwareStatusFwReady,
    HardwareStatusReset,
    HardwareStatusClosing,
    HardwareStatusNotReady
} MOAL_HARDWARE_STATUS;

/** moal_wait_option */
enum
{
    MOAL_NO_WAIT,
    MOAL_IOCTL_WAIT,
    MOAL_CMD_WAIT,
    MOAL_PROC_WAIT,
    MOAL_WSTATS_WAIT
};

/** moal_main_state */
enum
{
    MOAL_STATE_IDLE,
    MOAL_RECV_INT,
    MOAL_ENTER_WORK_QUEUE,
    MOAL_START_MAIN_PROCESS,
    MOAL_END_MAIN_PROCESS
};

/** HostCmd_Header */
typedef struct _HostCmd_Header
{
    /** Command */
    t_u16 command;
    /** Size */
    t_u16 size;
} HostCmd_Header;

#ifndef MIN
/** Find minimum */
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif

/*
 * OS timer specific
 */

/** Timer structure */
typedef struct _moal_drv_timer
{
        /** Timer list */
    struct timer_list tl;
        /** Timer function */
    void (*timer_function) (void *context);
        /** Timer function context */
    void *function_context;
        /** Time period */
    t_u32 time_period;
        /** Is timer periodic ? */
    t_u32 timer_is_periodic;
        /** Is timer cancelled ? */
    t_u32 timer_is_canceled;
} moal_drv_timer, *pmoal_drv_timer;

/**
 *  @brief Timer handler
 *
 *  @param fcontext	Timer context
 *
 *  @return		N/A
 */
static inline void
woal_timer_handler(unsigned long fcontext)
{
    pmoal_drv_timer timer = (pmoal_drv_timer) fcontext;

    timer->timer_function(timer->function_context);

    if (timer->timer_is_periodic == MTRUE) {
        mod_timer(&timer->tl, jiffies + ((timer->time_period * HZ) / 1000));
    } else {
        timer->timer_is_canceled = MTRUE;
        timer->time_period = 0;
    }
}

/**
 *  @brief Initialize timer
 *
 *  @param timer		Timer structure
 *  @param TimerFunction	Timer function
 *  @param FunctionContext	Timer function context
 *
 *  @return			N/A
 */
static inline void
woal_initialize_timer(pmoal_drv_timer timer,
                      void (*TimerFunction) (void *context),
                      void *FunctionContext)
{
    /* First, setup the timer to trigger the wlan_timer_handler proxy */
    init_timer(&timer->tl);
    timer->tl.function = woal_timer_handler;
    timer->tl.data = (t_ptr) timer;

    /* Then tell the proxy which function to call and what to pass it */
    timer->timer_function = TimerFunction;
    timer->function_context = FunctionContext;
    timer->timer_is_canceled = MTRUE;
    timer->time_period = 0;
    timer->timer_is_periodic = MFALSE;
}

/**
 *  @brief Modify timer
 *
 *  @param timer		Timer structure
 *  @param MillisecondPeriod	Time period in millisecond
 *
 *  @return			N/A
 */
static inline void
woal_mod_timer(pmoal_drv_timer timer, t_u32 MillisecondPeriod)
{
    timer->time_period = MillisecondPeriod;
    mod_timer(&timer->tl, jiffies + (MillisecondPeriod * HZ) / 1000);
    timer->timer_is_canceled = MFALSE;
}

/**
 *  @brief Cancel timer
 *
 *  @param timer	Timer structure
 *
 *  @return		N/A
 */
static inline void
woal_cancel_timer(moal_drv_timer * timer)
{
    del_timer(&timer->tl);
    timer->timer_is_canceled = MTRUE;
    timer->time_period = 0;
}

#ifdef REASSOCIATION
/*
 * OS Thread Specific
 */

#include	<linux/kthread.h>

/** Kernel thread structure */
typedef struct _moal_thread
{
    /** Task control structrue */
    struct task_struct *task;
    /** Pointer to wait_queue_head */
    wait_queue_head_t wait_q;
    /** PID */
    pid_t pid;
    /** Pointer to moal_handle */
    void *handle;
} moal_thread;

/**
 *  @brief Activate thread
 *
 *  @param thr			Thread structure
 *  @return			N/A
 */
static inline void
woal_activate_thread(moal_thread * thr)
{
    /** Initialize the wait queue */
    init_waitqueue_head(&thr->wait_q);

    /** Record the thread pid */
    thr->pid = current->pid;
}

/**
 *  @brief De-activate thread
 *
 *  @param thr			Thread structure
 *  @return			N/A
 */
static inline void
woal_deactivate_thread(moal_thread * thr)
{
    /* Reset the pid */
    thr->pid = 0;
}

/**
 *  @brief Create and run the thread
 *
 *  @param threadfunc		Thread function
 *  @param thr			Thread structure
 *  @param name			Thread name
 *  @return			N/A
 */
static inline void
woal_create_thread(int (*threadfunc) (void *), moal_thread * thr, char *name)
{
    /* Create and run the thread */
    thr->task = kthread_run(threadfunc, thr, "%s", name);
}
#endif /* REASSOCIATION */

/* The following macros are neccessary to retain compatibility
 * around the workqueue chenges happened in kernels >= 2.6.20:
 * - INIT_WORK changed to take 2 arguments and let the work function
 *   get its own data through the container_of macro
 * - delayed works have been split from normal works to save some
 *   memory usage in struct work_struct
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
/** Work_queue work initialization */
#define MLAN_INIT_WORK(_work, _fun)                 INIT_WORK(_work, ((void (*)(void *))_fun), _work)
/** Work_queue delayed work initialization */
#define MLAN_INIT_DELAYED_WORK(_work, _fun)         INIT_WORK(_work, ((void (*)(void *))_fun), _work)
/** Work_queue container parameter */
#define MLAN_DELAYED_CONTAINER_OF(_ptr, _type, _m)  container_of(_ptr, _type, _m)
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20) */
/** Work_queue work initialization */
#define MLAN_INIT_WORK                              INIT_WORK
/** Work_queue delayed work initialization */
#define MLAN_INIT_DELAYED_WORK                      INIT_DELAYED_WORK
/** Work_queue container parameter */
#define MLAN_DELAYED_CONTAINER_OF(_ptr, _type, _m)  container_of(_ptr, _type, _m.work)
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20) */

/**
 *  @brief Schedule timeout
 *
 *  @param millisec	Timeout duration in milli second
 *
 *  @return		N/A
 */
static inline void
woal_sched_timeout(t_u32 millisec)
{
    set_current_state(TASK_INTERRUPTIBLE);

    schedule_timeout((millisec * HZ) / 1000);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#define IN6PTON_XDIGIT		0x00010000
#define IN6PTON_DIGIT		0x00020000
#define IN6PTON_COLON_MASK	0x00700000
#define IN6PTON_COLON_1		0x00100000      /* single : requested */
#define IN6PTON_COLON_2		0x00200000      /* second : requested */
#define IN6PTON_COLON_1_2	0x00400000      /* :: requested */
#define IN6PTON_DOT		0x00800000      /* . */
#define IN6PTON_DELIM		0x10000000
#define IN6PTON_NULL		0x20000000      /* first/tail */
#define IN6PTON_UNKNOWN		0x40000000

static inline int
xdigit2bin(char c, int delim)
{
    if (c == delim || c == '\0')
        return IN6PTON_DELIM;
    if (c == ':')
        return IN6PTON_COLON_MASK;
    if (c == '.')
        return IN6PTON_DOT;
    if (c >= '0' && c <= '9')
        return (IN6PTON_XDIGIT | IN6PTON_DIGIT | (c - '0'));
    if (c >= 'a' && c <= 'f')
        return (IN6PTON_XDIGIT | (c - 'a' + 10));
    if (c >= 'A' && c <= 'F')
        return (IN6PTON_XDIGIT | (c - 'A' + 10));
    if (delim == -1)
        return IN6PTON_DELIM;
    return IN6PTON_UNKNOWN;
}

static inline int
in4_pton(const char *src, int srclen, u8 * dst, int delim, const char **end)
{
    const char *s;
    u8 *d;
    u8 dbuf[4];
    int ret = 0;
    int i;
    int w = 0;

    if (srclen < 0)
        srclen = strlen(src);
    s = src;
    d = dbuf;
    i = 0;
    while (1) {
        int c;
        c = xdigit2bin(srclen > 0 ? *s : '\0', delim);
        if (!
            (c &
             (IN6PTON_DIGIT | IN6PTON_DOT | IN6PTON_DELIM |
              IN6PTON_COLON_MASK))) {
            goto out;
        }
        if (c & (IN6PTON_DOT | IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
            if (w == 0)
                goto out;
            *d++ = w & 0xff;
            w = 0;
            i++;
            if (c & (IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
                if (i != 4)
                    goto out;
                break;
            }
            goto cont;
        }
        w = (w * 10) + c;
        if ((w & 0xffff) > 255) {
            goto out;
        }
      cont:
        if (i >= 4)
            goto out;
        s++;
        srclen--;
    }
    ret = 1;
    memcpy(dst, dbuf, sizeof(dbuf));
  out:
    if (end)
        *end = s;
    return ret;
}
#endif /* < 2.6.19 */

#ifndef __ATTRIB_ALIGN__
#define __ATTRIB_ALIGN__ __attribute__((aligned(4)))
#endif

#ifndef __ATTRIB_PACK__
#define __ATTRIB_PACK__ __attribute__ ((packed))
#endif

/** Get module */
#define MODULE_GET	try_module_get(THIS_MODULE)
/** Put module */
#define MODULE_PUT	module_put(THIS_MODULE)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
/** Initialize semaphore */
#define MOAL_INIT_SEMAPHORE(x)    	init_MUTEX(x)
/** Initialize semaphore */
#define MOAL_INIT_SEMAPHORE_LOCKED(x) 	init_MUTEX_LOCKED(x)
#else
/** Initialize semaphore */
#define MOAL_INIT_SEMAPHORE(x)    	sema_init(x,1)
/** Initialize semaphore */
#define MOAL_INIT_SEMAPHORE_LOCKED(x) 	sema_init(x,0)
#endif

/** Acquire semaphore and with blocking */
#define MOAL_ACQ_SEMAPHORE_BLOCK(x)	down_interruptible(x)
/** Acquire semaphore without blocking */
#define MOAL_ACQ_SEMAPHORE_NOBLOCK(x)	down_trylock(x)
/** Release semaphore */
#define MOAL_REL_SEMAPHORE(x) 		up(x)

/** Request FW timeout in second */
#define REQUEST_FW_TIMEOUT		30

/** Default watchdog timeout */
#define MRVDRV_DEFAULT_WATCHDOG_TIMEOUT (5 * HZ)

#ifdef UAP_SUPPORT
/** Default watchdog timeout
    Increase the value to avoid kernel Tx timeout message in case
    station in PS mode or left.
    The default value of PS station ageout timer is 40 seconds.
    Hence, the watchdog timer is set to a value higher than it.
*/
#define MRVDRV_DEFAULT_UAP_WATCHDOG_TIMEOUT (41 * HZ)
#endif

/** Threshold value of number of times the Tx timeout happened */
#define NUM_TX_TIMEOUT_THRESHOLD      5

/** 10 seconds */
#define MOAL_TIMER_10S                10000
/** 5 seconds */
#define MOAL_TIMER_5S                 5000
/** 1 second */
#define MOAL_TIMER_1S                 1000

/** Default value of re-assoc timer */
#define REASSOC_TIMER_DEFAULT         500

/** Netlink protocol number */
#define NETLINK_MARVELL     (MAX_LINKS - 1)
/** Netlink maximum payload size */
#define NL_MAX_PAYLOAD      1024
/** Netlink multicast group number */
#define NL_MULTICAST_GROUP  1

/** MAX Tx Pending count */
#define MAX_TX_PENDING    	100

/** LOW Tx Pending count */
#define LOW_TX_PENDING      80

/** Offset for subcommand */
#define SUBCMD_OFFSET       4

/** Macro to extract the TOS field from a skb */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#define SKB_TOS(skb) (ip_hdr(skb)->tos)
#else
#define SKB_TOS(skb) (skb->nh.iph->tos)
#endif

/** Offset for TOS field in the IP header */
#define IPTOS_OFFSET 5

/** Offset for DSCP in the tos field */
#define DSCP_OFFSET 2

/** wait_queue structure */
typedef struct _wait_queue
{
        /** Pointer to wait_queue_head */
    wait_queue_head_t *wait;
        /** Wait condition */
    BOOLEAN condition;
        /** Start time */
    t_u32 start_time;
        /** Status from MLAN */
    mlan_status status;
} wait_queue, *pwait_queue;

/** Auto Rate */
#define AUTO_RATE 0xFF

#define STA_WEXT_MASK        MBIT(0)
#define UAP_WEXT_MASK        MBIT(1)
#define STA_CFG80211_MASK    MBIT(2)
#define UAP_CFG80211_MASK    MBIT(3)
#ifdef STA_CFG80211
#ifdef STA_SUPPORT
/** Is STA CFG80211 enabled in module param */
#define IS_STA_CFG80211(x)          (x & STA_CFG80211_MASK)
#endif
#endif
#ifdef UAP_CFG80211
#ifdef UAP_SUPPORT
/** Is UAP CFG80211 enabled in module param */
#define IS_UAP_CFG80211(x)          (x & UAP_CFG80211_MASK)
#endif
#endif
#if defined(STA_CFG80211) || defined(UAP_CFG80211)
/** Is UAP or STA CFG80211 enabled in module param */
#define IS_STA_OR_UAP_CFG80211(x)   (x & (STA_CFG80211_MASK | UAP_CFG80211_MASK))
#endif

#ifdef STA_WEXT
/** Is STA WEXT enabled in module param */
#define IS_STA_WEXT(x)              (x & STA_WEXT_MASK)
#endif /* STA_WEXT */
#ifdef UAP_WEXT
/** Is UAP WEXT enabled in module param */
#define IS_UAP_WEXT(x)              (x & UAP_WEXT_MASK)
#endif /* UAP_WEXT */
#if defined(STA_WEXT) || defined(UAP_WEXT)
/** Is UAP or STA WEXT enabled in module param */
#define IS_STA_OR_UAP_WEXT(x)       (x & (STA_WEXT_MASK | UAP_WEXT_MASK))
#endif

#ifdef STA_SUPPORT
/** Driver mode STA bit */
#define DRV_MODE_STA       MBIT(0)
/** Maximum STA BSS */
#define MAX_STA_BSS        1
/** Default STA BSS */
#define DEF_STA_BSS        1
#endif
#ifdef UAP_SUPPORT
/** Driver mode uAP bit */
#define DRV_MODE_UAP       MBIT(1)
/** Maximum uAP BSS */
#define MAX_UAP_BSS        2
/** Default uAP BSS */
#define DEF_UAP_BSS        1
#endif
#if defined(WIFI_DIRECT_SUPPORT)
/** Driver mode WIFIDIRECT bit */
#define DRV_MODE_WIFIDIRECT       MBIT(2)
/** Maximum WIFIDIRECT BSS */
#define MAX_WIFIDIRECT_BSS        1
/** Default WIFIDIRECT BSS */
#define DEF_WIFIDIRECT_BSS        1
#endif /* WIFI_DIRECT_SUPPORT && V14_FEATURE */

typedef struct _moal_drv_mode
{
    /** driver mode */
    t_u16 drv_mode;
    /** total number of interfaces */
    t_u16 intf_num;
    /** attribute of bss */
    mlan_bss_attr *bss_attr;
    /** name of firmware image */
    char *fw_name;
} moal_drv_mode;

#ifdef PROC_DEBUG
/** Debug data */
struct debug_data
{
    /** Name */
    char name[32];
    /** Size */
    t_u32 size;
    /** Address */
    t_ptr addr;
};

/** Private debug data */
struct debug_data_priv
{
    /** moal_private handle */
    moal_private *priv;
    /** Debug items */
    struct debug_data *items;
    /** numbre of item */
    int num_of_items;
};
#endif

/** Maximum IP address buffer length */
#define IPADDR_MAX_BUF          20
/** IP address operation: Remove */
#define IPADDR_OP_REMOVE        0

/** Private structure for MOAL */
struct _moal_private
{
        /** Handle structure */
    moal_handle *phandle;
        /** Tx timeout count */
    t_u32 num_tx_timeout;
        /** BSS index */
    t_u8 bss_index;
        /** BSS type */
    t_u8 bss_type;
        /** BSS role */
    t_u8 bss_role;
        /** MAC address information */
    t_u8 current_addr[ETH_ALEN];
        /** Media connection status */
    BOOLEAN media_connected;
#ifdef UAP_SUPPORT
        /** uAP started or not */
    BOOLEAN bss_started;
#endif
#ifdef STA_SUPPORT
        /** scan type */
    t_u8 scan_type;
        /** bg_scan_start */
    t_u8 bg_scan_start;
        /** bg_scan reported */
    t_u8 bg_scan_reported;
        /** bg_scan config */
    wlan_bgscan_cfg scan_cfg;
#endif
        /** Net device pointer */
    struct net_device *netdev;
        /** Net device statistics structure */
    struct net_device_stats stats;
#if defined(STA_CFG80211) || defined(UAP_CFG80211)
        /** Country code for regulatory domain */
    t_u8 country_code[COUNTRY_CODE_LEN];
        /** Wireless device pointer */
    struct wireless_dev *wdev;
        /** channel parameter for UAP/GO */
    t_u16 channel;
        /** cipher */
    t_u32 cipher;
        /** key index */
    t_u8 key_index;
        /** key len */
    t_u16 key_len;
        /** key data */
    t_u8 key_material[MLAN_MAX_KEY_LENGTH];
        /** probereq index for mgmt ie */
    t_u16 probereq_index;
#endif
#ifdef STA_CFG80211
#ifdef STA_SUPPORT
        /** CFG80211 scan request description */
    struct cfg80211_scan_request *scan_request;
        /** CFG80211 association description */
    t_u8 cfg_bssid[ETH_ALEN];
        /** Disconnect request from CFG80211 */
    bool cfg_disconnect;
#endif                          /* STA_SUPPORT */
#endif                          /* STA_CFG80211 */
        /** IOCTL wait queue */
    wait_queue_head_t ioctl_wait_q __ATTRIB_ALIGN__;
        /** CMD wait queue */
    wait_queue_head_t cmd_wait_q __ATTRIB_ALIGN__;
#ifdef CONFIG_PROC_FS
        /** Proc entry */
    struct proc_dir_entry *proc_entry;
        /** Proc entry name */
    t_s8 proc_entry_name[IFNAMSIZ];
        /** PROC wait queue */
    wait_queue_head_t proc_wait_q __ATTRIB_ALIGN__;
#endif                          /* CONFIG_PROC_FS */
#ifdef STA_SUPPORT
        /** Nickname */
    t_u8 nick_name[16];
        /** AdHoc link sensed flag */
    BOOLEAN is_adhoc_link_sensed;
        /** Current WEP key index */
    t_u16 current_key_index;
#ifdef REASSOCIATION
    mlan_ssid_bssid prev_ssid_bssid;
        /** Re-association required */
    BOOLEAN reassoc_required;
        /** Flag of re-association on/off */
    BOOLEAN reassoc_on;
#endif                          /* REASSOCIATION */
        /** Report scan result */
    t_u8 report_scan_result;
        /** wpa_version */
    t_u8 wpa_version;
        /** key mgmt */
    t_u8 key_mgmt;
        /** rx_filter */
    t_u8 rx_filter;
#endif                          /* STA_SUPPORT */
        /** Rate index */
    t_u16 rate_index;
#if defined(STA_WEXT) || defined(UAP_WEXT)
        /** IW statistics */
    struct iw_statistics w_stats;
        /** w_stats wait queue */
    wait_queue_head_t w_stats_wait_q __ATTRIB_ALIGN__;
#endif
#ifdef UAP_WEXT
    /** Pairwise Cipher used for WPA/WPA2 mode */
    t_u16 pairwise_cipher;
    /** Group Cipher */
    t_u16 group_cipher;
    /** Protocol stored during uap wext configuratoin */
    t_u16 uap_protocol;
    /** Key Mgmt whether PSK or 1x */
    t_u16 uap_key_mgmt;
    /** Beacon IE length from hostapd */
    t_u16 bcn_ie_len;
    /** Beacon IE buffer from hostapd */
    t_u8 bcn_ie_buf[MAX_IE_SIZE];
#endif

#ifdef PROC_DEBUG
    /** MLAN debug info */
    struct debug_data_priv items_priv;
#endif
};

/** Handle data structure for MOAL */
struct _moal_handle
{
        /** MLAN adapter structure */
    t_void *pmlan_adapter;
        /** Private pointer */
    moal_private *priv[MLAN_MAX_BSS_NUM];
        /** Priv number */
    t_u8 priv_num;
        /** Bss attr */
    moal_drv_mode drv_mode;
        /** set mac address flag */
    t_u8 set_mac_addr;
        /** MAC address */
    t_u8 mac_addr[ETH_ALEN];
#ifdef CONFIG_PROC_FS
        /** Proc top level directory entry */
    struct proc_dir_entry *proc_mwlan;
#endif
        /** Firmware */
    const struct firmware *firmware;
        /** Firmware request start time */
    struct timeval req_fw_time;
        /** Init config file */
    const struct firmware *user_data;
        /** Hotplug device */
    struct device *hotplug_device;
        /** STATUS variables */
    MOAL_HARDWARE_STATUS hardware_status;
        /** POWER MANAGEMENT AND PnP SUPPORT */
    BOOLEAN surprise_removed;
        /** Firmware release number */
    t_u32 fw_release_number;
        /** Init wait queue token */
    t_u16 init_wait_q_woken;
        /** Init wait queue */
    wait_queue_head_t init_wait_q __ATTRIB_ALIGN__;
        /** Device suspend flag */
    BOOLEAN is_suspended;
#ifdef SDIO_SUSPEND_RESUME
        /** suspend notify flag */
    BOOLEAN suspend_notify_req;
#endif
        /** Host Sleep activated flag */
    t_u8 hs_activated;
        /** Host Sleep activated event wait queue token */
    t_u16 hs_activate_wait_q_woken;
        /** Host Sleep activated event wait queue */
    wait_queue_head_t hs_activate_wait_q __ATTRIB_ALIGN__;
        /** Card pointer */
    t_void *card;
        /** Rx pending in MLAN */
    atomic_t rx_pending;
        /** Tx packet pending count in mlan */
    atomic_t tx_pending;
        /** IOCTL pending count in mlan */
    atomic_t ioctl_pending;
        /** Malloc count */
    t_u32 malloc_count;
        /** lock count */
    t_u32 lock_count;
        /** mlan buffer alloc count */
    t_u32 mbufalloc_count;
        /** hs skip count */
    t_u32 hs_skip_count;
        /** hs force count */
    t_u32 hs_force_count;
        /** suspend_fail flag */
    BOOLEAN suspend_fail;
#ifdef REASSOCIATION
        /** Re-association thread */
    moal_thread reassoc_thread;
        /** Re-association timer set flag */
    BOOLEAN is_reassoc_timer_set;
        /** Re-association timer */
    moal_drv_timer reassoc_timer __ATTRIB_ALIGN__;
        /**  */
    struct semaphore reassoc_sem;
        /** Bitmap for re-association on/off */
    t_u8 reassoc_on;
#endif                          /* REASSOCIATION */
        /** Driver workqueue */
    struct workqueue_struct *workqueue;
        /** main work */
    struct work_struct main_work;
#if defined(STA_CFG80211) || defined(UAP_CFG80211)
#ifdef WIFI_DIRECT_SUPPORT
        /** remain on channel flag */
    t_u8 remain_on_channel;
        /** ieee802_11_channel */
    struct ieee80211_channel chan;
        /** channel type */
    enum nl80211_channel_type channel_type;
        /** cookie */
    t_u64 cookie;
#endif
#endif
        /** Read SDIO registers for debugging */
    t_u32 sdio_reg_dbg;
        /** Netlink kernel socket */
    struct sock *nl_sk;
        /** Netlink kernel socket number */
    t_u32 netlink_num;
    /** w_stats wait queue token */
    BOOLEAN meas_wait_q_woken;
    /** w_stats wait queue */
    wait_queue_head_t meas_wait_q __ATTRIB_ALIGN__;
    /** Measurement start jiffes */
    t_u32 meas_start_jiffies;
    /** CAC checking period flag */
    BOOLEAN cac_period;
    /** BSS START command delay executing flag */
    BOOLEAN delay_bss_start;
    /** SSID,BSSID parameter of delay executing */
    mlan_ssid_bssid delay_ssid_bssid;
#ifdef DFS_TESTING_SUPPORT
    /** cac period length, valid only when dfs testing is enabled */
    t_u32 cac_period_jiffies;
#endif
    /** handle index - for multiple card supports */
    t_u8 handle_idx;
#ifdef SDIO_MMC_DEBUG
        /** cmd53 write state */
    u8 cmd53w;
        /** cmd53 read state */
    u8 cmd53r;
#endif
#ifdef STA_SUPPORT
        /** Scan pending on blocked flag */
    t_u8 scan_pending_on_block;
        /** Async scan semaphore */
    struct semaphore async_sem;

#endif
        /** main state */
    t_u8 main_state;
        /** cmd52 function */
    t_u8 cmd52_func;
        /** cmd52 register */
    t_u8 cmd52_reg;
        /** cmd52 value */
    t_u8 cmd52_val;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
        /** spinlock to stop_queue/wake_queue*/
    spinlock_t queue_lock;
#endif
};

/**
 *  @brief set trans_start for each TX queue.
 *
 *  @param dev		A pointer to net_device structure
 *
 *  @return			N/A
 */
static inline void
woal_set_trans_start(struct net_device *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
    unsigned int i;
    for (i = 0; i < dev->num_tx_queues; i++) {
        netdev_get_tx_queue(dev, i)->trans_start = jiffies;
    }
#endif
    dev->trans_start = jiffies;
}

/**
 *  @brief Start queue
 *
 *  @param dev		A pointer to net_device structure
 *
 *  @return			N/A
 */
static inline void
woal_start_queue(struct net_device *dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34)
    netif_start_queue(dev);
#else
    netif_tx_start_all_queues(dev);
#endif
}

/**
 *  @brief Stop queue
 *
 *  @param dev		A pointer to net_device structure
 *
 *  @return			N/A
 */
static inline void
woal_stop_queue(struct net_device *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
    unsigned long flags;
    moal_private *priv = (moal_private *) netdev_priv(dev);
    spin_lock_irqsave(&priv->phandle->queue_lock, flags);
    woal_set_trans_start(dev);
    if (!netif_queue_stopped(dev))
        netif_tx_stop_all_queues(dev);
    spin_unlock_irqrestore(&priv->phandle->queue_lock, flags);
#else
    woal_set_trans_start(dev);
    if (!netif_queue_stopped(dev))
        netif_stop_queue(dev);
#endif
}

/**
 *  @brief wake queue
 *
 *  @param dev		A pointer to net_device structure
 *
 *  @return			N/A
 */
static inline void
woal_wake_queue(struct net_device *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
    unsigned long flags;
    moal_private *priv = (moal_private *) netdev_priv(dev);
    spin_lock_irqsave(&priv->phandle->queue_lock, flags);
    if (netif_queue_stopped(dev))
        netif_tx_wake_all_queues(dev);
    spin_unlock_irqrestore(&priv->phandle->queue_lock, flags);
#else
    if (netif_queue_stopped(dev))
        netif_wake_queue(dev);
#endif
}

/** Max number of char in custom event - use multiple of them if needed */
#define IW_CUSTOM_MAX		256     /* In bytes */

/** Debug Macro definition*/
#ifdef	DEBUG_LEVEL1
extern t_u32 drvdbg;

#ifdef	DEBUG_LEVEL2
#define	PRINTM_MINFO(msg...)  do {if (drvdbg & MINFO) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MWARN(msg...)  do {if (drvdbg & MWARN) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MENTRY(msg...) do {if (drvdbg & MENTRY) printk(KERN_DEBUG msg);} while(0)
#else
#define	PRINTM_MINFO(msg...)  do {} while (0)
#define	PRINTM_MWARN(msg...)  do {} while (0)
#define	PRINTM_MENTRY(msg...) do {} while (0)
#endif /* DEBUG_LEVEL2 */

#define	PRINTM_MFW_D(msg...)  do {if (drvdbg & MFW_D) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MCMD_D(msg...) do {if (drvdbg & MCMD_D) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MDAT_D(msg...) do {if (drvdbg & MDAT_D) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MIF_D(msg...) do {if (drvdbg & MIF_D) printk(KERN_DEBUG msg);} while(0)

#define	PRINTM_MIOCTL(msg...) do {if (drvdbg & MIOCTL) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MINTR(msg...)  do {if (drvdbg & MINTR) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MEVENT(msg...) do {if (drvdbg & MEVENT) printk(msg);} while(0)
#define	PRINTM_MCMND(msg...)  do {if (drvdbg & MCMND) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MDATA(msg...)  do {if (drvdbg & MDATA) printk(KERN_DEBUG msg);} while(0)
#define	PRINTM_MERROR(msg...) do {if (drvdbg & MERROR) printk(KERN_ERR msg);} while(0)
#define	PRINTM_MFATAL(msg...) do {if (drvdbg & MFATAL) printk(KERN_ERR msg);} while(0)
#define	PRINTM_MMSG(msg...)   do {if (drvdbg & MMSG) printk(KERN_ALERT msg);} while(0)

#define	PRINTM(level,msg...) PRINTM_##level(msg)

#else

#define	PRINTM(level,msg...) do {} while (0)

#endif /* DEBUG_LEVEL1 */

/** Wait until a condition becomes true */
#define MASSERT(cond)                   \
do {                                    \
    if (!(cond)) {                      \
        PRINTM(MFATAL, "ASSERT: %s: %i\n", __FUNCTION__, __LINE__); \
        panic("Assert failed: Panic!"); \
    }                                   \
} while(0)

/** Log entry point for debugging */
#define	ENTER()			PRINTM(MENTRY, "Enter: %s\n", \
                                    __FUNCTION__)
/** Log exit point for debugging */
#define	LEAVE()			PRINTM(MENTRY, "Leave: %s\n", \
                                    __FUNCTION__)

#ifdef DEBUG_LEVEL1
#define DBG_DUMP_BUF_LEN 	64
#define MAX_DUMP_PER_LINE	16

static inline void
hexdump(char *prompt, t_u8 * buf, int len)
{
    int i;
    char dbgdumpbuf[DBG_DUMP_BUF_LEN];
    char *ptr = dbgdumpbuf;

    printk(KERN_DEBUG "%s:\n", prompt);
    for (i = 1; i <= len; i++) {
        ptr += snprintf(ptr, 4, "%02x ", *buf);
        buf++;
        if (i % MAX_DUMP_PER_LINE == 0) {
            *ptr = 0;
            printk(KERN_DEBUG "%s\n", dbgdumpbuf);
            ptr = dbgdumpbuf;
        }
    }
    if (len % MAX_DUMP_PER_LINE) {
        *ptr = 0;
        printk(KERN_DEBUG "%s\n", dbgdumpbuf);
    }
}

#define DBG_HEXDUMP_MERROR(x,y,z)    do {if (drvdbg & MERROR) hexdump(x,y,z);} while(0)
#define DBG_HEXDUMP_MCMD_D(x,y,z)    do {if (drvdbg & MCMD_D) hexdump(x,y,z);} while(0)
#define DBG_HEXDUMP_MDAT_D(x,y,z)    do {if (drvdbg & MDAT_D) hexdump(x,y,z);} while(0)
#define DBG_HEXDUMP_MIF_D(x,y,z)     do {if (drvdbg & MIF_D) hexdump(x,y,z);} while(0)
#define DBG_HEXDUMP_MEVT_D(x,y,z)    do {if (drvdbg & MEVT_D) hexdump(x,y,z);} while(0)
#define DBG_HEXDUMP_MFW_D(x,y,z)     do {if (drvdbg & MFW_D) hexdump(x,y,z);} while(0)
#define	DBG_HEXDUMP(level,x,y,z)    DBG_HEXDUMP_##level(x,y,z)

#else
/** Do nothing since debugging is not turned on */
#define DBG_HEXDUMP(level,x,y,z)    do {} while (0)
#endif

#ifdef DEBUG_LEVEL2
#define HEXDUMP(x,y,z)              do {if (drvdbg & MINFO) hexdump(x,y,z);} while(0)
#else
/** Do nothing since debugging is not turned on */
#define HEXDUMP(x,y,z)              do {} while (0)
#endif

#ifdef BIG_ENDIAN_SUPPORT
/** Convert from 16 bit little endian format to CPU format */
#define woal_le16_to_cpu(x) le16_to_cpu(x)
/** Convert from 32 bit little endian format to CPU format */
#define woal_le32_to_cpu(x) le32_to_cpu(x)
/** Convert from 64 bit little endian format to CPU format */
#define woal_le64_to_cpu(x) le64_to_cpu(x)
/** Convert to 16 bit little endian format from CPU format */
#define woal_cpu_to_le16(x) cpu_to_le16(x)
/** Convert to 32 bit little endian format from CPU format */
#define woal_cpu_to_le32(x) cpu_to_le32(x)
/** Convert to 64 bit little endian format from CPU format */
#define woal_cpu_to_le64(x) cpu_to_le64(x)
#else
/** Do nothing */
#define woal_le16_to_cpu(x) x
/** Do nothing */
#define woal_le32_to_cpu(x) x
/** Do nothing */
#define woal_le64_to_cpu(x) x
/** Do nothing */
#define woal_cpu_to_le16(x) x
/** Do nothing */
#define woal_cpu_to_le32(x) x
/** Do nothing */
#define woal_cpu_to_le64(x) x
#endif

/**
 *  @brief This function returns first available priv
 *  based on the BSS role
 *
 *  @param handle    A pointer to moal_handle
 *  @param bss_role  BSS role or MLAN_BSS_ROLE_ANY
 *
 *  @return          Pointer to moal_private
 */
static inline moal_private *
woal_get_priv(moal_handle * handle, mlan_bss_role bss_role)
{
    int i;

    for (i = 0; i < MIN(handle->priv_num, MLAN_MAX_BSS_NUM); i++) {
        if (handle->priv[i]) {
            if (bss_role == MLAN_BSS_ROLE_ANY ||
                GET_BSS_ROLE(handle->priv[i]) == bss_role)
                return (handle->priv[i]);
        }
    }
    return NULL;
}

/** Max line length allowed in init config file */
#define MAX_LINE_LEN        256
/** Max MAC address string length allowed */
#define MAX_MAC_ADDR_LEN    18
/** Max register type/offset/value etc. parameter length allowed */
#define MAX_PARAM_LEN       12

/** HostCmd_CMD_CFG_DATA for CAL data */
#define HostCmd_CMD_CFG_DATA 0x008f
/** HostCmd action set */
#define HostCmd_ACT_GEN_SET 0x0001
/** HostCmd CAL data header length */
#define CFG_DATA_HEADER_LEN	6

typedef struct _HostCmd_DS_GEN
{
    t_u16 command;
    t_u16 size;
    t_u16 seq_num;
    t_u16 result;
} HostCmd_DS_GEN;

typedef struct _HostCmd_DS_802_11_CFG_DATA
{
    /** Action */
    t_u16 action;
    /** Type */
    t_u16 type;
    /** Data length */
    t_u16 data_len;
    /** Data */
    t_u8 data[1];
} __ATTRIB_PACK__ HostCmd_DS_802_11_CFG_DATA;

/** combo scan header */
#define WEXT_CSCAN_HEADER		"CSCAN S\x01\x00\x00S\x00"
/** combo scan header size */
#define WEXT_CSCAN_HEADER_SIZE		12
/** combo scan ssid section */
#define WEXT_CSCAN_SSID_SECTION		'S'
/** commbo scan channel section */
#define WEXT_CSCAN_CHANNEL_SECTION	'C'
/** commbo scan passive dwell section */
#define WEXT_CSCAN_PASV_DWELL_SECTION	'P'
/** commbo scan home dwell section */
#define WEXT_CSCAN_HOME_DWELL_SECTION	'H'
/** BGSCAN RSSI section */
#define WEXT_BGSCAN_RSSI_SECTION	 'R'
/** BGSCAN SCAN INTERVAL SECTION */
#define WEXT_BGSCAN_INTERVAL_SECTION 'T'

/** band AUTO */
#define	WIFI_FREQUENCY_BAND_AUTO		0
/** band 5G */
#define	WIFI_FREQUENCY_BAND_5GHZ        1
/** band 2G */
#define	WIFI_FREQUENCY_BAND_2GHZ		2
/** All band */
#define WIFI_FREQUENCY_ALL_BAND         3

/** Rx filter: IPV4 multicast */
#define RX_FILTER_IPV4_MULTICAST        1
/** Rx filter: broadcast */
#define RX_FILTER_BROADCAST             2
/** Rx filter: unicast */
#define RX_FILTER_UNICAST               4
/** Rx filter: IPV6 multicast */
#define RX_FILTER_IPV6_MULTICAST        8

/**  Convert ASCII string to hex value */
int woal_ascii2hex(t_u8 * d, char *s, t_u32 dlen);
/**  Convert mac address from string to t_u8 buffer */
void woal_mac2u8(t_u8 * mac_addr, char *buf);
/**  Extract token from string */
char *woal_strsep(char **s, char delim, char esc);
/** Return int value of a given ASCII string */
mlan_status woal_atoi(int *data, char *a);
/** Return hex value of a given ASCII string */
int woal_atox(char *a);
/** Allocate buffer */
pmlan_buffer woal_alloc_mlan_buffer(moal_handle * handle, int size);
/** Allocate IOCTL request buffer */
pmlan_ioctl_req woal_alloc_mlan_ioctl_req(int size);
/** Free buffer */
void woal_free_mlan_buffer(moal_handle * handle, pmlan_buffer pmbuf);
/** Get private structure of a BSS by index */
moal_private *woal_bss_index_to_priv(moal_handle * handle, t_u8 bss_index);
/* Functions in interface module */
/** Add card */
moal_handle *woal_add_card(void *card);
/** Remove card */
mlan_status woal_remove_card(void *card);
/** broadcast event */
mlan_status woal_broadcast_event(moal_private * priv, t_u8 * payload,
                                 t_u32 len);
/** switch driver mode */
mlan_status woal_switch_drv_mode(moal_handle * handle, t_u32 mode);

/** Interrupt handler */
void woal_interrupt(moal_handle * handle);

#ifdef STA_WEXT
#endif
/** Get version */
void woal_get_version(moal_handle * handle, char *version, int maxlen);
/** Get Driver Version */
int woal_get_driver_version(moal_private * priv, struct ifreq *req);
/** Get extended driver version */
int woal_get_driver_verext(moal_private * priv, struct ifreq *ireq);
/** Mgmt frame forward registration */
int woal_reg_rx_mgmt_ind(moal_private * priv, t_u16 action,
                         t_u32 * pmgmt_subtype_mask, t_u8 wait_option);
#ifdef DEBUG_LEVEL1
/** Set driver debug bit masks */
int woal_set_drvdbg(moal_private * priv, t_u32 drvdbg);
#endif
/** Set/Get TX beamforming configurations */
mlan_status woal_set_get_tx_bf_cfg(moal_private * priv, t_u16 action,
                                   mlan_ds_11n_tx_bf_cfg * bf_cfg);
/** Request MAC address setting */
mlan_status woal_request_set_mac_address(moal_private * priv);
/** Request multicast list setting */
void woal_request_set_multicast_list(moal_private * priv,
                                     struct net_device *dev);
/** Request IOCTL action */
mlan_status woal_request_ioctl(moal_private * priv, mlan_ioctl_req * req,
                               t_u8 wait_option);
mlan_status woal_request_soft_reset(moal_handle * handle);
#ifdef PROC_DEBUG
/** Get debug information */
mlan_status woal_get_debug_info(moal_private * priv, t_u8 wait_option,
                                mlan_debug_info * debug_info);
/** Set debug information */
mlan_status woal_set_debug_info(moal_private * priv, t_u8 wait_option,
                                mlan_debug_info * debug_info);
#endif
/** Disconnect */
mlan_status woal_disconnect(moal_private * priv, t_u8 wait_option, t_u8 * mac);
/** associate */
mlan_status woal_bss_start(moal_private * priv, t_u8 wait_option,
                           mlan_ssid_bssid * ssid_bssid);
/** Request firmware information */
mlan_status woal_request_get_fw_info(moal_private * priv, t_u8 wait_option,
                                     mlan_fw_info * fw_info);
/** Set/get Host Sleep parameters */
mlan_status woal_set_get_hs_params(moal_private * priv, t_u16 action,
                                   t_u8 wait_option, mlan_ds_hs_cfg * hscfg);
/** Cancel Host Sleep configuration */
mlan_status woal_cancel_hs(moal_private * priv, t_u8 wait_option);
/** Enable Host Sleep configuration */
int woal_enable_hs(moal_private * priv);
/** hs active timeout 2 second */
#define HS_ACTIVE_TIMEOUT  (2 * HZ)

/** set deep sleep */
int woal_set_deep_sleep(moal_private * priv, t_u8 wait_option,
                        BOOLEAN bdeep_sleep, t_u16 idletime);

/** Get BSS information */
mlan_status woal_get_bss_info(moal_private * priv, t_u8 wait_option,
                              mlan_bss_info * bss_info);
void woal_process_ioctl_resp(moal_private * priv, mlan_ioctl_req * req);
#ifdef STA_SUPPORT
void woal_send_disconnect_to_system(moal_private * priv);
void woal_send_mic_error_event(moal_private * priv, t_u32 event);
void woal_ioctl_get_bss_resp(moal_private * priv, mlan_ds_bss * bss);
void woal_ioctl_get_info_resp(moal_private * priv, mlan_ds_get_info * info);
/** Get signal information */
mlan_status woal_get_signal_info(moal_private * priv, t_u8 wait_option,
                                 mlan_ds_get_signal * signal);
#ifdef STA_WEXT
/** Get mode */
t_u32 woal_get_mode(moal_private * priv, t_u8 wait_option);
/** Get data rates */
mlan_status woal_get_data_rates(moal_private * priv, t_u8 wait_option,
                                moal_802_11_rates * m_rates);
void woal_send_iwevcustom_event(moal_private * priv, t_s8 * str);
/** Get statistics information */
mlan_status woal_get_stats_info(moal_private * priv, t_u8 wait_option,
                                mlan_ds_get_stats * stats);
/** Get channel list */
mlan_status woal_get_channel_list(moal_private * priv, t_u8 wait_option,
                                  mlan_chan_list * chanlist);
#endif
/** Set/Get retry count */
mlan_status woal_set_get_retry(moal_private * priv, t_u32 action,
                               t_u8 wait_option, int *value);
/** Set/Get RTS threshold */
mlan_status woal_set_get_rts(moal_private * priv, t_u32 action,
                             t_u8 wait_option, int *value);
/** Set/Get fragment threshold */
mlan_status woal_set_get_frag(moal_private * priv, t_u32 action,
                              t_u8 wait_option, int *value);
/** Set/Get generic element */
mlan_status woal_set_get_gen_ie(moal_private * priv, t_u32 action, t_u8 * ie,
                                int *ie_len);
/** Set/Get TX power */
mlan_status woal_set_get_tx_power(moal_private * priv, t_u32 action,
                                  mlan_power_cfg_t * pwr);
/** Set/Get power IEEE management */
mlan_status woal_set_get_power_mgmt(moal_private * priv, t_u32 action,
                                    int *disabled, int type);
/** Get data rate */
mlan_status woal_set_get_data_rate(moal_private * priv, t_u8 action,
                                   mlan_rate_cfg_t * datarate);
/** Request a network scan */
mlan_status woal_request_scan(moal_private * priv, t_u8 wait_option,
                              mlan_802_11_ssid * req_ssid);
/** Set radio on/off */
int woal_set_radio(moal_private * priv, t_u8 option);
/** Set region code */
mlan_status woal_set_region_code(moal_private * priv, char *region);
/** Set authentication mode */
mlan_status woal_set_auth_mode(moal_private * priv, t_u8 wait_option,
                               t_u32 auth_mode);
/** Set encryption mode */
mlan_status woal_set_encrypt_mode(moal_private * priv, t_u8 wait_option,
                                  t_u32 encrypt_mode);
/** Enable wep key */
mlan_status woal_enable_wep_key(moal_private * priv, t_u8 wait_option);
/** Set WPA enable */
mlan_status woal_set_wpa_enable(moal_private * priv, t_u8 wait_option,
                                t_u32 enable);

/** Find best network to connect */
mlan_status woal_find_best_network(moal_private * priv, t_u8 wait_option,
                                   mlan_ssid_bssid * ssid_bssid);
/** Set Ad-Hoc channel */
mlan_status woal_change_adhoc_chan(moal_private * priv, int channel);

/** Get scan table */
mlan_status woal_get_scan_table(moal_private * priv, t_u8 wait_option,
                                mlan_scan_resp * scanresp);
/** Get authentication mode */
mlan_status woal_get_auth_mode(moal_private * priv, t_u8 wait_option,
                               t_u32 * auth_mode);
/** Get encryption mode */
mlan_status woal_get_encrypt_mode(moal_private * priv, t_u8 wait_option,
                                  t_u32 * encrypt_mode);
/** Get WPA state */
mlan_status woal_get_wpa_enable(moal_private * priv, t_u8 wait_option,
                                t_u32 * enable);
#endif  /**STA_SUPPORT */

mlan_status woal_set_wapi_enable(moal_private * priv, t_u8 wait_option,
                                 t_u32 enable);

/** Initialize priv */
void woal_init_priv(moal_private * priv, t_u8 wait_option);
/** Reset interface(s) */
int woal_reset_intf(moal_private * priv, t_u8 wait_option, int all_intf);
/** common ioctl for uap, station */
int woal_custom_ie_ioctl(struct net_device *dev, struct ifreq *req);
int woal_send_host_packet(struct net_device *dev, struct ifreq *req);
/** Private command ID to pass mgmt frame */
#define WOAL_MGMT_FRAME_TX_IOCTL          (SIOCDEVPRIVATE + 12)

int woal_get_bss_type(struct net_device *dev, struct ifreq *req);
#if defined(STA_WEXT) || defined(UAP_WEXT)
int woal_host_command(moal_private * priv, struct iwreq *wrq);
#endif
#if defined(WIFI_DIRECT_SUPPORT)
#if defined(STA_SUPPORT) && defined(UAP_SUPPORT)
#if defined(STA_WEXT) || defined(UAP_WEXT)
int woal_set_get_bss_role(moal_private * priv, struct iwreq *wrq);
#endif
#endif
#endif
#if defined(WIFI_DIRECT_SUPPORT) || defined(UAP_SUPPORT)
/** hostcmd ioctl for uap, wifidirect */
int woal_hostcmd_ioctl(struct net_device *dev, struct ifreq *req);
#endif

#if defined(WIFI_DIRECT_SUPPORT)
mlan_status woal_set_remain_channel_ioctl(moal_private * priv, t_u8 wait_option,
                                          mlan_ds_remain_chan * pchan);
mlan_status woal_cfg80211_wifi_direct_mode_cfg(moal_private * priv,
                                               t_u16 action, t_u16 * mode);
#endif /* WIFI_DIRECT_SUPPORT */

#ifdef CONFIG_PROC_FS
/** Initialize proc fs */
void woal_proc_init(moal_handle * handle);
/** Clean up proc fs */
void woal_proc_exit(moal_handle * handle);
/** Create proc entry */
void woal_create_proc_entry(moal_private * priv);
/** Remove proc entry */
void woal_proc_remove(moal_private * priv);
/** string to number */
int woal_string_to_number(char *s);
#endif

#ifdef PROC_DEBUG
/** Create debug proc fs */
void woal_debug_entry(moal_private * priv);
/** Remove debug proc fs */
void woal_debug_remove(moal_private * priv);
#endif /* PROC_DEBUG */

/** check pm info */
mlan_status woal_get_pm_info(moal_private * priv, mlan_ds_ps_info * pm_info);

#ifdef REASSOCIATION
int woal_reassociation_thread(void *data);
void woal_reassoc_timer_func(void *context);
#endif /* REASSOCIATION */

t_void woal_main_work_queue(struct work_struct *work);

int woal_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);
moal_private *woal_add_interface(moal_handle * handle, t_u8 bss_num,
                                 t_u8 bss_type);
void woal_remove_interface(moal_handle * handle, t_u8 bss_index);
void woal_set_multicast_list(struct net_device *dev);
mlan_status woal_request_fw(moal_handle * handle);

int woal_11h_channel_check_ioctl(moal_private * priv);
void woal_cancel_cac_block(moal_private * priv);
void woal_moal_debug_info(moal_private * priv, moal_handle * handle, u8 flag);

#ifdef STA_SUPPORT
mlan_status woal_get_powermode(moal_private * priv, int *powermode);
mlan_status woal_set_scan_type(moal_private * priv, t_u32 scan_type);
mlan_status woal_set_powermode(moal_private * priv, char *powermode);
int woal_find_essid(moal_private * priv, mlan_ssid_bssid * ssid_bssid);
mlan_status woal_do_scan(moal_private * priv, wlan_user_scan_cfg * scan_cfg);
int woal_set_combo_scan(moal_private * priv, char *buf, int length);
mlan_status woal_get_band(moal_private * priv, int *band);
mlan_status woal_set_band(moal_private * priv, char *pband);
mlan_status woal_add_rxfilter(moal_private * priv, char *rxfilter);
mlan_status woal_remove_rxfilter(moal_private * priv, char *rxfilter);
mlan_status woal_set_qos_cfg(moal_private * priv, char *qos_cfg);
int woal_set_sleeppd(moal_private * priv, char *psleeppd);
mlan_status woal_set_rssi_low_threshold(moal_private * priv, char *rssi);
mlan_status woal_set_bg_scan(moal_private * priv, char *buf, int length);
mlan_status woal_stop_bg_scan(moal_private * priv);
void woal_reconfig_bgscan(moal_handle * handle);
#endif

#endif /* _MOAL_MAIN_H */
