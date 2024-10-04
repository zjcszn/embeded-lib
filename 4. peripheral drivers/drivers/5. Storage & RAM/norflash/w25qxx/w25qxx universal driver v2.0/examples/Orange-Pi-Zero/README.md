# W25Qxx Universal Driver v2.0

![Orangepi-zero-with-spi-flash-chip](http://www.orangepi.org/img/computersAndMmicrocontrollers/Zero/Rectangle%20641.png) 

## Usage example for Debian Linux on [Orange Pi Zero board](http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/details/Orange-Pi-Zero.html)

Starting in 2016, all **Orange Pi Zero** boards have spi flash chip installed by default.
It is available over `/dev/spidev0.0` interface, which is not enabled by default in [Armbian](https://www.armbian.com/orange-pi-zero/).

In my case, I have **Macronix** `MX25L1606E` which is cheaper alternative to **Winbond**. <br/>

_**NOTE:** Direct call to `w25qxx_overwrite_verify()` function will not work due to hard-coded limitation of the [spidev driver](https://github.com/orangepi-xunlong/orangepi_h3_linux/blob/master/OrangePi-Kernel/linux-4.9/drivers/spi/spidev.c) inside kernel (as of Oct 2022),
which doesn't allow transferring more than 4095 bytes in one transaction AND current architecture of the library, which requires performing `(5+4096) = 4101` bytes long transactions for overwriting sectors.<br/>
Nevertheless, initializing and reading works just fine, which is enough for this little demo._

#### Following steps are required to successfully run this demo,

0. Add lines below to `/boot/armbianEnv.txt` to enable `/dev/spidev0.0` 
    ```
    overlays=spi-spidev
    param_spidev_spi_bus=0
    ```
0. Reboot the device
0. Compile and run the example, using command:
    ``` bash
    ./build.sh && ./opi_spi_flash_test
    ```
##### Expected output:

    ```
    Initializing SPI interface...OK
    Testing spi flash chip...
    
    w25qxx_chip_detect(): dev->dev_id: 4 (chip val: 14)
    w25qxx_init(): manufacturer_id=C2; device_id=14;
    Chip 1 UID: '00000000000000'
    Manufacturer:   MACRONIX
    Dev_type:       W25Q16
    Dev_cap:        2048 Kbytes
    Interface:      SPI
    Intf_mode:      NORMAL
    Addr_mode:      3B
    Supply volt:    2.7..3.6
    Supply curr:    25 ma
    Temp_range:     85°C..-40°C
    Driver vers:    2.0
    
    Chip 1 read data_pre: [4 bytes @ 0x001FFF00]: 'FFFFFFFF'
    DONE.
    ```
