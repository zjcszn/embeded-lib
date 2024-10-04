@ECHO OFF

SET TOOLCHAIN_PATH=D:/Programs/arm-gcc/10.3/bin
SET OPENOCD_PATH=D:/Programs/OpenOCD/bin

SET CC=%TOOLCHAIN_PATH%/arm-none-eabi-gcc.exe
SET OBJCOPY=%TOOLCHAIN_PATH%/arm-none-eabi-objcopy.exe
SET OO=%OPENOCD_PATH%/openocd.exe

:: SET DEFINES=-DSTM32F401xx -DW25QXX_DYNAMIC_OVERWRITE_BUFFER -DW25QXX_DEBUG
SET DEFINES=-DSTM32F401xx
SET INCLUDES=-ICMSIS -I../../src
SET SOURCES=CMSIS/startup_stm32f401xc.s CMSIS/system_stm32f4xx.c ../../src/w25qxx.c bsd.c main.c
SET TARG_FLAGS=-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -fno-strict-aliasing -fshort-enums -fno-builtin -Wdouble-promotion -TCMSIS/STM32F401CC_FLASH.ld -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc

SET CFLAGS=-Os %TARG_FLAGS% %DEFINES% %INCLUDES% %SOURCES%

CD /d "%~dp0"
ECHO Compiling...
%CC% %CFLAGS% -o out.elf || exit 1

ECHO Converting...
ECHO %OBJCOPY% -O ihex out.elf out.hex
%OBJCOPY% -O ihex out.elf out.hex
ECHO %OBJCOPY% -O binary out.elf out.bin
%OBJCOPY% -O binary out.elf out.bin

IF "%1" == "-f" (
	ECHO Flashing...
	%OO% -f interface/stlink.cfg -c "transport select hla_swd;" -c "source [find target/stm32f4x.cfg]; adapter speed 4000; init; reset halt; program out.hex verify; reset run; exit;"
) ELSE (
	ECHO.
	ECHO Run '%~nx0 -f' to load firmware into a chip using OpenOCD via ST-LinkV2 debugger after compilation.
)
