#include "DEVICE_CFG.H"
#include "UART.h"
#include <string.h>
#include <stdio.h> 
#include "FM175XX.h"
#include "FM175XX_REG.h"

#include "LPCD_API.h"
#include "LPCD_CFG.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_map.h"
int main(void)
{	  
unsigned char reg_data;

	RCC_Config();			//时钟配置
	GPIO_Config();		//GPIO配置
	
	USART3_Config();	//UART3配置
	
	SPI_Config();			//SPI配置
		
	EXTI_Config();  	//外部中断配置
			
	UART_Com_Para_Init(&UART_Com3_Para);    //初始化Uart1的通讯参数

	Uart_Send_Msg("FM17522,FM17550 LPCD DEMO\r\n");
	Uart_Send_Msg("Release Version 27_1 2017.06.16\r\n");
	
	LED_ARM_WORKING_ON;
	
	FM175XX_HardReset();

	GetReg(JREG_VERSION,&reg_data);
	if(reg_data == 0x88)
		Uart_Send_Msg("IC Version = V03\r\n");
	if(reg_data == 0x89)
		Uart_Send_Msg("IC Version = V03+\r\n");
	Uart_Send_Msg("Lpcd Debug Info Enable,Y or N?\r\n");
	
	if(Scan_User_Reply() == True)
		Lpcd_Debug_Info = 1;
	else
		Lpcd_Debug_Info = 0;
	
	Uart_Send_Msg("Lpcd Debug Test Enable,Y or N?\r\n");
	
	if(Scan_User_Reply() == True)
		Lpcd_Debug_Test = 1;
	else
		Lpcd_Debug_Test = 0;
	LED_ARM_WORKING_OFF;	

	if(Lpcd_Calibration_Event()== SUCCESS)//进行FM175XX校准		
		Lpcd_Calibration_Backup();
	else
		{
			while(1);
			//首次校准失败，错误处理
		}
	Lpcd_Set_Mode(1);//进入LPCD模式	
		
	while(1)
	{	
		PWR_EnterSTOPMode( PWR_Regulator_ON, PWR_STOPEntry_WFI); //MCU 进入STOP模式		
		Lpcd_Set_Mode(0);//退出LPCD 
		Lpcd_Get_IRQ(&Lpcd.Irq);		
		Lpcd_IRQ_Event();
		Lpcd_Set_Mode(1);//进入LPCD模式			
	}

}








