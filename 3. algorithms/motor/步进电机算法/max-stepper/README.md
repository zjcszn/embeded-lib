# MaxStepper

#### 介绍
步进电机(伺服电机)驱动库. 通过脉冲/方向引脚控制电机, 自动实现加减速/限位等功能. 该库是位置型算法, 适合需要精确定位的场合.


#### 软件架构
软件架构说明


#### 安装教程

本程序为一个电机控制类,如果你的编译器不支持Class,需要自行修改.

#### 使用说明

 **//定义一个电机实例 MOTORZ1** 

`MaxStepper  MOTORZ1;`

 **//设置电机使用的定时器** 

 **//电机旋转一圈的脉冲数(细分/电子齿轮)** 

 **//脉冲输出引脚 Z1_MOTOR_PLS** 

 **//方向控制引脚 Z1_MOTOR_DIR** 

`MOTORZ1.begin( TIM1,6400,Z1_MOTOR_PLS,Z1_MOTOR_DIR );`

 **//设置电机速度** 

`MOTORZ1.speed(1000);`

 **//使能电机加减速功能** 

`MOTORZ1.EnRamp(true);`

 **//设置加减速长度** 

`MOTORZ1.RampLength(DEFAULT_Z_RAMP_LENGTH);`

 **//设置电机的最小速度** 

`MOTORZ1.MinSpeed(3);`

 **//重要:将scan()函数放入定制器1的中断函数中,才能自动输出脉冲** 


```
void MAX_TIMER1_ISR(void)
{
    MOTORZ1.scan();
}
```


 **//电机移动函数** 

`MOTORZ1.MoveTo(1500); //移动到坐标1500处`

`MOTORZ1.run(DIR_ZHENG,1500); //往正方向移动1500个脉冲`



#### 参与贡献


