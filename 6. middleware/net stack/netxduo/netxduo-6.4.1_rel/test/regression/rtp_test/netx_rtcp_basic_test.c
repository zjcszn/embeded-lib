#include "tx_api.h"
#include "nx_api.h"
#include "netxtestcontrol.h"

extern void test_control_return(UINT);

#if !defined(NX_DISABLE_IPV4) && defined(__PRODUCT_NETXDUO__) && !defined(NX_DISABLE_PACKET_CHAIN)
#include    "nx_rtp_sender.h"

#define DEMO_STACK_SIZE            4096

#define NUM_PACKETS                24
#define PACKET_SIZE                1536
#define PACKET_POOL_SIZE           (NUM_PACKETS * (PACKET_SIZE + sizeof(NX_PACKET)))

#define RTP_SERVER_ADDRESS         IP_ADDRESS(1,2,3,4)
#define RTP_CLIENT_ADDRESS         IP_ADDRESS(1,2,3,5)
#define RTP_CLIENT_RTP_PORT        6002
#define RTP_CLIENT_RTCP_PORT       6003
#define RTP_PAYLOAD_TYPE_VIDEO     96
#define RTP_PAYLOAD_TYPE_AUDIO     97
#define CNAME                      "AzureRTOS@microsoft.com"

/* Define test data. */
#define TEST_TIMESTAMP             1234
#define TEST_MSW                   123
#define TEST_LSW                   456
#define TEST_TIME_DURATION         (6 * TX_TIMER_TICKS_PER_SECOND)

static UCHAR test_rtp_packet_data[] = "test rtp packet data";

/* Define the ThreadX object control blocks...  */

static TX_THREAD                   ntest_0;
static TX_THREAD                   ntest_1;

static NX_PACKET_POOL              pool_0;
static NX_IP                       ip_0;
static NX_IP                       ip_1;
static NX_UDP_SOCKET               rtcp_client_socket;

/* Define rtp sender control block.  */
static NX_RTP_SENDER               rtp_0;
static NX_RTP_SESSION              rtp_session_0;
static NX_RTP_SESSION              rtp_session_1;
static NX_RTP_SESSION              rtp_session_2;

/* Define the counters used in the test application...  */
static UINT                        rtcp_packet_counter = 0;

/* Define thread prototypes.  */

static void ntest_0_entry(ULONG thread_input);
static void ntest_1_entry(ULONG thread_input);
extern void _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);
extern void test_control_return(UINT status);

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_rtcp_basic_test_application_define(void *first_unused_memory)
#endif
{

CHAR       *pointer;
UINT        status;

    /* Print out test information banner.  */
    printf("NetX Test:   RTCP Basic Test............................................");

    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Create the server thread.  */
    tx_thread_create(&ntest_0, "thread 0", ntest_0_entry, 0,
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer = pointer + DEMO_STACK_SIZE;

    /* Create the client thread.  */
    tx_thread_create(&ntest_1, "thread 1", ntest_1_entry, 0,
                     pointer, DEMO_STACK_SIZE, 
                     3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer = pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", PACKET_SIZE, pointer, PACKET_POOL_SIZE);
    pointer = pointer + PACKET_POOL_SIZE;
    CHECK_STATUS(0, status);

    /* Create server IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", RTP_SERVER_ADDRESS, 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer = pointer + 2048;
    CHECK_STATUS(0, status);

    /* Create client IP instance.  */
    status = nx_ip_create(&ip_1, "NetX IP Instance 1", RTP_CLIENT_ADDRESS, 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer = pointer + 2048;
    CHECK_STATUS(0, status);

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;
    CHECK_STATUS(0, status);

    /* Enable ARP and supply ARP cache memory for IP Instance 1.  */
    status = nx_arp_enable(&ip_1, (void *) pointer, 1024);
    pointer = pointer + 1024;
    CHECK_STATUS(0, status);

    /* Enable UDP processing for both IP instances.  */
    status = nx_udp_enable(&ip_0);
    CHECK_STATUS(0, status);
    status = nx_udp_enable(&ip_1);
    CHECK_STATUS(0, status);
}

/* Define server threads.  */
static void    ntest_0_entry(ULONG thread_input)
{
UINT        status;
NXD_ADDRESS client_ip_address;
NX_PACKET  *send_packet;
UINT        time_start;

    /* Create RTP sender.  */
    status = nx_rtp_sender_create(&rtp_0, &ip_0, &pool_0, CNAME, sizeof(CNAME) - 1);
    CHECK_STATUS(0, status);
    rtp_session_0.nx_rtp_session_ssrc = 11478;

    /* Setup rtp sender session.  */
    client_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
    client_ip_address.nxd_ip_address.v4 = RTP_CLIENT_ADDRESS;
    status = nx_rtp_sender_session_create(&rtp_0, &rtp_session_0, RTP_PAYLOAD_TYPE_VIDEO,
                                          0, &client_ip_address,
                                          RTP_CLIENT_RTP_PORT, RTP_CLIENT_RTCP_PORT);
    CHECK_STATUS(0, status);

    status = nx_rtp_sender_session_create(&rtp_0, &rtp_session_1, RTP_PAYLOAD_TYPE_AUDIO,
                                          0, &client_ip_address,
                                          RTP_CLIENT_RTP_PORT, RTP_CLIENT_RTCP_PORT);
    CHECK_STATUS(0, status);

    status = nx_rtp_sender_session_create(&rtp_0, &rtp_session_2, RTP_PAYLOAD_TYPE_AUDIO,
                                          0, &client_ip_address,
                                          RTP_CLIENT_RTP_PORT, RTP_CLIENT_RTCP_PORT);
    CHECK_STATUS(0, status);

    /* If more than one rtp packet is sent during the first tick, rtcp packet will also be sent more than once.
       To make a stable test result, wait for a tick here to avoid this situation. */
    tx_thread_sleep(1);

    time_start = tx_time_get();

    while(tx_time_get() - time_start < TEST_TIME_DURATION)
    {
        /* Allocate a packet */
        status = nx_rtp_sender_session_packet_allocate(&rtp_session_0, &send_packet, 5 * NX_IP_PERIODIC_RATE);
        CHECK_STATUS(0, status);

        /* Copy payload data into the packet. */
        status = nx_packet_data_append(send_packet, (void*)test_rtp_packet_data, sizeof(test_rtp_packet_data), rtp_0.nx_rtp_sender_ip_ptr->nx_ip_default_packet_pool, 5 * NX_IP_PERIODIC_RATE);
        CHECK_STATUS(0, status);

        status = nx_rtp_sender_session_packet_send(&rtp_session_0, send_packet, TEST_TIMESTAMP, TEST_MSW, TEST_LSW, 1);

        CHECK_STATUS(0, status);
    }

    /* Check if there is memory leak. */
    CHECK_STATUS(pool_0.nx_packet_pool_total, pool_0.nx_packet_pool_available);

    /* Check rtcp packet count. */
    CHECK_STATUS(TEST_TIME_DURATION / (TX_TIMER_TICKS_PER_SECOND * NX_RTCP_INTERVAL) + 1, rtcp_packet_counter);

    status = nx_rtp_sender_session_delete(&rtp_session_2);
    CHECK_STATUS(0, status);

    status = nx_rtp_sender_session_delete(&rtp_session_1);
    CHECK_STATUS(0, status);

    status = nx_rtp_sender_session_delete(&rtp_session_0);
    CHECK_STATUS(0, status);

    /* Return the test result.  */
    printf("SUCCESS!\n");
    test_control_return(0);
}

/* Define the client threads.  */
static void    ntest_1_entry(ULONG thread_input)
{
NX_PACKET *received_packet;
UINT       status;

    /* Create the rtp client socket.  */
    status = nx_udp_socket_create(&ip_1, &rtcp_client_socket, "RTCP Client Socket", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);
    CHECK_STATUS(0, status);

    status =  nx_udp_socket_bind(&rtcp_client_socket, RTP_CLIENT_RTCP_PORT, NX_IP_PERIODIC_RATE);
    CHECK_STATUS(0, status);

    while(nx_udp_socket_receive(&rtcp_client_socket, &received_packet, (NX_RTCP_INTERVAL + 1) * TX_TIMER_TICKS_PER_SECOND) == NX_SUCCESS)
    {
        rtcp_packet_counter++;;
        nx_packet_release(received_packet);
    }
}

#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_rtcp_basic_test_application_define(void *first_unused_memory)
#endif
{

    /* Print out test information banner.  */
    printf("NetX Test:   RTCP Basic Test............................................N/A\n"); 

    test_control_return(3);
}      
#endif

