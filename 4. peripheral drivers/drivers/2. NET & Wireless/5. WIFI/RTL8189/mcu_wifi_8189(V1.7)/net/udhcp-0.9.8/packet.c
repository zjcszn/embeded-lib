#define DEBUG
#include "debug.h"

#include <string.h>
#include "defs.h"
#include "packet.h"
#include "dhcp_debug.h"
#include "dhcpd.h"
#include "options.h"
#include "lwip/sockets.h"
#include "api.h"
#include "netif/etharp.h"

extern mutex_t socket_mutex;
extern 	struct netif *p_netif;

void init_header(struct dhcpMessage *packet, char type)
{
	memset(packet, 0, sizeof(struct dhcpMessage));
	switch (type) {
	case DHCPDISCOVER:
	case DHCPREQUEST:
	case DHCPRELEASE:
	case DHCPINFORM:
		packet->op = BOOTREQUEST;
		break;
	case DHCPOFFER:
	case DHCPACK:
	case DHCPNAK:
		packet->op = BOOTREPLY;
	}
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
}

time_t time(time_t* t)
{
	if(t)
		*t = jiffies;
	return jiffies;
}

u_int16_t checksum(uint16_t *addr, int nleft)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	register int32_t sum = 0;

	#if 0
	u_int16_t *source = (u_int16_t *) addr;

	while (count > 1)  {
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) {
		/* Make sure that the left-over byte is added correctly both
		 * with little and big endian hosts */
		u_int16_t tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	#endif
	while (nleft > 1) {
		sum += *addr++;
		nleft -= 2;
	}

	/* Mop up an odd byte, if necessary */
	if (nleft == 1) {
		if (/*BB_LITTLE_ENDIAN*/1)
			sum += *(uint8_t*)addr;
		else
			sum += *(uint8_t*)addr << 8;
	}

	/* Add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */
	return ~sum;
}

struct sockaddr_ll {
	unsigned short	sll_family;
	__be16		sll_protocol;
	int		sll_ifindex;
	unsigned short	sll_hatype;
	unsigned char	sll_pkttype;
	unsigned char	sll_halen;
	unsigned char	sll_addr[8];
};

#define IPVERSION	4
#define MAXTTL		255
#define IPDEFTTL	64

#define CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS 80

int udhcp_end_option(uint8_t *optionptr)
{
	int i = 0;

	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] != DHCP_PADDING)
			i += optionptr[i + OPT_LEN] + OPT_DATA-1;
		i++;
	}
	return i;
}

enum {
	IP_UDP_DHCP_SIZE = sizeof(struct udp_dhcp_packet) - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
	UDP_DHCP_SIZE    = sizeof(struct udp_dhcp_packet) - sizeof(struct iphdr) - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
	DHCP_SIZE        = sizeof(struct dhcpMessage) - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
};
/* Constuct a ip/udp header for a packet, and specify the source and dest hardware address */
int raw_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port, unsigned char *dest_arp, int ifindex)
{
	int len, padding;
	int result = -1;
	struct pbuf *p = 0;
	struct udp_dhcp_packet *packet = 0;
	struct eth_hdr eth;

	len = sizeof(struct ethhdr) + IP_UDP_DHCP_SIZE;

	packet = (struct udp_dhcp_packet*)mem_calloc(sizeof(struct udp_dhcp_packet), 1);
	
	if(!packet)
		return -1;

	memcpy(&(packet->data), payload, sizeof(struct dhcpMessage));

	padding = 308 - 1 - udhcp_end_option(packet->data.options);
	if (padding > DHCP_SIZE - 300)
		padding = DHCP_SIZE - 300;

	p_dbg("pading:%d", padding);

	len -= padding;

	memcpy(eth.dest.addr, dest_arp, 6);
	memcpy(eth.src.addr, p_netif->hwaddr, 6);
	eth.type = htons(ETH_P_IP);

	packet->ip.protocol = IPPROTO_UDP;
	packet->ip.saddr = source_ip;
	packet->ip.daddr = dest_ip;
	packet->udp.src = htons(source_port);
	packet->udp.dest = htons(dest_port);
	//packet->udp.len = htons(sizeof(packet->udp) + sizeof(struct dhcpMessage)); /* cheat on the psuedo-header */
	packet->udp.len = htons(UDP_DHCP_SIZE - padding);
	packet->ip.tot_len= packet->udp.len;

	packet->udp.chksum = checksum((uint16_t *)packet, IP_UDP_DHCP_SIZE - padding);

	packet->ip.tot_len= htons(IP_UDP_DHCP_SIZE - padding);
	packet->ip.ihl= sizeof(packet->ip) >> 2;
	packet->ip.version = IPVERSION;
	packet->ip.ttl = IPDEFTTL;

	//add by lck
	//packet->ip.frag_off = 0X40;
	//packet->ip.id = htons(ip_id);
	//ip_id++;
	
	packet->ip.check = checksum((uint16_t *)&(packet->ip), sizeof(packet->ip));

	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

	if(!p || p->len < len)
		goto err;

  	memcpy((char*)p->payload, (char*)&eth, sizeof(struct eth_hdr));
	memcpy((char*)p->payload + sizeof(struct eth_hdr), 
		(char*)packet, 
		len - sizeof(struct eth_hdr));

	p_netif->linkoutput(p_netif, p);

	result = 0;

err:

	if(packet)
		mem_free(packet);

	if(p)
		pbuf_free(p);
	
	return result;
}


/* Let the kernel do all the work for packet generation */
int kernel_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port)
{
	int n = 1;
	int fd, result;
	struct sockaddr_in client;

	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		return -1;
	}
	
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1){
		return -1;

	}
	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(source_port);
	client.sin_addr.s_addr = source_ip;

	if (bind(fd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1){
		return -1;
	}
	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(dest_port);
	client.sin_addr.s_addr = dest_ip; 

	if (connect(fd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1){
		return -1;
	}
	result = write(fd, payload, sizeof(struct dhcpMessage));
	close(fd);
	return result;
}	

