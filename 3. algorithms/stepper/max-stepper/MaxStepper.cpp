/*******************************************************************************
                            青岛市张彦欣单片机有限公司
                                版权所有  @ 2014

张彦欣
2014.10.08
*******************************************************************************/
#include  "MaxStepper.h"


//S曲线表
const float SRampTable[100] = 
{
    0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.02, 0.02, 0.02,
    0.02, 0.02, 0.03, 0.03, 0.03, 0.04, 0.04, 0.04, 0.05, 0.05,
    0.06, 0.06, 0.07, 0.08, 0.08, 0.09, 0.10, 0.11, 0.12, 0.13,
    0.14, 0.15, 0.17, 0.18, 0.20, 0.21, 0.23, 0.25, 0.27, 0.29,
    0.31, 0.33, 0.35, 0.38, 0.40, 0.43, 0.45, 0.48, 0.50, 0.52,
    0.55, 0.57, 0.60, 0.62, 0.65, 0.67, 0.69, 0.71, 0.73, 0.75,
    0.77, 0.79, 0.80, 0.82, 0.83, 0.85, 0.86, 0.87, 0.88, 0.89,
    0.90, 0.91, 0.92, 0.92, 0.93, 0.94, 0.94, 0.95, 0.95, 0.96,
    0.96, 0.96, 0.97, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98, 0.98,
    0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 1.00
};

/*******************************************************************************
函 数: MaxStepper(void)
功 能: 构造函数.完成变量的初始化

张彦欣
2014.11.16
*******************************************************************************/
MaxStepper::MaxStepper(void)
{
    config.RunDir     = DIR_ZHENG;  //电机的转动方向.

    config.PIN_Step   = NULL;       //驱动引脚.脉冲引脚.
    config.PIN_Dir    = NULL;       //驱动引脚.方向引脚.
    config.PlsPerCir  = 200;        //转一圈,脉冲数量.

    system.RemainSteps  = 0;        //目标步数.控制步进电机接下来运行多少步数.
    system.CurrentSteps = 0;        //当前步数.
    system.State        = MOTOR_STOP;
    system.CurFrq       = 0;

    config.RPM          = 0;        //电机的转速.config.RPM.    
    config.TargetFrq    = 0;        //目标频率.Hz.

    config.MinPlsFrq    = DEFAULT_MIN_PLS_FREQ;
                                    //脉冲引脚输出的脉冲最低频率.

    config.StopLevel  = LOW;        //当步进电机停止运转时,输出的电平.默认高电平.

    config.EnLsw      = false;      //限位开关使能标志位.
    config.LswLevel   = LOW;        //当限位开关动作时的电平.默认高电平.
    config.PIN_LswPos = NULL;       //增量限位开关引脚
    config.PIN_LswNag = NULL;       //减量限位开关引脚
    system.LSW_PosFlg = false;      //限位开关动作标志位(正方向)
    system.LSW_NagFlg = false;      //限位开关动作标志位(反方向)
    
    config.EnSoftLsw      = false;  //使能软件限位
    system.SoftLSW_PosFlg = false;  //软件限位报警标识
    system.SoftLSW_NagFlg = false;
    
    config.TimerShareEnable = false;//关闭定时器共享
    system.TimerShareFlg    = false;//定时器共享
    
    ramp.EnRamp     = false;        //斜坡功能使能标志.
    ramp.RampMode   = RAMP_MODE_L;  //斜坡模式.
    ramp.RampLength = 200;			//斜坡函数--加速的长度.  

	system.location = 0;            //全局变量.记录当前位置.
	config.DirOpp   = false;        //运动方向反向.
}

/*******************************************************************************
函 数: void Stepper::begin( TIM_TypeDef* myTimer,
							Uint myPlusPerCir, Uchar myPls, Uchar myDir )
功 能: 初始化驱动步进电机的各种参数.
参 数: myTimer -- 驱动该步进电机使用的定时计数器.
       myPlusPerCir -- 步进电机转动一圈所需要的脉冲数量.
       myPin1  -- 步进电机驱动的引脚1.脉冲引脚.
       myPin2  -- 步进电机驱动的引脚2.方向引脚.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::begin(	TIM_TypeDef* myTimer,
						Uint  myPlusPerCir, 
                        Uint  myPls,  Uint myDir,
                        Ulong PlsMod, Ulong DirMod,
                        Uchar PlsStopLvl )
{
	config.DrvTimer.begin(myTimer);     //初始化步进电机使用的定时器.
    config.DrvTimer.set(1000);          //设置中断频率
    config.DrvTimer.close();            //关闭定时器
    
    system.CurFrq    = 0;               //标记当前速度.

	config.PlsPerCir = myPlusPerCir;    //步进电机转动一圈所需要的脉冲数量.

	config.PIN_Step   = myPls;          //脉冲输出引脚
	config.PIN_Dir    = myDir;          //方向输出引脚

    pinMode(config.PIN_Step, PlsMod);   //脉冲引脚端口初始化.OUTPUT
    config.StopLevel = PlsStopLvl;      //设置当步进电机不运行时,脉冲输出端的电平.
    digitalWrite(config.PIN_Step, config.StopLevel);

	pinMode(config.PIN_Dir,  DirMod);   //方向引脚端口初始化.OUTPUT
    digitalWrite(config.PIN_Dir, config.RunDir);
    
    stop();                             //停止.
}

void MaxStepper::begin( TIM_TypeDef* myTimer, 
                        Uint  myPlusPerCir, 
                        Uchar myPls, Uchar myDir )
{
    begin(myTimer,myPlusPerCir,myPls,myDir,OUTPUT,OUTPUT,LOW);
}

/*******************************************************************************
函 数: void OS_Direction(void)
功 能: 电机运动方向控制

张彦欣
2014.10.08
*******************************************************************************/
inline void MaxStepper::OS_Direction(void)
{
	if(config.DirOpp)   //是否将正反方向反向
	{
		digitalWrite(config.PIN_Dir, !config.RunDir);
	}
	else
	{
		digitalWrite(config.PIN_Dir, config.RunDir);
	}
}

/*******************************************************************************
函 数: void OS_LoadTimer(void)
功 能: 加载定时计数器频率.内部函数.

张彦欣
2017.03.26
*******************************************************************************/
inline void MaxStepper::OS_LoadTimer(void)
{
    config.DrvTimer.set(system.CurFrq);
}

/*******************************************************************************
函 数: Uchar CurDir(void)
功 能: 获取当前电机的运动方向

张彦欣
2014.10.08
*******************************************************************************/
Uchar MaxStepper::CurDir(void)
{
    if(config.RunDir==DIR_ZHENG)
    {
        return DIR_ZHENG;
    }
    else
    {
        return DIR_FAN;
    }
}

/*******************************************************************************
函 数: void speed(float myRPM)
功 能: 设置步进电机的转速.
参 数: myRPM -- 步进电机转动速度.单位RPM.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::speed(float myRPM)
{
	if(myRPM<=0)                //设置的转速小于0.不正常.
	{
        stop();                 //停止电机.
        config.RPM=0;
        config.TargetFrq=0;
		return;
	}
    
    if( myRPM==config.RPM )     //要设置的速度值没有发生变化.
    {
		return;
    }
    
    config.RPM       = myRPM;   //转速 RPM
    config.TargetFrq = (config.RPM)*(float)(config.PlsPerCir)/60.00;
    
    // 用户设置的RampLength(加减速长度),是300RPM转速下的加减速长度
    // 要根据当前的速度,设置对应的实际加减速长度
    ramp.RampLength = (ramp.RampLengthSET)*(config.RPM)/300;
}

// 大写Spped()和小写speed()功能一样
// 为了兼容旧版本程序
void MaxStepper::Speed(float myRPM)
{
    speed(myRPM);
}

// 获取当前电机速度设定
float MaxStepper::speed(void)
{
    return config.RPM;
}

// 大写Spped()和小写speed()功能一样
// 为了兼容旧版本程序
float MaxStepper::Speed(void)
{
    return config.RPM;
}

/*******************************************************************************
函 数: void MinSpeed(float config.RPM)
功 能: 设置电机加减速阶段的起步频率.电机运行的最小速度.
参 数: config.RPM--步进电机起步频率.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::MinSpeed(float myRPM)
{
    float PlsFrq = (myRPM)*(float)(config.PlsPerCir)/60.0 ;
    
    if(PlsFrq>0)    //最小速度大于零
    {
        config.MinPlsFrq = PlsFrq;
    }
    else            //最低速度不能为零.不正确.
    {
        config.MinPlsFrq = DEFAULT_MIN_PLS_FREQ;    
    }
}

/*******************************************************************************
函 数: float CurSpeed(void)
功 能: 获取电机当前正在运行的速度.单位:RPM.
返 回: 当前速度.单位:config.RPM.

张彦欣
2014.10.08
*******************************************************************************/
float MaxStepper::CurSpeed(void)
{
    return (  (system.CurFrq)*60.0/(float)(config.PlsPerCir)  );
}

/*******************************************************************************
函 数: void run(Uchar myDir, Ulong mySteps)
功 能: 步进电机转动指令.命令步进电机转动一定的步数.
参 数: myDir   -- 步进电机转动方向. 0/1.
       mySteps -- 步进电机转动的步数.
举 例: run(DIR_ZHENG,500); 代表往正方向运行500个脉冲.过程自动加减速.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::run(Uchar myDir, Ulong mySteps)
{
    if( mySteps==0 )                    //电机运行步数不正确.
    {
        return;
    }
    
    config.RunDir = myDir;              //记录运动方向.
    OS_Direction();                     //将运动方向输出到引脚.
    
    system.RemainSteps  = mySteps;      //目标步数.两个中断为一个脉冲.
    system.CurrentSteps = 0;            //当前步数.
    
    system.CurFrq = config.TargetFrq;   //不管是否启用加减速.第一次中断频率越快越好.快速进入中断.
                                        //在scan()函数内会计算加减速
    system.State  = MOTOR_UP;           //状态:加速
    
    system.TimerShareFlg = true;        //定时器共享运行标志位
    
    OS_LoadTimer();                     //设置定时计数器频率.
    config.DrvTimer.open();             //开启定时计数器.
    return;
}

// 大写Run()和小写run()功能一样
// 为了兼容旧版本程序
void MaxStepper::Run(Uchar myDir, Ulong mySteps)
{
    run(myDir,mySteps);
}

// 不指定运行方向的时候,按照Steps的正负号判断方向
// 正数往正方向运行;负数往负方向运行
void MaxStepper::run(long Steps)
{
    if(Steps>=0)
    {
        run(DIR_ZHENG,Steps);
    }
    else
    {
        run(DIR_FAN,-Steps);
    }
}

// 大小写Run()/run()功能一样
// 为了兼容旧版本程序
void MaxStepper::Run(long Steps)
{
    run(Steps);
}

/*******************************************************************************
函 数: void Stop(void)
功 能: 立即停止步进电机的运行.关闭定时器.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::stop(void)
{
    config.DrvTimer.close();        //关闭定时器
    
    system.CurFrq = 0;              //当前速度.
    system.State  = MOTOR_STOP;     //电机状态:停止.    

	system.RemainSteps  = 0;
	system.CurrentSteps = 0;

    digitalWrite(config.PIN_Step, config.StopLevel);
                                    //"电机停止时输出电平"
}

void MaxStepper::Stop(void)
{
    stop();
}

/*******************************************************************************
函 数: void RStop(void)
功 能: 使用斜坡加减速功能停止步进电机.电机会缓慢停止.停止举例取决于RampLength.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::RStop(void)
{
    if(system.CurFrq==0)    //当前电机是停止的.
    {
        return;
    }

    if( !ramp.EnRamp )      //斜坡功能没有被启用.
    {
        stop();
        return;
    }

    if(system.CurrentSteps<=system.RemainSteps)     //运动的前半部分
    {
        if(system.CurrentSteps<ramp.RampLength)     //正在加速阶段.
        {
            system.RemainSteps = system.CurrentSteps;
        }
        else
        {
            system.RemainSteps = ramp.RampLength;
        }
    }
    else                            //运动的后半部分
    {
        if(system.RemainSteps<ramp.RampLength)//已经在减速阶段了.
        {                           //不需要处理.
        }
        else
        {
            system.RemainSteps = ramp.RampLength;
        }
    }
}

/*******************************************************************************
函 数: Uchar get(void)
功 能: 获取当前电机的运行状态.
返 回: 

MOTOR_STOP      电机停止中
MOTOR_UP        电机加速中
MOTOR_RUN       电机恒速中
MOTOR_DOWN      电机减速中
MOTOR_ALARM     报警

张彦欣
2014.10.08
*******************************************************************************/
Uchar MaxStepper::state(void)
{
    return (system.State);
}

Uchar MaxStepper::State(void)
{
    return state();
}

/*******************************************************************************
函 数: long location()
功 能: 获取电机的全局位置/设置全局位置.
参 数: 坐标.
返 回: 坐标.

张彦欣
2016.12.14
*******************************************************************************/
long MaxStepper::location(void)
{
	return (system.location);
}

void MaxStepper::location(long CurLocation)
{
	system.location = CurLocation;
}

/*******************************************************************************
函 数: void MoveTo(long TargetLocation)
功 能: 电机运动到指定全局位置.差量运行,从当前位置运行到目标位置.
参 数: TargetLocation-全局位置坐标.
距 离: 例如当前电机处于坐标100脉冲处,执行MoveTo(200)以后,电机会往正方向运行100脉冲.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::MoveTo(long TargetLocation)
{
	long CurLocation = location();		//获取电机的当前坐标

	if(CurLocation==TargetLocation)		//当前已经在目标点.不需要运动.
	{
		return;
	}

	if(TargetLocation>CurLocation)		//目标点在当前点的正方向.
	{
		run( DIR_ZHENG,(TargetLocation-CurLocation) );
	}
	else								//目标点在当前点的负方向.
	{
		run( DIR_FAN,  (CurLocation-TargetLocation) );
	}
}

/*******************************************************************************
函 数: void DirOpp(Uchar _Cmd)
功 能: 运动方向是否取反.当设置为取反的时候,会将方向控制引脚的逻辑取反.
       也就是原来正转的方向会变成反向.
参 数: false-运动方向不取反;true-取反.

张彦欣
2016.12.14
*******************************************************************************/
void MaxStepper::DirOpp(Uchar _Cmd)
{
	if( _Cmd )
		config.DirOpp = true;
	else
		config.DirOpp = false;
}

Uchar MaxStepper::DirOpp(void)
{
    return config.DirOpp;
}

/*******************************************************************************
函 数: void wait(void)
功 能: 一直等待,直到电机运行停止.阻塞函数,尽量不用.注意电机报警死锁.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::wait(void)
{
    while( system.State!=MOTOR_STOP )
    {
        KickDog();
    }
}

// 带超时的等待电机停止
void MaxStepper::wait(Ulong OVTime)
{
    Ulong CurTime = millis();
    
    for(;;)
    {
        // 电机停止后退出
        if(system.State==MOTOR_STOP)
        {
            return;
        }
        
        // 计算等待时间
        if( MaxTimePass(CurTime)>=OVTime )
        {
            return;
        }
    }
}

/*******************************************************************************
函 数: void swait(Uchar Port,Uchar Lvl)
功 能: 一直等待,直到电机运行停止.指定端口动作,退出.谨慎使用.
参 数: Port--指定端口;Lvl--端口动作时电平.

张彦欣
2017.08.22
*******************************************************************************/
Uchar MaxStepper::swait(unsigned char ulPin, unsigned char Level)
{
    swait(ulPin,Level,10);
}

Uchar MaxStepper::swait(Uchar ulPin, Uchar Level, Uint FilterTime)
{
    Ulong RunTime=millis();
    
    for(;;)
    {
        if(system.State==MOTOR_STOP)    //电机没有遇到端口变化就停止了
        {
            return false;
        }

        if( digitalRead(ulPin)==Level ) //满足停止条件.
        {
            if( MaxTimePass(RunTime)>=FilterTime)
            {                           //等待确认时间.ms.
                stop();                 //停止电机.
                return true;            //返回
            }
        }
        else
        {
            RunTime = millis();
        }
    }    
}

/*******************************************************************************
函 数: void LSWLevel(Uchar mySetLevel)
功 能: 设置限位开关动作时的电平.
参 数: mySetLevel-电平状态. 0-低电平;1-高电平.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::LSWLevel(Uchar mySetLevel)
{
	if(mySetLevel)
    {
        config.LswLevel = HIGH;        
    }
	else
    {
        config.LswLevel = LOW;        
    }
}

/*******************************************************************************
函 数: void EnLSW(void)
功 能: 开启/关闭限位开关功能.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::LSW(Uchar _Cmd)
{
    if(_Cmd)
    {
        config.EnLsw = true;
    }
    else
    {
        config.EnLsw      = false;
        system.LSW_PosFlg = false;
        system.LSW_NagFlg = false;
    }
}

/*******************************************************************************
函 数: void LSWPin(Uchar myPin1, Uchar myPin2)
功 能: 设置限位开关引脚.
参 数: myPin1 -- 限位开关增量限位.
       myPin2 -- 限位开关减量限位.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::LSWPin(Uchar myPin1, Uchar myPin2)
{
	config.PIN_LswPos 	= myPin1;
	config.PIN_LswNag 	= myPin2;
	
	pinMode(config.PIN_LswPos, INPUT);  //设置限位开关引脚为输入状态
	pinMode(config.PIN_LswNag, INPUT);
}

/*******************************************************************************
函 数: void LSWResetAlarm(void)
功 能: 复位接近开关的报警

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::LSWResetAlarm(void)
{
    system.LSW_PosFlg = false;
    system.LSW_NagFlg = false;
}

/*******************************************************************************
函 数: Uchar OS_LimitSwitch(void)
功 能: 限位开关动作函数.内部函数.
返 回: 0--限位开关没有动作;1--限位开关动作.

张彦欣
2014.10.08
*******************************************************************************/
inline Uchar MaxStepper::LSW_OS_LimitSwitch(void)
{
    if( !config.EnLsw )             //限位功能被关闭.
    {
        system.LSW_PosFlg = false;
        system.LSW_NagFlg = false;
        return false;
    }
    
    if(config.RunDir==DIR_ZHENG)    //电机正转.
    {
        if(digitalRead(config.PIN_LswPos)==config.LswLevel)
        {                           //正转.增量限位开关动作.
            system.LSW_PosFlg=true; //置标志位,限位开关动作.
            return true;
        }
        else                        //限位开关没有动作.
        {
            return false;
        }                
    }
    else                            //电机反转.
    {
        if(digitalRead(config.PIN_LswNag)==config.LswLevel)
        {                           //反转.负方向限位动作.
            system.LSW_NagFlg=true; //置标志位,限位开关动作.
            return true;
        }
        else
        {
            return false;
        }
    }
}

/*******************************************************************************
函 数: void SoftLimit(Uchar _Cmd)
功 能: 启用软件限位功能.限制全局坐标的最大值和最小值.
参 数: true/false

张彦欣
2023.04.23
*******************************************************************************/
void MaxStepper::SoftLimit(Uchar _Cmd)
{
    if(_Cmd)
    {
        config.EnSoftLsw = true;
    }
    else
    {
        config.EnSoftLsw      = false;
        system.SoftLSW_PosFlg = false;
        system.SoftLSW_NagFlg = false;
    }
}

// 读取软件限位的使能状态
Uchar MaxStepper::SoftLimit(void)
{
    return (config.EnSoftLsw);
}

/*******************************************************************************
函 数: void SetSoftLimitMin(long MinValue)
功 能: 设置软件限位的最小值

张彦欣
2023.04.23
*******************************************************************************/
void MaxStepper::SetSoftLimitMin(long MinValue)
{
    config.SoftLswMin = MinValue;
}

/*******************************************************************************
函 数: void SetSoftLimitMax(long MaxValue)
功 能: 设置软件限位的最大值

张彦欣
2023.04.23
*******************************************************************************/
void MaxStepper::SetSoftLimitMax(long MaxValue)
{
    config.SoftLswMax = MaxValue;
}

/*******************************************************************************
函 数: void SoftLimitResetAlarm(void)
功 能: 复位软件限位的报警

张彦欣
2023.04.23
*******************************************************************************/
void MaxStepper::SoftLimitResetAlarm(void)
{
    system.SoftLSW_PosFlg = false;
    system.SoftLSW_NagFlg = false;
}

/*******************************************************************************
函 数: Uchar SoftLSW_OS_Limit(void)
功 能: 软件限位判断函数.内部函数.

张彦欣
2023.04.23
*******************************************************************************/
Uchar MaxStepper::SoftLSW_OS_Limit(void)
{
    if( !config.EnSoftLsw )         //关闭功能
    {
        system.SoftLSW_PosFlg = false;
        system.SoftLSW_NagFlg = false;
        return false;
    }
    
    if(config.RunDir==DIR_ZHENG)    //电机正转.
    {
        if( system.location>config.SoftLswMax )
        {                           //正转.最大值限制.
            system.SoftLSW_PosFlg=true;
            return true;
        }
        else
        {
            return false;
        }
    }
    else                            //电机反转.
    {
        if( system.location<config.SoftLswMin )
        {                           //反转.最小值限制.
            system.SoftLSW_NagFlg=true;
            return true;
        }
        else
        {
            return false;
        }
    }
}

/*******************************************************************************
函 数: void RampLength(long myRampLength)
功 能: 斜坡函数设置函数.
参 数: myRampLength为300RPM转速(5RPS)下,电动机加减速的长度.

张彦欣
2014.10.21
*******************************************************************************/
void MaxStepper::RampLength(Ulong myRampLength)
{
    if(myRampLength==0)
        return;
    ramp.RampLengthSET = myRampLength;
    
    // 用户设置的RampLength(加减速长度),是300RPM转速下的加减速长度
    // 要根据当前的速度,设置对应的实际加减速长度
    ramp.RampLength = (ramp.RampLengthSET)*(config.RPM)/300;
}

/*******************************************************************************
函 数: void EnRamp(Uchar myCMD)
功 能: 开启/关闭斜坡函数功能.
参 数: false-关闭;true-开启.

张彦欣
2014.10.21
*******************************************************************************/
void MaxStepper::EnRamp(Uchar myCMD)
{
	if(myCMD)
        ramp.EnRamp = true;
	else
        ramp.EnRamp = false;
}

/*******************************************************************************
函 数: void RampMode(Uchar _Mode)
功 能: 设置加减速模式.(0--S曲线加减速;1--直线加减速)

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::RampMode(Uchar _Mode)
{
    ramp.RampMode = _Mode;
}

/*******************************************************************************
函 数: void OS_LRamp(void)
功 能: 直线加减速函数.

张彦欣
2014.10.08
*******************************************************************************/
inline void MaxStepper::OS_LRamp(void)
{
    //恒速运行
    if( (system.CurrentSteps>ramp.RampLength)&&(system.RemainSteps>ramp.RampLength) )
    {
        system.State  = MOTOR_RUN;          
        system.CurFrq = config.TargetFrq;
        OS_LoadTimer();
        return;
    }

    //加速
    if( system.CurrentSteps<=system.RemainSteps )
    {                                   //已经运行的步数<=剩余的步数.
        system.State  = MOTOR_UP;       //运动的前半部分.加速过程中.
        system.CurFrq = (Ulong)(config.TargetFrq)*system.CurrentSteps/ramp.RampLength + (Ulong)(config.MinPlsFrq);
        
        if( system.CurFrq>config.TargetFrq )
            system.CurFrq=config.TargetFrq;
        
        OS_LoadTimer();
        return;
    }
    //减速
    else                                //已经运行的步数>剩余的步数.
    {                                   //运动的后半部分.减速过程中.
        system.State  = MOTOR_DOWN;
        system.CurFrq = (Ulong)(config.TargetFrq)*system.RemainSteps/ramp.RampLength + (Ulong)(config.MinPlsFrq);
        
        if( system.CurFrq>config.TargetFrq )
            system.CurFrq=config.TargetFrq;
        
        OS_LoadTimer();
        return;
    }
}

/*******************************************************************************
函 数: void OS_SRamp(void)
功 能: S曲线加减速函数.

张彦欣
2014.10.08
*******************************************************************************/
inline void MaxStepper::OS_SRamp(void)
{
    //恒速阶段
    if( (system.CurrentSteps>ramp.RampLength)&&(system.RemainSteps>ramp.RampLength) )
    {                                       //不在加减速的范围之内.
        system.State  = MOTOR_RUN;          //恒速运行
        system.CurFrq = config.TargetFrq;
        OS_LoadTimer();
        return;
    }

    if( system.CurrentSteps<=system.RemainSteps )
    {                                   //已经运行的步数<=剩余的步数.
        system.State  = MOTOR_UP;       //运动的前半部分.加速过程中.
        
        Ulong Index   = 100*system.CurrentSteps/ramp.RampLength;
        if( Index>=100 ) Index=99;
        
        system.CurFrq = config.TargetFrq*SRampTable[Index];
        
        if( system.CurFrq>config.TargetFrq )
            system.CurFrq=config.TargetFrq;
        if( system.CurFrq<config.MinPlsFrq )
            system.CurFrq=config.MinPlsFrq;
        
        OS_LoadTimer();
        return;
    }
    else                                //已经运行的步数>剩余的步数.
    {                                   //运动的后半部分.减速过程中.
        system.State  = MOTOR_DOWN;
        
        Ulong Index   = 100*system.RemainSteps/ramp.RampLength;
        if( Index>=100 ) Index=99;
        
        system.CurFrq = config.TargetFrq*SRampTable[Index];
        
        if( system.CurFrq>config.TargetFrq )
            system.CurFrq=config.TargetFrq;
        if( system.CurFrq<config.MinPlsFrq )
            system.CurFrq=config.MinPlsFrq;
        
        OS_LoadTimer();
        return;
    }
}

/*******************************************************************************
函 数: void OS_NRamp(void)
功 能: 无加减速.

张彦欣
2014.10.08
*******************************************************************************/
inline void MaxStepper::OS_NRamp(void)
{
    system.State  = MOTOR_RUN;  //恒速运行
    system.CurFrq = config.TargetFrq;
    OS_LoadTimer();
    return;
}

/*******************************************************************************
函 数: void OS_ClrTimerShareFlg(void)
功 能: 清零"共享定时器"标志位

张彦欣
2023.11.06
*******************************************************************************/
inline void MaxStepper::OS_ClrTimerShareFlg(void)
{
    system.TimerShareFlg = false;
}

/*******************************************************************************
函 数: void EnableTimerShare(Uchar _Cmd)
功 能: 开启定时器共享功能.允许多个电机使用同一个定时器中断.但是电机不能同时运行.
备 注: 原理是通过标志位控制来决定某一个电机控制引脚起作用.

张彦欣
2023.11.06
*******************************************************************************/
void MaxStepper::EnableTimerShare(Uchar _Cmd)
{
    config.TimerShareEnable = true;
    system.TimerShareFlg    = false;
}

/*******************************************************************************
函 数: void scan(void)
功 能: 这个函数是由定时器中断调用的函数.

张彦欣
2014.10.08
*******************************************************************************/
void MaxStepper::scan(void)
{
    //-----------------
    //使能定时器共享功能 多个scan()函数共用一个定制器中断
    if( config.TimerShareEnable )
    {
        if( system.TimerShareFlg==false )
        {
            return;
        }
    }
    
    //-----------------
    //电机已经运行结束
    //步进电机已经运行完毕设置的步数.
    if( system.RemainSteps==0 )
    {
        stop();
        OS_ClrTimerShareFlg();
        return;
    }
    
    //-----------------
    //已经发生了限位报警
    if( system.LSW_PosFlg || system.LSW_NagFlg )
    {
        system.State = MOTOR_ALARM;
        digitalWrite(config.PIN_Step, config.StopLevel);
        config.DrvTimer.close();
        OS_ClrTimerShareFlg();
        return;
    }
    if( system.SoftLSW_PosFlg || system.SoftLSW_NagFlg )
    {
        system.State = MOTOR_ALARM;
        digitalWrite(config.PIN_Step, config.StopLevel);
        config.DrvTimer.close();
        OS_ClrTimerShareFlg();
        return;
    }
    
    //-----------------
    //脉冲输出
    digitalWrite(config.PIN_Step, !config.StopLevel);
    
    //-----------------
    //速度斜坡函数动作
    //
    if(ramp.EnRamp)                     //开启斜坡功能.
    {
        if(ramp.RampMode==RAMP_MODE_S)  //S曲线加减速.
            OS_SRamp();
        else
            OS_LRamp();
    }
    else                                //关闭斜坡功能.
    {
        OS_NRamp();
    }

    //-----------------
    //限位开关函数
    if( LSW_OS_LimitSwitch() )          //限位开关发生动作
    {
        stop();
        OS_ClrTimerShareFlg();
        return;
    }
    
    //-----------------
    //运行步数计算
    if( system.RemainSteps>0 )          //剩余步数
        system.RemainSteps--;

    system.CurrentSteps++;              //步进电机已经运行的步数.

    if( config.RunDir==DIR_ZHENG )      //记录全局位置.
        system.location++;
    else
        system.location--;
    
    //-----------------
    //软件限位
    if( SoftLSW_OS_Limit() )            //软件限位发生报警
    {
        stop();
        OS_ClrTimerShareFlg();
        return;
    }

    //-----------------
    //停止脉冲输出
    digitalWrite(config.PIN_Step, config.StopLevel);
    
    // if( system.RemainSteps==0 )          //步进电机已经运行完毕设置的步数.
    // {                                    //V3.5版本修正
    // stop();                              //不能在此处停止电机
    // return;                              //否则最后一个脉冲就会变得非常的窄
    // }
}



