# Description

* SOC: n/a ( WIN32 )
* EvalBoard: n/a
* Visual Studio 2019 / Windows 
* TCP master : 127.0.0.1:503
* UDP bind address : 0.0.0.0
* UDP listen port : -1

|Slave #| Registers | Start  | Length |
|-------|-----------|--------|--------|
| 255   | Holding   | 0x0001 |   10   |

# Notes

* Multiple demos available
* demo-udp.c shows usage of MODBUS/UDP protocol
* This demo connects to the stack, achieves 5 poll cycles and then disconnects automatically. 
* Holding register 1 is an incrementing counter
* Reads holding register from adress 5 - 10, increment them by one and store them at address 10. 



# History

* 2011-11-17: Created
* 2015-06-14: Added new WIN32 port with improved performance
* 2020-05-06: Updated to Visual Studio 2019

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |   OK    |   OK    |  2011-11-17      |