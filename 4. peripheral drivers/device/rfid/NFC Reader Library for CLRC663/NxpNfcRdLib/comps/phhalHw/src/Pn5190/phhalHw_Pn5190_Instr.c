/*----------------------------------------------------------------------------*/
/* Copyright 2019-2023 NXP                                                    */
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
* PN5190 HAL Instruction layer.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <ph_RefDefs.h>
#include <ph_Status.h>
#include <phhalHw.h>
#include <phDriver.h>

#ifdef _WIN32

#define PH_DRIVER_SET_LOW  0
#define PH_DRIVER_SET_HIGH 1

#define RESET_POWERDOWN_LEVEL  PH_DRIVER_SET_LOW
#define RESET_POWERUP_LEVEL    PH_DRIVER_SET_HIGH

#endif

#ifdef NXPBUILD__PHHAL_HW_PN5190
#include "phhalHw_Pn5190.h"
#include "phhalHw_Pn5190_InstrMngr.h"
#include <phhalHw_Pn5190_Instr.h>
#include "phhalHw_Pn5190_Int.h"
#include <phhalHw_Pn5190_Reg.h>


phStatus_t phhalHw_Pn5190_Instr_WriteRegister(
                                              phhalHw_Pn5190_DataParams_t * pDataParams,
                                              uint8_t bRegister,
                                              uint32_t dwValue
                                              )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    uint8_t  PH_MEMLOC_REM bIndex = 0U;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aInstrBuffer[5] = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Check if the Register address is valid. */
    if (bRegister > PHHAL_HW_PN5190_REG_END_ADDRESS)
    {
        if((bRegister < PHHAL_HW_PN5190_REG_START_ADDRESS_EXPERT) ||
                (bRegister > PHHAL_HW_PN5190_REG_END_ADDRESS_EXPERT))

        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_WRITE_REGISTER;

    sCmdParams.bQueue = true;

    /* Value */
    aInstrBuffer[bIndex++] = bRegister;
    aInstrBuffer[bIndex++] = (uint8_t)(dwValue & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwValue >> 8U) & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwValue >> 16U) & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwValue >> 24U) & 0xFFU);

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
               pDataParams,
               &sCmdParams,
               PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_WriteRegisterOrMask(
                                                    phhalHw_Pn5190_DataParams_t * pDataParams,
                                                    uint8_t bRegister,
                                                    uint32_t dwMask
                                                    )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    uint8_t  PH_MEMLOC_REM bIndex = 0U;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aInstrBuffer[5] = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Check if the Register address is valid. */
    if (bRegister > PHHAL_HW_PN5190_REG_END_ADDRESS)
    {
        if((bRegister < PHHAL_HW_PN5190_REG_START_ADDRESS_EXPERT) ||
                (bRegister > PHHAL_HW_PN5190_REG_END_ADDRESS_EXPERT))

        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_WRITE_REGISTER_OR_MASK;

    sCmdParams.bQueue = true;

    /* Value */
    aInstrBuffer[bIndex++] = bRegister;
    aInstrBuffer[bIndex++] = (uint8_t)(dwMask & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwMask >> 8U) & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwMask >> 16U) & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwMask >> 24U) & 0xFFU);

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_WriteRegisterAndMask(
                                                     phhalHw_Pn5190_DataParams_t * pDataParams,
                                                     uint8_t bRegister,
                                                     uint32_t dwMask
                                                     )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    uint8_t  PH_MEMLOC_REM bIndex = 0U;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aInstrBuffer[5] = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Check if the Register address is valid. */
    if (bRegister > PHHAL_HW_PN5190_REG_END_ADDRESS)
    {
        if((bRegister < PHHAL_HW_PN5190_REG_START_ADDRESS_EXPERT) ||
                (bRegister > PHHAL_HW_PN5190_REG_END_ADDRESS_EXPERT))

        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_WRITE_REGISTER_AND_MASK;

    sCmdParams.bQueue = true;

    /* Value */
    aInstrBuffer[bIndex++] = bRegister;
    aInstrBuffer[bIndex++] = (uint8_t)(dwMask & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwMask >> 8U) & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwMask >> 16U) & 0xFFU);
    aInstrBuffer[bIndex++] = (uint8_t)((dwMask >> 24U) & 0xFFU);

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_WriteRegisterMultiple(
                                                      phhalHw_Pn5190_DataParams_t * pDataParams,
                                                      uint8_t * pRegTypeValueSets,
                                                      uint16_t wSizeOfRegTypeValueSets
                                                      )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if (((pDataParams == NULL)) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate
    * pRegTypeValueSets
    * wSizeOfRegTypeValueSets
    * wSizeOfRegTypeValueSets for modulus of 6. One set of write register is of 6 byte*/

    if ((NULL == pRegTypeValueSets)
        || (wSizeOfRegTypeValueSets > PHHAL_HW_PN5190_MAX_REGISTER_TYPE_VALUE_SET)
        || (wSizeOfRegTypeValueSets < PHHAL_HW_PN5190_MIN_REGISTER_TYPE_VALUE_SET)
        || ( (wSizeOfRegTypeValueSets % PHHAL_HW_PN5190_MIN_REGISTER_TYPE_VALUE_SET) != 0U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Build the command frame */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_WRITE_REGISTER_MULTIPLE;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = pRegTypeValueSets;
    sCmdParams.wTxDataLength = wSizeOfRegTypeValueSets;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_ReadRegister(
                                             phhalHw_Pn5190_DataParams_t * pDataParams,
                                             uint8_t bRegister,
                                             uint32_t * pValue
                                             )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aInstrBuffer[1] = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;
    uint8_t PH_MEMLOC_REM bIndex = 0U;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Check if the Register address and input parameter is valid. */
    if (bRegister > PHHAL_HW_PN5190_REG_END_ADDRESS)
    {
        if((bRegister < PHHAL_HW_PN5190_REG_START_ADDRESS_EXPERT) ||
                (bRegister > PHHAL_HW_PN5190_REG_END_ADDRESS_EXPERT))

        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    if (pValue == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_READ_REGISTER;

    sCmdParams.bQueue = false;

    aInstrBuffer[bIndex++] = bRegister;

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if(status == PH_ERR_PN5190_SUCCESS)
    {
        *pValue = pRxData[0];
        *pValue |= (((uint32_t) pRxData[1]) << 8U);
        *pValue |= (((uint32_t) pRxData[2]) << 16U);
        *pValue |= (((uint32_t) pRxData[3]) << 24U);
    }
    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_ReadRegisterMultiple(
                                                     phhalHw_Pn5190_DataParams_t * pDataParams,
                                                     uint8_t * pRegisters,
                                                     uint8_t bNumOfRegisters,
                                                     uint8_t ** ppValues
                                                     )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    uint8_t PH_MEMLOC_REM bNumExpBytes = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Check input parameters. */
    if ((ppValues == NULL) || (pRegisters == NULL) ||
        (bNumOfRegisters > PHHAL_HW_PN5190_REG_END_ADDRESS))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    bNumExpBytes =  bNumOfRegisters * PHHAL_HW_PN5190_BYTES_PER_REGISTER;

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_READ_REGISTER_MULTIPLE;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = pRegisters;
    sCmdParams.wTxDataLength = bNumOfRegisters;

    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if(status == PH_ERR_PN5190_SUCCESS)
    {
        wRxLen = *sCmdParams.pRxLength;

        /* Validate the response length */
        if (wRxLen  != bNumExpBytes)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INTERNAL_ERROR, PH_COMP_HAL);
        }
        *ppValues = *sCmdParams.ppRxBuffer;
    }
    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_WriteE2Prom(
                                            phhalHw_Pn5190_DataParams_t * pDataParams,
                                            uint16_t wE2PromAddress,
                                            uint8_t * pDataToWrite,
                                            uint16_t wDataLength
                                            )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aAddnBuffer[2U] = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Check if the Register address is valid. */
    if (((wE2PromAddress + wDataLength) > PHHAL_HW_PN5190_EEPROM_END_ADDRS) ||
        (wDataLength == 0U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_WRITE_E2PROM;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = pDataToWrite;
    sCmdParams.wTxDataLength = wDataLength;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    aAddnBuffer[0] = (uint8_t)wE2PromAddress;
    aAddnBuffer[1] =  (uint8_t)(wE2PromAddress >> 8);
    sCmdParams.wAddnDataLen = 2U;
    sCmdParams.pAddnData = &aAddnBuffer[0];

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_ReadE2Prom(
                                           phhalHw_Pn5190_DataParams_t * pDataParams,
                                           uint16_t wE2PromAddress,
                                           uint8_t * pReadData,
                                           uint16_t wDataLength
                                           )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aInstrBuffer[4] = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t PH_MEMLOC_REM wRxLen = 0U;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Check if the Register address is valid. */
    if (((wE2PromAddress + wDataLength) > PHHAL_HW_PN5190_EEPROM_END_ADDRS) ||
        (pReadData == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_READ_E2PROM;

    sCmdParams.bQueue = false;

    aInstrBuffer[bIndex++] = (uint8_t) wE2PromAddress ;
    aInstrBuffer[bIndex++] = (uint8_t)(wE2PromAddress >> (8U));
    aInstrBuffer[bIndex++] = (uint8_t) wDataLength;
    aInstrBuffer[bIndex++] = (uint8_t) (wDataLength >> 8U);

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.ppRxBuffer = &pReadData;
    sCmdParams.pRxLength = &wRxLen;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);;
}

phStatus_t phhalHw_Pn5190_Instr_ReadCRCUserAreaE2Prom(
                                             phhalHw_Pn5190_DataParams_t * pDataParams,
                                             uint32_t * pValue
                                             )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    if (pValue == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_GET_CRC_USER_AREA_EEPROM;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = NULL;
    sCmdParams.wTxDataLength = 0U;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if(status == PH_ERR_PN5190_SUCCESS)
    {
        *pValue = pRxData[0];
        *pValue |= (((uint32_t) pRxData[1]) << 8U);
        *pValue |= (((uint32_t) pRxData[2]) << 16U);
        *pValue |= (((uint32_t) pRxData[3]) << 24U);
    }
    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_TransmitRFData(
    phhalHw_Pn5190_DataParams_t * pDataParams,
    uint16_t wOption,
    uint8_t bTxLastBits,
    uint8_t * pTxBuffer,
    uint16_t wTxBufferLength
    )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aAddnBuffer[2] = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate transmission buffer length
    * To validate bNumberOfValidBits indicate the exact data length to be transmitted.
    * */
    if (((pTxBuffer == NULL) && (wTxBufferLength != 0U) && ((wOption & PH_TRANSMIT_PREV_FRAME) == 0))||
        (wTxBufferLength > PHHAL_HW_PN5190_TX_DATA_MAX_LENGTH) ||
        (bTxLastBits > PHHAL_HW_PN5190_MAX_NUM_OF_BITS_FOR_LAST_BYTE))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    if ((0U == (wOption & (PH_TRANSMIT_LEAVE_BUFFER_BIT  | PH_TRANSMIT_PREV_FRAME))))
    {
        /* clear internal buffer if requested */
        pDataParams->wTxBufLen = 0U;
        pDataParams->wTxBufStartPos = 0U;
        pDataParams->wTxFwDataLen = 0U;
    }

    aAddnBuffer[bIndex++] = bTxLastBits;
    aAddnBuffer[bIndex++] = 0;

    sCmdParams.pAddnData = &aAddnBuffer[0];
    sCmdParams.wAddnDataLen = bIndex;

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_TRANSMIT_RF_DATA;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = pTxBuffer;
    sCmdParams.wTxDataLength = wTxBufferLength;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       wOption);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_RetrieveRFData(
                                               phhalHw_Pn5190_DataParams_t * pDataParams,
                                               uint8_t ** pRxBuffer,
                                               uint16_t * pRxDataLength
                                               )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t * pTmpRxBuffer = 0U;
    uint16_t wTmpRxBufferLen = 0U;
    uint16_t wTmpRxBufferSize = 0U;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameters. */
    if ((pRxBuffer == NULL) ||
        (pRxDataLength == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    phhalHw_Pn5190_Int_GetRxBuffer(pDataParams,
        &pTmpRxBuffer,
        &wTmpRxBufferLen,
        &wTmpRxBufferSize);

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_RETRIEVE_RF_DATA;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = NULL;
    sCmdParams.wTxDataLength = 0;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pTmpRxBuffer;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if (status == PH_ERR_PN5190_SUCCESS)
    {
        *pRxBuffer = *sCmdParams.ppRxBuffer;
        *pRxDataLength = *sCmdParams.pRxLength;
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_RetrieveRFFeliCaEMDData(
                                                        phhalHw_Pn5190_DataParams_t * pDataParams,
                                                        uint8_t bRetrieveConfigMask,
                                                        uint8_t ** pRxBuffer,
                                                        uint16_t * pRxDataLength
                                                        )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t * pTmpRxBuffer = 0U;
    uint16_t wTmpRxBufferLen = 0U;
    uint16_t wTmpRxBufferSize = 0U;
    uint8_t bInstrData = bRetrieveConfigMask;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameters. */
    if ((pRxBuffer == NULL) ||
        (pRxDataLength == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    phhalHw_Pn5190_Int_GetRxBuffer(pDataParams,
        &pTmpRxBuffer,
        &wTmpRxBufferLen,
        &wTmpRxBufferSize);

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_RETRIEVE_RF_FELICA_EMD_DATA;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = &bInstrData;
    sCmdParams.wTxDataLength = 1;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pTmpRxBuffer;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if (status == PH_ERR_PN5190_SUCCESS)
    {
        *pRxBuffer = *sCmdParams.ppRxBuffer;
        *pRxDataLength = *sCmdParams.pRxLength;
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_ReceiveRFData(
                                               phhalHw_Pn5190_DataParams_t * pDataParams,
                                               uint8_t bRespConfigMask,
                                               uint8_t ** ppRxBuffer,
                                               uint16_t * pRxDataLength
                                               )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t * pTmpRxBuffer = 0U;
    uint8_t bInstrData = bRespConfigMask;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameters. */
    if ((ppRxBuffer == NULL) ||
        (pRxDataLength == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_RECEIVE_RF_DATA;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = &bInstrData;
    sCmdParams.wTxDataLength = 1;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pTmpRxBuffer;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_RECEIVE_DEFAULT);


    if(status == PH_ERR_PN5190_SUCCESS)
    {
        *ppRxBuffer = *sCmdParams.ppRxBuffer;
        *pRxDataLength = *sCmdParams.pRxLength;
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_ExchangeRFData(
                                               phhalHw_Pn5190_DataParams_t * pDataParams,
                                               uint16_t wOption,
                                               uint8_t bTxLastBits,
                                               uint8_t bRespControl,
                                               uint8_t * pTxData,
                                               uint16_t wTxDataLength,
                                               uint8_t ** ppRxData,
                                               uint16_t * pRxDataLength
                                               )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_INTERNAL_ERROR;
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;
    uint8_t PH_MEMLOC_REM aInstrBuffer[2] = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

   /* Validate input parameters */
    if (((pTxData == NULL) && (wTxDataLength != 0U)) ||
        (wTxDataLength > PHHAL_HW_PN5190_TX_DATA_MAX_LENGTH) ||
        (bTxLastBits > PHHAL_HW_PN5190_MAX_NUM_OF_BITS_FOR_LAST_BYTE))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_EXCHANGE_RF_DATA;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = pTxData;
    sCmdParams.wTxDataLength = wTxDataLength;

    if ((0U == ((wOption & PH_EXCHANGE_LEAVE_BUFFER_BIT))))
    {
        /* clear internal buffer if requested */
        pDataParams->wTxBufLen = 0U;
        pDataParams->wTxBufStartPos = 0U;
        pDataParams->wTxFwDataLen = 0U;
    }

    aInstrBuffer[bIndex++] = bTxLastBits;
    aInstrBuffer[bIndex++] = bRespControl;

    sCmdParams.pAddnData = &aInstrBuffer[0];
    sCmdParams.wAddnDataLen = bIndex;

    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    status =  phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       wOption);

    if(status == PH_ERR_PN5190_SUCCESS)
    {
        if (ppRxData != NULL)
        {
            *ppRxData = *sCmdParams.ppRxBuffer;
        }
        if (pRxDataLength != NULL)
        {
            *pRxDataLength = *sCmdParams.pRxLength;
        }
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_SwitchModeStandby(
                                                  phhalHw_Pn5190_DataParams_t * pDataParams,
                                                  uint8_t bConfigMask,
                                                  uint16_t wWakeupCounterInMs
                                                  )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_INTERNAL_ERROR;
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aInstrBuffer[3] = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    aInstrBuffer[bIndex++] = bConfigMask;
    aInstrBuffer[bIndex++] = (uint8_t)wWakeupCounterInMs;
    aInstrBuffer[bIndex++] = (uint8_t)(wWakeupCounterInMs >> 8);

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_SWITCH_MODE_STANDBY;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if( status == PH_ERR_PN5190_SUCCESS)
    {
        pDataParams->dwExpectedEvent = PH_PN5190_EVT_BOOT;
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_LPCD_SetConfig(
                                               phhalHw_Pn5190_DataParams_t * pDataParams,
                                               uint16_t wConfig,
                                               uint32_t dwValue
                                               )
{
    phStatus_t  PH_MEMLOC_REM statusTmp = PH_ERR_PN5190_SUCCESS;

    switch(wConfig)
    {

    case PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS:
    {
        /* Value should be greater than 3ms */
        if(dwValue >= 0x03U)
        {
            pDataParams->wLPCDWakeupCounterInMs = dwValue;
        }
        else
        {
            pDataParams->wLPCDWakeupCounterInMs = 0x03U;
        }
     }
     break;

    case PHHAL_HW_CONFIG_LPCD_MODE:
        if((dwValue == PHHAL_HW_PN5190_LPCD_MODE_DEFAULT) || (dwValue ==  PHHAL_HW_PN5190_LPCD_MODE_POWERDOWN))
        {
            pDataParams->bLpcdMode = (uint8_t)dwValue;
        }
        else
        {
            statusTmp = PH_ERR_PN5190_INVALID_PARAMETER;
        }
        break;

    case PHHAL_HW_CONFIG_LPCD_CONFIG:
        if ((dwValue <=  PHHAL_HW_PN5190_LPCD_CTRL_SINGLEMODE_LPCD) ||
            (dwValue == PHHAL_HW_PN5190_LPCD_CTRL_SINGLEMODE_CALIB))
        {
            pDataParams->bLpcdConfig = (uint8_t)dwValue;
        }
        else
        {
            statusTmp = PH_ERR_PN5190_INVALID_PARAMETER;
        }
        break;

    case PHHAL_HW_CONFIG_LPCD_WAKEUP_CTRL:
        pDataParams->bLpcdWakeUpCtrl = (uint8_t)dwValue;
        break;

    case PHHAL_HW_CONFIG_LPCD_REF_VAL:
        pDataParams->dwLpcdRefVal = dwValue;
        break;

    case PHHAL_HW_CONFIG_LPCD_HFATT_VAL:
        pDataParams->bHFATTVal = (uint8_t)dwValue;
        break;

    default:
        statusTmp = PH_ERR_PN5190_UNSUPPORTED_PARAMETER;
        break;
    }

    return PH_ADD_COMPCODE(statusTmp, PH_COMP_HAL);
}


phStatus_t phhalHw_Pn5190_Instr_LPCD_GetConfig(
                                               phhalHw_Pn5190_DataParams_t * pDataParams,
                                               uint16_t wConfig,
                                               uint32_t * pValue
                                               )
{
    switch(wConfig)
    {
    case PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS:
        *pValue = pDataParams->wLPCDWakeupCounterInMs;
        break;

    case PHHAL_HW_CONFIG_LPCD_MODE:
        *pValue = pDataParams->bLpcdMode;
        break;

    case PHHAL_HW_CONFIG_LPCD_WAKEUP_CTRL:
        *pValue = pDataParams->bLpcdWakeUpCtrl;
        break;

    case PHHAL_HW_CONFIG_LPCD_REF_VAL:
        *pValue = pDataParams->dwLpcdRefVal;
        break;

    case PHHAL_HW_CONFIG_LPCD_RETRIEVE_EVENT_STATUS:
        *pValue = pDataParams->dwEventStatus;
        break;

    case PHHAL_HW_CONFIG_LPCD_HFATT_VAL:
        *pValue = pDataParams->bHFATTVal;
        break;

    default:
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
    }

    return PH_ERR_PN5190_SUCCESS;
}

phStatus_t phhalHw_Pn5190_Instr_SwitchModeLpcd(
                                               phhalHw_Pn5190_DataParams_t * pDataParams,
                                               uint8_t bLpcdMode,
                                               uint8_t bWakeUpCntrlHwLpcd,
                                               uint32_t dwLpcdRefValue,
                                               uint16_t wWakeupCounterInMs
                                               )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_INTERNAL_ERROR;
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aInstrBuffer[8] = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    aInstrBuffer[bIndex++] = bLpcdMode;
    aInstrBuffer[bIndex++] = bWakeUpCntrlHwLpcd;
    aInstrBuffer[bIndex++] = (uint8_t) dwLpcdRefValue;
    aInstrBuffer[bIndex++] = (uint8_t) (dwLpcdRefValue >> 8);
    aInstrBuffer[bIndex++] = (uint8_t) (dwLpcdRefValue >> 16);
    aInstrBuffer[bIndex++] = (uint8_t) (dwLpcdRefValue >> 24);
    aInstrBuffer[bIndex++] = (uint8_t) wWakeupCounterInMs;
    aInstrBuffer[bIndex++] = (uint8_t) (wWakeupCounterInMs >> 8);

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_SWITCH_MODE_LPCD;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_SwitchModeAutocoll(
                                                   phhalHw_Pn5190_DataParams_t * pDataParams,
                                                   uint8_t bRfTechnologyMask,
                                                   uint8_t bAutocollMode
                                                   )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aInstrBuffer[2] = {0U, 0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    if(0U == bRfTechnologyMask)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_SWITCH_MODE_AUTOCOLL;

    sCmdParams.bQueue = false;

    aInstrBuffer[bIndex++] = bRfTechnologyMask;
    aInstrBuffer[bIndex++] = bAutocollMode;

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;

    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
            pDataParams, &sCmdParams, PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_SwitchModeBootNormal(
                                                     phhalHw_Pn5190_DataParams_t * pDataParams
                                                     )
{
    uint8_t  PH_MEMLOC_REM aCmd[4] = {0};

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    aCmd[0]  = PHHAL_HW_PN5190_INT_SPI_WRITE;
    aCmd[1]  = 0x20;
    aCmd[2]  = 0x01;
    aCmd[3]  = 0x00;

    return phhalHw_Pn5190_Send(pDataParams,&aCmd[0], 4, PH_OFF);
}

phStatus_t phhalHw_Pn5190_Instr_SwitchModeNormal(
                                                 phhalHw_Pn5190_DataParams_t * pDataParams
                                                 )
{
    uint8_t    PH_MEMLOC_REM aCmd[4] = {0};
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_INTERNAL_ERROR;
    uint32_t   PH_MEMLOC_REM dwEventReceived = 0x0U;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    aCmd[0]  = PHHAL_HW_PN5190_INT_SPI_WRITE;
    aCmd[1]  = 0x20;
    aCmd[2]  = 0x00;
    aCmd[3]  = 0x00;

    PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_Send(pDataParams,&aCmd[0], 4, PH_ON));

    /*Wait for event*/
    status = phhalHw_Pn5190_Int_EventWait(
            pDataParams,
            PH_PN5190_EVT_IDLE | PH_PN5190_EVT_GENERAL_ERROR,
            PHOSAL_MAX_DELAY,
            true,
            &dwEventReceived);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_SwitchModeIdle(
                                               phhalHw_Pn5190_DataParams_t * pDataParams
                                               )
{
    uint8_t    PH_MEMLOC_REM aCmd[4] = {0};
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_INTERNAL_ERROR;
    uint32_t   PH_MEMLOC_REM dwEventReceived = 0x0U;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    aCmd[0]  = PHHAL_HW_PN5190_INT_SPI_WRITE;
    aCmd[1]  = 0x20;
    aCmd[2]  = 0x02;
    aCmd[3]  = 0x00;

    PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_Send(pDataParams, &aCmd[0], 4, PH_ON));

    /*Wait for event*/
    status = phhalHw_Pn5190_Int_EventWait(
            pDataParams,
            PH_PN5190_EVT_IDLE | PH_PN5190_EVT_GENERAL_ERROR,
            PHOSAL_MAX_DELAY,
            true,
            &dwEventReceived);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_MifareAuthenticate(
                                                   phhalHw_Pn5190_DataParams_t * pDataParams,
                                                   uint8_t * pKey,
                                                   uint8_t bKeyType,
                                                   uint8_t bBlockNo,
                                                   uint8_t * pUid
                                                   )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    uint16_t    PH_MEMLOC_REM wDataLenTmp;
    uint16_t    PH_MEMLOC_REM wBufferLength = 0;

    uint8_t    PH_MEMLOC_BUF aCmdBuffer[13];
    uint8_t  * PH_MEMLOC_BUF aRspBuffer;
    uint32_t   dwRegister;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameters */
    if ((NULL == pKey) ||
        (NULL == pUid) ||
        ((bKeyType != PHHAL_HW_MFC_KEYA) && (bKeyType != PHHAL_HW_MFC_KEYB)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Build the command frame */
    wBufferLength = 0U;

    /* Write Tx Data */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_MFC_AUTHENTICATE;

    sCmdParams.bQueue = false;

    /* Copy the data to be transmitted*/
    (void)memcpy(&aCmdBuffer[wBufferLength], pKey, PHHAL_HW_MFC_KEY_LENGTH);
    wBufferLength += PHHAL_HW_MFC_KEY_LENGTH;

    if(bKeyType == PHHAL_HW_MFC_KEYA)
    {
        aCmdBuffer[wBufferLength++] = PHHAL_HW_PN5190_INSTR_MFC_AUTHENTICATE_KEY_TYPE_A;
    }
    else
    {
        aCmdBuffer[wBufferLength++] = PHHAL_HW_PN5190_INSTR_MFC_AUTHENTICATE_KEY_TYPE_B;
    }

    aCmdBuffer[wBufferLength++] = bBlockNo;
    (void)memcpy(&aCmdBuffer[wBufferLength], pUid, PHHAL_HW_PN5190_MFC_UID_LEN);
    wBufferLength += PHHAL_HW_PN5190_MFC_UID_LEN;

    sCmdParams.pTxDataBuff = &aCmdBuffer[0];
    sCmdParams.wTxDataLength = wBufferLength;
    sCmdParams.ppRxBuffer = &aRspBuffer;
    sCmdParams.pRxLength = &wDataLenTmp;

    /* Send it to the chip */
    status =  phhalHw_Pn5190_InstMngr_HandleCmd(
            pDataParams,
            &sCmdParams,
            PH_EXCHANGE_DEFAULT);

    /* Validate the response */
    if(PH_ERR_PN5190_SUCCESS == status)
    {
        /* Check auth success */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_ReadRegister(pDataParams, SYSTEM_CONFIG, &dwRegister));
        if((0U == ((dwRegister & SYSTEM_CONFIG_MFC_CRYPTO_ON_MASK))))
        {
            status = PH_ERR_AUTH_ERROR;
            pDataParams->bMfcCryptoEnabled = PH_OFF;
        }
        else
        {
            pDataParams->bMfcCryptoEnabled = PH_ON;
        }
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_EpcGen2Inventory(
                                                 phhalHw_Pn5190_DataParams_t * pDataParams,
                                                 uint8_t * pSelectCommand,
                                                 uint8_t bSelectCommandLength,
                                                 uint8_t bSelectCommandBitsInLastByte,
                                                 uint8_t * pBeginRoundCommand,
                                                 uint8_t bTimeslotProcessingBehavior,
                                                 uint8_t ** ppRxBuffer,
                                                 uint16_t * pRxDataLength
                                                 )
{

    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wBufferLength = 0;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;

    uint8_t * pTmpRxBuffer = 0U;
    uint16_t wTmpRxBufferLen = 0U;
    uint16_t wTmpRxBufferSize = 0U;

    uint8_t    PH_MEMLOC_BUF aCmdBuffer[46];

    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};

    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))

    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /*
    Validate pBeginRoundCommand
    Validate bTimeslotProcessingBehavior */
    if ((NULL == pBeginRoundCommand)
        || (bTimeslotProcessingBehavior > PHHAL_HW_PN5190_MAX_EPC_GEN2_TIMESLOT))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Validate the parameters of Select command */
    if (0U != (bSelectCommandLength))
    {
        /* Validate pSelectCommand
        Validate bSelectCommandLength
        Validate bSelectCommandBitsInLastByte */
        if (                            \
            (NULL == pSelectCommand)    \
            || (bSelectCommandLength > PHHAL_HW_PN5190_MAX_SELECT_COMMAND_LENGTH)    \
            ||  (bSelectCommandBitsInLastByte > PHHAL_HW_PN5190_MAX_NUM_OF_BITS_FOR_LAST_BYTE) \
            )
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
        }
    }

    /* Build the command frame */
    wBufferLength = 0U;

    phhalHw_Pn5190_Int_GetRxBuffer(pDataParams,
        &pTmpRxBuffer,
        &wTmpRxBufferLen,
        &wTmpRxBufferSize);

    /* Write Tx Data */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_EPC_GEN2_INVENTORY;

    sCmdParams.bQueue = false;

    aCmdBuffer[wBufferLength++] = 0x00; /* ResumeInventory */
    aCmdBuffer[wBufferLength++] = bSelectCommandLength;
    /*
    No Select command is set prior to BeginRound command.
    'Valid Bits in last Byte' field and 'Select command' field shall not be present.
    */
    if(0U != (bSelectCommandLength))
    {
        aCmdBuffer[wBufferLength++] = bSelectCommandBitsInLastByte;

        /* Copy the Instruction payload and update the buffer length*/
        (void)memcpy(&aCmdBuffer[wBufferLength], pSelectCommand, bSelectCommandLength);
        wBufferLength+= bSelectCommandLength;
    }
    /* Copy the Instruction payload and update the buffer length*/
    (void)memcpy(&aCmdBuffer[wBufferLength], pBeginRoundCommand, PHHAL_HW_PN5190_BEGIN_COMMAND_LENGTH);
    wBufferLength+= PHHAL_HW_PN5190_BEGIN_COMMAND_LENGTH;

    aCmdBuffer[wBufferLength++] = bTimeslotProcessingBehavior;

    sCmdParams.pTxDataBuff = &aCmdBuffer[0];
    sCmdParams.wTxDataLength = wBufferLength;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pTmpRxBuffer;


    /* Send it to the chip */
    statusTmp =  phhalHw_Pn5190_InstMngr_HandleCmd(
            pDataParams,
            &sCmdParams,
            PH_EXCHANGE_DEFAULT);

    if (statusTmp == PH_ERR_PN5190_SUCCESS)
    {
        /* copy the RX buffer and length and send it to the upper layer */
        *ppRxBuffer = *sCmdParams.ppRxBuffer;
        *pRxDataLength = *sCmdParams.pRxLength;
    }

    return PH_ADD_COMPCODE(statusTmp, PH_COMP_HAL);
}


phStatus_t phhalHw_Pn5190_Instr_EpcGen2ResumeInventory(
                                                       phhalHw_Pn5190_DataParams_t * pDataParams,
                                                       uint8_t ** ppRxBuffer,
                                                       uint16_t * pRxDataLength
                                                       )
{

    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wBufferLength = 0;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;

    uint8_t * pTmpRxBuffer = 0U;
    uint16_t wTmpRxBufferLen = 0U;
    uint16_t wTmpRxBufferSize = 0U;

    uint8_t    PH_MEMLOC_BUF aCmdBuffer[2];

    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};

    if ((NULL == pDataParams) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))

    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Build the command frame */
    wBufferLength = 0U;

    phhalHw_Pn5190_Int_GetRxBuffer(pDataParams,
        &pTmpRxBuffer,
        &wTmpRxBufferLen,
        &wTmpRxBufferSize);

    /* Write Tx Data */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_EPC_GEN2_INVENTORY;

    sCmdParams.bQueue = false;

    aCmdBuffer[wBufferLength++] = 0x01; /* ResumeInventory */

    sCmdParams.pTxDataBuff = &aCmdBuffer[0];
    sCmdParams.wTxDataLength = wBufferLength;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pTmpRxBuffer;

    /* Send it to the chip */
    statusTmp =  phhalHw_Pn5190_InstMngr_HandleCmd(
            pDataParams,
            &sCmdParams,
            PH_EXCHANGE_DEFAULT);

    if (statusTmp == PH_ERR_PN5190_SUCCESS)
    {
        /* copy the RX buffer and length and send it to the upper layer */
        *ppRxBuffer = *sCmdParams.ppRxBuffer;
        *pRxDataLength = *sCmdParams.pRxLength;
    }

    return PH_ADD_COMPCODE(statusTmp, PH_COMP_HAL);
}


phStatus_t phhalHw_Pn5190_Instr_LoadRfConfiguration(
                                                    phhalHw_Pn5190_DataParams_t * pDataParams,
                                                    uint8_t bRfTxConfiguration,
                                                    uint8_t bRfRxConfiguration
                                                    )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    uint8_t   PH_MEMLOC_REM aInstrBuffer[2U] = {0};
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if (((pDataParams == NULL)) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Check TX and RX configuration parameters. */
    if (((bRfTxConfiguration > PHHAL_HW_PN5190_MAX_RF_TX_CONFIGURATION_INDEX) && (bRfTxConfiguration != 0xFFU)) ||
        ((bRfRxConfiguration > 0x9DU) && (bRfRxConfiguration != 0xFFU)) ||
        (bRfRxConfiguration < 0x80U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* If "no change" needed for TX and RX configuration, return success. */
    if ((bRfTxConfiguration == 0xFFU) && (bRfRxConfiguration == 0xFFU))
    {
        return PH_ERR_PN5190_SUCCESS;
    }

    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_LOAD_RF_CONFIGURATION;

    sCmdParams.bQueue = true;

    /* Value */
    aInstrBuffer[bIndex++] = bRfTxConfiguration;
    aInstrBuffer[bIndex++] = bRfRxConfiguration;

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return  PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_UpdateRfConfiguration(
                                                      phhalHw_Pn5190_DataParams_t * pDataParams,
                                                      uint8_t * pRfConfiguration,
                                                      uint8_t bRfConfigurationSize
                                                      )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameters.
    * bRfConfigurationSize is not a multiple of PHHAL_HW_PN5190_RF_CONFIGURATION_SIZE
    * bRfConfigurationSize is greater than PHHAL_HW_PN5190_MAX_RF_CONFIGURATION_SIZE
    * */
    if ((NULL == pRfConfiguration) ||
        (bRfConfigurationSize > PHHAL_HW_PN5190_MAX_RF_CONFIGURATION_SIZE) ||
        (0U != (bRfConfigurationSize % PHHAL_HW_PN5190_RF_CONFIGURATION_SIZE)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Build the command frame */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_UPDATE_RF_CONFIGURATION;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = pRfConfiguration;
    sCmdParams.wTxDataLength = bRfConfigurationSize;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return  PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_RetrieveRfConfiguration(
                                                        phhalHw_Pn5190_DataParams_t * pDataParams,
                                                        uint8_t bRfConfiguration,
                                                        uint8_t * pRfConfBuf,
                                                        uint16_t * pRfConfBufSize
                                                        )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t  PH_MEMLOC_REM sCmdParams = {0U};

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameters.
    * bRfRxConfiguration is greater than the maximum RF RX configuration
    * bRfRxConfiguration is not the PHHAL_HW_PN5190_CURRENT_RF_CONFIGURATION_INDEX
    * */
    if (((bRfConfiguration < PHHAL_HW_PN5190_MIN_RF_RX_CONFIGURATION_INDEX)
        && (bRfConfiguration > PHHAL_HW_PN5190_MAX_RF_TX_CONFIGURATION_INDEX)) ||
        (bRfConfiguration > PHHAL_HW_PN5190_MAX_RF_RX_CONFIGURATION_INDEX) ||
        (*pRfConfBufSize < PHHAL_HW_PN5190_MIN_RF_CONFIGURATION_BUFFER_SIZE))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Build the command frame */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_GET_RF_CONFIGURATION;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = &bRfConfiguration;
    sCmdParams.wTxDataLength = 1;
    sCmdParams.pRxLength = pRfConfBufSize;
    sCmdParams.ppRxBuffer = &pRfConfBuf;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_RfOn(
                                     phhalHw_Pn5190_DataParams_t * pDataParams,
                                     uint8_t bRfOnConfig
                                     )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    uint8_t   PH_MEMLOC_REM aInstrBuffer[2U] = {0};
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate RF ON parameter */
    if (bRfOnConfig > 3U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_FIELD_ON;

    sCmdParams.bQueue = false;

    /* Value */
    aInstrBuffer[bIndex++] = bRfOnConfig;

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_RfOff(
                                      phhalHw_Pn5190_DataParams_t * pDataParams
                                      )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_FIELD_OFF;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = NULL;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_ConfigureTestBusDigital(
                                                        phhalHw_Pn5190_DataParams_t * pDataParams,
                                                        uint8_t bSignalIndex,
                                                        uint8_t bBitIndex,
                                                        uint8_t bPadIndex
                                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t   PH_MEMLOC_REM aInstrBuffer[3U] = {0};
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_CONFIGURE_TESTBUS_DIGITAL;

    sCmdParams.bQueue = false;

    /* Value */
    aInstrBuffer[bIndex++] = bSignalIndex;
    aInstrBuffer[bIndex++] = bBitIndex;
    aInstrBuffer[bIndex++] = bPadIndex;

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return  PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_ConfigureMulTestBusDigital(
                                                        phhalHw_Pn5190_DataParams_t * pDataParams,
                                                        uint8_t bTestBusReset,
                                                        uint8_t bSignalIndex1,
                                                        uint8_t bBitIndex1,
                                                        uint8_t bPadIndex1,
                                                        uint8_t bSignalIndex2,
                                                        uint8_t bBitIndex2,
                                                        uint8_t bPadIndex2
                                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM aInstrBuffer[6U] = {0};
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint8_t    PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_CONFIGURE_MUL_TESTBUS_DIGITAL;

    sCmdParams.bQueue = false;

    if (bTestBusReset == PH_OFF)
    {
        /* Value */
        aInstrBuffer[bIndex++] = bSignalIndex1;
        aInstrBuffer[bIndex++] = bBitIndex1;
        aInstrBuffer[bIndex++] = bPadIndex1;
        aInstrBuffer[bIndex++] = bSignalIndex2;
        aInstrBuffer[bIndex++] = bBitIndex2;
        aInstrBuffer[bIndex++] = bPadIndex2;
    }
    else
    {
        /* Payload not exchanged, FW performs Digital Test bus RESET internally. */
    }

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return  PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_ConfigureTestBusAnalog(
                                                       phhalHw_Pn5190_DataParams_t * pDataParams,
                                                       uint8_t bConfig,
                                                       uint8_t bCombinedMode,
                                                       uint8_t bTBIndex0,
                                                       uint8_t bTBIndex1,
                                                       uint8_t bShiftIndex0,
                                                       uint8_t bShiftIndex1,
                                                       uint8_t bMask0,
                                                       uint8_t bMask1
                                                       )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t   PH_MEMLOC_REM aInstrBuffer[8U] = {0};
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_CONFIGURE_TESTBUS_ANALOG;

    sCmdParams.bQueue = false;

    /* Value */
    aInstrBuffer[bIndex++] = bConfig;
    aInstrBuffer[bIndex++] = bCombinedMode;
    aInstrBuffer[bIndex++] = bTBIndex0;
    aInstrBuffer[bIndex++] = bTBIndex1;
    aInstrBuffer[bIndex++] = bShiftIndex0;
    aInstrBuffer[bIndex++] = bShiftIndex1;
    aInstrBuffer[bIndex++] = bMask0;
    aInstrBuffer[bIndex++] = bMask1;

    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return  PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_CtsEnable(
                                          phhalHw_Pn5190_DataParams_t * pDataParams,
                                          uint8_t bOption
                                          )
{
    phStatus_t PH_MEMLOC_REM status;
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if (((pDataParams == NULL)) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate Option parameter */
    if ((bOption != PH_OFF) && (bOption != PH_ON))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_CTS_ENABLE;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = &bOption;
    sCmdParams.wTxDataLength = 0x01U;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if(status == PH_ERR_PN5190_SUCCESS)
    {
        pDataParams->dwExpectedEvent = PH_PN5190_EVT_CTS;
    }

    return  PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_CtsConfig(
                                          phhalHw_Pn5190_DataParams_t * pDataParams,
                                          phhalHw_Pn5190_Instr_CtsConfig_t * pCtsConfig
                                          )
{
    phStatus_t PH_MEMLOC_REM status;
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if (((pDataParams == NULL)) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameter */
    if (pCtsConfig == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_CTS_CONFIGURE;

    sCmdParams.bQueue = false;

    /* Value */
    sCmdParams.pTxDataBuff = (uint8_t *) pCtsConfig;
    sCmdParams.wTxDataLength = sizeof(phhalHw_Pn5190_Instr_CtsConfig_t);
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */
    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return  PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_CtsRetrieveLog(
                                               phhalHw_Pn5190_DataParams_t * pDataParams,
                                               uint8_t bChunkSize,
                                               uint8_t * pLogData,
                                               uint8_t * bLogDataSize
                                               )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t PH_MEMLOC_REM wRxLen = 0U;

    /* Check data parameters. */
    if (((pDataParams == NULL)) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameter */
    if ((pLogData == NULL) || (bLogDataSize == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    if (pDataParams->bCTSEvent == PH_OFF)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_NO_CTS_EVENT, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_CTS_RETRIEVE_LOG;

    sCmdParams.bQueue = false;

    /* Value */
    sCmdParams.pTxDataBuff = &bChunkSize;
    sCmdParams.wTxDataLength = 0x01U;
    sCmdParams.ppRxBuffer = &pLogData;
    sCmdParams.pRxLength = &wRxLen;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if ((status == PH_ERR_PN5190_SUCCESS) || (status == PH_ERR_PN5190_SUCCESS_CHAINING))
    {
        *bLogDataSize = wRxLen;
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_GetDieId(phhalHw_Pn5190_DataParams_t *pDataParams,
                                         uint8_t *pDieIdValue)
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameter */
    if (pDieIdValue == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_GET_DIE_ID;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = NULL;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pDieIdValue;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if(wRxLen != 16)
    {
        status = PH_ERR_LENGTH_ERROR;
    }
    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_GetVersion(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_Pn5190_Version_t *pVersion)
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t aVersionInfo[4] = {0};
    uint8_t * pVersionInfo;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;

    pVersionInfo = &aVersionInfo[0];
    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* Validate input parameter */
    if (pVersion == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_PARAMETER, PH_COMP_HAL);
    }

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_GET_VERSION;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = NULL;
    sCmdParams.ppRxBuffer = &pVersionInfo;
    sCmdParams.pRxLength = &wRxLen;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    if(status == PH_ERR_PN5190_SUCCESS)
    {
        pVersion->bHw_Version = aVersionInfo[0];
        pVersion->bROM_Version = aVersionInfo[1];
        pVersion->wFW_Version = (uint16_t)(aVersionInfo[2] | (aVersionInfo[3] << 8));
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}


phStatus_t phhalHw_Pn5190_Instr_SwitchModeDownload(phhalHw_Pn5190_DataParams_t *pDataParams)
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_SWITCH_MODE_DOWNLOAD;
    sCmdParams.bQueue = false;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Instr_PrbsTest(phhalHw_Pn5190_DataParams_t *pDataParams, uint8_t bPrbs_type)
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    uint8_t   PH_MEMLOC_REM aInstrBuffer[1] = {0};
    phhalHw_InstMngr_CmdParams_t PH_MEMLOC_REM sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;


    /* Check data parameters. */
    if ((pDataParams == NULL) ||
        (PH_GET_COMPCODE(pDataParams) != PH_COMP_HAL) ||
        (PH_GET_COMPID(pDataParams) != PHHAL_HW_PN5190_ID))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PN5190_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    aInstrBuffer[bIndex++] = bPrbs_type;

    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_PRBS_TEST;
    sCmdParams.bQueue = false;
    sCmdParams.pTxDataBuff = &aInstrBuffer[0];
    sCmdParams.wTxDataLength = bIndex;

    status = phhalHw_Pn5190_InstMngr_HandleCmd(
       pDataParams,
       &sCmdParams,
       PH_EXCHANGE_DEFAULT);

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

#endif /* NXPBUILD__PHHAL_HW_PN5190 */
