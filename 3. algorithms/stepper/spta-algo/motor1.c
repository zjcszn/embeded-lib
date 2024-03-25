#include "motor1.h"
#include "rtthread.h"
#include <math.h>
#include "rtconfig.h"
#include "motorapp.h"

#define DBG_TAG           "motor1"
#define DBG_LVL           DBG_LOG
#include <rtdbg.h> 

MOTOR1_CONTROL_SPTA motor1;

/**
  * @brief  Motor1(X轴深孔板电机) initialization.
  * @param  None.
  * @retval None.
  */
void motor1_init(void)
{
    motor1_io_init();
    motor1_TIMx_init();
    motor1_param_init();
}

/**
  * @brief  Motor1(X轴深孔板电机) IO port initialization.
  * @param  None.
  * @retval None.
  */
static void motor1_io_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);

    /* X轴深孔板电机: DIR_1: PA5, CP_1: PB6, EN_1: PA3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    MOTOR1_OUTPUT_ENABLE();

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_6);
}

/**
  * @brief  Motor1(X轴深孔板电机) TIMx initialization.
  * @param  None.
  * @retval None.
  */
static void motor1_TIMx_init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

    /* 基本环境配置 */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = 29;
    TIM_TimeBaseStructure.TIM_Prescaler = 14;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* 中断配置 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 使能定时器中断 */
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE);

    /* 关闭定时器4 */
    TIM_Cmd(TIM4, DISABLE);
}

/**
  * @brief  Motor1(X轴深孔板电机)  param initialization.
  * @param  None.
  * @retval None.
  */
static void motor1_param_init(void)
{
    rt_memset(&motor1,0,sizeof(motor1));
    MOTOR1_OUTPUT_ENABLE();    //电机使能
    motor1.clockwise=MOTOR1_CW;       //顺时针对应的方向
    motor1.TIMx=TIM4;
    motor1.divnum=1;            //细分
    motor1.GPIOBASE=GPIOB;
    motor1.PWMGPIO=GPIO_Pin_6;   //PWM-GPIO
    motor1.MaxPosition=10000;   //最大位置,按照细分前的步数统计
    motor1.MaxPosition_Pulse=(10000*1);
}

/**
  * @brief  Motor1(X轴深孔板电机)  走绝对位置.
  * @param  position: 绝对位置,按照细分前的步数统计
  * @param  accel: 加减速
  * @param  speed: 最大速度
  * @retval None.
  */
void box_moveabs(int32_t position,uint32_t accel,uint32_t speed)
{
    MOTOR1_CONTROL_SPTA *pmotor = &motor1;

    /*
                          0
    |_____________________|____|      //我们使用绝对位置时,还是按照零点为0来看待,程序会进行处理
                        零点

                         350
    |_____________________|____|      //实际程序中的零点坐标是350
                        零点
    */
    position += 350;   //为了走零点右边的地方,零点的当前为止设置为350
    
    /* 步数为0 */
    if ((position - pmotor->CurrentPosition) == 0)
    {
        return;
    }
    /* 步数不为0 */
    if(position < pmotor->MaxPosition )
    {
        box_moverel(position - pmotor->CurrentPosition,accel,speed);
    }
}


/**
  * @brief  Motor1(X轴深孔板电机)  走相对位置.
  * @param  step: 步数,按照细分前的步数统计,(正数为左移，负数为右移).
  * @param  accel: 加减速
  * @param  speed: 最大速度
  * @retval None.
  */
void box_moverel(int32_t step,uint32_t accel,uint32_t speed)
{
    MOTOR1_CONTROL_SPTA *pmotor = &motor1;

    if(step < 0) // 步数为负数
    {
        pmotor->dir=MOTOR1_CCW;      // 逆时针方向旋转
        MOTOR1_DIR_CCW();
        step =-step;   // 获取步数绝对值
    }
    else if(step  == 0)
    {
        return;
    }
    else 
    {
        pmotor->dir=MOTOR1_CW; // 顺时针方向旋转
        MOTOR1_DIR_CW();
    }

    pmotor->running=MOTOR1_RUNNING;    //转动完成标志
    pmotor->speedenbale=0;  //关闭速度控制
    pmotor->step_move=step*(pmotor->divnum);   //移动的步数
    pmotor->step_middle=pmotor->step_move>>1;
    pmotor->step_spmax=speed;   //最大速度
    pmotor->step_accel=accel;   //加速度
    pmotor->step_state=MOTOR1_ACCELERATING;
    pmotor->step_frac=0;
    pmotor->speed_frac=0;
    pmotor->step_acced=0;
    pmotor->step_speed=0;
    pmotor->step_count=0;

    if(experiment_ctrl != E_PROTV1_CONTROL_PAUSE)
    {
        TIM_Cmd(pmotor->TIMx,ENABLE);
    }
}

/**
  * @brief  Motor1(X轴深孔板电机) 电机SPTA算法公共处理函数.
  * @param  pmotor:电机结构体
  * @retval None.
  */
void TIM4_IRQHandler_SPTA(MOTOR1_CONTROL_SPTA *pmotor)
{
    int carry=0;

    /*拉低脉冲信号*/
    pmotor->GPIOBASE->BRR = pmotor->PWMGPIO;

    /*根据速度累加器是否溢出，决定是否产生一个步进脉冲*/
    pmotor->step_frac += pmotor->step_speed;
    carry = pmotor->step_frac >> 16;
    pmotor->step_frac -= carry << 16;
    //carry=1说明溢出
    if(carry!=0)
    {
        //溢出则产生一个脉冲
        pmotor->step_count+=1;
        /*拉高脉冲信号产生一个步进脉冲*/
        pmotor->GPIOBASE->BSRR = pmotor->PWMGPIO;

        //产生脉冲，则需要位置计算，根据方向调整位置
        if(pmotor->clockwise==pmotor->dir)
        {
            pmotor->CurrentPosition_Pulse++;
            if(pmotor->CurrentPosition_Pulse>=pmotor->MaxPosition_Pulse)
            {
                pmotor->CurrentPosition_Pulse=0;
            }
        }
        else
        {
            pmotor->CurrentPosition_Pulse--;
            if(pmotor->CurrentPosition_Pulse==0xffffffff)
            {
                pmotor->CurrentPosition_Pulse=pmotor->MaxPosition_Pulse-1;
            }
        }
        pmotor->CurrentPosition=pmotor->CurrentPosition_Pulse/pmotor->divnum;
    }

    //速度控制-已关闭
    if(pmotor->speedenbale)
    {
        if( (pmotor->step_speed>=pmotor->step_spmax&&pmotor->step_speed-pmotor->step_spmax<=3)||
                (pmotor->step_speed<=pmotor->step_spmax&&pmotor->step_spmax-pmotor->step_speed<=3))
        {
            return;
        }
    }

    /*根据电机的状态进行状态转换以及参数变换*/
    switch(pmotor->step_state)
    {
    /* 加速 */
    case MOTOR1_ACCELERATING:
        //计算加速阶段的步数
        if(carry) 
        {
            pmotor->step_acced++;
        }
        //加速阶段，速度累加器要根据设定的加速度值进行累加
        pmotor->speed_frac+=pmotor->step_accel;
        //溢出判断
        carry=pmotor->speed_frac>>17;
        pmotor->speed_frac-=carry<<17;
        if(carry) 
        {
            //如果速度累加器溢出，则步数速度器需要加上该值，以便推动
				//步数速度器的增长，为步数累加器溢出准备
            pmotor->step_speed+=carry;
        }
        if(!pmotor->speedenbale)
        {
            //到达了中值就要反转为减速
            if(pmotor->step_middle!=0) 
            {
                if(pmotor->step_count==pmotor->step_middle)
                {
                    pmotor->step_state=MOTOR1_DECELERATING;
                }
            } 
            else if(pmotor->step_count>0) 
            {
                pmotor->step_state=MOTOR1_DECELERATING;
            }
        }

        if(pmotor->step_speed>=pmotor->step_spmax)
        {
            //加速阶段到达一定的时刻，就达到了设定的最大速度
            pmotor->step_speed=pmotor->step_spmax;
            //转为最大速度状态
            pmotor->step_state=MOTOR1_AT_MAX;
        }
        break;

    /* 达到最大速度 */
    case MOTOR1_AT_MAX:
        if((pmotor->step_move - pmotor->step_count) <= pmotor->step_acced)
        {
            pmotor->step_state=MOTOR1_DECELERATING;
        }
        break;

    /* 减速 */
    case MOTOR1_DECELERATING:
        //减速阶段与加速阶段是相反的过程，原理也相同
        if(carry&&pmotor->step_acced>0) 
        {
            pmotor->step_acced--;
        }
        pmotor->speed_frac+=pmotor->step_accel;
        carry=pmotor->speed_frac>>17;
        pmotor->speed_frac-=carry<<17;

        if(carry&&pmotor->step_speed>carry) 
        {
            pmotor->step_speed-=carry;
        }
        if(!pmotor->speedenbale)
        {
            //在运行设定步数以后停止运行
            if(pmotor->step_count>=pmotor->step_move)
            {
                pmotor->step_state=MOTOR1_IDLE;
            }
        }
        break;

    /* 停止 */
    case MOTOR1_IDLE:
        TIM_Cmd(pmotor->TIMx, DISABLE);
        pmotor->running=MOTOR1_STOP;
        pmotor->step_spmax=0;
    }
}

/**
  * @brief  Adjust the timer to generate a pulse signal, and the Motor1(X轴深孔板电机) rotates.
  * @param  None.
  * @retval None.
  */
void TIM4_IRQHandler(void)
{
    /*
    请不要在此处加入多余的代码,那怕是if判断语句!!!
    此处只打开了一种中断类型,所以无需判断!!!
    加入任何多余的代码,都会影响电机运动的实时性
    */
    TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);
    TIM4_IRQHandler_SPTA(&motor1);
}

#if defined(RTE_USING_FINSH)

static void box(__IO int32_t step, __IO uint32_t accel, __IO uint32_t speed)
{
    LOG_D("motor1.CurrentPosition = %d",motor1.CurrentPosition);
    box_moveabs(step,accel,speed);
    AWAIT_BOX_STOP();
    LOG_D("motor1.CurrentPosition = %d",motor1.CurrentPosition);
}


#include "finsh.h"
FINSH_FUNCTION_EXPORT_ALIAS(box, box, box);

#endif














