#ifndef __MOTOR1_H
#define __MOTOR1_H
#include "stm32f10x.h"
#include "rtthread.h"

typedef __packed struct 
{
    uint8_t dir;			  	//方向
    uint8_t running;		  	//转动完成标志
    uint8_t divnum;		  		//分频数	 
    uint8_t speedenbale;		//是否使能速度控制	
    uint8_t clockwise;			//顺时针方向对应的值

    // uint32_t PulsesGiven;			  		//电机运动的总步数
	// uint32_t PulsesHaven;					//电机已经运行的步数
    uint32_t step_move;				//总共需要移动的步数
    uint32_t step_spmax;			//移动过程中的最大速度
    uint32_t step_accel;			//加速度/减速度
    uint32_t step_acced;			//加速阶段的加速步数

    uint32_t step_middle;				//移动的中点, = (step_move - 1) >> 1
    uint32_t step_count;				//已经运行的步数
    uint32_t step_frac;				    //步数计数器分数,
    uint32_t step_speed;				//当前速度, 16.8 bit format (HI byte always 0)
    uint32_t speed_frac;				//速度片段累加器，每次都累加step_accel，该数值大于某个值后step_speed增加
    uint8_t step_state;					//运动状态

    uint32_t CurrentPosition;			//当前位置,按照细分前的步数统计
    uint32_t MaxPosition;				//最大位置,超过该位置置0,按照细分前的步数统计
    uint32_t CurrentPosition_Pulse;		//当前位置
    uint32_t MaxPosition_Pulse;			//最大位置

    TIM_TypeDef* TIMx;                  //电机使用的定时器
    GPIO_TypeDef * GPIOBASE;            
    int32_t PWMGPIO;                    //产生PWM波的引脚
}MOTOR1_CONTROL_SPTA;

//运动状态
#define MOTOR1_IDLE				0
#define MOTOR1_ACCELERATING		1
#define MOTOR1_AT_MAX			2
#define MOTOR1_DECELERATING		3
#define MOTOR1_STOP             0
#define MOTOR1_RUNNING          1

//电机使能
#define MOTOR1_OUTPUT_ENABLE()             GPIO_ResetBits(GPIOA,GPIO_Pin_3)
#define MOTOR1_OUTPUT_DISABLE()            GPIO_SetBits(GPIOA,GPIO_Pin_3)

//电机方向 
#define MOTOR1_CCW          0         //右
#define MOTOR1_CW           1         //左
#define MOTOR1_DIR_CCW()	GPIO_ResetBits(GPIOA,GPIO_Pin_5)
#define MOTOR1_DIR_CW()		GPIO_SetBits(GPIOA,GPIO_Pin_5)


#define BOX_MOVE_ZERO  350     //深孔板零点移动位置
#define BOX_MOVE_MAX   10000   //深孔板最大移动位置
/* 等待电机到位,此宏定义主要用于校准的时候使用 */
#define AWAIT_BOX_STOP()       {while(motor1.step_state != MOTOR1_IDLE)\
                                {\
                                    rt_thread_mdelay(50);\
                                }}
/* 等待电机到位,并判断实验是否退出,此宏定义主要用于实验的时候使用 */
#define WAIT_BOX_STOP()        {while(motor1.step_state != MOTOR1_IDLE)\
                                {\
                                    if(running_state == E_RUNNING_STATUSV2_IDLE)\
                                    {\
                                        return E_RUNNING_STATUSV2_IDLE;\
                                    }\
                                    rt_thread_mdelay(20);\
                                }}


extern MOTOR1_CONTROL_SPTA motor1;

extern void motor1_init(void);
extern void motor1_io_init(void);
extern void motor1_TIMx_init(void);
extern void motor1_param_init(void);
extern void box_moveabs(int32_t position,uint32_t accel,uint32_t speed);
extern void box_moverel(int32_t step,uint32_t accel,uint32_t speed);



#endif
