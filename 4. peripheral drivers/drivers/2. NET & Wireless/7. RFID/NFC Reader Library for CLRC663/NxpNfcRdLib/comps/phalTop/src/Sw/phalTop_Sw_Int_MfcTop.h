/*----------------------------------------------------------------------------*/
/* Copyright 2016,2020,2022 NXP                                               */
/*                                                                            */
/* NXP Confidential. This software is owned or controlled by NXP and may only */
/* be used strictly in accordance with the applicable license terms.          */
/* By expressly accepting such terms or by downloading, installing,           */
/* activating and/or otherwise using the software, you are agreeing that you  */
/* have read, and that you agree to comply with and are bound by, such        */
/* license terms. If you do not agree to be bound by the applicable license   */
/* terms, then you may not retain, install, activate or otherwise use the     */
/* software.                                                                  */
/*----------------------------------------------------------------------------*/

/** \file
* Software Tag Operation Application Layer Component of
* Reader Library Framework.
* $Author: $
* $Revision: $ (v07.09.00)
* $Date: $
*/

#ifndef PHALMFCTOP_SW_H
#define PHALMFCTOP_SW_H

#ifdef NXPBUILD__PHAL_TOP_SW
#ifdef NXPBUILD__PHAL_TOP_MFC_SW

#define PH_ALMFCTOP_MFC_READ        0x30U    /**< MIFARE Classic contactless IC Read command byte */
#define PH_ALMFCTOP_MFC_WRITE       0xA0U   /**< MIFARE Classic Write command byte */

/*TODO: Needs to see whether the CRC Preset needs to be mirror inverted to 0xC7 */
#define PH_ALMFCTOP_MFC_CRC_PRESET  0xE3U  /**< CRC Preset as given in  the MAD spec */

/**
* \name MFC MEMORY LAYOUT
*/
/*@{*/
#define PHAL_MFCTOP_BLOCK_SIZE                     0x10U       /**< Size of a MIFARE Classic Block */
#define PHAL_MFCTOP_SECTOR_DATA_SIZE               0x03U       /**< Number of Data blocks in a sector */
#define PHAL_MFCTOP_SECTOR_SIZE                    0x04U       /**< Size of a MIFARE Classic Sector */
#define PHAL_MFCTOP_MAD2_SECTOR_DATA_SIZE          0x0FU       /**< Number of Data blocks in a Sector of MAD2 after 0x1F sector */
#define PHAL_MFCTOP_MAD2_SECTOR_SIZE               0x10U       /**< Size of a MIFARE Classic Sector after 2K Memory */

/**
* \name LAST SECTOR NUMBER OF RESPECTIVE BRANCH LAYOUT
*/
/*@{* */
#define PHAL_MFCTOP_MFC1K_LAST_SECTOR             0x0FU       /**< MFC1K has total 16 Sectors, Each sector has 4 blocks */
#define PHAL_MFCTOP_MFC4K_LAST_SECTOR             0x27U       /**< MFC4K has total 40 Sectors, Initial 32 sector has 4 blocks each last 8 sectors have 16 blocks each*/
#define PHAL_MFCTOP_MFP2K_LAST_SECTOR             0x1FU       /**< MFP2K has total 32 Sectors, Each sector has 4 blocks */
#define PHAL_MFCTOP_MFP4K_LAST_SECTOR             0x27U       /**< MFC4K has total 40 Sectors, Initial 32 sector has 4 blocks each last 8 sectors have 16 blocks each*/


/**
* \name NFC TLV
*/
/*@{*/
#define PHAL_MFCTOP_NDEF_TLV                       0x03U       /**< NDEF TLV for NFC TOP */
#define PHAL_MFCTOP_TERMINATOR_TLV                 0xFEU       /**< TERMINATOR TLV FOR NFC*/


/**
* \name Card Types
*/
/*@{*/
#define PHAL_MFCTOP_NO_CARD                        0x00U       /**< No Card is selected */
#define PHAL_MFCTOP_MFC_1K                         0x01U       /**< MFC 1K is selected*/
#define PHAL_MFCTOP_MFC_4K                         0x02U       /**< MFC 4K is selected*/
#define PHAL_MFCTOP_MFP_2K                         0x03U       /**< MFP 2K is selected*/
#define PHAL_MFCTOP_MFP_4K                         0x04U       /**< MFP 4K is selected*/



/**
* \name Tag States
*/
/*@{*/
#define PHAL_MFCTOP_STATE_NONE                     0x00U                /**< Default initial state. */
#define PHAL_MFCTOP_STATE_INITIALIZED              0x01U                /**< Initialized state. */
#define PHAL_MFCTOP_STATE_READONLY                 0x02U                /**< Read Only state. */
#define PHAL_MFCTOP_STATE_READWRITE                0x04U                /**< Read/Write state. */


/**
* \name MAD Sector Trailer Access Bits
*/
/*@{*/

#define PHAL_MFCTOP_INITIALISED_MAD_SECTOR_TRAILER_ACCESS_BITS  0x03U     /**< MAD Sector Trailer Access bits in Initialised and READ/Write State.*/
#define PHAL_MFCTOP_READONLY_MAD_SECTOR_TRAILER_ACCESS_BITS     0x06U     /**< MAD Sector Trailer Access bits in Read Only State.*/
#define PHAL_MFCTOP_INITIALISED_MAD_DATA_AREA_ACCESS_BITS       0x04U     /**< MAD Data Area Access bits in Initialised and READ/Write State.*/
#define PHAL_MFCTOP_READONLY_MAD_DATA_AREA_ACCESS_BITS          0x02U     /**< MAD Data Area Access bits in Read Only State.*/
#define PHAL_MFCTOP_INITIALISED_SECTOR_TRAILER_ACCESS_BITS      0x03U     /**< Sector Trailer Access bits in Initialised and READ/Write State.*/
#define PHAL_MFCTOP_READONLY_SECTOR_TRAILER_ACCESS_BITS         0x06U     /**< Sector Trailer Access bits in Read Only State.*/
#define PHAL_MFCTOP_INITIALISED_DATA_AREA_ACCESS_BITS           0x00U     /**< Data Area Access bits in Initialised and READ/Write State.*/
#define PHAL_MFCTOP_READONLY_DATA_AREA_ACCESS_BITS              0x02U     /**< Data Area Access bits in Read Only State.*/


/**
* \name MAD Sector Block Number
*/
/*@{*/

#define PHAL_MFCTOP_MAD1_SECTOR                               0x00U      /**< Sector defined as MAD1.*/
#define PHAL_MFCTOP_MAD1_SECTORTRAILER_BLOCK                  0x03U      /**< Sector Trailer Block of MAD1.*/
#define PHAL_MFCTOP_MAD2_SECTOR                               0x10U      /**< Sector defined as MAD2.*/
#define PHAL_MFCTOP_MAD2_SECTORTRAILER_BLOCK                  0x43U      /**< Sector Trailer Block of MAD2.*/
#define PHAL_MFCTOP_MAD1_ZEROTH_BLOCK                         0x00U      /**< Zeroth Block of MAD1 */
#define PHAL_MFCTOP_MAD1_FIRST_BLOCK                          0x01U      /**< First Block of MAD1 */
#define PHAL_MFCTOP_MAD1_SECOND_BLOCK                         0x02U      /**< Second Block of MAD1 */
#define PHAL_MFCTOP_MAD2_ZEROTH_BLOCK                         0x40U      /**< Zeroth Block of MAD1 */
#define PHAL_MFCTOP_MAD2_FIRST_BLOCK                          0x41U      /**< First Block of MAD1 */
#define PHAL_MFCTOP_MAD2_SECOND_BLOCK                         0x42U      /**< Second Block of MAD1 */


/**
* \name NFC Sector GPB structure
*/
/*@{*/


#define PHAL_MFCTOP_GPB_WRITEACCESS                               0x03U      /**< Write Access Conditions for GPB*/
#define PHAL_MFCTOP_GPB_READACCESS                                0x0CU      /**< Read Access Conditions for GPB*/
#define PHAL_MFCTOP_GPB_MINORVERSION                              0x30U      /**< Minor Version Number for GPB*/
#define PHAL_MFCTOP_GPB_MAJORVERSION                              0xC0U      /**< Major Version Number for GPB*/


/**
* \name Version Number for the current Implementation
*/
/*@{*/
#define PHAL_MFCTOP_MAJORVERSION                              0x01U      /**< Major Version Number*/
#define PHAL_MFCTOP_MINORVERSION                              0x00U      /**< Minor Version Number*/

/**
* \name READ AND WRITE ACCESS CONDITION
*/
/*@{*/
#define PHAL_MFCTOP_ACCESS_GRANTED                           0x00U       /**< Access is granted */
#define PHAL_MFCTOP_ACCESS_DENIED                            0x03U       /**< Access is denied */


/**
* \name READ AND WRITE ACCESS CONDITION
*/
/*@{*/
#define PHAL_MFCTOP_GPB_BYTE                                 0x09       /**< GPB Byte Order */

/**
* \name Default GPB
*/
/*@{*/
#define PHAL_MFCTOP_DEFAULT_GPB                              0x69U       /**< Default GPB for a blank card */
#define PHAL_MFCTOP_MFC1K_GPB                                0xC1U       /**< Default GPB for a MFC1K card with MAD1 */
#define PHAL_MFCTOP_MFC4K_GPB                                0xC2U       /**< Default GPB for a card with MFC2 */

phStatus_t phalTop_Sw_Int_MfcTop_CheckNdef(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint8_t * pTagState
                                );

phStatus_t phalTop_Sw_Int_MfcTop_FormatNdef(
                                 phalTop_Sw_DataParams_t * pDataParams
                                 );

phStatus_t phalTop_Sw_Int_MfcTop_ReadNdef(
                               phalTop_Sw_DataParams_t * pDataParams,
                               uint8_t * pData,
                               uint32_t * pLength
                               );

phStatus_t phalTop_Sw_Int_MfcTop_WriteNdef(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint8_t * pData,
                                uint32_t dwLength
                                );

phStatus_t phalTop_Sw_Int_MfcTop_EraseNdef(
                                        phalTop_Sw_DataParams_t * pDataParams
                                        );


#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
#endif /* NXPBUILD__PHAL_TOP_SW */

#endif /* PHALMFCTOP_SW_H */
