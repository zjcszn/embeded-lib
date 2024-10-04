/* dhcpd.c
 *
 * udhcp Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
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
#define DEBUG
#include "debug.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "dhcp_debug.h"
#include "dhcpd.h"
#include "socket.h"
#include "options.h"
#include "leases.h"

#include "api.h"
#include "packet.h"
#include "serverpacket.h"
#include "lwip/dhcp.h"
#include "lwip/sockets.h"

#include "leases.h"
/* globals */
struct dhcpOfferedAddr *leases = 0;
struct server_config_t server_config;
struct dhcpMessage *dhcp_packet = 0;

extern 	mutex_t socket_mutex;
extern 	struct netif *p_netif;

int init_udhcpd()
{
	int  socket_s, err, n = 1;
	struct sockaddr_in serv;
	p_dbg_enter;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	memset(&server_config, 0, sizeof(struct server_config_t));
	memcpy(server_config.arp, p_netif->hwaddr, 6);
	
	dhcp_packet = 0;
	
	server_config.auto_time = FALSE;
	server_config.conflict_time = 3600;
	server_config.decline_time = 3600;
	server_config.start = inet_addr("192.168.1.100");
	server_config.end = inet_addr("192.168.1.254");
	server_config.lease = LEASE_TIME;
	server_config.max_leases = 10;
	server_config.min_lease = 20;
	server_config.offer_time = 60;
	server_config.siaddr = inet_addr("0.0.0.0");
	server_config.sname = "wsum";

	serv.sin_family = AF_INET;
	serv.sin_port = htons(DHCP_SERVER_PORT);	
	serv.sin_addr.s_addr = htons(INADDR_ANY);
	
	mutex_lock(socket_mutex);		
	socket_s = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_s == -1)
	{
		mutex_unlock(socket_mutex);
		p_err("1");
		return -1;
	}
/*
	if (setsockopt(socket_s, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1) {
		mutex_unlock(socket_mutex);
		p_err("2");
		return -1;
	}*/
	if (setsockopt(socket_s, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n)) == -1) {
		mutex_unlock(socket_mutex);
		p_err("3");
		return -1;
	}
	
	err = bind(socket_s, (struct sockaddr*)&serv, sizeof(struct sockaddr_in));
	if(err == -1)
	{
		mutex_unlock(socket_mutex);
		p_err("4");
		return -1;
	}

	server_config.inited = TRUE;
	mutex_unlock(socket_mutex);
	return 0;
}

void enable_dhcp_server()
{
	p_dbg_enter;
	if(server_config.inited)
		server_config.open = TRUE;
}


//借用socket_mutex来保护dhcp_packet和leases
void disable_dhcp_server()
{
	p_dbg_enter;
	mutex_lock(socket_mutex);
	server_config.open = FALSE;
	if(dhcp_packet)
		mem_free(dhcp_packet);
	if(leases)
		mem_free(leases);

	dhcp_packet = 0;
	leases = 0;
	mutex_unlock(socket_mutex);
}


const char broken_vendors[][8] = {
		"MSFT 98",
		""
	};


void release_dhcp_packet()
{
	if(dhcp_packet)
	{
		mem_free(dhcp_packet);
		dhcp_packet = 0;
	}
}

int handle_dhcp_rcv(uint8_t *buff, int size)
{
	int i;

	struct dhcpOfferedAddr *lease;
	unsigned char *server_id, *requested;
	u_int32_t server_id_align = 0, requested_align = 0;
	unsigned char *state;
	
	char unsigned *vendor;
	
	p_dbg_enter;
	if(!server_config.open)
		return -1;
	
	if(!dhcp_packet)
		dhcp_packet = (struct dhcpMessage*)mem_malloc(sizeof(struct dhcpMessage));

	if(!dhcp_packet)
		return -1;

	if(!leases)
	{
		leases = (struct dhcpOfferedAddr *)mem_malloc(sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
		if(!leases)
		{
			release_dhcp_packet();
			return -1;
		}
		memset(leases, 0, sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	}

	mutex_lock(socket_mutex);

	if(size > sizeof(struct dhcpMessage))
	{
		size = sizeof(struct dhcpMessage);
	}

	memcpy(dhcp_packet, buff, size);
		
	if (ntohl(dhcp_packet->cookie) != DHCP_MAGIC) {
		LOG(LOG_ERR, "received bogus message, ignoring:%x", dhcp_packet->cookie);
		mutex_unlock(socket_mutex);
		release_dhcp_packet();
		return -2;
	}
	DEBUG(LOG_INFO, "Received a packet");
	
	if ((dhcp_packet->op == BOOTREQUEST) && ((vendor = get_option(dhcp_packet, DHCP_VENDOR)) != 0)) {
		for (i = 0; broken_vendors[i][0]; i++) {
			if (vendor[OPT_LEN - 2] == (unsigned char) strlen(broken_vendors[i]) &&
			    !strncmp((const char *)vendor, broken_vendors[i], vendor[OPT_LEN - 2])) {
			    	DEBUG(LOG_INFO, "broken client (%s), forcing broadcast",
			    		broken_vendors[i]);
			    	dhcp_packet->flags |= htons(BROADCAST_FLAG);
			}
		}
	}

	if ((state = get_option(dhcp_packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_ERR, "couldn't get option from packet, ignoring");
			mutex_unlock(socket_mutex);
			release_dhcp_packet();
			return -3;
	}
	
	/* ADDME: look for a static lease */
	lease = find_lease_by_chaddr(dhcp_packet->chaddr);
	switch (state[0]) {
	case DHCPDISCOVER:
		DEBUG(LOG_INFO,"received DISCOVER");
		
		if (sendOffer(dhcp_packet) < 0) {
			LOG(LOG_ERR, "send OFFER failed");
		}
		break;			
	case DHCPREQUEST:
		DEBUG(LOG_INFO, "received REQUEST");

		requested = get_option(dhcp_packet, DHCP_REQUESTED_IP);
		server_id = get_option(dhcp_packet, DHCP_SERVER_ID);

		if (requested) memcpy(&requested_align, requested, 4);
		if (server_id) memcpy(&server_id_align, server_id, 4);

		p_dbg("request:%x, server_id:%x", requested_align, server_id_align);

		if (lease) { /*ADDME: or static lease */
			if (server_id) {
				/* SELECTING State */
				DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
				if (server_id_align == p_netif->ip_addr.addr && requested && 
						requested_align == lease->yiaddr) {
					sendACK(dhcp_packet, lease->yiaddr);
				}
			} else {
				if (requested) {
					/* INIT-REBOOT State */
					p_dbg("2");
					if (lease->yiaddr == requested_align)
						sendACK(dhcp_packet, lease->yiaddr);
					else 
						sendNAK(dhcp_packet);
				} else {
					/* RENEWING or REBINDING State */
					p_dbg("3");
					if (lease->yiaddr == dhcp_packet->ciaddr)
						sendACK(dhcp_packet, lease->yiaddr);
					else {
						/* don't know what to do!!!! */
						sendNAK(dhcp_packet);
					}
				}						
			}
		
		/* what to do if we have no record of the client */
		} else if (server_id) {
			/* SELECTING State */
			p_dbg("4");
		} else if (requested) {
			/* INIT-REBOOT State */
			p_dbg("5");
			if ((lease = find_lease_by_yiaddr(requested_align)) != 0) {
				
				if (lease_expired(lease)) {
					/* probably best if we drop this lease */
					p_dbg("6");
					memset(lease->chaddr, 0, 16);
				/* make some contention for this address */
				} else sendNAK(dhcp_packet);
			} else if (requested_align < server_config.start || 
					 requested_align > server_config.end) {
					 p_dbg("7");
				sendNAK(dhcp_packet);
			} /* else remain silent */

		} else {
			 /* RENEWING or REBINDING State */
			 p_dbg("8");
		}
		break;
	case DHCPDECLINE:
		DEBUG(LOG_INFO,"received DECLINE");
		if (lease) {
			memset(lease->chaddr, 0, 16);
			lease->expires = time(0) + server_config.decline_time;
		}			
		break;
	case DHCPRELEASE:
		DEBUG(LOG_INFO,"received RELEASE");
		if (lease) lease->expires = time(0);
		break;
	case DHCPINFORM:
		DEBUG(LOG_INFO,"received INFORM");
		send_inform(dhcp_packet);
		break;	
	default:
		LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
	}
	mutex_unlock(socket_mutex);

	release_dhcp_packet();

	return 0;
}


#if 0
#ifdef COMBINED_BINARY	
int udhcpd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{	
	fd_set rfds;
	struct timeval tv;
	int server_socket = -1;
	int bytes, retval;
	struct dhcpMessage packet;
	unsigned char *state;
	unsigned char *server_id, *requested;
	u_int32_t server_id_align, requested_align;
	unsigned long timeout_end;
	struct option_set *option;
	struct dhcpOfferedAddr *lease;
	int pid_fd;
	int max_sock;
	int sig;
	
	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);

	memset(&server_config, 0, sizeof(struct server_config_t));
	
	if (argc < 2)
		read_config(DHCPD_CONF_FILE);
	else read_config(argv[1]);

	pid_fd = pidfile_acquire(server_config.pidfile);
	pidfile_write_release(pid_fd);

	if ((option = find_option(server_config.options, DHCP_LEASE_TIME))) {
		memcpy(&server_config.lease, option->data + 2, 4);
		server_config.lease = ntohl(server_config.lease);
	}
	else server_config.lease = LEASE_TIME;
	
	leases = malloc(sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	memset(leases, 0, sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	read_leases(server_config.lease_file);

	if (read_interface(server_config.interface, &server_config.ifindex,
			   &sys_cfg.netif->ip_addr.addr, server_config.arp) < 0)
		exit_server(1);

#ifndef DEBUGGING
	pid_fd = pidfile_acquire(server_config.pidfile); /* hold lock during fork. */
	if (daemon(0, 0) == -1) {
		perror("fork");
		exit_server(1);
	}
	pidfile_write_release(pid_fd);
#endif


	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR1, signal_handler);
	signal(SIGTERM, signal_handler);

	timeout_end = time(0) + server_config.auto_time;
	while(1) { /* loop until universe collapses */

		if (server_socket < 0)
			if ((server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface)) < 0) {
				LOG(LOG_ERR, "FATAL: couldn't create server socket, %s", strerror(errno));
				exit_server(0);
			}			

		FD_ZERO(&rfds);
		FD_SET(server_socket, &rfds);
		FD_SET(signal_pipe[0], &rfds);
		if (server_config.auto_time) {
			tv.tv_sec = timeout_end - time(0);
			tv.tv_usec = 0;
		}
		if (!server_config.auto_time || tv.tv_sec > 0) {
			max_sock = server_socket > signal_pipe[0] ? server_socket : signal_pipe[0];
			retval = select(max_sock + 1, &rfds, NULL, NULL, 
					server_config.auto_time ? &tv : NULL);
		} else retval = 0; /* If we already timed out, fall through */

		if (retval == 0) {
			write_leases();
			timeout_end = time(0) + server_config.auto_time;
			continue;
		} else if (retval < 0 && errno != EINTR) {
			DEBUG(LOG_INFO, "error on select");
			continue;
		}
		
		if (FD_ISSET(signal_pipe[0], &rfds)) {
			if (read(signal_pipe[0], &sig, sizeof(sig)) < 0)
				continue; /* probably just EINTR */
			switch (sig) {
			case SIGUSR1:
				LOG(LOG_INFO, "Received a SIGUSR1");
				write_leases();
				/* why not just reset the timeout, eh */
				timeout_end = time(0) + server_config.auto_time;
				continue;
			case SIGTERM:
				LOG(LOG_INFO, "Received a SIGTERM");
				exit_server(0);
			}
		}

		if ((bytes = get_packet(&packet, server_socket)) < 0) { /* this waits for a packet - idle */
			if (bytes == -1 && errno != EINTR) {
				DEBUG(LOG_INFO, "error on read, %s, reopening socket", strerror(errno));
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}

		if ((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_ERR, "couldn't get option from packet, ignoring");
			continue;
		}
		
		/* ADDME: look for a static lease */
		lease = find_lease_by_chaddr(packet.chaddr);
		switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG(LOG_INFO,"received DISCOVER");
			
			if (sendOffer(&packet) < 0) {
				LOG(LOG_ERR, "send OFFER failed");
			}
			break;			
 		case DHCPREQUEST:
			DEBUG(LOG_INFO, "received REQUEST");

			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);

			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);
		
			if (lease) { /*ADDME: or static lease */
				if (server_id) {
					/* SELECTING State */
					DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
					if (server_id_align == sys_cfg.netif->ip_addr.addr && requested && 
					    requested_align == lease->yiaddr) {
						sendACK(&packet, lease->yiaddr);
					}
				} else {
					if (requested) {
						/* INIT-REBOOT State */
						if (lease->yiaddr == requested_align)
							sendACK(&packet, lease->yiaddr);
						else sendNAK(&packet);
					} else {
						/* RENEWING or REBINDING State */
						if (lease->yiaddr == packet.ciaddr)
							sendACK(&packet, lease->yiaddr);
						else {
							/* don't know what to do!!!! */
							sendNAK(&packet);
						}
					}						
				}
			
			/* what to do if we have no record of the client */
			} else if (server_id) {
				/* SELECTING State */

			} else if (requested) {
				/* INIT-REBOOT State */
				if ((lease = find_lease_by_yiaddr(requested_align))) {
					if (lease_expired(lease)) {
						/* probably best if we drop this lease */
						memset(lease->chaddr, 0, 16);
					/* make some contention for this address */
					} else sendNAK(&packet);
				} else if (requested_align < server_config.start || 
					   requested_align > server_config.end) {
					sendNAK(&packet);
				} /* else remain silent */

			} else {
				 /* RENEWING or REBINDING State */
			}
			break;
		case DHCPDECLINE:
			DEBUG(LOG_INFO,"received DECLINE");
			if (lease) {
				memset(lease->chaddr, 0, 16);
				lease->expires = time(0) + server_config.decline_time;
			}			
			break;
		case DHCPRELEASE:
			DEBUG(LOG_INFO,"received RELEASE");
			if (lease) lease->expires = time(0);
			break;
		case DHCPINFORM:
			DEBUG(LOG_INFO,"received INFORM");
			send_inform(&packet);
			break;	
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}
	}

	return 0;
}
#endif

