#include "motor1.h"
#include "rtthread.h"
#include <math.h>

#define DBG_TAG           "motor1"
#define DBG_LVL           DBG_LOG
#include <rtdbg.h> 

MOTOR1_speedRampData MOTOR1_srd = {0};
__IO int32_t  MOTOR1_step_position     = 0;           //当前位置
__IO uint8_t  MOTOR1_MotionStatus      = 0;   //是否在运动？0：停止，1：运动

/**
  * @brief  Motor1(X轴深孔板电机) initialization.
  * @param  None.
  * @retval None.
  */
void motor1_init(void)
{
    motor1_io_init();
    motor1_TIMx_init();
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
	MOTOR1_DIR_CW();
    MOTOR1_OUTPUT_ENABLE();

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
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
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

    /* 基本环境配置 */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = MOTOR1_TIM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* 比较输出配置 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;   //比较输出模式：反转输出
    TIM_OCInitStructure.TIM_Pulse = 0xFFFF;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);

    /* 中断配置 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 失能比较输出通道 */
    TIM_CCxCmd(TIM4,TIM_Channel_1,DISABLE);
    /* 使能定时器中断 */
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    TIM_ITConfig(TIM4,TIM_IT_CC1, ENABLE);

    /* 关闭定时器4 */
    TIM_Cmd(TIM4, DISABLE);
}

/**
  * @brief  Motor1(X轴深孔板电机) 相对位置移动.
  * @param  step:移动的步数 (正数为左移，负数为右移).
  * @param  accel:加速度
  * @param  decel:减速度
  * @param  speed:最大速度
  * @retval None.
  */
void motor1_moverel(__IO int32_t step, __IO uint32_t accel, __IO uint32_t decel, __IO uint32_t speed)
{
    __IO uint16_t tim_count;
    // 达到最大速度时的步数
    __IO uint32_t max_s_lim;
    // 必须要开始减速的步数（如果加速没有达到最大速度）
    __IO uint32_t accel_lim;

    if(step < 0) // 步数为负数
    {
        MOTOR1_srd.dir = MOTOR1_CCW; // 逆时针方向旋转
        MOTOR1_DIR_CCW();
        step =-step;   // 获取步数绝对值
    }
    else
    {
        MOTOR1_srd.dir = MOTOR1_CW; // 顺时针方向旋转
        MOTOR1_DIR_CW();
    }

    if(step == 1)    // 步数为1
    {
        MOTOR1_srd.accel_count = -1;   // 只移动一步
        MOTOR1_srd.run_state = MOTOR1_DECEL;  // 减速状态.
        MOTOR1_srd.step_delay = 1000;	// 短延时
    }
    else if(step != 0)  // 如果目标运动步数不为0
    {
        // 设置最大速度极限, 计算得到min_delay用于定时器的计数器的值。
        MOTOR1_srd.min_delay = (int32_t)(MOTOR1_A_T_x10/speed);

        // 通过计算第一个(c0) 的步进延时来设定加速度，其中accel单位为0.1rad/sec^2
        MOTOR1_srd.step_delay = (int32_t)((MOTOR1_T1_FREQ_148 * sqrt(MOTOR1_A_SQ / accel))/10);

        // 计算多少步之后达到最大速度的限制
        max_s_lim = (uint32_t)(speed*speed/(MOTOR1_A_x200*accel/10));
        // 如果达到最大速度小于0.5步，我们将四舍五入为0
        // 但实际我们必须移动至少一步才能达到想要的速度
        if(max_s_lim == 0) {
            max_s_lim = 1;
        }

        // 计算多少步之后我们必须开始减速
        accel_lim = (uint32_t)(step*decel/(accel+decel));
        // 我们必须加速至少1步才能才能开始减速.
        if(accel_lim == 0) {
            accel_lim = 1;
        }

        // 使用限制条件我们可以计算出减速阶段步数
        if(accel_lim <= max_s_lim) {
            MOTOR1_srd.decel_val = accel_lim - step;
        }
        else {
            MOTOR1_srd.decel_val = -(max_s_lim*accel/decel);
        }
        // 当只剩下一步我们必须减速
        if(MOTOR1_srd.decel_val == 0) {
            MOTOR1_srd.decel_val = -1;
        }

        // 计算开始减速时的步数
        MOTOR1_srd.decel_start = step + MOTOR1_srd.decel_val;

        // 如果最大速度很慢，我们就不需要进行加速运动
        if(MOTOR1_srd.step_delay <= MOTOR1_srd.min_delay) {
            MOTOR1_srd.step_delay = MOTOR1_srd.min_delay;
            MOTOR1_srd.run_state = MOTOR1_RUN;
        }
        else {
            MOTOR1_srd.run_state = MOTOR1_ACCEL;
        }
        // 复位加速度计数值
        MOTOR1_srd.accel_count = 0;
    }
    else if(step == 0) //防止开关定时器,电机抖动造成位置偏移.
    {
        return ;
    }
    MOTOR1_MotionStatus = 1; // 电机为运动状态
    tim_count=TIM_GetCounter(TIM4);
    TIM_SetCompare1(TIM4,tim_count+MOTOR1_srd.step_delay);   //设置定时器比较值
    TIM_CCxCmd(TIM4,TIM_Channel_1,ENABLE);   //使能定时器通道
    //MOTOR1_OUTPUT_ENABLE();   //使能电机引脚
    /* 开启定时器4 */
    TIM_Cmd(TIM4, ENABLE);
}

/**
  * @brief  Motor1(X轴深孔板电机) 绝对位置移动.
  * @param  absstep:绝对位置位置.
  * @param  accel:加速度.
  * @param  decel:减速度.
  * @param  speed:最大速度.
  * @retval None.
  */
void motor1_moveabs(__IO int32_t absstep, __IO uint32_t accel, __IO uint32_t decel, __IO uint32_t speed)
{
    //位置计算:绝对位置-当前位置.
    motor1_moverel(absstep-MOTOR1_step_position, accel, decel, speed);
}

/**
  * @brief  Adjust the timer to generate a pulse signal, and the Motor1(X轴深孔板电机) rotates.
  * @param  None.
  * @retval None.
  */
void TIM4_IRQHandler(void)
{
    __IO uint16_t tim_count=0;
    // 保存新（下）一个延时周期
    uint16_t new_step_delay=0;
    // 加速过程中最后一次延时（脉冲周期）.
    __IO static uint16_t last_accel_delay=0;
    // 总移动步数计数器
    __IO static uint32_t step_count = 0;
    // 记录new_step_delay中的余数，提高下一步计算的精度
    __IO static int32_t rest = 0;
    //定时器使用翻转模式，需要进入两次中断才输出一个完整脉冲
    __IO static uint8_t i=0;

    rt_interrupt_enter();
    
    if(TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
    {
        //清除定时器中断
        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
        
        //设置比较值
        tim_count=TIM_GetCounter(TIM4);
        TIM_SetCompare1(TIM4,tim_count+MOTOR1_srd.step_delay);

        i++;     // 定时器中断次数计数值
        if(i==2) // 2次，说明已经输出一个完整脉冲
        {
            i=0;   // 清零定时器中断次数计数值
            switch(MOTOR1_srd.run_state) // 加减速曲线阶段
            {
            case MOTOR1_STOP:
                step_count = 0;  // 清零步数计数器
                rest = 0;        // 清零余值
                // 关闭通道
                TIM_CCxCmd(TIM4,TIM_Channel_1,DISABLE);
                TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
                /* 关闭定时器4 */
                TIM_Cmd(TIM4, DISABLE);
                //MOTOR1_OUTPUT_DISABLE();   // 电机使能引脚关闭
                MOTOR1_MotionStatus = 0;  // 电机为停止状态
                break;

            case MOTOR1_ACCEL:
                step_count++;      // 步数加1
                if(MOTOR1_srd.dir==MOTOR1_CW)
                {
                    MOTOR1_step_position++; // 绝对位置加1
                }
                else
                {
                    MOTOR1_step_position--; // 绝对位置减1
                }
                MOTOR1_srd.accel_count++; // 加速计数值加1
                new_step_delay = MOTOR1_srd.step_delay - (((2 *MOTOR1_srd.step_delay) + rest)/(4 * MOTOR1_srd.accel_count + 1));//计算新(下)一步脉冲周期(时间间隔)
                rest = ((2 * MOTOR1_srd.step_delay)+rest)%(4 * MOTOR1_srd.accel_count + 1);// 计算余数，下次计算补上余数，减少误差
                if(step_count >= MOTOR1_srd.decel_start)// 检查是够应该开始减速
                {
                    MOTOR1_srd.accel_count = MOTOR1_srd.decel_val; // 加速计数值为减速阶段计数值的初始值
                    MOTOR1_srd.run_state = MOTOR1_DECEL;           // 下个脉冲进入减速阶段
                }
                else if(new_step_delay <= MOTOR1_srd.min_delay) // 检查是否到达期望的最大速度
                {
                    last_accel_delay = new_step_delay; // 保存加速过程中最后一次延时（脉冲周期）
                    new_step_delay = MOTOR1_srd.min_delay;    // 使用min_delay（对应最大速度speed）
                    rest = 0;                          // 清零余值
                    MOTOR1_srd.run_state = MOTOR1_RUN;               // 设置为匀速运行状态
                }
                break;

            case MOTOR1_RUN:
                step_count++;  // 步数加1
                if(MOTOR1_srd.dir==MOTOR1_CW)
                {
                    MOTOR1_step_position++; // 绝对位置加1
                }
                else
                {
                    MOTOR1_step_position--; // 绝对位置减1
                }
                new_step_delay = MOTOR1_srd.min_delay;     // 使用min_delay（对应最大速度speed）
                if(step_count >= MOTOR1_srd.decel_start)   // 需要开始减速
                {
                    MOTOR1_srd.accel_count = MOTOR1_srd.decel_val;  // 减速步数做为加速计数值
                    new_step_delay = last_accel_delay;// 加阶段最后的延时做为减速阶段的起始延时(脉冲周期)
                    MOTOR1_srd.run_state = MOTOR1_DECEL;            // 状态改变为减速
                }
                break;

            case MOTOR1_DECEL:
                step_count++;  // 步数加1
                if(MOTOR1_srd.dir==MOTOR1_CW)
                {
                    MOTOR1_step_position++; // 绝对位置加1
                }
                else
                {
                    MOTOR1_step_position--; // 绝对位置减1
                }
                MOTOR1_srd.accel_count++;
                new_step_delay = MOTOR1_srd.step_delay - (((2 * MOTOR1_srd.step_delay) + rest)/(4 * MOTOR1_srd.accel_count + 1)); //计算新(下)一步脉冲周期(时间间隔)
                rest = ((2 * MOTOR1_srd.step_delay)+rest)%(4 * MOTOR1_srd.accel_count + 1);// 计算余数，下次计算补上余数，减少误差

                //检查是否为最后一步
                if(MOTOR1_srd.accel_count >= 0)
                {
                    MOTOR1_srd.run_state = MOTOR1_STOP;
                }
                break;
            }
            MOTOR1_srd.step_delay = new_step_delay; // 为下个(新的)延时(脉冲周期)赋值
        }
    }
    rt_interrupt_leave();
}

static void box(__IO int32_t step, __IO uint32_t accel, __IO uint32_t decel, __IO uint32_t speed)
{
    if(step<-200 || step>BOX_MOVE_MAX)
    {
        LOG_D("step error.");
    }
    else
    {
        motor1_moveabs(step,accel,decel,speed);
    }
}


#include "finsh.h"
FINSH_FUNCTION_EXPORT_ALIAS(box, box, box);
















