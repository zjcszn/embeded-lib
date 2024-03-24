#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"



#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/sockets.h"

struct netif if_wifi;
struct netif *p_netif = NULL; //一个netif对应一个网络设备

//socket互斥锁
mutex_t socket_mutex;

extern int close_socket(uint32_t socket_num);


void tcpip_init_end(void *pdata)
{
	p_dbg("%s", __FUNCTION__);
}

extern err_t ethernetif_init(struct netif *netif);
extern err_t tcpip_input(struct pbuf *p, struct netif *inp);
int lwip_netif_init()
{
	struct ip_addr ipaddr, netmask, gw;

	gw.addr = inet_addr("192.168.1.1");
	ipaddr.addr = inet_addr("192.168.1.8");
	netmask.addr = inet_addr("255.255.255.0");

	if (p_netif)
		netif_remove(p_netif);

	p_netif = &if_wifi;
	if (netif_add(p_netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input) == 0)
	{
		p_err("netif_add faild \r\n");
		return  - 1;
	}

	netif_set_default(p_netif);
	netif_set_up(p_netif);
	return 0;
}

/*
 * @brief  自动获取ip地址
 *
 */
int auto_get_ip()
{
	struct ip_addr ipaddr;
	ipaddr.addr = 0;
	p_dbg_enter;
	netif_set_down(p_netif); //先关掉,dhcp_start会启动它
	netif_set_ipaddr(p_netif, &ipaddr); //清零
	netif_set_netmask(p_netif, &ipaddr);
	netif_set_gw(p_netif, &ipaddr);
	dns_setserver(0, &ipaddr);
	dhcp_start(p_netif);
	return 0;
}

/*
 * @brief  域名解析
 *
 */
int get_host_by_name(char *hostname, uint32_t *addr)
{
	char err_t;
	ip_addr_t ip_addr;
	/**
	 * Execute a DNS query, only one IP address is returned
	 *
	 * @param name a string representation of the DNS host name to query
	 * @param addr a preallocated ip_addr_t where to store the resolved IP address
	 * @return ERR_OK: resolving succeeded
	 *         ERR_MEM: memory error, try again later
	 *         ERR_ARG: dns client not initialized or invalid hostname
	 *         ERR_VAL: dns server response was invalid
	 */
	mutex_lock(socket_mutex);
	err_t = netconn_gethostbyname(hostname, &ip_addr);
	mutex_unlock(socket_mutex);
	if (err_t == ERR_OK)
	{
		*addr = ip_addr.addr;
		return 0;
	}

	return  - 1;
}

/*
 * @brief  获取socket连接的状态
 *
 */
int get_connect_stat(int socket_num, struct ret_connect_stat *stat)
{
	struct lwip_sock *sock;

	memset(stat, 0, sizeof(struct ret_connect_stat));
	sock = get_socket(socket_num);
	if (!sock || !sock->conn)
	{
		return  - 1;
	}

	stat->socket_num = socket_num;
	if (sock->conn->type == NETCONN_TCP)
		stat->type = SOCK_STREAM;
	else if (sock->conn->type == NETCONN_UDP)
		stat->type = SOCK_DGRAM;
	else
		stat->type = 0;

	if (stat->type == SOCK_STREAM)
	{
		stat->local_port = sock->conn->pcb.tcp->local_port;
		stat->remote_port = sock->conn->pcb.tcp->remote_port;
		stat->local_addr = sock->conn->pcb.tcp->local_ip.addr;
		stat->remote_addr = sock->conn->pcb.tcp->remote_ip.addr;
	}
	else if (stat->type == SOCK_DGRAM)
	{
		stat->remote_port = sock->conn->pcb.udp->remote_port;
		stat->local_port = sock->conn->pcb.udp->local_port;
		stat->local_addr = sock->conn->pcb.udp->local_ip.addr;
		stat->remote_addr = sock->conn->pcb.udp->remote_ip.addr;
	}
	else
		return  - 1;

	return 0;
}


int tcp_link(char *ip, uint16_t port)
{
	int socket_c, err;
	struct sockaddr_in addr;

	p_dbg("enter %s\n", __FUNCTION__);
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	socket_c = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_c < 0)
	{
		p_err("get socket err:%d", errno);
		return  - 1;
	}
	//mutex_lock(socket_mutex);
	err = connect(socket_c, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));
	//mutex_unlock(socket_mutex);
	if (err ==  - 1)
	{
		p_err("link err:%d\n", errno);
		close_socket(socket_c);
		return  - 1;
	}

	//设置keepalive，如果远程段非正常关闭，我们可以尽快察觉到并释放已经断开的链接
	err = 1;
	if (setsockopt(socket_c, SOL_SOCKET, SO_KEEPALIVE, &err, sizeof(int)) ==  - 1)
		p_err("SO_KEEPALIVE err\n");

	

	p_dbg("link ok\n");
	return socket_c;
}

//
/*
 *UDP通信不要求预先连接，
 *预先连接的好处是可以直接调用send发送数据
 *
 */
int udp_link(char *ip, uint16_t port)
{
	int socket_c, err;
	struct sockaddr_in addr;

	p_dbg("enter %s\n", __FUNCTION__);
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	socket_c = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_c < 0)
	{
		p_err("get socket err:%d", errno);
		return  - 1;
	} 
	mutex_lock(socket_mutex);
	err = connect(socket_c, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));
	mutex_unlock(socket_mutex);
	if (err ==  - 1)
	{
		p_err("link err:%d\n", errno);
		close_socket(socket_c);
		return  - 1;
	}

	//设置keepalive，如果远程段非正常关闭，我们可以尽快察觉到并释放已经断开的链接
	err = 1;
	if (setsockopt(socket_c, SOL_SOCKET, SO_KEEPALIVE, &err, sizeof(int)) ==  - 1)
		p_err("SO_KEEPALIVE err\n");


	p_dbg("link ok\n");
	return socket_c;
}

//
/*
 *添加到多播组，同一组内的设备可以互相收发广播数据
 *
 *ip地址范围224.0.0.1 - 239.255.255.255
 */
int udp_add_membership(char *ip, uint16_t port)
{
	int socket_c, err;
	struct sockaddr_in addr;
	struct ip_mreq imr;
	p_dbg("enter %s\n", __FUNCTION__);

	socket_c = socket(AF_INET, SOCK_DGRAM, 0);
	p_dbg("socket ret:%d", socket_c);

	mutex_lock(socket_mutex);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_len =  sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	err = bind(socket_c, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));
	p_dbg("bind ret:%d", err);
	if (err ==  - 1)
	{
		close(socket_c);
		mutex_unlock(socket_mutex);
		return  - 1;
	}

	//imr.imr_interface.s_addr = htons(INADDR_ANY);
	imr.imr_interface.s_addr =  p_netif->ip_addr.addr;
	imr.imr_multiaddr.s_addr = inet_addr(ip);
	err = setsockopt(socket_c, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(struct ip_mreq));
	p_dbg("IP_ADD_MEMBERSHIP ret:%d", err);
	if (err ==  - 1)
	{
		close(socket_c);
		mutex_unlock(socket_mutex);
		return  - 1;
	} 
	mutex_unlock(socket_mutex);

	p_dbg("link ok\n");
	return socket_c;
}

/*
 *创建UDP服务器，并且设置为可以接收广播
 *
 *
 */
int udp_create_server(uint16_t port)
{
	int socket_c, err;
	struct sockaddr_in addr;

	p_dbg("enter %s\n", __FUNCTION__);
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htons(INADDR_ANY);

	socket_c = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_c < 0)
	{
		p_err("get socket err:%d", errno);
		return  - 1;
	} mutex_lock(socket_mutex);
	err = bind(socket_c, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));
	p_dbg("bind ret:%d", err);
	if (err ==  - 1)
	{
		close(socket_c);
		mutex_unlock(socket_mutex);
		return  - 1;

	}
#if 0	//如果希望接收广播数据,请置1
	err = setsockopt(socket_c, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n));
	p_dbg("SO_BROADCAST ret:%d", err);
	if(err == -1)
	{
	close(socket_c);
	mutex_unlock(socket_mutex);
	return -1;
	}
#endif
	mutex_unlock(socket_mutex);
	p_dbg("server ok:%d", socket_c);
	return socket_c;
}

/*
 *remote_port和remote_addr为大端模式
 */
int udp_data_send(int socket, void *data, int len, uint16_t remote_port, uint32_t remote_addr)
{
	int remen = len;

	struct lwip_sock *sock;
	p_dbg("enter %s ,%d\n", __FUNCTION__, socket);

	sock = get_socket(socket);
	if (!sock || !sock->conn)
	{
		return 0;
	}

	if (sock->conn->type != NETCONN_UDP)
	{
		p_err("is not udp socket");
		return 0;
	}
	do
	{
		struct sockaddr udp_remote_addr;
		mutex_lock(socket_mutex);
		memset(&udp_remote_addr, 0, sizeof(struct sockaddr));
		udp_remote_addr.sa_len = sizeof(struct sockaddr);
		udp_remote_addr.sa_family = AF_INET;
		memcpy(udp_remote_addr.sa_data, &remote_port, 2);
		memcpy(&udp_remote_addr.sa_data[2], &remote_addr, 4);

		len = sendto(socket, (u8*)data, len, 0, &udp_remote_addr, sizeof(struct sockaddr));

		mutex_unlock(socket_mutex);
		if (len ==  - 1)
		{
			//EAGAIN == timeout
			if ((sockets[socket].err == EAGAIN) || (sockets[socket].err == ENOMEM) || (sockets[socket].err == EINTR))
			{
				return 0;
			} p_err("send fatal err:%d\n", sockets[socket].err);
			close_socket(socket);
			break;
		}

		if (len != remen)
		{
			if (sockets[socket].err != 0)
			{
				p_err("send err1:%d,%d\n", len, remen);
				close_socket(socket);
			}

			if (len == 0)
				p_err("send 0:%d\n", sockets[socket].err);
		}
	}
	while (0);
	return len;
}

extern void tcp_recv_thread(void *arg)
	;
int init_lwip(void)
{
	socket_mutex = mutex_init(__FUNCTION__);  //对socket的操作尽量使用mutex锁(非阻塞的操作),以提高系统稳定性

	tcpip_init(tcpip_init_end, NULL);

	lwip_netif_init();

	thread_create(tcp_recv_thread, 0, TASK_TCP_RECV_PRIO, 0, TASK_TCP_RECV_STACK_SIZE, "tcp_recv_thread");

	return 0;
}
