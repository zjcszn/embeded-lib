/* 12.24 TCP SHOULD send MSS option in every SYN segment
   when its receive MSS differs from the default 536.   */

/* Procedure
   1. Client send a SYN to Server.
   2. Use advanced_packet_process function to receive and deal with the SYN packet, 
      then change the MSS.
*/

#include    "tx_api.h"
#include    "nx_api.h"
#include    "nx_tcp.h"
#include    "nx_ip.h"
extern void test_control_return(UINT status);

#if !defined(NX_DISABLE_IPV4)

#define     DEMO_STACK_SIZE    2048


/* Define the ThreadX and NetX object control blocks...  */


static TX_THREAD               ntest_0;
static TX_THREAD               ntest_1;
static NX_PACKET_POOL          pool_0;
static NX_IP                   ip_0;
static NX_IP                   ip_1;
static NX_TCP_SOCKET           client_socket;
static NX_TCP_SOCKET           server_socket;

/* Define the counters used in the demo application...  */

static ULONG                   error_counter;
static ULONG                   syn_counter;
static ULONG                   syn_ack_counter;

/* Define thread prototypes.  */
static void    ntest_0_entry(ULONG thread_input);
static void    ntest_0_connect_received(NX_TCP_SOCKET *server_socket, UINT port);
static void    ntest_0_disconnect_received(NX_TCP_SOCKET *server_socket);
static void    ntest_1_entry(ULONG thread_input);
extern void    _nx_ram_network_driver_1500(struct NX_IP_DRIVER_STRUCT *driver_req);
static void    my_tcp_packet_receive_12_24(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
static void    my_tcp_packet_receive_12_24_2(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
/* Define what the initial system looks like.  */

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void           netx_12_24_application_define(void *first_unused_memory)
#endif
{
CHAR       *pointer;
UINT       status;

    /* Setup the working pointer.  */
    pointer = (CHAR *) first_unused_memory;

    error_counter = 0;
    syn_counter = 0;
    syn_ack_counter = 0;



    /* Create the main thread.  */
    tx_thread_create(&ntest_0, "thread 1", ntest_0_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer = pointer + DEMO_STACK_SIZE;

    /* Create the main thread.  */
    tx_thread_create(&ntest_1, "thread 0", ntest_1_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer = pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 1536, pointer, 1536*16);
    pointer = pointer + 1536*16;

    if(status)
        error_counter++;

    /* Create another IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(1, 2, 3, 5), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver_1500,
                          pointer, 2048, 1);
    pointer = pointer + 2048;

    if(status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_1, "NetX IP Instance 1", IP_ADDRESS(1, 2, 3, 4), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver_1500,
                          pointer, 2048, 1);
    pointer = pointer + 2048;

    if(status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for IP Instance 1.  */
    status = nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status += nx_arp_enable(&ip_1, (void *) pointer, 1024);
    pointer = pointer + 1024;

    if(status)
        error_counter++;

    /* Enable ICMP for IP Instance 0 and 1.  */
    status = nx_icmp_enable(&ip_0);
    status += nx_icmp_enable(&ip_1);


    /* Check ARP enable status.  */
    if(status)
        error_counter++;

    /* Enable TCP processing for both IP instances.  */
    status = nx_tcp_enable(&ip_0);
    status += nx_tcp_enable(&ip_1);


    /* Check TCP enable status.  */
    if(status)
        error_counter++;
}

/* Define the test threads.  */

static void    ntest_0_entry(ULONG thread_input)
{
UINT       status;
ULONG      actual_status;


    /* Ensure the IP instance has been initialized.  */
    status = nx_ip_status_check(&ip_0, NX_IP_INITIALIZE_DONE, &actual_status, NX_IP_PERIODIC_RATE);

    /* Check status...  */
    if(status)
        error_counter++;

    /* Create a socket.  */
    status = nx_tcp_socket_create(&ip_0, &server_socket, "Server Socket", 
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 200,
                                  NX_NULL, ntest_0_disconnect_received);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Setup this thread to listen.  */
    status = nx_tcp_server_socket_listen(&ip_0, 12, &server_socket, 5, ntest_0_connect_received);

    /* Check for error.  */
    if(status)
        error_counter++;

    /*Let the server socket to check the SYN segment*/
    ip_0.nx_ip_tcp_packet_receive = my_tcp_packet_receive_12_24;

    /* Accept a client socket connection.  */
    status = nx_tcp_server_socket_accept(&server_socket, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Disconnect the server socket.  */
    status = nx_tcp_socket_disconnect(&server_socket, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Unaccept the server socket.  */
    status = nx_tcp_server_socket_unaccept(&server_socket);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Unlisten on the server port.  */
    status =  nx_tcp_server_socket_unlisten(&ip_0, 12);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Delete the socket.  */
    status = nx_tcp_socket_delete(&server_socket);

    /* Check for error.  */
    if(status)
        error_counter++;

}


static void    ntest_1_entry(ULONG thread_input)
{
UINT       status;

    /* Print out test information banner.  */
    printf("NetX Test:   TCP Spec 12.24 Test.......................................");

    /* Check for earlier error.  */
    if(error_counter)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Create a socket.  */
    status = nx_tcp_socket_create(&ip_1, &client_socket, "Client Socket", 
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 300,
                                  NX_NULL, NX_NULL);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Bind the socket.  */
    status = nx_tcp_client_socket_bind(&client_socket, 12, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if(status)
        error_counter++;

    /*Let the client socket to check the SYN+ACK segment*/
    ip_1.nx_ip_tcp_packet_receive = my_tcp_packet_receive_12_24_2;


    /* Call connect to send a SYN  */ 
    status = nx_tcp_client_socket_connect(&client_socket, IP_ADDRESS(1, 2, 3, 5), 12, 5 * NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Disconnect this socket.  */
    status = nx_tcp_socket_disconnect(&client_socket, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Unbind the socket.  */
    status = nx_tcp_client_socket_unbind(&client_socket);

    /* Check for error.  */
    if(status)
        error_counter++;

    status = nx_tcp_socket_delete(&client_socket);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Determine if the test was successful.  */
    if((error_counter !=0) || (syn_counter != 1) || (syn_ack_counter != 1))
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



static void    ntest_0_connect_received(NX_TCP_SOCKET *socket_ptr, UINT port)
{

    /* Check for the proper socket and port.  */
    if((socket_ptr != &server_socket) || (port != 12))
        error_counter++;
}

static void    ntest_0_disconnect_received(NX_TCP_SOCKET *socket)
{

    /* Check for proper disconnected socket.  */
    if(socket != &server_socket)
        error_counter++;
}


static void    my_tcp_packet_receive_12_24(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_TCP_HEADER *header_ptr;
ULONG         option_words;
ULONG         mss_option;

    header_ptr = (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);

    /* Client receives a SYN packet.  */
    if((header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT) && !(header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT) && !(header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT))
    {
        /* It is a SYN packet.  */
        option_words =  (header_ptr -> nx_tcp_header_word_3 >> 28) - 5;
                              
        /*Get the MSS option form the SYN packet.*/
        _nx_tcp_mss_option_get((packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_TCP_HEADER)), option_words*sizeof(ULONG), &mss_option);

        if(mss_option != 536)
            syn_counter++;

        ip_0.nx_ip_tcp_packet_receive = _nx_tcp_packet_receive;

    }

    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);

    _nx_tcp_packet_receive(ip_ptr, packet_ptr);

}

static void    my_tcp_packet_receive_12_24_2(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_TCP_HEADER *header_ptr;
ULONG         option_words;


    header_ptr = (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);

    /* Client receives a SYN,ACK packet.  */
    if((header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT) && (header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT) && !(header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT))
    {
        /* It is a SYN,ACK packet.  */
        option_words =  (header_ptr -> nx_tcp_header_word_3 >> 28) - 5;

        if(option_words == 2)
            syn_ack_counter++;

        ip_1.nx_ip_tcp_packet_receive = _nx_tcp_packet_receive;

    }

    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);

    _nx_tcp_packet_receive(ip_ptr, packet_ptr);

}
#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_12_24_application_define(void *first_unused_memory)
#endif
{

    /* Print out test information banner.  */
    printf("NetX Test:   TCP Spec 12.24 Test.......................................N/A\n"); 

    test_control_return(3);  
}      
#endif