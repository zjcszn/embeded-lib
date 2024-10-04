/*******************************************************************************
                        青岛市张彦欣单片机有限公司
                            版权所有 @ 2018

步进电机驱动库.主要用于加减速频繁或速度型应用.可以方便的控制加减速的加速度.
加减速的加速度可以不同,并可以随时改变.

张彦欣
2018.04.08
*******************************************************************************/
#include  "SpeedStepper.h"



/*******************************************************************************
函 数: SpeedStepper(void)
功 能: 构造函数.完成变量的初始化.
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
SpeedStepper::SpeedStepper(void)
{
    hardware.PinPls     = null;
    hardware.PinDir     = null;
    hardware.PlsPerCir  = 400;
    hardware.StopLevel  = LOW;
    
    ramp.RampUpStep     = 1.0;
    ramp.RampDnStep     = 1.0;
    
    system.CurFrq       = 0;            //系统状态结构体.
    system.CurDir       = DIR_ZHENG;
    system.location     = 0;
    system.locationHi   = 0;
    system.State        = MOTOR_STOP;
    
    system.EnMaxLocationLimit = false;      //软件最大限位
    system.MaxLocation        = 100000000;
    system.EnMinLocationLimit = false;
    system.MinLocation        = -100000000; //软件最小限位
    
    config.SetFrq       = 0;            //设置频率值
    config.DirOpp       = false;        //电机是否反向
    config.PlsDelayTime = 5;            //脉冲延时时间.默认5uS.
}

/*******************************************************************************
函 数: void begin(  TIM_TypeDef* UseTimer,
                    Uint  PlusPerCir, 
                    Uchar DrvPls, Uchar DrvDir,
                    Ulong PlsMod, Ulong DirMod,
                    Uchar PlsStopLvl )
功 能: 初始化驱动步进电机的各种参数.
参 数: UseTimer   -- 驱动该步进电机使用的定时计数器.
       PlusPerCir -- 步进电机转动一圈所需要的脉冲数量.
       DrvPls     -- 脉冲引脚.
       DrvDir     -- 方向引脚.
       PlsMod     -- 脉冲引脚端口模式.
       DirMod     -- 方向引脚端口模式.
       PlsStopLvl -- 脉冲引脚停止时电平.
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::begin(   TIM_TypeDef* UseTimer,
                            TIM_TypeDef* RampTimer,
                            Uint  PlusPerCir, 
                            Uchar DrvPls,
                            Uchar DrvDir,
                            Uchar PlsStopLvl )
{
    DrvTimer.begin(UseTimer);           //步进电机使用的定时器.
    DrvTimer.set(100);                  //设置定时器的中断频率.
    DrvTimer.close();                   //关闭定时器
    
    if( RampTimer!=null )               //加减速定时器
    {
        RmpTimer.begin(RampTimer);
        RmpTimer.set(100);              //加减速定时器固定每100ms中断一次
        RmpTimer.open();                //如果电机加减速非常非常急(加速度大)
    }                                   //可以加大中断频率
    
    hardware.PlsPerCir = PlusPerCir;    //步进电机转动一圈所需要的脉冲数量.
    
    hardware.PinPls    = DrvPls;        //驱动引脚+方向引脚.
    hardware.PinDir    = DrvDir;
    
    pinMode(hardware.PinPls, OUTPUT);   //脉冲引脚.
    pinMode(hardware.PinDir, OUTPUT);   //方向引脚.
    
    OS_Direction();                     //方向输出.
    hardware.StopLevel = PlsStopLvl;    //停机时脉冲电平.
    
    config.MinSpeed = 0.1;              //默认最低转速0.1RPM
    config.MinFrq   = (0.1)*(float)(hardware.PlsPerCir)/60.00;
    
    stop();                             //电机初始化的时候停机.
}

/*******************************************************************************
函 数: Uchar CurDir(void)
功 能: 获取当前电机的运动方向.
参 数: None
返 回: DIR_ZHENG/DIR_FAN

张彦欣
2018.04.08
*******************************************************************************/
Uchar SpeedStepper::CurDir(void)
{
    return (system.CurDir);
}

/*******************************************************************************
函 数: void DirOpp(Uchar _Cmd)
功 能: 运动方向是否取反.
参 数: _Cmd--运动方向是否取反.false--不取反;true--取反.
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::DirOpp(Uchar _Cmd)
{
    if(_Cmd)
    {
        config.DirOpp = true;
    }
    else
    {
        config.DirOpp = false;
    }
}

/*******************************************************************************
函 数: void MinSpeedpeed(float _RPM)
功 能: 设置步进电机的最低运转速度.
参 数: myRPM -- 步进电机转动速度.单位RPM.
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::MinSpeed(float _RPM)
{
    if(_RPM<0)      //设置的转速小于0.不正常.
    {               //可以等于零
        return;
    }
    
    config.MinSpeed = _RPM;
    config.MinFrq   = (_RPM)*(float)(hardware.PlsPerCir)/60.00;
}

/*******************************************************************************
函 数: void speed(float myRPM)
功 能: 设置步进电机的转速.
参 数: myRPM -- 步进电机转动速度.单位RPM.
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::speed(float _RPM)
{
    if(_RPM<0)      //设置的转速小于0.不正常.
    {               //可以等于零.用来控制电机慢慢停下.
        return;
    }
    
    config.SetFrq = (_RPM)*(float)(hardware.PlsPerCir)/60.00;
    
//  Serial.print("SetFrq:");
//  Serial.println(config.SetFrq);
}

/*******************************************************************************
函 数: void run(void)
功 能: 电机开始运行.
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::run(void)
{
    if( state()!=MOTOR_STOP )       //电机正在运行中.
    {
    //  Serial.println("MOTOR Running!");
        return;
    }
    
    if( config.SetFrq==0 )          //设置速度是零.不需要启动电机.
    {
    //  Serial.println("SetFrq is Zero!");
        return;
    }
    
    // SpeedStepper不能关闭斜坡功能
    // 当前电机肯定是停止的
    // 电机状态: 加速中
    system.State  = MOTOR_UP;        
    system.CurFrq = ramp.RampUpStep;
    
    if(system.CurFrq>config.SetFrq)
    {
        system.CurFrq=config.SetFrq;
    }  
    
    //电机运行的最小频率
    float MinFrq = (config.MinSpeed)*(float)(hardware.PlsPerCir)/60.00;
    if( system.CurFrq<MinFrq )
    {
        system.CurFrq = MinFrq;
    }
    
    OS_LoadTimer();                 //重新加载定时器.
    DrvTimer.SetCounter(0xFFFF);    //V1.1修改.电机低速启动的时候,延时时间过长.
                                    //原因是Timer预分频设置第二次中断才生效.
                                    //第一次中断时间过长.6秒左右.电机就会延迟6秒钟启动.
                                    //这里强迫第一次中断立即产生.
    DrvTimer.open();                //开启定时计数器.
}

/*******************************************************************************
函 数: float CurSpeed(void)
       float CurFrq(void)
功 能: 获取当前转速/脉冲频率
参 数: None
返 回: RPM/Hz

张彦欣
2018.04.08
*******************************************************************************/
float SpeedStepper::CurSpeed(void)
{
    if( hardware.PlsPerCir<=0 ) //电机转1圈的脉冲数
    {
        hardware.PlsPerCir = 200;
    }
    
    float RData = (system.CurFrq) * 60.0 / (float)(hardware.PlsPerCir);
    
    return RData;               //单位:RPM
}

float SpeedStepper::CurFrq(void)
{
    return system.CurFrq;       //单位:Hz
}

/*******************************************************************************
函 数: void PlsDelay(Uint Dly)
功 能: 脉冲延时时间.
参 数: Dly--延时时间.单位:微秒.
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::PlsDelay(Uint Dly)
{
    config.PlsDelayTime = Dly;
}

/*******************************************************************************
函 数: Uchar dir(Uchar RunDir, long RunSteps, Uchar _Cmd)
功 能: 修改步进电机的运动方向
参 数: RunDir : 步进电机转动方向. 0/1.
       _Cmd   : 是否强制更新电机运行方向
返 回: 执行成功/没有执行

张彦欣
2018.04.08
*******************************************************************************/
Uchar SpeedStepper::dir(Uchar RunDir,Uchar _Cmd)
{
    if( (system.CurDir!=RunDir)&&(_Cmd==false) )
    {
        return false;
    }
    
    system.CurDir = RunDir;     //记录运动方向.
    OS_Direction();             //将运动方向输出到引脚.  
    
    return true;
}

Uchar SpeedStepper::dir(Uchar RunDir)
{
    dir(RunDir,true);
}

/*******************************************************************************
函 数: void Stop(void)
功 能: 立即停止步进电机的运行.此函数会立即停止电机,并没有减速效果.
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::stop(void)
{
    config.SetFrq = 0;
    system.CurFrq = 0;
    system.State  = MOTOR_STOP;
    
    DrvTimer.close();
}

/*******************************************************************************
函 数: Uchar get(void)
功 能: 获取当前电机的运行状态.
参 数: None
返 回: MOTOR_STOP/MOTOR_UP/MOTOR_RUN/MOTOR_DOWN

张彦欣
2018.04.08
*******************************************************************************/
Uchar SpeedStepper::state(void)
{
    return (system.State);
}

/*******************************************************************************
函 数: long location(void)
功 能: 获取全局位置/设置全局位置.
参 数: 坐标.
返 回: 坐标.

张彦欣
2018.04.08
*******************************************************************************/
long SpeedStepper::location(void)
{
    return (system.location);
}

void SpeedStepper::location(long _location)
{
    system.location = _location;
}

long SpeedStepper::locationHi(void)
{
    return (system.locationHi);
}

void SpeedStepper::locationHi(long _locationHi)
{
    system.locationHi = _locationHi;
}

/*******************************************************************************
函 数: void MaxLocation(Uchar _Cmd,long Value)
功 能: 设置电机的软件限位最大值
参 数: _Cmd:是否启用最大限位;Value:最大限位值.
返 回: None

张彦欣
2018.11.21
*******************************************************************************/
void SpeedStepper::MaxLocation(Uchar _Cmd,long Value)
{
    if(_Cmd)
    {
        system.EnMaxLocationLimit = true;
        system.MaxLocation        = Value;
    }
    else
    {
        system.EnMaxLocationLimit = false;
        system.MaxLocation        = Value;
    }
}

/*******************************************************************************
函 数: void MinLocation(Uchar _Cmd,long Value)
功 能: 设置电机的软件限位最小值
参 数: _Cmd:是否启用最小限位;Value:最小限位值.
返 回: None

张彦欣
2018.11.21
*******************************************************************************/
void SpeedStepper::MinLocation(Uchar _Cmd,long Value)
{
    if(_Cmd)
    {
        system.EnMinLocationLimit = true;
        system.MinLocation        = Value;
    }
    else
    {
        system.EnMinLocationLimit = false;
        system.MinLocation        = Value;
    }
}

/*******************************************************************************
函 数: void OS_LoadTimer(void)
功 能: 加载定时计数器频率
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::OS_LoadTimer(void)
{
    DrvTimer.set(system.CurFrq);
}

/*******************************************************************************
函 数: void OS_MotorStps(void)
       void OS_MotorEnd(void)
功 能: 步进电机脉冲输出函数.操作相关的端口.
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
inline void SpeedStepper::OS_MotorStps(void)
{
    digitalWrite(hardware.PinPls,!hardware.StopLevel);
    delayMicroseconds(config.PlsDelayTime);
}

inline void SpeedStepper::OS_MotorEnd(void)
{
    digitalWrite(hardware.PinPls,hardware.StopLevel);
}

/*******************************************************************************
函 数: void OS_location()
功 能: 记录步进电机全局位置.
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::OS_location(void)
{
    if(system.CurDir==DIR_ZHENG)        
    {
        system.location++;              //全局位置坐标
        
        if(system.location>2000000000L)
        {
            system.location = 0;
            system.locationHi++;
        }
        
        if(system.EnMaxLocationLimit)   //使能了最大值限位
        {
            if(system.location>=system.MaxLocation)
            {
                speed(0);
            }
        }
    }
    else
    {
        system.location--;              //全局位置坐标
        
        if(system.location<-2000000000L)
        {
            system.location = 0;
            system.locationHi--;
        }
        
        if(system.EnMinLocationLimit)   //使能了最小值限位
        {
            if(system.location<=system.MinLocation)
            {
                speed(0);
            }
        }
    }
}

/*******************************************************************************
函 数: void ARamp(long _Accel)
功 能: 设置系统加速度设置
参 数: _AccelUp:加速时候的加速度.单位RPM/S.
       _AccelDn:减速时候的加速度.
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::ARamp(float _AccelUp,float _AccelDn)
{
    float UpFrq = (_AccelUp)*(float)(hardware.PlsPerCir)/60.00;
    float DnFrq = (_AccelDn)*(float)(hardware.PlsPerCir)/60.00;
    
    float UpStep= MaxRounding(UpFrq/100.0,4);
    float DnStep= MaxRounding(DnFrq/100.0,4);
    
    if(UpStep<=0.1)
    {
        UpStep=0.1;
    }
    if(DnStep<=0.1)
    {
        DnStep=0.1;
    }
    
    ramp.RampUpStep = UpStep;
    ramp.RampDnStep = DnStep;
}

void SpeedStepper::ARamp(float _Accel)
{
    ARamp(_Accel,_Accel);
}

/*******************************************************************************
函 数: void OS_Direction(void)
功 能: 电机运动方向控制.
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::OS_Direction(void)
{
    if(config.DirOpp)
    {
        digitalWrite(hardware.PinDir, !system.CurDir);
    }
    else
    {
        digitalWrite(hardware.PinDir, system.CurDir);
    }
    
    delayMicroseconds(10);  //稍微延时 等待方向引脚电平稳定
}

/*******************************************************************************
函 数: void OS_HiLRamp(void)
功 能: 直线加减速函数.高速函数.本函数每10毫秒运行1次.
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::OS_HiLRamp(void)
{
    if(system.State==MOTOR_STOP)            //电机处于停止状态.
    {                                       //直接退出.2018.05.01
        return;
    }
    
    if( system.CurFrq<config.SetFrq )       //需要加速
    {
        system.CurFrq += ramp.RampUpStep;
        
        if( system.CurFrq<config.MinFrq )    //最低启动速度是MinFrq
        {
            system.CurFrq = config.MinFrq;
        }
        if(system.CurFrq>config.SetFrq)
        {
            system.CurFrq=config.SetFrq;
        }
        
        OS_LoadTimer();                     //重新加载定时器.
        system.State = MOTOR_UP;            //电机状态:加速中.
    }
    else if( system.CurFrq>config.SetFrq )  //需要减速
    {
        system.CurFrq -= ramp.RampDnStep;
        
        if(system.CurFrq<config.SetFrq)
        {
            system.CurFrq=config.SetFrq;
        }
        if(system.CurFrq<config.MinFrq)     //当前速度低于最低速度
        {
            OS_LoadTimer();                 //重新加载定时器.   
            DrvTimer.close();               //开启定时计数器.
            system.State = MOTOR_STOP;      //标志位.停止.
        }
        
        OS_LoadTimer();                     //重新加载定时器. 
        system.State = MOTOR_DOWN;          //电机状态:减速中.
    }
    else
    {
        if(system.CurFrq==0)                //当前状态,停止.
        {
            OS_LoadTimer();                 //重新加载定时器.   
            DrvTimer.close();               //开启定时计数器.
            system.State = MOTOR_STOP;      //标志位.停止.
        }
        else
        {
            OS_LoadTimer();                 //重新加载定时器. 
            system.State = MOTOR_RUN;       //电机状态:恒速中.
        }
    }
}

/*******************************************************************************
函 数: void TISR(void)
功 能: 加减速中断函数.固定周期调用该函数.用来控制电机加减速.100Hz
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::TISR(void)
{
    OS_HiLRamp();
}

/*******************************************************************************
函 数: void SpeedStepper::System_MotorISR(void)
功 能: 这个函数是由定时器中断调用的函数.用于生成脉冲信号.
参 数: None
返 回: None

张彦欣
2018.04.08
*******************************************************************************/
void SpeedStepper::scan(void)
{
    OS_MotorStps();     //开始输出脉冲信号.
    OS_location();      //记录全局位置.
    OS_MotorEnd();      //关闭脉冲输出.
}




