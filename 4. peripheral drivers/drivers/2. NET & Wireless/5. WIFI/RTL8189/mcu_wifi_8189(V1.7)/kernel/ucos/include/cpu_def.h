/*
*********************************************************************************************************
*                                               uC/CPU
*                                    CPU CONFIGURATION & PORT LAYER
*
*                          (c) Copyright 2004-2009; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/CPU is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/CPU in a commercial
*               product you need to contact Micrium to properly license its use in your
*               product.  We provide ALL the source code for your convenience and to
*               help you experience uC/CPU.  The fact that the source code is provided
*               does NOT mean that you can use it without paying a licensing fee.
*
*               Knowledge of the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      CPU CONFIGURATION DEFINES
*
* Filename      : cpu_def.h
* Version       : V1.23
* Programmer(s) : ITJ
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  CPU_DEF_MODULE_PRESENT
#define  CPU_DEF_MODULE_PRESENT


/*
*********************************************************************************************************
*                                       CPU WORD CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_ADDR_SIZE & CPU_CFG_DATA_SIZE in 'cpu.h' with CPU's word sizes :
*
*                   CPU_WORD_SIZE_08             8-bit word size
*                   CPU_WORD_SIZE_16            16-bit word size
*                   CPU_WORD_SIZE_32            32-bit word size
*                   CPU_WORD_SIZE_64            64-bit word size            See Note #1a
*
*               (a) 64-bit word size NOT currently supported.
*
*           (2) Configure CPU_CFG_ENDIAN_TYPE in 'cpu.h' with CPU's data-word-memory order :
*
*               (a) CPU_ENDIAN_TYPE_BIG         Big-   endian word order (CPU words' most  significant
*                                                                         octet @ lowest memory address)
*               (b) CPU_ENDIAN_TYPE_LITTLE      Little-endian word order (CPU words' least significant
*                                                                         octet @ lowest memory address)
*********************************************************************************************************
*/

                                                        /* ---------------------- CPU WORD SIZE ----------------------- */
#define  CPU_WORD_SIZE_08                          1    /*  8-bit word size (in octets).                                */
#define  CPU_WORD_SIZE_16                          2    /* 16-bit word size (in octets).                                */
#define  CPU_WORD_SIZE_32                          4    /* 32-bit word size (in octets).                                */
#define  CPU_WORD_SIZE_64                          8    /* 64-bit word size (in octets) [see Note #1a].                 */


                                                        /* ------------------ CPU WORD-ENDIAN ORDER ------------------- */
#define  CPU_ENDIAN_TYPE_NONE                      0
#define  CPU_ENDIAN_TYPE_BIG                       1    /* Big-   endian word order (see Note #1a).                     */
#define  CPU_ENDIAN_TYPE_LITTLE                    2    /* Little-endian word order (see Note #1b).                     */


/*
*********************************************************************************************************
*                                       CPU STACK CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_STK_GROWTH in 'cpu.h' with CPU's stack growth order :
*
*               (a) CPU_STK_GROWTH_LO_TO_HI     CPU stack pointer increments to the next higher  stack 
*                                                   memory address after data is pushed onto the stack
*               (b) CPU_STK_GROWTH_HI_TO_LO     CPU stack pointer decrements to the next lower   stack 
*                                                   memory address after data is pushed onto the stack
*********************************************************************************************************
*/

                                                        /* ------------------ CPU STACK GROWTH ORDER ------------------ */
#define  CPU_STK_GROWTH_NONE                       0
#define  CPU_STK_GROWTH_LO_TO_HI                   1    /* CPU stk incs towards higher mem addrs (see Note #1a).        */
#define  CPU_STK_GROWTH_HI_TO_LO                   2    /* CPU stk decs towards lower  mem addrs (see Note #1b).        */


/*$PAGE*/
/*
*********************************************************************************************************
*                                   CRITICAL SECTION CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_CRITICAL_METHOD with CPU's/compiler's critical section method :
*
*                                                       Enter/Exit critical sections by ...
*
*                   CPU_CRITICAL_METHOD_INT_DIS_EN      Disable/Enable interrupts
*                   CPU_CRITICAL_METHOD_STATUS_STK      Push/Pop       interrupt status onto stack
*                   CPU_CRITICAL_METHOD_STATUS_LOCAL    Save/Restore   interrupt status to local variable
*
*               (a) CPU_CRITICAL_METHOD_INT_DIS_EN  is NOT a preferred method since it does NOT support
*                   multiple levels of interrupts.  However, with some CPUs/compilers, this is the only
*                   available method.
*
*               (b) CPU_CRITICAL_METHOD_STATUS_STK    is one preferred method since it supports multiple
*                   levels of interrupts.  However, this method assumes that the compiler provides C-level
*                   &/or assembly-level functionality for the following :
*
*                     ENTER CRITICAL SECTION :
*                       (1) Push/save   interrupt status onto a local stack
*                       (2) Disable     interrupts
*
*                     EXIT  CRITICAL SECTION :
*                       (3) Pop/restore interrupt status from a local stack
*
*               (c) CPU_CRITICAL_METHOD_STATUS_LOCAL  is one preferred method since it supports multiple
*                   levels of interrupts.  However, this method assumes that the compiler provides C-level
*                   &/or assembly-level functionality for the following :
*
*                     ENTER CRITICAL SECTION :
*                       (1) Save    interrupt status into a local variable
*                       (2) Disable interrupts
*
*                     EXIT  CRITICAL SECTION :
*                       (3) Restore interrupt status from a local variable
*
*           (2) Critical section macro's most likely require inline assembly.  If the compiler does NOT
*               allow inline assembly in C source files, critical section macro's MUST call an assembly
*               subroutine defined in a 'cpu_a.asm' file located in the following software directory :
*
*                   \<CPU-Compiler Directory>\<cpu>\<compiler>\
*
*                       where
*                               <CPU-Compiler Directory>    directory path for common   CPU-compiler software
*                               <cpu>                       directory name for specific CPU
*                               <compiler>                  directory name for specific compiler
*
*           (3) (a) To save/restore interrupt status, a local variable 'cpu_sr' of type 'CPU_SR' MAY need 
*                   to be declared (e.g. if 'CPU_CRITICAL_METHOD_STATUS_LOCAL' method is configured).
*
*                   (1) 'cpu_sr' local variable SHOULD be declared via the CPU_SR_ALLOC() macro which, 
*                        if used, MUST be declared following ALL other local variables (see any 'cpu.h  
*                        CRITICAL SECTION CONFIGURATION  Note #3a1').
*
*               (b) Configure 'CPU_SR' data type with the appropriate-sized CPU data type large enough to 
*                   completely store the CPU's/compiler's status word.
*********************************************************************************************************
*/

                                                        /* --------------- CPU CRITICAL SECTION METHODS --------------- */
#define  CPU_CRITICAL_METHOD_NONE                  0    /*                                                              */
#define  CPU_CRITICAL_METHOD_INT_DIS_EN            1    /* DIS/EN       ints                    (see Note #1a).         */
#define  CPU_CRITICAL_METHOD_STATUS_STK            2    /* Push/Pop     int status onto stk     (see Note #1b).         */
#define  CPU_CRITICAL_METHOD_STATUS_LOCAL          3    /* Save/Restore int status to local var (see Note #1c).         */


/*$PAGE*/
/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                  /* End of CPU definition module include.                        */

