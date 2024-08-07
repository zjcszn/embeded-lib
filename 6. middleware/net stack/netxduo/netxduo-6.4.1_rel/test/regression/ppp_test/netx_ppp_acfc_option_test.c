/* This tests the processing of compressed Access and Control fields.  */

#include "tx_api.h"
#include "nx_api.h"
#include "nx_ppp.h"
  
extern void         test_control_return(UINT status);

#if !defined(NX_DISABLE_IPV4) && defined(NX_PPP_COMPRESSION_ENABLE)

/* Define demo stack size.   */

#define DEMO_STACK_SIZE     2048
#define DEMO_DATA           "ABCDEFGHIJKLMNOPQRSTUVWXYZ "


/* Define the ThreadX and NetX object control blocks...  */

static TX_THREAD               thread_0;
static TX_THREAD               thread_1;
static NX_PACKET_POOL          pool_0;
static NX_IP                   ip_0;
static NX_IP                   ip_1;
static NX_PPP                  ppp_0;
static NX_PPP                  ppp_1;
static NX_UDP_SOCKET           socket_0;
static NX_UDP_SOCKET           socket_1;


/* Define the counters used in the demo application...  */

static ULONG                   error_counter = 0; 
static UINT                    thread_1_alive = NX_TRUE;
static ULONG                   ppp_0_link_up_counter;
static ULONG                   ppp_0_link_down_counter;
static ULONG                   ppp_1_link_up_counter;
static ULONG                   ppp_1_link_down_counter;

/* Define thread prototypes.  */
static void         thread_0_entry(ULONG thread_input);
static void         thread_1_entry(ULONG thread_input);
static void         ppp_0_serial_byte_output(UCHAR byte);
static void         ppp_1_serial_byte_output(UCHAR byte);
static void         invalid_packet_handler(NX_PACKET *packet_ptr);
static void         link_up_callback(NX_PPP *ppp_ptr);
static void         link_down_callback(NX_PPP *ppp_ptr);
static UINT         generate_login(CHAR *name, CHAR *password);
static UINT         verify_login(CHAR *name, CHAR *password);

/* UDP packet data with Access and Control fields compressed.
00 21 45 00 00 38 00 21 00 00 80 11 32 a8  01 02 03 05 01 02 03 04 00 89 00 88 00 24 e9 8e 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 20 00
*/
static UCHAR udp_data[] = {
0x7e, 0x7d, 0x20, 0x21, 0x45, 0x7d, 0x20, 0x7d, 0x20, 0x38, 0x7d, 0x20, 0x7d,
0x21, 0x7d, 0x20, 0x7d, 0x20, 0x80, 0x7d, 0x31, 0x32, 0xa8, 0x7d, 0x21, 0x7d, 0x22, 0x7d, 0x23,
0x7d, 0x24, 0x7d, 0x21, 0x7d, 0x22, 0x7d, 0x23, 0x7d, 0x25, 0x7d, 0x20, 0x88, 0x7d, 0x20, 0x89,
0x7d, 0x20, 0x24, 0x7d, 0x20, 0x7d, 0x20, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
0x5a, 0x20, 0x7d, 0x20, 0x6f, 0x5e, 0x7e,
};

static UINT udp_data_send = NX_FALSE;

/* Define what the initial system looks like.  */

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_ppp_acfc_option_test_application_define(void *first_unused_memory)
#endif
{

CHAR    *pointer;
UINT    status;


    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Create the thread 0.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            5, 5, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;
                   
    /* Create the thread 1.  */
    tx_thread_create(&thread_1, "thread 1", thread_1_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            5, 5, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status =  nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 2 * NX_PPP_MIN_PACKET_PAYLOAD, pointer, 2048);
    pointer = pointer + 2048;

    /* Check for pool creation error.  */
    if (status)
    {
        error_counter++;
    }

    /* Create the first PPP instance.  */
    status =  nx_ppp_create(&ppp_0, "PPP 0", &ip_0, pointer, 2048, 1, &pool_0, invalid_packet_handler, ppp_0_serial_byte_output);
    pointer =  pointer + 2048;

    /* Check for PPP create error.   */
    if (status)
    {
        error_counter++;
    }

    /* Define IP address. This PPP instance is effectively the server since it has both IP addresses. */
    status =  nx_ppp_ip_address_assign(&ppp_0, IP_ADDRESS(1, 2, 3, 4), IP_ADDRESS(1, 2, 3, 5));
    
    /* Check for PPP IP address assign error.   */
    if (status)
    {
        error_counter++;
    }

    /* Register the link up/down callbacks.  */
    status =  nx_ppp_link_up_notify(&ppp_0, link_up_callback);
    status += nx_ppp_link_down_notify(&ppp_0, link_down_callback);

    /* Check for PPP link up/down callback registration error(s).   */
    if (status)
    {
        error_counter++;
    }

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(0, 0, 0, 0), 0xFFFFF000UL, &pool_0, nx_ppp_driver,
                          pointer, 2048, 1);
    pointer =  pointer + 2048;

    /* Check for IP create error.   */
    if (status)
    {
        error_counter++;
    }

    /* Create the next PPP instance.  */
    status =  nx_ppp_create(&ppp_1, "PPP 1", &ip_1, pointer, 2048, 1, &pool_0, invalid_packet_handler, ppp_1_serial_byte_output);
    pointer =  pointer + 2048;

    /* Check for PPP create error.   */
    if (status)
    {
        error_counter++;
    }

    /* Define IP address. This PPP instance is effectively the client since it doesn't have any IP addresses. */
    status =  nx_ppp_ip_address_assign(&ppp_1, IP_ADDRESS(0, 0, 0, 0), IP_ADDRESS(0, 0, 0, 0));

    /* Check for PPP IP address assign error.   */
    if (status)
    {
        error_counter++;
    }

    /* Register the link up/down callbacks.  */
    status =  nx_ppp_link_up_notify(&ppp_1, link_up_callback);
    status += nx_ppp_link_down_notify(&ppp_1, link_down_callback);

    /* Check for PPP link up/down callback registration error(s).   */
    if (status)
    {
        error_counter++;
    }

    /* Create another IP instance.  */
    status = nx_ip_create(&ip_1, "NetX IP Instance 1", IP_ADDRESS(0, 0, 0, 0), 0xFFFFF000UL, &pool_0, nx_ppp_driver,
                           pointer, 2048, 1);
    pointer =  pointer + 2048;

    /* Check for IP create error.   */
    if (status)
    {
        error_counter++;
    }

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&ip_0);
    status += nx_udp_enable(&ip_1);
    if (status)
    {
        error_counter++;
    }
}


/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{

UINT        status;
ULONG       ip_status;
NX_PACKET   *my_packet;


    printf("NetX Test:   PPP ACFC Option Test......................................");

    if (error_counter)
    {

        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Wait for the link to come up.  */
    do
    {
    
        status =  nx_ip_status_check(&ip_0, NX_IP_LINK_ENABLED, &ip_status, 20 * NX_IP_PERIODIC_RATE);
    }while(status != NX_SUCCESS);
  
    /* Create a UDP socket.  */
    status = nx_udp_socket_create(&ip_0, &socket_0, "Socket 0", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);

    /* Check status.  */
    if (status)
    {
        error_counter++;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&socket_0, 0x88, NX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        error_counter++;
    }

    /* Disable checksum logic for this socket.  */
    nx_udp_socket_checksum_disable(&socket_0);

    /* Let receiver thread run.  */
    tx_thread_relinquish();

    /* Allocate a packet.  */
    status =  nx_packet_allocate(&pool_0, &my_packet, NX_UDP_PACKET, NX_WAIT_FOREVER);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
    }

    /* Write ABCs into the packet payload!  */
    nx_packet_data_append(my_packet, DEMO_DATA, sizeof(DEMO_DATA), &pool_0, NX_WAIT_FOREVER);

    /* Send the UDP packet.  */
    status =  nx_udp_socket_send(&socket_0, my_packet, IP_ADDRESS(1, 2, 3, 5), 0x89);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
    }

    /* Relinquish to thread 1.  */
    tx_thread_relinquish();

    /* Wait for the other thread to finish. */
    while(thread_1_alive)
    {
        tx_thread_sleep(1 * NX_IP_PERIODIC_RATE);
    }

    if (error_counter)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }
    else
    {
        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

void    thread_1_entry(ULONG thread_input)
{

UINT        status;
ULONG       ip_status;
NX_PACKET   *my_packet;


    /* Wait for the link to come up.  */
    do
    {
    
        status =  nx_ip_status_check(&ip_1, NX_IP_LINK_ENABLED, &ip_status, 20 * NX_IP_PERIODIC_RATE);
    }while(status != NX_SUCCESS);

    /* Create a UDP socket.  */
    status = nx_udp_socket_create(&ip_1, &socket_1, "Socket 1", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);

    /* Check status.  */
    if (status)
    {
        error_counter++;
        thread_1_alive = NX_FALSE;
        return;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&socket_1, 0x89, NX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        error_counter++;
        thread_1_alive = NX_FALSE;
        return;
    }

    /* Receive a UDP packet.  */
    status =  nx_udp_socket_receive(&socket_1, &my_packet, 5 * NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
    }
    else
    {

        /* Release the packet.  */
        status =  nx_packet_release(my_packet);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            error_counter++;
        }
    }

    thread_1_alive = NX_FALSE;

    return;

}

/* Define serial output routines.  Normally these routines would
   map to physical UART routines and the nx_ppp_byte_receive call
   would be made from a UART receive interrupt.  */

static void    ppp_0_serial_byte_output(UCHAR byte)
{
UINT i;

    if (ppp_0.nx_ppp_ipcp_state < NX_PPP_IPCP_COMPLETED_STATE)
    {

        /* Just feed the PPP 0 input routine.  */
        nx_ppp_byte_receive(&ppp_1, byte);
    }
    else if (udp_data_send == NX_FALSE)
    {
        /* Modify the data to send compressed protocol field.  */
        for (i = 0; i < sizeof(udp_data); i++)
        {
            nx_ppp_byte_receive(&ppp_1, udp_data[i]);
        }

        udp_data_send = NX_TRUE;
    }
}

static void    ppp_1_serial_byte_output(UCHAR byte)
{

    /* Just feed the PPP 0 input routine.  */
    nx_ppp_byte_receive(&ppp_0, byte);
}

static void invalid_packet_handler(NX_PACKET *packet_ptr)
{
    /* Print out the non-PPP byte. In Windows, the string "CLIENT" will
       be sent before Windows PPP starts. Once CLIENT is received, we need
       to send "CLIENTSERVER" to establish communication. It's also possible
       to receive modem commands here that might need some response to 
       continue.  */
    nx_packet_release(packet_ptr);
}

static void link_up_callback(NX_PPP *ppp_ptr)
{

    /* Just increment the link up counter.  */
    if (ppp_ptr == &ppp_0)
        ppp_0_link_up_counter++;
    else
        ppp_1_link_up_counter++;
}

static void link_down_callback(NX_PPP *ppp_ptr)
{

    /* Just increment the link down counter.  */
    if (ppp_ptr == &ppp_0)
        ppp_0_link_down_counter++;
    else
        ppp_1_link_down_counter++;

    /* Restart the PPP instance.  */
    nx_ppp_restart(ppp_ptr);
}
#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_ppp_acfc_option_test_application_define(void *first_unused_memory)
#endif
{

    /* Print out test information banner.  */
    printf("NetX Test:   PPP ACFC Option Test......................................N/A\n"); 

    test_control_return(3);  
}      
#endif

