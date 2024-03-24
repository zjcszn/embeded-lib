# Description

* SOC: PIC18F45K22
* EvalBoard: n/a
* MPLAB X IDE v5.35
* EUSART on PORTA
* ASCII / MODBUS @ 19200 NONE 

|Slave  #| Registers | Start  |Length  |
|:------:|:---------:|:------:|:------:|
|1       | Holding   | 0x0000 | 15     |  
|1       | Input     | 0x0002 | 4      | 


# Notes

Register 0 is an incrementing counter.
Reads holding register from address 5 - 10, increment them by one and store them at address 10. 
Reads the input registers from address 2 - 5 and write them to the holding registers at address 1 - 4.
Led is turned on/off depending on the value of the variable bTurnOn.

# History
* 2008-02-26: Created
* 2008-04-03:  - Added support for disabling the additional API checks.
               - Fixed bug with coil/discrete registers.
* 2020-05-05: Updated to MPLAB X IDE v5.35

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |     OK  |   OK    |  2008-02-26      |
