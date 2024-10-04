/* serverpacket.c
 *
 * Constuct and send DHCP server packets
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
//#define DEBUG
#include <lwip/inet.h>
#include <string.h>

#include "packet.h"
#include "dhcp_debug.h"
#include "dhcpd.h"
#include "leases.h"
#include "options.h"
#include "api.h"
#include "debug.h"
#include "lwip/dns.h"

extern 	struct netif *p_netif;
/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
	LOG(LOG_INFO, "Forwarding packet to relay");

	return kernel_packet(payload, p_netif->ip_addr.addr, SERVER_PORT,
			payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	unsigned char *chaddr;
	u_int32_t ciaddr;

	#if  1
	if (force_broadcast) {
		LOG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else if (payload->ciaddr) {
		LOG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		LOG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		LOG(LOG_INFO, "unicasting packet to client yiaddr, %x, %x:%x:%x:%x:%x:%x", 
			payload->yiaddr,
			payload->chaddr[0],payload->chaddr[1],payload->chaddr[2],
			payload->chaddr[3],payload->chaddr[4],payload->chaddr[5]);
		ciaddr = payload->yiaddr;
		chaddr = payload->chaddr;

		//LOG(LOG_INFO, "%x, %x", (uint32_t)sockets[14].conn->pcb.udp->local_ip.addr,
		//	(uint32_t)sockets[14].conn->pcb.udp->remote_ip.addr);
	}
	#else
	
	if (force_broadcast
	 || (payload->flags & htons(BROADCAST_FLAG))
	 || payload->ciaddr == 0
	) {
		V(LOG_INFO, "Broadcasting packet to client");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		LOG(LOG_INFO, "Unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;
	}
	#endif
	
	return raw_packet(payload, p_netif->ip_addr.addr, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr, server_config.ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload);
	else 
		ret = send_packet_to_client(payload, force_broadcast);
	return ret;
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	init_header(packet, type);
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_SERVER_ID, p_netif->ip_addr.addr);
	add_simple_option(packet->options, DHCP_SUBNET, p_netif->netmask.addr);
	add_simple_option(packet->options, DHCP_ROUTER, p_netif->gw.addr);
	add_simple_option(packet->options, DHCP_DNS_SERVER, dns_getserver(0).addr);
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
	packet->siaddr = server_config.siaddr;
	if (server_config.sname)
		strncpy((char*)packet->sname, (char*)server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy((char*)packet->file, (char*)server_config.boot_file, sizeof(packet->file) - 1);
}
	

/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage *packet;
	struct dhcpOfferedAddr *lease = NULL;
	u_int32_t req_align, lease_time_align = server_config.lease;
	unsigned char *req, *lease_time;
	struct option_set *curr;
	struct in_addr addr;
	int ret;

	p_dbg_enter;
	addr = addr;
	packet = (struct dhcpMessage*)mem_calloc(sizeof(struct dhcpMessage), 1);
	if(!packet)
		return -1;

	init_packet(packet, oldpacket, DHCPOFFER);
	
	/* ADDME: if static, short circuit */
	/* the client is in our lease/offered table */
	if ((lease = find_lease_by_chaddr(oldpacket->chaddr))!=0) {
		if (!lease_expired(lease)) 
			lease_time_align = lease->expires - time(0);
		packet->yiaddr = lease->yiaddr;
		p_dbg("1");
	/* Or the client has a requested ip */
	} else if (((req = get_option(oldpacket, DHCP_REQUESTED_IP))!=0) &&

		   /* Don't look here (ugly hackish thing to do) */
		   memcpy(&req_align, req, 4) &&
		   /* and the ip is in the lease range */
		   ntohl(req_align) >= ntohl(server_config.start) &&
		   ntohl(req_align) <= ntohl(server_config.end) &&
		   
		   /* and its not already taken/offered */ /* ADDME: check that its not a static lease */
		   ((((lease = find_lease_by_yiaddr(req_align)) == 0) ||
		   
		   /* or its taken, but expired */ /* ADDME: or maybe in here */
		   lease_expired(lease)))) {
				packet->yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */
		   p_dbg("2");
	/* otherwise, find a free IP */ /*ADDME: is it a static lease? */
	} else {

		p_dbg("3");
		packet->yiaddr = find_address(0);
		
		/* try for an expired lease */
		if (!packet->yiaddr) packet->yiaddr = find_address(1);
	}
	
	if(!packet->yiaddr) {
		LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
		mem_free(packet);
		return -1;
	}
	
	if (!add_lease(packet->chaddr, packet->yiaddr, server_config.offer_time)) {
		LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
		mem_free(packet);
		return -1;
	}		

	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))!=0) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
	}

	/* Make sure we aren't just using the lease time from the previous offer */
	if (lease_time_align < server_config.min_lease) 
		lease_time_align = server_config.lease;
	/* ADDME: end of short circuit */		
	add_simple_option(packet->options, DHCP_LEASE_TIME, htonl(lease_time_align));

	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet->options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(packet);
	
	addr.s_addr = packet->yiaddr;
	LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
	ret = send_packet(packet, 0);
	mem_free(packet);
	return ret;
}


int sendNAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage *packet;
	int ret;
	packet = (struct dhcpMessage*)mem_calloc(sizeof(struct dhcpMessage), 1);
	if(!packet)
		return -1;

	init_packet(packet, oldpacket, DHCPNAK);
	
	LOG(LOG_INFO, "sending NAK");
	ret = send_packet(packet, 1);
	mem_free(packet);
	return ret;
}


int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr)
{
	struct dhcpMessage *packet;
	struct option_set *curr;
	unsigned char *lease_time;
	u_int32_t lease_time_align = server_config.lease;
	struct in_addr addr;

	p_dbg_enter;
	addr = addr;
	packet = (struct dhcpMessage*)mem_calloc(sizeof(struct dhcpMessage), 1);
	if(!packet)
		return -1;

	init_packet(packet, oldpacket, DHCPACK);
	packet->yiaddr = yiaddr;
	
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))!=0) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
		else if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}
	
	add_simple_option(packet->options, DHCP_LEASE_TIME, htonl(lease_time_align));
	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet->options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(packet);

	addr.s_addr = packet->yiaddr;
	LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));

	if (send_packet(packet, 0) < 0) {
		mem_free(packet);
		return -1;
	}
	add_lease(packet->chaddr, packet->yiaddr, lease_time_align);
	mem_free(packet);
	return 0;
}


int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage *packet;
	struct option_set *curr;
	int ret;
	packet = (struct dhcpMessage*)mem_calloc(sizeof(struct dhcpMessage), 1);
	if(!packet)
		return -1;

	init_packet(packet, oldpacket, DHCPACK);
	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet->options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(packet);

	ret = send_packet(packet, 0);

	mem_free(packet);

	return ret;
}



