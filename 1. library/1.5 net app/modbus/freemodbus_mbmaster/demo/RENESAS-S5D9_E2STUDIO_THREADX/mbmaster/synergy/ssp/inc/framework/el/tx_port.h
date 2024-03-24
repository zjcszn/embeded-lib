/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2015 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This software is copyrighted by and is the sole property of Express   */ 
/*  Logic, Inc.  All rights, title, ownership, or other interests         */ 
/*  in the software remain the property of Express Logic, Inc.  This      */ 
/*  software may only be used in accordance with the corresponding        */ 
/*  license agreement.  Any unauthorized use, duplication, transmission,  */ 
/*  distribution, or disclosure of this software is expressly forbidden.  */ 
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */ 
/*  written consent of Express Logic, Inc.                                */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** ThreadX Component                                                     */
/**                                                                       */
/**   Port Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** This file selects ThreadX port specific information based on the MCU  */
/** and compiler selected.                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef TX_PORT_H_TOP_LEVEL
#define TX_PORT_H_TOP_LEVEL

#define TX_INCLUDE_USER_DEFINE_FILE
#ifdef TX_SRC_USER_H_
#error "Include order error detected: tx_port.h must be included before tx_src_user.h because tx_src_user.h undefines macros defined in tx_port.h"
#endif
#if   defined(__IAR_SYSTEMS_ICC__)
    #if   (__CORE__ == __ARM7EM__)
        #include "./cm4_iar/tx_port.h"
    #elif (__CORE__ == __ARM6M__)
        #include "./cm0plus_iar/tx_port.h"
    #elif (__CORE__ == __ARM8M_BASELINE__)
        #include "./cm23_iar/tx_port.h"
    #else
        #warning Unsupported Architecture
    #endif
#elif defined(__GNUC__)
    #if   __ARM_ARCH_7EM__
        #include "./cm4_gcc/tx_port.h"
    #elif __ARM_ARCH_6M__
        #include "./cm0plus_gcc/tx_port.h"    
    #elif __ARM_ARCH_8M_BASE__
        #include "./cm23_gcc/tx_port.h"
    #else
        #warning Unsupported Architecture
    #endif
#else
    #warning Unsupported compiler
#endif

#if   defined(__IAR_SYSTEMS_ICC__)
#define TX_FUNCTION_NAKED __stackless
#elif defined(__GNUC__)
#define TX_FUNCTION_NAKED __attribute__((naked))
#else
    #warning Unsupported compiler
#endif

#endif /* TX_PORT_H_TOP_LEVEL */
