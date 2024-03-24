/*
 * MODBUS Master Library: A portable MODBUS master for MODBUS ASCII/RTU/TCP.
 * Copyright (c) 2008 Christian Walter <cwalter@embedded-solutions.at>
 * Copyright (c) 2020 Embedded Experts GmbH <modbus@embedded-experts.at>
 *
 * All rights reserved. Use of this file is subject to license terms.
 */

#ifndef _MBM_UDP_H
#define _MBM_UDP_H

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

#if defined( DOXYGEN ) || ( MBM_UDP_ENABLED == 1 )
/*!
 * \if INTERNAL_DOCS
 * \addtogroup mbm_udp_int
 * @{
 * \endif
 */

/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Function prototypes ------------------------------*/
eMBErrorCode
eMBMUDPFrameInit( xMBMInternalHandle *pxIntHdl, const CHAR *pcUDPBindAddress, LONG uUDPListenPort );

eMBErrorCode
eMBMUDPFrameSetClient( xMBMInternalHandle *pxIntHdl, const CHAR *pcUDPClientAddress, USHORT usUDPPort );

/*!
 * \if INTERNAL_DOCS
 * @}
 * \endif
 */
#endif

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
