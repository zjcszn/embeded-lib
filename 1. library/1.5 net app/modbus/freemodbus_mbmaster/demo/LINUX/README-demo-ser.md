# Description

* SOC: n/a ( Any system running Linux )
* EvalBoard: n/a
* GCC / Make
* /dev/ttyS1
* ASCII / MODBUS @ 19200 NONE 

|Slave  #| Registers | Start  | Length |
|:------:|:---------:|:------:|:------:|
|1       | Input     | 0x0002 |   4    |
|1       | Holding   | 0x0000 |   26   |


# Notes

* Multiple demos available.
* This README describes demo-ser.c project.
* Register 0 is an incrementing counter.
* Reads holding register from address 5 - 25, increment them by one and store them at address 10. 
* Reads the input registers from address 2 - 5 and write them to the holding registers at address 1 - 4.


# History
* 2008-02-07: Created
* 2009-10-18: Added additional debug output in case an operation fails.
* 2015-11-03: BUGFIX (ASCII) Fixed race condition by stopping the intercharactertimeout in the receive function. Otherwise in case of a slavetimeout it could result in an incorrect callback to the stack.from the porting layer.
       - BUGFIX (EXAMPLE) Example for non blocking mode did not work correctly as polling would never start.
       - BUGFIX (LINUX) Improved implementation for the event queue for the classic LINUX porting layer.
* 2019-03-19: Reverted to previous serial timeout implementation        




| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |     OK  |   OK    |  2008-02-07      |