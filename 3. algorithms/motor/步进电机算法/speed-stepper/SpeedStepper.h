/*******************************************************************************
                        青岛市张彦欣单片机有限公司
                            版权所有 @ 2018
    
  SuperStepper库用于驱动步进电机或伺服电机.输出信号分为脉冲信号Pls和方向引脚Dir.
必须接步进电机驱动器或伺服电机驱动器.
  主要用于加减速频繁或速度型应用.可以方便的控制加减速的加速度.
  加减速的加速度可以不同.

张彦欣
2017.03.26
*******************************************************************************/
#ifndef  _SPEED_STEPPER_H_
#define  _SPEED_STEPPER_H_
    
    
#include "MaxDuino.h"
#include "MaxTimer.h"
    
    
#define  DIR_FOREWARD   0   //正转
#define  DIR_REVERSAL   1   //反转

// 为了便于工程上的同事读取正反 使用拼音标记
// 2018.10.01
#define  DIR_ZHENG      DIR_FOREWARD
#define  DIR_FAN        DIR_REVERSAL


#define  MOTOR_STOP     0   //电机停止中.
#define  MOTOR_UP       1   //电机加速中.
#define  MOTOR_RUN      2   //电机恒速中.
#define  MOTOR_DOWN     3   //电机减速中.




struct RampStruct
{
    float    RampUpStep;    //加速步距.
    float    RampDnStep;    //减速步距.
};

struct HardwareStruct
{   
    Uchar    PinPls;        //驱动引脚.脉冲引脚.
    Uchar    PinDir;        //驱动引脚.方向引脚.
    
    Uint     PlsPerCir;     //步进电机转动一圈,所需要的脉冲数量.
    Uchar    StopLevel;     //当步进电机停止运转时.
};

struct ConfigStruct
{
    float   SetFrq;         //目标频率.单位:Hz.
                            //根据RPM计算出来的定时器中断频率.
    
    Uchar   DirOpp;         //运动方向是否反向.true/false.
    
    Uint    PlsDelayTime;   //延时时间.单位:微秒.
    
    float   MinSpeed;       //电机运行的最小速度.单位RPM.
    float   MinFrq;         //脉冲输入的最小频率
};

struct SystemStruct
{
    float   CurFrq;         //记录当前速度.单位:Hz.
    Uchar   CurDir;         //当前电机转动方向.
    
    long    location;       //当前全局位置.
    long    locationHi;
    
    Uchar   EnMaxLocationLimit;
    long    MaxLocation;
    Uchar   EnMinLocationLimit;
    long    MinLocation;
    
    Uchar   State;          //电机运行状态(加减速状态).
};


class SpeedStepper 
{
    public:   
    
    HardwareStruct  hardware;
    ConfigStruct    config;
    RampStruct      ramp;
    SystemStruct    system;
    
    MaxTimer        DrvTimer;
    MaxTimer        RmpTimer;
    
    
    void  begin( TIM_TypeDef* UseTimer,
                 TIM_TypeDef* RampTimer,
                 Uint  myPlusPerCir,            //电机转动一圈脉冲数.
                 Uchar myPls,                   //脉冲引脚+方向引脚.
                 Uchar myDir,      
                 Uchar PlsStopLvl );            //停机时脉冲引脚电平.
                
                
    void  MinSpeed(float _RPM);                 //设置最低运转速度. 
    void  speed(float _RPM);                    //设置速度.单位:_RPM.
    float CurSpeed();                           //获取当前速度.单位:_RPM.
    float CurFrq();
    
    void  PlsDelay(Uint Dly);                   //脉冲延时.
    
    Uchar dir(Uchar Dir, Uchar _Cmd);           //电机运行一定的步数.
    Uchar dir(Uchar Dir);
    
    void  run(void);                            //电机开始运行.
    
    void  stop(void);                           //立即无条件停止电机.
    
    Uchar state(void);                          //获取电机运行状态.
    
    void  ARamp(float _Accel);                  //设置加速度.单位:_RPM/M
    void  ARamp(float _AccelUp,float _AccelDown);
    
    long  location(void);                       //获取当前位置.
    void  location(long CurLocation);           //设置当前位置.
    
    long  locationHi(void);
    void  locationHi(long CurLocationHi);
    
    void  MaxLocation(Uchar _Cmd,long Value);   //开启位置限制.
    void  MinLocation(Uchar _Cmd,long Value);
    
    void  DirOpp(Uchar _Cmd);                   //运动方向是否反向.
    Uchar CurDir(void);                         //获取当前运动方向.
    
    void  scan(void);                           //步进电机驱动函数.在ISR中运行.
    void  TISR(void);                           //高速电机的加减速中断.
    
    SpeedStepper(void);                         //构造函数.完成变量初始化.
    
    
    private:
    
    void    OS_HiLRamp(void);                   //直线加减速(高速).
    void    OS_Direction(void);                 //电机运动方向控制.
    
    void    OS_LoadTimer(void);                 //重载定时计数器频率.
    
    void    OS_MotorStps(void);                 //输出一个脉冲信号.
    void    OS_MotorEnd(void);                  //关闭脉冲输出.
    
    void    OS_location(void);                  //记录全局位置.
};
    
    
    
#endif
    



