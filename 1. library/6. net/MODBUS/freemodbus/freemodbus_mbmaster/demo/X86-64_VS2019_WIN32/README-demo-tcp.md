# Description

* SOC: n/a ( WIN32 )
* EvalBoard: n/a
* Visual Studio 2019 / Windows 
* TCP master : 192.168.3.2:502

|Slave  #| Registers | Start  | Length |
|--------|-----------|--------|--------| 
| 1      | Holding   | 0x0000 |   11   |



# Notes
* Multiple demos available.
* demo-tcp.c shows usage of modbus/TCP.
* This demo connects/disconnects to the stack 5 times and then disconnects automatically. 
* Holding register 0 is an incrementing counter.
* Reads holding register from adress 5 - 10, increment them by one and store them at address 10. 


# History
* 2008-10-29: Created
* 2009-12-20: - Added new serial mode slow computers.
              - Fixed porting layer so that it works with master/slave at same time.
* 2015-06-14: Added new WIN32 port with improved performance
* 2020-05-06: Updated to Visual Studio 2019
# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |   OK    |   OK    |  2008-10-29      |
