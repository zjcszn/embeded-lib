/*
 * Copyright (c) 2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LWIP_LWIPOPTS_H
#define LWIP_LWIPOPTS_H

/* Options managed by RTE */
#include "FreeRTOS.h"
#include "net_sntp.h"

/* See "lwip/opt.h" for all available configuration options */

#define LWIP_DEBUG
//#define LWIP_NOASSERT

/*
   ------------------------------------
   -------------- NO SYS --------------
   ------------------------------------
#define NO_SYS               managed by RTE
#define LWIP_TIMERS          managed by RTE
#define SYS_LIGHTWEIGHT_PROT managed by RTE
*/

#define NO_SYS                          0
#define SYS_LIGHTWEIGHT_PROT            (!NO_SYS)
#define LWIP_ETH_THREAD_PRIO            (configMAX_PRIORITIES - 2)
#define LWIP_THREAD_CORE_PRIO           (configMAX_PRIORITIES - 1)
#define LWIP_APP_THREAD_PRIO            (24)

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
#define MEM_LIBC_MALLOC                 0
#define MEMP_MEM_MALLOC                 0
#define MEMP_MEM_INIT                   0
#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        (10*1024)
#define MEMP_OVERFLOW_CHECK             1
#define MEMP_SANITY_CHECK               0
#define MEM_OVERFLOW_CHECK              1
#define MEM_SANITY_CHECK                0

/* ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
 */
#define MEMP_NUM_PBUF                   24
#define MEMP_NUM_RAW_PCB                8
#define MEMP_NUM_UDP_PCB                8
#define MEMP_NUM_TCP_PCB                12
#define MEMP_NUM_TCP_PCB_LISTEN         8
#define MEMP_NUM_TCP_SEG                32
#define MEMP_NUM_REASSDATA              12
#define MEMP_NUM_FRAG_PBUF              8
#define MEMP_NUM_ARP_QUEUE              4
#define MEMP_NUM_IGMP_GROUP             4
#define MEMP_NUM_NETBUF                 8
#define MEMP_NUM_NETCONN                8
#define MEMP_NUM_SELECT_CB              4
#define MEMP_NUM_TCPIP_MSG_API          8
#define MEMP_NUM_TCPIP_MSG_INPKT        8
#define MEMP_NUM_NETDB                  2
#define MEMP_NUM_LOCALHOSTLIST          1
#define MEMP_NUM_SYS_TIMEOUT            (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 8)
#define PBUF_POOL_SIZE                  12

/*
   ---------------------------------
   ---------- PPP options ----------
   ---------------------------------
*/
#define PPP_SUPPORT                     0

/*
   ---------------------------------
   ---------- ARP options ----------
   ---------------------------------
#define LWIP_ETHERNET      managed by RTE
*/
#define LWIP_ETHERNET                   1
#define LWIP_ARP                        1
#define ARP_TABLE_SIZE                  4
#define ETHARP_SUPPORT_VLAN             0
#define ETH_PAD_SIZE                    0
#define ETHARP_SUPPORT_STATIC_ENTRIES   0

/*
   --------------------------------
   ---------- IP options ----------
   --------------------------------
#define LWIP_IPV4          managed by RTE
*/
#define LWIP_IPV4                       1
#define IP_FORWARD                      0
#define IP_REASSEMBLY                   1
#define IP_FRAG                         1
#define IP_OPTIONS_ALLOWED              0
#define IP_REASS_MAX_PBUFS              12
#define IP_DEFAULT_TTL                  128
#define IP_SOF_BROADCAST                1
#define IP_SOF_BROADCAST_RECV           1

/*
   ----------------------------------
   ---------- ICMP options ----------
   ----------------------------------
*/
#define LWIP_ICMP                       1

/*
   ---------------------------------
   ---------- RAW options ----------
   ---------------------------------
*/
#define LWIP_RAW                        1

/*
   ----------------------------------
   ---------- DHCP options ----------
   ----------------------------------
*/
#define LWIP_DHCP                       1
#define LWIP_DHCP_BOOTP_FILE            0
#define LWIP_DHCP_GET_NTP_SRV           1
#define LWIP_DHCP_MAX_NTP_SERVERS       3
#define LWIP_DHCP_MAX_TRIES             3       /* User define: if tries > maxtries, set static ip... */

/*
   ------------------------------------
   ---------- AUTOIP options ----------
   ------------------------------------
*/
#define LWIP_AUTOIP                     0
#define LWIP_DHCP_AUTOIP_COOP           0
#define LWIP_DHCP_AUTOIP_COOP_TRIES     5

/*
   ----------------------------------
   ---------- IGMP options ----------
   ----------------------------------
*/
#define LWIP_IGMP                       1

/*
   ----------------------------------
   ---------- DNS options -----------
   ----------------------------------
*/
#define LWIP_DNS                        1
#define DNS_TABLE_SIZE                  4
#define DNS_MAX_NAME_LENGTH             256

/*
   ----------------------------------
   --------- SNTP options -----------
   ----------------------------------
*/
#define LWIP_SNTP                       1
#define SNTP_SET_SYSTEM_TIME            sntp_set_system_time
#define SNTP_STARTUP_DELAY_FUNC         5000
#define SNTP_MAX_SERVERS                LWIP_DHCP_MAX_NTP_SERVERS
#define SNTP_GET_SERVERS_FROM_DHCP      LWIP_DHCP_GET_NTP_SRV
#define SNTP_UPDATE_DELAY               3600000
#define SNTP_SERVER_DNS                 0
#if     SNTP_SERVER_DNS
#define SNTP_SERVER_ADDRESS             "ntp1.aliyun.com"
#endif
#define SNTP_CHECK_RESPONSE             1
#define SNTP_DEBUG                      LWIP_DBG_OFF


/*
   ---------------------------------
   ---------- UDP options ----------
   ---------------------------------
*/
#define LWIP_UDP                        1
#define LWIP_UDPLITE                    0

/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/
#define LWIP_TCP                        1
#define TCP_QUEUE_OOSEQ                 0
#define LWIP_TCP_SACK_OUT               0
#define LWIP_TCP_MAX_SACK_NUM           4
#define TCP_MSS                         1460    /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */
#define TCP_OOSEQ_MAX_BYTES             0
#define TCP_OOSEQ_MAX_PBUFS             0
#define TCP_LISTEN_BACKLOG              1
#define TCP_DEFAULT_LISTEN_BACKLOG      0xff
#define LWIP_EVENT_API                  0
#define LWIP_CALLBACK_API               1
#define LWIP_WND_SCALE                  0
#define TCP_RCV_SCALE                   0
#define LWIP_TCP_PCB_NUM_EXT_ARGS       0

#define LWIP_ALTCP                      1
#define LWIP_ALTCP_TLS                  0       /* port to mbedtls */
#define LWIP_ALTCP_TLS_MBEDTLS          0

#define TCP_WND                         (8 * TCP_MSS)
#define TCP_SND_BUF                     (8 * TCP_MSS)

/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
  as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */
#define TCP_SND_QUEUELEN                (4 * TCP_SND_BUF/TCP_MSS)

/*
   ----------------------------------
   ---------- Pbuf options ----------
   ----------------------------------
*/
#define PBUF_LINK_ENCAPSULATION_HLEN    0

/*
   ------------------------------------------------
   ---------- Network Interfaces options ----------
   ------------------------------------------------
*/
#define LWIP_SINGLE_NETIF               0
#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_API                  0
#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_EXT_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_REMOVE_CALLBACK      0
#define LWIP_NETIF_HWADDRHINT           1
#define LWIP_NUM_NETIF_CLIENT_DATA      1

/*
   ------------------------------------
   ---------- LOOPIF options ----------
   ------------------------------------
*/
#define LWIP_HAVE_LOOPIF                1
#define LWIP_LOOPIF_MULTICAST           0
#define LWIP_NETIF_LOOPBACK             1
#define LWIP_LOOPBACK_MAX_PBUFS         0

/*
   ------------------------------------
   ---------- Thread options ----------
   ------------------------------------
*/
#define TCPIP_THREAD_NAME               "tcpip_thread"
#define TCPIP_THREAD_STACKSIZE          (4096 * 2)
#define TCPIP_THREAD_PRIO               LWIP_THREAD_CORE_PRIO
#define TCPIP_MBOX_SIZE                 10
#define SLIPIF_THREAD_NAME              "slipif_loop"
#define SLIPIF_THREAD_STACKSIZE         1024
#define SLIPIF_THREAD_PRIO              LWIP_THREAD_CORE_PRIO
#define DEFAULT_THREAD_NAME             "lwIP"
#define DEFAULT_THREAD_STACKSIZE        2048
#define DEFAULT_THREAD_PRIO             LWIP_THREAD_CORE_PRIO
#define DEFAULT_RAW_RECVMBOX_SIZE       10
#define DEFAULT_UDP_RECVMBOX_SIZE       10
#define DEFAULT_TCP_RECVMBOX_SIZE       10
#define DEFAULT_ACCEPTMBOX_SIZE         10

/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
#define LWIP_NETCONN                    (NO_SYS == 0)
#define LWIP_TCPIP_TIMEOUT              1
#define LWIP_NETCONN_SEM_PER_THREAD     1
#define LWIP_NETCONN_FULLDUPLEX         1

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
#define LWIP_SOCKET                     (NO_SYS == 0)
#define LWIP_COMPAT_SOCKETS             1
#define LWIP_POSIX_SOCKETS_IO_NAMES     1
#define LWIP_TCP_KEEPALIVE              1
#define LWIP_SO_SNDTIMEO                1
#define LWIP_SO_RCVTIMEO                1
#define LWIP_SO_SNDRCVTIMEO_NONSTANDARD 0
#define LWIP_SO_RCVBUF                  1
#define LWIP_SO_LINGER                  0
#define SO_REUSE                        0
#define SO_REUSE_RXTOALL                0
#define LWIP_SOCKET_SELECT              1
#define LWIP_SOCKET_POLL                1

/*
   ----------------------------------------
   ---------- Statistics options ----------
   ----------------------------------------
*/
#define LWIP_STATS                      0

/*
   ---------------------------------------
   ---------- IPv6 options ---------------
   ---------------------------------------
#define LWIP_IPV6           managed by RTE
*/
#define LWIP_IPV6                       0
#define LWIP_IPV6_FORWARD               0
#define LWIP_IPV6_FRAG                  1
#define LWIP_IPV6_REASS                 1
#define LWIP_IPV6_AUTOCONFIG            1
#define LWIP_ICMP6                      1
#define LWIP_IPV6_MLD                   0
#define MEMP_NUM_MLD6_GROUP             4
#define LWIP_ND6_NUM_NEIGHBORS          10
#define LWIP_ND6_NUM_DESTINATIONS       10
#define LWIP_ND6_NUM_PREFIXES           5
#define LWIP_ND6_NUM_ROUTERS            3
#define LWIP_ND6_ALLOW_RA_UPDATES       1
#define LWIP_ND6_TCP_REACHABILITY_HINTS 1
#define LWIP_IPV6_DHCP6                 0
#define LWIP_IPV6_DHCP6_STATEFUL        0
#define LWIP_IPV6_DHCP6_STATELESS       1
#define LWIP_DHCP6_GET_NTP_SRV          0
#define LWIP_DHCP6_MAX_NTP_SERVERS      1

/*
   ----------------------------------
   ---------- MDNS options -----------
   ----------------------------------
*/
#define LWIP_MDNS_RESPONDER             0
#define MDNS_MAX_SERVICES               1
#define MDNS_RESP_USENETIF_EXTCALLBACK  LWIP_NETIF_EXT_STATUS_CALLBACK
#define MDNS_DEBUG                      LWIP_DBG_OFF

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

/*
The STM32H7xx allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
#define CHECKSUM_BY_HARDWARE

#ifdef CHECKSUM_BY_HARDWARE
/* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
#define CHECKSUM_GEN_IP                 0
/* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
#define CHECKSUM_GEN_UDP                0
/* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
#define CHECKSUM_GEN_TCP                0
/* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
#define CHECKSUM_CHECK_IP               0
/* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
#define CHECKSUM_CHECK_UDP              0
/* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
#define CHECKSUM_CHECK_TCP              0
/* CHECKSUM_GEN_ICMP==1: Check checksums by hardware for outgoing ICMP packets.*/
/* Hardware TCP/UDP checksum insertion not supported when packet is an IPv4 fragment*/
#define CHECKSUM_GEN_ICMP               0
/* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
#define CHECKSUM_CHECK_ICMP             0

#else
/* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
#define CHECKSUM_GEN_IP                 1
/* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
#define CHECKSUM_GEN_UDP                1
/* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
#define CHECKSUM_GEN_TCP                1
/* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
#define CHECKSUM_CHECK_IP               1
/* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
#define CHECKSUM_CHECK_UDP              1
/* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
#define CHECKSUM_CHECK_TCP              1
/* CHECKSUM_GEN_ICMP==1: Check checksums by hardware for outgoing ICMP packets.*/
#define CHECKSUM_GEN_ICMP               1
/* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
#define CHECKSUM_CHECK_ICMP             1
#endif


/*
   --------------------------------------
   ---------- IP ------------------------
   --------------------------------------
*/
/* Static IP address */
#define IP_ADDR0                    192
#define IP_ADDR1                    168
#define IP_ADDR2                    137
#define IP_ADDR3                    25

/* NET mask*/
#define NETMASK_ADDR0               255
#define NETMASK_ADDR1               255
#define NETMASK_ADDR2               255
#define NETMASK_ADDR3               0

/* Gateway address */
#define GW_ADDR0                    192
#define GW_ADDR1                    168
#define GW_ADDR2                    137
#define GW_ADDR3                    1

/* DNS server address */
#define DNS1_ADDR0                  114
#define DNS1_ADDR1                  114
#define DNS1_ADDR2                  114
#define DNS1_ADDR3                  114

/*
   --------------------------------------
   ------------- HTTP Server-- ----------
   --------------------------------------
*/

#define LWIP_HTTPD_SUPPORT_POST             1
#define LWIP_HTTPD_CGI                      1
#define LWIP_HTTPD_CGI_SSI                  0
#define LWIP_HTTPD_SSI                      1
#define LWIP_HTTPD_SSI_INCLUDE_TAG          0
#define LWIP_HTTPD_MAX_TAG_NAME_LEN         16
#define LWIP_HTTPD_SSI_RAW                  1
#define LWIP_HTTPD_MAX_CGI_PARAMETERS       16
#define LWIP_HTTPD_DYNAMIC_HEADERS          1
#define HTTPD_SERVER_PORT                   LWIP_IANA_PORT_HTTP
#define LWIP_HTTPD_SUPPORT_EXTSTATUS        0
#define LWIP_HTTPD_SUPPORT_V09              0
#define LWIP_HTTPD_SUPPORT_11_KEEPALIVE     1
#define LWIP_HTTPD_SUPPORT_REQUESTLIST      1
#define HTTPD_MAX_RETRIES                   3
#define LWIP_HTTPD_FILE_EXTENSION           1
#define LWIP_HTTPD_CUSTOM_FILES             1
#define LWIP_HTTPD_DYNAMIC_HEADERS          1
#define LWIP_HTTPD_DYNAMIC_FILE_READ        1
#define LWIP_HTTPD_SUPPORT_AUTH_COOKIE      1
#define LWIP_HTTPD_FILE_STATE               0
#define LWIP_HTTPD_AUTH_COOKIE_NAME         "Session"
#define LWIP_HTTPD_AUTH_COOKIE_LENGTH       16
#define LWIP_HTTPD_AUTH_COOKIE_MAX_AGE      (30)   // 30 min
#define LWIP_HTTPD_AUTH_DENIED_FILE         "/index.shtml"
#define LWIP_HTTPD_AUTH_SESSION_MAXNUM      5



/*
   --------------------------------------
   ------------- LWIP Debug -------------
   --------------------------------------
*/

#ifdef  LWIP_DEBUG
#define LWIP_DBG_MIN_LEVEL          LWIP_DBG_LEVEL_ALL
#define PPP_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define DNS_DEBUG                   LWIP_DBG_OFF
#define AUTOIP_DEBUG                LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define IGMP_DEBUG                  LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define HTTPD_DEBUG                 LWIP_DBG_OFF
#define COOKIES_DEBUG               LWIP_DBG_ON
#endif

#if defined (MEM_USE_POOLS) && MEM_USE_POOLS
#define MEMP_USE_CUSTOM_POOLS       1
#endif

#endif /* LWIP_LWIPOPTS_H */
