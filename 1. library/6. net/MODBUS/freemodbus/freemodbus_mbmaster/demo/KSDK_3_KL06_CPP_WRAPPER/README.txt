KSDK / PE / FreeRTOS port
=========================

MODBUS slave port for the FRDM-KE06Z platform. The port is based on

* Kinetis Design Studio 3.X
* McuOnEclipse Process Expert addons used for FreeRTOS support [1]

USAGE
=====

On startup a new thread <AppTask> defined in modbus.cpp is started.
It creates a new instance of a subclass of mbrtuslave. The subclass
implements the appropriate register callbacks which are called whenever
a MODBUS master reads or writes a specific set of registers. The 
default implementation return an exception.

PORTING
=======

For adapting to a specific platform the following steps have to be
taken

mbportserial.c: If required add direction pin control for the RS485
 driver enable pin. The demo already contains this and the pin is 
 mapped to PTE5
mbportserial.c: The function <eMBPSerialInit> uses the port argument
 to initialize the appropriate serial device. 


[1] https://sourceforge.net/projects/mcuoneclipse/files/PEx%20Components/





