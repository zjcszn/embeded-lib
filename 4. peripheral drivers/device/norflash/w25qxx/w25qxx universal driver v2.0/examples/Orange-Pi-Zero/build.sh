#!/bin/bash

CFLAGS= #-DW25QXX_DEBUG #-DW25QXX_DYNAMIC_OVERWRITE_BUFFER

gcc -std=gnu17 -O2 "-I../../src/" "../../src/w25qxx.c" $CFLAGS intf_orange_pi_zero.c main.c -o opi_spi_flash_test
