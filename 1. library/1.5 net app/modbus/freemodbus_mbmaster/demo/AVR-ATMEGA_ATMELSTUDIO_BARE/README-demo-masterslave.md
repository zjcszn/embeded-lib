# Description

* SOC: AVR_ATmega328P
* EvalBoard: n/a
* Atmel Studio 7.0 / BARE
* USART0 and USART1 on PORTC
* RTU / RS485 @ 19200 NONE 

|Slave  #| Registers | Start  |Length  |
|:------:|:---------:|:------:|:------:|
| 1      | Holding   | 0x0000 | 10     |  



# Notes

This demo demonstrates one slave instance and one master instance of the stack running simultaneously.


# History

* 2008-12-06: Created
* 2010-02-28: Updated AVR port to work with ICCAVR.
* 2010-08-19: Added CodeVision AVR support.
* 2020-05-04: Updated to Atmel Studio 7.0

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |     OK  |   OK    |  2008-12-06      |