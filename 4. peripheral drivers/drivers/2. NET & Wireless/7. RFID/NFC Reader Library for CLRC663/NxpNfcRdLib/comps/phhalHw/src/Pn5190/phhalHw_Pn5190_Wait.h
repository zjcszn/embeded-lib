/*----------------------------------------------------------------------------*/
/* Copyright 2019-2020 NXP                                                    */
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
* Wait Routines definitions for Pn5190 specific HAL-Component of Reader Library Framework.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/


#ifndef PHHALHW_PN5190_WAIT_H_
#define PHHALHW_PN5190_WAIT_H_


/**
* \brief Waits for the interrupts to occur, this is a blocking call.
* The function does not return back until RF interrupt occurs.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_ABORTED Current wait is aborted.
* \retval #PH_OSAL_IO_TIMEOUT TimeOut Occurred.
*/

phStatus_t phhalHw_Pn5190_WaitIrq(
    phhalHw_Pn5190_DataParams_t * pDataParams,            /**<[In] DataParameter representing this layer. */
    uint32_t * pReceivedEvents                            /**<[InOut] Return received Events. */
    );

/**
* \brief Used by Hal for waiting for desired Events to Occur.
* Function return when desired event Occur Or upon timeOut.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_ABORTED Current wait is aborted.
* \retval #PH_ERR_INTERNAL_ERROR Unexpected event occurred.
* \retval #PH_OSAL_IO_TIMEOUT TimeOut Occurred.
*/
phStatus_t phhalHw_Pn5190_WaitForEvent(
    phhalHw_Pn5190_DataParams_t * pDataParams,            /**<[In] DataParameter representing this layer. */
    uint32_t dwEvtsWaitFor,                               /**<[In] Specific Evt we are waiting for. */
    uint32_t dwEvtsTimeOut,                               /**<[In] Time in Milli-Sec, for Events to occur. */
    uint8_t bPayloadPresnt,                               /**<[In] Specifies if payload is to be returned using Pointer . */
    uint32_t * dwEventsReceived,                          /**<[InOut] Returns Event received */
    uint8_t ** ppEvtPayLoad                               /**<[InOut] Contains the pointer to payload data of the Evt */
    );

#ifndef _WIN32
/**
* \brief Waits for Abort resp.
* Function return when Abort Response is received Or upon timeOut.
* \return Status code
* \retval #PH_ERR_SUCCESS abort response received successfully.
* \retval #PH_ERR_INTERNAL_ERROR Unexpected event occurred.
* \retval #PH_OSAL_IO_TIMEOUT TimeOut Occurred.
*/
phStatus_t  phhalHw_Pn5190_Wait_IrqAbortHandling(
    phhalHw_Pn5190_DataParams_t * pDataParams,            /**<[In] DataParameter representing this layer. */
    phhalHw_InstMngr_CmdParams_t * pCmdParams,            /**<[In] Parameters of command under process. */
    uint8_t bAbortCMDExe                                  /**<[In] SWITCH_MODE_NORMAL CMD will be sent then Parameters of command under process. */
    );
#endif

#endif /* PHHALHW_PN5190_WAIT_H_ */
