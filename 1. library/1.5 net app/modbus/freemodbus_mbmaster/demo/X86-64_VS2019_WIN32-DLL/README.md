# Description

* SOC: n/a ( WIN32 )
* EvalBoard: n/a
* Visual Studio 2019 / Windows 
* TCP master : 127.0.0.1:503


|Slave #| Registers | Start  | length |
|:-----:|:---------:|:------:|:------:|
| 255   | Holding   | 0x0001 |   10   |

# Notes


* This demo shows usage of MODBUS/UDP protocol using Dynamic Link Library ( DLL ).
* This demo connects to the stack, achieves 5 poll cycles and then disconnects automatically. 
* Holding register 1 is an incrementing counter
* Reads holding register from adress 5 - 10, increment them by one and store them at address 10. 



# History

* 2020-06-02: Created

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |   OK    |   OK    |       -          |