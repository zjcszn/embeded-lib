/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2018 by Express Logic Inc.               */ 
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
/*                                                                        */ 
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */ 
/*                                                                        */ 
/*    tx_port.h                                          Synergy/GNU      */ 
/*                                                           5.2          */ 
/*                                                                        */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file contains data type definitions that make the ThreadX      */ 
/*    real-time kernel function identically on a variety of different     */ 
/*    processor architectures.  For example, the size or number of bits   */ 
/*    in an "int" data type vary between microprocessor architectures and */ 
/*    even C compilers for the same microprocessor.  ThreadX does not     */ 
/*    directly use native C data types.  Instead, ThreadX creates its     */ 
/*    own special types that can be mapped to actual data types by this   */ 
/*    file to guarantee consistency in the interface and functionality.   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  09-25-2015     William E. Lamie         Initial Renesas Synergy/GNU   */ 
/*                                            Support Version 5.0         */ 
/*  03-07-2016     William E. Lamie         Modified comment(s), and      */ 
/*                                            changed CHAR to "char",     */ 
/*                                            resulting in version 5.1    */ 
/*  10-27-2017     William E. Lamie         Modified comment(s), and      */
/*                                            disabled preemption in      */
/*                                            post initialization,        */
/*                                            resulting in version 5.2    */ 
/*                                                                        */ 
/**************************************************************************/ 

#ifndef TX_PORT_H
#define TX_PORT_H


/* Define default parameters for the Cortex-M4 build for smaller footprint.  */

#define TX_TIMER_PROCESS_IN_ISR
#define TX_DISABLE_PREEMPTION_THRESHOLD
#define TX_DISABLE_NOTIFY_CALLBACKS

/* Determine if the optional ThreadX user define file should be used.  */

#ifdef TX_INCLUDE_USER_DEFINE_FILE

/* Yes, include the user defines in tx_user.h. The defines in this file may 
   alternately be defined on the command line.  */

#include "tx_user.h"
#endif

/* Define default timer ticks as a simple expression suitable 
   for preprocessor expressions */
#ifndef TX_TIMER_TICKS_PER_SECOND
#define TX_TIMER_TICKS_PER_SECOND (100UL)
#endif

/* Define compiler library include files.  */

#include <stdlib.h>
#include <string.h>

/* CMSIS-CORE access. */

#include "tx_cmsis.h"

/* Define ThreadX basic types for this port.  */ 

#define VOID                                    void
typedef char                                    CHAR;
typedef unsigned char                           UCHAR;
typedef signed int                              INT;
typedef unsigned int                            UINT;
typedef signed long                             LONG;
typedef unsigned long                           ULONG;
typedef signed short                            SHORT;
typedef unsigned short                          USHORT;


/* Define the priority levels for ThreadX.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32.  */

#ifndef TX_MAX_PRIORITIES
#define TX_MAX_PRIORITIES                       32
#endif


/* Define the minimum stack for a ThreadX thread on this processor. If the size supplied during
   thread creation is less than this value, the thread create call will return an error.  */

#ifndef TX_MINIMUM_STACK
#define TX_MINIMUM_STACK                        200         /* Minimum stack size for this port  */
#endif


/* Define the system timer thread's default stack size and priority.  These are only applicable
   if TX_TIMER_PROCESS_IN_ISR is not defined.  */

#ifndef TX_TIMER_THREAD_STACK_SIZE
#define TX_TIMER_THREAD_STACK_SIZE              1024        /* Default timer thread stack size  */
#endif

#ifndef TX_TIMER_THREAD_PRIORITY    
#define TX_TIMER_THREAD_PRIORITY                0           /* Default timer thread priority    */ 
#endif


/* Define various constants for the ThreadX Synergy GNU port.  */ 

#define TX_INT_DISABLE                          1           /* Disable interrupts               */
#define TX_INT_ENABLE                           0           /* Enable interrupts                */


/* Define the clock source for trace event entry time stamp.  */

#ifndef TX_MISRA_ENABLE
#ifndef TX_TRACE_TIME_SOURCE
#define TX_TRACE_TIME_SOURCE                    *((ULONG *) 0xE0001004)  
#endif
#else
ULONG   _tx_misra_time_stamp_get(VOID);
#define TX_TRACE_TIME_SOURCE                    _tx_misra_time_stamp_get()
#endif

#ifndef TX_TRACE_TIME_MASK
#define TX_TRACE_TIME_MASK                      0xFFFFFFFFUL
#endif


/* Define the port specific options for the _tx_build_options variable. This variable indicates
   how the ThreadX library was built.  */

#define TX_PORT_SPECIFIC_BUILD_OPTIONS          0


/* Define the in-line initialization constant so that modules with in-line
   initialization capabilities can prevent their initialization from being
   a function call.  */

#ifdef TX_MISRA_ENABLE
#define TX_DISABLE_INLINE
#else
#define TX_INLINE_INITIALIZATION
#endif


/* Determine whether or not stack checking is enabled. By default, ThreadX stack checking is 
   disabled. When the following is defined, ThreadX thread stack checking is enabled.  If stack
   checking is enabled (TX_ENABLE_STACK_CHECKING is defined), the TX_DISABLE_STACK_FILLING
   define is negated, thereby forcing the stack fill which is necessary for the stack checking
   logic.  */

#ifdef TX_ENABLE_STACK_CHECKING
#undef TX_DISABLE_STACK_FILLING
#endif


/* Define the TX_THREAD control block extensions for this port. The main reason
   for the multiple macros is so that backward compatibility can be maintained with 
   existing ThreadX kernel awareness modules.  */

#ifndef TX_THREAD_EXTENSION_0
#define TX_THREAD_EXTENSION_0          
#endif
#ifndef TX_THREAD_EXTENSION_1
#define TX_THREAD_EXTENSION_1                  
#endif
#ifndef TX_THREAD_EXTENSION_2
#define TX_THREAD_EXTENSION_2                   ULONG       tx_thread_vfp_stack_frame;
#endif
#ifdef TX_ENABLE_EPK_SUPPORT
#include "tx_execution_profile.h"
#endif
#ifndef TX_THREAD_EXTENSION_3
#define TX_THREAD_EXTENSION_3                   int   bsd_errno;         
#endif


/* Define the port extensions of the remaining ThreadX objects.  */

#ifndef TX_BLOCK_POOL_EXTENSION
#define TX_BLOCK_POOL_EXTENSION
#endif
#ifndef TX_BYTE_POOL_EXTENSION
#define TX_BYTE_POOL_EXTENSION
#endif
#ifndef TX_EVENT_FLAGS_GROUP_EXTENSION
#define TX_EVENT_FLAGS_GROUP_EXTENSION
#endif
#ifndef TX_MUTEX_EXTENSION
#define TX_MUTEX_EXTENSION
#endif
#ifndef TX_QUEUE_EXTENSION
#define TX_QUEUE_EXTENSION
#endif
#ifndef TX_SEMAPHORE_EXTENSION
#define TX_SEMAPHORE_EXTENSION
#endif
#ifndef TX_TIMER_EXTENSION
#define TX_TIMER_EXTENSION
#endif


/* Define the user extension field of the thread control block.  Nothing 
   additional is needed for this port so it is defined as white space.  */

#ifndef TX_THREAD_USER_EXTENSION
#define TX_THREAD_USER_EXTENSION    
#endif


/* Define the macros for processing extensions in tx_thread_create, tx_thread_delete,
   tx_thread_shell_entry, and tx_thread_terminate.  */


#ifndef TX_THREAD_CREATE_EXTENSION
#define TX_THREAD_CREATE_EXTENSION(thread_ptr)                                  
#endif
#ifndef TX_THREAD_DELETE_EXTENSION
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                                  
#endif
#ifndef TX_THREAD_COMPLETED_EXTENSION
#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)
#endif
#ifndef TX_THREAD_TERMINATED_EXTENSION
#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)
#endif


/* Define the ThreadX object creation extensions for the remaining objects.  */

#ifndef TX_BLOCK_POOL_CREATE_EXTENSION
#define TX_BLOCK_POOL_CREATE_EXTENSION(pool_ptr)
#endif
#ifndef TX_BYTE_POOL_CREATE_EXTENSION
#define TX_BYTE_POOL_CREATE_EXTENSION(pool_ptr)
#endif
#ifndef TX_EVENT_FLAGS_GROUP_CREATE_EXTENSION
#define TX_EVENT_FLAGS_GROUP_CREATE_EXTENSION(group_ptr)
#endif
#ifndef TX_MUTEX_CREATE_EXTENSION
#define TX_MUTEX_CREATE_EXTENSION(mutex_ptr)
#endif
#ifndef TX_QUEUE_CREATE_EXTENSION
#define TX_QUEUE_CREATE_EXTENSION(queue_ptr)
#endif
#ifndef TX_SEMAPHORE_CREATE_EXTENSION
#define TX_SEMAPHORE_CREATE_EXTENSION(semaphore_ptr)
#endif
#ifndef TX_TIMER_CREATE_EXTENSION
#define TX_TIMER_CREATE_EXTENSION(timer_ptr)
#endif


/* Define the ThreadX object deletion extensions for the remaining objects.  */

#ifndef TX_BLOCK_POOL_DELETE_EXTENSION
#define TX_BLOCK_POOL_DELETE_EXTENSION(pool_ptr)
#endif
#ifndef TX_BYTE_POOL_DELETE_EXTENSION
#define TX_BYTE_POOL_DELETE_EXTENSION(pool_ptr)
#endif
#ifndef TX_EVENT_FLAGS_GROUP_DELETE_EXTENSION
#define TX_EVENT_FLAGS_GROUP_DELETE_EXTENSION(group_ptr)
#endif
#ifndef TX_MUTEX_DELETE_EXTENSION
#define TX_MUTEX_DELETE_EXTENSION(mutex_ptr)
#endif
#ifndef TX_QUEUE_DELETE_EXTENSION
#define TX_QUEUE_DELETE_EXTENSION(queue_ptr)
#endif
#ifndef TX_SEMAPHORE_DELETE_EXTENSION
#define TX_SEMAPHORE_DELETE_EXTENSION(semaphore_ptr)
#endif
#ifndef TX_TIMER_DELETE_EXTENSION
#define TX_TIMER_DELETE_EXTENSION(timer_ptr)
#endif


/* Define the get system state macro.   */
   
#ifndef TX_THREAD_GET_SYSTEM_STATE
#ifndef TX_MISRA_ENABLE

__attribute__( ( always_inline ) ) static inline unsigned int __get_ipsr_value(void)
{

unsigned int  ipsr_value;

    __asm__ volatile (" MRS  %0,IPSR ": "=r" (ipsr_value) );
    return(ipsr_value);
}


#define TX_THREAD_GET_SYSTEM_STATE()        (_tx_thread_system_state | __get_ipsr_value())
#else
ULONG   _tx_misra_ipsr_get(VOID);
#define TX_THREAD_GET_SYSTEM_STATE()        (_tx_thread_system_state | _tx_misra_ipsr_get())
#endif
#endif


/* Define the check for whether or not to call the _tx_thread_system_return function.  A non-zero value
   indicates that _tx_thread_system_return should not be called.  */

#ifndef TX_THREAD_SYSTEM_RETURN_CHECK
#define TX_THREAD_SYSTEM_RETURN_CHECK(c)    (c) = ((ULONG) _tx_thread_preempt_disable);
#endif

/* Define the macro to ensure _tx_thread_preempt_disable is set early in initialization in order to 
   prevent early scheduling on Cortex-M parts.  */
   
#define TX_PORT_SPECIFIC_POST_INITIALIZATION    _tx_thread_preempt_disable++;

/* This ARM architecture has the CLZ instruction. This is available on 
   architectures v5 and above. If available, redefine the macro for calculating the 
   lowest bit set.  */
   
#ifndef TX_DISABLE_INLINE
#define TX_LOWEST_SET_BIT_CALCULATE(m, b)       m = m & ((ULONG) (-((LONG) m))); \
                                                b = (UINT) __CLZ(m); \
                                                b = 31 - b;
#endif


/* Define ThreadX interrupt lockout and restore macros for protection on 
   access of critical kernel information.  The restore interrupt macro must 
   restore the interrupt posture of the running thread prior to the value 
   present prior to the disable macro.  In most cases, the save area macro
   is used to define a local function save area for the disable and restore
   macros.  */

#ifndef TX_DISABLE_INLINE

/* Define GNU specific macros, with in-line assembly for performance.  */

__attribute__( ( always_inline ) ) static inline unsigned int __disable_interrupts(void)
{

unsigned int  primask_value;

    __asm__ volatile (" MRS  %0,PRIMASK ": "=r" (primask_value) );
    __asm__ volatile (" CPSID i" : : : "memory" );
    return(primask_value);
}

__attribute__( ( always_inline ) ) static inline void __restore_interrupts(unsigned int primask_value)
{

    __asm__ volatile (" MSR  PRIMASK,%0": : "r" (primask_value): "memory" );
}

__attribute__( ( always_inline ) ) static inline unsigned int __get_primask_value(void)
{

unsigned int  primask_value;

    __asm__ volatile (" MRS  %0,PRIMASK ": "=r" (primask_value) );
    return(primask_value);
}

__attribute__( ( always_inline ) ) static inline void __enable_interrupts(void)
{

    __asm__ volatile (" CPSIE  i": : : "memory" );
}

__attribute__( ( always_inline ) ) static inline void _tx_thread_system_return_inline(void)
{
unsigned int interrupt_save;

    /* Set PENDSVBIT in ICSR */
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

    /* Is it a thread returning */
    if (__get_ipsr_value() == 0U)
    {
        interrupt_save = __get_primask_value();
        __enable_interrupts();
        __restore_interrupts(interrupt_save);
    }   
}


#define TX_INTERRUPT_SAVE_AREA  UINT interrupt_save;

#define TX_DISABLE                              interrupt_save =  __disable_interrupts();
#define TX_RESTORE                              __restore_interrupts(interrupt_save);


/* Redefine _tx_thread_system_return for improved performance.  */

#define _tx_thread_system_return                _tx_thread_system_return_inline


#else

UINT   _tx_thread_interrupt_disable(VOID);
VOID   _tx_thread_interrupt_restore(UINT new_posture);

#define TX_INTERRUPT_SAVE_AREA  UINT interrupt_save;

#define TX_DISABLE                              interrupt_save = _tx_thread_interrupt_disable();
#define TX_RESTORE                              _tx_thread_interrupt_restore(interrupt_save);
#endif


/* Define prototypes specific to the ThreadX Synergy implementation.  */

VOID  tx_isr_start(ULONG isr_id);
VOID  tx_isr_end(ULONG isr_id);

/* Define whether FPU is being used or not. */
#if ( defined ( __GNUC__ ) && defined (__VFP_FP__) && !defined (__SOFTFP__) ) || \
    ( defined ( __ICCARM__ ) && defined ( __ARMVFP__ ) && (__FPU_PRESENT == 1) )

/* Enable FPU register saving when necessary. */
    
#define TX_VFP_ENABLE
#else
/* FPU not being used. */
#endif

/* Define the location of the vector table */

extern void * __Vectors[];
#define TX_VECTOR_TABLE __Vectors


/* Define the location of the begining of the free RAM  */

extern void * __RAM_segment_used_end__;
#define TX_FREE_MEMORY_START (&__RAM_segment_used_end__)

/* Define the version ID of ThreadX.  This may be utilized by the application.  */

#ifdef TX_THREAD_INIT
CHAR                            _tx_version_id[] = 
                                    "Copyright (c) 1996-2018 Express Logic Inc. * ThreadX Synergy/GNU Version G5.9 SN: 4154-280-5000 *";
#else
extern  CHAR                    _tx_version_id[100];
#endif


#endif





