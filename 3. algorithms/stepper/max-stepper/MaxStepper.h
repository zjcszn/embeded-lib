/*******************************************************************************
                        青岛市张彦欣单片机有限公司
                            版权所有  @ 2014
    
    MaxStepper库用于驱动步进电机或伺服电机.输出信号分为脉冲信号Pls和方向引脚Dir.
必须接步进电机驱动器或伺服电机驱动器.

张彦欣
2014.10.08
*******************************************************************************/
#ifndef  _MAX_STEPPER_H_
#define  _MAX_STEPPER_H_



#include "MaxDuino.h"
#include "MaxTimer.h"


#define  DIR_FOREWARD           0   //正转
#define  DIR_REVERSAL           1   //反转

// 为了便于工程上的同事读取正反 使用拼音标记
// 2018.10.01
#define  DIR_ZHENG              DIR_FOREWARD
#define  DIR_FAN                DIR_REVERSAL


#define  RAMP_MODE_S            0   //加减速模式.S曲线加减速.
#define  RAMP_MODE_L            1   //直线加减速.


#define  DEFAULT_MIN_PLS_FREQ   10  //默认最低脉冲频率.


#define  MOTOR_STOP             0   //电机停止中.
#define  MOTOR_UP               1   //电机加速中.
#define  MOTOR_RUN              2   //电机恒速中.
#define  MOTOR_DOWN             3   //电机减速中.
#define  MOTOR_ALARM            4   //报警


struct STRUCT_RAMP
{
    Uchar    EnRamp;                //斜坡功能使能标志.
    Uchar    RampMode;              //斜坡模式.
	Ulong    RampLength;            //斜坡函数--加速的长度.  
    
    Ulong    RampLengthSET;         //用户设置的300RPM下的加减速长度
};

struct STRUCT_SYSTEM
{
    Ulong   RemainSteps;            //剩余步数.
    Ulong   CurrentSteps;           //当前步数.
    
	float   CurFrq;                 //记录当前速度.
	Uchar   State;                  //电机运行状态(加减速状态).
    
    long    location;		        //记录当前位置.
    
    Uchar   LSW_PosFlg;		        //限位开关动作标志位(正方向)
    Uchar   LSW_NagFlg;		        //限位开关动作标志位(反方向)
    
    Uchar   SoftLSW_PosFlg;         //软件限位报警(正方向)
    Uchar   SoftLSW_NagFlg;         //软件限位报警(反方向)
    
    Uchar   TimerShareFlg;          //定时器共用
};

struct STRUCT_CONFIG
{
    MaxTimer    DrvTimer;           //定义一个步进电机使用的定时计数器变量.
    
    Uchar   PIN_LswPos;             //增量限位开关引脚
	Uchar   PIN_LswNag;             //减量限位开关引脚
    
    Uint    PIN_Step;               //驱动引脚.脉冲引脚.
    Uint    PIN_Dir;                //驱动引脚.方向引脚.
    
    Uint    PlsPerCir;              // 步进电机转动一圈,所需要的脉冲数量.
    
    Uchar   DirOpp;		            //运动方向是否反向.
    Uchar   RunDir;		            //电机的转动方向.
    
    float   RPM;                    //电机的转速._RPM.    
    float   TargetFrq;              //目标频率.
    
    Uchar   StopLevel;              //当步进电机停止运转时,输出的电平.
                                    //默认高电平.

    Uchar   EnLsw;                  //限位开关使能标志位.
    Uchar   LswLevel;               //当限位开关动作时的电平.默认高电平.
	
    Uchar   EnSoftLsw;              //使能软件限位
    long    SoftLswMax;             //软件限位.最大值.
    long    SoftLswMin;             //软件限位.最小值.
    
    float   MinPlsFrq;              //脉冲引脚输出的脉冲最低频率.
    
    Uchar   TimerShareEnable;       //使能定时器共用功能
};


class MaxStepper 
{
  public:

	void  begin( TIM_TypeDef* myTimer, 
                 Uint  myPlusPerCir, 
                 Uint  myPls,  Uint  myDir,
                 Ulong PlsMod, Ulong DirMod,
                 Uchar PlsStopLvl );
    void  begin( TIM_TypeDef* myTimer, 
                 Uint myPlusPerCir, 
                 Uchar myPls, Uchar myDir );


	void  speed(float _RPM);               		//设置速度.单位:_RPM.
    void  Speed(float _RPM);
    float speed(void);
    float Speed(void);
    
    void  MinSpeed(float _RPM);              	//设置系统的最低转速.加减速的时候起作用.
    float CurSpeed();                           //获取当前速度.单位:_RPM.

	void  run(Uchar Dir, Ulong Steps);       	//电机运行一定的步数.
    void  run(long Steps);
    void  Run(Uchar Dir, Ulong Steps);
    void  Run(long Steps);
    
	void  stop(void);                        	//立即无条件停止电机.
    void  Stop(void);
    
    void  RStop(void);                       	//斜坡停止功能.

    Uchar state(void);                      	//获取电机运行状态.
    Uchar State(void);
    
    void  wait(void);							//等待电机停止.
    void  wait(Ulong OVTime);                   //带超时的等待.

    Uchar swait(Uchar ulPin,Uchar Level);       //等待电机停止,指定端口动作退出.
    Uchar swait(Uchar ulPin,Uchar Level,Uint FilterTime);

    void  LSW(Uchar _Cmd);
	void  LSWPin(Uchar myPin1, Uchar myPin2);   //限位开关操作函数.
	void  LSWLevel(Uchar mySetLevel);			//接近开关动作时的电平.
    void  LSWResetAlarm(void);                  //复位接近开关的报警
    
	void  EnRamp(Uchar myCMD);	                //开启斜坡功能.    
	void  RampLength(Ulong myRampLength);       //斜坡函数设置.
    void  RampMode(Uchar _Mode);                //加减速模式.
    
    void  SoftLimit(Uchar _Cmd);                //启用软件限位
    Uchar SoftLimit(void);                      //读取软件限位的使能状态
    void  SetSoftLimitMin(long MinValue);       //设置软件限位的最小值
    void  SetSoftLimitMax(long MinValue);       //设置软件限位的最大值
    void  SoftLimitResetAlarm(void);            //复位软件限位的报警
    
    void  EnableTimerShare(Uchar _Cmd);         //开启定时器共享功能
    
	long  location();							//获取当前位置.
	void  location(long CurLocation);			//获取当前位置.
	
	void  DirOpp(Uchar _Cmd);					//运动方向是否反向.
    Uchar DirOpp(void);                         //获取运动方向是否反向
	Uchar CurDir(void);                         //获取当前运动方向.
    
	void  MoveTo(long TargetLocation);			//移动到目标全局位置.
	
    void  scan(void);		//步进电机驱动函数.在ISR中运行.
							//并非用户调用.
    
    MaxStepper(void);		//构造函数.完成变量初始化.
	
	
private:

    MaxTimer        DrvTimer;       //定时计数器变量.
	
    STRUCT_RAMP     ramp;
    STRUCT_SYSTEM   system;
    STRUCT_CONFIG   config;
    
    void    OS_LoadTimer(void);     //定时器加载频率
    
    void    OS_LRamp(void);         //直线加减速.
    void    OS_SRamp(void);         //S曲线加减速.
    void    OS_NRamp(void);         //无加减速.
    
    void    OS_Direction(void);         //电机运动方向控制.
    Uchar   LSW_OS_LimitSwitch(void);   //限位开关函数.
    Uchar   SoftLSW_OS_Limit(void);     //软件限位
    void    OS_ClrTimerShareFlg();      //清零共享定时器标志位
};



#endif




