# Description

* SOC: n/a ( WIN32 )
* EvalBoard: n/a
* Visual Studio 2019 / Windows
* COM7 
* RTU @ MODBUS 19200 NONE

|Slave  #| Registers | Start  | Length |
|:------:|-----------|--------|--------|
| 1      | Holding   | 0x0000 |   26   |
| 1      | Input     | 0x0002 |   4    |


# Notes

* Multiple demos available
* This README is for demo.c project
* Holding register 0 is an incrementing counter
* Reads the input registers from address 2 - 5 and writes them to the holding registers at address 1 - 4
* Reads holding register from adress 5 - 25, increment them by one and store them at address 10



# History

* 2008-06-08: Created
* 2008-09-02: Synced with changes from improved Windows and Windows CE port
* 2009-12-20: - Added new serial mode slow computers.
              - Fixed porting layer so that it works with master/slave at same time.
* 2015-06-14: Added new WIN32 port with improved performance
* 2020-05-06: Updated to Visual Studio 2019

# Status

| Project | Compile | Linking | Tested on Target |
| --------|:-------:|:-------:|:----------------:|
| OK      |   OK    |   OK    |  2008-10-29      |