# Description

* SOC: n/a ( WIN32 )
* EvalBoard: n/a
* Visual Studio 2019 / Windows 
* COM1
* RTU / MODBUS @ 19200 NONE
* Implements a custom function code

|Slave  #| Registers | Start  | Length |
|:------:|:---------:|:------:|:------:|
| 1      | Input     | -      | -      |
| 1      | Holding   | -      | -      |


# Notes
* Multiple demos available
* demo-custom.c shows implementation of a custom function code (fc=65)


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
