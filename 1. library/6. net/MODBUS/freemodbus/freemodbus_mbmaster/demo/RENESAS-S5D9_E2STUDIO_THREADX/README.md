# Description

* SOC: S5D9
* EvalBoard: n/a
* e² studio/ ThreadX
* UART0
* RTU / RS232 or RS485 @ 38400 ODD 

|Slave  #| Registers | Start  |Length  |
|:------:|:---------:|:------:|:------:|
|1       | Holding   | 0x0000 | 10     |  



# Notes
Using SSP 1.7.5.
Register 0 is an incrementing counter.
Reads holding register from address 0 - 9, increment them by one and store them at their same address. 



# History

* 2019-07-11: Created
* 2020-06-04: Updated to latest verison of e² studio

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |     OK  |   OK    |  2019-07-11      |
