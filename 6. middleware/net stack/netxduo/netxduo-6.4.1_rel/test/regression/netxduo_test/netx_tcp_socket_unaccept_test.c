/* This NetX test concentrates on the TCP Socket unaccept operation.  */

#include   "tx_api.h"
#include   "nx_api.h"
#include   "nx_tcp.h"
extern void    test_control_return(UINT status);

#if !defined(NX_DISABLE_IPV4)

#define     DEMO_STACK_SIZE         2048


/* Define the ThreadX and NetX object control blocks...  */

static TX_THREAD               thread_0;
static TX_THREAD               thread_1;

static NX_PACKET_POOL          pool_0;
static NX_IP                   ip_0;
static NX_IP                   ip_1;
static NX_TCP_SOCKET           client_socket_0; 
static NX_TCP_SOCKET           client_socket_1;
static NX_TCP_SOCKET           server_socket_0; 
static NX_TCP_SOCKET           server_socket_1;      

/* The 2 ports will hashed to the same index. */
#define SERVER_PORT_0          0x00000100
#define SERVER_PORT_1          0x00008100
                                                  

/* Define the counters used in the demo application...  */

static ULONG                   error_counter =     0;


/* Define thread prototypes.  */

static void    thread_0_entry(ULONG thread_input);
static void    thread_1_entry(ULONG thread_input);
extern void    _nx_ram_network_driver_256(struct NX_IP_DRIVER_STRUCT *driver_req);


/* Define what the initial system looks like.  */

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_tcp_socket_unaccept_test_application_define(void *first_unused_memory)
#endif
{

CHAR    *pointer;
UINT    status;

                     
    error_counter =     0;

    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;   

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create the main thread.  */
    tx_thread_create(&thread_1, "thread 1", thread_1_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;


    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status =  nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 256, pointer, 8192);
    pointer = pointer + 8192;

    if (status)
        error_counter++;
                         
    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(1, 2, 3, 4), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver_256,
                    pointer, 2048, 1);
    pointer =  pointer + 2048;

    /* Create another IP instance.  */
    status += nx_ip_create(&ip_1, "NetX IP Instance 1", IP_ADDRESS(1, 2, 3, 5), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver_256,
                    pointer, 2048, 1);
    pointer =  pointer + 2048;

    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status =  nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Enable ARP and supply ARP cache memory for IP Instance 1.  */
    status +=  nx_arp_enable(&ip_1, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Check ARP enable status.  */
    if (status)
        error_counter++;

    /* Enable TCP processing for both IP instances.  */
    status =  nx_tcp_enable(&ip_0);
    status += nx_tcp_enable(&ip_1);

    /* Check TCP enable status.  */
    if (status)
        error_counter++;
}              


/* Define the test threads.  */

static void    thread_0_entry(ULONG thread_input)
{

UINT        status;  
UINT        client_port;

    /* Print out some test information banners.  */
    printf("NetX Test:   TCP Socket Unaccept Test..................................");

    /* Check for earlier error.  */
    if (error_counter)
    {

        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Create a socket.  */
    status =  nx_tcp_socket_create(&ip_0, &client_socket_0, "Client Socket 0", 
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 200,
                                   NX_NULL, NX_NULL);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Get a free port for the client's use.  */
    status =  nx_tcp_free_port_find(&ip_0, 1, &client_port);

    /* Check for error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Bind the socket.  */
    status =  nx_tcp_client_socket_bind(&client_socket_0, client_port, NX_NO_WAIT);

    /* Check for error.  */  
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(1);
    }                       

    /* Attempt to connect the socket.  */
    status =  nx_tcp_client_socket_connect(&client_socket_0, IP_ADDRESS(1, 2, 3, 5), SERVER_PORT_0, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if (status)           
    {

        printf("ERROR!\n");
        test_control_return(1);
    }          
                      
    /* Create a socket.  */
    status =  nx_tcp_socket_create(&ip_0, &client_socket_1, "Client Socket 1", 
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 200,
                                   NX_NULL, NX_NULL);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Get a free port for the client's use.  */
    status =  nx_tcp_free_port_find(&ip_0, 1, &client_port);

    /* Check for error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Bind the socket.  */
    status =  nx_tcp_client_socket_bind(&client_socket_1, client_port, NX_NO_WAIT);

    /* Check for error.  */  
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(1);
    }                       

    /* Attempt to connect the socket.  */
    status =  nx_tcp_client_socket_connect(&client_socket_1, IP_ADDRESS(1, 2, 3, 5), SERVER_PORT_1, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if (status)           
    {

        printf("ERROR!\n");
        test_control_return(1);
    }          
                  
    /* Disconnect the server socket.  */
    status =  nx_tcp_socket_disconnect(&client_socket_1, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if (status)
        error_counter++;     

    /* Check status.  */
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
    

static void    thread_1_entry(ULONG thread_input)
{

UINT            status;
ULONG           actual_status;


    /* Ensure the IP instance has been initialized.  */
    status =  nx_ip_status_check(&ip_1, NX_IP_INITIALIZE_DONE, &actual_status, NX_IP_PERIODIC_RATE);

    /* Check status...  */
    if (status != NX_SUCCESS)
    {

        error_counter++;
    }

    /* Create a socket.  */
    status =  nx_tcp_socket_create(&ip_1, &server_socket_0, "Server Socket 0", 
                                NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 100,
                                NX_NULL, NX_NULL);
                                
    /* Check for error.  */
    if (status)
        error_counter++;      

    /* Setup this thread to listen.  */
    status =  nx_tcp_server_socket_listen(&ip_1, SERVER_PORT_0, &server_socket_0, 5, NX_NULL);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Check the socket state.  */
    if (server_socket_0.nx_tcp_socket_state != NX_TCP_LISTEN_STATE) 
        error_counter++;
                    
    /* Unaccept server socket.  */
    status =  nx_tcp_server_socket_unaccept(&server_socket_0);

    /* Check for error.  */
    if (status)
        error_counter++;
                           
    /* Check the socket state.  */
    if (server_socket_0.nx_tcp_socket_state != NX_TCP_CLOSED) 
        error_counter++;

    /* Setup this thread to listen.  */
    status =  nx_tcp_server_socket_relisten(&ip_1, SERVER_PORT_0, &server_socket_0);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Accept a client socket connection.  */
    status =  nx_tcp_server_socket_accept(&server_socket_0, 5 * NX_IP_PERIODIC_RATE);                                     

    /* Check for error.  */
    if (status)
        error_counter++;
                                      
    /* Create a socket.  */
    status =  nx_tcp_socket_create(&ip_1, &server_socket_1, "Server Socket 1", 
                                NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 100,
                                NX_NULL, NX_NULL);
                                
    /* Check for error.  */
    if (status)
        error_counter++;      

    /* Setup this thread to listen.  */
    status =  nx_tcp_server_socket_listen(&ip_1, SERVER_PORT_1, &server_socket_1, 5, NX_NULL);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Accept a client socket connection.  */
    status =  nx_tcp_server_socket_accept(&server_socket_1, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Disconnect the server socket.  */
    status =  nx_tcp_socket_disconnect(&server_socket_1, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Unaccept server socket.  */
    status =  nx_tcp_server_socket_unaccept(&server_socket_1);

    /* Check for error.  */
    if (status)
        error_counter++;            
}
#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_tcp_socket_unaccept_test_application_define(void *first_unused_memory)
#endif
{

    /* Print out test information banner.  */
    printf("NetX Test:   TCP Socket Unaccept Test..................................N/A\n"); 

    test_control_return(3);  
}      
#endif