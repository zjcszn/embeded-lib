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
*                                           CORE CPU MODULE
*
* Filename      : cpu_core.h
* Version       : V1.23
* Programmer(s) : SR
*                 ITJ
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  CPU_CORE_MODULE_PRESENT
#define  CPU_CORE_MODULE_PRESENT


/*$PAGE*/
/*
*********************************************************************************************************
*                                   CORE CPU MODULE VERSION NUMBER
*
* Note(s) : (1) (a) The core CPU module software version is denoted as follows :
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

#define  CPU_CORE_VERSION                                123u   /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   CPU_CORE_MODULE
#define  CPU_CORE_EXT
#else
#define  CPU_CORE_EXT  extern
#endif


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <cpu_cfg.h>

#include  <lib_mem.h>
#include  <lib_str.h>


/*$PAGE*/
/*
*********************************************************************************************************
*                                          CPU CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure CPU parameters.  DO NOT MODIFY.
*
*           (2) CPU timestamp timer feature is required for :
*
*               (a) CPU timestamps
*               (b) CPU interrupts disabled time measurement
*
*               See also 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                      & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1'.
*********************************************************************************************************
*/

#if    ((CPU_CFG_TS_EN == DEF_ENABLED) || \
(defined(CPU_CFG_INT_DIS_MEAS_EN)))
#define  CPU_CFG_TS_TMR_EN                      DEF_ENABLED
#else
#define  CPU_CFG_TS_TMR_EN                      DEF_DISABLED
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define  CPU_TIME_MEAS_NBR_MIN                             1
#define  CPU_TIME_MEAS_NBR_MAX                           128


/*
*********************************************************************************************************
*                                           CPU ERROR CODES
*********************************************************************************************************
*/

#define  CPU_ERR_NONE                                      0
#define  CPU_ERR_NULL_PTR                                 10

#define  CPU_ERR_NAME_SIZE                               100


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      CPU ERROR CODES DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  CPU_ERR;


/*
*********************************************************************************************************
*                                       CPU TIMESTAMP DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT32U  CPU_TS;


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

#if    (CPU_CFG_NAME_EN   == DEF_ENABLED)
CPU_CORE_EXT  CPU_CHAR    CPU_Name[CPU_CFG_NAME_SIZE];          /* CPU host name.                                       */
#endif


#if    (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_CORE_EXT  CPU_INT16U  CPU_TS_TmrNbrShifts;                  /* Nbr left-shifts to adj ts tmr.                       */
#endif

#if    (CPU_CFG_TS_EN     == DEF_ENABLED)
CPU_CORE_EXT  CPU_TS      CPU_TS_Lo;                            /* Cur  ts (lo half) [in ts tmr cnts].                  */
CPU_CORE_EXT  CPU_TS      CPU_TS_Hi;                            /* Cur  ts (hi half) [in ts tmr cnts].                  */
CPU_CORE_EXT  CPU_TS      CPU_TS_Prev;                          /* Prev ts           [in ts tmr cnts].                  */
#endif

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
CPU_CORE_EXT  CPU_INT16U  CPU_IntDisNestCtr;                    /* Nbr nested ints dis'd ctr.                           */
                                                                /* Ints dis'd time (in ts tmr cnts) : ...               */
CPU_CORE_EXT  CPU_TS      CPU_IntDisMeasStartRaw_cnts;          /* ...  start time.                                     */
CPU_CORE_EXT  CPU_TS      CPU_IntDisMeasStopRaw_cnts;           /* ...  stop  time.                                     */
CPU_CORE_EXT  CPU_TS      CPU_IntDisMeasOvrhdRaw_cnts;          /* ...        time meas ovrhd.                          */
CPU_CORE_EXT  CPU_TS      CPU_IntDisMeasMaxCurRaw_cnts;         /* ... (raw/un-adj'd)     resetable max time dis'd.     */
CPU_CORE_EXT  CPU_TS      CPU_IntDisMeasMaxRaw_cnts;            /* ... (raw/un-adj'd) non-resetable max time dis'd.     */
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*
* Note(s) : (1) CPU interrupts disabled time measurement functions prototyped/defined only if 
*               CPU_CFG_INT_DIS_MEAS_EN  #define'd in 'cpu_cfg.h'.
*
*           (2) CPU_CntLeadZeros() prototyped/defined respectively in :
*
*               (a) 'cpu.h'/'cpu_a.asm',       if CPU_CFG_LEAD_ZEROS_ASM_PRESENT      #define'd in 'cpu.h'/
*                                                 'cpu_cfg.h' to enable assembly-version function
*
*               (b) 'cpu_core.h'/'cpu_core.c', if CPU_CFG_LEAD_ZEROS_ASM_PRESENT  NOT #define'd in 'cpu.h'/
*                                                 'cpu_cfg.h' to enable C-source-version function otherwise
*********************************************************************************************************
*/

void      CPU_Init                 (void);


#if      (CPU_CFG_NAME_EN == DEF_ENABLED)                       /* ------------------ CPU NAME FNCTS ------------------ */
void      CPU_NameClr              (void);

void      CPU_NameGet              (CPU_CHAR  *p_name,
                                    CPU_ERR   *p_err);

void      CPU_NameSet              (CPU_CHAR  *p_name,
                                    CPU_ERR   *p_err);
#endif


#if      (CPU_CFG_TS_EN   == DEF_ENABLED)                       /* ------------------- CPU TS FNCTS ------------------- */
void      CPU_TS_Get               (CPU_TS    *p_ts_lo,
                                    CPU_TS    *p_ts_hi);

CPU_TS    CPU_TS_GetLo             (void);

void      CPU_TS_Update            (void);
#endif


#ifdef    CPU_CFG_INT_DIS_MEAS_EN                               /* ------------ CPU INT DIS TIME MEAS FNCTS ----------- */
                                                                /* See Note #1.                                         */
CPU_TS    CPU_IntDisMeasMaxCurReset(void);

CPU_TS    CPU_IntDisMeasMaxCurGet  (void);

CPU_TS    CPU_IntDisMeasMaxGet     (void);


void      CPU_IntDisMeasStart      (void);

void      CPU_IntDisMeasStop       (void);
#endif


#ifndef   CPU_CFG_LEAD_ZEROS_ASM_PRESENT                        /* ------------- CPU CNT LEAD ZEROS FNCTS ------------- */
CPU_DATA  CPU_CntLeadZeros         (CPU_DATA   val);            /* See Note #2.                                         */
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                      DEFINED IN PRODUCT'S BSP
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          CPU_TS_TmrInit()
*
* Description : Initialize & start CPU timestamp timer.
*
* Argument(s) : none.
*
* Return(s)   : Number of left-shifts to scale & return timer as (32-bit) 'CPU_TS' data type 
*                 (see Note #2a1), if necessary.
*
*               0 (see Note #2a2), otherwise.
*
* Caller(s)   : CPU_TS_Init().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but MUST NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrInit() is an application/BSP function that MUST be defined by the developer 
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be scaled & returned via (32-bit) 'CPU_TS' data type.
*
*                       (1) If timer has less bits, left-shift timer values until the most 
*                           significant bit of the timer value is shifted into    the most 
*                           significant bit of the return timestamp data type.
*                       (2) If timer has more bits, truncate   timer values' higher-order 
*                           bits greater than  the return timestamp data type.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                   (c) When applicable, timer period SHOULD be less than the typical measured time 
*                       but MUST be less than the maximum measured time; otherwise, timer resolution 
*                       inadequate to measure desired times.
*
*                   See also 'CPU_TS_TmrRd()  Note #2'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_INT16U  CPU_TS_TmrInit(void);
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_TS_TmrRd()
*
* Description : Get current CPU timestamp timer count value.
*
* Argument(s) : none.
*
* Return(s)   : (32-bit) Timestamp timer count (see Notes #2a & #2b).
*
* Caller(s)   : CPU_TS_Init(),
*               CPU_TS_UpdateHandler(),
*               CPU_IntDisMeasStart(),
*               CPU_IntDisMeasStop().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but SHOULD NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrRd() is an application/BSP function that MUST be defined by the developer 
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via (32-bit) 'CPU_TS' data type.
*
*                       (1) If timer has less bits, left-shift timer values until the most 
*                           significant bit of the timer value is shifted into    the most 
*                           significant bit of the return timestamp data type.
*                       (2) If timer has more bits, truncate   timer values' higher-order 
*                           bits greater than  the return timestamp data type.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                       (1) If timer is a 'down' counter whose values decrease with each time count,
*                           then the returned timer value MUST be ones-complemented.
*
*                   (c) (1) When applicable, the amount of time measured by CPU timestamps is 
*                           calculated by either of the following equations :
*
*                           (A) Time measured  =  Number timer counts  *  Timer period
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured 
*                                       Timer period            Timer's period in some units of 
*                                                                   (fractional) seconds
*                                       Time measured           Amount of time measured, in same 
*                                                                   units of (fractional) seconds 
*                                                                   as the Timer period
*
*                                                  Number timer counts
*                           (B) Time measured  =  ---------------------
*                                                    Timer frequency
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured 
*                                       Timer frequency         Timer's frequency in some units 
*                                                                   of counts per second
*                                       Time measured           Amount of time measured, in seconds
*
*                       (2) Timer period SHOULD be less than the typical measured time but MUST be less 
*                           than the maximum measured time; otherwise, timer resolution inadequate to 
*                           measure desired times.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_TS  CPU_TS_TmrRd(void);
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          CPU_TS_to_uSec()
*
* Description : Convert a CPU timestamp from timer counts to microseconds.
*
* Argument(s) : ts_lo_cnts      CPU timestamp lower half (in timestamp timer counts [see Note #2aA]).
*
*               ts_hi_cnts      CPU timestamp upper half (in timestamp timer counts [see Note #2aA]).
*
*               p_ts_lo_usec    Pointer to variable that will receive the converted CPU timestamp's 
*                                   lower half [in microseconds (see Note #2aD)], if available.
*
*               p_ts_hi_usec    Pointer to variable that will receive the converted CPU timestamp's 
*                                   upper half [in microseconds (see Note #2aD)], if available.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
*               This function is an (optional) CPU module application interface (API) function which 
*               MAY be implemented by application/BSP function(s) [see Note #1] & MAY be called by 
*               application function(s).
*
* Note(s)     : (1) CPU_TS_to_uSec() is an application/BSP function that MAY be optionally defined by 
*                   the developer when either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) The amount of time measured by CPU timestamps is calculated by either of 
*                       the following equations :
*
*                                                                        10^6 microseconds
*                       (1) Time measured  =   Number timer counts   *  -------------------  *  Timer period  
*                                                                            1 second
*
*                                              Number timer counts       10^6 microseconds
*                       (2) Time measured  =  ---------------------  *  -------------------
*                                                Timer frequency             1 second
*
*                               where
*
*                                   (A) Number timer counts     Number of timer counts measured
*                                   (B) Timer frequency         Timer's frequency in some units 
*                                                                   of counts per second
*                                   (C) Timer period            Timer's period in some units of 
*                                                                   (fractional)  seconds
*                                   (D) Time measured           Amount of time measured, 
*                                                                   in microseconds
*
*                   (b) Timer period SHOULD be less than the typical measured time but MUST be less 
*                       than the maximum measured time; otherwise, timer resolution inadequate to 
*                       measure desired times.
*
*                   (c) Specific implementations may convert any number of CPU timestamps bits, up 
*                       to 64, into microseconds.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_to_uSec(CPU_TS       ts_lo_cnts,
                     CPU_TS       ts_hi_cnts,
                     CPU_INT32U  *p_ts_lo_usec,
                     CPU_INT32U  *p_ts_hi_usec);
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  CPU_CFG_NAME_EN
#error  "CPU_CFG_NAME_EN                       not #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]           "
#error  "                                [     ||  DEF_DISABLED]           "

#elif  ((CPU_CFG_NAME_EN != DEF_ENABLED ) && \
        (CPU_CFG_NAME_EN != DEF_DISABLED))
#error  "CPU_CFG_NAME_EN                 illegally #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]           "
#error  "                                [     ||  DEF_DISABLED]           "

#else

#ifndef  CPU_CFG_NAME_SIZE
#error  "CPU_CFG_NAME_SIZE                     not #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  >=   1]                 "
#error  "                                [     &&  <= 255]                 "

#elif  ((CPU_CFG_NAME_SIZE <                   1) || \
        (CPU_CFG_NAME_SIZE > DEF_INT_08U_MAX_VAL))
#error  "CPU_CFG_NAME_SIZE               illegally #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  >=   1]                 "
#error  "                                [     &&  <= 255]                 "
#endif

#endif




#ifndef  CPU_CFG_TS_EN
#error  "CPU_CFG_TS_EN                         not #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  DEF_DISABLED]           "
#error  "                                [     ||  DEF_ENABLED ]           "

#elif  ((CPU_CFG_TS_EN != DEF_DISABLED) && \
        (CPU_CFG_TS_EN != DEF_ENABLED ))
#error  "CPU_CFG_TS_EN                   illegally #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  DEF_DISABLED]           "
#error  "                                [     ||  DEF_ENABLED ]           "

#endif


                                                                /* Correctly configured in 'cpu_core.h'; DO NOT MODIFY. */
#ifndef  CPU_CFG_TS_TMR_EN
#error  "CPU_CFG_TS_TMR_EN                     not #define'd in 'cpu_core.h'"
#error  "                                [MUST be  DEF_DISABLED]            "
#error  "                                [     ||  DEF_ENABLED ]            "

#elif  ((CPU_CFG_TS_TMR_EN != DEF_DISABLED) && \
        (CPU_CFG_TS_TMR_EN != DEF_ENABLED ))
#error  "CPU_CFG_TS_TMR_EN               illegally #define'd in 'cpu_core.h'"
#error  "                                [MUST be  DEF_DISABLED]            "
#error  "                                [     ||  DEF_ENABLED ]            "

#endif



#ifndef  CPU_CFG_INT_DIS_MEAS_EN
#if 0                                                           /* Optionally configured in 'cpu_cfg.h'; DO NOT MODIFY. */
#error  "CPU_CFG_INT_DIS_MEAS_EN               not #define'd in 'cpu_cfg.h'"
#endif

#else

#ifndef  CPU_CFG_INT_DIS_MEAS_OVRHD_NBR
#error  "CPU_CFG_INT_DIS_MEAS_OVRHD_NBR        not #define'd in 'cpu_cfg.h' "
#error  "                                [MUST be  >= CPU_TIME_MEAS_NBR_MIN]"
#error  "                                [     ||  <= CPU_TIME_MEAS_NBR_MAX]"

#elif  ((CPU_CFG_INT_DIS_MEAS_OVRHD_NBR < CPU_TIME_MEAS_NBR_MIN) || \
        (CPU_CFG_INT_DIS_MEAS_OVRHD_NBR > CPU_TIME_MEAS_NBR_MAX))
#error  "CPU_CFG_INT_DIS_MEAS_OVRHD_NBR  illegally #define'd in 'cpu_cfg.h' "
#error  "                                [MUST be  >= CPU_TIME_MEAS_NBR_MIN]"
#error  "                                [     ||  <= CPU_TIME_MEAS_NBR_MAX]"

#endif

#endif




#ifndef  CPU_CFG_LEAD_ZEROS_ASM_PRESENT
#if 0                                                           /* Optionally configured in 'cpu_cfg.h'; DO NOT MODIFY. */
#error  "CPU_CFG_LEAD_ZEROS_ASM_PRESENT        not #define'd in 'cpu.h'/'cpu_cfg.h'"
#endif
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of CPU core module include.                      */

