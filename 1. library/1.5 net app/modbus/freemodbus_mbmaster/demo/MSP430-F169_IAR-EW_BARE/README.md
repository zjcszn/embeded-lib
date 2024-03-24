# Description

* SOC: MSP430F169
* EvalBoard: n/a
* IAR Embedded Workbench 8.4
* USART0
* RTU / MODBUS @ 19200 NONE 

|Slave  #| Registers | Start  |Length  |
|:------:|:---------:|:------:|:------:|
| 1      | Holding   | 0x0000 | 15     |  
| 1      | Input     | 0x0002 | 4      | 


# Notes

Register 0 is an incrementing counter.
Reads holding register from address 5 - 10, increment them by one and store them at address 10. 
Reads the input registers from address 2 - 5 and write them to the holding registers at address 1 - 4.
Led is turned on/off depending on the value of the variable bTurnOn.
 

# History

* 2010-11-28: Created
* 2020-05-05: Updated to IAR Embedded Workbench 8.4

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |     OK  |   OK    |  2010-11-28      |