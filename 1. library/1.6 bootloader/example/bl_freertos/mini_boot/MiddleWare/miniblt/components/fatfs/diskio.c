/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "bsp_sdcard.h"

/* Definitions of physical drive number for each drive */
#define DEV_SD		0	/* Example: Map Ramdisk to physical drive 0 */

#define SDIO_POLLING_MODE			0
#define SDIO_DMA_TIMEOUT			0xFFFF


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	switch (pdrv)
    {
        case DEV_SD :
            stat &= ~STA_NOINIT;
            break;

        default:
            break;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	switch (pdrv)
    {
        case DEV_SD :
            BSP_SD_DeInit();
            if (BSP_SD_Init() == MSD_OK)
            {
                stat &= ~STA_NOINIT;
            }
            break;

        default:
            break;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	uint32_t timeout = SDIO_DMA_TIMEOUT;

	switch (pdrv) {

		case DEV_SD :

#if SDIO_POLLING_MODE == 1

			if (BSP_SD_ReadBlocks((uint32_t *)buff, sector, count, SD_DATATIMEOUT) != MSD_OK)
			{
				return RES_ERROR;
			}
			return RES_OK;

#else
/*
			if (BSP_SD_ReadBlocks_DMA((uint32_t *)buff, sector, count) != MSD_OK)
			{
				return RES_ERROR;
			}

			if (BSP_SD_WaitReadOperation() != MSD_OK)
			{
				return RES_ERROR;
			}

			while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
			{
				if (timeout-- == 0)	return RES_ERROR;
			}
			return RES_OK;
*/
			if (BSP_SD_ReadBlocks_DMA((uint32_t *)buff, sector, count) != MSD_OK)
			{
				return RES_ERROR;
			}

			while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
			{
				if (timeout-- == 0)	return RES_ERROR;
			}
			return RES_OK;

#endif

	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	uint32_t timeout = SDIO_DMA_TIMEOUT;

	switch (pdrv) {

		case DEV_SD :

#if SDIO_POLLING_MODE == 1

			if (BSP_SD_WriteBlocks((uint32_t *)buff, sector, count, SD_DATATIMEOUT) != MSD_OK)
			{
				return RES_ERROR;
			}
			while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
			{
				if (timeout-- == 0)	return RES_ERROR;
			}
			return RES_OK;

#else
/*
			if (BSP_SD_WriteBlocks_DMA((uint32_t *)buff, sector, count) != MSD_OK) 
			{
				return RES_ERROR;
			}
			if (BSP_SD_WaitWriteOperation() != MSD_OK)
			{
				return RES_ERROR;
			}
			while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
			{
				if (timeout-- == 0)	return RES_ERROR;
			}
			return RES_OK;
*/
			if (BSP_SD_WriteBlocks_DMA((uint32_t *)buff, sector, count) != MSD_OK) 
			{
				return RES_ERROR;
			}
			
			while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
			{
				if (timeout-- == 0)	return RES_ERROR;
			}
			return RES_OK;

#endif		

	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (pdrv) {

		case DEV_SD :
			return RES_OK;
	}

	return RES_PARERR;
}

