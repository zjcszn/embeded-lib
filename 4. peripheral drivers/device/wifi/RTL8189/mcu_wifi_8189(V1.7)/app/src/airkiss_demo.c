#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

#include <osdep_service.h>
#include <drv_types.h>
#include <xmit_osdep.h>
#include <recv_osdep.h>
#include <hal_intf.h>
#include <rtw_ioctl.h>
#include <rtw_version.h>


#include "airkiss_demo.h"
#include "airkiss.h"

#define WIFI_MAX_CHANNEL	14

static airkiss_result_t result;
uint8_t desire_ssid_crc = 0;
static char desire_ssid[33];

static airkiss_context_t *akcontex = NULL;
static timer_t *ch_sw_tmr = NULL;
static timer_t *key_detect_timer = NULL;
wait_event_t wait_complite;
static int cur_channel = 1;
static bool airkiss_working = FALSE;

const airkiss_config_t akconf = { 
(airkiss_memset_fn)&memset, 
(airkiss_memcpy_fn)&memcpy, 
(airkiss_memcmp_fn)&memcmp, 
(airkiss_printf_fn)&printf };

extern mutex_t socket_mutex;;
extern timer_t *led4_timer, *led5_timer;
extern int wifi_set_smart_link_recv_hook(smart_link_recv_hook hook);
void stop_airkiss(void);

static void scan_all_callback(BSS_TABLE_SIMPLE *Tab)
{
	int i = 0;
	if(Tab)
	{
		BSS_ENTRY_SIMPLE *p_bss_entry = Tab->BssEntry;
		if (p_bss_entry)
		{
			do
			{
				uint8_t crc;

				crc = calcrc_bytes(p_bss_entry->res_data.essid, p_bss_entry->res_data.essid_len);
				p_dbg("\r\n%d: %s", i++, p_bss_entry->res_data.essid);
				
				p_dbg(" mode:%d", p_bss_entry->res_data.mode);
				p_dbg(" auth:%d", p_bss_entry->res_data.auth);
				p_dbg(" rssi:%d", p_bss_entry->res_data.rssi);
				p_dbg(" channel:%d", p_bss_entry->res_data.channel);
				p_dbg(" ssid len:%d", p_bss_entry->res_data.essid_len);
				p_dbg(" crc:%x", crc);
				dump_hex("	mac", p_bss_entry->res_data.bssid, 6);
				
				if(crc == desire_ssid_crc)
				{
					p_dbg("find desire ssid:%s", p_bss_entry->res_data.essid);

					if(p_bss_entry->res_data.essid_len <= 32)
					{
						memcpy(desire_ssid, p_bss_entry->res_data.essid, p_bss_entry->res_data.essid_len);
						desire_ssid[p_bss_entry->res_data.essid_len] = 0;
						result.ssid = desire_ssid;
						result.ssid_length = p_bss_entry->res_data.essid_len;
						
						break;
					}
				}

				p_bss_entry = p_bss_entry->next;
			}while (p_bss_entry);
		}

	}

	wake_up(wait_complite);
}


static void scan_desire_callback(BSS_TABLE_SIMPLE *Tab)
{
	wake_up(wait_complite);
}

//wifi连接完成后发送UDP广播，通知用户
void airkiss_send_result_to_user(unsigned char random)
{
	int fd;
	int n = 1;
	int err;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(10000);
	addr.sin_addr.s_addr = IPADDR_BROADCAST;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		p_err("get socket err:%d", errno);
		return;
	} 

	err = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n));
	//p_dbg("SO_BROADCAST ret:%d", err);
	if(err == -1)
	{
		close(fd);
		return;
	}

	mutex_lock(socket_mutex);
	sendto(fd, (u8*)&random, 1, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
	mutex_unlock(socket_mutex);

	close(fd);
	//p_dbg_exit;
}

int wifi_scan(void (*scan_result_callback)(BSS_TABLE_SIMPLE *Tab), 
		char *essid);
//wifi连接完成后发送UDP广播，通知用户
void wifi_connnect_by_airkiss_result(airkiss_result_t *result)
{
	int i;
	int ret;
	int retry_cnt = 3;
	desire_ssid_crc = result->reserved;
	ret = ret;
	while(retry_cnt--)
	{
		wifi_scan(scan_all_callback, "");

		clear_wait_event(wait_complite);
		ret = wait_event_timeout(wait_complite, 5000);
		
		if(result->ssid_length && result->ssid)
		{
			wifi_scan(scan_desire_callback, result->ssid);
			wait_event_timeout(wait_complite, 5000);

			p_dbg("connect to %s, key:%s", result->ssid, result->pwd);
			wifi_set_mode(MODE_STATION);
			wifi_connect(result->ssid, result->pwd);

			i = 50;
			while(i--)
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
				for(i = 0;i < 100; i++){
					airkiss_send_result_to_user(result->random);
					sleep(10);
				}
			}
			break;
		
		}else	
			p_err("not find desire ap ,try again");
	}

	
}

void channel_switch_timer(void *arg)
{
	wifi_set_channel(cur_channel++);
	if(cur_channel > WIFI_MAX_CHANNEL){
		p_dbg("scan all channel");
		cur_channel = 1;
	}
}

void airkiss_recv_hook (void* ptr, unsigned int len)
{
	int ret;
	ret = airkiss_recv(akcontex, ptr, len);

	if(ret == AIRKISS_STATUS_CHANNEL_LOCKED)
	{
		if(ch_sw_tmr){
			timer_free(ch_sw_tmr);
			ch_sw_tmr = 0;
		}
	}
	else if(ret == AIRKISS_STATUS_COMPLETE)
	{
		wake_up(wait_complite);
	}
}

int start_airkiss()
{
	int ret;

	p_dbg_enter;
	akcontex = mem_malloc(sizeof(airkiss_context_t));
	if(!akcontex)
		return -1;
	ret = airkiss_init(akcontex, &akconf);
	ch_sw_tmr = timer_setup(100, 1, channel_switch_timer, NULL);
	if(!ch_sw_tmr)
		goto end;

	if(airkiss_working)
	{
		p_err("airkiss is pending");
		return -1;
	}
	wifi_disconnect();
	
	airkiss_working = TRUE;
	wifi_set_smart_link_recv_hook((smart_link_recv_hook)airkiss_recv_hook);
	wifi_set_promisc(TRUE);
	add_timer(ch_sw_tmr);
	mod_timer(led5_timer, 300); //led5开始闪烁
	
	clear_wait_event(wait_complite);
	ret = wait_event_timeout(wait_complite, 30000);

	if(ret == 0)
	{
		airkiss_get_result(akcontex, &result);
		wifi_set_promisc(FALSE);
		wifi_set_smart_link_recv_hook(NULL);
		
		p_dbg("get result, key:%s, ssid_crc:%x, random:%d", result.pwd, result.reserved, result.random);
		wifi_connnect_by_airkiss_result(&result);
	}
	stop_airkiss();
	
	return 0;
end:
	if(ch_sw_tmr)
		timer_free(ch_sw_tmr);
	ch_sw_tmr = NULL;
	
	if(akcontex)
		mem_free(akcontex);
	akcontex = NULL;
	return -1;
}


void stop_airkiss()
{
	p_dbg_enter;
	wifi_set_promisc(FALSE);
	wifi_set_smart_link_recv_hook(NULL);
	del_timer(led5_timer);
	LED5_OFF;
	
	if(ch_sw_tmr)
		timer_free(ch_sw_tmr);
	ch_sw_tmr = 0;
	
	if(akcontex)
		mem_free(akcontex);
	akcontex = NULL;
	
	airkiss_working = FALSE;
}

static void key_detect(void *arg)
{
	static int key_cnt = 0;
#if SUPPORT_AUDIO
	//打开音频时按键用于发送音频控制
	if(audio_cfg.audio_dev_open)
		return;
#endif
	if(BUTTON_STAT)
	{
		key_cnt = 0;
	}
	else
	{
		if(!airkiss_working)
		{
			if(key_cnt++ >= 3)
			{
				key_cnt = 0;
				send_cmd_to_self('p');
			}
		}
	}
}

int airkiss_demo_init()
{
	wait_complite = init_event();
	
	key_detect_timer = timer_setup(1000, 1, key_detect, NULL);
	if(key_detect_timer)
		add_timer(key_detect_timer);
	return 0;
}

void test_show_rcr()
{
	_adapter *padapter;
	u32 v;
	if(!wifi_dev)
		return;

	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);
	
	v = rtw_read32(padapter, 0x0608);

	p_dbg("rcr:%x", v);
}

