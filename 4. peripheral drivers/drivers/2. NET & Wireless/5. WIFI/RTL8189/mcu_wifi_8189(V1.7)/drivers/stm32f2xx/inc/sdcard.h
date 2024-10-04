#ifndef __SDCARD_H
#define __SDCARD_H


#include "stm32f2xx.h"
#include "lwip/sys.h"

#define SDIO_RESET_PORT 			GPIOB
#define SDIO_RESET_PIN				GPIO_Pin_12
#define SDIO_PDN_PORT 				GPIOA
#define SDIO_PDN_PIN				GPIO_Pin_2

#define SDIO_RESET_HIGH			GPIO_SET(SDIO_RESET_PORT, SDIO_RESET_PIN)
#define SDIO_RESET_LOW				GPIO_CLR(SDIO_RESET_PORT, SDIO_RESET_PIN)

#define SDIO_PDN_HIGH				GPIO_SET(SDIO_PDN_PORT, SDIO_PDN_PIN)
#define SDIO_PDN_LOW				GPIO_CLR(SDIO_PDN_PORT, SDIO_PDN_PIN)

#define SD_SEND_IF_COND           8   /* bcr  [11:0] See below   R7  */
#define SD_IO_SEND_OP_COND          5 /* bcr  [23:0] OCR         R4  */
#define SD_IO_RW_DIRECT            52 /* ac   [31:0] See below   R5  */
#define SD_IO_RW_EXTENDED          53 /* adtc [31:0] See below   R5  */

#define MMC_CARD_BUSY	0x80000000	/* Card Power up status bit */

#define MMC_VDD_165_195 0x00000080    /* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21   0x00000100    /* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22   0x00000200    /* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23   0x00000400    /* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24   0x00000800    /* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25   0x00001000    /* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26   0x00002000    /* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27   0x00004000    /* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28   0x00008000    /* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29   0x00010000    /* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30   0x00020000    /* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31   0x00040000    /* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32   0x00080000    /* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33   0x00100000    /* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34   0x00200000    /* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35   0x00400000    /* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36   0x00800000    /* VDD voltage 3.5 ~ 3.6 */

struct sdio_cccr {
	unsigned int		sdio_vsn;
	unsigned int		sd_vsn;
	unsigned int		multi_block:1,
	low_speed:1,
	wide_bus:1,
	high_power:1,
	high_speed:1,
	disable_cd:1;
};

#define R5_COM_CRC_ERROR	(1 << 15)	/* er, b */
#define R5_ILLEGAL_COMMAND	(1 << 14)	/* er, b */
#define R5_ERROR		(1 << 11)	/* erx, c */
#define R5_FUNCTION_NUMBER	(1 << 9)	/* er, c */
#define R5_OUT_OF_RANGE		(1 << 8)	/* er, c */
#define R5_STATUS(x)		(x & 0xCB00)
#define R5_IO_CURRENT_STATE(x)	((x & 0x3000) >> 12) /* s, b */

	
#define SDIO_CCCR_CCCR		0x00
	
#define  SDIO_CCCR_REV_1_00	0	/* CCCR/FBR Version 1.00 */
#define  SDIO_CCCR_REV_1_10	1	/* CCCR/FBR Version 1.10 */
#define  SDIO_CCCR_REV_1_20	2	/* CCCR/FBR Version 1.20 */
	
#define  SDIO_SDIO_REV_1_00	0	/* SDIO Spec Version 1.00 */
#define  SDIO_SDIO_REV_1_10	1	/* SDIO Spec Version 1.10 */
#define  SDIO_SDIO_REV_1_20	2	/* SDIO Spec Version 1.20 */
#define  SDIO_SDIO_REV_2_00	3	/* SDIO Spec Version 2.00 */
	
#define SDIO_CCCR_SD		0x01
	
#define  SDIO_SD_REV_1_01	0	/* SD Physical Spec Version 1.01 */
#define  SDIO_SD_REV_1_10	1	/* SD Physical Spec Version 1.10 */
#define  SDIO_SD_REV_2_00	2	/* SD Physical Spec Version 2.00 */
	
#define SDIO_CCCR_IOEx		0x02
#define SDIO_CCCR_IORx		0x03
	
#define SDIO_CCCR_IENx		0x04	/* Function/Master Interrupt Enable */
#define SDIO_CCCR_INTx		0x05	/* Function Interrupt Pending */
	
#define SDIO_CCCR_ABORT		0x06	/* function abort/card reset */
	
#define SDIO_CCCR_IF		0x07	/* bus interface controls */
	
#define  SDIO_BUS_WIDTH_1BIT	0x00
#define  SDIO_BUS_WIDTH_4BIT	0x02
#define  SDIO_BUS_ECSI		0x20	/* Enable continuous SPI interrupt */
#define  SDIO_BUS_SCSI		0x40	/* Support continuous SPI interrupt */
	
#define  SDIO_BUS_ASYNC_INT	0x20
	
#define  SDIO_BUS_CD_DISABLE     0x80	/* disable pull-up on DAT3 (pin 1) */
	
#define SDIO_CCCR_CAPS		0x08
	
#define  SDIO_CCCR_CAP_SDC	0x01	/* can do CMD52 while data transfer */
#define  SDIO_CCCR_CAP_SMB	0x02	/* can do multi-block xfers (CMD53) */
#define  SDIO_CCCR_CAP_SRW	0x04	/* supports read-wait protocol */
#define  SDIO_CCCR_CAP_SBS	0x08	/* supports suspend/resume */
#define  SDIO_CCCR_CAP_S4MI	0x10	/* interrupt during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_E4MI	0x20	/* enable ints during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_LSC	0x40	/* low speed card */
#define  SDIO_CCCR_CAP_4BLS	0x80	/* 4 bit low speed card */
	
#define SDIO_CCCR_CIS		0x09	/* common CIS pointer (3 bytes) */
	
	/* Following 4 regs are valid only if SBS is set */
#define SDIO_CCCR_SUSPEND	0x0c
#define SDIO_CCCR_SELx		0x0d
#define SDIO_CCCR_EXECx		0x0e
#define SDIO_CCCR_READYx	0x0f
	
#define SDIO_CCCR_BLKSIZE	0x10
	
#define SDIO_CCCR_POWER		0x12
	
#define  SDIO_POWER_SMPC	0x01	/* Supports Master Power Control */
#define  SDIO_POWER_EMPC	0x02	/* Enable Master Power Control */
	
#define  SDIO_CCCR_SPEED		0x13
	
#define  SDIO_SPEED_SHS		0x01	/* Supports High-Speed mode */
#define  SDIO_SPEED_EHS		0x02	/* Enable High-Speed mode */
	
	/*
	 * Function Basic Registers (FBR)
	 */
	
#define SDIO_FBR_BASE(f)	((f) * 0x100) /* base of function f's FBRs */
	
#define SDIO_FBR_STD_IF		0x00
	
#define  SDIO_FBR_SUPPORTS_CSA	0x40	/* supports Code Storage Area */
#define  SDIO_FBR_ENABLE_CSA	0x80	/* enable Code Storage Area */
	
#define SDIO_FBR_STD_IF_EXT	0x01
	
#define SDIO_FBR_POWER		0x02
	
#define  SDIO_FBR_POWER_SPS	0x01	/* Supports Power Selection */
#define  SDIO_FBR_POWER_EPS	0x02	/* Enable (low) Power Selection */
	
#define SDIO_FBR_CIS		0x09	/* CIS pointer (3 bytes) */
	
	
#define SDIO_FBR_CSA		0x0C	/* CSA pointer (3 bytes) */
	
#define SDIO_FBR_CSA_DATA	0x0F
	
#define SDIO_FBR_BLKSIZE	0x10	/* block size (2 bytes) */



#define IF_SDIO_IOPORT		0x00

#define IF_SDIO_H_INT_MASK	0x04
#define   IF_SDIO_H_INT_OFLOW	0x08
#define   IF_SDIO_H_INT_UFLOW	0x04
#define   IF_SDIO_H_INT_DNLD	0x02
#define   IF_SDIO_H_INT_UPLD	0x01

#define IF_SDIO_H_INT_STATUS	0x05
#define IF_SDIO_H_INT_RSR	       0x06
#define IF_SDIO_H_INT_STATUS2	0x07

#define IF_SDIO_RD_BASE		0x10

#define IF_SDIO_STATUS		0x20
#define   IF_SDIO_IO_RDY	0x08
#define   IF_SDIO_CIS_RDY	0x04
#define   IF_SDIO_UL_RDY	0x02
#define   IF_SDIO_DL_RDY	0x01

#define IF_SDIO_C_INT_MASK	0x24
#define IF_SDIO_C_INT_STATUS	0x28
#define IF_SDIO_C_INT_RSR	0x2C

#define IF_SDIO_SCRATCH		0x34
#define IF_SDIO_SCRATCH_OLD	0x80fe
#define IF_SDIO_FW_STATUS	0x40
#define   IF_SDIO_FIRMWARE_OK	0xfedc

#define IF_SDIO_RX_LEN		0x42
#define IF_SDIO_RX_UNIT		0x43

#define IF_SDIO_EVENT           0x80fc

#define IF_SDIO_BLOCK_SIZE	256
#define CONFIGURATION_REG               0x03
#define HOST_POWER_UP                   (0x1U << 1)


/* Exported types ------------------------------------------------------------*/
typedef enum
{
  /* SDIO specific error defines */
  SD_OK                                      = 0,
  SD_CMD_CRC_FAIL                    = (1), /* Command response received (but CRC check failed) */
  SD_DATA_CRC_FAIL                   = (2), /* Data bock sent/received (CRC check Failed) */
  SD_CMD_RSP_TIMEOUT                 = (3), /* Command response timeout */
  SD_DATA_TIMEOUT                    = (4), /* Data time out */
  SD_TX_UNDERRUN                     = (5), /* Transmit FIFO under-run */
  SD_RX_OVERRUN                      = (6), /* Receive FIFO over-run */
  SD_START_BIT_ERR                   = (7), /* Start bit not detected on all data signals in widE bus mode */
  SD_CMD_OUT_OF_RANGE                = (8), /* CMD's argument was out of range.*/
  SD_ADDR_MISALIGNED                 = (9), /* Misaligned address */
  SD_BLOCK_LEN_ERR                   = (10), /* Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
  SD_ERASE_SEQ_ERR                   = (11), /* An error in the sequence of erase command occurs.*/
  SD_BAD_ERASE_PARAM                 = (12), /* An Invalid selection for erase groups */
  SD_WRITE_PROT_VIOLATION            = (13), /* Attempt to program a write protect block */
  SD_LOCK_UNLOCK_FAILED              = (14), /* Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
  SD_COM_CRC_FAILED                  = (15), /* CRC check of the previous command failed */
  SD_ILLEGAL_CMD                     = (16), /* Command is not legal for the card state */
  SD_CARD_ECC_FAILED                 = (17), /* Card internal ECC was applied but failed to correct the data */
  SD_CC_ERROR                        = (18), /* Internal card controller error */
  SD_GENERAL_UNKNOWN_ERROR           = (19), /* General or Unknown error */
  SD_STREAM_READ_UNDERRUN            = (20), /* The card could not sustain data transfer in stream read operation. */
  SD_STREAM_WRITE_OVERRUN            = (21), /* The card could not sustain data programming in stream mode */
  SD_CID_CSD_OVERWRITE               = (22), /* CID/CSD overwrite error */
  SD_WP_ERASE_SKIP                   = (23), /* only partial address space was erased */
  SD_CARD_ECC_DISABLED               = (24), /* Command has been executed without using internal ECC */
  SD_ERASE_RESET                     = (25), /* Erase sequence was cleared before executing because an out of erase sequence command was received */
  SD_AKE_SEQ_ERROR                   = (26), /* Error in sequence of authentication. */
  SD_INVALID_VOLTRANGE               = (27),
  SD_ADDR_OUT_OF_RANGE               = (28),
  SD_SWITCH_ERROR                    = (29),
  SD_SDIO_DISABLED                   = (30),
  SD_SDIO_FUNCTION_BUSY              = (31),
  SD_SDIO_FUNCTION_FAILED            = (32),
  SD_SDIO_UNKNOWN_FUNCTION           = (33),

  /* Standard error defines */
  SD_INTERNAL_ERROR, 
  SD_NOT_CONFIGURED,
  SD_REQUEST_PENDING, 
  SD_REQUEST_NOT_APPLICABLE, 
  SD_INVALID_PARAMETER,  
  SD_UNSUPPORTED_FEATURE,  
  SD_UNSUPPORTED_HW,  
  SD_ERROR,  
} SD_Error;

/* SDIO Commands  Index */
#define SDIO_GO_IDLE_STATE                       ((uint8_t)0)
#define SDIO_SEND_OP_COND                        ((uint8_t)1)
#define SDIO_ALL_SEND_CID                        ((uint8_t)2)
#define SDIO_SET_REL_ADDR                        ((uint8_t)3) /* SDIO_SEND_REL_ADDR for SD Card */
#define SDIO_SET_DSR                             ((uint8_t)4)
#define SDIO_SDIO_SEN_OP_COND                    ((uint8_t)5)
#define SDIO_HS_SWITCH                           ((uint8_t)6)
#define SDIO_SEL_DESEL_CARD                      ((uint8_t)7)
#define SDIO_HS_SEND_EXT_CSD                     ((uint8_t)8)
#define SDIO_SEND_CSD                            ((uint8_t)9)
#define SDIO_SEND_CID                            ((uint8_t)10)
#define SDIO_READ_DAT_UNTIL_STOP                 ((uint8_t)11) /* SD Card doesn't support it */
#define SDIO_STOP_TRANSMISSION                   ((uint8_t)12)
#define SDIO_SEND_STATUS                         ((uint8_t)13)
#define SDIO_HS_BUSTEST_READ                     ((uint8_t)14)
#define SDIO_GO_INACTIVE_STATE                   ((uint8_t)15)
#define SDIO_SET_BLOCKLEN                        ((uint8_t)16)
#define SDIO_READ_SINGLE_BLOCK                   ((uint8_t)17)
#define SDIO_READ_MULT_BLOCK                     ((uint8_t)18)
#define SDIO_HS_BUSTEST_WRITE                    ((uint8_t)19)
#define SDIO_WRITE_DAT_UNTIL_STOP                ((uint8_t)20) /* SD Card doesn't support it */
#define SDIO_SET_BLOCK_COUNT                     ((uint8_t)23) /* SD Card doesn't support it */
#define SDIO_WRITE_SINGLE_BLOCK                  ((uint8_t)24)
#define SDIO_WRITE_MULT_BLOCK                    ((uint8_t)25)
#define SDIO_PROG_CID                            ((uint8_t)26) /* reserved for manufacturers */
#define SDIO_PROG_CSD                            ((uint8_t)27)
#define SDIO_SET_WRITE_PROT                      ((uint8_t)28)
#define SDIO_CLR_WRITE_PROT                      ((uint8_t)29)
#define SDIO_SEND_WRITE_PROT                     ((uint8_t)30)
#define SDIO_SD_ERASE_GRP_START                  ((uint8_t)32) /* To set the address of the first write
                                                                  block to be erased. (For SD card only) */
#define SDIO_SD_ERASE_GRP_END                    ((uint8_t)33) /* To set the address of the last write block of the
                                                                  continuous range to be erased. (For SD card only) */
#define SDIO_ERASE_GRP_START                     ((uint8_t)35) /* To set the address of the first write block to be erased.
                                                                  (For MMC card only spec 3.31) */

#define SDIO_ERASE_GRP_END                       ((uint8_t)36) /* To set the address of the last write block of the
                                                                  continuous range to be erased. (For MMC card only spec 3.31) */

#define SDIO_ERASE                               ((uint8_t)38)
#define SDIO_FAST_IO                             ((uint8_t)39) /* SD Card doesn't support it */
#define SDIO_GO_IRQ_STATE                        ((uint8_t)40) /* SD Card doesn't support it */
#define SDIO_LOCK_UNLOCK                         ((uint8_t)42)
#define SDIO_APP_CMD                             ((uint8_t)55)
#define SDIO_GEN_CMD                             ((uint8_t)56)
#define SDIO_NO_CMD                              ((uint8_t)64)

/* Following commands are SD Card Specific commands.
   SDIO_APP_CMD should be sent before sending these
   commands. */
#define SDIO_APP_SD_SET_BUSWIDTH                 ((uint8_t)6)  /* For SD Card only */
#define SDIO_SD_APP_STAUS                        ((uint8_t)13) /* For SD Card only */
#define SDIO_SD_APP_SEND_NUM_WRITE_BLOCKS        ((uint8_t)22) /* For SD Card only */
#define SDIO_SD_APP_OP_COND                      ((uint8_t)41) /* For SD Card only */
#define SDIO_SD_APP_SET_CLR_CARD_DETECT          ((uint8_t)42) /* For SD Card only */
#define SDIO_SD_APP_SEND_SCR                     ((uint8_t)51) /* For SD Card only */
#define SDIO_SDIO_RW_DIRECT                      ((uint8_t)52) /* For SD I/O Card only */
#define SDIO_SDIO_RW_EXTENDED                    ((uint8_t)53) /* For SD I/O Card only */

/* Following commands are SD Card Specific security commands.
   SDIO_APP_CMD should be sent before sending these commands. */
#define SDIO_SD_APP_GET_MKB                      ((uint8_t)43) /* For SD Card only */
#define SDIO_SD_APP_GET_MID                      ((uint8_t)44) /* For SD Card only */
#define SDIO_SD_APP_SET_CER_RN1                  ((uint8_t)45) /* For SD Card only */
#define SDIO_SD_APP_GET_CER_RN2                  ((uint8_t)46) /* For SD Card only */
#define SDIO_SD_APP_SET_CER_RES2                 ((uint8_t)47) /* For SD Card only */
#define SDIO_SD_APP_GET_CER_RES1                 ((uint8_t)48) /* For SD Card only */
#define SDIO_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((uint8_t)18) /* For SD Card only */
#define SDIO_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((uint8_t)25) /* For SD Card only */
#define SDIO_SD_APP_SECURE_ERASE                 ((uint8_t)38) /* For SD Card only */
#define SDIO_SD_APP_CHANGE_SECURE_AREA           ((uint8_t)49) /* For SD Card only */
#define SDIO_SD_APP_SECURE_WRITE_MKB             ((uint8_t)48) /* For SD Card only */

typedef enum
{
  SD_NO_TRANSFER  = 0,
  SD_TRANSFER_IN_PROGRESS
} SDTransferState;

typedef struct
{
  uint16_t TransferredBytes;
  SD_Error TransferError;
  uint8_t  padding;
} SDLastTransferInfo;

typedef struct       /* Card Specific Data */
{
  __IO uint8_t  CSDStruct;            /* CSD structure */
  __IO uint8_t  SysSpecVersion;       /* System specification version */
  __IO uint8_t  Reserved1;            /* Reserved */
  __IO uint8_t  TAAC;                 /* Data read access-time 1 */
  __IO uint8_t  NSAC;                 /* Data read access-time 2 in CLK cycles */
  __IO uint8_t  MaxBusClkFrec;        /* Max. bus clock frequency */
  __IO uint16_t CardComdClasses;      /* Card command classes */
  __IO uint8_t  RdBlockLen;           /* Max. read data block length */
  __IO uint8_t  PartBlockRead;        /* Partial blocks for read allowed */
  __IO uint8_t  WrBlockMisalign;      /* Write block misalignment */
  __IO uint8_t  RdBlockMisalign;      /* Read block misalignment */
  __IO uint8_t  DSRImpl;              /* DSR implemented */
  __IO uint8_t  Reserved2;            /* Reserved */
  __IO uint32_t DeviceSize;           /* Device Size */
  __IO uint8_t  MaxRdCurrentVDDMin;   /* Max. read current @ VDD min */
  __IO uint8_t  MaxRdCurrentVDDMax;   /* Max. read current @ VDD max */
  __IO uint8_t  MaxWrCurrentVDDMin;   /* Max. write current @ VDD min */
  __IO uint8_t  MaxWrCurrentVDDMax;   /* Max. write current @ VDD max */
  __IO uint8_t  DeviceSizeMul;        /* Device size multiplier */
  __IO uint8_t  EraseGrSize;          /* Erase group size */
  __IO uint8_t  EraseGrMul;           /* Erase group size multiplier */
  __IO uint8_t  WrProtectGrSize;      /* Write protect group size */
  __IO uint8_t  WrProtectGrEnable;    /* Write protect group enable */
  __IO uint8_t  ManDeflECC;           /* Manufacturer default ECC */
  __IO uint8_t  WrSpeedFact;          /* Write speed factor */
  __IO uint8_t  MaxWrBlockLen;        /* Max. write data block length */
  __IO uint8_t  WriteBlockPaPartial;  /* Partial blocks for write allowed */
  __IO uint8_t  Reserved3;            /* Reserded */
  __IO uint8_t  ContentProtectAppli;  /* Content protection application */
  __IO uint8_t  FileFormatGrouop;     /* File format group */
  __IO uint8_t  CopyFlag;             /* Copy flag (OTP) */
  __IO uint8_t  PermWrProtect;        /* Permanent write protection */
  __IO uint8_t  TempWrProtect;        /* Temporary write protection */
  __IO uint8_t  FileFormat;           /* File Format */
  __IO uint8_t  ECC;                  /* ECC code */
  __IO uint8_t  CSD_CRC;              /* CSD CRC */
  __IO uint8_t  Reserved4;            /* always 1*/
} SD_CSD;

typedef struct      /*Card Identification Data*/
{
  __IO uint8_t  ManufacturerID;       /* ManufacturerID */
  __IO uint16_t OEM_AppliID;          /* OEM/Application ID */
  __IO uint32_t ProdName1;            /* Product Name part1 */
  __IO uint8_t  ProdName2;            /* Product Name part2*/
  __IO uint8_t  ProdRev;              /* Product Revision */
  __IO uint32_t ProdSN;               /* Product Serial Number */
  __IO uint8_t  Reserved1;            /* Reserved1 */
  __IO uint16_t ManufactDate;         /* Manufacturing Date */
  __IO uint8_t  CID_CRC;              /* CID CRC */
  __IO uint8_t  Reserved2;            /* always 1 */
} SD_CID;

typedef struct      /*Card Identification Data*/
{
  	uint32_t sizeofword;
	uint32_t word_pos;
	uint32_t err;
	uint32_t *data;
} SD_DATA;


#define DISABLE_SDIO_IRQ()  SDIO_ITConfig(SDIO_IT_SDIOIT , DISABLE)
#define ENABLE_SDIO_IRQ()  SDIO_ITConfig(SDIO_IT_SDIOIT , ENABLE)

typedef struct
{
  SD_CSD SD_csd;
  SD_CID SD_cid;
  uint32_t CardCapacity; /* Card Capacity */
  uint32_t CardBlockSize; /* Card Block Size */
  uint16_t RCA;					 /* 神马来的？ */
  uint8_t CardType;
} SD_CardInfo;

/* SDIO CIS Tuple code */
#define CISTPL_NULL      0x00
#define CISTPL_CHECKSUM  0x10
#define CISTPL_VERS_1    0x15
#define CISTPL_ALTSTR    0x16
#define CISTPL_MANFID    0x20
#define CISTPL_FUNCID    0x21
#define CISTPL_FUNCE     0x22
#define CISTPL_SDIO_STD  0x91
#define CISTPL_SDIO_EXT  0x92
#define CISTPL_END       0xff


/* Exported constants --------------------------------------------------------*/
#define SD_DMA_MODE                     ((uint32_t)0x00000000)
#define SD_INTERRUPT_MODE               ((uint32_t)0x00000001)
#define SD_POLLING_MODE                 ((uint32_t)0x00000002)

/* Supported Memory Cards */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1     ((uint32_t)0x0)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0     ((uint32_t)0x1)
#define SDIO_HIGH_CAPACITY_SD_CARD         ((uint32_t)0x2)
#define SDIO_MULTIMEDIA_CARD               ((uint32_t)0x3)
#define SDIO_SECURE_DIGITAL_IO_CARD        ((uint32_t)0x4)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD    ((uint32_t)0x5)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD  ((uint32_t)0x6)
#define SDIO_HIGH_CAPACITY_MMC_CARD        ((uint32_t)0x7)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int SD_Init(void);
SD_Error SD_PowerON(void);
SD_Error SD_PowerOFF(void);
SD_Error SD_InitializeCards(void);
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);
SD_Error SD_EnableWideBusOperation(uint32_t WideMode);
SD_Error SD_SetDeviceMode(uint32_t Mode);
SD_Error SD_SelectDeselect(uint32_t addr);
SD_Error SD_ReadBlock(uint32_t addr, uint32_t *readbuff, uint16_t BlockSize);
SD_Error SD_ReadMultiBlocks(uint32_t addr, uint32_t *readbuff, uint16_t BlockSize, uint32_t NumberOfBlocks);
SD_Error SD_WriteBlock(uint32_t addr, uint32_t *writebuff, uint16_t BlockSize);
SD_Error SD_WriteMultiBlocks(uint32_t addr, uint32_t *writebuff, uint16_t BlockSize, uint32_t NumberOfBlocks);
SDTransferState SD_GetTransferState(void);
SD_Error SD_StopTransfer(void);
SD_Error SD_Erase(uint32_t startaddr, uint32_t endaddr);
SD_Error SD_SendStatus(uint32_t *pcardstatus);
SD_Error SD_SendSDStatus(uint32_t *psdstatus);
SD_Error SD_ProcessIRQSrc(void);
int mmc_send_io_op_cond(u32 ocr, u32 *rocr);

int mmc_io_rw_direct_host(int write, unsigned fn,
	unsigned addr, u8 in, u8 *out);

int mmc_io_rw_direct(int write, unsigned fn,unsigned addr, u8 in, u8 *out);

int mmc_io_rw_extended(int write, unsigned fn,
	unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz);
int mmc_go_idle(void);
int sdio_read_common_cis(void);
void enable_sdio_int(void);

extern uint32_t CardType;
extern uint32_t CSD_Tab[4], CID_Tab[4], RCA;
extern uint32_t DeviceMode;
extern uint32_t TotalNumberOfBytes, StopCondition;
extern uint32_t *SrcBuffer, *DestBuffer;
extern volatile SD_Error TransferError;
extern __IO uint32_t TransferEnd;
extern __IO uint32_t NumberOfBytes;
extern SDIO_InitTypeDef SDIO_InitStructure;
extern SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
extern SDIO_DataInitTypeDef SDIO_DataInitStructure;
extern struct sdio_cccr cccr;
extern struct sdio_func func;

#endif /* __SDCARD_H */	  

