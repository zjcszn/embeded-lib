#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "dhcpd.h"
#include "webserver.h"
#include "airkiss_demo.h"
#include <cctype>

/**
 *开发板版本说明
 *
 *V1.0	初始版本
 *V1.1  修复对wep加密方式的支持
 *V1.2  修复对tkip加密方式的支持,消除大部分部分编译警告
 *V1.3  修复对tkip/aes混合加密方式的支持，现在已经能支持所有加密方式
 *V1.4  修复搜索时返回频道不对的问题
 *V1.5  修复airkiss测试无法连服务器问题
 *V1.6  修复aes_decipher函数内存泄露的问题
 *V1.7  修复在调用wifi_disconnect之后无法再次进入混杂模式的问题(参考wifi_set_promisc)
 */


#define VIRSION		"V1.7"

wait_event_t command_event = 0;
char command = 0;

extern timer_t *led4_timer, *led5_timer;

#define help_string   \
"测试功能:\r\n"\
"	0:基本信息\r\n"\
"	1:扫描\r\n"\
"	2:连接WIFI\r\n"\
"	3:关闭WIFI连接\r\n"\
"	4::创建AP\r\n"\
"	5:连接TCP\r\n"\
"	6:关闭TCP连接\r\n"\
"	7:send发送\r\n"\
"	8:建立TCP服务器\r\n"\
"	9:关闭TCP服务器\r\n"\
"	a:启用音频功能\r\n"\
"	b:关闭音频功能\r\n"\
"	c:dns测试\r\n"\
"	d:自动获取IP地址\r\n"\
"	e:打印内存信息\r\n"\
"	f:获取wifi连接信息\r\n"\
"	g:连接UDP\r\n"\
"	h:加入多播组\r\n"\
"	i:建立UDP服务器\r\n"\
"	j:sendto发送\r\n"\
"	k:切换音频模式(mp3/pcm)\r\n"\
"	l:调节音量\r\n"\
"	m:DEBUG输出\r\n"\
"	n:\r\n"\
"	o:\r\n"\
"	p:airkiss+machtalk测试\r\n"\
"	q:\r\n"\
"	r:使能省电模式\r\n"\
"	s:关闭省电模式\r\n"\
"	t:monitor输出\r\n"\
"	u:测试TCP全速发送\r\n"\
"	v:结束TCP全速发送\r\n"\
"	w:回发测试开关\r\n"\
"	x:重启\r\n"\
"	y:测试machtalk远程控制\r\n"\
"	z:进入airkiss配置模式\r\n"\
"	?:帮助\r\n"\
"请输入上面的命令选择测试的功能:"


void print_help()
{
	p_dbg(help_string);
}

extern test_create_adhoc(void);
void handle_cmd(char cmd)
{
	cmd = tolower(cmd);
	switch (cmd)
	{
		case '0':
			show_sys_info();
			break;
		case '1':
			test_scan();
			break;
		case '2':
			test_wifi_connect();
			break;
		case '3':
			test_wifi_disconnect();
			break;
		case '4':
			test_create_ap();
			break;
		case '5':
			test_tcp_link();
			break;
		case '6':
			test_tcp_unlink();
			break;
		case '7':
			test_send("this is test data");
			break;
		case '8':
			test_tcp_server();
			break;
		case '9':
			test_close_tcp_server();
			break;
#if SUPPORT_AUDIO			
		case 'a':
			test_open_audio();
			break;
		case 'b':
			test_close_audio();
			break;
#endif			
		case 'c':
			test_dns("www.baidu.com");
			break;
		case 'd':
			test_auto_get_ip();
			break;
		case 'e':
			mem_slide_check(1);
			break;
		case 'f':
			test_wifi_get_stats();
			break;
		case 'g':
			test_udp_link();
			break;
		case 'h':
			test_multicast_join();
			break;
		case 'i':
			test_udp_server();
			break;
		case 'j':
			test_sendto("this is test data");
			break;
#if SUPPORT_AUDIO
		case 'k':
			switch_audio_mode();
			break;
		case 'l':
			if(audio_cfg.volume > 0)
				audio_cfg.volume -= 10;
			else
				audio_cfg.volume = 100;
			p_dbg("当前音量:%d", audio_cfg.volume);
			break;
#endif
		case 'm':
			switch_dbg();
			break;
		case 'n':
			test_wifi_connect1();
			break;
		case 'o':
			wifi_set_promisc(TRUE);
			break;
		case 'p':
			custom_test_demo();
			break;
		case 'q':
			break;
		case 'r':
			test_power_save_enable();
			break;
		case 's':
			test_power_save_disable();
			break;
		case 't':
			monitor_switch();
			break;
		case 'u':
			test_full_speed_send();
			break;
		case 'v':
			test_full_speed_send_stop();
			break;
		case 'w':
			switch_loopback_test();
			break;
		case 'x':
			soft_reset();
			break;
		case 'y':
#if MACHTALK_ENABLE
			test_connect_to_machtalk_server();
#endif
			break;
		case 'z':
			start_airkiss();
			break;
		case '?':
			print_help();
			break;
		default:
			p_err("unkown cmd");
			break;
	}
}

void main_thread(void *pdata)
{
	int ret;
#ifdef DEBUG
	RCC_ClocksTypeDef RCC_ClocksStatus;
#endif
	
	driver_misc_init(); //初始化一些杂项(驱动相关)
	usr_gpio_init(); //初始化GPIO
	LED6_ON; //点亮LED6

	OSStatInit(); //初始化UCOS状态

	uart1_init(); //初始化串口1

	//打印MCU总线时钟
#ifdef DEBUG
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	p_dbg("SYSCLK_Frequency:%d,HCLK_Frequency:%d,PCLK1_Frequency:%d,PCLK2_Frequency:%d,ADCCLK_Frequency:%d\n", 
		RCC_ClocksStatus.SYSCLK_Frequency, 
		RCC_ClocksStatus.HCLK_Frequency, 
		RCC_ClocksStatus.PCLK1_Frequency, 
		RCC_ClocksStatus.PCLK2_Frequency, 
		0);
#endif

	m25p80_init(); //初始化SPI-flash
#if SUPPORT_AUDIO
	audio_dev_init(); //初始化音频驱动的数据结构
#endif
	init_work_thread();	//初始化工作线程

    	misc_init(); //初始化一些杂项(app相关)

	ret = SD_Init();    //初始化SDIO设备
		
	if (!ret){
		ret = init_wifi();//初始化WIFI芯片
	}

	if(ret){//网卡初始化失败,没法再往下跑了
		p_err("wifi card faild");  
       		mod_timer(led4_timer, 100); //LED4开始闪烁
		while(1)sleep(1000);    
	}

   	 init_monitor(); //初始化monitor模块,必须在init_wifi之后调用

	 load_cfg();

	init_lwip(); //初始化lwip协议栈

	init_udhcpd(); //初始化dhcp服务器

	enable_dhcp_server(); // 开启dhcp服务器,如果工作在sta模式,可以不开启dhcpserver

	airkiss_demo_init(); //预先初始化airkiss

	command_event = init_event(); //初始化一个等待事件

	p_dbg("代码版本:%s", VIRSION);
	p_dbg("启动时间%d.%d S", os_time_get() / 1000, os_time_get() % 1000);
	
/*
 *下面启动开发板预设的功能
 *开启web服务器, 端口号:80
 *开启TCP服务器，端口号:4800
 *连接到路由器，路由器名称和密码可通过airkiss配置
 *
*/
#if	SUPPORT_WEBSERVER
	web_server_init();
#endif
	test_tcp_server();
	//test_wifi_connect();
	
	print_help();

	LED6_OFF; 

	while (1)
	{
		//等待用户命令
		wait_event(command_event);

		//执行命令
		handle_cmd(command);
	}
}

int main(void)
{

	OSInit();

	_mem_init(); //初始化内存分配

	thread_create(main_thread, 0, TASK_MAIN_PRIO, 0, TASK_MAIN_STACK_SIZE, "main_thread");

	OSStart();
	return 0;
}
