#ifndef __USR_CFG_H
#define	__USR_CFG_H

#include "type.h"
#include "defs.h"



//内存优化选项，将一部分数组改为动态分配
//#ifdef STORE_AUTH_IE
//#define USE_LINUX_IOTRL				
//#define USE_FAKE_EFUSE
// #ifdef USE_RF_SHADOW
#define WPS_IE_ALLOC_DYNAMIC
//#define XMIT_BUFF_ALLOC_DYNAMIC

#define LINUX_VERSION_CODE		1
#define KERNEL_VERSION(a,b,c)	(0)

#define KERN_DEBUG
#define KERN_ERR
#define KERN_ALERT
#define KERN_CRIT
#define KERN_INFO

#define printk		  printf
#define panic		  printf

#define netif_carrier_ok(x)	1	//mark 需要实现
#define netif_carrier_off(x)
#define netif_stop_queue(x)
#define netif_start_queue(x)
#define netif_carrier_on(x)
#define netif_wake_queue(x)

#define sdio_claim_host(x)
#define sdio_release_host(x)

#define kfree 	mem_free
#define kmalloc(x,y) mem_malloc(x)
#define kzalloc(x,y) mem_calloc(x, 1)

//#define __constant_htons	htons

#define random32	get_random

#define HZ 			OS_TICKS_PER_SEC

#define  is_broadcast_ether_addr(addr)  (((addr)[0] & (addr)[1] & (addr)[2] & (addr)[3] & (addr)[4] & (addr)[5]) == 0xff)

#define  is_zero_ether_addr(addr)  (!((addr)[0] | (addr)[1] | (addr)[2] | (addr)[3] | (addr)[4] | (addr)[5]))

#define  is_multicast_ether_addr(addr)  (0x01 & (addr)[0])

#define  is_local_ether_addr(addr)  (0x02 & (addr)[0])

#define  is_unicast_ether_addr(addr)	!(is_multicast_ether_addr)

#define  is_valid_ether_addr(addr)	 (!is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr))

#define jiffies_to_msecs(x)	(x*(1000/OS_TICKS_PER_SEC))

typedef enum
{	
	MODE_STATION = 0,
	MODE_ADHOC,	
	MODE_AP,
	MODE_P2P_GO,
	MODE_P2P_GC,
	MODE_MAX_VALUE
}MODE_TYPE;

/*
struct scan_result_data{
	uint32_t   	num;
	uint8_t		mode;   			//工作模式	
	uint8_t		proto;   			//安全认证类型
	uint8_t		cipher;   			//加密模式
	uint8_t		channel;   			//频道
	uint32_t	freq;				//频率
	int32		rssi;				//信号强度
	uint8_t 	essid_len;
	uint8_t 	essid[32];  		// essid
	uint8_t 	bssid[6];  			//mac地址
}__packed;
*/

#define PACKET_HOST		0		/* To us		*/
#define PACKET_BROADCAST	1		/* To all		*/
#define PACKET_MULTICAST	2		/* To group		*/
#define PACKET_OTHERHOST	3		/* To someone else 	*/
#define PACKET_OUTGOING		4		/* Outgoing of any type */
/* These ones are invisible by user level */
#define PACKET_LOOPBACK		5		/* MC/BRD frame looped back */
#define PACKET_FASTROUTE	6		/* Fastrouted frame	*/


/** Maximum length of lines in configuration file */
#define MAX_CONFIG_LINE                 1024
/** Ethernet address length */
#define ETH_ALEN                        6
/** MAC BROADCAST */
#define MAC_BROADCAST   0x1FF
/** MAC MULTICAST */
#define MAC_MULTICAST   0x1FE

/** Character, 1 byte */
typedef char t_s8;
/** Unsigned character, 1 byte */
typedef unsigned char t_u8;

/** Short integer */
typedef signed short t_s16;
/** Unsigned short integer */
typedef unsigned short t_u16;

/** Integer */
typedef signed int t_s32;
/** Unsigned integer */
typedef unsigned int t_u32;

/** Long long integer */
typedef __int64 t_s64;
/** Unsigned long long integer */
typedef unsigned __int64 t_u64;

typedef unsigned int uint;
typedef unsigned int ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;


#define inline __inline

#define GFP_ATOMIC	1
#define GFP_KERNEL	2
#define GFP_DMA		4

#define sdio_set_drvdata(x,y) 	x->dev_data = y
#define sdio_get_drvdata(x) 	x->dev_data


#define SIOCDEVPRIVATE	0x89F0	/* to 89FF */

#define ETHERNET_HEADER_SIZE		14	/*!< Ethernet Header Length*/
#define LLC_HEADER_SIZE			6	/*!< LLC Header Length*/

/*
 *	TCP option
 */
 
#define TCPOPT_NOP		1	/* Padding */
#define TCPOPT_EOL		0	/* End of options */
#define TCPOPT_MSS		2	/* Segment size negotiating */
#define TCPOPT_WINDOW		3	/* Window scaling */
#define TCPOPT_SACK_PERM        4       /* SACK Permitted */
#define TCPOPT_SACK             5       /* SACK Block */
#define TCPOPT_TIMESTAMP	8	/* Better RTT estimations/PAWS */
#define TCPOPT_MD5SIG		19	/* MD5 Signature (RFC2385) */
#define TCPOPT_COOKIE		253	/* Cookie extension (experimental) */


/*
 *     TCP option lengths
 */

#define TCPOLEN_MSS            4
#define TCPOLEN_WINDOW         3
#define TCPOLEN_SACK_PERM      2
#define TCPOLEN_TIMESTAMP      10
#define TCPOLEN_MD5SIG         18
#define TCPOLEN_COOKIE_BASE    2	/* Cookie-less header extension */
#define TCPOLEN_COOKIE_PAIR    3	/* Cookie pair header extension */
//#define TCPOLEN_COOKIE_MIN     (TCPOLEN_COOKIE_BASE+TCP_COOKIE_MIN)
//#define TCPOLEN_COOKIE_MAX     (TCPOLEN_COOKIE_BASE+TCP_COOKIE_MAX)


/* Flags for encoding (along with the token) */
#define IW_ENCODE_INDEX		0x00FF	/* Token index (if needed) */
#define IW_ENCODE_FLAGS		0xFF00	/* Flags defined below */
#define IW_ENCODE_MODE		0xF000	/* Modes defined below */
#define IW_ENCODE_DISABLED	0x8000	/* Encoding disabled */
#define IW_ENCODE_ENABLED	0x0000	/* Encoding enabled */
#define IW_ENCODE_RESTRICTED	0x4000	/* Refuse non-encoded packets */
#define IW_ENCODE_OPEN		0x2000	/* Accept non-encoded packets */
#define IW_ENCODE_NOKEY		0x0800  /* Key is write only, so not present */
#define IW_ENCODE_TEMP		0x0400  /* Temporary key */

/* Power management flags available (along with the value, if any) */
#define IW_POWER_ON		0x0000	/* No details... */
#define IW_POWER_TYPE		0xF000	/* Type of parameter */
#define IW_POWER_PERIOD		0x1000	/* Value is a period/duration of  */
#define IW_POWER_TIMEOUT	0x2000	/* Value is a timeout (to go asleep) */
#define IW_POWER_MODE		0x0F00	/* Power Management mode */
#define IW_POWER_UNICAST_R	0x0100	/* Receive only unicast messages */
#define IW_POWER_MULTICAST_R	0x0200	/* Receive only multicast messages */
#define IW_POWER_ALL_R		0x0300	/* Receive all messages though PM */
#define IW_POWER_FORCE_S	0x0400	/* Force PM procedure for sending unicast */
#define IW_POWER_REPEATER	0x0800	/* Repeat broadcast messages in PM period */
#define IW_POWER_MODIFIER	0x000F	/* Modify a parameter */
#define IW_POWER_MIN		0x0001	/* Value is a minimum  */
#define IW_POWER_MAX		0x0002	/* Value is a maximum */
#define IW_POWER_RELATIVE	0x0004	/* Value is not in seconds/ms/us */

/* Transmit Power flags available */
#define IW_TXPOW_TYPE		0x00FF	/* Type of value */
#define IW_TXPOW_DBM		0x0000	/* Value is in dBm */
#define IW_TXPOW_MWATT		0x0001	/* Value is in mW */
#define IW_TXPOW_RELATIVE	0x0002	/* Value is in arbitrary units */
#define IW_TXPOW_RANGE		0x1000	/* Range of value between min/max */

/* Retry limits and lifetime flags available */
#define IW_RETRY_ON		0x0000	/* No details... */
#define IW_RETRY_TYPE		0xF000	/* Type of parameter */
#define IW_RETRY_LIMIT		0x1000	/* Maximum number of retries*/
#define IW_RETRY_LIFETIME	0x2000	/* Maximum duration of retries in us */
#define IW_RETRY_MODIFIER	0x00FF	/* Modify a parameter */
#define IW_RETRY_MIN		0x0001	/* Value is a minimum  */
#define IW_RETRY_MAX		0x0002	/* Value is a maximum */
#define IW_RETRY_RELATIVE	0x0004	/* Value is not in seconds/ms/us */
#define IW_RETRY_SHORT		0x0010	/* Value is for short packets  */
#define IW_RETRY_LONG		0x0020	/* Value is for long packets */

/* Scanning request flags */
#define IW_SCAN_DEFAULT		0x0000	/* Default scan of the driver */
#define IW_SCAN_ALL_ESSID	0x0001	/* Scan all ESSIDs */
#define IW_SCAN_THIS_ESSID	0x0002	/* Scan only this ESSID */
#define IW_SCAN_ALL_FREQ	0x0004	/* Scan all Frequencies */
#define IW_SCAN_THIS_FREQ	0x0008	/* Scan only this Frequency */
#define IW_SCAN_ALL_MODE	0x0010	/* Scan all Modes */
#define IW_SCAN_THIS_MODE	0x0020	/* Scan only this Mode */
#define IW_SCAN_ALL_RATE	0x0040	/* Scan all Bit-Rates */
#define IW_SCAN_THIS_RATE	0x0080	/* Scan only this Bit-Rate */
/* struct iw_scan_req scan_type */
#define IW_SCAN_TYPE_ACTIVE 0
#define IW_SCAN_TYPE_PASSIVE 1
/* Maximum size of returned data */
#define IW_SCAN_MAX_DATA	4096	/* In bytes */

/* Scan capability flags - in (struct iw_range *)->scan_capa */
#define IW_SCAN_CAPA_NONE		0x00
#define IW_SCAN_CAPA_ESSID		0x01
#define IW_SCAN_CAPA_BSSID		0x02
#define IW_SCAN_CAPA_CHANNEL	0x04
#define IW_SCAN_CAPA_MODE		0x08
#define IW_SCAN_CAPA_RATE		0x10
#define IW_SCAN_CAPA_TYPE		0x20
#define IW_SCAN_CAPA_TIME		0x40

#define mdelay	sleep

#define netif_device_detach(x)
#define netif_device_attach(x)


struct list_head {
	struct list_head *next, *prev;
};

struct list {
	struct list *next, *prev;
};

#ifndef offsetof
#define offsetof(type, member) ((long) &((type *) 0)->member)
#endif

#define container_of(ptr, type, member)  ((type *)( (char *)ptr - offsetof(type,member)))

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_for_each_entry(type, pos, head, member)				\
	for (pos = list_entry((head)->next, type, member);	\
	     &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, type, member))
	     
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)	

#define list_for_each_entry_safe(type, pos, n, head, member)			\
	for (pos = list_entry((head)->next, type, member),	\
		n = list_entry(pos->member.next, type, member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, type, member))
	     
#define list_first_entry(ptr, type, member) \
			list_entry((ptr)->next, type, member)

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

void INIT_LIST_HEAD(struct list_head *list);
void list_add_tail(struct list_head *new, struct list_head *head);
void list_add(struct list_head *new, struct list_head *head);
void list_del(struct list_head *entry);


void list_del(struct list_head *entry);

int copy_from_user(void *dst, void *src, int size);
#define copy_to_user	copy_from_user


#define __ATTRIB_ALIGN__

#define wait_queue_head_t	wait_event_t

#define spinlock_t   unsigned int

#define unlikely
#define likely

#define spin_lock_init(x)	*(x) = 0

#define spin_lock_irqsave(__lock, __irqflag)	\
do{												\
	OS_ENTER_CRITICAL_EX(*(__lock));			\
}while(0)	

#define spin_unlock_irqrestore(__lock , __irqflag)		\
do{												\
	OS_EXIT_CRITICAL_EX(*(__lock));					\
}while(0)

typedef u16  __sum16;
typedef u32  __wsum;

/* Modes of operation */
#define IW_MODE_AUTO	0	/* Let the driver decides */
#define IW_MODE_ADHOC	1	/* Single cell network */
#define IW_MODE_INFRA	2	/* Multi cell network, roaming, ... */
#define IW_MODE_MASTER	3	/* Synchronisation master or Access Point */
#define IW_MODE_REPEAT	4	/* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND	5	/* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR	6	/* Passive monitor (listen only) */
#define IW_MODE_MESH	7	/* Mesh (IEEE 802.11s) network */


#define ZEROSIZE 1

struct iphdr {
	__u8	ihl:4,
		version:4;
	__u8	tos;
	__be16	tot_len;
	__be16	id;
	__be16	frag_off;
	__u8	ttl;
	__u8	protocol;
	__sum16	check;
	__be32	saddr;
	__be32	daddr;
	/*The options start here. */
};

struct tcphdr {
	__be16	source;
	__be16	dest;
	__be32	seq;
	__be32	ack_seq;
	__u16	res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		ece:1,
		cwr:1;
	__be16	window;
	__sum16	check;
	__be16	urg_ptr;
};




#define udelay delay_us

#define EXPORT_SYMBOL(x)

#define free_netdev mem_free

#ifndef DEFINED_TIMEVAL_STRUCT
#define DEFINED_TIMEVAL_STRUCT
struct timeval {
  long    tv_sec;         /* seconds */
  long    tv_usec;        /* and microseconds */
};
#endif

void do_gettimeofday(struct timeval *t);
union ktime timeval_to_ktime(struct timeval *t);

#define THIS_MODULE	0

#define ARRAY_SIZE(x,y)	(sizeof(x)/sizeof(y)) 

int	try_module_get(int);
int	module_put(int);
int ieee80211_frequency_to_channel(int freq);

#endif
