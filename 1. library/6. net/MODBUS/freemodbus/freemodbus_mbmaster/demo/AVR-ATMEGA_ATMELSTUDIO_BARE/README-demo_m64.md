# Description

* SOC: AVR_ATmega64
* EvalBoard: n/a
* Atmel Studio 7.0 / BARE
* USART on PORTC 
* RTU / RS485 or RS232 @ 19200 NONE 

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
* 2007-06-23: Created
* 2008-09-18: Added configuration file for master/slave demo.
              Increased number of software timers.
* 2008-12-06: Added demo for master/slave stack running at the same time.
* 2010-02-28: Updated AVR port to work with ICCAVR.
* 2010-08-19: Added CodeVision AVR support.
* 2020-05-04: Updated to Atmel Studio 7.0

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |     OK  |   OK    |  2007-06-23      |