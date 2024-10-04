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
*                                     CORE CUSTOM LIBRARY MODULE
*
* Filename      : lib_def.h
* Version       : V1.30
* Programmer(s) : ITJ
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
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  LIB_DEF_MODULE_PRESENT
#define  LIB_DEF_MODULE_PRESENT


/*$PAGE*/
/*
*********************************************************************************************************
*                                CUSTOM LIBRARY MODULE VERSION NUMBER
*
* Note(s) : (1) (a) The custom library module software version is denoted as follows :
*
*                       Vx.yy
*
*                           where
*                                   V               denotes 'Version' label
*                                   x               denotes major software version revision number
*                                   yy              denotes minor software version revision number
*
*               (b) The software version label #define is formatted as follows :
*
*                       ver = x.yy * 100
*
*                           where
*                                   ver             denotes software version number scaled as an integer value
*                                   x.yy            denotes software version number
*********************************************************************************************************
*/

#define  LIB_VERSION                                     130u   /* See Note #1.                                         */


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
*********************************************************************************************************
*/

#include  <cpu.h>


/*$PAGE*/
/*
*********************************************************************************************************
*                                          STANDARD DEFINES
*********************************************************************************************************
*/

                                                                /* ----------------- BOOLEAN DEFINES ------------------ */
#define  DEF_FALSE                                         0u
#define  DEF_TRUE                                          1u

#define  DEF_NO                                            0u
#define  DEF_YES                                           1u

#define  DEF_DISABLED                                      0u
#define  DEF_ENABLED                                       1u

#define  DEF_INACTIVE                                      0u
#define  DEF_ACTIVE                                        1u

#define  DEF_OFF                                           0u
#define  DEF_ON                                            1u

#define  DEF_CLR                                           0u
#define  DEF_SET                                           1u

#define  DEF_FAIL                                          0u
#define  DEF_OK                                            1u


                                                                /* ------------------- BIT DEFINES -------------------- */
#define  DEF_BIT_NONE                                   0x00u

#define  DEF_BIT_00                                     0x01u
#define  DEF_BIT_01                                     0x02u
#define  DEF_BIT_02                                     0x04u
#define  DEF_BIT_03                                     0x08u
#define  DEF_BIT_04                                     0x10u
#define  DEF_BIT_05                                     0x20u
#define  DEF_BIT_06                                     0x40u
#define  DEF_BIT_07                                     0x80u

#define  DEF_BIT_08                                   0x0100u
#define  DEF_BIT_09                                   0x0200u
#define  DEF_BIT_10                                   0x0400u
#define  DEF_BIT_11                                   0x0800u
#define  DEF_BIT_12                                   0x1000u
#define  DEF_BIT_13                                   0x2000u
#define  DEF_BIT_14                                   0x4000u
#define  DEF_BIT_15                                   0x8000u

#define  DEF_BIT_16                               0x00010000u
#define  DEF_BIT_17                               0x00020000u
#define  DEF_BIT_18                               0x00040000u
#define  DEF_BIT_19                               0x00080000u
#define  DEF_BIT_20                               0x00100000u
#define  DEF_BIT_21                               0x00200000u
#define  DEF_BIT_22                               0x00400000u
#define  DEF_BIT_23                               0x00800000u

#define  DEF_BIT_24                               0x01000000u
#define  DEF_BIT_25                               0x02000000u
#define  DEF_BIT_26                               0x04000000u
#define  DEF_BIT_27                               0x08000000u
#define  DEF_BIT_28                               0x10000000u
#define  DEF_BIT_29                               0x20000000u
#define  DEF_BIT_30                               0x40000000u
#define  DEF_BIT_31                               0x80000000u

                                                                /* ------------------ OCTET DEFINES ------------------- */
#define  DEF_OCTET_NBR_BITS                                8u
#define  DEF_OCTET_MASK                                 0xFFu

#define  DEF_NIBBLE_NBR_BITS                               4u
#define  DEF_NIBBLE_MASK                                0x0Fu


                                                                /* --------------- NUMBER BASE DEFINES ---------------- */
#define  DEF_NBR_BASE_BIN                                  2u
#define  DEF_NBR_BASE_OCT                                  8u
#define  DEF_NBR_BASE_DEC                                 10u
#define  DEF_NBR_BASE_HEX                                 16u


/*$PAGE*/
                                                                /* ----------------- INTEGER DEFINES ------------------ */
#define  DEF_INT_08_NBR_BITS                               8u
#define  DEF_INT_08_MASK                                0xFFu

#define  DEF_INT_08U_MIN_VAL                               0u
#define  DEF_INT_08U_MAX_VAL                             255u

#define  DEF_INT_08S_MIN_VAL_ONES_CPL                  (-127)
#define  DEF_INT_08S_MAX_VAL_ONES_CPL                    127

#define  DEF_INT_08S_MIN_VAL                            (DEF_INT_08S_MIN_VAL_ONES_CPL - 1)
#define  DEF_INT_08S_MAX_VAL                             DEF_INT_08S_MAX_VAL_ONES_CPL

#define  DEF_INT_08U_NBR_DIG_MIN                           1u
#define  DEF_INT_08U_NBR_DIG_MAX                           3u

#define  DEF_INT_08S_NBR_DIG_MIN                           3u
#define  DEF_INT_08S_NBR_DIG_MAX                           3u



#define  DEF_INT_16_NBR_BITS                              16u
#define  DEF_INT_16_MASK                              0xFFFFu

#define  DEF_INT_16U_MIN_VAL                               0u
#define  DEF_INT_16U_MAX_VAL                           65535u

#define  DEF_INT_16S_MIN_VAL_ONES_CPL                (-32767)
#define  DEF_INT_16S_MAX_VAL_ONES_CPL                  32767

#define  DEF_INT_16S_MIN_VAL                            (DEF_INT_16S_MIN_VAL_ONES_CPL - 1)
#define  DEF_INT_16S_MAX_VAL                             DEF_INT_16S_MAX_VAL_ONES_CPL

#define  DEF_INT_16U_NBR_DIG_MIN                           1u
#define  DEF_INT_16U_NBR_DIG_MAX                           5u

#define  DEF_INT_16S_NBR_DIG_MIN                           5u
#define  DEF_INT_16S_NBR_DIG_MAX                           5u



#define  DEF_INT_32_NBR_BITS                              32u
#define  DEF_INT_32_MASK                          0xFFFFFFFFu

#define  DEF_INT_32U_MIN_VAL                               0u
#define  DEF_INT_32U_MAX_VAL                      4294967295u

#define  DEF_INT_32S_MIN_VAL_ONES_CPL           (-2147483647)
#define  DEF_INT_32S_MAX_VAL_ONES_CPL             2147483647

#define  DEF_INT_32S_MIN_VAL                            (DEF_INT_32S_MIN_VAL_ONES_CPL - 1)
#define  DEF_INT_32S_MAX_VAL                             DEF_INT_32S_MAX_VAL_ONES_CPL

#define  DEF_INT_32U_NBR_DIG_MIN                           1u
#define  DEF_INT_32U_NBR_DIG_MAX                          10u

#define  DEF_INT_32S_NBR_DIG_MIN                          10u
#define  DEF_INT_32S_NBR_DIG_MAX                          10u



#define  DEF_INT_64_NBR_BITS                              64u
#define  DEF_INT_64_MASK                  0xFFFFFFFFFFFFFFFFu

#define  DEF_INT_64U_MIN_VAL                               0u
#define  DEF_INT_64U_MAX_VAL            18446744073709551615u

#define  DEF_INT_64S_MIN_VAL_ONES_CPL  (-9223372036854775807)
#define  DEF_INT_64S_MAX_VAL_ONES_CPL    9223372036854775807

#define  DEF_INT_64S_MIN_VAL                            (DEF_INT_64S_MIN_VAL_ONES_CPL - 1)
#define  DEF_INT_64S_MAX_VAL                             DEF_INT_64S_MAX_VAL_ONES_CPL

#define  DEF_INT_64U_NBR_DIG_MIN                           1u
#define  DEF_INT_64U_NBR_DIG_MAX                          20u

#define  DEF_INT_64S_NBR_DIG_MIN                          19u
#define  DEF_INT_64S_NBR_DIG_MAX                          19u



/*$PAGE*/
                                                                /* --------------- CPU INTEGER DEFINES ---------------- */
#define  DEF_INT_CPU_NBR_BITS                           (CPU_CFG_DATA_SIZE * DEF_OCTET_NBR_BITS)


#if     (DEF_INT_CPU_NBR_BITS == DEF_INT_08_NBR_BITS)


#define  DEF_INT_CPU_MASK                                DEF_INT_08_MASK

#define  DEF_INT_CPU_U_MIN_VAL                           DEF_INT_08U_MIN_VAL
#define  DEF_INT_CPU_U_MAX_VAL                           DEF_INT_08U_MAX_VAL

#define  DEF_INT_CPU_S_MIN_VAL                           DEF_INT_08S_MIN_VAL
#define  DEF_INT_CPU_S_MAX_VAL                           DEF_INT_08S_MAX_VAL

#define  DEF_INT_CPU_S_MIN_VAL_ONES_CPL                  DEF_INT_08S_MIN_VAL_ONES_CPL
#define  DEF_INT_CPU_S_MAX_VAL_ONES_CPL                  DEF_INT_08S_MAX_VAL_ONES_CPL



#elif   (DEF_INT_CPU_NBR_BITS == DEF_INT_16_NBR_BITS)


#define  DEF_INT_CPU_MASK                                DEF_INT_16_MASK

#define  DEF_INT_CPU_U_MIN_VAL                           DEF_INT_16U_MIN_VAL
#define  DEF_INT_CPU_U_MAX_VAL                           DEF_INT_16U_MAX_VAL

#define  DEF_INT_CPU_S_MIN_VAL                           DEF_INT_16S_MIN_VAL
#define  DEF_INT_CPU_S_MAX_VAL                           DEF_INT_16S_MAX_VAL

#define  DEF_INT_CPU_S_MIN_VAL_ONES_CPL                  DEF_INT_16S_MIN_VAL_ONES_CPL
#define  DEF_INT_CPU_S_MAX_VAL_ONES_CPL                  DEF_INT_16S_MAX_VAL_ONES_CPL



#elif   (DEF_INT_CPU_NBR_BITS == DEF_INT_32_NBR_BITS)


#define  DEF_INT_CPU_MASK                                DEF_INT_32_MASK

#define  DEF_INT_CPU_U_MIN_VAL                           DEF_INT_32U_MIN_VAL
#define  DEF_INT_CPU_U_MAX_VAL                           DEF_INT_32U_MAX_VAL

#define  DEF_INT_CPU_S_MIN_VAL                           DEF_INT_32S_MIN_VAL
#define  DEF_INT_CPU_S_MAX_VAL                           DEF_INT_32S_MAX_VAL

#define  DEF_INT_CPU_S_MIN_VAL_ONES_CPL                  DEF_INT_32S_MIN_VAL_ONES_CPL
#define  DEF_INT_CPU_S_MAX_VAL_ONES_CPL                  DEF_INT_32S_MAX_VAL_ONES_CPL



#elif   (DEF_INT_CPU_NBR_BITS == DEF_INT_64_NBR_BITS)


#define  DEF_INT_CPU_MASK                                DEF_INT_64_MASK

#define  DEF_INT_CPU_U_MIN_VAL                           DEF_INT_64U_MIN_VAL
#define  DEF_INT_CPU_U_MAX_VAL                           DEF_INT_64U_MAX_VAL

#define  DEF_INT_CPU_S_MIN_VAL                           DEF_INT_64S_MIN_VAL
#define  DEF_INT_CPU_S_MAX_VAL                           DEF_INT_64S_MAX_VAL

#define  DEF_INT_CPU_S_MIN_VAL_ONES_CPL                  DEF_INT_64S_MIN_VAL_ONES_CPL
#define  DEF_INT_CPU_S_MAX_VAL_ONES_CPL                  DEF_INT_64S_MAX_VAL_ONES_CPL



#else

#error  "CPU_CFG_DATA_SIZE  illegally #defined in 'cpu.h'      "
#error  "                   [See 'cpu.h  CONFIGURATION ERRORS']"

#endif



/*$PAGE*/
                                                                /* ------------------- TIME DEFINES ------------------- */
#define  DEF_TIME_NBR_DAY_PER_WK                           7uL
#define  DEF_TIME_NBR_DAY_PER_YR                         365uL
#define  DEF_TIME_NBR_DAY_PER_YR_LEAP                    366uL

#define  DEF_TIME_NBR_HR_PER_DAY                          24uL
#define  DEF_TIME_NBR_HR_PER_WK                         (DEF_TIME_NBR_HR_PER_DAY  * DEF_TIME_NBR_DAY_PER_WK     )
#define  DEF_TIME_NBR_HR_PER_YR                         (DEF_TIME_NBR_HR_PER_DAY  * DEF_TIME_NBR_DAY_PER_YR     )
#define  DEF_TIME_NBR_HR_PER_YR_LEAP                    (DEF_TIME_NBR_HR_PER_DAY  * DEF_TIME_NBR_DAY_PER_YR_LEAP)

#define  DEF_TIME_NBR_MIN_PER_HR                          60uL
#define  DEF_TIME_NBR_MIN_PER_DAY                       (DEF_TIME_NBR_MIN_PER_HR  * DEF_TIME_NBR_HR_PER_DAY     )
#define  DEF_TIME_NBR_MIN_PER_WK                        (DEF_TIME_NBR_MIN_PER_DAY * DEF_TIME_NBR_DAY_PER_WK     )
#define  DEF_TIME_NBR_MIN_PER_YR                        (DEF_TIME_NBR_MIN_PER_DAY * DEF_TIME_NBR_DAY_PER_YR     )
#define  DEF_TIME_NBR_MIN_PER_YR_LEAP                   (DEF_TIME_NBR_MIN_PER_DAY * DEF_TIME_NBR_DAY_PER_YR_LEAP)

#define  DEF_TIME_NBR_SEC_PER_MIN                         60uL
#define  DEF_TIME_NBR_SEC_PER_HR                        (DEF_TIME_NBR_SEC_PER_MIN * DEF_TIME_NBR_MIN_PER_HR     )
#define  DEF_TIME_NBR_SEC_PER_DAY                       (DEF_TIME_NBR_SEC_PER_HR  * DEF_TIME_NBR_HR_PER_DAY     )
#define  DEF_TIME_NBR_SEC_PER_WK                        (DEF_TIME_NBR_SEC_PER_DAY * DEF_TIME_NBR_DAY_PER_WK     )
#define  DEF_TIME_NBR_SEC_PER_YR                        (DEF_TIME_NBR_SEC_PER_DAY * DEF_TIME_NBR_DAY_PER_YR     )
#define  DEF_TIME_NBR_SEC_PER_YR_LEAP                   (DEF_TIME_NBR_SEC_PER_DAY * DEF_TIME_NBR_DAY_PER_YR_LEAP)

#define  DEF_TIME_NBR_mS_PER_SEC                        1000uL
#define  DEF_TIME_NBR_uS_PER_SEC                     1000000uL
#define  DEF_TIME_NBR_nS_PER_SEC                  1000000000uL


/*$PAGE*/
/*
*********************************************************************************************************
*                                             ERROR CODES
*
* Note(s) : (1) All generic library error codes are #define'd in 'lib_def.h';
*               Any module-specific error codes are #define'd in library module header files.
*********************************************************************************************************
*/

#define  LIB_ERR_NONE                                      0


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                    LIBRARY ERROR CODES DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  LIB_ERR;


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                             BIT MACRO'S
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              DEF_BIT()
*
* Description : Create bit mask with single, specified bit set.
*
* Argument(s) : bit         Bit number of bit to set.
*
* Return(s)   : Bit mask with single, specified bit set.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) 'bit' values that overflow the target CPU &/or compiler environment (e.g. negative
*                   or greater-than-CPU-data-size values) MAY generate compiler warnings &/or errors.
*********************************************************************************************************
*/

#define  DEF_BIT(bit)                            (1u << (bit))


/*$PAGE*/
/*
*********************************************************************************************************
*                                           DEF_BIT_MASK()
*
* Description : Shift a bit mask.
*
* Argument(s) : bit_mask    Bit mask to shift.
*
*               bit_shift   Number of bit positions to left-shift bit mask.
*
* Return(s)   : Shifted bit mask.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) 'bit_shift' values that overflow the target CPU &/or compiler environment (e.g. negative
*                   or greater-than-CPU-data-size values) MAY generate compiler warnings &/or errors.
*********************************************************************************************************
*/

#define  DEF_BIT_MASK(bit_mask, bit_shift)             ((bit_mask) << (bit_shift))


/*
*********************************************************************************************************
*                                           DEF_BIT_FIELD()
*
* Description : Create & shift a contiguous bit field.
*
* Argument(s) : bit_field   Number of contiguous bits to set in the bit field.
*
*               bit_shift   Number of bit positions   to left-shift bit field.
*
* Return(s)   : Shifted bit field.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) 'bit_field'/'bit_shift' values that overflow the target CPU &/or compiler environment 
*                   (e.g. negative or greater-than-CPU-data-size values) MAY generate compiler warnings
*                   &/or errors.
*********************************************************************************************************
*/

#define  DEF_BIT_FIELD(bit_field, bit_shift)         ((((bit_field) >= DEF_INT_CPU_NBR_BITS) ? (DEF_INT_CPU_U_MAX_VAL)   \
                                                                                             : (DEF_BIT(bit_field) - 1)) \
                                                                                                    << (bit_shift))


/*$PAGE*/
/*
*********************************************************************************************************
*                                            DEF_BIT_SET()
*
* Description : Set specified bit(s) in a value.
*
* Argument(s) : val         Value to modify by setting specified bit(s).
*
*               mask        Mask of bits to set.
*
* Return(s)   : Modified value with specified bit(s) set.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_BIT_SET(val, mask)                        ((val) |=  (mask))


/*
*********************************************************************************************************
*                                            DEF_BIT_CLR()
*
* Description : Clear specified bit(s) in a value.
*
* Argument(s) : val         Value to modify by clearing specified bit(s).
*
*               mask        Mask of bits to clear.
*
* Return(s)   : Modified value with specified bit(s) clear.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_BIT_CLR(val, mask)                        ((val) &= ~(mask))


/*$PAGE*/
/*
*********************************************************************************************************
*                                          DEF_BIT_IS_SET()
*
* Description : Determine if specified bit(s) in a value are set.
*
* Argument(s) : val         Value to check for specified bit(s) set.
*
*               mask        Mask of bits to check if set.
*
* Return(s)   : DEF_YES, if ALL specified bit(s) are     set in value.
*     
*               DEF_NO,	 if ALL specified bit(s) are NOT set in value.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_BIT_IS_SET(val, mask)                   ((((val) & (mask)) == (mask)) ? (DEF_YES) : (DEF_NO ))


/*
*********************************************************************************************************
*                                          DEF_BIT_IS_CLR()
*
* Description : Determine if specified bit(s) in a value are clear.
*
* Argument(s) : val         Value to check for specified bit(s) clear.
*
*               mask        Mask of bits to check if clear.
*
* Return(s)   : DEF_YES, if ALL specified bit(s) are     clear in value.
*     
*               DEF_NO,	 if ALL specified bit(s) are NOT clear in value.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_BIT_IS_CLR(val, mask)                    (((val) & (mask))            ? (DEF_NO ) : (DEF_YES))


/*$PAGE*/
/*
*********************************************************************************************************
*                                        DEF_BIT_IS_SET_ANY()
*
* Description : Determine if any specified bit(s) in a value are set.
*
* Argument(s) : val         Value to check for specified bit(s) set.
*
*               mask        Mask of bits to check if set.
*
* Return(s)   : DEF_YES, if ANY specified bit(s) are     set in value.
*     
*               DEF_NO,	 if ALL specified bit(s) are NOT set in value.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_BIT_IS_SET_ANY(val, mask)                (((val) & (mask))            ? (DEF_YES) : (DEF_NO ))


/*
*********************************************************************************************************
*                                        DEF_BIT_IS_CLR_ANY()
*
* Description : Determine if any specified bit(s) in a value are clear.
*
* Argument(s) : val         Value to check for specified bit(s) clear.
*
*               mask        Mask of bits to check if clear.
*
* Return(s)   : DEF_YES, if ANY specified bit(s) are     clear in value.
*     
*               DEF_NO,	 if ALL specified bit(s) are NOT clear in value.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_BIT_IS_CLR_ANY(val, mask)               ((((val) & (mask)) != (mask)) ? (DEF_YES) : (DEF_NO ))


/*$PAGE*/
/*
*********************************************************************************************************
*                                            MATH MACRO'S
*
* Note(s) : Ideally, ALL mathematical macro's & functions SHOULD be defined in the custom mathematic 
*           library ('lib_math.*').  #### However, to maintain backwards compatibility with previously-
*           released modules, mathematical macro & function definitions should only be moved to the 
*           custom mathematic library once all previously-released modules are updated to include the 
*           custom mathematic library.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              DEF_MIN()
*
* Description : Determine the minimum of two values.
*
* Argument(s) : a           First  value.
*
*               b           Second value.
*
* Return(s)   : Minimum of the two values.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_MIN(a, b)                                  (((a) < (b)) ? (a) : (b))


/*
*********************************************************************************************************
*                                              DEF_MAX()
*
* Description : Determine the maximum of two values.
*
* Argument(s) : a           First  value.
*
*               b           Second value.
*
* Return(s)   : Maximum of the two values.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_MAX(a, b)                                  (((a) > (b)) ? (a) : (b))


/*$PAGE*/
/*
*********************************************************************************************************
*                                              DEF_ABS()
*
* Description : Determine the absolute value of a value.
*
* Argument(s) : a           Value to calculate absolute value.
*
* Return(s)   : Absolute value of the value.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  DEF_ABS(a)                                     (((a) < 0) ? (-(a)) : (a))


/*$PAGE*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of lib def module include.                       */

