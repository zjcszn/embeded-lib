# Description

* SOC: n/a ( WIN32 )
* EvalBoard: n/a
* Visual Studio 2019 / Windows 
* TCP master :  213.129.231.174:502

|Slave #| Registers | Start  | Length |
|-------|-----------|--------|--------|
| 1     | Holding   | 0x0000 |   1    |



# Notes

* Multiple demos available
* demo-tcp-single.c shows usage of modbus/TCP.
* This demo connects/disconnects to the stack 5 times and then disconnects automatically. 
* Holding register 0 is an incrementing counter



# History

* 2016-06-09: Created
* 2020-05-06: Updated to Visual Studio 2019

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |   OK    |   OK    |  2016-06-09      |