/*
 * MODBUS Master Library: A portable MODBUS master for MODBUS ASCII/RTU/TCP.
 * Copyright (c) 2008 Christian Walter <cwalter@embedded-solutions.at>
 * Copyright (c) 2020 Embedded Experts GmbH <modbus@embedded-experts.at>
 *
 * All rights reserved. Use of this file is subject to license terms.
 */

#ifndef _MBM_ASCII_H
#define _MBM_ASCII_H

#if MBM_ASCII_ENABLED == 1

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

/*!
 * \if INTERNAL_DOCS
 * \addtogroup mbm_ascii_int
 * @{
 * \endif
 */

/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Function prototypes ------------------------------*/

/*! \brief Configure a MODBUS master ASCII instances.
 * \internal
 *
 * \param pxIntHdl An internal handle.
 * \param ucPort The port. This value is passed through to the porting layer.
 * \param ulBaudRate Baudrate.
 * \param eParity Parity.
 * \param ucStopBits Number of stopbits to use.
 *
 * \return MB_ENOERR if a new instances has been created. In this case the
 *   memers pxFrameHdl, pFrameSendFN, pFrameRecvFN and pFrameCloseFN in the
 *   handle are updated to point to this RTU instances. If pxIntHdl equals
 *   \c NULL or the baudrate is zero it returns MB_EINVAL. Otherwise the
 *   error code depends on the porting layer.
 */
eMBErrorCode
eMBMSerialASCIIInit( xMBMInternalHandle *pxIntHdl, UCHAR ucPort, ULONG ulBaudRate, eMBSerialParity eParity,
                     UCHAR ucStopBits );

/*!
 * \if INTERNAL_DOCS
 * @}
 * \endif
 */

#ifdef __cplusplus
PR_END_EXTERN_C
#endif

#endif

#endif
