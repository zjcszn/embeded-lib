#define DEBUG
#include "debug.h"
#include "drivers.h"
#include "stdio.h"
#include "api.h"
#include "lwip/stats.h"
#include "lwip/sockets.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/arch/sys_arch.h"
#include <absacc.h>

#if SYS_MONITOR == 0
bool monitor_enable = 0;
#else
bool monitor_enable = 1;
#endif

int dbg_level = 1;


void monitor_switch()
{
	monitor_enable = !monitor_enable;
	if(monitor_enable)
		p_dbg("打开monitor");
	else
		p_dbg("关闭monitor");
}

void send_test_pkg(int cmd, const void *data, int size)
{
	static uint8_t g_seq = 0;
	uint8_t *pkg;
	uint8_t *p_data = (uint8_t*)data;
	int i;
	uint8_t sum = 0;
	if ((size > 1024) || (size && (p_data == 0)))
	{
		return ;
	}

	pkg = (uint8_t*)mem_malloc(size + 7);
	if (pkg == 0)
	{
		p_err("send_pkg mem_malloc err, size:%d\n", size + 7);
		return ;
	}
	pkg[0] = 0xaa;
	pkg[1] = (uint8_t)cmd;
	pkg[2] = (uint8_t)((size + 7) % 256);
	pkg[3] = (uint8_t)((size + 7) / 256);
	pkg[4] = (uint8_t)g_seq++;
	sum ^= pkg[1];
	sum ^= pkg[2];
	sum ^= pkg[3];
	sum ^= pkg[4];
	for (i = 0; i < size; i++)
	{
		pkg[i + 5] = p_data[i];
		sum ^= p_data[i];
	}
	pkg[i + 5] = sum;
	pkg[i + 6] = 0x55;

	uart1_send(pkg, i + 7);

	mem_free(pkg);
}


#pragma section = "monitor_item_table"

char *p_global_buff = 0;
#define GLOBAL_TEST_BUFF_SIZE 1024

typedef enum
{
	STAT_TYPE_SYS = 1, STAT_TYPE_TASK, STAT_TYPE_WIFI, STAT_TYPE_SOCKET, STAT_TYPE_MAX_VALUE
} STAT_TYPE;


DECLARE_MONITOR_ITEM("monitor item example", monitor_send_cnt);

char *err = "test_buff err";

#define CHECK_AND_ADD_BUFF	{	\
tmp[127] = 0;\
cnt += strlen(tmp);\
if(cnt > (GLOBAL_TEST_BUFF_SIZE - 64)){\
p_err("%s l:%d len:%d, tmp_len:%d ", err, __LINE__, cnt , strlen(tmp));\
goto end;\
}\
strcat(p_global_buff, tmp);\
}

#define CHECK_AND_ADD_LINE	{	\
sprintf(tmp, "\r\n");	\
tmp[127] = 0;\
cnt += strlen(tmp);\
if(cnt > (GLOBAL_TEST_BUFF_SIZE - 64)){\
p_err("%s l:%d len:%d, tmp_len:%d ", err, __LINE__, cnt , strlen(tmp));\
goto end;\
}\
strcat(p_global_buff, tmp);\
}


extern const uint32_t monitor_item_table$$Base;
extern const uint32_t monitor_item_table$$Limit;

extern char STACK$$Base[];
#define STACK_SIZE 0X200

uint32_t get_msp_size()
{
	return (uint32_t)STACK_SIZE;
}

uint32_t get_msp_free()
{
	int i;
	uint32_t *stack_base = (uint32_t*)STACK$$Base;
	uint32_t stack_size = STACK_SIZE;
	stack_size = stack_size / 4;
	for (i = 0; i < stack_size; i++)
	{
		if (stack_base[i])
			break;
	}

	return i *4;
}


void send_sys_stat_data()
{
	char *tmp;
	int cnt = 0, i;
	uint32_t start, end;

	tmp = (char*)mem_malloc(128);
	if (!tmp)
		return ;

	INC_MONITOR_ITEM_VALUE(monitor_send_cnt);

	send_test_pkg(STAT_TYPE_SYS, 0, 0);

	memset(p_global_buff, 0, GLOBAL_TEST_BUFF_SIZE);

	snprintf(tmp, 127, "CPU使用率:%u\r\n", OSCPUUsage);
	CHECK_AND_ADD_BUFF;

	snprintf(tmp, 127, "进程切换数:%u\r\n", OSCtxSwCtr);
	CHECK_AND_ADD_BUFF;


	snprintf(tmp, 127, "进程数:%d/%d\r\n", OSTaskCtr, OS_MAX_TASKS);
	CHECK_AND_ADD_BUFF;

	for (i = 0; i < OS_MAX_EVENTS; i++)
	{
		if (OSEventFreeList == (OSEventTbl + i))
			break;
	}
	snprintf(tmp, 127, "event used:%d/%d\r\n", i, OS_MAX_EVENTS);
	CHECK_AND_ADD_BUFF;

	snprintf(tmp, 127, "timer used:%d/%d\r\n", OSTmrUsed, OS_TMR_CFG_MAX);
	CHECK_AND_ADD_BUFF;

	snprintf(tmp, 127, "main stack free:%d/%d\r\n", get_msp_free(), get_msp_size());
	CHECK_AND_ADD_BUFF;

	for (i = 0; i < OS_MAX_QS; i++)
	{
		if (OSQFreeList == (OSQTbl + i))
			break;
	}
	snprintf(tmp, 127, "qs used:%d/%d\r\n", i, OS_MAX_QS);
	CHECK_AND_ADD_BUFF;

	snprintf(tmp, 127, "运行时长:%d s\r\n", os_time_get() / 1000);
	CHECK_AND_ADD_BUFF;

	CHECK_AND_ADD_LINE;

	p_global_buff[cnt] = 0;
	send_test_pkg(STAT_TYPE_SYS, p_global_buff, cnt + 1);

	for (i = 0; i < OS_MAX_QS; i++)
	{
		if (OSQFreeList == (OSQTbl + i))
			break;
	}

	start = (uint32_t) &monitor_item_table$$Base;
	end = (uint32_t) &monitor_item_table$$Limit;

	cnt = 0;
	memset(p_global_buff, 0, GLOBAL_TEST_BUFF_SIZE);

	for (; start < end; start += sizeof(struct dev_monitor_item))
	{
		struct dev_monitor_item *item = (struct dev_monitor_item*)start;

		//logd("%s:%d\r\n", item->name, item->value);
		snprintf(tmp, 127, "%s:%d\r\n", item->name, item->value);
		CHECK_AND_ADD_BUFF;

		p_global_buff[cnt] = 0;

		if (cnt > 512)
		{
			send_test_pkg(STAT_TYPE_SYS, p_global_buff, cnt + 1);
			memset(p_global_buff, 0, GLOBAL_TEST_BUFF_SIZE);
			cnt = 0;
		}
	}

	if (cnt)
	{
		send_test_pkg(STAT_TYPE_SYS, p_global_buff, cnt + 1);
	}
	
#if OS_EVENT_NAME_EN
	for (i = 0; i < OS_MAX_EVENTS; i++)
	{
		struct dev_monitor_item *item = (struct dev_monitor_item*)start;
		
		if(OSEventTbl[i].OSEventType == OS_EVENT_TYPE_SEM && 
			OSEventTbl[i].OSEventName[0] != '?')
		{
			//logd("%s:%d\r\n", item->name, item->value);
			snprintf(tmp, 127, "%s:%d\r\n", OSEventTbl[i].OSEventName, OSEventTbl[i].OSEventCnt);
			CHECK_AND_ADD_BUFF;

			p_global_buff[cnt] = 0;
			
			if (cnt > 512)
			{
				send_test_pkg(STAT_TYPE_SYS, p_global_buff, cnt + 1);
				memset(p_global_buff, 0, GLOBAL_TEST_BUFF_SIZE);
				cnt = 0;
			}
		}
	}
	if (cnt)
	{
		send_test_pkg(STAT_TYPE_SYS, p_global_buff, cnt + 1);
	}
#endif

end: 
	mem_free(tmp);
}


void send_task_stat_data()
{
	OS_TCB *ptcb = 0,  *tmp_tcp = 0;
	INT8U prio;
	uint32_t irq_flag;
	char *tmp;
	int cnt = 0;

	tmp = (char*)mem_malloc(128);
	if (!tmp)
		goto end;

	send_test_pkg(STAT_TYPE_TASK, 0, 0);

	tmp_tcp = (OS_TCB*)mem_malloc(sizeof(OS_TCB));
	if (!tmp_tcp)
		goto end;

	for (prio = 0; prio <= OS_TASK_IDLE_PRIO; prio++)
	{
		ptcb = OSTCBPrioTbl[prio];
		if (ptcb != (OS_TCB*)0)
		{
			if (ptcb != OS_TCB_RESERVED)
			{
				memset(p_global_buff, 0, GLOBAL_TEST_BUFF_SIZE);

				irq_flag = local_irq_save();
				*tmp_tcp =  *ptcb;
				local_irq_restore(irq_flag);
				cnt = 0;

				snprintf(tmp, 127, "%s\r\n", ptcb->OSTCBTaskName);
				CHECK_AND_ADD_BUFF;

				snprintf(tmp, 127, "  ID号:%d\r\n", tmp_tcp->OSTCBId);
				CHECK_AND_ADD_BUFF;
				snprintf(tmp, 127, "  优先级:%d\r\n", tmp_tcp->OSTCBPrio);
				CHECK_AND_ADD_BUFF;

				snprintf(tmp, 127, "  栈底地址0x%08x\r\n", tmp_tcp->OSTCBStkBottom);
				CHECK_AND_ADD_BUFF;
				snprintf(tmp, 127, "  堆栈大小%d WORD\r\n", tmp_tcp->OSTCBStkSize);
				CHECK_AND_ADD_BUFF;
				snprintf(tmp, 127, "  已使用堆栈%d WORD\r\n", tmp_tcp->OSTCBStkUsed / 4);
				CHECK_AND_ADD_BUFF;

				snprintf(tmp,127, "  线程切换 :%d\r\n",tmp_tcp->OSTCBCtxSwCtr);
				CHECK_AND_ADD_BUFF;
				 
				p_global_buff[cnt] = 0;
				send_test_pkg(STAT_TYPE_TASK, p_global_buff, cnt + 1);
			}
		}
	}
	end: if (tmp_tcp)
		mem_free(tmp_tcp);
	if (tmp)
		mem_free(tmp);
}


extern struct lwip_sock sockets[NUM_SOCKETS];

#define send_lwip_stat(X)	memset(p_global_buff, 0, GLOBAL_TEST_BUFF_SIZE);	\
snprintf(tmp,127, ""###X##"\r\n  xmit:%d,recv:%d,drop:%d,memerr:%d,chkerr:%d,lenerr:%d,rterr:%d,proterr:%d,opterr:%d,err:%d\r\n", \
lwip_stats.X.xmit,lwip_stats.X.recv,lwip_stats.X.drop,lwip_stats.X.memerr,lwip_stats.X.chkerr,lwip_stats.X.lenerr,lwip_stats.X.rterr,lwip_stats.X.proterr,lwip_stats.X.opterr,lwip_stats.X.err);\
CHECK_AND_ADD_BUFF;\
send_test_pkg(STAT_TYPE_SOCKET, p_global_buff, cnt + 1);

void send_socket_stat_data()
{
	int i, cnt = 0;
	char *tmp;
	tmp = (char*)mem_malloc(128);
	if (!tmp)
		goto end;

	send_test_pkg(STAT_TYPE_SOCKET, 0, 0);
	#if LINK_STATS
	send_lwip_stat(link);
	#endif
	#if ETHARP_STATS
	send_lwip_stat(etharp);
	#endif
	#if IPFRAG_STATS
	send_lwip_stat(ip_frag);
	#endif
	#if IP_STATS
	send_lwip_stat(ip);
	#endif
	#if ICMP_STATS
	send_lwip_stat(icmp);
	#endif
	#if UDP_STATS
	send_lwip_stat(udp);
	#endif
	#if TCP_STATS
	send_lwip_stat(tcp);
	#endif

	for (i = 0; i < MEMP_NUM_NETCONN; i++)
	{

		if (sockets[i].conn)
		{
			memset(p_global_buff, 0, GLOBAL_TEST_BUFF_SIZE);

			CHECK_AND_ADD_LINE;

			snprintf(tmp, 127, "socket:%d\r\n", i);
			CHECK_AND_ADD_BUFF;

			snprintf(tmp, 127, "  send_window:%d\r\n", sockets[i].conn->pcb.tcp->snd_wnd);
			CHECK_AND_ADD_BUFF;

			snprintf(tmp, 127, "  send_bufsize:%d\r\n", sockets[i].conn->pcb.tcp->snd_buf);
			CHECK_AND_ADD_BUFF;

			snprintf(tmp, 127, "  recv_window:%d\r\n", sockets[i].conn->pcb.tcp->rcv_wnd);
			CHECK_AND_ADD_BUFF;

			send_test_pkg(STAT_TYPE_SOCKET, p_global_buff, cnt + 1);
		}
	}
end: 
	if (tmp)
		mem_free(tmp);
}


void handle_sys_monitor()
{
	send_sys_stat_data();
	send_task_stat_data();
	send_socket_stat_data();
}


void dev_monitor_task(void *arg)
{
	sleep(2000);
	p_global_buff = (char*)mem_malloc(GLOBAL_TEST_BUFF_SIZE);
	monitor_enable = 0;
	while(1)
	{
		sleep(1000);
		if(monitor_enable){
			handle_sys_monitor();
		}
	}
}



