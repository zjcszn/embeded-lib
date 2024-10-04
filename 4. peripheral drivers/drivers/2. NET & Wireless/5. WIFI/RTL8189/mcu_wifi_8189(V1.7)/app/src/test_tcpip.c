#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "lwip\sockets.h"
#include "lwip\netif.h"
#include "lwip\dns.h"
#include "lwip\api.h"
#include "lwip\tcp.h"

#include "debug.h"
#include "lwip/dhcp.h"
#include "dhcpd.h"
#include "sys_misc.h"

#include "webserver.h"

//全局变量声明
extern int errno; //lwip错误号

/*
 * socket互斥锁，对socket的所有操作都需要上锁
 * 如果不加锁，lwip会很不稳定，详细原因没有研究
 * 需要注意的是像send这样可能会阻塞的函数，如果加了锁将会降低数据
 * 收发的效率，所以收发数据时采用非阻塞方式要好些(MSG_DONTWAIT)
 */
extern mutex_t socket_mutex;

//函数声明
void test_tcp_recv(void *_fd);

//socket 文件句柄
int client_socket_fd =  - 1;
int server_socket_fd =  - 1;
int remote_socket_fd =  - 1;

struct sockaddr udp_remote_client __attribute__((aligned(4))); //用于保存远程udp客户端信息

//thread 句柄
int server_accept_thread_fd =  - 1;

//用于select操作的变量
fd_set rfds;

/*
 * @brief  连接到IP地址为192.168.1.101，端口号4700的TCP服务器
 *
 */
void test_tcp_link()
{
	char *ip = "192.168.0.108";
	uint16_t port = 4700;
	p_dbg_enter;
	p_dbg("连接到:%s, 端口号:%d", ip, port);
	if (!is_wifi_connected())
	{

		p_err("wifi not connect");
		return ;
	}

	client_socket_fd = tcp_link(ip, port);

	p_dbg_exit;
}

/*
 * @brief  连接到IP地址为192.168.1.101，端口号4701的UDP服务器
 *
 */
void test_udp_link()
{
	char *ip = "192.168.1.100";
	uint16_t port = 4701;
	p_dbg_enter;
	p_dbg("连接到:%s, 端口号:%d", ip, port);
	if (!is_wifi_connected())
	{

		p_err("wifi not connect");
		return ;
	}

	client_socket_fd = udp_link(ip, port);

	p_dbg_exit;
}

/*
 * @brief  添加到多播组
 * 组播地址为224.0.0.1，端口号4702
 */
void test_multicast_join()
{
	char *ip = "239.118.0.0";
	uint16_t port = 10000;
	p_dbg_enter;
	p_dbg("连接到:%s, 端口号:%d", ip, port);
	if (!is_wifi_connected())
	{

		p_err("wifi not connect");
		return ;
	}

	client_socket_fd = udp_add_membership(ip, port);

	p_dbg_exit;
}

/*
 * @brief  建立udp服务器
 * 端口号4703
 */
void test_udp_server()
{
	uint16_t port = 4703;
	p_dbg_enter;
	p_dbg("test_udp_server, 端口号:%d", port);
	if (!is_wifi_connected())
	{

		p_err("wifi not connect");
		return ;
	}

	server_socket_fd = udp_create_server(port);

	p_dbg_exit;
}


/*
 * @brief 关闭一个socket
 *注意这里会执行mutex_lock(socket_mutex);应用中防止嵌套
 * 
 */
int close_socket(uint32_t socket_num)
{
	struct lwip_sock *sock;
	int ret;
	p_dbg_enter;
	sock = get_socket(socket_num);

	if (!sock || !sock->conn)
	{
		p_err("close_socket err1\n");
		return  - 1;
	} if (!((sock->conn->state != NETCONN_CONNECT) || (NETCONN_FLAG_IN_NONBLOCKING_CONNECT &(sock->conn->flags))))
	{
		p_err("close_socket err2\n");
		return  - 1;
	}
	mutex_lock(socket_mutex);
	shutdown(socket_num, SHUT_RDWR);
	ret = close(socket_num);
	if (ret ==  - 1)
		p_err("close_socket err4:%d\n", ret);
	if (FD_ISSET(socket_num, &rfds))
		FD_CLR(socket_num, &rfds);
	mutex_unlock(socket_mutex);
	p_dbg_exit;
	return ret;
}

/*
 * @brief  断开TCP连接
 *
 */
void test_tcp_unlink()
{
	p_dbg_enter;
	if (client_socket_fd !=  - 1)
	{
		close_socket(client_socket_fd);
		client_socket_fd =  - 1;
	}
	p_dbg_exit;
}


DECLARE_MONITOR_ITEM("tcp totol send", tcp_totol_send);

/*
 * @brief  发送数据到远程服务器
 *
 */
void test_send(char *pstr)
{
	int ret, len = strlen(pstr);
	p_dbg_enter;
	if (remote_socket_fd !=  - 1)
	{
		mutex_lock(socket_mutex);
		ret = send(remote_socket_fd, pstr, len, 0);
		mutex_unlock(socket_mutex);
		if (ret != len)
		{
			p_err("send data err:%d", ret);
			close_socket(remote_socket_fd);
			remote_socket_fd =  - 1;
		}
	}
	if (client_socket_fd !=  - 1)
	{
		mutex_lock(socket_mutex);
		ret = send(client_socket_fd, pstr, len, 0);
		mutex_unlock(socket_mutex);
		if (ret != len)
		{
			p_err("send data er1r:%d", ret);
			close_socket(client_socket_fd);
			client_socket_fd =  - 1;
		}
	}
	ADD_MONITOR_VALUE(tcp_totol_send, len);
	p_dbg_exit;
}

/*
 * @brief TCP数据发送线程
 * tcp_send_stop用于用户控制结束测试
 *
 */
int tcp_send_stop = 0;
#define TEST_PACKET_SIZE 	1024
void tcp_send_thread(void *arg)
{
	int i;
	char *send_buff;

	send_buff = (char*)malloc(TEST_PACKET_SIZE);
	if(!send_buff)
		goto end;

	if(client_socket_fd == -1){
		p_err("请先与服务器建立TCP连接");
		goto end;
	}

	for(i = 0; i < TEST_PACKET_SIZE; i++)
		send_buff[i] = i;
	p_dbg("tcp send start");
	while(1)
	{

		mutex_lock(socket_mutex);
		//采用阻塞方式发送,这种方式发送会阻塞相当长时间
		i = send(client_socket_fd, send_buff, TEST_PACKET_SIZE, /*MSG_DONTWAIT*/0);
		mutex_unlock(socket_mutex);

		ADD_MONITOR_VALUE(tcp_totol_send, TEST_PACKET_SIZE);
		if(tcp_send_stop)
		{
			p_dbg("stop tcp send test");
			goto end;
		}
		//sleep(10); //如果想减慢发送速度，可以在这里延时一下
	}

end:
	p_dbg("tcp send end");
	if(send_buff)
		mem_free(send_buff);
	thread_exit(thread_myself());
}
/*
 * @brief 满负荷发送，请先建立TCP连接
 * 建立单独的线程
 *
 */
void test_full_speed_send()
{
	p_dbg_enter;
	tcp_send_stop = 0;
	thread_create(tcp_send_thread, 0, TASK_TCP_SEND_PRIO, 0, TASK_TCP_SEND_STACK_SIZE, "tcp_send_thread");
	p_dbg_exit;
}

void test_full_speed_send_stop()
{
	p_dbg_enter;
	tcp_send_stop = 1;
	p_dbg_exit;
}


/*
 *这里测试发送udp数据到远程端
 *这里新建一个socket并发送数据到192.168.1.101:4703
 */
void test_sendto1(char *pstr)
{
	int fd;
	struct sockaddr_in addr;
	int len = strlen(pstr);
	
	p_dbg_enter;

	p_dbg("enter %s\n", __FUNCTION__);
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(4703);
	addr.sin_addr.s_addr = inet_addr("192.168.1.101");

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		p_err("get socket err:%d", errno);
		return;
	} 

	mutex_lock(socket_mutex);
	len = sendto(fd, (u8*)pstr, len, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
	mutex_unlock(socket_mutex);
	p_dbg("ret:%d", len);
	close(fd);
	
	p_dbg_exit;
}

/*
 *这里测试发送udp数据到远程端
 *fd 使用server_socket_fd，即test_udp_server函数建立的socket
 */
void test_sendto(char *pstr)
{
#if 1
	struct lwip_sock *sock;
	struct sockaddr_in addr;
	uint32_t remote_addr;
	uint16_t remote_port;
	int len = strlen(pstr);
	p_dbg_enter;
	if (server_socket_fd ==  - 1)
		return ;

	memcpy(&addr, &udp_remote_client, sizeof(struct sockaddr_in));
	remote_addr = addr.sin_addr.s_addr;
	remote_port = addr.sin_port;

	sock = get_socket(server_socket_fd);
	if (!sock || !sock->conn)
	{
		return ;
	}

	if (sock->conn->type != NETCONN_UDP)
	{
		p_err("is not udp socket");
		return ;
	}
	p_dbg("发送数据到:%x, 端口号:%d", remote_addr, remote_port); //这里显示的大端模式的数值

	udp_data_send(server_socket_fd, pstr, len, remote_port, remote_addr);
	p_dbg_exit;
#endif
	test_sendto1(pstr);	
}


/*
 * @brief  关闭本地服务器
 *
 */
void test_close_tcp_server()
{
	p_dbg_enter;
	if (remote_socket_fd !=  - 1)
	{
		close_socket(remote_socket_fd);
		remote_socket_fd =  - 1;
	}

	if (server_socket_fd !=  - 1)
	{
		close_socket(server_socket_fd);
		server_socket_fd =  - 1;
	}

	if (server_accept_thread_fd !=  - 1)
	{
		thread_exit(server_accept_thread_fd);
		server_accept_thread_fd =  - 1;
	}

	p_dbg_exit;
}

/*
 * @brief 本地服务器监听线程
 *
 */
void tcp_accept_task(void *server_fd)
{
	int sockaddr_len, new_socket, opt;
	struct sockaddr_in addr;

	sockaddr_len = sizeof(struct sockaddr);

	while (1)
	{
		p_dbg("waiting for remote connect");
		new_socket = accept((int)server_fd, (struct sockaddr*) &addr, (socklen_t*) &sockaddr_len);
		if (new_socket ==  - 1)
		{
			p_err("accept err");
			break;
		} p_dbg("accept a new client");
		remote_socket_fd = new_socket;

		opt = 1;
		if (setsockopt(new_socket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(int)) ==  - 1)
			p_err("SO_KEEPALIVE err\n");

	}
	thread_exit(server_accept_thread_fd);
}


void test_dns(char *hostname)
{
	uint32_t addr;
	int ret;

	p_dbg_enter;
	ret = get_host_by_name(hostname, &addr);

	if (ret == 0)
	{
		p_dbg("get %s, ipaddr:: %d.%d.%d.%d\n", hostname, ip4_addr1(&addr), ip4_addr2(&addr), ip4_addr3(&addr), ip4_addr4(&addr));

	}
	p_dbg_exit;
}

/*
 * @brief  在本地建立一个服务器并等待连接
 * 服务器端口号4800
 *
 */
void test_tcp_server()
{

	int socket_s =  - 1, err = 0;
	uint16_t port = 4800;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	p_dbg_enter;

	p_dbg("建立服务器, 端口号:%d", port);
	test_close_tcp_server();

	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = htons(INADDR_ANY);

	socket_s = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_s ==  - 1)
	{
		goto err;
	}
	err = bind(socket_s, (struct sockaddr*) &serv, sizeof(struct sockaddr_in));
	if (err ==  - 1)
	{
		goto err;
	} err = listen(socket_s, 4);
	if (err ==  - 1)
	{
		goto err;
	}

	server_socket_fd = socket_s;

	server_accept_thread_fd = thread_create(tcp_accept_task, (void*)server_socket_fd, TASK_TCP_ACCEPT_PRIO, 0, TASK_ACCEPT_STACK_SIZE, "tcp_accept_task");

	return ;
	err: if (err < 0)
		p_err("err:%d", err);
	if (socket_s !=  - 1)
		close_socket(socket_s);
	p_dbg_exit;
}


void show_tcpip_info()
{
	ip_addr_t dns_server;

	dns_server = dns_getserver(0);

	p_dbg("ipaddr: %d.%d.%d.%d", ip4_addr1(&p_netif->ip_addr.addr), ip4_addr2(&p_netif->ip_addr.addr), ip4_addr3(&p_netif->ip_addr.addr), ip4_addr4(&p_netif->ip_addr.addr));

	p_dbg("netmask: %d.%d.%d.%d", ip4_addr1(&p_netif->netmask.addr), ip4_addr2(&p_netif->netmask.addr), ip4_addr3(&p_netif->netmask.addr), ip4_addr4(&p_netif->netmask.addr));

	p_dbg("gw: %d.%d.%d.%d", ip4_addr1(&p_netif->gw.addr), ip4_addr2(&p_netif->gw.addr), ip4_addr3(&p_netif->gw.addr), ip4_addr4(&p_netif->gw.addr));


	p_dbg("dns_server: %d.%d.%d.%d", ip4_addr1(&dns_server.addr), ip4_addr2(&dns_server.addr), ip4_addr3(&dns_server.addr), ip4_addr4(&dns_server.addr));
}


/*
 * @brief  自动获取IP测试
 *
 *
 */
void test_auto_get_ip()
{
	int i, wait_time = 10;

	auto_get_ip();

	for (i = 0; i < wait_time; i++)
	{
		p_dbg("%d", i);
		if (p_netif->ip_addr.addr)
			break;
		sleep(1000);
	}

	if (p_netif->ip_addr.addr)
	{
		show_tcpip_info();
	}

}

#if SUPPORT_AUDIO

/*
 * @brief  打开音频功能
 *
 */
void test_open_audio()
{
	int ret;
    
#ifdef CHIP_F103
	p_err_fun;
#else
    if(audio_cfg.audio_dev_open)
        audio_dev_close(); 
    
	ret = audio_dev_open();
	if (ret != 0)
	{
		p_err("open err");
		return ;
	}

	audio_cfg.volume = 100;
	audio_cfg.audio_net_port = 4700;
	audio_cfg.dec_input->samplerate = audio_cfg.adc->samplerate = AUDIO_SAMPLERATE32000;

	adc_switch_samplerate(audio_cfg.adc->samplerate);
	dac_switch_samplerate(audio_cfg.dec_input->samplerate);
#endif
}

/*
 * @brief  关闭音频功能
 *
 */
void test_close_audio()
{
	audio_dev_close();
}

/*
 * @brief  发送音频数据
 * 被adc_recv_thread调用
 */
void send_audio_data_to_remote(char *buff, int len)
{
	int ret = 0;
	if (remote_socket_fd !=  - 1)
	{
		mutex_lock(socket_mutex);
		ret = send(remote_socket_fd, buff, len, 0);
		mutex_unlock(socket_mutex);
		if (ret != len)
		{
			p_err("send data err:%d", ret);
			close_socket(remote_socket_fd);
			remote_socket_fd =  - 1;
		}
	}
	if (client_socket_fd !=  - 1)
	{
		mutex_lock(socket_mutex);
		ret = send(client_socket_fd, buff, len, 0);
		mutex_unlock(socket_mutex);
		if (ret != len)
		{
			p_err("send data er1r:%d", ret);
			close_socket(client_socket_fd);
			client_socket_fd =  - 1;
		}
	}
	if(ret > 0)	
		ADD_MONITOR_VALUE(tcp_totol_send, ret);
}
#endif



int is_led_ctrl_cmd(char *buff)
{
	if ((buff[0] == 0xaa) && (buff[1] == 0x55) && (buff[2] == 1 || buff[2] == 2))
		return 1;

	return 0;
}

/*是否是udpserver服务器，如果是的话我们要取出远程端地址*/
int is_udp_server_socket(int num)
{
	struct lwip_sock *sock;

	if (num != server_socket_fd)
		return 0;

	sock = get_socket(num);
	if (!sock || !sock->conn)
	{
		return 0;
	}

	if (sock->conn->type == NETCONN_UDP)
	{
		return 1;
	}

	return 0;

}


int is_dhcp_socket(int num)
{
	uint16_t tmp_port = 0;
	struct lwip_sock *sock;

	sock = get_socket(num);
	if (!sock || !sock->conn)
	{
		return 0;
	}

	if (sock->conn->type == NETCONN_UDP)
	{
		tmp_port = sock->conn->pcb.udp->local_port; //server

		if (tmp_port == DHCP_SERVER_PORT)
			return 1;
	}
	return 0;
}

int is_web_socket(int num)
{
	uint16_t tmp_port = 0;
	struct lwip_sock *sock;

	sock = get_socket(num);
	if (!sock || !sock->conn)
	{
		return 0;
	}

	if (sock->conn->type == NETCONN_TCP)
	{
		tmp_port = sock->conn->pcb.tcp->local_port; //server

		if (tmp_port == 80)
			return 1;
	}
	return 0;
}

char loopback_enable = 1;
void switch_loopback_test()
{
	loopback_enable = !loopback_enable;
	if(loopback_enable)
		p_dbg("使能回发测试");
	else
		p_dbg("关闭回发测试");
}


/*
 *selcet模式接收数据的例子，可以监控多个socket的数据接收
 *所有发送到开发板的udp和tcp数据都从这里读取
 */
DECLARE_MONITOR_ITEM("tcp totol recv", tcp_totol_recv);
#define TCP_RCV_SIZE 1024
void tcp_recv_thread(void *arg)
{
	int i, size, retval, select_size, udp_server_data;
	struct lwip_sock *sock;
	char *tcp_rcv_buff;
	struct timeval tv;

	tcp_rcv_buff = (char*)mem_malloc(TCP_RCV_SIZE + 4);
	while (1)
	{
		mutex_lock(socket_mutex);
		FD_ZERO(&rfds);

		for (i = 0; i < MEMP_NUM_NETCONN; i++)
		{
			sock = get_socket(i);
			if (sock && sock->conn && sock->conn->recvmbox)
			//socket关闭和接收关闭则不能接收
			{
				FD_SET(i, &rfds);
			}
		}
		select_size = MEMP_NUM_NETCONN;

		mutex_unlock(socket_mutex);

		tv.tv_sec = 5;
		tv.tv_usec = 0;
		retval = select(select_size, &rfds, NULL, NULL, &tv);
		if ((retval ==  - 1) || (retval == 0))
		{
			sleep(50); //内存错误
		}
		else
		{
			if (retval)
			{
				for (i = 0; i < select_size; i++)
				{
					if (FD_ISSET(i, &rfds))
					{
						struct sockaddr remote_addr;
						mutex_lock(socket_mutex);

						#if 1	//使用recvfrom接收
						retval = sizeof(struct sockaddr);
						size = recvfrom(i, tcp_rcv_buff, TCP_RCV_SIZE, MSG_DONTWAIT, &remote_addr, (socklen_t*) &retval);
						/*	
						p_dbg("rcv from:%d,%d,%d,%d,%d,%d\n",udp_remote_addr.sa_data[0],remote_addr.sa_data[1],
						remote_addr.sa_data[2],
						remote_addr.sa_data[3],
						remote_addr.sa_data[4],
						remote_addr.sa_data[5]	);*/
						#else //使用recv接收
						size = recv(i, tcp_rcv_buff, TCP_RCV_SIZE, MSG_DONTWAIT);
						#endif
						mutex_unlock(socket_mutex);
						if (size ==  - 1)
						{
							if (errno == EWOULDBLOCK || errno == ENOMEM) 
							{
								//出现这两种情况不代表socket断开,所以不能关闭socket,不过很少出现这种情况:)
								p_err("tcp_recv err:%d,%d\n", i, errno);
								sleep(10);
							}
							else
							{
								p_err("tcp_recv fatal err:%d,%d\n", i, errno);
								close_socket(i);
							}
							continue;
						}
						if (size == 0)
						//0代表连接已经关闭
						{
							if (errno != 0)
							{
								p_err("tcp_client_recv %d err1:%d\n", i, errno);
								close_socket(i);
							}
							else
								p_err("rcv 0 byte?\n");
							continue;
						}

						//p_dbg("socket:%d rcv:%d\n", i, size);
						ADD_MONITOR_VALUE(tcp_totol_recv, size);
						if (is_udp_server_socket(i))
						{
							udp_remote_client = remote_addr;
							udp_server_data = 1;
						}
						else
							udp_server_data = 0;
#if	SUPPORT_WEBSERVER
						if(is_web_socket(i))
						{
							handle_web_sm(i, (PUCHAR)tcp_rcv_buff, size);
						}
						else 
#endif
						if (is_dhcp_socket(i))
						{

							handle_dhcp_rcv((uint8_t*)tcp_rcv_buff, size);
						}
						else
#if MACHTALK_ENABLE
						if (is_machtalk_socket(i))
						{

							handle_machtalk_rcv(i, (uint8_t*)tcp_rcv_buff, size);
						}
						else
#endif
						{

							if(0)
								;
							else
#if SUPPORT_AUDIO
							if (audio_cfg.audio_dev_open)
							{
								handle_audio_stream((unsigned char*)tcp_rcv_buff, size); //播放音频
							}
							else
#endif
							{

								if (is_led_ctrl_cmd(tcp_rcv_buff))
								{
									if (tcp_rcv_buff[2] == 1)
										led_switch(tcp_rcv_buff[3]);
									if (tcp_rcv_buff[2] == 2)
										led_bright(tcp_rcv_buff[3]);
								}

								//发回测试,采用非阻塞方式发送
								//非阻塞方式发送时一次不一定能全部发送出去，所以会看到接收的数据量会大于发送的数据量
								//如果使用阻塞方式发送，这里将阻塞相当长的时间，影响测试效果
								if(loopback_enable)
								{
									mutex_lock(socket_mutex);
									if (udp_server_data)
										size = sendto(i, (u8*)tcp_rcv_buff, size, MSG_DONTWAIT, &udp_remote_client, sizeof(struct sockaddr));
									else
										size = send(i, tcp_rcv_buff, size, MSG_DONTWAIT);
									mutex_unlock(socket_mutex);
									if(size > 0)
										ADD_MONITOR_VALUE(tcp_totol_send, size);
								}							
								
								//打印接收到的数据
								//dump_hex("data", tcp_rcv_buff, size);//16进制形式打印出来
								//p_dbg("%s", tcp_rcv_buff);//字符串形式打印出来
							}
						}
					} //end of if(FD_ISSET(i,&rfds))
				} //end of for(i = 0; i < select_size; i++)
			}
		}
	}
}
