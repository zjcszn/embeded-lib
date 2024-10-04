#define DEBUG
#include <string.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "lwip/sockets.h"
#include "rt_config.h"
#include "user_interface.h"
#include "netif/etharp.h"

extern mutex_t *socket_mutex;

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

	p_dbg("get sum, size:%d", nleft);
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
	int rem_len, len, padding;
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

	mutex_lock(socket_mutex);

	memcpy(eth.dest.addr, dest_arp, 6);
	memcpy(eth.src.addr, sys_cfg.netif->hwaddr, 6);
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

	p_dbg("chksum:%x", packet->udp.chksum);

	packet->ip.tot_len= htons(IP_UDP_DHCP_SIZE - padding);
	packet->ip.ihl= sizeof(packet->ip) >> 2;
	packet->ip.version = IPVERSION;
	packet->ip.ttl = IPDEFTTL;

	//add by lck
	//packet->ip.frag_off = 0X40;
	//packet->ip.id = htons(ip_id);
	//ip_id++;
	
	packet->ip.check = checksum((uint16_t *)&(packet->ip), sizeof(packet->ip));

	p_dbg("check:%x", packet->ip.check);

	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

	if(!p || p->len < len)
		goto err;

  	memcpy((char*)p->payload, (char*)&eth, sizeof(struct eth_hdr));
	memcpy((char*)p->payload + sizeof(struct eth_hdr), 
		(char*)packet, 
		len - sizeof(struct eth_hdr));

	sys_cfg.netif->linkoutput(sys_cfg.netif, p);

	result = 0;
#if 0
	int fd, n, err;
	int result;
	struct pbuf *p, *q;
	struct sockaddr_in cli;
	struct sockaddr_ll dest;
	struct udp_dhcp_packet packet;

	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP, -1)) < 0) {
		DEBUG(LOG_ERR, "socket call failed: %s", (errno));
		return -1;
	}
	
	memset(&dest, 0, sizeof(dest));
	memset(&packet, 0, sizeof(packet));

	
	dest.sll_family = PF_INET;
	dest.sll_protocol = htons(ETH_P_IP);
	dest.sll_ifindex = ifindex;
	dest.sll_halen = 6;
	memcpy(dest.sll_addr, dest_arp, 6);
	if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0) {
		DEBUG(LOG_ERR, "bind call failed: %s", (errno));
		close(fd);
		return -1;
	}
	
	mutex_lock(socket_mutex);

	err = bind(fd, (struct sockaddr*)&cli, sizeof(struct sockaddr_in));
	if(err == -1)
	{
		mutex_unlock(socket_mutex);
		p_err("4");
		return -1;
	}

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source_ip;
	packet.ip.daddr = dest_ip;
	packet.udp.src = htons(source_port);
	packet.udp.dest = htons(dest_port);
	packet.udp.len = htons(sizeof(packet.udp) + sizeof(struct dhcpMessage)); /* cheat on the psuedo-header */
	packet.ip.tot_len= packet.udp.len;
	memcpy(&(packet.data), payload, sizeof(struct dhcpMessage));
	packet.udp.chksum = checksum(&packet, sizeof(struct udp_dhcp_packet));
	
	packet.ip.tot_len= htons(sizeof(struct udp_dhcp_packet));
	packet.ip.ihl= sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = checksum(&(packet.ip), sizeof(packet.ip));

	result = sendto(fd, &packet, sizeof(struct udp_dhcp_packet), 0, (struct sockaddr *) &dest, sizeof(dest));
	if (result <= 0) {
		DEBUG(LOG_ERR, "write on socket failed: %s", errno);
	}
	close(fd);
	mutex_unlock(socket_mutex);
#endif
err:
	mutex_unlock(socket_mutex);

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

	mutex_lock(socket_mutex);
	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP, -1)) < 0)
	{
		mutex_unlock(socket_mutex);
		return -1;
	}
	
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1){
		mutex_unlock(socket_mutex);
		return -1;

	}
	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(source_port);
	client.sin_addr.s_addr = source_ip;

	if (bind(fd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1){
		mutex_unlock(socket_mutex);
		return -1;
	}
	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(dest_port);
	client.sin_addr.s_addr = dest_ip; 

	if (connect(fd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1){
		mutex_unlock(socket_mutex);
		return -1;
	}
	result = write(fd, payload, sizeof(struct dhcpMessage));
	close(fd);
	mutex_unlock(socket_mutex);
	return result;
}	

