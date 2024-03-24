#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "machtalk_remote_control.h"
#include "lwip\sockets.h"
#include "lwip\netif.h"
#include "lwip\dns.h"
#include "lwip\api.h"
#include "lwip\tcp.h"

#include "debug.h"
#include "lwip/dhcp.h"
#include "dhcpd.h"
#include "drivers.h"
#include "sys_misc.h"

#if MACHTALK_ENABLE

enum MACHTALK_STAT{
	MACHTALK_STAT_IDLE,
	MACHTALK_STAT_LOGIN_OK,
	MACHTALK_STAT_LOGIN_FAILD,
	MACHTALK_STAT_WORK,
};

char login_server_ip[] = "60.211.201.42";
uint16_t login_server_port = 7779;

char work_server_ip[32];
uint16_t work_server_port = 0;
int work_sock_fd = -1;

//如果使用自己注册的账号和设备，请修改下面这两项
char device_id[] = "d21083c82860428aaf1b749b9e608aa4";	//设备id
char device_pwd[] = "d25c89d413e17541d5ab6f49b3c31d57";	//pin码经过md5加密,原文63075668

char user_name[32] = {0};

int mid;

static bool led_stat = FALSE, key_stat = FALSE;
timer_t *key_detect_timer = NULL;

enum MACHTALK_STAT machtalk_stat = MACHTALK_STAT_IDLE;

extern timer_t *led4_timer, *led5_timer;

/**
 * @brief  判断是否是与machtalk通信的连接(通过端口号)
 * @param 
 * @retval 
 */
int is_machtalk_socket(int socket)
{
	uint16_t tmp_port = 0;
	struct lwip_sock *sock;

	sock = get_socket(socket);
	if (!sock || !sock->conn)
	{
		return 0;
	}

	if (sock->conn->type == NETCONN_TCP)
	{
		tmp_port = sock->conn->pcb.tcp->remote_port; //server

		if (tmp_port == login_server_port)
		{
			p_dbg("is login socket");
			return 1;
		}else if(tmp_port == work_server_port)
		{
			p_dbg("is work socket");
			return 2;
		}else
			return 0;
	}
	return 0;
}

/**
 * @brief  获取段值
 * @param 
 * @retval 
 */
int get_seg_value(char *buff, char *seg, char *value, int max_len)
{
    char *tmp, *tmp_end;;
    tmp = strstr(buff, seg);
    if(!tmp)
        return -1;

    tmp += strlen(seg) + 2;
    if(*tmp == '\"')
        tmp++;

    tmp_end = strstr(tmp, "\"");
    if(!tmp_end){
        tmp_end = strstr(tmp, ",");
        if(!tmp_end){
            tmp_end = strstr(tmp, "}");
            if(!tmp_end){
                p_err_fun;
                return -1;
            }
        }
    }

    if(tmp_end - tmp > (max_len - 1)){
        p_err("err value Lenth:%d", tmp_end - tmp);
        return -1;
    }
    max_len = tmp_end - tmp;
    memcpy(value, tmp, max_len);
    value[max_len] = 0;

    p_dbg("get seg:%s, value:%s", seg, value);
    return 0;
}


/**
 * @brief  处理用户的控制命令,这里就led开关
 * @param 
 * @retval 
 */
extern uint8_t led_value;
void handle_user_opt(int dvid, int value)
{
	//led 的dvid是1
	if(dvid == 1)
	{
		if(value){
			led_stat = TRUE;
			LED6_ON;
		}else{
			led_stat = FALSE;
			LED6_OFF;
		}
	}
}

/**
 * @brief  主动发送当前按键和led状态
 * @param 
 * @retval 
 */
 void send_new_stat()
{
	char *cmd_buff = (char*)mem_calloc(256, 1);

	if(!cmd_buff)
		return;
	
	sprintf(cmd_buff, "{\"cmd\": \"post\","
                "\"mid\": \"%d\","
                "\"to\": \"%s\","
                "\"values\": ["
                "{"
                "\"dvid\": \"%d\","
                "\"value\": \"%d\""
                "}"
                "{"
                "\"dvid\": \"%d\","
                "\"value\": \"%d\""
                "}"
                "]"
                "}\r\n", mid, user_name, 1, led_stat, 2, key_stat);

      p_dbg("send cmd:%s", cmd_buff);

      send(work_sock_fd, cmd_buff, strlen(cmd_buff), 0);

	  mem_free(cmd_buff);
}

/**
 * @brief  处理接收的数据
 * 不进行数据包重组,期望每次接收到的都是完整的一帧
 * @param 
 * @retval 
 */
void handle_machtalk_rcv(int socket, uint8_t *buff, int size)
{
	int socket_type;
    char value[64];
    
	if(size > /*TCP_RCV_SIZE*/1024)
		return;
	buff[size] = 0;	//tcp_rcv_buff大小比TCP_RCV_SIZE大4个字节，不必担心内存溢出
	p_dbg("rcv:%s", buff);
	socket_type = is_machtalk_socket(socket);
	if(socket_type != 1 && socket_type != 2)
	{
		p_err("err sock type:%d", socket_type);
		return;
	}
	
	if(buff[0] != '{')
	{
		buff[size] = 0;
		p_err("data err:%s", buff);
		return;
	}

	if(socket_type == 1)
	{
		int success;
        if(get_seg_value((char*)buff, "success", value, 64))
		{
			p_err_fun;
			return;
		}
		success = atoi(value);
		p_dbg("success:%d", success);
		if(!success)
		{
			p_err_fun;
			machtalk_stat = MACHTALK_STAT_LOGIN_FAILD;
			return;
		}

        if(get_seg_value((char*)buff, "mid", value, 64))
		{
			p_err_fun;
			return;
		}
		mid = atoi(value);
		p_dbg("mid:%d", mid);
		mid++;

        if(get_seg_value((char*)buff, "port", value, 64))
		{
			p_err_fun;
			return;
		}
		work_server_port = atoi(value);
		p_dbg("port:%d", work_server_port);

        if(get_seg_value((char*)buff, "ip", value, 64))
		{
			p_err_fun;
			return;
		}
		memcpy(work_server_ip, value, strlen(value));
		p_dbg("work_server_ip:%s", work_server_ip);
		
		machtalk_stat = MACHTALK_STAT_LOGIN_OK;

		mod_timer(led4_timer, 500);
		
	}

	if(socket_type == 2)
	{
		char *tmp;
		int i;
		tmp = strstr((char*)buff, ":");
		if(!tmp)
		{
			char cmd_buff[8];
			i = atoi((char*)buff + 1);
			p_dbg("heartbeat:%d", i);
			sprintf(cmd_buff, "{%d}\r\n", i + 1);
			send(work_sock_fd, cmd_buff, strlen(cmd_buff), 0);
			return;
		}

        if(get_seg_value((char*)buff, "mid", value, 64))
        {
             p_err_fun;
             return;
        }
        mid = atoi(value);
        p_dbg("mid:%d", mid);
	mid++;
        if(get_seg_value((char*)buff, "cmd", value, 64))
		{
			p_err_fun;
			return;
		}

        if(!strcmp(value, "query"))
        {
            char *cmd_buff;
            char from[32];
            
            if(get_seg_value((char*)buff, "from", from, 32))
    		{
    			p_err_fun;
    			return;
    		}

			strncpy(user_name, from, 32);

            if(get_seg_value((char*)buff, "to", value, 64))
    		{
    			p_err_fun;
    			return;
    		}

            cmd_buff = (char*)mem_calloc(256, 1);
            if(!cmd_buff)
            {
    			p_err_fun;
    			return;
    		}    

			key_stat = BUTTON_STAT?TRUE:FALSE;

            sprintf(cmd_buff, "{\"cmd\": \"resp\","
                "\"mid\": \"%d\","
                "\"from\": \"%s\","
                "\"to\": \"%s\","
                "\"success\": 1,"
                "\"values\": ["
                "{"
                "\"dvid\": \"1\","		//LED
                "\"value\": \"%d\""
        		"}"
				"{"
                "\"dvid\": \"2\","		//KEY
                "\"value\": \"%d\""
                "}"
                "]"
                "}\r\n", mid, value, from, led_stat, key_stat);

            p_dbg("send cmd:%s", cmd_buff);

            send(work_sock_fd, cmd_buff, strlen(cmd_buff), 0);

            mem_free(cmd_buff);
            return;
        }

        if(!strcmp(value, "opt"))
        {
            char *cmd_buff;
            char from[32];
            int dvid, int_value;
            
            if(get_seg_value((char*)buff, "from", from, 32))
    		{
    			p_err_fun;
    			return;
    		}

			strncpy(user_name, from, 32);

            if(get_seg_value((char*)buff, "dvid", value, 64))
    		{
    			p_err_fun;
    			return;
    		}
            dvid = atoi(value);

            if(get_seg_value((char*)buff, "value", value, 64))
    		{
    			p_err_fun;
    			return;
    		}
            int_value = atoi(value);

			handle_user_opt(dvid, int_value);

            //这个必须放在最后,value里面保存了to值
            if(get_seg_value((char*)buff, "to", value, 64))
    		{
    			p_err_fun;
    			return;
    		}

            cmd_buff = (char*)mem_calloc(256, 1);
            if(!cmd_buff)
            {
    			p_err_fun;
    			return;
    		}    
			
			key_stat = BUTTON_STAT?TRUE:FALSE;

            sprintf(cmd_buff, "{\"cmd\": \"post\","
                "\"mid\": \"%d\","
                "\"from\": \"%s\","
                "\"to\": \"%s\","
                "\"success\": 1,"
                "\"values\": ["
                "{"
                "\"dvid\": \"%d\","
                "\"value\": \"%d\""
                "},"
                "{"
                "\"dvid\": \"%d\","
                "\"value\": \"%d\""
                "}"
                "]"
                "}\r\n", mid, value, from, 1, led_stat, 2, key_stat);

            p_dbg("send cmd:%s", cmd_buff);

            send(work_sock_fd, cmd_buff, strlen(cmd_buff), 0);

            mem_free(cmd_buff);
            return;
        }
        
	}
		
}

extern int close_socket(uint32_t socket_num);
int reomote_control_exit()
{
    if(work_sock_fd == -1)
        return -1;

    send(work_sock_fd, "{\"cmd\":\"exit\"}", strlen("{\"cmd\":\"exit\"}"), 0);

    close_socket(work_sock_fd);
    work_sock_fd = -1;
    return 0;
}

/**
 * @brief  登陆,先连接登录服务器,再连接到其返回的工作服务器
 *与工作服务器的连接是长连接,建立连接之后将通过心跳维持
 * 
 * @param 
 * @retval 
 */
int reomote_control_login()
{
	int ret = -1;
	int fd;
	char *ip = login_server_ip;
	uint16_t port = login_server_port;
	char *cmd_buff = 0;
	p_dbg_enter;
	p_dbg("连接到:%s, 端口号:%d", ip, port);
	if (!is_wifi_connected())
	{

		p_err("wifi not connect");
		goto end;
	}

	fd = tcp_link(ip, port);
	if(fd < 0)
	{
		p_err("connect to login server err");
		goto end;
	}

	cmd_buff = (char*)mem_calloc(128, 1);
	if(!cmd_buff)
		goto end;

	snprintf(cmd_buff, 128, "{\"cmd\":\"login\","
		"\"mid\":\"1\","
		"\"type\":\"2\","
		"\"usr\":\"%s\","
		"\"pwd\":\"%s\"}\r\n", device_id, device_pwd);
	
	p_dbg("login cmd:%s", cmd_buff);
	machtalk_stat = MACHTALK_STAT_IDLE;
	send(fd, cmd_buff, strlen(cmd_buff), 0);

	fd = 100;
	while(fd--)
	{
		sleep(100);
		if(machtalk_stat != MACHTALK_STAT_IDLE)
			break;
	}

	if(machtalk_stat != MACHTALK_STAT_LOGIN_OK)
	{
		p_err("login faild:%d", machtalk_stat);
		goto end;
	}
	p_dbg("login ok, connect to work werver");
	machtalk_stat = MACHTALK_STAT_IDLE;
	work_sock_fd = tcp_link(work_server_ip, work_server_port);
	if(work_sock_fd < 0)
	{
		p_err("connect to work server err");
		goto end;
	}
	machtalk_stat = MACHTALK_STAT_WORK;

	sprintf(cmd_buff, "{\"cmd\":\"connect\","
		"\"mid\":\"2\","
		"\"type\":\"2\","
		"\"usr\":\"%s\","
		"\"encode\":\"\"}\r\n", device_id);
	p_dbg("connect cmd:%s", cmd_buff);
	send(work_sock_fd, cmd_buff, strlen(cmd_buff), 0);

	ret = 0;
end:
	if(cmd_buff)
		mem_free(cmd_buff);

	
	return ret;
}

/**
 * @brief   每秒检测一次按键状态,如有变化则发送一次
 *  服务器对post命令的频率限制为1HZ
 * @param 
 * @retval 
 */

static void key_detect(void *arg)
{
	static bool old_key_stat = FALSE;

	key_stat = BUTTON_STAT?TRUE:FALSE;

	if(old_key_stat != key_stat)
	{
		old_key_stat = key_stat;
		send_new_stat();
	}
}



int test_connect_to_machtalk_server()
{
	mod_timer(led4_timer, 300);
	reomote_control_login();
	if(key_detect_timer)
		del_timer(key_detect_timer);

	key_detect_timer = timer_setup(1000, 1, key_detect, NULL);
	if(key_detect_timer)
		add_timer(key_detect_timer);
	return 0;
}

#endif


