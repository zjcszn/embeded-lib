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
* Filename      : cpu_core.c
* Version       : V1.23
* Programmer(s) : SR
*                 ITJ
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    CPU_CORE_MODULE
#include  <cpu_core.h>


/*$PAGE*/
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                  CPU COUNT LEAD ZEROs LOOKUP TABLE
*
* Note(s) : (1) Index into bit pattern table determines the number of leading zeros in an 8-bit value :
*
*                         b07  b06  b05  b04  b03  b02  b01  b00    # Leading Zeros
*                         ---  ---  ---  ---  ---  ---  ---  ---    ---------------
*                          1    x    x    x    x    x    x    x            0
*                          0    1    x    x    x    x    x    x            1
*                          0    0    1    x    x    x    x    x            2
*                          0    0    0    1    x    x    x    x            3
*                          0    0    0    0    1    x    x    x            4
*                          0    0    0    0    0    1    x    x            5
*                          0    0    0    0    0    0    1    x            6
*                          0    0    0    0    0    0    0    1            7
*                          0    0    0    0    0    0    0    0            8
*********************************************************************************************************
*/

#ifndef   CPU_CFG_LEAD_ZEROS_ASM_PRESENT
static  const  CPU_INT08U  CPU_CntLeadZerosTbl[256] = {         /* Data vals :                                          */
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,             /*   0x00 to 0x0F                                       */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,             /*   0x10 to 0x1F                                       */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,             /*   0x20 to 0x2F                                       */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,             /*   0x30 to 0x3F                                       */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,             /*   0x40 to 0x4F                                       */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,             /*   0x50 to 0x5F                                       */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,             /*   0x60 to 0x6F                                       */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,             /*   0x70 to 0x7F                                       */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             /*   0x80 to 0x8F                                       */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             /*   0x90 to 0x9F                                       */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             /*   0xA0 to 0xAF                                       */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             /*   0xB0 to 0xBF                                       */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             /*   0xC0 to 0xCF                                       */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             /*   0xD0 to 0xDF                                       */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             /*   0xE0 to 0xEF                                       */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0              /*   0xF0 to 0xFF                                       */
};
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if    (CPU_CFG_NAME_EN   == DEF_ENABLED)                           /* ---------------- CPU NAME FNCTS ---------------- */
static  void    CPU_NameInit         (void);
#endif


                                                                    /* ----------------- CPU TS FNCTS ----------------- */
#if   ((CPU_CFG_TS_TMR_EN == DEF_ENABLED) || \
       (CPU_CFG_TS_EN     == DEF_ENABLED)) 
static  void    CPU_TS_Init          (void);
#endif

#if    (CPU_CFG_TS_EN     == DEF_ENABLED)
static  void    CPU_TS_UpdateHandler (CPU_TS  *p_ts_lo,
                                      CPU_TS  *p_ts_hi);
#endif


#ifdef  CPU_CFG_INT_DIS_MEAS_EN                                     /* ---------- CPU INT DIS TIME MEAS FNCTS --------- */
static  void    CPU_IntDisMeasInit   (void);

static  CPU_TS  CPU_IntDisMeasMaxCalc(CPU_TS   time_max_cnts_raw);
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                             CPU_Init()
*
* Description : (1) Initialize CPU module :
*
*                   (a) Initialize CPU timestamps
*                   (b) Initialize CPU interrupts disabled time measurements
*                   (c) Initialize CPU host name
*
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Your Product's Application.
*
*               This function is a CPU initialization function & MAY be called by application/
*               initialization function(s).
*
* Note(s)     : (2) CPU_Init() MUST be called ... :
*
*                   (a) ONLY ONCE from a product's application; ...
*                   (b) BEFORE product's application calls any core CPU module function(s)
*
*               (3) The following initialization functions MUST be sequenced as follows :
*
*                   (a) CPU_TS_Init()           SHOULD precede ALL calls to other CPU timestamp functions
*
*                   (b) CPU_IntDisMeasInit()    SHOULD precede ALL calls to CPU_CRITICAL_ENTER()/CPU_CRITICAL_EXIT() 
*                                                   & other CPU interrupts disabled time measurement functions
*********************************************************************************************************
*/

void  CPU_Init (void)
{
                                                                /* --------------------- INIT TS ---------------------- */
#if ((CPU_CFG_TS_TMR_EN == DEF_ENABLED) || \
     (CPU_CFG_TS_EN     == DEF_ENABLED)) 
    CPU_TS_Init();                                              /* See Note #3a.                                        */
#endif
                                                                /* -------------- INIT INT DIS TIME MEAS -------------- */
#ifdef  CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasInit();                                       /* See Note #3b.                                        */
#endif

                                                                /* ------------------ INIT CPU NAME ------------------- */
#if (CPU_CFG_NAME_EN == DEF_ENABLED)
     CPU_NameInit();
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            CPU_NameClr()
*
* Description : Clear CPU Name.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_NameInit(),
*               Application.
*
*               This function is a CPU module application interface (API) function & MAY be called by
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
void  CPU_NameClr (void)
{
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    Mem_Clr((void     *)&CPU_Name[0],
            (CPU_SIZE_T) CPU_CFG_NAME_SIZE);
    CPU_CRITICAL_EXIT();
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                            CPU_NameGet()
*
* Description : Get CPU host name.
*
* Argument(s) : p_name      Pointer to an ASCII character array that will receive the return CPU host 
*                               name ASCII string from this function (see Note #1).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               CPU_ERR_NONE                    CPU host name successfully returned.
*                               CPU_ERR_NULL_PTR                Argument 'p_name' passed a NULL pointer.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
*               This function is a CPU module application interface (API) function & MAY be called by
*               application function(s).
*
* Note(s)     : (1) The size of the ASCII character array that will receive the return CPU host name 
*                   ASCII string :
*
*                   (a) MUST   be greater than or equal to the current CPU host name's ASCII string 
*                           size including the terminating NULL character;
*                   (b) SHOULD be greater than or equal to CPU_CFG_NAME_SIZE
*********************************************************************************************************
*/

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
void  CPU_NameGet (CPU_CHAR  *p_name,
                   CPU_ERR   *p_err)
{
    CPU_SR_ALLOC();


    if (p_name == (CPU_CHAR *)0) {
       *p_err = CPU_ERR_NULL_PTR;
        return;
    }

    CPU_CRITICAL_ENTER();
    Str_Copy((CPU_CHAR *) p_name,
             (CPU_CHAR *)&CPU_Name[0]);
    CPU_CRITICAL_EXIT();

   *p_err = CPU_ERR_NONE;
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                            CPU_NameSet()
*
* Description : Set CPU host name.
*
* Argument(s) : p_name      Pointer to CPU host name to set.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               CPU_ERR_NONE                    CPU host name successfully set.
*                               CPU_ERR_NULL_PTR                Argument 'p_name' passed a NULL pointer.
*                               CPU_ERR_NAME_SIZE               Invalid CPU host name size (see Note #1).
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
*               This function is a CPU module application interface (API) function & MAY be called by
*               application function(s).
*
* Note(s)     : (1) 'p_name' ASCII string size, including the terminating NULL character, MUST be less
*                    than or equal to CPU_CFG_NAME_SIZE.
*********************************************************************************************************
*/

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
void  CPU_NameSet (CPU_CHAR  *p_name,
                   CPU_ERR   *p_err)
{
    CPU_SIZE_T  len;
    CPU_SR_ALLOC();


    if (p_name == (CPU_CHAR *)0) {
       *p_err = CPU_ERR_NULL_PTR;
        return;
    }

    len = Str_Len(p_name);
    if (len < CPU_CFG_NAME_SIZE) {                              /* If       cfg name len < max name size, ...           */
        CPU_CRITICAL_ENTER();
        Str_Copy((CPU_CHAR *)&CPU_Name[0],                      /* ... copy cfg name to CPU host name.                  */
                 (CPU_CHAR *) p_name);
        CPU_CRITICAL_EXIT();
       *p_err = CPU_ERR_NONE;

    } else {
       *p_err = CPU_ERR_NAME_SIZE;
    }
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                            CPU_TS_Get()
*
* Description : Get current CPU timestamp.
*
* Argument(s) : p_ts_lo     Pointer to timestamp variable that will receive the current CPU timestamp's 
*                               lower half (in timestamp timer counts), if available.
*
*               p_ts_hi     Pointer to timestamp variable that will receive the current CPU timestamp's 
*                               upper half (in timestamp timer counts), if available.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
*               This function is a CPU module application interface (API) function & MAY be called by
*               application function(s).
*
* Note(s)     : (1) When applicable, the amount of time measured by CPU timestamps is calculated by 
*                   either of the following equations :
*
*                       (a) Time measured  =  Number timer counts  *  Timer period
*
*                               where
*
*                                   Number timer counts     Number of timer counts measured 
*                                   Timer period            Timer's period in some units of 
*                                                               (fractional) seconds
*                                   Time measured           Amount of time measured, in same 
*                                                               units of (fractional) seconds 
*                                                               as the Timer period
*
*                                              Number timer counts
*                       (b) Time measured  =  ---------------------
*                                                Timer frequency
*
*                               where
*
*                                   Number timer counts     Number of timer counts measured 
*                                   Timer frequency         Timer's frequency in some units 
*                                                               of counts per second
*                                   Time measured           Amount of time measured, in seconds
*
*                   See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()  Note #2c1'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_EN == DEF_ENABLED)
void  CPU_TS_Get (CPU_TS  *p_ts_lo,
                  CPU_TS  *p_ts_hi)
{
    CPU_TS_UpdateHandler(p_ts_lo, p_ts_hi);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_TS_GetLo()
*
* Description : Get current CPU timestamp, lower-half only.
*
* Argument(s) : none.
*
* Return(s)   : Current CPU timestamp's lower half (in timestamp timer counts).
*
* Caller(s)   : Application.
*
*               This function is a CPU module application interface (API) function & MAY be called by
*               application function(s).
*
* Note(s)     : (1) When applicable, the amount of time measured by CPU timestamps is calculated by 
*                   either of the following equations :
*
*                       (a) Time measured  =  Number timer counts  *  Timer period
*
*                               where
*
*                                   Number timer counts     Number of timer counts measured 
*                                   Timer period            Timer's period in some units of 
*                                                               (fractional) seconds
*                                   Time measured           Amount of time measured, in same 
*                                                               units of (fractional) seconds 
*                                                               as the Timer period
*
*                                              Number timer counts
*                       (b) Time measured  =  ---------------------
*                                                Timer frequency
*
*                               where
*
*                                   Number timer counts     Number of timer counts measured 
*                                   Timer frequency         Timer's frequency in some units 
*                                                               of counts per second
*                                   Time measured           Amount of time measured, in seconds
*
*                   See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()  Note #2c1'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_EN == DEF_ENABLED)
CPU_TS  CPU_TS_GetLo (void)
{
    CPU_TS  ts_lo;


    CPU_TS_UpdateHandler((CPU_TS *)&ts_lo,
                         (CPU_TS *) 0);

    return (ts_lo);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_TS_Update()
*
* Description : Update current CPU timestamp. 
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application/BSP periodic time handler (see Note #1).
*
* Note(s)     : (1) (a) CPU timestamp MUST be updated periodically by some application (or BSP) time 
*                       handler in order to (adequately) maintain the CPU timestamp time.
*
*                   (b) CPU timestamp MUST be updated more frequently than timestamp timer overflows; 
*                       otherwise, CPU timestamp will lose time.
*
*                       See also 'CPU_TS_UpdateHandler()  Note #2'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_EN == DEF_ENABLED)
void  CPU_TS_Update (void)
{
    CPU_TS_UpdateHandler((CPU_TS *)0,
                         (CPU_TS *)0);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                     CPU_IntDisMeasMaxCurReset()
*
* Description : Reset current maximum interrupts disabled time.
*
* Argument(s) : none.
*
* Return(s)   : Maximum interrupts disabled time (in timestamp timer counts) before resetting.
*
*               See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()    Note #2c'
*                      & 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_to_uSec()  Note #2'.
*
* Caller(s)   : Application.
*
*               This function is a CPU module application interface (API) function & MAY be called 
*               by application function(s).
*
* Note(s)     : (1) After initialization, 'CPU_IntDisMeasMaxCurRaw_cnts' MUST ALWAYS be accessed 
*                   exclusively with interrupts disabled -- but NOT with critical sections.
*********************************************************************************************************
*/

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
CPU_TS  CPU_IntDisMeasMaxCurReset (void)
{
    CPU_TS  time_max_cnts;
    CPU_SR_ALLOC();


    time_max_cnts                = CPU_IntDisMeasMaxCurGet();
    CPU_INT_DIS();
    CPU_IntDisMeasMaxCurRaw_cnts = 0;
    CPU_INT_EN();

    return (time_max_cnts);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                      CPU_IntDisMeasMaxCurGet()
*
* Description : Get current maximum interrupts disabled time.
*
* Argument(s) : none.
*
* Return(s)   : Current maximum interrupts disabled time (in timestamp timer counts).
*
*               See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()    Note #2c'
*                      & 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_to_uSec()  Note #2'.
*
* Caller(s)   : CPU_IntDisMeasMaxCurReset(),
*               Application.
*
*               This function is a CPU module application interface (API) function & MAY be called 
*               by application function(s).
*
* Note(s)     : (1) After initialization, 'CPU_IntDisMeasMaxCurRaw_cnts' MUST ALWAYS be accessed 
*                   exclusively with interrupts disabled -- but NOT with critical sections.
*********************************************************************************************************
*/

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
CPU_TS  CPU_IntDisMeasMaxCurGet (void)
{
    CPU_TS  time_max_cnts;
    CPU_TS  time_max_cnts_raw;
    CPU_SR_ALLOC();


    CPU_INT_DIS();
    time_max_cnts_raw = CPU_IntDisMeasMaxCurRaw_cnts;
    CPU_INT_EN();
    time_max_cnts     = CPU_IntDisMeasMaxCalc(time_max_cnts_raw);

    return (time_max_cnts);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       CPU_IntDisMeasMaxGet()
*
* Description : Get (non-resetable) maximum interrupts disabled time.
*
* Argument(s) : none.
*
* Return(s)   : (Non-resetable) maximum interrupts disabled time (in timestamp timer counts).
*
*               See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()    Note #2c'
*                      & 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_to_uSec()  Note #2'.
*
* Caller(s)   : CPU_IntDisMeasInit(),
*               Application.
*
*               This function is a CPU module application interface (API) function & MAY be called 
*               by application function(s).
*
* Note(s)     : (1) After initialization, 'CPU_IntDisMeasMaxRaw_cnts' MUST ALWAYS be accessed 
*                   exclusively with interrupts disabled -- but NOT with critical sections.
*********************************************************************************************************
*/

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
CPU_TS  CPU_IntDisMeasMaxGet (void)
{
    CPU_TS  time_max_cnts;
    CPU_TS  time_max_cnts_raw;
    CPU_SR_ALLOC();


    CPU_INT_DIS();
    time_max_cnts_raw = CPU_IntDisMeasMaxRaw_cnts;
    CPU_INT_EN();
    time_max_cnts     = CPU_IntDisMeasMaxCalc(time_max_cnts_raw);

    return (time_max_cnts);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CPU_IntDisMeasStart()
*
* Description : Start interrupts disabled time measurement.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_CRITICAL_ENTER().
*
*               This function is an INTERNAL CPU module function & MUST NOT be called by application 
*               function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
void  CPU_IntDisMeasStart (void)
{
    if (CPU_IntDisNestCtr == 0) {                                   /* If ints NOT yet dis'd, ...                       */
        CPU_IntDisMeasStartRaw_cnts = CPU_TS_TmrRd();               /* ... get ints dis'd start time.                   */
    }
    CPU_IntDisNestCtr++;
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CPU_IntDisMeasStop()
*
* Description : Stop interrupts disabled time measurement.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_CRITICAL_EXIT().
*
*               This function is an INTERNAL CPU module function & MUST NOT be called by application 
*               function(s).
*
* Note(s)     : (1) (a) The total amount of time interrupts are disabled by system &/or application code 
*                       during critical sections is calculated by the following equations :
*
*                       (1) When interrupts disabled time measurements are disabled :
*
*
*                               |   CRITICAL  |                           |   CRITICAL  |
*                               |<- SECTION ->|                           |<- SECTION ->|
*                               |    ENTER    |                           |    EXIT     |
*
*                            Disable                                    Enable
*                           Interrupts                                Interrupts
*
*                               ||           ||                           ||           ||
*                               ||           ||                           ||           ||
*                               ||       |   ||<------------------------->||       |   ||
*                               ||       |<->||             |             ||<----->|   ||
*                               ||       | | ||             |             ||   |   |   ||
*                                        | |                |                  |   |
*                                   interrupts            time                 interrupts
*                                    disabled                 interrupts       |enabled
*                                          |                   disabled        |
*                                          |              (via application)    |
*                                       time                                 time
*                                           interrupts                           interrupts
*                                         disabled ovrhd                        enabled ovrhd
*
*
*                           (A) time            =  [ time            -  time           ]  -  time
*                                   interrupts     [     interrupts         interrupts ]         total
*                                    disabled      [      enabled            disabled  ]         ovrhd
*                               (via application)
*
*
*                           (B) time       =  time              +  time
*                                   total         interrupts           interrupts
*                                   ovrhd        enabled ovrhd       disabled ovrhd
*
*
*                                   where
*
*                                           time                    time interrupts are disabled between 
*                                               interrupts              first critical section enter & 
*                                                disabled               last  critical section exit (i.e. 
*                                           (via application)           minus total overhead time)
*
*                                           time                    time when interrupts are disabled
*                                               interrupts
*                                                disabled
*
*                                           time                    time when interrupts are  enabled
*                                               interrupts
*                                                enabled
*
*
*                                           time                    total overhead time to disable/enable 
*                                               total                   interrupts during critical section 
*                                               ovrhd                   enter & exit
*
*                                           time                    total overhead time to disable interrupts
*                                               interrupts              during critical section enter
*                                             disabled ovrhd
*
*                                           time                    total overhead time to enable  interrupts
*                                               interrupts              during critical section exit
*                                              enabled ovrhd
*
*$PAGE*
*
*                       (2) When interrupts disabled time measurements are enabled :
*
*
*        |                                    |                           |                                       |
*        |<----- CRITICAL SECTION ENTER ----->|                           |<------- CRITICAL SECTION EXIT ------->|
*        |                                    |                           |                                       |
*
*                   Time                                                 Time
*     Disable    Measurement                                          Measurement                  Enable
*    Interrupts     Start                                                Stop                    Interrupts
*
*        ||           |                      ||                           ||                         |           ||
*        ||           |                      ||                           ||                         |           ||
*        ||           |        |             ||<------------------------->||               |         |           ||
*        ||       |   |        |<----------->||             |             ||<------------->|         |       |   ||
*        ||       |   |        |      |      ||             |             ||       |       |         |       |   ||
*                 |            |      |                     |                      |       |                 |
*            interrupts       get     |                   time                     |      get            interrupts
*             disabled    start time  |                       interrupts           |   stop time          enabled
*                            meas     |                        disabled            |     meas
*                                   time                  (via application)      time
*                                       start meas                                   stop meas
*                                         ovrhd                                        ovrhd
*
*
*                                                   [ time      -  time      ]  -  time
*                                                   [     stop         start ]         total meas
*                                                   [     meas         meas  ]           ovrhd
*                           (A) time            =  -----------------------------------------------
*                                   interrupts                       nbr scaled
*                                    disabled                          shifts
*                               (via application)                   2
*
*
*                           (B) time            =  time            +  time
*                                   total meas         start meas         stop meas
*                                     ovrhd              ovrhd              ovrhd
*
*
*                                   where
*
*                                           time                    time interrupts are disabled between first 
*                                               interrupts              critical section enter & last critical 
*                                                disabled               section exit (i.e. minus measurement 
*                                           (via application)           overhead time; however, this does NOT 
*                                                                       include any overhead time to disable 
*                                                                       or enable interrupts during critical 
*                                                                       section enter & exit)
*
*                                           time                    time of disable interrupts start time 
*                                               start                   measurement (in raw, scaled timer 
*                                               meas                    counts)
*
*                                           time                    time of disable interrupts stop  time 
*                                               stop                    measurement (in raw, scaled timer 
*                                               meas                    counts)
*
*
*                                           time                    total overhead time to start/stop disabled 
*                                               total meas              interrupts time measurements (in raw, 
*                                                 ovrhd                 scaled timer counts)
*
*                                           time                    total overhead time after getting start 
*                                               start meas              time until end of start measurement 
*                                                 ovrhd                 function (in raw, scaled timer counts)
*
*                                           time                    total overhead time from beginning of stop 
*                                               stop meas               measurement function until after getting 
*                                                 ovrhd                 stop time (in raw, scaled timer counts)
*
*                                           nbr scaled              number of left bit-shifts, if any, to scale 
*                                             shifts                    timer to (32-bit) 'CPU_TS' data type 
*                                                                       (see also 'cpu_core.h  CPU_TS_TmrRd()  
*                                                                        Note #2a1')
*
*$PAGE*
*                   (b) (1) (A) In order to correctly handle unsigned subtraction overflows of start times 
*                               from stop times, timestamp timer count values MUST be scaled to (32-bit) 
*                               'CPU_TS' data type.
*
*                               See 'cpu_core.h  CPU_TS_TmrRd()  Note #2a'.
*
*                           (B) Since unsigned subtraction of start times from stop times assumes increasing 
*                               values, timestamp timer count values MUST increase with each time count.
*
*                               See 'cpu_core.h  CPU_TS_TmrRd()  Note #2b'.
*
*                       (2) (A) To expedite & reduce interrupts disabled time measurement overhead; only the 
*                               subtraction of the raw, scaled start times from stop times is performed.
*
*                           (B) The final calculations to subtract the interrupts disabled time measurement 
*                               overhead AND to convert from raw, scaled timer counts to normalized timestamp 
*                               timer counts are performed asynchronously in appropriate API functions.
*
*                               See also 'CPU_IntDisMeasMaxCalc()  Note #1b'.
*********************************************************************************************************
*/

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
void  CPU_IntDisMeasStop (void)
{
    CPU_TS  time_ints_disd_cnts;


    CPU_IntDisNestCtr--;
    if (CPU_IntDisNestCtr == 0) {                                   /* If ints NO longer dis'd,        ...              */
        CPU_IntDisMeasStopRaw_cnts = CPU_TS_TmrRd();                /* ... get  ints dis'd stop time & ...              */
                                                                    /* ... calc ints dis'd tot  time (see Note #1b2A).  */
        time_ints_disd_cnts        = CPU_IntDisMeasStopRaw_cnts -
                                     CPU_IntDisMeasStartRaw_cnts;
                                                                    /* Calc max ints dis'd times.                       */
        if (CPU_IntDisMeasMaxCurRaw_cnts < time_ints_disd_cnts) {
            CPU_IntDisMeasMaxCurRaw_cnts = time_ints_disd_cnts;
        }
        if (CPU_IntDisMeasMaxRaw_cnts    < time_ints_disd_cnts) {
            CPU_IntDisMeasMaxRaw_cnts    = time_ints_disd_cnts;
        }
    }
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         CPU_CntLeadZeros()
*
* Description : Count the number of contiguous, most-significant, leading zero bits in a data value.
*
* Argument(s) : val         Data value to count leading zero bits.
*
* Return(s)   : Number of contiguous, most-significant, leading zero bits in 'val', if NO errors.
*
*               0,                                                                  otherwise.
*
* Caller(s)   : Application.
*
*               This function is an INTERNAL CPU module function but MAY be called by application function(s).
*
* Note(s)     : (1) (a) Supports the following data value sizes :
*
*                       (1)  8-bits
*                       (2) 16-bits
*                       (3) 32-bits
*
*                       See also 'cpu_def.h  CPU WORD CONFIGURATION  Note #1'.
*
*                   (b) (1) For  8-bit values :
*
*                                  b07  b06  b05  b04  b03  b02  b01  b00    # Leading Zeros
*                                  ---  ---  ---  ---  ---  ---  ---  ---    ---------------
*                                   1    x    x    x    x    x    x    x            0
*                                   0    1    x    x    x    x    x    x            1
*                                   0    0    1    x    x    x    x    x            2
*                                   0    0    0    1    x    x    x    x            3
*                                   0    0    0    0    1    x    x    x            4
*                                   0    0    0    0    0    1    x    x            5
*                                   0    0    0    0    0    0    1    x            6
*                                   0    0    0    0    0    0    0    1            7
*                                   0    0    0    0    0    0    0    0            8
*
*
*                       (2) For 16-bit values :
*
*                             b15  b14  b13  ...  b04  b03  b02  b01  b00    # Leading Zeros
*                             ---  ---  ---       ---  ---  ---  ---  ---    ---------------
*                              1    x    x         x    x    x    x    x            0
*                              0    1    x         x    x    x    x    x            1
*                              0    0    1         x    x    x    x    x            2
*                              :    :    :         :    :    :    :    :            :
*                              :    :    :         :    :    :    :    :            :
*                              0    0    0         1    x    x    x    x           11
*                              0    0    0         0    1    x    x    x           12
*                              0    0    0         0    0    1    x    x           13
*                              0    0    0         0    0    0    1    x           14
*                              0    0    0         0    0    0    0    1           15
*                              0    0    0         0    0    0    0    0           16
*
*
*                       (3) For 32-bit values :
*
*                             b31  b30  b29  ...  b04  b03  b02  b01  b00    # Leading Zeros
*                             ---  ---  ---       ---  ---  ---  ---  ---    ---------------
*                              1    x    x         x    x    x    x    x            0
*                              0    1    x         x    x    x    x    x            1
*                              0    0    1         x    x    x    x    x            2
*                              :    :    :         :    :    :    :    :            :
*                              :    :    :         :    :    :    :    :            :
*                              0    0    0         1    x    x    x    x           27
*                              0    0    0         0    1    x    x    x           28
*                              0    0    0         0    0    1    x    x           29
*                              0    0    0         0    0    0    1    x           30
*                              0    0    0         0    0    0    0    1           31
*                              0    0    0         0    0    0    0    0           32
*
*
*                       See also 'CPU COUNT LEAD ZEROs LOOKUP TABLE  Note #1'.
*********************************************************************************************************
*/
/*$PAGE*/
#ifndef   CPU_CFG_LEAD_ZEROS_ASM_PRESENT
CPU_DATA  CPU_CntLeadZeros (CPU_DATA  val)
{
    CPU_DATA    nbr_lead_zeros_msb;
    CPU_DATA    nbr_lead_zeros_tbl;
    CPU_DATA    nbr_lead_zeros_tot;
    CPU_INT08U  nbr_shift;
    CPU_INT08U  ix;


#if   (CPU_CFG_DATA_SIZE == CPU_WORD_SIZE_08)                       /* ---------------  8-BIT DATA VAL ---------------- */
                                                                    /* Chk bits [07:00] :            ...                */
    nbr_shift          =  0;                                        /* ... right-shift 'val'  0 bits ...                */
    nbr_lead_zeros_msb =  0;                                        /* ... & nbr msb lead zeros =  0.                   */


#elif (CPU_CFG_DATA_SIZE == CPU_WORD_SIZE_16)                       /* --------------- 16-BIT DATA VAL ---------------- */
    if (val > 0x00FFL) {                                            /* Chk bits [15:08] :            ...                */
        nbr_shift          =  8;                                    /* ... right-shift 'val'  8 bits ...                */
        nbr_lead_zeros_msb =  0;                                    /* ... & nbr msb lead zeros =  0.                   */

    } else {                                                        /* Chk bits [07:00] :            ...                */
        nbr_shift          =  0;                                    /* ... right-shift 'val'  0 bits ...                */
        nbr_lead_zeros_msb =  8;                                    /* ... & nbr msb lead zeros =  0.                   */
    }


#elif (CPU_CFG_DATA_SIZE == CPU_WORD_SIZE_32)                       /* --------------- 32-BIT DATA VAL ---------------- */
    if (val > 0x0000FFFFL) {
        if (val > 0x00FFFFFFL) {                                    /* Chk bits [31:24] :            ...                */
            nbr_shift          = 24;                                /* ... right-shift 'val' 24 bits ...                */
            nbr_lead_zeros_msb =  0;                                /* ... & nbr msb lead zeros =  0.                   */

        } else {                                                    /* Chk bits [23:16] :            ...                */
            nbr_shift          = 16;                                /* ... right-shift 'val' 16 bits ...                */
            nbr_lead_zeros_msb =  8;                                /* ... & nbr msb lead zeros =  8.                   */
        }

    } else {
        if (val > 0x000000FFL) {                                    /* Chk bits [15:08] :            ...                */
            nbr_shift          =  8;                                /* ... right-shift 'val'  8 bits ...                */
            nbr_lead_zeros_msb = 16;                                /* ... & nbr msb lead zeros = 16.                   */

        } else {                                                    /* Chk bits [07:00] :            ...                */
            nbr_shift          =  0;                                /* ... right-shift 'val'  0 bits ...                */
            nbr_lead_zeros_msb = 24;                                /* ... & nbr msb lead zeros = 24.                   */
        }
    }


#else                                                               /* See Note #1a.                                    */
    return (0);
#endif


                                                                    /* ------------- CALC NBR LEAD ZEROS -------------- */
    ix                 = (CPU_INT08U)(val >> nbr_shift) & 0xFF;     /* Calc lookup tbl ix.                              */
    nbr_lead_zeros_tbl = (CPU_DATA  ) CPU_CntLeadZerosTbl[ix];      /* Get  lookup tbl nbr lead zeros.                  */
    nbr_lead_zeros_tot = (CPU_DATA  ) nbr_lead_zeros_msb +          /* Calc        tot nbr lead zeros.                  */
                                      nbr_lead_zeros_tbl;


    return (nbr_lead_zeros_tot);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           CPU_NameInit()
*
* Description : Initialize CPU Name.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
static  void  CPU_NameInit (void)
{
    CPU_NameClr();
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                            CPU_TS_Init()
*
* Description : (1) Initialize CPU timestamp :
*
*                   (a) Initialize/start CPU timestamp timer                            See Note #1
*                   (b) Initialize       CPU timestamp controls
*
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_Init().
*
* Note(s)     : (1) CPU_TS_TmrInit() SHOULD precede calls to all other CPU timestamp functions; 
*                   otherwise, invalid time measurements may be calculated/returned.
*
*                   See also 'CPU_Init()  Note #3a'.
*********************************************************************************************************
*/

#if ((CPU_CFG_TS_TMR_EN == DEF_ENABLED) || \
     (CPU_CFG_TS_EN     == DEF_ENABLED)) 
static  void  CPU_TS_Init (void)
{
#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
    CPU_TS_TmrNbrShifts = CPU_TS_TmrInit();                     /* Init & start ts tmr (see Note #1).                   */
#endif

#if (CPU_CFG_TS_EN == DEF_ENABLED)
    CPU_TS_Lo   = 0;                                            /* Init cur  ts (lo half).                              */
    CPU_TS_Hi   = 0;                                            /* Init cur  ts (hi half).                              */
    CPU_TS_Prev = CPU_TS_TmrRd();                               /* Init prev ts.                                        */
#endif
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       CPU_TS_UpdateHandler()
*
* Description : Update current CPU timestamp. 
*
* Argument(s) : p_ts_lo     Pointer to timestamp variable that will receive the current CPU timestamp's 
*                               lower half (in timestamp timer counts), if available.
*
*               p_ts_hi     Pointer to timestamp variable that will receive the current CPU timestamp's 
*                               upper half (in timestamp timer counts), if available.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_TS_Update(),
*               CPU_TS_Get(),
*               CPU_TS_GetLo().
*
* Note(s)     : (1) After initialization, 'CPU_TS_Lo', 'CPU_TS_Hi', & 'CPU_TS_Prev' MUST ALWAYS be accessed 
*                   AND updated exclusively with interrupts disabled -- but NOT with critical sections.
*
*               (2) CPU timestamp MUST be updated more frequently than timestamp timer overflows; otherwise, 
*                   CPU timestamp will lose time.
*
*                   See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()  Note #2c2'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_EN == DEF_ENABLED)
static  void  CPU_TS_UpdateHandler (CPU_TS  *p_ts_lo,
                                    CPU_TS  *p_ts_hi)
{
    CPU_TS  ts_cur;
    CPU_TS  ts_delta;
    CPU_TS  ts_lo;
    CPU_TS  ts_hi;
    CPU_SR_ALLOC();


    ts_cur     = CPU_TS_TmrRd();                                /* Get   cur   ts (in ts tmr cnts).                     */

    CPU_INT_DIS();
    ts_delta   = ts_cur - CPU_TS_Prev;                          /* Calc  delta ts.                                      */
    ts_delta >>= CPU_TS_TmrNbrShifts;                           /* Shift delta ts tmr cnts.                             */

    CPU_TS_Lo    += ts_delta;                                   /*    Inc lo ts by delta ts tmr cnts.                   */
    if (CPU_TS_Lo < ts_delta) {                                 /* If inc ovf'd, ..                                     */
        CPU_TS_Hi++;                                            /* .. inc hi ts.                                        */
    }

    CPU_TS_Prev = ts_cur;                                       /* Save  cur ts for next update.                        */
    ts_lo       = CPU_TS_Lo;
    ts_hi       = CPU_TS_Hi;
    CPU_INT_EN();

                                                                /* If req'd, rtn ts.                                    */
    if (p_ts_lo != (CPU_TS *)0) {
       *p_ts_lo  = (CPU_TS  )ts_lo;
    }
    if (p_ts_hi != (CPU_TS *)0) {
       *p_ts_hi  = (CPU_TS  )ts_hi;
    }
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CPU_IntDisMeasInit()
*
* Description : (1) Initialize interrupts disabled time measurements feature :
*
*                   (a) Initialize interrupts disabled time measurement controls
*                   (b) Calculate  interrupts disabled time measurement overhead
*
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_Init().
*
* Note(s)     : (2) CPU_IntDisMeasInit() SHOULD precede ALL calls to CPU_CRITICAL_ENTER()/CPU_CRITICAL_EXIT()
*                   & other CPU interrupts disabled time measurement functions; otherwise, invalid interrupts 
*                   disabled time measurements may be calculated/returned.
*
*                   See also 'CPU_Init()  Note #3b'.
*
*               (3) (a) (1) Interrupts disabled time measurement overhead performed multiple times to calculate 
*                           a rounded average with better accuracy, hopefully of +/- one timer count.
*
*                       (2) However, a single overhead time measurement is recommended, even for instruction-
*                           cache-enabled CPUs, since critical sections are NOT typically called within 
*                           instruction-cached loops.  Thus, a single non-cached/non-averaged time measurement 
*                           is a more realistic overhead for the majority of non-cached interrupts disabled 
*                           time measurements.
*
*                   (b) Interrupts MUST be disabled while measuring the interrupts disabled time measurement 
*                       overhead; otherwise, overhead measurements could be interrupted which would incorrectly 
*                       calculate an inflated overhead time which would then incorrectly calculate deflated 
*                       interrupts disabled times.
*********************************************************************************************************
*/

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
static  void  CPU_IntDisMeasInit (void)
{
    CPU_TS      time_meas_tot_cnts;
    CPU_INT16U  i;
    CPU_SR_ALLOC();

                                                                /* ----------- INIT INT DIS TIME MEAS CTRLS ----------- */
    CPU_IntDisNestCtr            = 0;
    CPU_IntDisMeasStartRaw_cnts  = 0;
    CPU_IntDisMeasStopRaw_cnts   = 0;
    CPU_IntDisMeasMaxCurRaw_cnts = 0;
    CPU_IntDisMeasMaxRaw_cnts    = 0;
    CPU_IntDisMeasOvrhdRaw_cnts  = 0;

                                                                /* ----------- CALC INT DIS TIME MEAS OVRHD ----------- */
    time_meas_tot_cnts = 0;
    CPU_INT_DIS();                                              /* Ints MUST be dis'd for ovrhd calc (see Note #3b).    */
    for (i = 0; i < CPU_CFG_INT_DIS_MEAS_OVRHD_NBR; i++) {
        CPU_IntDisMeasMaxCurRaw_cnts = 0;
        CPU_IntDisMeasStart();                                  /* Perform multiple consecutive start/stop time meas's  */
        CPU_IntDisMeasStop();
        time_meas_tot_cnts += CPU_IntDisMeasMaxCurRaw_cnts;     /* ...       & sum time meas max's                  ... */
    }
                                                                /* ... to calc avg time meas ovrhd (see Note #3a).      */
    CPU_IntDisMeasOvrhdRaw_cnts  = (time_meas_tot_cnts + (CPU_CFG_INT_DIS_MEAS_OVRHD_NBR / 2))
                                                       /  CPU_CFG_INT_DIS_MEAS_OVRHD_NBR;
    CPU_IntDisMeasMaxCurRaw_cnts =  0;                          /* Reset max ints dis'd times.                          */
    CPU_IntDisMeasMaxRaw_cnts    =  0;
    CPU_INT_EN();
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       CPU_IntDisMeasMaxCalc()
*
* Description : Calculate maximum interrupts disabled time.
*
* Argument(s) : none.
*
* Return(s)   : Maximum interrupts disabled time (in timestamp timer counts).
*
* Caller(s)   : CPU_IntDisMeasMaxCurGet(),
*               CPU_IntDisMeasMaxGet().
*
* Note(s)     : (1) (a) The total amount of time interrupts are disabled by system &/or application code 
*                       during critical sections is calculated by the following equations :
*
*
*                                               [ time      -  time      ]  -  time
*                                               [     stop         start ]         total meas
*                                               [     meas         meas  ]           ovrhd
*                       (1) time            =  -----------------------------------------------
*                               interrupts                       nbr scaled
*                                disabled                          shifts
*                           (via application)                   2
*
*
*                       (2) time            =  time            +  time
*                               total meas         start meas         stop meas
*                                 ovrhd              ovrhd              ovrhd
*
*
*                               where
*
*                                       time                    time interrupts are disabled between 
*                                           interrupts              first critical section enter & 
*                                            disabled               last  critical section exit minus 
*                                       (via application)           time measurement overhead
*
*                                       time                    time of disable interrupts start time 
*                                           start                   measurement (in raw, scaled timer 
*                                           meas                    counts)
*
*                                       time                    time of disable interrupts stop  time 
*                                           stop                    measurement (in raw, scaled timer 
*                                           meas                    counts)
*
*                                       time                    total overhead time to start/stop disabled 
*                                           total meas              interrupts time measurements (in raw, 
*                                             ovrhd                 scaled timer counts)
*
*                                       time                    total overhead time after getting start 
*                                           start meas              time until end of start measurement 
*                                             ovrhd                 function (in raw, scaled timer counts)
*
*                                       time                    total overhead time from beginning of stop 
*                                           stop meas               measurement function until after getting 
*                                             ovrhd                 stop time (in raw, scaled timer counts)
*
*                                       nbr scaled              number of left bit-shifts, if any, to 
*                                         shifts                    scale timer to (32-bit) 'CPU_TS' 
*                                                                   data type (see also 'cpu_core.h  
*                                                                   CPU_TS_TmrRd()  Note #2a1')
*
*
*                   (b) To expedite & reduce interrupts disabled time measurement overhead, the final 
*                       calculations to subtract the interrupts disabled time measurement overhead AND 
*                       to convert from raw, scaled timer counts to normalized timestamp timer counts 
*                       are performed asynchronously in API functions.
*
*                       See also 'CPU_IntDisMeasStop()  Note #1b2'.
*$PAGE*
*                   (c) The amount of time interrupts are disabled is calculated by either of the 
*                       following equations :
*
*                       (1) Interrupts disabled time  =  Number timer counts  *  Timer period
*
*                               where
*
*                                   Number timer counts             Number of timer counts measured 
*                                   Timer period                    Timer's period in some units of 
*                                                                       (fractional) seconds
*                                   Interrupts disabled time        Amount of time interrupts are 
*                                                                       disabled, in same units of 
*                                                                       (fractional) seconds as the 
*                                                                       Timer period
*
*                                                         Number timer counts
*                       (2) Interrupts disabled time  =  ---------------------
*                                                           Timer frequency
*
*                               where
*
*                                   Number timer counts             Number of timer counts measured 
*                                   Timer frequency                 Timer's frequency in some units 
*                                                                       of counts per second
*                                   Interrupts disabled time        Amount of time interrupts are 
*                                                                       disabled, in seconds
*
*                       See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()    Note #2c'
*                              & 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_to_uSec()  Note #2'.
*
*               (2) Although it is not typical, it is possible for an interrupts disabled time 
*                   measurement to be less than the interrupts disabled time measurement overhead; 
*                   especially if the overhead was calculated with a single, non-cached measurement 
*                   & critical sections are called within instruction-cached loops.
*********************************************************************************************************
*/

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
static  CPU_TS  CPU_IntDisMeasMaxCalc (CPU_TS  time_max_cnts_raw)
{
    CPU_TS  time_max_cnts;


    time_max_cnts = time_max_cnts_raw;
    if (time_max_cnts >  CPU_IntDisMeasOvrhdRaw_cnts) {         /* If       max ints dis'd time >  ovrhd time, ...      */
        time_max_cnts -= CPU_IntDisMeasOvrhdRaw_cnts;           /* ... adj  max ints dis'd time by ovrhd time; ...      */
    } else {                                                    /* ... else max ints dis'd time <  ovrhd time, ...      */
        time_max_cnts  = 0;                                     /* ... clr  max ints dis'd time (see Note #2).          */
    }

    time_max_cnts >>= CPU_TS_TmrNbrShifts;                      /* Shift    max ints dis'd time.                        */

    return (time_max_cnts);
}
#endif

