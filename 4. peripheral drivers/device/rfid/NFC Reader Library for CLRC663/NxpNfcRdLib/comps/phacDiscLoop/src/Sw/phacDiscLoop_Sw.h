/*----------------------------------------------------------------------------*/
/* Copyright 2012-2021 NXP                                                    */
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
* This File contains all functions that's needed by Discovery Loop
* for Starting the discovery loop, Set & Get Configuration and Card Activation,
* as part of the Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/
#ifndef PHACDICLOOP_SW_H
#define PHACDICLOOP_SW_H

#include <ph_Status.h>
#include <phNxpNfcRdLib_Config.h>

#ifdef NXPBUILD__PHAC_DISCLOOP_SW

#define PHAC_DISCLOOP_SW_TYPEF_MAX_TSN                          16U    /**< Maximum time slot number for Type F. */
#define PHAC_DISCLOOP_SW_I3P4_MAX_CID                           14U    /**< Maximum permissible size of CID(Card IDentifier). */
#define PHAC_DISCLOOP_SW_I3P4_MAX_DSI                            3U    /**< Maximum permissible size of DSI(Divisor Send Integer). */
#define PHAC_DISCLOOP_SW_I3P4_MAX_DRI                            3U    /**< Maximum permissible size of DRI(Divisor Receive Integer). */

/* Default device limit for poll technologies */
#define PHAC_DISCLOOP_TYPEA_DEFAULT_DEVICE_LIMIT              0x01U    /**< The default value loaded for device limit for Type A tags. */
#define PHAC_DISCLOOP_TYPEB_DEFAULT_DEVICE_LIMIT              0x01U    /**< Default value for maximum number of TypeB tags that can be detected. */
#define PHAC_DISCLOOP_TYPEF_DEFAULT_DEVICE_LIMIT              0x01U    /**< Default value for maximum number of TypeF tags that can be detected. */
#define PHAC_DISCLOOP_TYPEV_DEFAULT_DEVICE_LIMIT              0x01U    /**< Default value loaded for device limit for Type V tags is equal to #PHAC_DISCLOOP_CFG_MAX_VICINITY_CARDS_SUPPORTED. */
#define PHAC_DISCLOOP_I18000P3M3_DEFAULT_DEVICE_LIMIT         0x01U    /**< Default value loaded for device limit for 18000p3m3 tags is equal to #PHAC_DISCLOOP_CFG_MAX_VICINITY_CARDS_SUPPORTED.. */

/**
 * \brief Function to start the discovery loop
 *
 * \return Error code
 * \retval #PH_ERR_SUCCESS
 * \retval Other errors depends on component.
 *         For more info about various components present refer
 *         NFCReaderlibrary.chm,
 *         "Component identifiers" under the section "Generic Definitions".
 *         For info about Error Codes Refer "Error Code Collection"
 */
phStatus_t phacDiscLoop_Sw_Run(
    phacDiscLoop_Sw_DataParams_t * pDataParams,            /**< [In] pointer to this layer's parameter structure. */
    uint8_t bEntryPoint                                    /**< [In] Entry point to start Discovery Loop. */
    );

/**
 * \brief   Function to Get the configuration value
 *
 * \return  Error code
 *
 * \retval #PH_ERR_SUCCESS         operation successful
 * \retval #PH_ERR_PROTOCOL_ERROR  Invalid option/response received
 * \retval Other errors depends on component.
 *         For more info about various components present refer
 *         NFCReaderlibrary.chm,
 *         "Component identifiers" under the section "Generic Definitions".
 *         For info about Error Codes Refer "Error Code Collection"
 */
phStatus_t phacDiscLoop_Sw_GetConfig(
    phacDiscLoop_Sw_DataParams_t *  pDataParams,           /**< [In] Pointer to this layer's parameter structure */
    uint16_t wConfig,                                      /**< [In] Configuration identifier */
    uint16_t *pValue                                       /**< [Out] Configuration value */
    );

/**
 * \brief Function to Set the configuration value
 *
 * \return Error code
 * \retval #PH_ERR_SUCCESS            operation successful
 * \retval #PH_ERR_INVALID_PARAMETER  Invalid option/response received
 * \retval Other errors depends on component.
 *         For more info about various components present refer
 *         NFCReaderlibrary.chm,
 *         "Component identifiers" under the section "Generic Definitions".
 *         For info about Error Codes Refer "Error Code Collection"
 */
phStatus_t phacDiscLoop_Sw_SetConfig(
    phacDiscLoop_Sw_DataParams_t *  pDataParams,           /**< [In] Pointer to this layer's parameter structure */
    uint16_t wConfig,                                      /**< [In] Configuration identifier */
    uint16_t wValue                                        /**< [In] Configuration value */
    );

/**
 * \brief Activates the given tag type with given index
 *
 * \return Error code
 * \retval #PH_ERR_SUCCESS      On success
 * \retval #PH_ERR_INVALID_ARG  If invalid index or card type was given
 */
phStatus_t phacDiscLoop_Sw_ActivateCard(
    phacDiscLoop_Sw_DataParams_t *  pDataParams,           /**< [In] Pointer to this layer's parameter structure */
    uint8_t bTechType,                                      /**< [In] Tag type could be Type A,B or F */
    uint8_t bTagIndex                                      /**< [In] The tag which has to be activated */
    );

/**
 * \brief Function to Configure the Discovery loop Passive Polling loop Sequence.
 *
 * \return Error code
 * \retval #PH_ERR_SUCCESS            operation successful
 * \retval #PH_ERR_INVALID_PARAMETER  Invalid option/response received
 */
phStatus_t phacDiscLoop_Sw_CfgPollSeq(
    phacDiscLoop_Sw_DataParams_t * pDataParams,
    uint8_t * pDiscloopCfgPollSeq
    );

#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
#endif /* PHACDICLOOP_H */
