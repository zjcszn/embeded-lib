#define DEBUG
#include "debug.h"
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "airkiss_demo.h"

#include "dhcpd.h"

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_byteorder.h>

#include <hal_intf.h>
#include <lwip/netif.h>

int wifi_get_stats(uint8_t *mac, struct station_info *sinfo);
int wifi_scan(void (*scan_result_callback)(BSS_TABLE_SIMPLE *Tab), 
		char *essid);

void print_scan_result(struct wlan_network *dat)
{
	char *cipher = "";
	char *mode;
	
	char essid_name[33];

	if(dat->BcnInfo.encryp_protocol == ENCRYP_PROTOCOL_OPENSYS)
		cipher = "NONE";
	else
	{
		if(dat->BcnInfo.encryp_protocol == ENCRYP_PROTOCOL_WEP)
			cipher = "WEP";
		if(dat->BcnInfo.encryp_protocol == ENCRYP_PROTOCOL_WPA)
			cipher = "WPA";
		if(dat->BcnInfo.encryp_protocol == ENCRYP_PROTOCOL_WPA2)
			cipher = "WPA2";
	}	

	if(dat->network.InfrastructureMode == Ndis802_11IBSS)
		mode = "AD-HOC";
	else if(dat->network.InfrastructureMode == Ndis802_11Infrastructure)
		mode = "AP";
	else
		mode = "unkown";
	
	memcpy(essid_name, dat->network.Ssid.Ssid,32);
	essid_name[32] = 0;
	
	p_dbg("	信号强度:%ddbm",dat->network.Rssi);
	p_dbg("	加密模式:%s", cipher);
	p_dbg("	模式:%s", mode);	
	p_dbg("	ssid:%s",essid_name);
	dump_hex("	bssid",dat->network.MacAddress, 6);
}


void scan_done_callback(BSS_TABLE_SIMPLE *Tab)
{
	int i = 0;
	if(Tab)
	{
		BSS_ENTRY_SIMPLE *p_bss_entry = Tab->BssEntry;
		if (p_bss_entry)
		{
			do
			{
				p_dbg("\r\n%d: %s", i++, p_bss_entry->res_data.essid);
				
				p_dbg(" mode:%d", p_bss_entry->res_data.mode);
				p_dbg(" auth:%d", p_bss_entry->res_data.auth);
				p_dbg(" rssi:%d", p_bss_entry->res_data.rssi);
				p_dbg(" channel:%d", p_bss_entry->res_data.channel);
				dump_hex("	mac", p_bss_entry->res_data.bssid, 6);
				p_bss_entry = p_bss_entry->next;
			}while (p_bss_entry);
		}

	}
}

/*
*测试扫描,命令'1'
*/
void test_scan()
{
	p_dbg_enter;
	wifi_scan(scan_done_callback, "");
	p_dbg_exit;
}

/**
 *测试WIFI连接,命令'2'
 *连接到名字为"LCK”的路由器，加密模式和频道自动适应
 *密码长度在WPA或WPA2模式下8 <= len <= 64;在WEP模式下必须为5或13
 */
void test_wifi_connect()
{
	char *essid = save_cfg.link.essid;
	char *password = save_cfg.link.key;
	int connect_timeout = 50;
	p_dbg_enter;

	p_dbg("连接到:%s, 密码:%s", essid, password);

	if (is_wifi_connected())
		wifi_disconnect();

	wifi_set_mode(MODE_STATION);

	wifi_scan(NULL, essid);

	sleep(2000);

	wifi_connect(essid, password);

	while(connect_timeout--)
	{
		if (is_wifi_connected())
		{
			break;
		}
		sleep(100);
	}
	
	if (is_wifi_connected())
	{
		p_dbg("wifi connect ok");
	}
	p_dbg_exit;

}

void test_wifi_connect1()
{
	char *essid = /*save_cfg.link.essid*/"note";
	char *password = /*save_cfg.link.key*/"ckxr1314";
	int connect_timeout = 50;
	p_dbg_enter;

	p_dbg("连接到:%s, 密码:%s", essid, password);

	if (is_wifi_connected())
		wifi_disconnect();

	wifi_set_mode(MODE_STATION);

	wifi_scan(NULL, essid);

	sleep(2000);

	wifi_connect(essid, password);

	while(connect_timeout--)
	{
		if (is_wifi_connected())
		{
			break;
		}
		sleep(100);
	}
	
	if (is_wifi_connected())
	{
		p_dbg("wifi connect ok");
	}
	p_dbg_exit;

}

/*
 *测试断开WIFI连接,命令'3'
 *
 */
void test_wifi_disconnect()
{
	p_dbg_enter;
	wifi_disconnect();
	p_dbg_exit;
}



/*
 *建立AP,命令'4'（AP功能尚未完善）
 *
 */
void test_create_ap()
{
	char *essid = "xrf_ap";
	char *password = "12345678";
	p_dbg_enter;
	p_dbg("建立ap:%s, 密码:%s", essid, password);

	wifi_set_mode(MODE_AP);

	wifi_ap_cfg(essid, password, KEY_WPA2, 6, 4);

	p_dbg_exit;
}


/*
 *测试获取WIFI连接信息,因可能有多个客户端连接,所以需要指定其地址
 *这里从test_get_station_list里面取最后一个mac地址作为测试
 *所以执行此测试之前请先执行test_get_station_list
 *,命令'f'
 */
uint8_t sta_mac[8] = {0,0,0,0,0,0};
void test_wifi_get_stats()
{
	int ret;
	struct station_info sinfo;
	
	ret = wifi_get_stats(sta_mac, &sinfo);

	if(ret == 0)
		p_dbg("wifi stats, rssi:%d", sinfo.signal);
	else
		p_err("test_wifi_get_stats err");
}

void test_power_save_enable()
{
	p_dbg("enter power save");
	wifi_power_cfg(1);
}

void test_power_save_disable()
{
	p_dbg("exit power save");
	wifi_power_cfg(0);
}

/*
*客户测试范例，包括如下内容
*1. 启动airkiss等待用户配置路由器ssid和密码(通过airkissdemo或者微信)
*2. 路由器ssid和密码配置成功且路由器成功连接，则自动获取IP地址
*3. 连接machtalk远程服务器
*4. 用户通过machtalk_remote_contrl软件通过远程控制开发板的led
*/
void custom_test_demo()
{
	int ret;
	ret = start_airkiss();
	if(ret || !is_wifi_connected()){
		p_err("airkiss config faild");
		return;
	}
	//此时WIFI已经连接上， 开始获取到ip地址
	test_auto_get_ip();
	if (!p_netif->ip_addr.addr)
	if(ret){
		p_err("auto get ip faild");
		return;
	}

	//开始machtalk测试
	test_connect_to_machtalk_server();
}

