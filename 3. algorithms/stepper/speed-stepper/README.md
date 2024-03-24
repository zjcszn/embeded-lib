# SpeedStepper


#### 介绍
步进电机(伺服电机)驱动库. 通过脉冲/方向引脚控制电机, 自动实现加减速/限位等功能. 该库是速度型算法, 适合平滑调速的场合, 比如拉线轮或是传送带等等.


#### 安装教程

本程序为一个电机控制类,如果你的编译器不支持类Class,需要自行修改, 把函数从类里面提取出来.

#### 使用说明

 **//定义一个电机实例** 

`SpeedStepper MyMotor;`

`MyMotor.begin( TIM1,TIM2,6400,MOTOR_PLS,MOTOR_DIR,LOW );`

 其中的TIM2是加减速控制定时器,是固定100Hz中断的, 可以多个电机共用一个.例如:

`SpeedStepper MyMotor2;`

`MyMotor2.begin( TIM3,TIM2,6400,MOTOR_PLS,MOTOR_DIR,LOW );`

 **//设置电机速度** 

`MyMotor.speed(1000);`

 **//使能电机加减速功能** 

`MyMotor.ARamp(500,500);`

 **//开始运行** 

`MyMotor.run();`

 **//重要:将scan()函数放入定制器1的中断函数中,才能自动输出脉冲** 

```
void MAX_TIMER1_ISR(void)
{
    MyMotor.scan();
}
```

```
void MAX_TIMER2_ISR(void)
{
    MyMotor.TISR();
    MyMotor2.TISR();
}
```

```
void MAX_TIMER3_ISR(void)
{
    MyMotor2.scan();
}
```