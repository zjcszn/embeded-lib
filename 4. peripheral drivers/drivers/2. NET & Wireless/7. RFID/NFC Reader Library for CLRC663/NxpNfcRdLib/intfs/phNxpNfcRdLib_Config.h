/*----------------------------------------------------------------------------*/
/* Copyright 2016-2023 NXP                                                    */
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
* NXPNFCRDLIB Configuration File, User can change configuration based on use cases and system needs.
* Changes in these configuration leads to change in flash size and behavior of NxpNfcRdLib.
* $Author: $
* $Revision: $ (v07.09.00)
* $Date: $
*
*/

#ifndef PH_NXPNFCRDLIB_CONFIG_H
#define PH_NXPNFCRDLIB_CONFIG_H

#include "ph_NxpBuild.h"

/* Used in NFCLIB (Simplified API). */

/**< Maximum HAL Tx buffer size by allocated by Simplified API Layer. */
#if !defined(PH_NXPNFCRDLIB_CONFIG_HAL_TX_BUFFSIZE) && !defined(NXPBUILD__PHHAL_HW_RC663) && !defined(NXPBUILD__PHHAL_HW_PN5190) && !defined(NXPBUILD__PHHAL_HW_PN76XX)
    #define PH_NXPNFCRDLIB_CONFIG_HAL_TX_BUFFSIZE                    300U
#endif
#if !defined(PH_NXPNFCRDLIB_CONFIG_HAL_TX_BUFFSIZE) && (defined(NXPBUILD__PHHAL_HW_RC663) || defined(NXPBUILD__PHHAL_HW_PN5190) || defined(NXPBUILD__PHHAL_HW_PN76XX))
    #define PH_NXPNFCRDLIB_CONFIG_HAL_TX_BUFFSIZE                    1030U
#endif

/**< Maximum HAL Rx buffer size by allocated by Simplified API Layer. */
#if !defined(PH_NXPNFCRDLIB_CONFIG_HAL_RX_BUFFSIZE) && !defined(NXPBUILD__PHHAL_HW_RC663) && !defined(NXPBUILD__PHHAL_HW_PN5190) && !defined(NXPBUILD__PHHAL_HW_PN76XX)
    #define PH_NXPNFCRDLIB_CONFIG_HAL_RX_BUFFSIZE                    600U
#endif
#if !defined(PH_NXPNFCRDLIB_CONFIG_HAL_RX_BUFFSIZE) && (defined(NXPBUILD__PHHAL_HW_RC663) || defined(NXPBUILD__PHHAL_HW_PN5190) || defined(NXPBUILD__PHHAL_HW_PN76XX))
    #define PH_NXPNFCRDLIB_CONFIG_HAL_RX_BUFFSIZE                    2060U
#endif

#define PH_NXPNFCRDLIB_CONFIG_ATS_BUFF_LENGTH                    64U       /**< Maximum ATS response buffer length. */
#define PH_NXPNFCRDLIB_CONFIG_HCE_BUFF_LENGTH                    300U      /**< Buffer length for HCE used when UpdateBinary or Custom commands are supported. */

/* Used in Discovery Loop. */
#define PH_NXPNFCRDLIB_CONFIG_MAX_PROXIMITY_CARDS_SUPPORTED      0x01U    /**< Maximum proximity cards (Type A, Type B and Type F) detected by Discovery Loop. */
#define PH_NXPNFCRDLIB_CONFIG_MAX_VICINITY_CARDS_SUPPORTED       0x01U    /**< Maximum vicinity cards (ISO15693 and ISO18000p3m3) detected by Discovery Loop. */

/* Default guard times used in Discovery Loop and NFCLIB (Simplified API).
 * As per Nfc Forum Activity and EMVCo Specification. */
#define PH_NXPNFCRDLIB_CONFIG_TYPEA_GT                           5100U    /**< Guard time configuration for Type A poll in Micro seconds.*/
#define PH_NXPNFCRDLIB_CONFIG_TYPEB_GT                           5100U    /**< Guard time configuration for Type B poll in Micro seconds.*/
#define PH_NXPNFCRDLIB_CONFIG_TYPEF_GT                           20400U   /**< Guard time configuration for Type F poll in Micro seconds.*/
#define PH_NXPNFCRDLIB_CONFIG_B_TO_F_GT                          15300U   /**< Guard time configuration for Type F poll preceded by Type B poll. */
#define PH_NXPNFCRDLIB_CONFIG_TYPEV_GT                           5100U    /**< Guard time configuration for Type V poll in Micro seconds.*/
#define PH_NXPNFCRDLIB_CONFIG_I18000P3M3_GT                      1000U    /**< Guard time configuration for 18000p3m3 poll in Micro seconds.*/

/* Proprietary Technology Guard Time(PTGT) configuration.
 * Note: These values are achieved by considering NFC Forum Activity Specification and all the platforms(Frontends and NFC Controllers).
 * */
#define PH_NXPNFCRDLIB_CONFIG_PRE_PTGTA                          1000U    /**< Proprietary Technology Guard Time(PTGT) configuration in Micro seconds preceded by NFC-A polling. */
#define PH_NXPNFCRDLIB_CONFIG_PRE_PTGTB                          3800U    /**< Proprietary Technology Guard Time(PTGT) configuration in Micro seconds preceded by NFC-B polling. */
#define PH_NXPNFCRDLIB_CONFIG_PRE_PTGTF                          1000U    /**< Proprietary Technology Guard Time(PTGT) configuration in Micro seconds preceded by NFC-F polling. */
#define PH_NXPNFCRDLIB_CONFIG_PRE_PTGTV                          1000U    /**< Proprietary Technology Guard Time(PTGT) configuration in Micro seconds preceded by NFC-V polling. */
#define PH_NXPNFCRDLIB_CONFIG_PRE_PTGTACM                        1000U    /**< Proprietary Technology Guard Time(PTGT) configuration in Micro seconds preceded by NFC-ACM polling. */
#define PH_NXPNFCRDLIB_CONFIG_PRE_PTGT                           5100U    /**< Proprietary Technology Guard Time(PTGT) configuration in Micro seconds preceded by no other technology polling. */

/* Used in Discovery Loop and NFCLIB (Simplified API) as per EMVCo Specification. */
#ifndef PH_NXPNFCRDLIB_CONFIG_EMVCO_FIELD_OFF_DELAY_US
    #define PH_NXPNFCRDLIB_CONFIG_EMVCO_FIELD_OFF_DELAY_US       5100U    /**< Wait time tP as per EMVCo specification. Simplified API EMVCo profile uses this value for Card removal procedure. */
#endif /* PH_NXPNFCRDLIB_CONFIG_EMVCO_FIELD_OFF_DELAY_US */

#define PH_NXPNFCRDLIB_CONFIG_EMVCO_REMOVAL_RETRY_COUNT          3U       /**< Poll command retry count used in removal procedure of EMVCo profile. */

#define PH_NXPNFCRDLIB_CONFIG_EMVCO_PROP_TECH_POLL_RF_OFF_DELAY_US  5100U /**< Wait time tP as per EMVCo specification. Simplified API EMVCo profile uses this value for Card removal procedure. */

#if (defined(NXPBUILD__PHHAL_HW_RC663) || defined(NXPBUILD__PHHAL_HW_PN5190) || defined(NXPBUILD__PHHAL_HW_PN76XX)) && defined(PH_NXPNFCRDLIB_CONFIG_HIGHER_FSDI)
    #define PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE             10U      /**< Default FSCI value. */
    #define PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE                     10U      /**< With RC663 and PN5190 we can configure the required FSDI value. But as frame error detection with CRC16 is not good beyond 1KBytes Limiting the value to 10. */
#endif

#ifndef PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE
    #define PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE             8U       /**< Default FSCI value. This value is fixed for PN5180 and PN7462AU as these Readers doesn't support frame size. */
#endif /* PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE */

#ifndef PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE
    #define PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE                     8U       /**< Default FSDI Configuration. This value is fixed for PN5180 and PN7462AU as these Readers doesn't support frame size. */
#endif /* PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE */

/* TODO : Max retry of RATS and Attrib can be 2 in case of EMVCo. Need to check. */

/* EMVCo values used in PAL 14443-3a, PAL 14443-4a, PAL 14443-3b and Discovery Loop. */
#define PH_NXPNFCRDLIB_CONFIG_EMVCO_TMIN_RETRANSMISSION          3000U    /**< Tmin retransmission used to retransmit command in case of timeout error as per EMVCo 3.1.*/
#define PH_NXPNFCRDLIB_CONFIG_EMVCO_DEFAULT_RETRANSMISSION       PH_NXPNFCRDLIB_CONFIG_EMVCO_TMIN_RETRANSMISSION    /**< Default retransmission time to retransmit command in case of timeout error.*/
#define PH_NXPNFCRDLIB_CONFIG_EMVCO_RETRYCOUNT                   2U       /**< Retry count used to retransmit command in case of timeout error as per EMVCo 3.1.*/

/* User should update this macro with expected NDEF length that can be read from the tag.
 * For which buffer of this length needs to be allocated properly by User and pass this buffer to phalTop_ReadNdef() API.
 * */
#ifndef PH_NXPNFCRDLIB_CONFIG_MAX_NDEF_DATA
    #define PH_NXPNFCRDLIB_CONFIG_MAX_NDEF_DATA                  0x100U    /**< Default value is 256 bytes, Maximum NDEF DATA that can be exchanged at a time, Application Buffer for NDEF Exchange should equal this */
#endif /* PH_NXPNFCRDLIB_CONFIG_MAX_NDEF_DATA */

/** \name HAL Rc663 macros
 */

/** \name Memory related constants
*/
/*@{*/
#define PH_NXPNFCRDLIB_CONFIG_RC663_PRECACHED_255BYTES           0xFFU    /**< Maximum number of bytes to precache into FIFO before triggering a command for 255 FIFO. */
#define PH_NXPNFCRDLIB_CONFIG_RC663_PRECACHED_512BYTES           0x200U   /**< Maximum number of bytes to precache into FIFO before triggering a command for 512 FIFO. */
#define PH_NXPNFCRDLIB_CONFIG_RC663_DEFAULT_PRECACHED_BYTES      PH_NXPNFCRDLIB_CONFIG_RC663_PRECACHED_255BYTES /**< Default number of bytes to precache into FIFO. */
/*@}*/

/**
* \name FIFO configuration values
*/
/*@{*/
#define PH_NXPNFCRDLIB_CONFIG_RC663_VALUE_FIFOSIZE_255           0x0000U  /**< Configure FIFO-Size to 256 bytes */
#define PH_NXPNFCRDLIB_CONFIG_RC663_VALUE_FIFOSIZE_512           0x0001U  /**< Configure FIFO-Size to 512 bytes */
#define PH_NXPNFCRDLIB_CONFIG_RC663_DEFAULT_FIFOSIZE             PH_NXPNFCRDLIB_CONFIG_RC663_VALUE_FIFOSIZE_255 /**< Default FIFO-Size */
/*@}*/

#endif /* PH_NXPNFCRDLIB_CONFIG_H */
