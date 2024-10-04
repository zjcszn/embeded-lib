# Description

* SOC: -
* EvalBoard: -
* GCC / Make
* USART0 
* ASCII / MODBUS @ 19200 NONE 

|Slave  #| Registers | Start  |Length  |
|:------:|:---------:|:------:|:------:|
| 1      | Holding   | 0x0000 | 15     |  
| 1      | Input     | 0x0002 | 4      | 
 


# Notes

Write an incrementing counter to register address 10.
Read holding register from address 5 - 10, increment them by one and store them at address 10. 

# History
* 2008-04-06: Created
* 2015-10-07: Updated mbportserial.c


# Status

| Project | Compile | Linking | Tested on Target |
| --------|:-------:|:-------:|:----------------:|
| OK      |     OK  |   OK    |        n\a       |