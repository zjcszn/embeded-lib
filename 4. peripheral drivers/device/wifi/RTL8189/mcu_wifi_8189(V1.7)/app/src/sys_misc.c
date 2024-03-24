#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "lwip/netif.h"
#include <absacc.h>

timer_t *led4_timer, *led5_timer;

struct SAVE_CFG save_cfg;

void delay_1us()
{
	int i = 25;
	while (i--)
		;
}

void delay_us(uint32_t us)
{
	while (us--)
		delay_1us();
}

uint8_t led_value = 0;
uint8_t led_bright_value = 10;

void led_switch(uint8_t value)
{
	p_dbg("led_switch:%x", value);
	led_value = value;
}

void led_bright(uint8_t value)
{
	p_dbg("led_bright:%d", value);
	if (value > 99)
		value = 99;

	led_bright_value = value / 10;
}

/*
在SysTick_Handler里面调用，每1ms调用一次，用于刷新led状态
 */
void led_scan()
{
	static int led_bright_cnt = 0;

	if (led_bright_cnt++ >= 10)
		led_bright_cnt = 0;

	if (led_bright_cnt == 0 && led_value)
	{
#if !USE_I2S_MODE
		if (led_value &(1 << 0))
			GPIO1_SET;
		if (led_value &(1 << 1))
			GPIO2_SET;
		if (led_value &(1 << 2))
			GPIO3_SET;
#endif
		if (led_value &(1 << 3))
			GPIO4_SET;
		if (led_value &(1 << 4))
			GPIO5_SET;
		if (led_value &(1 << 5))
			GPIO6_SET;
	}

	if (led_bright_cnt == led_bright_value)
	{
#if !USE_I2S_MODE
		GPIO1_CLR;
		GPIO2_CLR;
		GPIO3_CLR;
#endif
		GPIO4_CLR;
		GPIO5_CLR;
		GPIO6_CLR;
	}
}


SWITCH_EVENT g_switch = SWITCH_EVENT_OFF;
void button_stat_callback()
{
	static uint8_t last_stat = 1;
	BUTTON_EVENT button_event = BUTTON_EVENT_IDLE;

	if (BUTTON_STAT && !last_stat)
	{
		last_stat = BUTTON_STAT;
		button_event = BUTTON_EVENT_UP;
		GPIO6_SET;
		//p_dbg("button up");
	}

	if (!BUTTON_STAT && last_stat)
	{
		last_stat = BUTTON_STAT;
		button_event = BUTTON_EVENT_DOWN;
		GPIO6_CLR;
		//p_dbg("button down");
	}

	if (button_event == BUTTON_EVENT_UP)
	{
		if(g_switch == SWITCH_EVENT_OFF)
			g_switch = SWITCH_EVENT_ON;
		else
			g_switch = SWITCH_EVENT_ON;
#if SUPPORT_AUDIO
		//camera_button_event(g_switch);
		audio_button_event(g_switch);
#endif
	}
}


void assert_failed(uint8_t *file, uint32_t line)
{
	p_err("assert_failed in:%s,line:%d \n", file ? file : "n", line);
	while (1)
		;
}

void led4_timer_callback(void *arg)
{
	static bool led_stat = FALSE;

	led_stat = !led_stat;

	if(led_stat)
		LED4_ON;
	else
		LED4_OFF;
}
void led5_timer_callback(void *arg)
{
	static bool led_stat = FALSE;

	led_stat = !led_stat;

	if(led_stat)
		LED5_ON;
	else
		LED5_OFF;
}


/**
 * @brief 定义两个定时器用于led的闪烁(只是初始化，还没开启)
 */
void misc_init()
{
	led4_timer = timer_setup(500, 1, led4_timer_callback, NULL);
	led5_timer = timer_setup(500, 1, led5_timer_callback, NULL);
}

void show_sys_info()
{

	p_dbg("mac:%02x-%02x-%02x-%02x-%02x-%02x", p_netif->hwaddr[0], p_netif->hwaddr[1], p_netif->hwaddr[2], p_netif->hwaddr[3], p_netif->hwaddr[4], p_netif->hwaddr[5]);
	show_tcpip_info();
}

void soft_reset()
{
	p_err("system well reset\n");
	NVIC_SystemReset();
  	__DSB();  
	while (1);
}

void dump_os_info()
{
	p_err("current task info");
	p_err("OSTCBStkPtr:%x", OSTCBCur->OSTCBStkPtr);
#if OS_TASK_CREATE_EXT_EN > 0u
	p_err("OSTCBStkBottom:%x", OSTCBCur->OSTCBStkBottom);
	p_err("OSTCBStkSize:%d", OSTCBCur->OSTCBStkSize);
	p_err("OSTCBId:%x", OSTCBCur->OSTCBId);
#endif
#if OS_TASK_NAME_EN > 0u
    p_err("OSTCBTaskName:%s", OSTCBCur->OSTCBTaskName);
#endif

	p_err("\r\nlast task info");
	p_err("OSTCBStkPtr:%x", OSTCBCur->OSTCBPrev->OSTCBStkPtr);
#if OS_TASK_CREATE_EXT_EN > 0u
	p_err("OSTCBStkBottom:%x", OSTCBCur->OSTCBPrev->OSTCBStkBottom);
	p_err("OSTCBStkSize:%d", OSTCBCur->OSTCBPrev->OSTCBStkSize);
	p_err("OSTCBId:%x", OSTCBCur->OSTCBPrev->OSTCBId);
#endif
#if OS_TASK_NAME_EN > 0u
    p_err("OSTCBTaskName:%s", OSTCBCur->OSTCBPrev->OSTCBTaskName);
#endif

	p_err("\r\nnext task info");
	p_err("OSTCBStkPtr:%x", OSTCBCur->OSTCBNext->OSTCBStkPtr);
#if OS_TASK_CREATE_EXT_EN > 0u
	p_err("OSTCBStkBottom:%x", OSTCBCur->OSTCBNext->OSTCBStkBottom);
	p_err("OSTCBStkSize:%d", OSTCBCur->OSTCBNext->OSTCBStkSize);
	p_err("OSTCBId:%x", OSTCBCur->OSTCBNext->OSTCBId);
#endif
#if OS_TASK_NAME_EN > 0u
    p_err("OSTCBTaskName:%s", OSTCBCur->OSTCBNext->OSTCBTaskName);
#endif
	
}

void HardFault_Handler()
{
	static int once = 0;
	if (!once)
	{
		once = 1;
		mem_slide_check(0);
	}
	p_err("%s\n", __FUNCTION__);
	
	dump_os_info();
	while (1)
		;
}


#if OS_APP_HOOKS_EN > 0u
void App_TaskCreateHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskDelHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskReturnHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TCBInitHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskSwHook(void){

}

void App_TimeTickHook(void){}

//uC/OS-II Stat线程中调用此函数，每100MS一次
void App_TaskStatHook()
{
	#if USE_MEM_DEBUG
	mem_slide_check(0);
	#endif
	//button_stat_callback();
}

#endif
extern uint32_t drvdbg;
extern uint32_t drvdbg_save;
void switch_dbg(void)
{
	if(dbg_level)
	{
		p_dbg("close dbg");
	}
	dbg_level = !dbg_level;
	if(dbg_level)
	{
		p_dbg("open dbg");
#ifdef DEBUG_LEVEL1
		drvdbg = drvdbg_save;
#endif
	}
#ifdef DEBUG_LEVEL1	
	else
		drvdbg = 0;
#endif
}

void usr_gpio_init()
{
	gpio_cfg((uint32_t)BUTTON_PORT, BUTTON_PIN, GPIO_Mode_IPU);

#if !USE_I2S_MODE
	GPIO1_OUTPUT;
	GPIO2_OUTPUT;
	GPIO3_OUTPUT;
#endif
	GPIO4_OUTPUT;
	GPIO5_OUTPUT;
	GPIO6_OUTPUT;

	LED4_OFF;
	LED5_OFF;
	LED6_OFF;
}

uint8_t get_check_sum(void *buff, int len)
{
	uint8_t *p_buff = buff, sum = 0;
	int i;
	for(i = 0 ; i < len; i++)
		sum = sum^p_buff[i];
	return sum;
}

void make_cfg_sum()
{
	save_cfg.check_sum = get_check_sum(&save_cfg, sizeof(struct SAVE_CFG) - 1);
}

int is_cfg_available()
{
	int i;
	uint8_t *p_buff = (uint8_t *)&save_cfg;
	p_dbg("enter %s\n", __FUNCTION__);
	for(i = 0; i < sizeof(struct SAVE_CFG); i++)	
	{
		//如果只有0和ff两种数据可以认为是无效的
		if((p_buff[i] != 0) && (p_buff[i] != 0xff))
			break;
	}
	if(i == sizeof(struct SAVE_CFG))
		return 0;
	i = (save_cfg.check_sum == get_check_sum(&save_cfg, sizeof(struct SAVE_CFG) - 1));
	p_dbg("ret %d\n", i);
	return i;
}

static void load_default_cfg()
{
	p_dbg("enter %s\n", __FUNCTION__);
	memset(&save_cfg, 0,  sizeof(struct SAVE_CFG));

	sprintf((char*)save_cfg.link.essid, "LCK");
	sprintf((char*)save_cfg.link.key, "ckxr1314");
	//ADD MORE
}

int _save_cfg()
{
	int ret;
	struct SAVE_CFG *tmp = 0;
	p_dbg("enter %s\n", __FUNCTION__);
	make_cfg_sum();

	tmp = (struct SAVE_CFG*)mem_malloc(sizeof(struct SAVE_CFG));

	if(tmp)
	{
		m25p80_read(SAVE_CFG_ADDR, sizeof(struct SAVE_CFG), (u_char*)tmp);
		if(memcmp(tmp, &save_cfg, sizeof(struct SAVE_CFG)) == 0)
		{
			p_dbg("no need to write flash");
			return 0;
		}
	}
		
	ret = m25p80_erase(SAVE_CFG_ADDR, sizeof(struct SAVE_CFG));
	if(!ret)
	{
		ret = m25p80_write(SAVE_CFG_ADDR, sizeof(struct SAVE_CFG), (const u_char*)&save_cfg);
	}
	
	if(ret)
		p_err("save err");

	if(tmp)
		mem_free(tmp);

	return ret;
}


struct SAVE_CFG* read_cfg()
{
	m25p80_read(SAVE_CFG_ADDR, sizeof(struct SAVE_CFG), (u_char*)&save_cfg);
	return &save_cfg;
}

void load_cfg()
{
	int need_clear = 0;
	p_dbg("enter %s\n", __FUNCTION__);
//	memcpy(&web_cfg, (void*)SAVE_CFG_ADDR, sizeof(struct SAVE_CFG));
	m25p80_read(SAVE_CFG_ADDR, sizeof(struct SAVE_CFG), (u_char*)&save_cfg);

	if((!is_cfg_available()) || need_clear)
	{
		load_default_cfg();
		_save_cfg();
	}
}


