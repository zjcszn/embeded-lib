# Description

* SOC: n/a ( Any system running Linux )
* EvalBoard: n/a
* GCC / Make
* TCP master 

|Slave         #| Registers | Start  | Length |
|:-------------:|:---------:|:------:|:------:|
|127.0.0.1:502  | Holding   | 0x0000 |   1    | 
|127.0.0.1:502  | Holding   | 0x0005 |   6    | 
|127.0.0.1:502  | Input     | 0x0005 |   6    |


# Notes

* Multiple demos available.
* demo-tcp.c shows usage of modbus/TCP.
* This demo connects/disconnects to the stack 5 times and then disconnects automatically. 
* Register 0 is an incrementing counter.
* Reads holding register from address 5 - 10, increment them by one and store them at address 10. 
* Reads input register from address 5 - 10.

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