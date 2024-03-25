#ifndef __MOTOR1_H
#define __MOTOR1_H
#include "stm32f10x.h"
#include "rtthread.h"

/* 类型定义 ------------------------------------------------------------------*/
typedef struct {
  __IO uint8_t  run_state ;  // 电机旋转状态
  __IO uint8_t  dir ;        // 电机旋转方向
  __IO int32_t  step_delay;  // 下个脉冲周期（时间间隔），启动时为加速度
  __IO uint32_t decel_start; // 启动减速位置
  __IO int32_t  decel_val;   // 减速阶段步数
  __IO int32_t  min_delay;   // 最小脉冲周期(最大速度，即匀速段速度)
  __IO int32_t  accel_count; // 加减速阶段计数值
}MOTOR1_speedRampData;

/* 宏定义 --------------------------------------------------------------------*/
//电机转动方向
#define MOTOR1_DIR_CCW()                   GPIO_ResetBits(GPIOA,GPIO_Pin_5)
#define MOTOR1_DIR_CW()                    GPIO_SetBits(GPIOA,GPIO_Pin_5)
//电机使能
#define MOTOR1_OUTPUT_ENABLE()             GPIO_ResetBits(GPIOA,GPIO_Pin_3)
#define MOTOR1_OUTPUT_DISABLE()            GPIO_SetBits(GPIOA,GPIO_Pin_3)

#define MOTOR1_FALSE                                 0
#define MOTOR1_TRUE                                  1
#define MOTOR1_CW                                    0 // 顺时针
#define MOTOR1_CCW                                   1 // 逆时针

//加减速曲线状态
#define MOTOR1_STOP                                  0 //停止
#define MOTOR1_ACCEL                                 1 //加速阶段
#define MOTOR1_DECEL                                 2 //减速阶段
#define MOTOR1_RUN                                   3 //匀速阶段

//定义定时器预分频，定时器实际时钟频率为：72MHz/（STEPMOTOR_TIMx_PRESCALER+1）
//#define MOTOR1_TIM_PRESCALER               3  // 步进电机驱动器细分设置为：   32  细分
//#define MOTOR1_TIM_PRESCALER               7  // 步进电机驱动器细分设置为：   16  细分
//#define MOTOR1_TIM_PRESCALER               15  // 步进电机驱动器细分设置为：   8  细分
#define MOTOR1_TIM_PRESCALER               31  // 步进电机驱动器细分设置为：   4  细分
//#define MOTOR1_TIM_PRESCALER               63  // 步进电机驱动器细分设置为：   2  细分
//#define MOTOR1_TIM_PRESCALER               127  // 步进电机驱动器细分设置为：   1  细分

#define MOTOR1_T1_FREQ                               (72000000/(MOTOR1_TIM_PRESCALER+1)) // 频率ft值
#define MOTOR1_FSPR                                  200  //步进电机单圈步数
#define MOTOR1_MICRO_STEP                            4   // 步进电机驱动器细分数
#define MOTOR1_SPR                                   (MOTOR1_FSPR*MOTOR1_MICRO_STEP)   // 旋转一圈需要的脉冲数

//数字常数
#define MOTOR1_ALPHA                                 ((float)(2*3.14159/MOTOR1_SPR))       // α= 2*pi/spr
#define MOTOR1_A_T_x10                               ((float)(10*MOTOR1_ALPHA*MOTOR1_T1_FREQ))
#define MOTOR1_T1_FREQ_148                           ((float)((MOTOR1_T1_FREQ*0.676)/10)) // 0.676为误差修正值
#define MOTOR1_A_SQ                                  ((float)(2*100000*MOTOR1_ALPHA)) 
#define MOTOR1_A_x200                                ((float)(200*MOTOR1_ALPHA))

//深孔板最大移动位置
#define BOX_MOVE_MAX   10000
/* 等待电机到位,此宏定义主要用于校准的时候使用 */
#define WAIT_MOTOR1_STOP()     {while(MOTOR1_srd.run_state != MOTOR1_STOP)\
                                {\
                                    rt_thread_mdelay(10);\
                                }\
                                rt_thread_mdelay(50);}   //请注意,此处的延迟必不可少!!! 
/* 等待电机到位,并判断实验是否退出,此宏定义主要用于实验的时候使用 */
#define WAIT_M1_STOP()         {while(MOTOR1_srd.run_state != MOTOR1_STOP)\
                                {\
                                    if(running_state == E_RUNNING_STATUSV2_IDLE)\
                                    {\
                                        return;\
                                    }\
                                    rt_thread_mdelay(10);\
                                }\
                                rt_thread_mdelay(50);}   //请注意,此处的延迟必不可少!!!
extern MOTOR1_speedRampData MOTOR1_srd;
extern __IO int32_t  MOTOR1_step_position;
extern __IO uint8_t  MOTOR1_MotionStatus;

extern void motor1_init(void);
extern void motor1_io_init(void);
extern void motor1_TIMx_init(void);
extern void motor1_moveabs(__IO int32_t absstep, __IO uint32_t accel, __IO uint32_t decel, __IO uint32_t speed);
extern void motor1_moverel(__IO int32_t step, __IO uint32_t accel, __IO uint32_t decel, __IO uint32_t speed);




#endif
