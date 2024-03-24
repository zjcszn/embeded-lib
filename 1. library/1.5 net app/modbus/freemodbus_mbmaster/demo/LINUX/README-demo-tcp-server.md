# Description

* SOC: n/a ( Any system running Linux )
* EvalBoard: n/a
* GCC / Make
* RTU master 1: /dev/ttyS1, MODBUS @ 19200 NONE
* TCP server: 127.0.0.1:502

|Slave  #| Registers | Start  | Length |
|:------:|:---------:|:------:|:------:|
| -      |    -      |    -   |  -     | 


# Notes

* Multiple demos available.
* demo-tcp-server.c project is a test for the TCP porting layer.


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