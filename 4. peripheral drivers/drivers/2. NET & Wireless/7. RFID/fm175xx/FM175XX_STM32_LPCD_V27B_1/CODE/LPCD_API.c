/*********************************************************************
*                                                                    *
*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      *
*   All rights reserved. Licensed Software Material.                 *
*                                                                    *
*   Unauthorized use, duplication, or distribution is strictly       *
*   prohibited by law.                                               *
*                                                                    *
*********************************************************************/

/********************************************************************/
/* 	FM175XX LPCD API V271函数库																			*/
/* 	主要功能:						        																		*/
/* 	    LPCD校准分为基准电压校准和输出驱动校准两个步骤         			*/
/*      LPCD中断触发后，调整触发阈值，不必重新校准        					*/
/*      提高LPCD检测灵敏度和不同校准状态下的一致性                  */
/*      阈值MAX增加偏移量，避免误触发                               */
/* 	编制:宋耀海 																										*/
/* 	编制时间:2017年9月7日																						*/
/* 																																	*/
/********************************************************************/
#include "FM175XX_REG.h"
#include "LPCD_API.h"
#include "LPCD_CFG.h"
#include "FM175XX.h"
#include "UART.h"
#include "DEVICE_CFG.h"
#include "Reader_API.h"
//********************************************************************
//内部函数列表
//********************************************************************
unsigned char Lpcd_Set_Timer(void);
unsigned char Lpcd_Set_Gain(unsigned char lpcd_gain);
unsigned char Lpcd_Set_Threshold(unsigned char lpcd_threshold_min,unsigned char lpcd_threshold_max);
unsigned char Lpcd_Set_Driver(unsigned char lpcd_cwp,unsigned char lpcd_cwn,unsigned char lpcd_tx2_en);
unsigned char Lpcd_Set_Reference(unsigned char lpcd_bias_current,unsigned char lpcd_reference);
unsigned char Lpcd_WaitFor_Irq(unsigned char IrqType);
unsigned char Lpcd_Get_Calibration_Value(unsigned char *value);
unsigned char Lpcd_Get_Value(unsigned char *value);
unsigned char Lpcd_Set_Aux(unsigned char mode);
void Lpcd_Calibration_Backup(void);
void Lpcd_Calibration_Restore(void);
unsigned char Lpcd_Debug_Info = 0;
unsigned char Lpcd_Debug_Test = 0;
void Lpcd_Set_IRQ_pin(void);
unsigned char Lpcd_Card_Event(void);
struct lpcd_struct Lpcd;

void Lpcd_Set_IRQ_pin(void)
{
  SetReg(JREG_COMMIEN,BIT7);//IRQ引脚反相输出			 
	SetReg(JREG_DIVIEN,BIT7);//IRQ引脚CMOS输出模式（IRQ引脚不需要外接上拉电阻）   
}

//***********************************************
//函数名称：Lpcd_Reset_Status()
//函数功能：LPCD功能复位
//入口参数：
//出口参数：SUCCESS：操作完成 
//***********************************************
unsigned char Lpcd_Reset_Status(void)//LPCD功能复位
{
    SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_RSTN);  //复位LPCD寄存器
    SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_RSTN); //复位放开LPCD寄存器
    return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_Init_Register()
//函数功能：LPCD寄存器初始化
//入口参数：
//出口参数：SUCCESS：操作完成  
//***********************************************
unsigned char Lpcd_Init_Register(void)
{
  Lpcd_Set_IRQ_pin();
	
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_EN);	//使能LPCD功能		
	
    if(LPCD_AUTO_DETECT_TIMES == 1)
        SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_SENSE_1);//配置1次检测
    else
        SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_SENSE_1);//配置3次检测
	
	SetReg_Ext(JREG_LPCD_CTRL3,(LPCD_MODE << 3) + BIT2);//配置LpcdCtrl3寄存器,LPCD工作模式	,BIT2默认为1
	SetReg_Ext(JREG_LPCD_AUTO_WUP_CFG,(LPCD_AUTO_WUP_EN << 3) + LPCD_AUTO_WUP_CFG );//配置Auto_Wup_Cfg寄存器 
                  
	return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_Set_Timer()
//函数功能：LPCD设置Timer
//入口参数：
//出口参数：SUCCESS：操作完成  
//***********************************************
unsigned char Lpcd_Set_Timer(void)
{
		Lpcd.Timer1 = LPCD_TIMER1_CFG & 0x0F;//TIMER1 = 0x01~0x0F
		Lpcd.Timer2 = LPCD_TIMER2_CFG & 0x1F;//TIMER2 = 0x01~0x1F
		Lpcd.Timer3 = LPCD_TIMER3_CFG & 0x1F;//TIMER3 = 0x03~0x1F
		Lpcd.TimerVmid = LPCD_TIMER_VMID_CFG & 0x1F;
    if (Lpcd.Timer3 > 0xF) //Timer3Cfg用到5bit，选择16分频
        {
        Lpcd.Timer3_Offset = 0x05;
				Lpcd.Timer3_Div  = 2;			//16分频
				Lpcd.Fullscale_Value =  ((Lpcd.Timer3 - 1)<<3) - Lpcd.Timer3_Offset;	
        }
	else if(Lpcd.Timer3 > 0x7) //Timer3Cfg用到4bit，选择8分频
        {
        Lpcd.Timer3_Offset = 0x0E;
				Lpcd.Timer3_Div  = 1;			//8分频
				Lpcd.Fullscale_Value =  ((Lpcd.Timer3 - 1)<<4) - Lpcd.Timer3_Offset;	
        }
	else 
        {
        Lpcd.Timer3_Offset = 0x1F;
				Lpcd.Timer3_Div  = 0;			//4分频
				Lpcd.Fullscale_Value =  ((Lpcd.Timer3 - 1)<<5) - Lpcd.Timer3_Offset;	
        }
	  Lpcd.Calibration.Range_L = LPCD_THRESHOLD;// 
    Lpcd.Calibration.Range_H = Lpcd.Fullscale_Value - LPCD_THRESHOLD;// 
		SetReg_Ext(JREG_LPCD_T1CFG,(Lpcd.Timer3_Div <<4) + Lpcd.Timer1);//配置Timer1Cfg寄存器
		SetReg_Ext(JREG_LPCD_T2CFG,Lpcd.Timer2);//配置Timer2Cfg寄存器
		SetReg_Ext(JREG_LPCD_T3CFG,Lpcd.Timer3);//配置Timer3Cfg寄存器	
    SetReg_Ext(JREG_LPCD_VMIDBD_CFG,Lpcd.TimerVmid);//配置VmidBdCfg寄存器
    return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_Set_Gain()
//函数功能：LPCD设置幅度检波包络信号的放大、衰减增益
//入口参数：
//出口参数：SUCCESS：操作完成  
//***********************************************
unsigned char Lpcd_Set_Gain(unsigned char lpcd_gain)
{
	lpcd_gain &= 0x1F;
	SetReg_Ext(JREG_LPCD_CTRL4,lpcd_gain);
    return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_Set_Threshold()
//函数功能：LPCD设置触发阈值
//入口参数：
//出口参数：SUCCESS：操作完成  
//***********************************************
unsigned char Lpcd_Set_Threshold(unsigned char lpcd_threshold_min,unsigned char lpcd_threshold_max)
{
unsigned char temp;
       
	if(lpcd_threshold_max < lpcd_threshold_min)
        {
        temp = lpcd_threshold_min;
        lpcd_threshold_min = lpcd_threshold_max;
        lpcd_threshold_max = temp;
        }
  
  SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_L,(lpcd_threshold_min & 0x3F));//写入THRESHOLD_Min阈值低6位

  SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_H,(lpcd_threshold_min>>6));//写入THRESHOLD_Min阈值高2位

	SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_L,(lpcd_threshold_max & 0x3F));//写入THRESHOLD_Max阈值低6位

	SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_H,(lpcd_threshold_max>>6));//写入THRESHOLD_Max阈值高2位
        
  return SUCCESS;

}

//***********************************************
//函数名称：Lpcd_Set_Drive()
//函数功能：LPCD设置输出驱动
//入口参数：
//出口参数：SUCCESS：操作完成  
//***********************************************
unsigned char Lpcd_Set_Driver(unsigned char lpcd_cwp,unsigned char lpcd_cwn,unsigned char lpcd_tx2_en)
{
    unsigned char reg_data;
	GetReg(JREG_VERSION,&reg_data);
    if(reg_data == 0x88)//V03版本芯片
        {
        if(lpcd_cwn > 1)
            lpcd_cwn = 1;
        lpcd_cwn &= 0x01;
        lpcd_cwp &= 0x07;
        SetReg_Ext(JREG_LPCD_CTRL2,((lpcd_tx2_en << 4) + (lpcd_cwn << 3) + lpcd_cwp));//V03版本芯片
        }
    if(reg_data == 0x89)//V03以上版本芯片
        {
          lpcd_cwn &= 0x0F;
          lpcd_cwp &= 0x07;
          SetReg_Ext(JREG_LPCD_CTRL2,((lpcd_tx2_en<<4) + lpcd_cwp));//V03以上版本芯片
          SetReg(JREG_GSN, lpcd_cwn << 4); //V03以上版本芯片
        }
  return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_Set_Reference()
//函数功能：LPCD设置基准信号的充电电流与充电电容
//入口参数：
//出口参数：SUCCESS：校准成功；ERROR：校准失败
//***********************************************
unsigned char Lpcd_Set_Reference(unsigned char lpcd_bias_current,unsigned char lpcd_reference)
{
		lpcd_bias_current &= 0x07; //0x00 ~ 0x07 基准电流
    lpcd_reference &= 0x7F;  //0x00 ~ 0x7F 基准电容
    SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((lpcd_reference  & 0x40)>>1) + (lpcd_bias_current & 0x07));       
    SetReg_Ext(JREG_LPCD_ADC_REFERECE,(lpcd_reference  & 0x3F));
    return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_Calibrate_Reference()
//函数功能：LPCD设置校准基准电压
//入口参数：
//出口参数：SUCCESS：校准成功；ERROR：校准失败
//***********************************************
unsigned char Lpcd_Calibrate_Reference(void)
{
    unsigned char i,result; 
    result = ERROR; //初始值设置为校准失败	
    Lpcd_Reset_Status();//校准基准电压前，先复位状态机
    for(i = 0 ;i < 0x7F;i ++)
    {
    Lpcd.Calibration.Reference = 0 + i;//
    Lpcd_Set_Reference(LPCD_BIAS_CURRENT,Lpcd.Calibration.Reference);//
    
    Lpcd.Calibration.Gain_Index = 10;//
    Lpcd_Set_Gain(LPCD_GAIN[Lpcd.Calibration.Gain_Index]);//
    
    Lpcd_Set_Driver(7,15,1);//
    Lpcd_Get_Calibration_Value(&Lpcd.Calibration.Value);
		if(Lpcd_Debug_Info)
			{
			Uart_Send_Msg("-> Refference = "); Uart_Send_Hex(&i,1); //Uart_Send_Msg("\r\n");
			Uart_Send_Msg("; Lpcd.Calibration.Value = "); Uart_Send_Hex(&Lpcd.Calibration.Value,1); Uart_Send_Msg("\r\n");
			}
    if((Lpcd.Calibration.Value == 0)&&(Lpcd.Calibration.Reference != 0))
      {
				Uart_Send_Msg("Calibra Reference Success!\r\n");	
        result = SUCCESS;
        break;//
      }
    if((Lpcd.Calibration.Value == 0)&&(Lpcd.Calibration.Reference == 0))
      {
        Uart_Send_Msg("Calibra Reference Error!\r\n");	
        result = ERROR; //
        break;
      }
    }
   return result;
}
//***********************************************
//函数名称：Lpcd_Calibrate_Driver()
//函数功能：LPCD设置校准输出驱动
//入口参数：
//出口参数：SUCCESS：操作完成  
//***********************************************
unsigned char Lpcd_Calibrate_Driver(void)
{
    unsigned char i,j;   
		
    Lpcd.Calibration.Gain_Index = LPCD_GAIN_INDEX ;//      
    
   for(j = 0;j < 11;j++)//
   { 
		Lpcd_Set_Gain(LPCD_GAIN[Lpcd.Calibration.Gain_Index]);// 设置Gain
    for(i = 0;i < 7;i ++ )//
       {
       Lpcd.Calibration.Driver_Index = i;
       Lpcd_Set_Driver(LPCD_P_DRIVER[Lpcd.Calibration.Driver_Index],LPCD_N_DRIVER[Lpcd.Calibration.Driver_Index],LPCD_TX2RF_EN);//
       Lpcd_Get_Calibration_Value(&Lpcd.Calibration.Value);//设置Driver
			 if(Lpcd_Debug_Info)
			 {				 
       Uart_Send_Msg("-> Driver = "); Uart_Send_Hex(&i,1); //Uart_Send_Msg("\r\n");
       Uart_Send_Msg("; Lpcd.Calibration.Value = "); Uart_Send_Hex(&Lpcd.Calibration.Value,1); Uart_Send_Msg("\r\n");
			 } 
       if((Lpcd.Calibration.Value > Lpcd.Calibration.Range_L)&&(Lpcd.Calibration.Value < Lpcd.Calibration.Range_H))
         {
           if((Lpcd.Calibration.Value - LPCD_THRESHOLD > 0 )&&((Lpcd.Calibration.Value + LPCD_THRESHOLD) < Lpcd.Fullscale_Value))//
             
           {
               Lpcd.Calibration.Threshold_Max = Lpcd.Calibration.Value + LPCD_THRESHOLD;
               Lpcd.Calibration.Threshold_Min = Lpcd.Calibration.Value - LPCD_THRESHOLD;
						 
               Uart_Send_Msg("-> Lpcd.Calibration.Value = "); Uart_Send_Hex(&Lpcd.Calibration.Value,1); //Uart_Send_Msg("\r\n");//
						   Uart_Send_Msg("; Lpcd.Fullscale_Value = "); Uart_Send_Hex(&Lpcd.Fullscale_Value,1); Uart_Send_Msg("\r\n");
						   Uart_Send_Msg("-> Driver = "); Uart_Send_Hex(&Lpcd.Calibration.Driver_Index,1);Uart_Send_Msg("; Gain = ");  Uart_Send_Hex(&Lpcd.Calibration.Gain_Index,1); Uart_Send_Msg("\r\n");
               Uart_Send_Msg("-> Lpcd.Calibration.Threshold_Max = "); Uart_Send_Hex(&Lpcd.Calibration.Threshold_Max,1); //
               Uart_Send_Msg("; Lpcd.Calibration.Threshold_Min = "); Uart_Send_Hex(&Lpcd.Calibration.Threshold_Min,1); Uart_Send_Msg("\r\n");//
               Lpcd_Set_Threshold(Lpcd.Calibration.Threshold_Min,Lpcd.Calibration.Threshold_Max);//
						   if(Lpcd_Debug_Test)
								 {
									while(1)//此循环为测试代码，供调试时使用
										 {
											 Lpcd_Get_Calibration_Value(&Lpcd.Calibration.Value);//
											 Uart_Send_Msg("-> Lpcd.Calibration.Value2 = "); Uart_Send_Hex(&Lpcd.Calibration.Value,1); Uart_Send_Msg("\r\n");
											 mDelay(500);
										 }
								 }
               Uart_Send_Msg("Calibra Driver Success!\r\n");	        
               return SUCCESS;
             }
         }	 
				 
				 
       }
			 
			  if(Lpcd.Calibration.Value > Lpcd.Calibration.Range_H)//
        {
          if(Lpcd.Calibration.Gain_Index == 11)
            break;//
          else
            Lpcd.Calibration.Gain_Index++;
           Uart_Send_Msg("Gain_Index++\r\n");
        }
      if(Lpcd.Calibration.Value < Lpcd.Calibration.Range_L)//
        {     
          if(Lpcd.Calibration.Gain_Index == 0)
            break;//
          else
             Lpcd.Calibration.Gain_Index--;
           Uart_Send_Msg("Gain_Index--\r\n");
        }
				
		}		
    Uart_Send_Msg("Calibra Driver Error !\r\n");//
    return ERROR;
}

//***********************************************
//函数名称：WaitForLpcdIrq()
//函数功能：等待LPCD中断
//入口参数：IrqType
//出口参数：SUCCESS：读取操作完成   ERROR：超时退出
//***********************************************
unsigned char Lpcd_WaitFor_Irq(unsigned char IrqType)
{
	unsigned char ExtRegData;
	unsigned char TimeOutCount;
	
	TimeOutCount = 0;
	GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);

	for(TimeOutCount = LPCD_CALIBRATE_IRQ_TIMEOUT; TimeOutCount > 0; TimeOutCount--)
    {
		uDelay(1);	//延时1us
		GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);	
		if(ExtRegData & IrqType)
			return SUCCESS; 
    }        
	return ERROR;	
}

//***********************************************
//函数名称：Lpcd_Get_Calibration_Value()
//函数功能：进行一次校准并读取LPCD数值
//入口参数： *value
//出口参数：SUCCESS：读取操作完成   ERROR:操作超时
//***********************************************
unsigned char Lpcd_Get_Calibration_Value(unsigned char *value)
{
	unsigned char result;
      	    
  SetReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIBRATE_VMID_EN);//使能VMID电源，BFL_JBIT_AMP_EN_SEL = 1 提前使能AMP	Lpcd_Reset_Status();  //清除CalibraIRq标志
  SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_CALIBRATE_EN);//关闭校准模式
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_CALIBRATE_EN);//使能校准模式
	
	result = Lpcd_WaitFor_Irq(JBIT_CALIBRATE_IRQ);//等待校准结束中断      

  ModifyReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIBRATE_VMID_EN,0);//关闭VMID电源 
	
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_CALIBRATE_EN);//关闭校准模式
	
	Lpcd_Get_Value(&*value);
	
  Lpcd_Reset_Status();  //清除CalibraIRq标志

	return result;
}

//***********************************************
//函数名称：Lpcd_Get_Value()
//函数功能：读取LPCD的数值
//入口参数： *value
//出口参数：SUCCESS：读取操作完成   ERROR:操作超时
//***********************************************

unsigned char Lpcd_Get_Value(unsigned char *value)
{
	unsigned char ExtRegData;
	GetReg_Ext(JREG_LPCD_ADC_RESULT_H,&ExtRegData);//读取幅度信息，高2位
	
	*value = ((ExtRegData & 0x3) << 6);

	GetReg_Ext(JREG_LPCD_ADC_RESULT_L,&ExtRegData);//读取幅度信息，低6位

	*value += (ExtRegData & 0x3F);  

	return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_IRQ_Event()
//函数功能：LPCD中断处理
//入口参数：
//出口参数：
//***********************************************
void Lpcd_IRQ_Event(void)
{	
	LED_ARM_WORKING_ON;		
	
	if(Lpcd.Irq & JBIT_AUTO_WUP_IRQ)
		{
			Lpcd_Calibration_Event();
		}
	if(Lpcd.Irq & JBIT_CARD_IN_IRQ)//如果有卡片中断，进入读卡操作
        {
					if((Lpcd.Value + LPCD_THRESHOLD)> Lpcd.Fullscale_Value )
						Lpcd.Calibration.Threshold_Max = Lpcd.Fullscale_Value;
					else						
						Lpcd.Calibration.Threshold_Max = Lpcd.Value + LPCD_THRESHOLD;
          if((Lpcd.Value - LPCD_THRESHOLD) > 0)
						Lpcd.Calibration.Threshold_Min = Lpcd.Value - LPCD_THRESHOLD;
          else
            Lpcd.Calibration.Threshold_Min = 0;
         
          Uart_Send_Msg("-> Lpcd.Calibration.Threshold_Max = "); Uart_Send_Hex(&Lpcd.Calibration.Threshold_Max,1); //
          Uart_Send_Msg("; Lpcd.Calibration.Threshold_Min = "); Uart_Send_Hex(&Lpcd.Calibration.Threshold_Min,1); Uart_Send_Msg("\r\n");//
          Lpcd_Set_Threshold(Lpcd.Calibration.Threshold_Min,Lpcd.Calibration.Threshold_Max);//LPCD配置触发阈值 
					Lpcd_Card_Event();//读卡流程						
	
				}   
	 Lpcd_Set_Driver(LPCD_P_DRIVER[Lpcd.Calibration.Driver_Index],LPCD_N_DRIVER[Lpcd.Calibration.Driver_Index],LPCD_TX2RF_EN);//配置LPCD输出驱动
	 Lpcd_Set_Reference(LPCD_BIAS_CURRENT,Lpcd.Calibration.Reference);//配置LPCD基准电容
	 Lpcd_Set_Gain(LPCD_GAIN[Lpcd.Calibration.Gain_Index]);//配置LPCD接收增益
	 Lpcd_Set_IRQ_pin();				
	 Lpcd.Irq = 0;//清除LPCD中断标志
		
   LED_ARM_WORKING_OFF;   	
	return;	
}

void Lpcd_Calibration_Restore(void)
{
	Lpcd.Calibration.Reference = Lpcd.Calibration_Backup.Reference;
	Lpcd.Calibration.Gain_Index = Lpcd.Calibration_Backup.Gain_Index;
  Lpcd.Calibration.Driver_Index = Lpcd.Calibration_Backup.Driver_Index;
	Uart_Send_Msg("-> Restore Success!\r\n");		
	return;
}

void Lpcd_Calibration_Backup(void)
{
	Lpcd.Calibration_Backup.Reference = Lpcd.Calibration.Reference;
	Lpcd.Calibration_Backup.Gain_Index = Lpcd.Calibration.Gain_Index;
  Lpcd.Calibration_Backup.Driver_Index = Lpcd.Calibration.Driver_Index;
	Uart_Send_Msg("-> Bakckup Success!\r\n");		
	return;
}
//***********************************************
//函数名称：Lpcd_Calibration_Event()
//函数功能：LCPD校准程序
//入口参数：无
//出口参数：SUCCESS:校准成功；ERROR：校准失败
//***********************************************
unsigned char Lpcd_Calibration_Event(void)
{
	unsigned char result;
  
	Lpcd_Init_Register();		//
	Lpcd_Set_Timer();
	Lpcd_Set_Aux(LPCD_AUX_EN);	//??AUX????

  Uart_Send_Msg("Start Calibration!\r\n");    
  Lpcd_Reset_Status();
  result = Lpcd_Calibrate_Reference();
     if(result == ERROR)
      return ERROR;
   
	result = Lpcd_Calibrate_Driver();
   
		if (result == SUCCESS)		
      {	
			  Uart_Send_Msg("Calibration Success!\r\n");
        return SUCCESS;
			} 
		else
			{
			  Uart_Send_Msg("Calibration Error!\r\n");
				Lpcd_Calibration_Restore();//恢复校准参数
			}	 
     
return ERROR;
}

//***********************************************
//函数名称：Lpcd_Set_Mode()
//函数功能：LCPD工作模式设置
//入口参数：mode = 1:开启 mode = 0 :关闭
//出口参数：
//***********************************************
void Lpcd_Set_Mode(unsigned char mode)
{
    if(mode == 1)
        {
        Lpcd_Set_IE(1);//FM175XX打开LPCD中断               
        FM175XX_HPD(0);//进入LPCD模式	 
        Uart_Send_Msg("LPCD Mode Entered!\r\n\r\n");       
				EXTI15_10_IRQ_CTRL(ENABLE);//MCU打开中断
        }
    else
        {               
				FM175XX_HPD(1);//退出LPCD模式	
        Lpcd_Set_IE(0);//FM75XX关闭LPCD中断
        EXTI15_10_IRQ_CTRL(DISABLE);//MCU关闭中断
        Uart_Send_Msg("LPCD Mode Exited!\r\n");  
        }     
    return;
}

//***********************************************
//函数名称：Lpcd_Get_IRQ()
//函数功能：LPCD读取中断标志
//入口参数：*irq IRQ标志
//出口参数:
//***********************************************
void Lpcd_Get_IRQ(unsigned char *irq)
{
  GetReg_Ext(JREG_LPCD_IRQ,&(*irq));//读取LPCD中断寄存器

	Uart_Send_Msg("-> Lpcd.Irq = "); Uart_Send_Hex(&*irq,1); 
	if(Lpcd.Irq & JBIT_CARD_IN_IRQ)
	{
		Lpcd_Get_Value(&Lpcd.Value);
		Uart_Send_Msg("; Lpcd.Value = "); Uart_Send_Hex(&Lpcd.Value,1);
	}
	Uart_Send_Msg("\r\n");
	Lpcd_Reset_Status();//清除FM175XX中断状态
	return;
}
//***********************************************
//函数名称：Lpcd_Set_IE()
//函数功能：LPCD中断引脚设置
//入口参数：mode = 1:开启 mode = 0 ：关闭
//出口参数：SUCCESS：操作完成
//***********************************************
unsigned char Lpcd_Set_IE(unsigned char mode)
{
    if(mode == 1)
        {
       	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_IE);//打开LPCD中断					
        }
    else
        {        
        SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_IE);//关闭LPCD中断			
        }
    return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_Set_Aux()
//函数功能：LPCD测试通道功能
//入口参数：mode = 1:开启 mode = 0 ：关闭
//出口参数：SUCCESS：操作完成
//***********************************************

unsigned char Lpcd_Set_Aux(unsigned char mode)
{
	if (mode == 1) 
        {
        ModifyReg_Ext(0x39,BIT7,1);			
				ModifyReg_Ext(0x3C,BIT1,0);	
				ModifyReg_Ext(0x3D,(BIT2 | BIT1 | BIT0),0);				
				ModifyReg_Ext(0x3E,(BIT1 | BIT0),0);
				SetReg(JREG_ANALOGTEST,0x00);
				SetReg_Ext(0x13,BIT0);//Aux1(pin19) = V_demodulation包络检波信号输出
				SetReg_Ext(0x14,BIT1);//Aux2(pin20)= Reference电压信号输出
				SetReg_Ext(0x15,0x00);//D2输出LPCD_OUT
				SetReg_Ext(0x16,(BIT4 | BIT3 ));//D1(pin25) = T3 ； D2(pin26) = LPCD_OUT    
				SetReg(JREG_TESTPINEN,0x86);//使能D1，D2 输出             
        }
	else
        {   
				SetReg_Ext(0x13,0x00);
				SetReg_Ext(0x14,0x00);
        SetReg_Ext(0x15,0x00);	
        SetReg_Ext(0x16,0x00);	
        SetReg(JREG_TESTPINEN,0x80);//关闭D1；D2 输出
        }
	return SUCCESS;
}

//***********************************************
//函数名称：Lpcd_Card_Event()
//函数功能：LPCD卡片操作
//入口参数：
//出口参数：SUCCESS：操作完成
//***********************************************
unsigned char Lpcd_Card_Event(void)
{
	unsigned char card_type,result;
				FM175XX_SoftReset();//??????????	
					SetRf(3);		
					result = FM175XX_Polling(&card_type);				
					SetRf(0);
				
					if(result == SUCCESS)			
						{		
						if(card_type & BIT0)
								{
								Uart_Send_Msg("-> TYPE_A Card Detected!\r\n");	
								Uart_Send_Msg("-> ATQA = ");Uart_Send_Hex(Type_A.ATQA, 2);//Uart_Send_Msg("\r\n");					
								Uart_Send_Msg("; UID = ");Uart_Send_Hex(Type_A.UID, 4 * Type_A.CASCADE_LEVEL);//Uart_Send_Msg("\r\n");					
								Uart_Send_Msg("; SAK = ");Uart_Send_Hex(Type_A.SAK, Type_A.CASCADE_LEVEL);Uart_Send_Msg("\r\n");
								}							
						if(card_type & BIT1)
								{		
									Uart_Send_Msg("-> TYPE_B Card Detected!\r\n");
									Uart_Send_Msg("-> ATQB = ");Uart_Send_Hex(Type_B.ATQB,12);Uart_Send_Msg("\r\n");	
									Uart_Send_Msg("-> UID = ");
									Uart_Send_Hex(Type_B.UID,8);
									Uart_Send_Msg("\r\n");
								}
						}
												
					else
						{
						Uart_Send_Msg("-> No Card Detected!\r\n");
						}
	return result;
}

