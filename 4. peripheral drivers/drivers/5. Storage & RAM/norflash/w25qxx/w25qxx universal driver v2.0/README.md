# W25Qxx Universal Driver v2.0

[![License](./img/license_bage_MIT.svg)](./LICENSE)
<!--[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](./LICENSE)-->

Modern rewrite of popular library.<br/>
Complete [example implementations](./examples) available for STM32F401 (BlackPill) and Orange PI Zero platforms.  
Heavily optimized code size, improved execution speed, minimized chip access instruction sequences and RAM usage.

## Feature summary

- Supported full spectrum of chip functionality (_fast-read_, _SFDP_, _memory block locking_, etc.)
- Production-ready code with advanced error detection and handling 
- RTOS-aware implementation 
    - Functions are reentrant and thread-safe
    - Embedded usage of user-defined optimized delay functions
    - Allowed mutexes to manage concurrent memory access*
- Implemented usage of some architecture-specific instructions for best performance (_like 'rev' aka 'bswap32()' for ARM_)
- Implemented selection of dynamic / static overwrite buffer (selected at compile time)
- Simple interfacing for any platform (see [examples](./examples))
- No external dependencies
- Smart overwriting function: skips erase-write cycle for blocks containing the same data
- Built-in verification of written data for detecting bad blocks

## Usage example

``` C
#include "w25qxx.h"

extern w25qxx_dev_t spi_flash_chip1; // allocated and filled with platform-specific calls and chip-specific bindings  elsewhere

int w25qxx_test(void) {
    int ret;
    char buf[W25QXX_UID_LEN] = {0}; // W25QXX_UID_LEN == 8
    
    ret = w25qxx_init(&spi_flash_chip1);
    if (ret != W25QXX_RET_SUCCESS) { // W25QXX_RET_SUCCESS == 0 
        printf("w25qxx_init(): Failed with code: %02X\r\n", ret); 
        return ret; 
    };

    ret = w25qxx_get_unique_id(&spi_flash_chip1, buf);
    if (ret) { printf("w25qxx_get_unique_id(): Failed with code: %02X (desc: '%s')\r\n", ret, w25qxx_ret2str(ret)); return ret; };

    char uid_str[W25QXX_UID_LEN * 2 + 1] = {0};
    ret = bytes2str(uid_str, buf, W25QXX_UID_LEN);
    if (ret != W25QXX_UID_LEN) { printf("bytes2str(): Failed somehow!\r\n"); return 1; };
    printf("Chip 1 UID: '%s'\r\n", uid_str);

    const int val = 123;
    const unsigned int val_addr = 0x00000000;
    
    ret = w25qxx_overwrite_verify(&spi_flash_chip1, val_addr, &val, sizeof(val));
    if (ret) { printf("w25qxx_overwrite_verify(): Failed with code: %02X (desc: '%s')\r\n", ret, w25qxx_ret2str(ret));  return ret; };

    ret = w25qxx_read_fast(&spi_flash_chip1, val_addr, buf, sizeof(ret));
    if (ret) { printf("w25qxx_read_fast(): Failed with code: %02X (desc: '%s')\r\n", ret, w25qxx_ret2str(ret));  return ret; };

    ret = *(int *)buf;
    printf("Chip 1 read data: %d\r\n", ret);

    return 0;
};

```

## TODO

- [ ] Chip access via SPI-dual / SPI-quad / Quad-SPI interfaces implemented but not tested

