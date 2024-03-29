/* Test IPv6 prefix with length not equal 64. */

#include    "tx_api.h"
#include    "nx_api.h"   
#include    "nx_ram_network_driver_test_1500.h"

extern void    test_control_return(UINT status);

#if defined(FEATURE_NX_IPV6) && (NX_IPV6_PREFIX_LIST_TABLE_SIZE == 8)
 
#include    "nx_tcp.h"
#include    "nx_ip.h"
#include    "nx_ipv6.h"  
#include    "nx_icmpv6.h"

#define     DEMO_STACK_SIZE    2048
#define     TEST_INTERFACE     0

/* Define the ThreadX and NetX object control blocks...  */

static TX_THREAD               thread_0;  
static NX_PACKET_POOL          pool_0;
static NX_IP                   ip_0;

/* Define the counters used in the demo application...  */

static ULONG                   error_counter;  

/* Define thread prototypes.  */
static void    thread_0_entry(ULONG thread_input);
extern void    test_control_return(UINT status);       
extern void    _nx_ram_network_driver_1500(struct NX_IP_DRIVER_STRUCT *driver_req);

/* RA packet with prefix length 128. */
static char pkt_prefix_128[] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, 
0xed, 0xeb, 0x40, 0x00, 0x07, 0x0d, 0x00, 0x00, 
0x75, 0x35, 0x00, 0x00, 0x03, 0xed, 0x01, 0x01, 
0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x04, 
0x80, 0xc0, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 
0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 
0x05, 0x01, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* RA packet with prefix length 120. */
static char pkt_prefix_120[] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, 
0xf5, 0xeb, 0x40, 0x00, 0x07, 0x0d, 0x00, 0x00, 
0x75, 0x35, 0x00, 0x00, 0x03, 0xed, 0x01, 0x01, 
0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x04, 
0x78, 0xc0, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 
0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 
0x05, 0x01, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* RA packet with prefix length 100. */
static char pkt_prefix_100[110] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, /* 33...... */
0x00, 0x00, 0xa0, 0xa0, 0x86, 0xdd, 0x60, 0x00, /* ......`. */
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, /* ...8:... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, /* ........ */
0x00, 0xff, 0xfe, 0x00, 0xa0, 0xa0, 0xff, 0x02, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, /* ........ */
0x57, 0x01, 0x40, 0x00, 0x00, 0x14, 0x00, 0x01, /* W.@..... */
0x86, 0xa0, 0x00, 0x00, 0x03, 0xe8, 0x05, 0x01, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x05, 0xdc, 0x03, 0x04, /* ........ */
0x64, 0xc0, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, /* d....... */
0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, /* ......?. */
0x05, 0x01, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00              /* ...... */
};

/* RA packet with prefix length 96. */
static char pkt_prefix_96[110] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, /* 33...... */
0x00, 0x00, 0xa0, 0xa0, 0x86, 0xdd, 0x60, 0x00, /* ......`. */
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, /* ...8:... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, /* ........ */
0x00, 0xff, 0xfe, 0x00, 0xa0, 0xa0, 0xff, 0x02, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, /* ........ */
0x5b, 0x01, 0x40, 0x00, 0x00, 0x14, 0x00, 0x01, /* [.@..... */
0x86, 0xa0, 0x00, 0x00, 0x03, 0xe8, 0x05, 0x01, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x05, 0xdc, 0x03, 0x04, /* ........ */
0x60, 0xc0, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, /* `....... */
0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, /* ......?. */
0x05, 0x01, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00              /* ...... */
};

/* RA packet with prefix length 92. */
static char pkt_prefix_92[110] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, /* 33...... */
0x00, 0x00, 0xa0, 0xa0, 0x86, 0xdd, 0x60, 0x00, /* ......`. */
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, /* ...8:... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, /* ........ */
0x00, 0xff, 0xfe, 0x00, 0xa0, 0xa0, 0xff, 0x02, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, /* ........ */
0x5f, 0x01, 0x40, 0x00, 0x00, 0x14, 0x00, 0x01, /* _.@..... */
0x86, 0xa0, 0x00, 0x00, 0x03, 0xe8, 0x05, 0x01, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x05, 0xdc, 0x03, 0x04, /* ........ */
0x5c, 0xc0, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, /* \....... */
0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, /* ......?. */
0x05, 0x01, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00              /* ...... */
};

/* RA packet with prefix length 88. */
static char pkt_prefix_88[110] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, /* 33...... */
0x00, 0x00, 0xa0, 0xa0, 0x86, 0xdd, 0x60, 0x00, /* ......`. */
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, /* ...8:... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, /* ........ */
0x00, 0xff, 0xfe, 0x00, 0xa0, 0xa0, 0xff, 0x02, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, /* ........ */
0x63, 0x01, 0x40, 0x00, 0x00, 0x14, 0x00, 0x01, /* c.@..... */
0x86, 0xa0, 0x00, 0x00, 0x03, 0xe8, 0x05, 0x01, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x05, 0xdc, 0x03, 0x04, /* ........ */
0x58, 0xc0, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, /* X....... */
0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, /* ......?. */
0x05, 0x01, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00              /* ...... */
};

/* RA packet with prefix length 64. */
static char pkt_prefix_64[] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, 
0x2d, 0xf6, 0x40, 0x00, 0x07, 0x0d, 0x00, 0x00, 
0x75, 0x35, 0x00, 0x00, 0x03, 0xed, 0x01, 0x01, 
0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x04, 
0x40, 0xc0, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 
0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 
0x05, 0x01, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                                               
/* RA packet with prefix length 0. */
static char pkt_prefix_0[] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, 
0x6d, 0xec, 0x40, 0x00, 0x07, 0x0d, 0x00, 0x00, 
0x75, 0x35, 0x00, 0x00, 0x03, 0xed, 0x01, 0x01, 
0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x04, 
0x00, 0xc0, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 
0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 
0x05, 0x01, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* RA packet with prefix length 64. */
static char pkt_prefix_full[] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x38, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, 
0x2d, 0xe6, 0x40, 0x00, 0x07, 0x0d, 0x00, 0x00, 
0x75, 0x35, 0x00, 0x00, 0x03, 0xed, 0x01, 0x01, 
0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x04, 
0x40, 0xc0, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 
0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 
0x05, 0x01, 0xff, 0xff, 0x01, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


/* Define what the initial system looks like.  */

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void           netx_ipv6_prefix_test_application_define(void *first_unused_memory)
#endif
{
CHAR       *pointer;
UINT       status;

    /* Setup the working pointer.  */
    pointer = (CHAR *) first_unused_memory;

    /* Initialize the value.  */
    error_counter = 0;

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
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

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(1,2,3,4), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver_1500,
        pointer, 2048, 1);
    pointer = pointer + 2048;
  
    /* Enable IPv6 */
    status += nxd_ipv6_enable(&ip_0); 

    /* Check IPv6 enable status.  */
    if(status)
        error_counter++;        

    /* Enable IPv6 ICMP  */
    status += nxd_icmp_enable(&ip_0); 

    /* Check IPv6 ICMP enable status.  */
    if(status)
        error_counter++;
}

/* Define the test threads.  */

static void    thread_0_entry(ULONG thread_input)
{
UINT                    status;   
UINT                    address_index;
NXD_ADDRESS             ipv6_address;
ULONG                   prefix_length;
UINT                    interface_index;
NX_PACKET              *packet_ptr;
NX_IPV6_PREFIX_ENTRY   *current_prefix;
UINT                    i;
CHAR                   *pkt_data[] = {pkt_prefix_64, pkt_prefix_128, 
                                      pkt_prefix_0, pkt_prefix_120, 
                                      pkt_prefix_100, pkt_prefix_96,
                                      pkt_prefix_92, pkt_prefix_88, pkt_prefix_full};
UINT                    pkt_len[] = {sizeof(pkt_prefix_64), sizeof(pkt_prefix_128), 
                                     sizeof(pkt_prefix_0), sizeof(pkt_prefix_120), 
                                     sizeof(pkt_prefix_100), sizeof(pkt_prefix_96),
                                     sizeof(pkt_prefix_92), sizeof(pkt_prefix_88), sizeof(pkt_prefix_full)};
ULONG                   prefix_len[] = {128, 120, 100, 96, 92, 88, 64, 0};

    /* Print out test information banner.  */
    printf("NetX Test:   IPv6 Prefix Test.........................................."); 

    /* Check for earlier error.  */
    if(error_counter)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }                     

    /* Set the linklocal address.  */
    status = nxd_ipv6_address_set(&ip_0, 0, NX_NULL, 10, &address_index); 

    /* Check the status.  */
    if(status)
        error_counter++;  

    /* Sleep 5 seconds for linklocal address DAD.  */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

    /* Get the linklocal address.  */
    status = nxd_ipv6_address_get(&ip_0, address_index, &ipv6_address, &prefix_length, &interface_index);
                       
    /* Check the status.  */
    if((status) || (prefix_length != 10) || (interface_index != 0))
        error_counter++;  

    for (i = 0; i < sizeof(pkt_data) / sizeof(CHAR *); i++)
    {

        /* Inject RA with prefix length 64. */
        status = nx_packet_allocate(&pool_0, &packet_ptr, NX_PHYSICAL_HEADER, NX_WAIT_FOREVER);

        /* Check status */
        if(status)
            error_counter ++;

        /* Fill in the packet with data. Skip the MAC header.  */
        memcpy(packet_ptr -> nx_packet_prepend_ptr, &pkt_data[i][14], pkt_len[i] - 14);
        packet_ptr -> nx_packet_length = pkt_len[i] - 14;
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

        /* Directly receive the RA packet.  */
        _nx_ip_packet_deferred_receive(&ip_0, packet_ptr);     

        if (i == 0)
        {

            /* Sleep one second and let prefix 64 bits timeout first. */
            tx_thread_sleep(NX_IP_PERIODIC_RATE);
        }
    }

    /* Sleep 5 seconds for linklocal address DAD.  */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

    /* Get the first global address.  */
    status = nxd_ipv6_address_get(&ip_0, address_index + 1, &ipv6_address, &prefix_length, &interface_index);
                       
    /* Check the status.  */
    if((status) || (prefix_length != 64) || (interface_index != 0))
        error_counter++;  

    /* Get the second global address.  */
    status = nxd_ipv6_address_get(&ip_0, address_index + 2, &ipv6_address, &prefix_length, &interface_index);

    /* No IPv6 address should be formed since prefix length is not 64. */
    if (status != NX_NO_INTERFACE_ADDRESS)
    {
        error_counter++;
    }

    /* Check prefix list. It should be linked from largest to smallest. */
    current_prefix = ip_0.nx_ipv6_prefix_list_ptr;

    for (i = 0; i < sizeof(prefix_len) / sizeof(ULONG); i++)
    {

        /* Check length of prefix. */
        if (current_prefix)
        {
            if (current_prefix -> nx_ipv6_prefix_entry_prefix_length != prefix_len[i])
            {
                error_counter++;
                break;
            }

            /* Move to next prefix entry. */
            current_prefix = current_prefix -> nx_ipv6_prefix_entry_next;
        }
        else
        {
            error_counter++;
            break;
        }
    }

    /* No more prefix. */
    if (current_prefix)
    {
        error_counter++;
    }

    /* Sleep 30 seconds for prefix timeout. */
    tx_thread_sleep(30 * NX_IP_PERIODIC_RATE);

    /* All prefixes are timeout. */
    if (ip_0.nx_ipv6_prefix_list_ptr != NX_NULL)
    {
        error_counter++;
    }

    /* Check the error.  */
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

#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void           netx_ipv6_prefix_test_application_define(void *first_unused_memory)
#endif
{

    /* Print out test information banner.  */
    printf("NetX Test:   IPv6 Prefix Test..........................................N/A\n");   
    test_control_return(3);

}
#endif /* FEATURE_NX_IPV6 */
