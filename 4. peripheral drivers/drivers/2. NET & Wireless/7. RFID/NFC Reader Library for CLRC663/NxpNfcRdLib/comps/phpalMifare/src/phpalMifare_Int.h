/*----------------------------------------------------------------------------*/
/* Copyright 2009, 2021 NXP                                                   */
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
* Internal definitions for Software MIFARE product Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
* History:
*  CHu: Generated 31. July 2009
*
*/

#ifndef PHPALMIFARE_INT_H
#define PHPALMIFARE_INT_H

#include <ph_Status.h>

/** \addtogroup ph_Private
* @{
*/

#define PHPAL_MIFARE_RESP_ACK    0x0AU    /**< MIFARE product ACK response code */
#define PHPAL_MIFARE_RESP_NAK0   0x00U    /**< MIFARE product NAK0 response code */
#define PHPAL_MIFARE_RESP_NAK1   0x01U    /**< MIFARE product NAK1 response code */
#define PHPAL_MIFARE_RESP_NAK4   0x04U    /**< MIFARE product NAK4 response code */
#define PHPAL_MIFARE_RESP_NAK5   0x05U    /**< MIFARE product NAK5 response code */
#define PHPAL_MIFARE_RESP_NAK6   0x06U    /**< MIFARE product NAK6 response code */
#define PHPAL_MIFARE_RESP_NAK7   0x07U    /**< MIFARE product NAK7 response code */
#define PHPAL_MIFARE_RESP_NAK9   0x09U    /**< MIFARE product NAK9 response code */
#define PHPAL_MIFARE_RESP_NAK3   0x03U    /**< NTAG   product NAK3 response code */

/** @}
* end of ph_Private
*/

#endif /* PHPALMIFARE_INT_H */
