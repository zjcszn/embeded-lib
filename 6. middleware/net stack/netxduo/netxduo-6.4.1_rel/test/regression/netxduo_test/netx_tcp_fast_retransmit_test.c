/* This NetX test concentrates on fast retransmit.  */

#include   "tx_api.h"
#include   "nx_api.h"
#include   "nx_tcp.h"
extern void    test_control_return(UINT status);

#if !defined(NX_DISABLE_IPV4)

#define     DEMO_STACK_SIZE    2048

#define MSG "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

/* Define the ThreadX and NetX object control blocks...  */

static TX_THREAD               thread_0;
static TX_THREAD               thread_1;

static NX_PACKET_POOL          pool_0;
static NX_IP                   ip_0;
static NX_IP                   ip_1;
static NX_TCP_SOCKET           client_socket;
static NX_TCP_SOCKET           server_socket;

/* Define the counters used in the demo application...  */

static ULONG                   error_counter;

extern ULONG                   packet_gather;

/* Define thread prototypes.  */

static void    thread_0_entry(ULONG thread_input);
static void    thread_1_entry(ULONG thread_input);
static void    thread_1_connect_received(NX_TCP_SOCKET *server_socket, UINT port);
static void    thread_1_disconnect_received(NX_TCP_SOCKET *server_socket);
extern void    _nx_ram_network_driver_256(struct NX_IP_DRIVER_STRUCT *driver_req);
static void    my_tcp_packet_receive_f_r(NX_IP *ip_ptr, NX_PACKET *packet_ptr);

/* Define what the initial system looks like.  */

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void           netx_tcp_fast_retransmit_test_application_define(void *first_unused_memory)
#endif
{
    CHAR           *pointer;
    UINT           status;

    /* Setup the working pointer.  */
    pointer = (CHAR *) first_unused_memory;

    error_counter = 0;

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer = pointer + DEMO_STACK_SIZE;

    /* Create the main thread.  */
    tx_thread_create(&thread_1, "thread 1", thread_1_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer = pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 512, pointer, 8192);
    pointer = pointer + 8192;

    if(status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(1, 2, 3, 4), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver_256,
        pointer, 2048, 1);
    pointer = pointer + 2048;

    /* Create another IP instance.  */
    status += nx_ip_create(&ip_1, "NetX IP Instance 1", IP_ADDRESS(1, 2, 3, 5), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver_256,
        pointer, 2048, 1);
    pointer = pointer + 2048;

    if(status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Enable ARP and supply ARP cache memory for IP Instance 1.  */
    status += nx_arp_enable(&ip_1, (void *) pointer, 1024);
    pointer = pointer + 1024;

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

static void    thread_0_entry(ULONG thread_input)
{
    UINT           status;
    NX_PACKET      *my_packet1;
    NX_PACKET      *my_packet2;
    NX_PACKET      *my_packet3;
    NX_PACKET      *my_packet4;
    char           *msg = MSG;

    /* Print out test information banner.  */
    printf("NetX Test:   TCP Fast Retransmit Test..................................");

    /* Check for earlier error.  */
    if(error_counter)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Create a socket.  */
    status = nx_tcp_socket_create(&ip_0, &client_socket, "Client Socket", 
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 300,
                                  NX_NULL, NX_NULL);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Bind the socket.  */
    status = nx_tcp_client_socket_bind(&client_socket, 12, NX_WAIT_FOREVER);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Attempt to connect the socket.  */
    tx_thread_relinquish();

    status = nx_tcp_client_socket_connect(&client_socket, IP_ADDRESS(1, 2, 3, 5), 12, 5 * NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Create 4 packets   */
    status = nx_packet_allocate(&pool_0, &my_packet1, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += nx_packet_allocate(&pool_0, &my_packet2, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += nx_packet_allocate(&pool_0, &my_packet3, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += nx_packet_allocate(&pool_0, &my_packet4, NX_TCP_PACKET, NX_WAIT_FOREVER);

    if(status)
        error_counter++;

    /* Fill in the packet with data.     */

    memcpy(my_packet1 -> nx_packet_prepend_ptr, &msg[0], 20);
    my_packet1 -> nx_packet_length = 20;
    my_packet1 -> nx_packet_append_ptr = my_packet1 -> nx_packet_prepend_ptr + 20;

    memcpy(my_packet2 -> nx_packet_prepend_ptr, &msg[20], 20);
    my_packet2 -> nx_packet_length = 20;
    my_packet2 -> nx_packet_append_ptr = my_packet2 -> nx_packet_prepend_ptr + 20;

    memcpy(my_packet3 -> nx_packet_prepend_ptr, &msg[40], 20);
    my_packet3 -> nx_packet_length = 20;
    my_packet3 -> nx_packet_append_ptr = my_packet3 -> nx_packet_prepend_ptr + 20;

    memcpy(my_packet4 -> nx_packet_prepend_ptr, &msg[60], 20);
    my_packet4 -> nx_packet_length = 20;
    my_packet4 -> nx_packet_append_ptr = my_packet4 -> nx_packet_prepend_ptr + 20;

    ip_1.nx_ip_tcp_packet_receive = my_tcp_packet_receive_f_r;

    /* Send the 1st one   */
    status = nx_tcp_socket_send(&client_socket, my_packet1, NX_IP_PERIODIC_RATE);

    /* Send the 2nd one   */
    status += nx_tcp_socket_send(&client_socket, my_packet2, NX_IP_PERIODIC_RATE);

    /* Send the 3rd one   */
    status += nx_tcp_socket_send(&client_socket, my_packet3, NX_IP_PERIODIC_RATE);

    /* Send the 4th one   */
    status += nx_tcp_socket_send(&client_socket, my_packet4, NX_IP_PERIODIC_RATE);

    if(status)
    {
        error_counter++;
    }

    /* Disable timeout retransmit.  */
    client_socket.nx_tcp_socket_timeout = 60 * 60 * _nx_tcp_fast_timer_rate;

    /* Disconnect this socket.  */
    status = nx_tcp_socket_disconnect(&client_socket, NX_WAIT_FOREVER);

    /* Check for error.  */
    if(status)
    {
        error_counter++;
    }

    /* Unbind the socket.  */
    status = nx_tcp_client_socket_unbind(&client_socket);

    /* Check for error.  */
    if(status)
    {
        error_counter++;
    }

    /* Delete the socket.  */
    status = nx_tcp_socket_delete(&client_socket);

    /* Check for error.  */
    if(status)
    {
        error_counter++;
    }
}

static char    rcv_buffer[200];
static void    thread_1_entry(ULONG thread_input)
{
    UINT           status;
    NX_PACKET      *packet_ptr;
    ULONG          actual_status;
    ULONG          recv_length = 0;

    /* Ensure the IP instance has been initialized.  */
    status = nx_ip_status_check(&ip_1, NX_IP_INITIALIZE_DONE, &actual_status, NX_IP_PERIODIC_RATE);

    /* Check status...  */
    if(status != NX_SUCCESS)
    {
        error_counter++;
        test_control_return(1);
    }

    /* Create a socket.  */
    status = nx_tcp_socket_create(&ip_1, &server_socket, "Server Socket", 
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 200,
                                  NX_NULL, thread_1_disconnect_received);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Setup this thread to listen.  */
    status = nx_tcp_server_socket_listen(&ip_1, 12, &server_socket, 5, thread_1_connect_received);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Accept a client socket connection.  */
    status = nx_tcp_server_socket_accept(&server_socket, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if(status)
        error_counter++;

    /* Receive a TCP message from the socket.  */
    while (nx_tcp_socket_receive(&server_socket, &packet_ptr, 5 * NX_IP_PERIODIC_RATE) == NX_SUCCESS)
    {

        if(packet_ptr -> nx_packet_length == 0)
            error_counter++;

        memcpy(&rcv_buffer[recv_length], packet_ptr -> nx_packet_prepend_ptr, packet_ptr -> nx_packet_length);
        recv_length += packet_ptr -> nx_packet_length;

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }

    if(recv_length != 80)
        error_counter++;

    if(memcmp(rcv_buffer, (void*)MSG, recv_length))
        error_counter++;

    /* Disconnect the server socket.  */
    status = nx_tcp_socket_disconnect(&server_socket, NX_WAIT_FOREVER);

    /* Check for error.  */
    if(status)
        error_counter++;

    status = nx_tcp_server_socket_unaccept(&server_socket);

    /* Check for error.  */
    if(status)
        error_counter++;

    tx_thread_relinquish();   

    /* Determine if the test was successful.  */
    if(error_counter)
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

static void    thread_1_connect_received(NX_TCP_SOCKET *socket_ptr, UINT port)
{

    /* Check for the proper socket and port.  */
    if((socket_ptr != &server_socket) || (port != 12))
        error_counter++;
}

static void    thread_1_disconnect_received(NX_TCP_SOCKET *socket)
{

    /* Check for proper disconnected socket.  */
    if(socket != &server_socket)
        error_counter++;
}

static void    my_tcp_packet_receive_f_r(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{
    nx_packet_release(packet_ptr);
    ip_1.nx_ip_tcp_packet_receive = _nx_tcp_packet_receive;
}
#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_tcp_fast_retransmit_test_application_define(void *first_unused_memory)
#endif
{

    /* Print out test information banner.  */
    printf("NetX Test:   TCP Fast Retransmit Test..................................N/A\n"); 

    test_control_return(3);  
}      
#endif