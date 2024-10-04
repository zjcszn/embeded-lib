#ifndef _LPCD_API_H
#define _LPCD_API_H

extern unsigned char Lpcd_Debug_Info;
extern unsigned char Lpcd_Debug_Test;

static const unsigned char LPCD_GAIN[11] = {0x00,0x02,0x01,0x03,0x07,0x0B,0x0F,0x13,0x17,0x1B,0x1F};
																					//0.4x 0.5x 0.6x 1.0x 1.2x 1.4x 2.0x 2.2x 2.6x 3.2x 4.0x
static const unsigned char LPCD_P_DRIVER[7]={0x01,0x02,0x03,0x04,0x05,0x06,0x07};
static const unsigned char LPCD_N_DRIVER[7]={0x01,0x04,0x06,0x08,0x0A,0x0C,0x0F};
//0=180欧姆；1=90欧姆；2=46欧姆；3=23欧姆；4=12欧姆；5=6欧姆；6=3欧姆；7=1.5欧姆；
//static const unsigned char LPCD_N_DRIVE[7]={0x01,0x02,0x03,0x04,0x05,0x06,0x07};
//static const unsigned char LPCD_P_DRIVE[16]={0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07};
//0=180欧姆；1=90欧姆；2=46欧姆；3=23欧姆；4=12欧姆；5=6欧姆；6=3欧姆；7=1.5欧姆；
//static const unsigned char LPCD_N_DRIVE[16]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};


struct calibration_struct
{
 unsigned char Reference;//基准电容
 unsigned char Value;    //LPCD校准数值
 unsigned char Threshold_Max;//LPCD阈值
 unsigned char Threshold_Min;//LPCD阈值
 unsigned char Gain_Index;//LPCD校准Gain的序号
 unsigned char Driver_Index;	//LPCD校准Drive的序号
 unsigned char Range_L;	
 unsigned char Range_H;
};

struct calibration_backup_struct
{
 unsigned char Reference;//基准电容
 unsigned char Gain_Index;//LPCD校准Gain的序号
 unsigned char Driver_Index;	//LPCD校准Drive的序号
};

struct lpcd_struct
{
 unsigned char Timer1;//T1休眠时间
 unsigned char Timer2;//T2启动时间
 unsigned char Timer3;//T3发射时间
 unsigned char Timer3_Div ;//T3分频系数
 unsigned char Timer3_Offset;
 unsigned char TimerVmid;//VMID启动时间
 unsigned char Fullscale_Value;	//T3时间的满程数值
 unsigned char Value;	
 unsigned char Irq;//LPCD中断	
 struct calibration_struct Calibration;
 struct calibration_backup_struct Calibration_Backup;
};

extern struct lpcd_struct Lpcd;
extern unsigned char Lpcd_Init_Register(void);
extern unsigned char Lpcd_Set_Aux(unsigned char mode);
extern unsigned char Lpcd_Set_IE(unsigned char mode);
extern unsigned char Lpcd_Reset_Status(void);//LPCD功能复位
extern void Lpcd_IRQ_Event(void);
extern unsigned char Lpcd_Calibration_Event(void);
extern void Lpcd_Get_IRQ(unsigned char *irq);
extern void Lpcd_Set_Mode(unsigned char mode);
extern void Lpcd_Calibration_Backup(void);
extern void Lpcd_Calibration_Restore(void);
#endif


