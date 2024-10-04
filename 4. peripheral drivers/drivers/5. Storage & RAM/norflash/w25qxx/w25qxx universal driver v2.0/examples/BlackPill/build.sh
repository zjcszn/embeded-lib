#!/bin/bash

TOOLCHAIN_PATH=
OPENOCD_PATH=

CC="${TOOLCHAIN_PATH}arm-none-eabi-gcc"
OBJCOPY="${TOOLCHAIN_PATH}arm-none-eabi-objcopy"
OO="${OPENOCD_PATH}openocd"

#DEFINES="-DSTM32F401xx -DW25QXX_DYNAMIC_OVERWRITE_BUFFER -DW25QXX_DEBUG"
DEFINES="-DSTM32F401xx"
INCLUDES="-ICMSIS -I../../src"
SOURCES="CMSIS/startup_stm32f401xc.s CMSIS/system_stm32f4xx.c ../../src/w25qxx.c bsd.c main.c"
TARG_FLAGS="-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -fno-strict-aliasing -fshort-enums -fno-builtin -Wdouble-promotion -TCMSIS/STM32F401CC_FLASH.ld -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc"

CFLAGS="-Os $TARG_FLAGS $DEFINES $INCLUDES $SOURCES"

echo "Compiling..."
#echo $CC $CFLAGS -o out.elf
$CC $CFLAGS -o out.elf || exit 1

echo "Converting..."
#echo $OBJCOPY -O ihex out.elf out.hex
$OBJCOPY -O ihex out.elf out.hex
#echo $OBJCOPY -O binary out.elf out.bin
$OBJCOPY -O binary out.elf out.bin

if [ "$1" == "-f" ] ; then
	echo "Flashing..."
#	echo $OO -f interface/stlink.cfg -c "transport select hla_swd;" -c "source [find target/stm32f4x.cfg]; adapter speed 4000; init; reset halt; program out.hex verify; reset run; exit;"
	$OO -f interface/stlink.cfg -c "transport select hla_swd;" -c "source [find target/stm32f4x.cfg]; adapter speed 4000; init; reset halt; program out.hex verify; reset run; exit;"
else
	echo
	echo "Run './${0##*/} -f' to load firmware into a chip using OpenOCD via ST-LinkV2 debugger after compilation."
fi
