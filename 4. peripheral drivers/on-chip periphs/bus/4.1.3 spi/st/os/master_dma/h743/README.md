1. TXTF（Transmission Transfer Filled）置1时，所有发送操作都会被忽略，需要通过向 SPI2S_IFCR 的 TXTFC 位写入 1 来清零，故SPI发送前需清除 TXTF 标志。
2. H7 的 SPI 传输除了要使能 SPE位，还需将 CSTART 置1，这点与 F4 略有不同。
3. SPE 置0时 RXP RXWNE 会置1，这个是正常现象，SPE 置1后，会清空 FIFO，RXP RXWNE 会自动置0。