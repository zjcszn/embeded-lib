//
// Created by zjcszn on 2024/1/29.
//

#ifndef _BSP_SDRAM_H_
#define _BSP_SDRAM_H_

#define SDRAM_TIMEOUT   0 /* UNUSED */

#define SDRAM_ADDRESS   0xC0000000

#define SDRAM_TEST      1

// Mode Register: Burst Length
#define SDRAM_MODEREG_BURST_LENGTH_1                ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2                ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4                ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8                ((uint16_t)0x0003)

// Mode Register: Addressing Mode (SEQ OR INTER)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL         ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED        ((uint16_t)0x0008)

// Mode Register: CAS Latency
#define SDRAM_MODEREG_CAS_LATENCY_2                 ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3                 ((uint16_t)0x0030)

// Mode Register: Operate Mode
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD       ((uint16_t)0x0000)

// Mode Register: Write Mode
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE        ((uint16_t)0x0200)

void bsp_sdram_init(void);
void bsp_sdram_test(void);

#endif //_BSP_SDRAM_H_
