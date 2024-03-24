# Description

* SOC: n/a ( WIN32 )
* EvalBoard: n/a
* Visual Studio 2019 / Windows 
* Master 1: RTU / MODBUS @ 19200 NONE, COM2, Slave address : 1
* Master 2: RTU / MODBUS @ 38400 NONE, COM5, Slave address : 1

|Master #| Registers | Start  | Length |
|------- |-----------|--------|--------|
| 1      | Holding   | 0x0000 |   2    |
| 2      | Holding   | 0x0000 |   2    |


# Notes
* Multiple demos available
* demo-multi.c demonstrates two instances of the stack running simultaneously.
* Holding register 0 for both masters is an incrementing counter
* Reads holding register 1 for both masters 



# History
* 2008-07-20: Created
* 2008-09-02: Synced with changes from improved Windows and Windows CE port
* 2009-12-20: - Added new serial mode slow computers.
              - Fixed porting layer so that it works with master/slave at same time.
* 2015-06-14: Added new WIN32 port with improved performance
* 2020-05-06: Updated to Visual Studio 2019

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |   OK    |   OK    |  2008-07-20      |