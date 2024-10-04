#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include <absacc.h>

extern char Image$$RW_IRAM1$$ZI$$Limit[];
void _mem_init(void)
{
	uint32_t malloc_start, malloc_size;

	malloc_start = (uint32_t) &Image$$RW_IRAM1$$ZI$$Limit; //取空闲内存最低地址
	malloc_start = (malloc_start + 3) & (~0x03);// ALIGN TO word

//0x20000为板子内存总大小，更换MCU时需注意
	malloc_size = 0x20000000 + 0x20000 - malloc_start;

	sys_meminit((void*)malloc_start, malloc_size);
}

void init_systick()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	SysTick_Config(SystemCoreClock / (OS_TICKS_PER_SEC *10));
}

void init_rng()
{
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}

/**
 *读一个随机数
 */
uint32_t get_random(void)
{
	return RNG_GetRandomNumber();
}

void gpio_cfg(uint32_t group, uint32_t pin, uint8_t mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = pin;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	if (mode == GPIO_Mode_AIN)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else if (mode == GPIO_Mode_IN_FLOATING)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_IPD)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	}
	else
	if (mode == GPIO_Mode_IPU)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	}
	else
	if (mode == GPIO_Mode_Out_OD)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_Out_PP)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_AF_OD)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_AF_PP)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	}
	else
	if (mode == GPIO_Mode_AF_IF)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_AF_IPU)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	}
	GPIO_Init((GPIO_TypeDef*)group, &GPIO_InitStructure);
}

void driver_gpio_init()
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); //外部中断需要用到

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);

}


int check_rst_stat()
{
	uint32_t stat;
	stat = RCC->CSR;
	RCC->CSR = 1L << 24; //清除复位标志

	p_err("reset:");
	if (stat &(1UL << 31))
	// 低功耗复位
	{
		p_err("low power\n");
	}
	if (stat &(1UL << 30))
	//窗口看门狗复位
	{
		p_err("windw wdg\n");
	}
	if (stat &(1UL << 29))
	//独立看门狗复位
	{
		p_err("indep wdg\n");
	}
	if (stat &(1UL << 28))
	//软件复位
	{
		p_err("soft reset\n");
	}
	if (stat &(1UL << 27))
	//掉电复位
	{
		p_err("por reset\n");
	}
	if (stat &(1UL << 26))
	//rst复位
	{
		p_err("user reset\n");
	}

	return 0;
}

void driver_misc_init()
{
	driver_gpio_init();
	init_systick();
	init_rng();
}

