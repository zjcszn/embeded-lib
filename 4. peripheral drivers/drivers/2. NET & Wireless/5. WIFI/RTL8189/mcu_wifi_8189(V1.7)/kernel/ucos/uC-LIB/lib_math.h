/*
*********************************************************************************************************
*                                               uC/LIB
*                                       CUSTOM LIBRARY MODULES
*
*                          (c) Copyright 2004-2009; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/LIB is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/LIB in a commercial
*               product you need to contact Micrium to properly license its use in your
*               product.  We provide ALL the source code for your convenience and to
*               help you experience uC/LIB.  The fact that the source code is provided
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
*                                        MATHEMATIC OPERATIONS
*
* Filename      : lib_math.h
* Version       : V1.30
* Programmer(s) : SR
*                 ITJ
*********************************************************************************************************
* Note(s)       : (1) NO compiler-supplied standard library functions are used in library or product software.
*
*                     (a) ALL standard library functions are implemented in the custom library modules :
*
*                         (1) \<Custom Library Directory>\lib*.*
*
*                         (2) \<Custom Library Directory>\Ports\<cpu>\<compiler>\lib*_a.*
*
*                               where
*                                       <Custom Library Directory>      directory path for custom library software
*                                       <cpu>                           directory name for specific processor (CPU)
*                                       <compiler>                      directory name for specific compiler
*
*                     (b) Product-specific library functions are implemented in individual products.
*
*********************************************************************************************************
* Notice(s)     : (1) The Institute of Electrical and Electronics Engineers and The Open Group, have given
*                     us permission to reprint portions of their documentation.  Portions of this text are
*                     reprinted and reproduced in electronic form from the IEEE Std 1003.1, 2004 Edition,
*                     Standard for Information Technology -- Portable Operating System Interface (POSIX),
*                     The Open Group Base Specifications Issue 6, Copyright (C) 2001-2004 by the Institute
*                     of Electrical and Electronics Engineers, Inc and The Open Group.  In the event of any
*                     discrepancy between these versions and the original IEEE and The Open Group Standard,
*                     the original IEEE and The Open Group Standard is the referee document.  The original
*                     Standard can be obtained online at http://www.opengroup.org/unix/online.html.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  LIB_MATH_MODULE_PRESENT
#define  LIB_MATH_MODULE_PRESENT


/*$PAGE*/
/*
*********************************************************************************************************
*                                            INCLUDE FILES
*
* Note(s) : (1) The following common software files are located in the following directories :
*
*               (a) \<Custom Library Directory>\lib*.*
*
*               (b) (1) \<CPU-Compiler Directory>\cpu_def.h
*
*                   (2) \<CPU-Compiler Directory>\<cpu>\<compiler>\cpu*.*
*
*                           where
*                                   <Custom Library Directory>      directory path for custom   library      software
*                                   <CPU-Compiler Directory>        directory path for common   CPU-compiler software
*                                   <cpu>                           directory name for specific processor (CPU)
*                                   <compiler>                      directory name for specific compiler
*
*           (2) Compiler MUST be configured to include the '\<Custom Library Directory>\uC-LIB\',
*               '\<CPU-Compiler Directory>\' directory, & the specific CPU-compiler directory as
*               additional include path directories.
*
*           (3) NO compiler-supplied standard library functions SHOULD be used.
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <cpu_core.h>

#include  <lib_def.h>


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   LIB_MATH_MODULE
#define  LIB_MATH_EXT
#else
#define  LIB_MATH_EXT  extern
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        RANDOM NUMBER DEFINES
*
* Note(s) : (1) (a) IEEE Std 1003.1, 2004 Edition, Section 'rand() : DESCRIPTION' states that "if rand() 
*                   is called before any calls to srand() are made, the same sequence shall be generated 
*                   as when srand() is first called with a seed value of 1".
*
*               (b) (1) BSD/ANSI-C implements rand() as a Linear Congruential Generator (LCG) :
*
*                       (A) random_number       =  [(a * random_number ) + b]  modulo m
*                                        n + 1                        n
*
*                               where
*                                       (1) (a) random_number       Next     random number to generate
*                                                            n+1
*                                           (b) random_number       Previous random number    generated
*                                                            n
*                                           (c) random_number       Initial  random number seed
*                                                            0                      See also Note #1a
*
*                                       (2) a =   1103515245        LCG multiplier
*                                       (3) b =        12345        LCG incrementor
*                                       (4) m = RAND_MAX + 1        LCG modulus     See also Note #1b2
*
*                   (2) (A) IEEE Std 1003.1, 2004 Edition, Section 'rand() : DESCRIPTION' states that 
*                           "rand() ... shall compute a sequence of pseudo-random integers in the range 
*                           [0, {RAND_MAX}] with a period of at least 2^32".
*
*                       (B) However, BSD/ANSI-C 'stdlib.h' defines "RAND_MAX" as "0x7fffffff", or 2^31; 
*                           which therefore limits the range AND period to no more than 2^31.
*********************************************************************************************************
*/

#define  RAND_SEED_INIT_VAL                                1u   /* See Note #1a.                                        */

#define  RAND_LCG_PARAM_M                         0x7FFFFFFFu   /* See Note #1b2B.                                      */
#define  RAND_LCG_PARAM_A                         1103515245u   /* See Note #1b1A2.                                     */
#define  RAND_LCG_PARAM_B                              12345u   /* See Note #1b1A3.                                     */


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       RANDOM NUMBER DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT32U  RAND_NBR;


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void      Math_Init       (void);

                                                                /* ------------------ RAND NBR FNCTS ------------------ */
void      Math_RandSetSeed(RAND_NBR  seed);

RAND_NBR  Math_Rand       (void);

RAND_NBR  Math_RandSeed   (RAND_NBR  seed);


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of lib math module include.                      */

