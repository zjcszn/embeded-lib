# Description

* SOC: LPC1758
* EvalBoard: n/a
* LPCXpresso v8.2.2_650 / FreeRTOS 
* master 1: TCP
* master 2: RTU @ 38400 NONE, UART0

|Master #|Slave             #| Registers | Start  | Length |
|:------:|:-----------------:|:---------:|:------:|:------:| 
| 1      | 192.168.0.200:502 |Holding    | 0x0000 |   10   |
| 2      | 1                 |Holding    | 0x0000 |   10   |



# Notes
* Reads holding register from address 0 - 10 for both masters.
* LED0 is a blinking LED.
* Make sure to use the Release configuration for this demo.


# History
* 2011-01-02: Created
* 2014-08-23: Added new lwIP port
* 2020-06-02: Updated to LPCXpresso v8.2.2_650

# Status

| Project | Compile | Linking | Tested on Target |
|:-------:|:-------:|:-------:|:----------------:|
| OK      |   OK    |   OK    |  2011-01-02      |
