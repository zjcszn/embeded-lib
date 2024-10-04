# Description

* SOC: STM32-L072
* EvalBoard: STM32-L072
* Keil MDK-ARM 5.29 / FreeRTOS
* USART4 using DMA
* RTU / MODBUS @ 38400 ODD 

|Slave  #| Registers | Start  | Length | 
|:------:|:---------:|:------:|:------:|
| 1      | Input     | 0x0002 | 4      |



# Notes

Reads the input registers from address 2 - 5

# History
* 2016-08-27: Created
* 2017-03-07 : Added STM32/CubeMx bugfixes
* 2020-03-24: Updated to Keil ARM 5.29

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |1 Warning|   OK    |  2016-08-27      |


