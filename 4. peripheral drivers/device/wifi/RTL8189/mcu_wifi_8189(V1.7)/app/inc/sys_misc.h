#ifndef _MISC_H
#define _MISC_H

#if !USE_I2S_MODE
#define GPIO1_GROUP	GPIOA
#define GPIO2_GROUP	GPIOC
#define GPIO3_GROUP	GPIOC
#endif
#define GPIO4_GROUP	GPIOB
#define GPIO5_GROUP	GPIOB
#define GPIO6_GROUP	GPIOB

#if !USE_I2S_MODE
#define GPIO1_PIN	GPIO_Pin_7
#define GPIO2_PIN	GPIO_Pin_4
#define GPIO3_PIN	GPIO_Pin_5
#endif
#define GPIO4_PIN	GPIO_Pin_0
#define GPIO5_PIN	GPIO_Pin_1
#define GPIO6_PIN	GPIO_Pin_2

extern uint8_t led_value;

#define LED4_ON 		do{GPIO_SET(GPIO4_GROUP, GPIO4_PIN); led_value |= (1 << 3);}while(0)
#define LED4_OFF 	do{GPIO_CLR(GPIO4_GROUP, GPIO4_PIN); led_value &= ~(1 << 3);}while(0)

#define LED5_ON 		do{GPIO_SET(GPIO5_GROUP, GPIO5_PIN); led_value |= (1 << 4);}while(0)
#define LED5_OFF 	do{GPIO_CLR(GPIO5_GROUP, GPIO5_PIN); led_value &= ~(1 << 4);}while(0)

#define LED6_ON 		do{GPIO_SET(GPIO6_GROUP, GPIO6_PIN); led_value |= (1 << 5);}while(0)
#define LED6_OFF 	do{GPIO_CLR(GPIO6_GROUP, GPIO6_PIN); led_value &= ~(1 << 5);}while(0)


#if !USE_I2S_MODE
#define GPIO1_OUTPUT	gpio_cfg((uint32_t)GPIO1_GROUP, GPIO1_PIN, GPIO_Mode_Out_PP);
#define GPIO1_INPUT		gpio_cfg((uint32_t)GPIO1_GROUP, GPIO1_PIN, GPIO_Mode_IPU);
#define GPIO1_SET 		GPIO_SET(GPIO1_GROUP,GPIO1_PIN)
#define GPIO1_CLR 		GPIO_CLR(GPIO1_GROUP,GPIO1_PIN)

#define GPIO2_OUTPUT	gpio_cfg((uint32_t)GPIO2_GROUP, GPIO2_PIN, GPIO_Mode_Out_PP);
#define GPIO2_INPUT		gpio_cfg((uint32_t)GPIO2_GROUP, GPIO2_PIN, GPIO_Mode_IPU);
#define GPIO2_SET 		GPIO_SET(GPIO2_GROUP,GPIO2_PIN)
#define GPIO2_CLR 		GPIO_CLR(GPIO2_GROUP,GPIO2_PIN)

#define GPIO3_OUTPUT	gpio_cfg((uint32_t)GPIO3_GROUP, GPIO3_PIN, GPIO_Mode_Out_PP);
#define GPIO3_INPUT		gpio_cfg((uint32_t)GPIO3_GROUP, GPIO3_PIN, GPIO_Mode_IPU);
#define GPIO3_SET 		GPIO_SET(GPIO3_GROUP,GPIO3_PIN)
#define GPIO3_CLR 		GPIO_CLR(GPIO3_GROUP,GPIO3_PIN)
#endif

#define GPIO4_OUTPUT	gpio_cfg((uint32_t)GPIO4_GROUP, GPIO4_PIN, GPIO_Mode_Out_PP);
#define GPIO4_INPUT		gpio_cfg((uint32_t)GPIO4_GROUP, GPIO4_PIN, GPIO_Mode_IPU);
#define GPIO4_SET 		GPIO_SET(GPIO4_GROUP,GPIO4_PIN)
#define GPIO4_CLR 		GPIO_CLR(GPIO4_GROUP,GPIO4_PIN)

#define GPIO5_OUTPUT	gpio_cfg((uint32_t)GPIO5_GROUP, GPIO5_PIN, GPIO_Mode_Out_PP);
#define GPIO5_INPUT		gpio_cfg((uint32_t)GPIO5_GROUP, GPIO5_PIN, GPIO_Mode_IPU);
#define GPIO5_SET 		GPIO_SET(GPIO5_GROUP,GPIO5_PIN)
#define GPIO5_CLR 		GPIO_CLR(GPIO5_GROUP,GPIO5_PIN)


#define GPIO6_OUTPUT	gpio_cfg((uint32_t)GPIO6_GROUP, GPIO6_PIN, GPIO_Mode_Out_PP);
#define GPIO6_INPUT		gpio_cfg((uint32_t)GPIO6_GROUP, GPIO6_PIN, GPIO_Mode_IPU);
#define GPIO6_SET 		GPIO_SET(GPIO6_GROUP,GPIO6_PIN)
#define GPIO6_CLR		GPIO_CLR(GPIO6_GROUP,GPIO6_PIN)


#define BUTTON_PORT		GPIOA
#define BUTTON_PIN			GPIO_Pin_1
#define BUTTON_SOURCE		GPIO_PinSource1
#define BUTTON_EXTI_LINE 	EXTI_Line1

#define BUTTON_STAT 	GPIO_STAT(BUTTON_PORT, BUTTON_PIN)

typedef enum
{
	BUTTON_EVENT_IDLE, BUTTON_EVENT_UP, BUTTON_EVENT_DOWN, BUTTON_EVENT_TOGGLE_ON, BUTTON_EVENT_TOGGLE_OFF
} BUTTON_EVENT;

typedef enum
{
	SWITCH_EVENT_OFF, SWITCH_EVENT_ON
} SWITCH_EVENT;

#define SAVE_CFG_ADDR    			0

struct SAVE_CFG_LINK{

	char		essid[33];
	char		key[33];
}__packed;

struct SAVE_CFG{
	struct SAVE_CFG_LINK		link;		//wiifÅäÖÃ
	//add more
	char 	check_sum;
}__packed;

extern struct SAVE_CFG save_cfg;

void usr_gpio_init(void);
void init_systick(void);
void misc_init(void);
void led_switch(uint8_t value);
void led_bright(uint8_t value);
void button_stat_callback(void);

void soft_reset(void);
void delay_1us(void);
void delay_us(uint32_t us);
void load_cfg(void);

#endif
