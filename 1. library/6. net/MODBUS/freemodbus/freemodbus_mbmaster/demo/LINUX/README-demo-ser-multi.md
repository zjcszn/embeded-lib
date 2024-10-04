# Description

* SOC: n/a ( Any system running Linux )
* EvalBoard: n/a
* GCC / Make
* Master 1: /dev/ttyS2, RTU @ MODBUS 19200 NONE
* Master 2: /dev/ttyS5, RTU @ MODBUS 38400 NONE

For both masters:
|Slave  #| Registers | Start  | Length |
|:------:|:---------:|:------:|:------:|
| 1      | Holding   | 0x0000 |   2    | 



# Notes

* Multiple demos available.
* demo-ser-multi.c project demonstrates two master instances of the stack running simultaneously.
* Register 0 is an incrementing counter.


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