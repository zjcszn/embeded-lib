1. 下载 W5500 驱动 `https://github.com/Wiznet/ioLibrary_Driver`

2. 将 Ethernet 文件夹内的文件添加到工程中

3. 在 Ethernet/wizchip_conf.h 中完善芯片信息和IO模式 ，如下：

   ```c
   #define _WIZCHIP_      		W5500 
   /**
    * VDM: 可变数据长度模式，数据长度由主机 SCSn控制
    * FDM: 固定数据长度模式，W5500 SCSn直接接地，只能使用 1/2/4 字节数据帧
    */
   #define _WIZCHIP_IO_MODE_   _WIZCHIP_IO_MODE_SPI_VDM_   /* 按 W5500 手册建议使用 VDM 可变数据长度模式 */
   ```

   

4. 在 Ethernet/wizchip_conf.c 中完善接口函数

   ```c
   void 	  wizchip_cris_enter(void)           				{}
   void 	  wizchip_cris_exit(void)          					{}
   void 	wizchip_cs_select(void)            					{}
   void 	wizchip_cs_deselect(void)          					{}
   uint8_t wizchip_spi_readbyte(void)        					{return 0;}
   void 	wizchip_spi_writebyte(uint8_t wb) 					{}
   void 	wizchip_spi_readburst(uint8_t* pBuf, uint16_t len) 	{}
   void 	wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len) {}
   ```
   


5. 初始化 SPI / RST，执行 HardReset 复位 W5500，配置 PHY，网络信息（MAC、IP等），检查 PHY Link 状态。
6. 完成驱动移植，Internet 文件夹内是 WIZ 封装好的应用层接口，可根据需要进行移植。