# Description

* SOC: n/a ( WIN32 )
* EvalBoard: n/a
* Visual Studio 2019 / Windows 
* master 1: TCP 127.0.0.1:502
* master 2: RTU @ 19200 NONE, COM1

For both masters:
|Slave # | Registers | Start  | Length |
|------- |-----------|--------|--------|
| 1      | Holding   | 0x0000 |   2    |



# Notes

* Multiple demos available
* demo-tcp-multi.c shows running both TCP and RTU in parallel in the same application
* Holding register 0 for both masters is an incrementing counter
* Reads holding register 1 for both masters 



# History

* 2008-10-29: Created
* 2008-09-02: Synced with changes from improved Windows and Windows CE port
* 2009-12-20: - Added new serial mode slow computers.
              - Fixed porting layer so that it works with master/slave at same time.
* 2015-06-14: Added new WIN32 port with improved performance
* 2020-05-06: Updated to Visual Studio 2019

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |   OK    |   OK    |  2008-10-29      |