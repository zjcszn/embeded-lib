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
*                                       ASCII STRING MANAGEMENT
*
* Filename      : lib_str.c
* Version       : V1.30
* Programmer(s) : ITJ
*                 BAN
*                 JDH
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
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    LIB_STR_MODULE
#include  <lib_str.h>


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

static  const  CPU_INT32U  Str_MultOvfThTbl_Int32U[] = {
   (CPU_INT32U) DEF_INT_32U_MAX_VAL,                /*                Invalid base  0.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  1)),         /*                Invalid base  1.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  2)),         /* 32-bit mult ovf th for base  2.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  3)),         /* 32-bit mult ovf th for base  3.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  4)),         /* 32-bit mult ovf th for base  4.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  5)),         /* 32-bit mult ovf th for base  5.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  6)),         /* 32-bit mult ovf th for base  6.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  7)),         /* 32-bit mult ovf th for base  7.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  8)),         /* 32-bit mult ovf th for base  8.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL /  9)),         /* 32-bit mult ovf th for base  9.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 10)),         /* 32-bit mult ovf th for base 10.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 11)),         /* 32-bit mult ovf th for base 11.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 12)),         /* 32-bit mult ovf th for base 12.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 13)),         /* 32-bit mult ovf th for base 13.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 14)),         /* 32-bit mult ovf th for base 14.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 15)),         /* 32-bit mult ovf th for base 15.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 16)),         /* 32-bit mult ovf th for base 16.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 17)),         /* 32-bit mult ovf th for base 17.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 18)),         /* 32-bit mult ovf th for base 18.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 19)),         /* 32-bit mult ovf th for base 19.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 20)),         /* 32-bit mult ovf th for base 20.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 21)),         /* 32-bit mult ovf th for base 21.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 22)),         /* 32-bit mult ovf th for base 22.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 23)),         /* 32-bit mult ovf th for base 23.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 24)),         /* 32-bit mult ovf th for base 24.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 25)),         /* 32-bit mult ovf th for base 25.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 26)),         /* 32-bit mult ovf th for base 26.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 27)),         /* 32-bit mult ovf th for base 27.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 28)),         /* 32-bit mult ovf th for base 28.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 29)),         /* 32-bit mult ovf th for base 29.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 30)),         /* 32-bit mult ovf th for base 30.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 31)),         /* 32-bit mult ovf th for base 31.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 32)),         /* 32-bit mult ovf th for base 32.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 33)),         /* 32-bit mult ovf th for base 33.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 34)),         /* 32-bit mult ovf th for base 34.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 35)),         /* 32-bit mult ovf th for base 35.      */
  ((CPU_INT32U)(DEF_INT_32U_MAX_VAL / 36))          /* 32-bit mult ovf th for base 36.      */
};


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  CPU_CHAR    *Str_FmtNbr_Int32  (CPU_INT32U     nbr,
                                        CPU_INT08U     nbr_dig,
                                        CPU_INT08U     nbr_base,
                                        CPU_BOOLEAN    nbr_neg,
                                        CPU_CHAR       lead_char,
                                        CPU_BOOLEAN    lower_case,
                                        CPU_BOOLEAN    nul,
                                        CPU_CHAR      *pstr);

static  CPU_INT32U   Str_ParseNbr_Int32(CPU_CHAR      *pstr,
                                        CPU_CHAR     **pstr_next,
                                        CPU_INT08U     nbr_base,
                                        CPU_BOOLEAN    nbr_signed,
                                        CPU_BOOLEAN   *pnbr_neg);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                              Str_Len()
*
* Description : Calculate length of a string.
*
* Argument(s) : pstr        Pointer to string (see Note #1).
*
* Return(s)   : Length of string; number of characters in string before terminating NULL character.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffer NOT modified.
*
*               (2) String length calculation terminates when :
*
*                   (a) String pointer points to NULL.
*                       (1) String buffer overlaps with NULL address.
*                       (2) String length calculated for string up to but NOT beyond or including
*                           the NULL address.
*
*                   (b) Terminating NULL character found.
*                       (1) String length calculated for string up to but NOT           including
*                           the NULL character.
*********************************************************************************************************
*/

CPU_SIZE_T  Str_Len (CPU_CHAR  *pstr)
{
    CPU_SIZE_T  len;


    len = Str_Len_N((CPU_CHAR *)pstr,
                    (CPU_SIZE_T)DEF_INT_CPU_U_MAX_VAL);

    return (len);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             Str_Len_N()
*
* Description : Calculate length of a string, up to a maximum number of characters.
*
* Argument(s) : pstr        Pointer to string (see Note #1).
*
*               len_max     Maximum number of characters to search (see Note #2c).
*
* Return(s)   : Length of string; number of characters in string before terminating NULL character, 
*                   if terminating NULL character     found.
*               
*               Requested maximum number of characters to search, 
*                   if terminating NULL character NOT found.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffer NOT modified.
*
*               (2) String length calculation terminates when :
*
*                   (a) String pointer points to NULL.
*                       (1) String buffer overlaps with NULL address.
*                       (2) String length calculated for string up to but NOT beyond or including
*                           the NULL address.
*
*                   (b) Terminating NULL character found.
*                       (1) String length calculated for string up to but NOT           including
*                           the NULL character.
*
*                   (c) 'len_max' number of characters searched.
*                       (1) 'len_max' number of characters does NOT include the terminating NULL character.
*********************************************************************************************************
*/

CPU_SIZE_T  Str_Len_N (CPU_CHAR    *pstr,
                       CPU_SIZE_T   len_max)
{
    CPU_SIZE_T  len;


    len = 0;
    while (( pstr != (CPU_CHAR *)  0 ) &&                       /* Calc str len until NULL ptr (see Note #2a) ...       */
           (*pstr != (CPU_CHAR  )'\0') &&                       /* ... or NULL char found      (see Note #2b) ...       */
           ( len   < (CPU_SIZE_T)len_max)) {                    /* ... or max nbr chars srch'd (see Note #2c).          */
        len++;
        pstr++;
    }

    return (len);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             Str_Copy()
*
* Description : Copy source string to destination string buffer.
*
* Argument(s) : pdest       Pointer to destination string buffer to receive source string copy (see Note #1).
*
*               psrc        Pointer to source      string to copy into destination string buffer.
*
* Return(s)   : Pointer to destination string, if NO errors (see Note #2).
*
*               Pointer to NULL,               otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Destination buffer size NOT validated; buffer overruns MUST be prevented by caller.
*
*                   (a) Destination buffer size MUST be large enough to accommodate the entire source
*                       string size including the terminating NULL character.
*
*               (2) String copy terminates when :
*
*                   (a) Destination/Source string pointer(s) are passed NULL pointers.
*                       (1) No string copy performed; NULL pointer returned.
*
*                   (b) Destination/Source string pointer(s) points to NULL.
*                       (1) String buffer(s) overlap with NULL address.
*                       (2) Source string copied into destination string buffer up to but NOT beyond or
*                           including the NULL address; destination string buffer properly terminated
*                           with NULL character.
*
*                   (c) Source string's terminating NULL character found.
*                       (1) Entire source string copied into destination string buffer.
*********************************************************************************************************
*/

CPU_CHAR  *Str_Copy (CPU_CHAR  *pdest,
                     CPU_CHAR  *psrc)
{
    pdest = Str_Copy_N((CPU_CHAR *)pdest,
                       (CPU_CHAR *)psrc,
                       (CPU_SIZE_T)DEF_INT_CPU_U_MAX_VAL);

    return (pdest);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            Str_Copy_N()
*
* Description : Copy source string to destination string buffer, up to a maximum number of characters.
*
* Argument(s) : pdest       Pointer to destination string buffer to receive source string copy (see Note #1).
*
*               psrc        Pointer to source      string to copy into destination string buffer.
*
*               len_max     Maximum number of characters to copy (see Note #2d).
*
* Return(s)   : Pointer to destination string, if NO errors (see Note #2).
*
*               Pointer to NULL,               otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Destination buffer size NOT validated; buffer overruns MUST be prevented by caller.
*
*                   (a) Destination buffer size MUST be large enough to accommodate the entire source
*                       string size including the terminating NULL character.
*
*               (2) String copy terminates when :
*
*                   (a) Destination/Source string pointer(s) are passed NULL pointers.
*                       (1) No string copy performed; NULL pointer returned.
*
*                   (b) Destination/Source string pointer(s) points to NULL.
*                       (1) String buffer(s) overlap with NULL address.
*                       (2) Source string copied into destination string buffer up to but NOT beyond or
*                           including the NULL address; destination string buffer properly terminated
*                           with NULL character.
*
*                   (c) Source string's terminating NULL character found.
*                       (1) Entire source string copied into destination string buffer.
*
*                   (d) 'len_max' number of characters copied.
*                       (1) 'len_max' number of characters does NOT include the terminating NULL character.
*
*                           See also Note #1a.
*********************************************************************************************************
*/

CPU_CHAR  *Str_Copy_N (CPU_CHAR    *pdest,
                       CPU_CHAR    *psrc,
                       CPU_SIZE_T   len_max)
{
    CPU_CHAR    *pstr;
    CPU_CHAR    *pstr_next;
    CPU_SIZE_T   len_copy;

                                                                /* Rtn NULL if str ptr(s) NULL      (see Note #2a).     */
    if (pdest == (CPU_CHAR *)0) {
        return  ((CPU_CHAR *)0);
    }
    if (psrc  == (CPU_CHAR *)0) {
        return  ((CPU_CHAR *)0);
    }

    if (len_max == (CPU_SIZE_T)0) {                             /* Rtn NULL if copy len equals zero (see Note #2d).     */
        return  ((CPU_CHAR *)0);
    }


    pstr      = pdest;
    pstr_next = pstr;
    pstr_next++;
    len_copy  = 0;

    while (( pstr_next != (CPU_CHAR *)  0 ) &&                  /* Copy str until NULL ptr(s)  [see Note #2b]  ...      */
           ( psrc      != (CPU_CHAR *)  0 ) &&
           (*psrc      != (CPU_CHAR  )'\0') &&                  /* ... or NULL char found      (see Note #2c); ...      */
           ( len_copy  <  (CPU_SIZE_T)len_max)) {               /* ... or max nbr chars copied (see Note #2d).          */
       *pstr = *psrc;
        pstr++;
        pstr_next++;
        psrc++;
        len_copy++;
    }

   *pstr = (CPU_CHAR)'\0';                                      /* Append NULL char (see Note #2b2).                    */


    return (pdest);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                              Str_Cat()
*
* Description : Append concatenation string to destination string.
*
* Argument(s) : pdest       Pointer to destination   string to append concatenation  string (see Note #1).
*
*               pstr_cat    Pointer to concatenation string to append to destination string.
*
* Return(s)   : Pointer to destination string, if NO errors (see Note #2).
*
*               Pointer to NULL,               otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Destination string buffer size NOT validated; buffer overruns MUST be prevented by caller.
*
*                   (a) Destination buffer size MUST be large enough to accommodate the entire concatenated
*                       string size including the terminating NULL character.
*
*               (2) String concatenation terminates when :
*
*                   (a) Destination/Concatenation string pointer(s) are passed NULL pointers.
*                       (1) No string concatenation performed; NULL pointer returned.
*
*                   (b) Destination string overlaps with NULL address.
*                       (1) No string concatenation performed; NULL pointer returned.
*
*                   (c) Destination/Concatenation string pointer(s) points to NULL.
*                       (1) String buffer(s) overlap with NULL address.
*                       (2) Concatenation string appended into destination string buffer up to but NOT
*                           beyond or including the NULL address; destination string buffer properly
*                           terminated with NULL character.
*
*                   (d) Concatenation string's terminating NULL character found.
*                       (1) Entire concatenation string appended to destination string.
*********************************************************************************************************
*/

CPU_CHAR  *Str_Cat (CPU_CHAR  *pdest,
                    CPU_CHAR  *pstr_cat)
{
    pdest = Str_Cat_N((CPU_CHAR *)pdest,
                      (CPU_CHAR *)pstr_cat,
                      (CPU_SIZE_T)DEF_INT_CPU_U_MAX_VAL);

    return (pdest);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             Str_Cat_N()
*
* Description : Append concatenation string to destination string, up to a maximum number of characters.
*
* Argument(s) : pdest       Pointer to destination   string to append concatenation  string (see Note #1).
*
*               pstr_cat    Pointer to concatenation string to append to destination string.
*
*               len_max     Maximum number of characters to concatenate (see Note #2e).
*
* Return(s)   : Pointer to destination string, if NO errors (see Note #2).
*
*               Pointer to NULL,               otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Destination string buffer size NOT validated; buffer overruns MUST be prevented by caller.
*
*                   (a) Destination buffer size MUST be large enough to accommodate the entire concatenated
*                       string size including the terminating NULL character.
*
*               (2) String concatenation terminates when :
*
*                   (a) Destination/Concatenation string pointer(s) are passed NULL pointers.
*                       (1) No string concatenation performed; NULL pointer returned.
*
*                   (b) Destination string overlaps with NULL address.
*                       (1) No string concatenation performed; NULL pointer returned.
*
*                   (c) Destination/Concatenation string pointer(s) points to NULL.
*                       (1) String buffer(s) overlap with NULL address.
*                       (2) Concatenation string appended into destination string buffer up to but NOT
*                           beyond or including the NULL address; destination string buffer properly
*                           terminated with NULL character.
*
*                   (d) Concatenation string's terminating NULL character found.
*                       (1) Entire concatenation string appended to destination string.
*
*                   (e) 'len_max' number of characters concatenated.
*                       (1) 'len_max' number of characters does NOT include the terminating NULL character.
*
*                           See also Note #1a.
*********************************************************************************************************
*/
/*$PAGE*/
CPU_CHAR  *Str_Cat_N (CPU_CHAR    *pdest,
                      CPU_CHAR    *pstr_cat,
                      CPU_SIZE_T   len_max)
{
    CPU_CHAR    *pstr;
    CPU_CHAR    *pstr_next;
    CPU_SIZE_T   len_cat;

                                                                /* Rtn NULL if str ptr(s) NULL     (see Note #2a).      */
    if (pdest == (CPU_CHAR *)0) {
        return  ((CPU_CHAR *)0);
    }
    if (pstr_cat == (CPU_CHAR *)0) {
        return  ((CPU_CHAR *)0);
    }

    if (len_max == (CPU_SIZE_T)0) {                             /* Rtn NULL if cat len equals zero (see Note #2e).      */
        return  ((CPU_CHAR *)0);
    }


    pstr = pdest;
    while (( pstr != (CPU_CHAR *)  0 ) &&                       /* Adv to end of cur dest str until NULL ptr ...        */
           (*pstr != (CPU_CHAR  )'\0')) {                       /* ... or NULL char found..                             */
        pstr++;
    }
    if (pstr == (CPU_CHAR *)0) {                                /* If NULL str overrun, rtn NULL (see Note #2b).        */
        return ((CPU_CHAR *)0);
    }

    pstr_next = pstr;
    pstr_next++;
    len_cat   = 0;

    while (( pstr_next != (CPU_CHAR *)  0 ) &&                  /* Cat str until NULL ptr(s)  [see Note #2c]  ...       */
           ( pstr_cat  != (CPU_CHAR *)  0 ) &&
           (*pstr_cat  != (CPU_CHAR  )'\0') &&                  /* ... or NULL char found     (see Note #2d); ...       */
           ( len_cat   <  (CPU_SIZE_T)len_max)) {               /* ... or max nbr chars cat'd (see Note #2d).           */
       *pstr = *pstr_cat;
        pstr++;
        pstr_next++;
        pstr_cat++;
        len_cat++;
    }

   *pstr = (CPU_CHAR)'\0';                                      /* Append NULL char (see Note #2c2).                    */


    return (pdest);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                              Str_Cmp()
*
* Description : Determine if two strings are identical.
*
* Argument(s) : p1_str      Pointer to first  string (see Note #1).
*
*               p2_str      Pointer to second string (see Note #1).
*
* Return(s)   : 0,              if strings are identical             (see Notes #3a1A, #3a2A, #3b, & #3d).
*
*               Negative value, if 'p1_str' is less    than 'p2_str' (see Notes #3a1B1, #3a2B1, & #3c).
*
*               Positive value, if 'p1_str' is greater than 'p2_str' (see Notes #3a1B2, #3a2B2, & #3c).
*
*               See also Note #2b.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffers NOT modified.
*
*               (2) (a) IEEE Std 1003.1, 2004 Edition, Section 'strcmp() : DESCRIPTION' states that "the
*                       strcmp() function shall compare the string pointed to by s1 to the string pointed
*                       to by s2".
*
*                   (b) (1) IEEE Std 1003.1, 2004 Edition, Section 'strcmp() : RETURN VALUE' states that
*                          "upon successful completion, strncmp() shall return an integer greater than,
*                           equal to, or less than 0".
*
*                       (2) IEEE Std 1003.1, 2004 Edition, Section 'strcmp() : DESCRIPTION' adds that "the
*                           sign of a non-zero return value shall be determined by the sign of the difference
*                           between the values of the first pair of bytes ... that differ in the strings
*                           being compared".
*
*               (3) String comparison terminates when :
*
*                   (a) (1) (A) BOTH string pointer(s) are passed NULL pointers.
*                               (1) NULL strings identical; return 0.
*
*                           (B) (1) 'p1_str' passed a NULL pointer.
*                                   (a) Return negative value of character pointed to by 'p2_str'.
*
*                               (2) 'p2_str' passed a NULL pointer.
*                                   (a) Return positive value of character pointed to by 'p1_str'.
*
*                       (2) (A) BOTH strings point to NULL.
*                               (1) Strings overlap with NULL address.
*                               (2) Strings identical up to but NOT beyond or including the NULL address;
*                                   return 0.
*
*                           (B) (1) 'p1_str_next' points to NULL.
*                                   (a) 'p1_str' overlaps with NULL address.
*                                   (b) Strings compared up to but NOT beyond or including the NULL address.
*                                   (c) Return negative value of character pointed to by 'p2_str_next'.
*
*                               (2) 'p2_str_next' points to NULL.
*                                   (a) 'p2_str' overlaps with NULL address.
*                                   (b) Strings compared up to but NOT beyond or including the NULL address.
*                                   (c) Return positive value of character pointed to by 'p1_str_next'.
*
*                   (b) Terminating NULL character found in both strings.
*                       (1) Strings identical; return 0.
*                       (2) Only one NULL character test required in conditional since previous condition
*                           tested character equality.
*
*                   (c) Non-matching characters found.
*                       (1) Return signed-integer difference of the character pointed to by 'p2_str'
*                           from the character pointed to by 'p1_str'.
*
*               (4) Since 16-bit signed arithmetic is performed to calculate a non-identical comparison
*                   return value, 'CPU_CHAR' native data type size MUST be 8-bit.
*********************************************************************************************************
*/

CPU_INT16S  Str_Cmp (CPU_CHAR  *p1_str,
                     CPU_CHAR  *p2_str)
{
    CPU_INT16S  cmp_val;


    cmp_val = Str_Cmp_N((CPU_CHAR *)p1_str,
                        (CPU_CHAR *)p2_str,
                        (CPU_SIZE_T)DEF_INT_CPU_U_MAX_VAL);

    return (cmp_val);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             Str_Cmp_N()
*
* Description : Determine if two strings are identical for up to a maximum number of characters.
*
* Argument(s) : p1_str      Pointer to first  string (see Note #1).
*
*               p2_str      Pointer to second string (see Note #1).
*
*               len_max     Maximum number of characters to compare  (see Note  #3d).
*
* Return(s)   : 0,              if strings are identical             (see Notes #3a1A, #3a2A, #3b, & #3d).
*
*               Negative value, if 'p1_str' is less    than 'p2_str' (see Notes #3a1B1, #3a2B1, & #3c).
*
*               Positive value, if 'p1_str' is greater than 'p2_str' (see Notes #3a1B2, #3a2B2, & #3c).
*
*               See also Note #2b.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffers NOT modified.
*
*               (2) (a) IEEE Std 1003.1, 2004 Edition, Section 'strncmp() : DESCRIPTION' states that :
*
*                       (1) "The strncmp() function shall compare ... the array pointed to by s1 to the
*                            array pointed to by s2" but ...
*                       (2) "not more than 'n' bytes" of either array.
*
*                   (b) (1) IEEE Std 1003.1, 2004 Edition, Section 'strncmp() : RETURN VALUE' states that
*                          "upon successful completion, strncmp() shall return an integer greater than,
*                           equal to, or less than 0".
*
*                       (2) IEEE Std 1003.1, 2004 Edition, Section 'strncmp() : DESCRIPTION' adds that
*                          "the sign of a non-zero return value is determined by the sign of the difference
*                           between the values of the first pair of bytes ... that differ in the strings
*                           being compared".
*
*               (3) String comparison terminates when :
*
*                   (a) (1) (A) BOTH string pointer(s) are passed NULL pointers.
*                               (1) NULL strings identical; return 0.
*
*                           (B) (1) 'p1_str' passed a NULL pointer.
*                                   (a) Return negative value of character pointed to by 'p2_str'.
*
*                               (2) 'p2_str' passed a NULL pointer.
*                                   (a) Return positive value of character pointed to by 'p1_str'.
*
*                       (2) (A) BOTH strings point to NULL.
*                               (1) Strings overlap with NULL address.
*                               (2) Strings identical up to but NOT beyond or including the NULL address;
*                                   return 0.
*
*                           (B) (1) 'p1_str_next' points to NULL.
*                                   (a) 'p1_str' overlaps with NULL address.
*                                   (b) Strings compared up to but NOT beyond or including the NULL address.
*                                   (c) Return negative value of character pointed to by 'p2_str_next'.
*
*                               (2) 'p2_str_next' points to NULL.
*                                   (a) 'p2_str' overlaps with NULL address.
*                                   (b) Strings compared up to but NOT beyond or including the NULL address.
*                                   (c) Return positive value of character pointed to by 'p1_str_next'.
*
*                   (b) Terminating NULL character found in both strings.
*                       (1) Strings identical; return 0.
*                       (2) Only one NULL character test required in conditional since previous condition
*                           tested character equality.
*
*                   (c) Non-matching characters found.
*                       (1) Return signed-integer difference of the character pointed to by 'p2_str'
*                           from the character pointed to by 'p1_str'.
*
*                   (d) (1) 'len_max' passed a zero length.
*                           (A) Zero-length strings identical; return 0.
*
*                       (2) First 'len_max' number of characters identical.
*                           (A) Strings identical; return 0.
*
*                       See also Note #2a2.
*
*               (4) Since 16-bit signed arithmetic is performed to calculate a non-identical comparison
*                   return value, 'CPU_CHAR' native data type size MUST be 8-bit.
*********************************************************************************************************
*/
/*$PAGE*/
CPU_INT16S  Str_Cmp_N (CPU_CHAR    *p1_str,
                       CPU_CHAR    *p2_str,
                       CPU_SIZE_T   len_max)
{
    CPU_CHAR    *p1_str_next;
    CPU_CHAR    *p2_str_next;
    CPU_INT16S   cmp_val;
    CPU_SIZE_T   cmp_len;


    if (len_max == 0) {                                         /* If cmp len equals zero, rtn 0      (see Note #3d1).  */
        return ((CPU_INT16S)0);
    }

    if (p1_str == (CPU_CHAR *)0) {
        if (p2_str == (CPU_CHAR *)0) {
            return ((CPU_INT16S)0);                             /* If BOTH str ptrs NULL,  rtn 0      (see Note #3a1A). */
        }
        cmp_val = (CPU_INT16S)0 - (CPU_INT16S)(*p2_str);
        return (cmp_val);                                       /* If p1_str NULL, rtn neg p2_str val (see Note #3a1B1).*/
    }
    if (p2_str == (CPU_CHAR *)0) {
        cmp_val = (CPU_INT16S)(*p1_str);
        return (cmp_val);                                       /* If p2_str NULL, rtn pos p1_str val (see Note #3a1B2).*/
    }


    p1_str_next = p1_str;
    p2_str_next = p2_str;
    p1_str_next++;
    p2_str_next++;
    cmp_len     = 0;
    while ((*p1_str      == *p2_str)          &&                /* Cmp strs until non-matching chars (see Note #3c) ... */
           (*p1_str      != (CPU_CHAR  )'\0') &&                /* ... or NULL chars                 (see Note #3b) ... */
           ( p1_str_next != (CPU_CHAR *)  0 ) &&                /* ... or NULL ptr(s) found          (see Note #3a2);   */
           ( p2_str_next != (CPU_CHAR *)  0 ) &&
           ( cmp_len     <  (CPU_SIZE_T)len_max)) {             /* ... or max nbr chars cmp'd        (see Note #3d2).   */
        p1_str_next++;
        p2_str_next++;
        p1_str++;
        p2_str++;
        cmp_len++;
    }


    if (cmp_len == len_max) {                                       /* If strs     identical for len nbr of chars, ...  */
        return ((CPU_INT16S)0);                                     /* ... rtn 0                   (see Note #3d2).     */
    }

    if (*p1_str != *p2_str) {                                       /* If strs NOT identical, ...                       */
         cmp_val = (CPU_INT16S)(*p1_str) - (CPU_INT16S)(*p2_str);   /* ... calc & rtn char diff    (see Note #3c).      */

    } else if (*p1_str  == (CPU_CHAR)'\0') {                        /* If NULL char(s) found, ...                       */
         cmp_val = 0;                                               /* ... strs identical; rtn 0   (see Note #3b).      */

    } else {
        if (p1_str_next == (CPU_CHAR *)0) {
            if (p2_str_next == (CPU_CHAR *)0) {                     /* If BOTH next str ptrs NULL, ...                  */
                cmp_val  = (CPU_INT16S)0;                           /* ... rtn 0                   (see Note #3a2A).    */
            } else {                                                /* If p1_str_next NULL, ...                         */
                                                                    /* ... rtn neg p2_str_next val (see Note #3a2B1).   */
                cmp_val  = (CPU_INT16S)0 - (CPU_INT16S)(*p2_str_next);
            }
        } else {                                                    /* If p2_str_next NULL, ...                         */
            cmp_val = (CPU_INT16S)(*p1_str_next);                   /* ... rtn pos p1_str_next val (see Note #3a2B2).   */
        }
    }


    return (cmp_val);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         Str_CmpIgnoreCase()
*
* Description : Determine if two strings are identical, ignoring case.
*
* Argument(s) : p1_str      Pointer to first  string (see Note #1).
*
*               p2_str      Pointer to second string (see Note #1).
*
* Return(s)   : 0,              if strings are identical             (see Notes #3a1A, #3a2A, #3b, & #3d).
*
*               Negative value, if 'p1_str' is less    than 'p2_str' (see Notes #3a1B1, #3a2B1, & #3c).
*
*               Positive value, if 'p1_str' is greater than 'p2_str' (see Notes #3a1B2, #3a2B2, & #3c).
*
*               See also Note #2b.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffers NOT modified.
*
*               (2) (a) IEEE Std 1003.1, 2004 Edition, Section 'strcasecmp() : DESCRIPTION' states that :
*
*                       (1) (A) "The strcasecmp() function shall compare ... the string pointed to by s1
*                                to the string pointed to by s2" ...
*                           (B) "ignoring differences in case".
*
*                       (2) "strcasecmp() ... shall behave as if the strings had been converted to lowercase
*                            and then a byte comparison performed."
*
*                   (b) (1) IEEE Std 1003.1, 2004 Edition, Section 'strcasecmp() : RETURN VALUE' states that
*                          "upon successful completion, strcasecmp() shall return an integer greater than,
*                           equal to, or less than 0".
*
*                       (2) IEEE Std 1003.1, 2004 Edition, Section 'strcmp() : DESCRIPTION' adds that "the
*                           sign of a non-zero return value shall be determined by the sign of the difference
*                           between the values of the first pair of bytes ... that differ in the strings
*                           being compared".
*
*               (3) String comparison terminates when :
*
*                   (a) (1) (A) BOTH string pointer(s) are passed NULL pointers.
*                               (1) NULL strings identical; return 0.
*
*                           (B) (1) 'p1_str' passed a NULL pointer.
*                                   (a) Return negative value of character pointed to by 'p2_str', converted
*                                       to lower case (see Note #2a2).
*
*                               (2) 'p2_str' passed a NULL pointer.
*                                   (a) Return positive value of character pointed to by 'p1_str', converted
*                                       to lower case (see Note #2a2).
*
*                       (2) (A) BOTH strings point to NULL.
*                               (1) Strings overlap with NULL address.
*                               (2) Strings identical up to but NOT beyond or including the NULL address;
*                                   return 0.
*
*                           (B) (1) 'p1_str_next' points to NULL.
*                                   (a) 'p1_str' overlaps with NULL address.
*                                   (b) Strings compared up to but NOT beyond or including the NULL address.
*                                   (c) Return negative value of character pointed to by 'p2_str_next',
*                                       converted to lower case (see Note #2a2).
*
*                               (2) 'p2_str_next' points to NULL.
*                                   (a) 'p2_str' overlaps with NULL address.
*                                   (b) Strings compared up to but NOT beyond or including the NULL address.
*                                   (c) Return positive value of character pointed to by 'p1_str_next',
*                                       converted to lower case (see Note #2a2).
*
*                   (b) Terminating NULL character found in both strings.
*                       (1) Strings identical; return 0.
*                       (2) Only one NULL character test required in conditional since previous condition
*                           tested character equality.
*
*                   (c) Non-matching characters found.
*                       (1) Return signed-integer difference of the character pointed to by 'p2_str',
*                           converted to lower case, from the character pointed to by 'p1_str', converted
*                           to lower case.
*
*               (4) Since 16-bit signed arithmetic is performed to calculate a non-identical comparison
*                   return value, 'CPU_CHAR' native data type size MUST be 8-bit.
*********************************************************************************************************
*/
/*$PAGE*/
CPU_INT16S  Str_CmpIgnoreCase (CPU_CHAR  *p1_str,
                               CPU_CHAR  *p2_str)
{
    CPU_INT16S  cmp_val;


    cmp_val = Str_CmpIgnoreCase_N((CPU_CHAR *)p1_str,
                                  (CPU_CHAR *)p2_str,
                                  (CPU_SIZE_T)DEF_INT_CPU_U_MAX_VAL);

    return (cmp_val);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        Str_CmpIgnoreCase_N()
*
* Description : Determine if two strings are identical for up to a maximum number of characters,
*                   ignoring case.
*
* Argument(s) : p1_str      Pointer to first  string (see Note #1).
*
*               p2_str      Pointer to second string (see Note #1).
*
*               len_max     Maximum number of characters to compare  (see Note  #3d).
*
* Return(s)   : 0,              if strings are identical             (see Notes #3a1A, #3a2A, #3b, & #3d).
*
*               Negative value, if 'p1_str' is less    than 'p2_str' (see Notes #3a1B1, #3a2B1, & #3c).
*
*               Positive value, if 'p1_str' is greater than 'p2_str' (see Notes #3a1B2, #3a2B2, & #3c).
*
*               See also Note #2b.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffers NOT modified.
*
*               (2) (a) IEEE Std 1003.1, 2004 Edition, Section 'strncasecmp() : DESCRIPTION' states that :
*
*                       (1) (A) "The strncasecmp() function shall compare ... the string pointed to by s1
*                                to the string pointed to by s2" ...
*                           (B) "ignoring differences in case" & ...
*                           (C) "not more than 'n' bytes" of either string.
*
*                       (2) "strncasecmp() shall behave as if the strings had been converted to lowercase
*                            and then a byte comparison performed."
*
*                   (b) (1) IEEE Std 1003.1, 2004 Edition, Section 'strncasecmp() : RETURN VALUE' states that
*                          "upon successful completion, strncasecmp() shall return an integer greater than,
*                           equal to, or less than 0".
*
*                       (2) IEEE Std 1003.1, 2004 Edition, Section 'strcmp() : DESCRIPTION' adds that "the
*                           sign of a non-zero return value shall be determined by the sign of the difference
*                           between the values of the first pair of bytes ... that differ in the strings
*                           being compared".
*
*               (3) String comparison terminates when :
*
*                   (a) (1) (A) BOTH string pointer(s) are passed NULL pointers.
*                               (1) NULL strings identical; return 0.
*
*                           (B) (1) 'p1_str' passed a NULL pointer.
*                                   (a) Return negative value of character pointed to by 'p2_str', converted
*                                       to lower case (see Note #2a2).
*
*                               (2) 'p2_str' passed a NULL pointer.
*                                   (a) Return positive value of character pointed to by 'p1_str', converted
*                                       to lower case (see Note #2a2).
*
*                       (2) (A) BOTH strings point to NULL.
*                               (1) Strings overlap with NULL address.
*                               (2) Strings identical up to but NOT beyond or including the NULL address;
*                                   return 0.
*
*                           (B) (1) 'p1_str_next' points to NULL.
*                                   (a) 'p1_str' overlaps with NULL address.
*                                   (b) Strings compared up to but NOT beyond or including the NULL address.
*                                   (c) Return negative value of character pointed to by 'p2_str_next',
*                                       converted to lower case (see Note #2a2).
*
*                               (2) 'p2_str_next' points to NULL.
*                                   (a) 'p2_str' overlaps with NULL address.
*                                   (b) Strings compared up to but NOT beyond or including the NULL address.
*                                   (c) Return positive value of character pointed to by 'p1_str_next',
*                                       converted to lower case (see Note #2a2).
*
*                   (b) Terminating NULL character found in both strings.
*                       (1) Strings identical; return 0.
*                       (2) Only one NULL character test required in conditional since previous condition
*                           tested character equality.
*
*                   (c) Non-matching characters found.
*                       (1) Return signed-integer difference of the character pointed to by 'p2_str',
*                           converted to lower case, from the character pointed to by 'p1_str', converted
*                           to lower case.
*
*                   (d) (1) 'len_max' passed a zero length.
*                           (A) Zero-length strings identical; return 0.
*
*                       (2) First 'len_max' number of characters identical.
*                           (A) Strings identical; return 0.
*
*                       See also Note #2a1C.
*$PAGE*
*               (4) Since 16-bit signed arithmetic is performed to calculate a non-identical comparison
*                   return value, 'CPU_CHAR' native data type size MUST be 8-bit.
*********************************************************************************************************
*/

CPU_INT16S  Str_CmpIgnoreCase_N (CPU_CHAR    *p1_str,
                                 CPU_CHAR    *p2_str,
                                 CPU_SIZE_T   len_max)
{
    CPU_CHAR    *p1_str_next;
    CPU_CHAR    *p2_str_next;
    CPU_CHAR     char1;
    CPU_CHAR     char2;
    CPU_INT16S   cmp_val;
    CPU_SIZE_T   cmp_len;


    if (len_max == 0) {                                         /* If cmp len equals zero, rtn 0      (see Note #3d1).  */
        return ((CPU_INT16S)0);
    }

    if (p1_str == (CPU_CHAR *)0) {
        if (p2_str == (CPU_CHAR *)0) {
            return ((CPU_INT16S)0);                             /* If BOTH str ptrs NULL,  rtn 0      (see Note #3a1A). */
        }
        char2   =  ASCII_TO_LOWER(*p2_str);
        cmp_val = (CPU_INT16S)0 - (CPU_INT16S)char2;
        return (cmp_val);                                       /* If p1_str NULL, rtn neg p2_str val (see Note #3a1B1).*/
    }
    if (p2_str == (CPU_CHAR *)0) {
        char1   =  ASCII_TO_LOWER(*p1_str);
        cmp_val = (CPU_INT16S)char1;
        return (cmp_val);                                       /* If p2_str NULL, rtn pos p1_str val (see Note #3a1B2).*/
    }


    p1_str_next = p1_str;
    p2_str_next = p2_str;
    p1_str_next++;
    p2_str_next++;
    char1       = ASCII_TO_LOWER(*p1_str);
    char2       = ASCII_TO_LOWER(*p2_str);
    cmp_len     = 0;
    while (( char1       ==  char2)           &&                /* Cmp strs until non-matching chars (see Note #3c) ... */
           (*p1_str      != (CPU_CHAR  )'\0') &&                /* ... or NULL chars                 (see Note #3b) ... */
           ( p1_str_next != (CPU_CHAR *)  0 ) &&                /* ... or NULL ptr(s) found          (see Note #3a2);   */
           ( p2_str_next != (CPU_CHAR *)  0 ) &&
           ( cmp_len     <  (CPU_SIZE_T)len_max)) {             /* ... or max nbr chars cmp'd        (see Note #3d2).   */
        p1_str_next++;
        p2_str_next++;
        p1_str++;
        p2_str++;
        cmp_len++;
        char1 = ASCII_TO_LOWER(*p1_str);
        char2 = ASCII_TO_LOWER(*p2_str);
    }


    if (cmp_len == len_max) {                                   /* If strs     identical for len nbr of chars, ...      */
        return ((CPU_INT16S)0);                                 /* ... rtn 0                   (see Note #3d2).         */
    }

    if (char1 != char2) {                                       /* If strs NOT identical, ...                           */
         cmp_val = (CPU_INT16S)char1 - (CPU_INT16S)char2;       /* ... calc & rtn char diff    (see Note #3c).          */

    } else if (char1 == (CPU_CHAR)'\0') {                       /* If NULL char(s) found, ...                           */
         cmp_val = 0;                                           /* ... strs identical; rtn 0   (see Note #3b).          */

    } else {
        if (p1_str_next == (CPU_CHAR *)0) {
            if (p2_str_next == (CPU_CHAR *)0) {                 /* If BOTH next str ptrs NULL, ...                      */
                cmp_val  = (CPU_INT16S)0;                       /* ... rtn 0                   (see Note #3a2A).        */
            } else {                                            /* If p1_str_next NULL, ...                             */
                char2    =  ASCII_TO_LOWER(*p2_str_next);
                cmp_val  = (CPU_INT16S)0 - (CPU_INT16S)char2;   /* ... rtn neg p2_str_next val (see Note #3a2B1).       */
            }
        } else {                                                /* If p2_str_next NULL, ...                             */
            char1   =  ASCII_TO_LOWER(*p1_str_next);
            cmp_val = (CPU_INT16S)char1;                        /* ... rtn pos p1_str_next val (see Note #3a2B2).       */
        }
    }


    return (cmp_val);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             Str_Char()
*
* Description : Search string for first occurrence of specific character.
*
* Argument(s) : pstr            Pointer to string (see Note #1).
*
*               srch_char       Search character.
*
* Return(s)   : Pointer to first occurrence of search character in string, if any.
*
*               Pointer to NULL,                                           otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffer NOT modified.
*
*               (2) String search terminates when :
*
*                   (a) String pointer passed a NULL pointer.
*                       (1) No string search performed; NULL pointer returned.
*
*                   (b) String pointer points to NULL.
*                       (1) String overlaps with NULL address.
*                       (2) String searched up to but NOT beyond or including the NULL address.
*
*                   (c) String's terminating NULL character found.
*                       (1) Search character NOT found in search string; NULL pointer returned.
*                       (2) Applicable ONLY IF search character is NOT the terminating NULL character.
*
*                   (d) Search character found.
*                       (1) Return pointer to first occurrence of search character in search string.
*********************************************************************************************************
*/

CPU_CHAR  *Str_Char (CPU_CHAR  *pstr,
                     CPU_CHAR   srch_char)
{
    pstr = Str_Char_N((CPU_CHAR *)pstr,
                      (CPU_SIZE_T)DEF_INT_CPU_U_MAX_VAL,
                      (CPU_CHAR  )srch_char);

    return (pstr);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            Str_Char_N()
*
* Description : Search string for first occurrence of specific character, up to a maximum number of characters.
*
* Argument(s) : pstr            Pointer to string (see Note #1).
*
*               len_max         Maximum number of characters to search (see Notes #2e & #3).
*
*               srch_char       Search character.
*
* Return(s)   : Pointer to first occurrence of search character in string, if any.
*
*               Pointer to NULL,                                           otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffer NOT modified.
*
*               (2) String search terminates when :
*
*                   (a) String pointer passed a NULL pointer.
*                       (1) No string search performed; NULL pointer returned.
*
*                   (b) String pointer points to NULL.
*                       (1) String overlaps with NULL address.
*                       (2) String searched up to but NOT beyond or including the NULL address.
*
*                   (c) String's terminating NULL character found.
*                       (1) Search character NOT found in search string; NULL pointer returned.
*                       (2) Applicable ONLY IF search character is NOT the terminating NULL character.
*
*                   (d) Search character found.
*                       (1) Return pointer to first occurrence of search character in search string.
*
*                   (e) 'len_max' number of characters searched.
*                       (1) 'len_max' number of characters does NOT include terminating NULL character.
*
*               (3) Ideally, the 'len_max' parameter would be the last parameter in this function's
*                   paramter list for consistency with all other custom string library functions.
*                   However, the 'len_max' parameter is ordered to comply with the standard library
*                   function's parameter list.
*********************************************************************************************************
*/

CPU_CHAR  *Str_Char_N (CPU_CHAR    *pstr,
                       CPU_SIZE_T   len_max,
                       CPU_CHAR     srch_char)
{
    CPU_CHAR    *pstr_next;
    CPU_SIZE_T   len_srch;


    if (pstr == (CPU_CHAR *)0) {                                /* Rtn NULL if srch str ptr NULL    (see Note #2a).     */
        return ((CPU_CHAR *)0);
    }

    if (len_max == (CPU_SIZE_T)0) {                             /* Rtn NULL if srch len equals zero (see Note #2e).     */
        return ((CPU_CHAR *)0);
    }


    pstr_next = pstr;
    pstr_next++;
    len_srch  = 0;
    while (( pstr_next != (CPU_CHAR *)  0 )      &&             /* Srch str until NULL ptr(s)  [see Note #2b]  ...      */
           (*pstr      != (CPU_CHAR  )'\0')      &&             /* ... or NULL char            (see Note #2c)  ...      */
           (*pstr      != (CPU_CHAR  )srch_char) &&             /* ... or srch char found      (see Note #2d); ...      */
           ( len_srch  <  (CPU_SIZE_T)len_max)) {               /* ... or max nbr chars srch'd (see Note #2e).          */
        pstr++;
        pstr_next++;
        len_srch++;
    }


    if (*pstr != srch_char) {                                   /* If srch char NOT found, str points to NULL; ...      */
         return ((CPU_CHAR *)0);                                /* ... rtn NULL (see Notes #2b & #2c).                  */
    }

    return (pstr);                                              /* Else rtn ptr to found srch char (see Note #2d).      */
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           Str_Char_Last()
*
* Description : Search string for last occurrence of specific character.
*
* Argument(s) : pstr            Pointer to string (see Note #1).
*
*               srch_char       Search character.
*
* Return(s)   : Pointer to last occurrence of search character in string, if any.
*
*               Pointer to NULL,                                          otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffer NOT modified.
*
*               (2) String search terminates when :
*
*                   (a) String pointer passed a NULL pointer.
*                       (1) No string search performed; NULL pointer returned.
*
*                   (b) String pointer points to NULL.
*                       (1) String overlaps with NULL address.
*                       (2) String searched up to but NOT beyond or including the NULL address.
*                       (3) NULL address boundary handled in Str_Len().
*
*                   (c) String searched from end to beginning.
*                       (1) Search character NOT found in search string; NULL pointer returned.
*                       (2) Applicable ONLY IF search character is NOT the terminating NULL character.
*
*                   (d) Search character found.
*                       (1) Return pointer to first occurrence of search character in search string.
*********************************************************************************************************
*/

CPU_CHAR  *Str_Char_Last (CPU_CHAR  *pstr,
                          CPU_CHAR   srch_char)
{
    CPU_CHAR    *pstr_next;
    CPU_SIZE_T   str_len;


    if (pstr == (CPU_CHAR *)0) {                                /* Rtn NULL if srch str ptr NULL (see Note #2a).        */
        return ((CPU_CHAR *)0);
    }


    pstr_next  = pstr;
    str_len    = Str_Len(pstr);
    pstr_next += str_len;
    while (( pstr_next != pstr) &&                              /* Srch str from end until begining (see Note #2c) ...  */
           (*pstr_next != srch_char)) {                         /* ... until srch char found        (see Note #2d).     */
        pstr_next--;
    }


    if (*pstr_next != srch_char) {                              /* If srch char NOT found, str points to NULL; ...      */
         return ((CPU_CHAR *)0);                                /* ... rtn NULL (see Notes #2b & #2c).                  */
    }

    return (pstr_next);                                         /* Else rtn ptr to found srch char (see Note #2d).      */
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             Str_Str()
*
* Description : Search string for first occurence of a specific search string.
*
* Argument(s) : pstr            Pointer to        string (see Note #1).
*
*               psrch_str       Pointer to search string (see Note #1).
*
* Return(s)   : Pointer to first occurrence of search string in string, if any.
*
*               Pointer to NULL,                                        otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffers NOT modified.
*
*               (2) String search terminates when :
*
*                   (a) String pointer passed a NULL pointer.
*                       (1) No string search performed; NULL pointer returned.
*
*                   (b) Search string length greater than string length.
*                       (1) No string search performed; NULL pointer returned.
*
*                   (c) Search string length equal to zero.
*                       (1) NULL search string at end of string returned.
*
*                   (d) Entire string has been searched.
*                       (1) Maximum size of the search is defined as the subtraction of the
*                           search string length from the string length.
*                       (2) Search string not found; NULL pointer returned.
*
*                   (e) Search string found.
*                       (1) Search string found according to Str_Cmp_N() return value.
*                       (2) Return pointer to first occurrence of search string in string.
*********************************************************************************************************
*/

CPU_CHAR  *Str_Str (CPU_CHAR  *pstr,
                    CPU_CHAR  *psrch_str)
{
    CPU_SIZE_T    str_len;
    CPU_SIZE_T    srch_str_len;
    CPU_SIZE_T    srch_len;
    CPU_SIZE_T    srch_ix;
    CPU_BOOLEAN   srch_done;
    CPU_INT16S    srch_cmp;
    CPU_CHAR     *pstr_srch_ix;

                                                                /* Rtn NULL if str ptr(s) NULL (see Note #2a).          */
    if (pstr == (CPU_CHAR *)0) {
        return ((CPU_CHAR *)0);
    }
    if (psrch_str == (CPU_CHAR *)0) {
        return ((CPU_CHAR *)0);
    }


    str_len      = Str_Len(pstr);
    srch_str_len = Str_Len(psrch_str);
    if (srch_str_len > str_len) {                               /* If srch str len > str len, rtn NULL  (see Note #2b). */
        return ((CPU_CHAR *)0);
    }
    if (srch_str_len == 0) {                                    /* If srch str len = 0, srch str equal NULL str; ...    */
        pstr_srch_ix = (CPU_CHAR *)(pstr + str_len);            /* ... rtn ptr to NULL str found in str (see Note #2c). */
        return (pstr_srch_ix);
    }

    srch_len  = str_len - srch_str_len;                         /* Determine srch len (see Note #2d1).                  */
    srch_ix   = 0;
    srch_done = DEF_NO;
    do {
        pstr_srch_ix = (CPU_CHAR *)(pstr + srch_ix);
        srch_cmp     =  Str_Cmp_N(pstr_srch_ix, psrch_str, srch_str_len);
        srch_done    = (srch_cmp == 0) ? DEF_YES : DEF_NO;
        srch_ix++;
    } while ((srch_done == DEF_NO) && (srch_ix <= srch_len));


    if (srch_cmp != 0) {                                        /* If srch str NOT found, rtn NULL  (see Note #2d).     */
        return ((CPU_CHAR *)0);
    }

    return (pstr_srch_ix);                                      /* Rtn ptr to srch str found in str (see Note #2e).     */
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         Str_FmtNbr_Int32U()
*
* Description : Format 32-bit unsigned integer into a multi-digit character string.
*
* Argument(s) : nbr             Number           to format.
*
*               nbr_dig         Number of digits to format (see Note #1).
*
*                               The following may be used to specify the number of digits to format :
*
*                                   DEF_INT_32U_NBR_DIG_MIN     Minimum number of 32-bit unsigned digits
*                                   DEF_INT_32U_NBR_DIG_MAX     Maximum number of 32-bit unsigned digits
*
*               nbr_base        Base   of number to format (see Note #2).
*
*                               The following may be used to specify the number base :
*
*                                   DEF_NBR_BASE_BIN            Base  2
*                                   DEF_NBR_BASE_OCT            Base  8
*                                   DEF_NBR_BASE_DEC            Base 10
*                                   DEF_NBR_BASE_HEX            Base 16
*
*               lead_char       Prepend leading character  (see Note #3) :
*
*                                   '\0'                    Do NOT prepend leading character to string.
*                                     Printable character          Prepend leading character to string.
*                                   Unprintable character   Do NOT format string.
*
*               lower_case      Format alphabetic characters (if any) in lower case :
*
*                                   DEF_NO          Format alphabetic characters in upper case.
*                                   DEF_YES         Format alphabetic characters in lower case.
*
*               nul             Append terminating NULL-character (see Note #4) :
*
*                                   DEF_NO          Do NOT append terminating NULL-character to string.
*                                   DEF_YES                Append terminating NULL-character to string.
*
*               pstr            Pointer to character array to return formatted number string (see Note #5).
*
* Return(s)   : Pointer to formatted string, if NO errors.
*
*               Pointer to NULL,             otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) If the number of digits to format ('nbr_dig') is less than the number of significant
*                   integer digits of the number to format ('nbr'); then the most-significant digits of
*                   the formatted number will be truncated.
*
*                       Example :
*
*                           nbr     = 23456
*                           nbr_dig = 3
*
*                           pstr    = "456"
*$PAGE*
*               (2) The number's base MUST be between 2 & 36, inclusive.
*
*               (3) (a) Leading character option prepends leading characters prior to the first non-zero
*                       digit.  The number of leading characters is such that the total number of integer
*                       digits is equal to the requested number of integer digits to format ('nbr_dig').
*
*                   (b) Leading character MUST be a printable ASCII character.
*
*                   (c) (1) If the value of the number to format is     zero  ...
*                       (2) ... & the number of digits to format is non-zero, ...
*                       (3) ... but NO leading character available;           ...
*                       (4) ... then one digit of '0' value is formatted.
*
*                           This is NOT a leading character; but a single integer digit of '0' value.
*
*               (4) (a) NULL-character terminate option DISABLED prevents overwriting previous character
*                       array formatting.
*
*                   (b) WARNING: Unless 'pstr' character array is pre-/post-terminated, NULL-character
*                       terminate option DISABLED will cause character string run-on.
*
*               (5) (a) Format buffer size NOT validated; buffer overruns MUST be prevented by caller.
*
*                   (b) To prevent character buffer overrun :
*
*                           Character array size MUST be  >=  ('nbr_dig'         +
*                                                              1 'NUL' terminator)  characters
*********************************************************************************************************
*/

CPU_CHAR  *Str_FmtNbr_Int32U (CPU_INT32U    nbr,
                              CPU_INT08U    nbr_dig,
                              CPU_INT08U    nbr_base,
                              CPU_CHAR      lead_char,
                              CPU_BOOLEAN   lower_case,
                              CPU_BOOLEAN   nul,
                              CPU_CHAR     *pstr)
{
    CPU_CHAR  *pstr_fmt;


    pstr_fmt = Str_FmtNbr_Int32((CPU_INT32U )nbr,               /* Fmt unsigned int into str.                           */
                                (CPU_INT08U )nbr_dig,
                                (CPU_INT08U )nbr_base,
                                (CPU_BOOLEAN)DEF_NO,
                                (CPU_CHAR   )lead_char,
                                (CPU_BOOLEAN)lower_case,
                                (CPU_BOOLEAN)nul,
                                (CPU_CHAR  *)pstr);

    return (pstr_fmt);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         Str_FmtNbr_Int32S()
*
* Description : Format 32-bit signed integer into a multi-digit character string.
*
* Argument(s) : nbr             Number           to format.
*
*               nbr_dig         Number of digits to format (see Note #1).
*
*                               The following may be used to specify the number of digits to format :
*
*                                   DEF_INT_32S_NBR_DIG_MIN     Minimum number of 32-bit signed digits
*                                   DEF_INT_32S_NBR_DIG_MAX     Maximum number of 32-bit signed digits
*
*               nbr_base        Base   of number to format (see Note #2).
*
*                               The following may be used to specify the number base :
*
*                                   DEF_NBR_BASE_BIN            Base  2
*                                   DEF_NBR_BASE_OCT            Base  8
*                                   DEF_NBR_BASE_DEC            Base 10
*                                   DEF_NBR_BASE_HEX            Base 16
*
*               lead_char       Prepend leading character  (see Note #3) :
*
*                                   '\0'                    Do NOT prepend leading character to string.
*                                     Printable character          Prepend leading character to string.
*                                   Unprintable character   Do NOT format string.
*
*               lower_case      Format alphabetic characters (if any) in lower case :
*
*                                   DEF_NO          Format alphabetic characters in upper case.
*                                   DEF_YES         Format alphabetic characters in lower case.
*
*               nul             Append terminating NULL-character (see Note #4) :
*
*                                   DEF_NO          Do NOT append terminating NULL-character to string.
*                                   DEF_YES                Append terminating NULL-character to string.
*
*               pstr            Pointer to character array to return formatted number string (see Note #5).
*
* Return(s)   : Pointer to formatted string, if NO errors.
*
*               Pointer to NULL,             otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) (a) If the number of digits to format ('nbr_dig') is less than the number of significant
*                       integer digits of the number to format ('nbr'); then the most-significant digits of
*                       the formatted number will be truncated.
*
*                           Example :
*
*                               nbr     = 23456
*                               nbr_dig = 3
*
*                               pstr    = "456"
*
*                   (b) If number to format ('nbr') is negative but the most-significant digits of the
*                       formatted number are truncated (see Note #2a); the negative sign still prefixes
*                       the truncated formatted number.
*
*                           Example :
*
*                               nbr     = -23456
*                               nbr_dig =  3
*
*                               pstr    = "-456"
*$PAGE*
*               (2) The number's base MUST be between 2 & 36, inclusive.
*
*               (3) (a) Leading character option prepends leading characters prior to the first non-zero
*                       digit.  The number of leading characters is such that the total number of integer
*                       digits is equal to the requested number of integer digits to format ('nbr_dig').
*
*                   (b) Leading character MUST be a printable ASCII character.
*
*                   (c) (1) If the value of the number to format is     zero  ...
*                       (2) ... & the number of digits to format is non-zero, ...
*                       (3) ... but NO leading character available;           ...
*                       (4) ... then one digit of '0' value is formatted.
*
*                           This is NOT a leading character; but a single integer digit of '0' value.
*
*               (4) (a) NULL-character terminate option DISABLED prevents overwriting previous character
*                       array formatting.
*
*                   (b) WARNING: Unless 'pstr' character array is pre-/post-terminated, NULL-character
*                       terminate option DISABLED will cause character string run-on.
*
*               (5) (a) Format buffer size NOT validated; buffer overruns MUST be prevented by caller.
*
*                   (b) To prevent character buffer overrun :
*
*                           Character array size MUST be  >=  ('nbr_dig'         +
*                                                              1 negative sign   +
*                                                              1 'NUL' terminator)  characters
*********************************************************************************************************
*/

CPU_CHAR  *Str_FmtNbr_Int32S (CPU_INT32S    nbr,
                              CPU_INT08U    nbr_dig,
                              CPU_INT08U    nbr_base,
                              CPU_CHAR      lead_char,
                              CPU_BOOLEAN   lower_case,
                              CPU_BOOLEAN   nul,
                              CPU_CHAR     *pstr)
{
    CPU_CHAR     *pstr_fmt;
    CPU_INT32S    nbr_fmt;
    CPU_BOOLEAN   nbr_neg;


    nbr_fmt = nbr;
    if (nbr_fmt < 0) {                                          /* If nbr neg, ...                                      */
        nbr_fmt = -nbr_fmt;                                     /* ... negate nbr.                                      */
        nbr_neg =  DEF_YES;
    } else {
        nbr_neg =  DEF_NO;
    }

    pstr_fmt = Str_FmtNbr_Int32((CPU_INT32U )nbr_fmt,           /* Fmt signed int into str.                             */
                                (CPU_INT08U )nbr_dig,
                                (CPU_INT08U )nbr_base,
                                (CPU_BOOLEAN)nbr_neg,
                                (CPU_CHAR   )lead_char,
                                (CPU_BOOLEAN)lower_case,
                                (CPU_BOOLEAN)nul,
                                (CPU_CHAR  *)pstr);

    return (pstr_fmt);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           Str_FmtNbr_32()
*
* Description : Format number into a multi-digit character string.
*
* Argument(s) : nbr             Number                         to format (see Note #1).
*
*               nbr_dig         Number of decimal       digits to format (see Note #2).
*
*               nbr_dp          Number of decimal point digits to format.
*
*               lead_char       Prepend leading character (see Note #3) :
*
*                                   '\0'                    Do NOT prepend leading character to string.
*                                     Printable character          Prepend leading character to string.
*                                   Unprintable character   Do NOT format string (see Note #6b).
*
*               nul             Append terminating NULL-character (see Note #4) :
*
*                                   DEF_NO          Do NOT append terminating NULL-character to string.
*                                   DEF_YES                Append terminating NULL-character to string.
*
*               pstr            Pointer to character array to return formatted number string (see Note #5).
*
* Return(s)   : Pointer to formatted string, if NO errors (see Note #6).
*
*               Pointer to NULL,             otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) (a) The maximum accuracy for 32-bit floating-point numbers :
*
*
*                                 Maximum Accuracy            log [Internal-Base ^ (Number-Internal-Base-Digits)]
*                           32-bit Floating-point Number  =  -----------------------------------------------------
*                                                                             log [External-Base]
*
*                                                             log [2 ^ 24]
*                                                         =  --------------
*                                                               log [10]
*
*                                                         <  7.225  Base-10 Digits
*
*                               where
*                                       Internal-Base                   Internal number base of floating-
*                                                                           point numbers (i.e.  2)
*                                       External-Base                   External number base of floating-
*                                                                           point numbers (i.e. 10)
*                                       Number-Internal-Base-Digits     Number of internal number base
*                                                                           significant digits (i.e. 24)
*
*                   (b) Some compilers' floating-point routines MAY further reduce the maximum accuracy.
*
*                   (c) If the total number of digits to format ('nbr_dig + nbr_dp') is greater than the
*                       maximum accuracy; digits following the first, significantly-accurate digits will
*                       be inaccurate.
*$PAGE*
*               (2) (a) If the number of digits to format ('nbr_dig') is less than the number of significant
*                       integer digits of the number to format ('nbr'); then the most-significant digits of
*                       the formatted number will be truncated.
*
*                           Example :
*
*                               nbr     = 23456.789
*                               nbr_dig = 3
*                               nbr_dp  = 2
*
*                               pstr    = "456.78"
*
*                   (b) If number to format ('nbr') is negative but the most-significant digits of the
*                       formatted number are truncated (see Note #2a); the negative sign still prefixes
*                       the truncated formatted number.
*
*                           Example :
*
*                               nbr     = -23456.789
*                               nbr_dig =  3
*                               nbr_dp  =  2
*
*                               pstr    = "-456.78"
*
*               (3) (a) Leading character option prepends leading characters prior to the first non-zero
*                       digit.  The number of leading characters is such that the total number of integer
*                       digits is equal to the requested number of integer digits to format ('nbr_dig').
*
*                   (b) Leading character MUST be a printable ASCII character.
*
*                   (c) (1) If the integer value of the number to format is zero ...
*                       (2) ... & the number of digits to format is non-zero,    ...
*                       (3) ... but NO leading character available;              ...
*                       (4) ... then one digit of '0' value is formatted.
*
*                           This is NOT a leading character; but a single integer digit of '0' value.
*
*               (4) (a) NULL-character terminate option DISABLED prevents overwriting previous character
*                       array formatting.
*
*                   (b) WARNING: Unless 'pstr' character array is pre-/post-terminated, NULL-character
*                       terminate option DISABLED will cause character string run-on.
*
*               (5) (a) Format buffer size NOT validated; buffer overruns MUST be prevented by caller.
*
*                   (b) To prevent character buffer overrun :
*
*                           Character array size MUST be  >=  ('nbr_dig'         +
*                                                              'nbr_dp'          +
*                                                              1 negative sign   +
*                                                              1 decimal point   +
*                                                              1 'NUL' terminator)  characters
*
*               (6) String format terminates when :
*
*                   (a) Format string pointer is passed a NULL pointer.
*                       (1) No string format performed; NULL pointer returned.
*
*                   (b) Lead character is NOT a printable character (see Note #3b).
*                       (1) No string format performed; NULL pointer returned.
*
*                   (c) Number successfully formatted into character string array.
*********************************************************************************************************
*/
/*$PAGE*/
#if (LIB_STR_CFG_FP_EN == DEF_ENABLED)
CPU_CHAR  *Str_FmtNbr_32 (CPU_FP32      nbr,
                          CPU_INT08U    nbr_dig,
                          CPU_INT08U    nbr_dp,
                          CPU_CHAR      lead_char,
                          CPU_BOOLEAN   nul,
                          CPU_CHAR     *pstr)
{
    CPU_CHAR     *pstr_fmt;
    CPU_FP32      nbr_fmt;
    CPU_INT32U    dig_nbr;
    CPU_INT32U    dig_val;
    CPU_FP32      dig_exp;
    CPU_FP32      dp_exp;
    CPU_INT08U    i;
    CPU_BOOLEAN   print_char;


    if (pstr == (CPU_CHAR *)0) {                                /* Rtn NULL if str ptr NULL (see Note #6a).             */
        return ((CPU_CHAR *)0);
    }

    if (lead_char != (CPU_CHAR)'\0') {
        print_char =  ASCII_IsPrint(lead_char);
        if (print_char != DEF_YES) {                            /* Rtn NULL if lead char non-printable (see Note #6b).  */
            return  ((CPU_CHAR *)0);
        }
    }


    nbr_fmt  = nbr;
    pstr_fmt = pstr;

    if (nbr_fmt < 0.0f) {                                       /* If nbr neg,               ...                        */
        nbr_fmt = -nbr_fmt;                                     /* ... negate nbr;           ...                        */
        if ((nbr_dig > 0) ||                                    /* ... & if at least one dig ...                        */
            (nbr_dp  > 0)) {                                    /* ...   or at least one dp; ...                        */
            *pstr_fmt++ = '-';                                  /* ... prepend a neg sign (see Note #2b).               */
        }
    }

    if (nbr_dig > 0) {
        dig_exp = 1.0f;
        for (i = 1; i < nbr_dig; i++) {
            dig_exp *= 10.0f;
        }
        for (i = nbr_dig; i > 0; i--) {                         /* Fmt str for desired nbr digs.                        */
            dig_nbr = (CPU_INT32U)(nbr_fmt / dig_exp);
            if ((dig_nbr >  0) ||                               /* If dig nbr > 0              (see Note #3c1), ...     */
                (nbr_dig == 1) ||                               /* ... OR exactly 1 dig to fmt (see Note #3c2), ...     */
                (i       == 1)) {                               /* ... OR on one's  dig to fmt;                 ...     */
                                                                /* ... calc & fmt dig val.                              */
                dig_val    = (CPU_INT32U)(dig_nbr % 10 );
               *pstr_fmt++ = (CPU_CHAR  )(dig_val + '0');

            } else if (lead_char != (CPU_CHAR)'\0') {           /* ... else if avail,                           ...     */
               *pstr_fmt++ = (CPU_CHAR)lead_char;               /* ... fmt lead char.                                   */
            }

            dig_exp /= 10.0f;                                   /* Shift to next least-significant dig.                 */
        }
    }

    if (nbr_dp > 0) {
       *pstr_fmt++ = '.';                                       /* Append dp prior to dp conversion.                    */
        dp_exp = 10.0f;
        for (i = 0; i < nbr_dp; i++) {                          /* Fmt str for desired nbr dp.                          */
            dig_nbr     = (CPU_INT32U)(nbr_fmt * dp_exp);
            dig_val     = (CPU_INT32U)(dig_nbr % 10 );
           *pstr_fmt++  = (CPU_CHAR  )(dig_val + '0');
            dp_exp     *=  10.0f;                               /* Shift to next least-significant dp.                  */
        }
    }

    if (nul != DEF_NO) {                                        /* If NOT DISABLED, append NULL char (see Note #4).     */
       *pstr_fmt = (CPU_CHAR)'\0';
    }


    return (pstr);                                              /* Rtn ptr to fmt'd str (see Note #6c).                 */
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                        Str_ParseNbr_Int32U()
*
* Description : Parse 32-bit unsigned integer from string.
*
* Argument(s) : pstr        Pointer to string (see Notes #1 & #2a).
*
*               pstr_next   Pointer to a variable to ... :
*
*                               (a) Return a pointer to first character following the integer string,
*                                       if NO errors (see Note  #2a2B2);
*                               (b) Return a pointer to 'pstr',
*                                       otherwise    (see Note  #2a2A2).
*
*               nbr_base    Base of number to parse  (see Notes #2a1B1 & #2a2B1).
*
* Return(s)   : Parsed integer,      if integer parsed with NO overflow   (see Note #2a3A).
*
*               DEF_INT_32U_MAX_VAL, if integer parsed but     overflowed (see Note #2a3A1).
*
*               0,                   otherwise                            (see Note #2a3B).
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffer NOT modified.
*
*               (2) (a) IEEE Std 1003.1, 2004 Edition, Section 'strtoul() : DESCRIPTION' states that "these
*                       functions shall convert the initial portion of the string pointed to by 'str' to a
*                       type unsigned long ... representation" :
*
*                       (1) "First, they decompose the input string into three parts" :
*
*                           (A) "An initial, possibly empty, sequence of white-space characters [as specified
*                                by isspace()]."
*
*                               (1) "The subject sequence is defined as the longest initial subsequence of the
*                                    input string, starting with the first non-white-space character that is of
*                                    the expected form.  The subject sequence shall contain no characters if the
*                                    input string is empty or consists entirely of white-space characters."
*
*                           (B) (1) "A subject sequence interpreted as an integer represented in some radix
*                                    determined by the value of 'base'" :
*
*                                   (a) "If the value of 'base' is 0, the expected form of the subject sequence
*                                        is that of a decimal constant, octal constant, or hexadecimal constant" :
*
*                                       (1) "A decimal constant begins with a non-zero digit, and consists of a
*                                            sequence of decimal digits."
*
*                                       (2) "An octal constant consists of the prefix '0' optionally followed by
*                                            a sequence of the digits '0' to '7' only."
*
*                                       (3) "A hexadecimal constant consists of the prefix '0x' or '0X' followed
*                                            by a sequence of the decimal digits and letters 'a' (or 'A') to 'f'
*                                            (or 'F') with values 10 to 15 respectively."
*
*                                   (b) "If the value of 'base' is between 2 and 36, the expected form of the
*                                        subject sequence is a sequence of letters and digits representing an
*                                        integer with the radix specified by 'base'" :
*
*                                       (1) (A) "The letters from 'a' (or 'A') to 'z' (or 'Z') inclusive are
*                                                ascribed the values 10 to 35"; ...
*                                           (B) "only letters whose ascribed values are less than that of base
*                                                are permitted."
*
*                                       (2) (A) "If the value of 'base' is 16, the characters '0x' or '0X' may
*                                                optionally precede the sequence of letters and digits."
*
*                                           (B) Although NO specification states that "if the value of 'base'
*                                               is" 8, the '0' character "may optionally precede the sequence
*                                               of letters and digits"; it seems reasonable to allow the '0'
*                                               character to be optionally parsed.
*
*                               (2) "A subject sequence .... may be preceded by a '+' or '-' sign."
*
*                                   (a) However, it does NOT seem reasonable to parse & convert a negative number
*                                       integer string into an unsigned integer.
*
*                           (C) (1) (a) "A final string of one or more unrecognized characters,"  ...
*                                   (b) "including the terminating null byte of the input string" ...
*                               (2) "other than a sign or a permissible letter or digit."
*$PAGE*
*                       (2) Second, "they shall attempt to convert the subject sequence to an unsigned integer" :
*
*                           (A) "If the subject sequence is empty or does not have the expected form" :
*
*                               (1) "no conversion [is] performed"; ...
*                               (2) "the value of 'str' [is] stored in the object pointed to by 'endptr',
*                                    provided that 'endptr' is not a null pointer."
*
*                           (B) "If the subject sequence has the expected form" :
*
*                               (1) (a) "and the value of 'base' is 0, the sequence of characters starting with
*                                        the first digit shall be interpreted as an integer constant."
*
*                                   (b) "and the value of 'base' is between 2 and 36, it shall be used as the
*                                        base for conversion, ascribing to each letter its value as given above"
*                                        (see Note #2a1B1b1A).
*
*                               (2) "A pointer to the final string shall be stored in the object pointed to by
*                                   'endptr', provided that 'endptr' is not a null pointer."
*
*                       (3) Lastly, IEEE Std 1003.1, 2004 Edition, Section 'strtoul() : RETURN VALUE' states that :
*
*                           (A) "Upon successful completion, these functions shall return the converted value."
*                               (1) "If the correct value is outside the range of representable values, {ULONG_MAX}
*                                    ... shall be returned."
*
*                           (B) "If no conversion could be performed, 0 shall be returned."
*
*                   (b) (1) IEEE Std 1003.1, 2004 Edition, Section 'strtoul() : ERRORS' states that "these functions
*                           shall fail if" :
*
*                           (A) "[EINVAL] - The value of 'base' is not supported."
*
*                           (B) "[ERANGE] - The value to be returned is not representable."
*
*                       (2) IEEE Std 1003.1, 2004 Edition, Section 'strtoul() : ERRORS' states that "these functions
*                           may fail if" :
*
*                           (A) "[EINVAL] - No conversion could be performed."
*
*               (3) Return integer value & next string pointer should be used to diagnose parse success or failure :
*
*                   (a) Valid parse string integer :
*
*                           pstr      = "     ABCDE xyz"
*                           nbr_base  = 16
*
*                           nbr       = 703710
*                           pstr_next = " xyz"
*
*
*                   (b) Invalid parse string integer :
*
*                           pstr      = "     ABCDE"
*                           nbr_base  = 10
*
*                           nbr       =  0
*                           pstr_next = pstr = "     ABCDE"
*
*
*                   (c) Valid hexadecimal parse string integer :
*
*                           pstr      = "     0xGABCDE"
*                           nbr_base  = 16
*
*                           nbr       =  0
*                           pstr_next = "xGABCDE"
*
*
*                   (d) Valid decimal parse string integer ('0x' prefix ignored
*                               following invalid hexadecimal characters) :
*
*                           pstr      = "     0xGABCDE"
*                           nbr_base  =  0
*
*                           nbr       =  0
*                           pstr_next = "xGABCDE"
*
*
*                   (e) Valid decimal parse string integer ('0'  prefix ignored
*                               following invalid octal       characters) :
*
*                           pstr      = "     0GABCDE"
*                           nbr_base  =  0
*
*                           nbr       =  0
*                           pstr_next = "GABCDE"
*
*$PAGE*
*                   (f) Parse string integer overflow :
*
*                           pstr      = "   12345678901234567890*123456"
*                           nbr_base  = 10
*
*                           nbr       = DEF_INT_32U_MAX_VAL
*                           pstr_next = "*123456"
*
*
*                   (g) Invalid negative unsigned parse string :
*
*                           pstr      = "  -12345678901234567890*123456"
*                           nbr_base  = 10
*
*                           nbr       = 0
*                           pstr_next = pstr = "  -12345678901234567890*123456"
*
*********************************************************************************************************
*/

CPU_INT32U  Str_ParseNbr_Int32U (CPU_CHAR     *pstr,
                                 CPU_CHAR    **pstr_next,
                                 CPU_INT08U    nbr_base)
{
    CPU_INT32U  nbr;


    nbr = Str_ParseNbr_Int32((CPU_CHAR    *)pstr,               /* Parse/convert str ...                                */
                             (CPU_CHAR   **)pstr_next,
                             (CPU_INT08U   )nbr_base,
                             (CPU_BOOLEAN  )DEF_NO,             /* ... as unsigned int (see Note #2a2).                 */
                             (CPU_BOOLEAN *)0);

    return (nbr);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        Str_ParseNbr_Int32S()
*
* Description : Parse 32-bit signed integer from string.
*
* Argument(s) : pstr        Pointer to string (see Notes #1 & #2a).
*
*               pstr_next   Pointer to a variable to ... :
*
*                               (a) Return a pointer to first character following the integer string,
*                                       if NO errors (see Note  #2a2B2);
*                               (b) Return a pointer to 'pstr',
*                                       otherwise    (see Note  #2a2A2).
*
*               nbr_base    Base of number to parse  (see Notes #2a1B1 & #2a2B1).
*
* Return(s)   : Parsed integer,      if integer parsed with NO over- or underflow (see Note #2a3A).
*
*               DEF_INT_32S_MIN_VAL, if integer parsed but negatively underflowed (see Note #2a3A1a).
*
*               DEF_INT_32U_MAX_VAL, if integer parsed but positively overflowed  (see Note #2a3A1b).
*
*               0,                   otherwise                                    (see Note #2a3B).
*
* Caller(s)   : Application.
*
* Note(s)     : (1) String buffer NOT modified.
*
*               (2) (a) IEEE Std 1003.1, 2004 Edition, Section 'strtol() : DESCRIPTION' states that "these
*                       functions shall convert the initial portion of the string pointed to by 'str' to a
*                       type long ... representation" :
*
*                       (1) "First, they decompose the input string into three parts" :
*
*                           (A) "An initial, possibly empty, sequence of white-space characters [as specified
*                                by isspace()]."
*
*                               (1) "The subject sequence is defined as the longest initial subsequence of the
*                                    input string, starting with the first non-white-space character that is of
*                                    the expected form.  The subject sequence shall contain no characters if the
*                                    input string is empty or consists entirely of white-space characters."
*
*                           (B) (1) "A subject sequence interpreted as an integer represented in some radix
*                                    determined by the value of 'base'" :
*
*                                   (a) "If the value of 'base' is 0, the expected form of the subject sequence
*                                        is that of a decimal constant, octal constant, or hexadecimal constant" :
*
*                                       (1) "A decimal constant begins with a non-zero digit, and consists of a
*                                            sequence of decimal digits."
*
*                                       (2) "An octal constant consists of the prefix '0' optionally followed by
*                                            a sequence of the digits '0' to '7' only."
*
*                                       (3) "A hexadecimal constant consists of the prefix '0x' or '0X' followed
*                                            by a sequence of the decimal digits and letters 'a' (or 'A') to 'f'
*                                            (or 'F') with values 10 to 15 respectively."
*
*                                   (b) "If the value of 'base' is between 2 and 36, the expected form of the
*                                        subject sequence is a sequence of letters and digits representing an
*                                        integer with the radix specified by 'base'" :
*
*                                       (1) (A) "The letters from 'a' (or 'A') to 'z' (or 'Z') inclusive are
*                                                ascribed the values 10 to 35"; ...
*                                           (B) "only letters whose ascribed values are less than that of base
*                                                are permitted."
*
*                                       (2) (A) "If the value of 'base' is 16, the characters '0x' or '0X' may
*                                                optionally precede the sequence of letters and digits."
*
*                                           (B) Although NO specification states that "if the value of 'base'
*                                               is" 8, the '0' character "may optionally precede the sequence
*                                               of letters and digits"; it seems reasonable to allow the '0'
*                                               character to be optionally parsed.
*
*                               (2) "A subject sequence .... may be preceded by a '+' or '-' sign."
*
*                                   (a) However, it does NOT seem reasonable to parse & convert a negative number
*                                       integer string into an unsigned integer.
*
*                           (C) (1) (a) "A final string of one or more unrecognized characters,"  ...
*                                   (b) "including the terminating null byte of the input string" ...
*                               (2) "other than a sign or a permissible letter or digit."
*$PAGE*
*                       (2) Second, "they shall attempt to convert the subject sequence to an integer" :
*
*                           (A) "If the subject sequence is empty or does not have the expected form" :
*
*                               (1) "no conversion is performed"; ...
*                               (2) "the value of 'str' is stored in the object pointed to by 'endptr',
*                                    provided that 'endptr' is not a null pointer."
*
*                           (B) "If the subject sequence has the expected form" :
*
*                               (1) (a) "and the value of 'base' is 0, the sequence of characters starting with
*                                        the first digit shall be interpreted as an integer constant."
*
*                                   (b) "and the value of 'base' is between 2 and 36, it shall be used as the
*                                        base for conversion, ascribing to each letter its value as given above"
*                                        (see Note #2a1B1b1A).
*
*                               (2) "A pointer to the final string shall be stored in the object pointed to by
*                                   'endptr', provided that 'endptr' is not a null pointer."
*
*                       (3) Lastly, IEEE Std 1003.1, 2004 Edition, Section 'strtol() : RETURN VALUE' states that :
*
*                           (A) "Upon successful completion, these functions shall return the converted value."
*
*                               (1) "If the correct value is outside the range of representable values", either
*                                    of the following "shall be returned" :
*                                   (a) "{LONG_MIN}" or ...
*                                   (b) "{LONG_MAX}"
*
*                           (B) "If no conversion could be performed, 0 shall be returned."
*
*                   (b) (1) IEEE Std 1003.1, 2004 Edition, Section 'strtoul() : ERRORS' states that "these functions
*                           shall fail if" :
*
*                           (A) "[EINVAL] - The value of 'base' is not supported."
*
*                           (B) "[ERANGE] - The value to be returned is not representable."
*
*                       (2) IEEE Std 1003.1, 2004 Edition, Section 'strtoul() : ERRORS' states that "these functions
*                           may fail if" :
*
*                           (A) "[EINVAL] - No conversion could be performed."
*
*               (3) Return integer value & next string pointer should be used to diagnose parse success or failure :
*
*                   (a) Valid parse string integer :
*
*                           pstr      = "     ABCDE xyz"
*                           nbr_base  = 16
*
*                           nbr       = 703710
*                           pstr_next = " xyz"
*
*
*                   (b) Invalid parse string integer :
*
*                           pstr      = "     ABCDE"
*                           nbr_base  = 10
*
*                           nbr       =  0
*                           pstr_next = pstr = "     ABCDE"
*
*
*                   (c) Valid hexadecimal parse string integer :
*
*                           pstr      = "     0xGABCDE"
*                           nbr_base  = 16
*
*                           nbr       =  0
*                           pstr_next = "xGABCDE"
*
*
*                   (d) Valid decimal parse string integer ('0x' prefix ignored
*                               following invalid hexadecimal characters) :
*
*                           pstr      = "     0xGABCDE"
*                           nbr_base  =  0
*
*                           nbr       =  0
*                           pstr_next = "xGABCDE"
*
*
*                   (e) Valid decimal parse string integer ('0'  prefix ignored
*                               following invalid octal       characters) :
*
*                           pstr      = "     0GABCDE"
*                           nbr_base  =  0
*
*                           nbr       =  0
*                           pstr_next = "GABCDE"
*
*$PAGE*
*                   (f) Parse string integer overflow :
*
*                           pstr      = "   12345678901234567890*123456"
*                           nbr_base  = 10
*
*                           nbr       = DEF_INT_32S_MAX_VAL
*                           pstr_next = "*123456"
*
*
*                   (g) Parse string integer underflow :
*
*                           pstr     = "  -12345678901234567890*123456"
*                           nbr_base = 10
*
*                           nbr       = DEF_INT_32S_MIN_VAL
*                           pstr_next = "*123456"
*
*********************************************************************************************************
*/

CPU_INT32S  Str_ParseNbr_Int32S (CPU_CHAR     *pstr,
                                 CPU_CHAR    **pstr_next,
                                 CPU_INT08U    nbr_base)
{
    CPU_INT32S   nbr;
    CPU_INT32U   nbr_abs;
    CPU_BOOLEAN  nbr_neg;


    nbr_abs = Str_ParseNbr_Int32((CPU_CHAR    *) pstr,          /* Parse/convert str ...                                */
                                 (CPU_CHAR   **) pstr_next,
                                 (CPU_INT08U   ) nbr_base,
                                 (CPU_BOOLEAN  ) DEF_YES,       /* ... as signed int (see Note #2a2).                   */
                                 (CPU_BOOLEAN *)&nbr_neg);

    if (nbr_neg == DEF_NO) {                                    /* Chk for neg nbr & ovf/undf (see Note #2a3A1).        */
        nbr = (nbr_abs > (CPU_INT32U) DEF_INT_32S_MAX_VAL)          ?  (CPU_INT32S)DEF_INT_32S_MAX_VAL
                                                                    :  (CPU_INT32S)nbr_abs;
    } else {
        nbr = (nbr_abs > (CPU_INT32U)-DEF_INT_32S_MIN_VAL_ONES_CPL) ?  (CPU_INT32S)DEF_INT_32S_MIN_VAL
                                                                    : -(CPU_INT32S)nbr_abs;
    }

    return (nbr);
}


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
*                                         Str_FmtNbr_Int32()
*
* Description : Format 32-bit integer into a multi-digit character string.
*
* Argument(s) : nbr             Number           to format.
*
*               nbr_dig         Number of digits to format (see Note #1).
*
*               nbr_base        Base   of number to format (see Note #2).
*
*               nbr_neg         Indicates whether number to format is negative :
*
*                                   DEF_NO          Number is non-negative.
*                                   DEF_YES         Number is     negative.
*
*               lead_char       Prepend leading character  (see Note #3) :
*
*                                   '\0'                    Do NOT prepend leading character to string.
*                                     Printable character          Prepend leading character to string.
*                                   Unprintable character   Do NOT format string (see Note #6b).
*
*               lower_case      Format alphabetic characters (if any) in lower case :
*
*                                   DEF_NO          Format alphabetic characters in upper case.
*                                   DEF_YES         Format alphabetic characters in lower case.
*
*               nul             Append terminating NULL-character (see Note #4) :
*
*                                   DEF_NO          Do NOT append terminating NULL-character to string.
*                                   DEF_YES                Append terminating NULL-character to string.
*
*               pstr            Pointer to character array to return formatted number string (see Note #5).
*
* Return(s)   : Pointer to formatted string, if NO errors (see Note #6).
*
*               Pointer to NULL,             otherwise.
*
* Caller(s)   : Str_FmtNbr_Int32U(),
*               Str_FmtNbr_Int32S().
*
* Note(s)     : (1) (a) The maximum number of digits to format for 32-bit integer numbers :
*
*
*                               Maximum Number of             [  log (Number)      ]
*                             32-bit Integer Digits  =  floor [ -------------- + 1 ]
*                                   to Format                 [   log (Base)       ]
*
*                               where
*                                       Number                  Number to format
*                                       Base            Base of number to format
*
*                   (b) (1) If the number of digits to format ('nbr_dig') is less than the number of
*                           significant integer digits of the number to format ('nbr'); then the most-
*                           significant digits of the formatted number will be truncated.
*
*                               Example :
*
*                                   nbr     = 23456
*                                   nbr_dig = 3
*
*                                   pstr    = "456"
*
*                       (2) If number to format ('nbr') is negative but the most-significant digits of
*                           the formatted number are truncated (see Note #2a); the negative sign still
*                           prefixes the truncated formatted number.
*
*                               Example :
*
*                                   nbr     = -23456
*                                   nbr_dig =  3
*
*                                   pstr    = "-456"
*$PAGE*
*               (2) The number's base MUST be between 2 & 36, inclusive.
*
*               (3) (a) Leading character option prepends leading characters prior to the first non-zero
*                       digit.  The number of leading characters is such that the total number of integer
*                       digits is equal to the requested number of integer digits to format ('nbr_dig').
*
*                   (b) Leading character MUST be a printable ASCII character.
*
*                   (c) (1) If the value of the number to format is     zero  ...
*                       (2) ... & the number of digits to format is non-zero, ...
*                       (3) ... but NO leading character available;           ...
*                       (4) ... then one digit of '0' value is formatted.
*
*                           This is NOT a leading character; but a single integer digit of '0' value.
*
*               (4) (a) NULL-character terminate option DISABLED prevents overwriting previous character
*                       array formatting.
*
*                   (b) WARNING: Unless 'pstr' character array is pre-/post-terminated, NULL-character
*                       terminate option DISABLED will cause character string run-on.
*
*               (5) (a) Format buffer size NOT validated; buffer overruns MUST be prevented by caller.
*
*                   (b) To prevent character buffer overrun :
*
*                           Character array size MUST be  >=  ('nbr_dig'         +
*                                                              1 negative sign   +
*                                                              1 'NUL' terminator)  characters
*               (6) String format terminates when :
*
*                   (a) Format string pointer is passed a NULL pointer.
*                       (1) No string format performed; NULL pointer returned.
*
*                   (b) Base is passed an invalid base (see Note #2).
*                       (1) No string format performed; NULL pointer returned.
*
*                   (c) Lead character is NOT a printable character (see Note #3b).
*                       (1) No string format performed; NULL pointer returned.
*
*                   (d) Number successfully formatted into character string array.
*********************************************************************************************************
*/

static  CPU_CHAR  *Str_FmtNbr_Int32 (CPU_INT32U    nbr,
                                     CPU_INT08U    nbr_dig,
                                     CPU_INT08U    nbr_base,
                                     CPU_BOOLEAN   nbr_neg,
                                     CPU_CHAR      lead_char,
                                     CPU_BOOLEAN   lower_case,
                                     CPU_BOOLEAN   nul,
                                     CPU_CHAR     *pstr)
{
    CPU_CHAR     *pstr_fmt;
    CPU_INT32U    nbr_fmt;
    CPU_INT32U    nbr_exp;
    CPU_INT32U    nbr_dig_val;
    CPU_INT08U    log_floor;
    CPU_INT08U    i;
    CPU_BOOLEAN   ovf;
    CPU_BOOLEAN   print_char;


/*$PAGE*/
    if (pstr == (CPU_CHAR *)0) {                                /* Rtn NULL if str ptr NULL (see Note #6a).             */
        return ((CPU_CHAR *)0);
    }
                                                                /* Rtn NULL if invalid base (see Note #6b).             */
    if ((nbr_base <  2) ||
        (nbr_base > 36)) {
        return ((CPU_CHAR *)0);
    }

    if (lead_char == (CPU_CHAR)'\0') {                          /* If lead char NOT avail,                   ...        */
        log_floor = 1;                                          /* ... calc floor of nbr digs (see Note #1a) ...        */
        nbr_exp   = nbr_base;
        ovf       = DEF_NO;
        while ((nbr_exp <= nbr) &&
               (ovf     == DEF_NO)) {
            if (nbr_exp <= Str_MultOvfThTbl_Int32U[nbr_base]) {
                nbr_exp *= nbr_base;
            } else {
                ovf      = DEF_YES;
            }
            log_floor++;
        }
        if (nbr_dig > log_floor) {
            nbr_dig = log_floor;                                /* ... & lim max nbr digs.                              */
        }

    } else {
        print_char = ASCII_IsPrint(lead_char);
        if (print_char != DEF_YES) {                            /* Rtn NULL if lead char non-printable (see Note #6c).  */
            return ((CPU_CHAR *)0);
        }
    }


    nbr_fmt  = nbr;
    pstr_fmt = pstr;

    if (nbr_neg == DEF_YES) {                                   /* If nbr neg                 ...                       */
        if (nbr_dig > 0) {                                      /* ... & if at least one dig, ...                       */
           *pstr_fmt++ = '-';                                   /* ... prepend a neg sign.                              */
        }
    }

    pstr_fmt += nbr_dig;                                        /* Start fmt from least significant dig.                */

    if (nul != DEF_NO) {                                        /* If NOT DISABLED, append NULL char (see Note #4).     */
       *pstr_fmt = (CPU_CHAR)'\0';
    }
    pstr_fmt--;

    if (nbr_dig > 0) {
        for (i = 1; i <= nbr_dig; i++) {                        /* Fmt str for desired nbr digs.                        */
            if ((nbr_fmt >  0) ||                               /* If fmt nbr > 0              (see Note #3c1), ...     */
                (nbr_dig == 1) ||                               /* ... OR exactly 1 dig to fmt (see Note #3c2), ...     */
                (i       == 1)) {                               /* ... OR on one's  dig to fmt;                 ...     */
                                                                /* ... calc & fmt dig val;                      ...     */
                nbr_dig_val = nbr_fmt % nbr_base;
                if (nbr_dig_val <= 9) {
                   *pstr_fmt-- = (CPU_CHAR)(nbr_dig_val + '0');
                } else {
                    if (lower_case !=  DEF_YES) {
                       *pstr_fmt--  = (CPU_CHAR)((nbr_dig_val - 10) + 'A');
                    } else {
                       *pstr_fmt--  = (CPU_CHAR)((nbr_dig_val - 10) + 'a');
                    }
                }

            } else if (lead_char != (CPU_CHAR)'\0') {           /* ... else if avail,                           ...     */
               *pstr_fmt-- = (CPU_CHAR)lead_char;               /* ... fmt lead char.                                   */
            }

            nbr_fmt /= nbr_base;                                /* Shift nbr to next more significant dig.              */
        }
    }


    return (pstr);                                              /* Rtn ptr to fmt'd str (see Note #6d).                 */
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        Str_ParseNbr_Int32()
*
* Description : Parse 32-bit integer from string.
*
* Argument(s) : pstr            Pointer to string (see Notes #1 & #2a).
*
*               pstr_next       Pointer to a variable to ... :
*
*                                   (a) Return a pointer to first character following the integer string,
*                                           if NO errors (see Note  #2a2B2);
*                                   (b) Return a pointer to 'pstr',
*                                           otherwise    (see Note  #2a2A2).
*
*               nbr_base        Base of number to parse  (see Notes #2a1B1 & #2a2B1).
*
*               nbr_signed      Indicates whether number to parse is signed :
*
*                                   DEF_NO                  Number is unsigned.
*                                   DEF_YES                 Number is   signed.
*
*               pnbr_neg        Pointer to a variable to return if the parsed (signed) number is negative :
*
*                                   DEF_NO                  Number is non-negative.
*                                   DEF_YES                 Number is     negative.
*
* Return(s)   : Parsed integer,      if integer parsed with NO overflow   (see Note #2a3A).
*
*               DEF_INT_32U_MAX_VAL, if integer parsed but     overflowed (see Note #2a3A1).
*
*               0,                   otherwise                            (see Note #2a3B).
*
* Caller(s)   : Str_ParseNbr_Int32U(),
*               Str_ParseNbr_Int32S().
*
* Note(s)     : (1) String buffer NOT modified.
*
*               (2) (a) IEEE Std 1003.1, 2004 Edition, Section 'strtol() : DESCRIPTION' states that "these
*                       functions shall convert the initial portion of the string pointed to by 'str' to a
*                       type long ... representation" :
*
*                       (1) "First, they decompose the input string into three parts" :
*
*                           (A) "An initial, possibly empty, sequence of white-space characters [as specified
*                                by isspace()]."
*
*                               (1) "The subject sequence is defined as the longest initial subsequence of the
*                                    input string, starting with the first non-white-space character that is of
*                                    the expected form.  The subject sequence shall contain no characters if the
*                                    input string is empty or consists entirely of white-space characters."
*
*                           (B) (1) "A subject sequence interpreted as an integer represented in some radix
*                                    determined by the value of 'base'" :
*
*                                   (a) "If the value of 'base' is 0, the expected form of the subject sequence
*                                        is that of a decimal constant, octal constant, or hexadecimal constant" :
*
*                                       (1) "A decimal constant begins with a non-zero digit, and consists of a
*                                            sequence of decimal digits."
*
*                                       (2) "An octal constant consists of the prefix '0' optionally followed by
*                                            a sequence of the digits '0' to '7' only."
*
*                                       (3) "A hexadecimal constant consists of the prefix '0x' or '0X' followed
*                                            by a sequence of the decimal digits and letters 'a' (or 'A') to 'f'
*                                            (or 'F') with values 10 to 15 respectively."
*
*                                   (b) "If the value of 'base' is between 2 and 36, the expected form of the
*                                        subject sequence is a sequence of letters and digits representing an
*                                        integer with the radix specified by 'base'" :
*
*                                       (1) (A) "The letters from 'a' (or 'A') to 'z' (or 'Z') inclusive are
*                                                ascribed the values 10 to 35"; ...
*                                           (B) "only letters whose ascribed values are less than that of base
*                                                are permitted."
*
*                                       (2) (A) "If the value of 'base' is 16, the characters '0x' or '0X' may
*                                                optionally precede the sequence of letters and digits."
*
*                                           (B) Although NO specification states that "if the value of 'base'
*                                               is" 8, the '0' character "may optionally precede the sequence
*                                               of letters and digits"; it seems reasonable to allow the '0'
*                                               character to be optionally parsed.
*$PAGE*
*                               (2) "A subject sequence .... may be preceded by a '+' or '-' sign."
*
*                                   (a) It does NOT seem reasonable to parse & convert a negative number
*                                       integer string into an unsigned integer.  However, a negative sign
*                                       for an unsigned integer will automatically be parsed as an invalid
*                                       character (see Note #2aC1).
*
*                           (C) (1) (a) "A final string of one or more unrecognized characters,"  ...
*                                   (b) "including the terminating null byte of the input string" ...
*                               (2) "other than a sign or a permissible letter or digit."
*
*                       (2) Second, "they shall attempt to convert the subject sequence to an integer" :
*
*                           (A) "If the subject sequence is empty or does not have the expected form" :
*
*                               (1) "no conversion is performed"; ...
*                               (2) "the value of 'str' is stored in the object pointed to by 'endptr',
*                                    provided that 'endptr' is not a null pointer."
*
*                           (B) "If the subject sequence has the expected form" :
*
*                               (1) (a) "and the value of 'base' is 0, the sequence of characters starting with
*                                        the first digit shall be interpreted as an integer constant."
*
*                                   (b) "and the value of 'base' is between 2 and 36, it shall be used as the
*                                        base for conversion, ascribing to each letter its value as given above"
*                                        (see Note #2a1B1b1A).
*
*                               (2) "A pointer to the final string shall be stored in the object pointed to by
*                                   'endptr', provided that 'endptr' is not a null pointer."
*
*                       (3) Lastly, IEEE Std 1003.1, 2004 Edition, Section 'strtol() : RETURN VALUE' states that :
*
*                           (A) "Upon successful completion, these functions shall return the converted value."
*                               (1) "If the correct value is outside the range of representable values, {LONG_MIN}
*                                    [or] {LONG_MAX} ... shall be returned."
*
*                           (B) "If no conversion could be performed, 0 shall be returned."
*
*                   (b) (1) IEEE Std 1003.1, 2004 Edition, Section 'strtoul() : ERRORS' states that "these functions
*                           shall fail if" :
*
*                           (A) "[EINVAL] - The value of 'base' is not supported."
*
*                           (B) "[ERANGE] - The value to be returned is not representable."
*
*                       (2) IEEE Std 1003.1, 2004 Edition, Section 'strtoul() : ERRORS' states that "these functions
*                           may fail if" :
*
*                           (A) "[EINVAL] - No conversion could be performed."
*$PAGE*
*               (3) Return integer value & next string pointer should be used to diagnose parse success or failure :
*
*                   (a) Valid parse string integer :
*
*                           pstr      = "     ABCDE xyz"
*                           nbr_base  = 16
*
*                           nbr       = 703710
*                           pstr_next = " xyz"
*
*
*                   (b) Invalid parse string integer :
*
*                           pstr      = "     ABCDE"
*                           nbr_base  = 10
*
*                           nbr       =  0
*                           pstr_next = pstr = "     ABCDE"
*
*
*                   (c) Valid hexadecimal parse string integer :
*
*                           pstr      = "     0xGABCDE"
*                           nbr_base  = 16
*
*                           nbr       =  0
*                           pstr_next = "xGABCDE"
*
*
*                   (d) Valid decimal parse string integer ('0x' prefix ignored
*                               following invalid hexadecimal characters) :
*
*                           pstr      = "     0xGABCDE"
*                           nbr_base  =  0
*
*                           nbr       =  0
*                           pstr_next = "xGABCDE"
*
*
*                   (e) Valid decimal parse string integer ('0'  prefix ignored
*                               following invalid octal       characters) :
*
*                           pstr      = "     0GABCDE"
*                           nbr_base  =  0
*
*                           nbr       =  0
*                           pstr_next = "GABCDE"
*
*
*                   (f) Parse string integer overflow :
*
*                           pstr      = "   12345678901234567890*123456"
*                           nbr_base  = 10
*
*                           nbr       = DEF_INT_32U_MAX_VAL
*                           pstr_next = "*123456"
*
*
*                   (g) Parse string integer underflow :
*
*                           pstr      = "  -12345678901234567890*123456"
*                           nbr_base  = 10
*
*                           nbr       = DEF_INT_32S_MIN_VAL
*                           pstr_next = "*123456"
*
*
*               (4) String parse terminates when :
*
*                   (a) Base passed an invalid base (see Note #2a1B1b).
*                       (1) No conversion performed; zero returned.
*
*                   (b) (1) Parse string passed a NULL pointer OR empty integer sequence (see Note #2a2A).
*                           (A) No conversion performed; zero returned.
*
*                       (2) Invalid parse string character found (see Note #2a1C).
*                           (A) Parsed integer returned.
*                           (B) 'pstr_next' points to invalid character.
*
*                       (3) Entire  parse string converted (see Note #2a2B).
*                           (A) Parsed integer returned.
*                           (B) 'pstr_next' points to terminating NULL character.
*
*               (5) Pointers to variables that return values MUST be initialized PRIOR to all other
*                   validation or function handling in case of any error(s).
*********************************************************************************************************
*/
/*$PAGE*/
static  CPU_INT32U  Str_ParseNbr_Int32 (CPU_CHAR      *pstr,
                                        CPU_CHAR     **pstr_next,
                                        CPU_INT08U     nbr_base,
                                        CPU_BOOLEAN    nbr_signed,
                                        CPU_BOOLEAN   *pnbr_neg)
{
    CPU_CHAR     *pstr_parse;
    CPU_CHAR     *pstr_parse_nbr;
    CPU_CHAR      parse_char;
    CPU_INT08U    parse_dig;
    CPU_INT32U    nbr;
    CPU_BOOLEAN   nbr_dig;
    CPU_BOOLEAN   nbr_alpha;
    CPU_BOOLEAN   nbr_hex;
    CPU_BOOLEAN   nbr_hex_lower;
    CPU_BOOLEAN   whitespace;
    CPU_BOOLEAN   neg;
    CPU_BOOLEAN   ovf;
    CPU_BOOLEAN   done;

                                                                /* Init rtn vals for err (see Note #5).                  */
    if (pstr_next != (CPU_CHAR **)0) {
       *pstr_next  = (CPU_CHAR  *)pstr;
    }

    if (pnbr_neg != (CPU_BOOLEAN *)0) {
       *pnbr_neg  = (CPU_BOOLEAN  )DEF_NO;
    }


    if (pstr == (CPU_CHAR *)0) {                                /* Rtn zero if str ptr NULL (see Note #4b1).            */
        return ((CPU_INT32U)0);
    }
                                                                /* Rtn zero if invalid base (see Note #4a).             */
    if ((nbr_base == 1) ||
        (nbr_base > 36)) {
        return ((CPU_INT32U)0);
    }


                                                                /* ------------- IGNORE PRECEDING CHAR(S) ------------- */
    pstr_parse = pstr;                                          /* Save ptr to init'l str for err (see Note #2a2A2).    */

    whitespace = ASCII_IsSpace(*pstr_parse);
    while (whitespace == DEF_YES) {                             /* Ignore initial white-space char(s) [see Note #2a1A]. */
        pstr_parse++;
        whitespace = ASCII_IsSpace(*pstr_parse);
    }

    switch (*pstr_parse) {
        case '+':                                               /* Ignore   pos sign (see Note #2a1B2).                 */
             pstr_parse++;
             neg = DEF_NO;
             break;


        case '-':                                               /* Validate neg sign (see Note #2a1B2a).                */
             if (nbr_signed == DEF_YES) {
                 pstr_parse++;
             }
             neg = DEF_YES;
             break;


        default:
             neg = DEF_NO;
             break;
    }


/*$PAGE*/
                                                                /* --------- IGNORE NBR BASE PRECEDING CHAR(S) -------- */
    pstr_parse_nbr = pstr_parse;                                /* Save ptr to str's nbr (see Note #2a1A1).             */

    switch (nbr_base) {
        case  0:                                                /* Determine unspecified nbr base (see Notes #2a1B1a).  */
             if (*pstr_parse == '0') {                          /* If avail, ...                                        */
                  pstr_parse++;                                 /* ... adv past '0'  prefix (see Note #2a1B1b2).        */
                  switch (*pstr_parse) {
                      case 'x':                                 /* For '0x' prefix, ...                                 */
                      case 'X':
                           nbr_base   = 16;                     /* ... set nbr base = 16    (see Note #2a1B1a3).        */
                           parse_char = (CPU_CHAR)(*(pstr_parse + 1));
                           nbr_hex    =  ASCII_IsDigHex(parse_char);
                           if (nbr_hex == DEF_YES) {            /* If next char is valid hex dig, ...                   */
                               pstr_parse++;                    /* ... adv past '0x' prefix (see Note #2a1B1b2A).       */
                           }
                           break;


                      default:                                  /* For '0'  prefix, ...                                 */
                           nbr_base =  8;                       /* ... set nbr base =  8    (see Note #2a1B1a2).        */
                           break;
                  }

             } else {                                           /* For non-'0' prefix, ...                              */
                 nbr_base = 10;                                 /* ... set nbr base = 10    (see Note #2a1B1a1).        */
             }
             break;


        case  8:                                                /* See Note #2a1B1a2.                                   */
             if (*pstr_parse == '0') {                          /* If avail, ...                                        */
                  pstr_parse++;                                 /* ... adv past '0'  prefix (see Note #2a1B1b2B).       */
             }
             break;


        case 16:                                                /* See Note #2a1B1a3.                                   */
             if (*pstr_parse == '0') {                          /* If avail, ...                                        */
                  pstr_parse++;                                 /* ... adv past '0'  prefix (see Note #2a1B1b2).        */
                  switch (*pstr_parse) {
                      case 'x':
                      case 'X':
                           parse_char = (CPU_CHAR)(*(pstr_parse + 1));
                           nbr_hex    =  ASCII_IsDigHex(parse_char);
                           if (nbr_hex == DEF_YES) {            /* If next char is valid hex dig, ...                   */
                               pstr_parse++;                    /* ... adv past '0x' prefix (see Note #2a1B1b2A).       */
                           }
                           break;


                      default:
                           break;
                  }
             }
             break;


        default:                                                /* See Note #2a1B1b.                                    */
             break;
    }


/*$PAGE*/
                                                                /* ------------------ PARSE INT STR ------------------- */
    nbr  = 0;
    ovf  = DEF_NO;
    done = DEF_NO;

    while (done == DEF_NO) {                                    /* Parse str for desired nbr base digs (see Note #2a2). */
        parse_char = (CPU_CHAR)*pstr_parse;
        nbr_alpha  =  ASCII_IsAlphaNum(parse_char);
        if (nbr_alpha == DEF_YES) {                             /* If valid alpha num nbr dig avail, ...                */
                                                                /* ... convert parse char into nbr dig.                 */
            nbr_dig = ASCII_IsDig(parse_char);
            if (nbr_dig == DEF_YES) {
                parse_dig = (CPU_INT08U)(parse_char - '0');
            } else {
                nbr_hex_lower = ASCII_IsLower(parse_char);
                if (nbr_hex_lower == DEF_YES) {
                    parse_dig = (CPU_INT08U)((parse_char - 'a') + 10);
                } else {
                    parse_dig = (CPU_INT08U)((parse_char - 'A') + 10);
                }
            }

            if (parse_dig < nbr_base) {                         /* If parse char valid for nbr base ...                 */
                if (ovf == DEF_NO) {                            /* ... & nbr NOT yet ovf'd,         ...                 */
                    if (nbr <= Str_MultOvfThTbl_Int32U[nbr_base]) {
                                                                /* ... merge parse char dig into nbr.                   */
                        nbr *= nbr_base;
                        nbr += parse_dig;
                        if (nbr < parse_dig) {
                            ovf = DEF_YES;
                        }
                    } else {
                        ovf = DEF_YES;
                    }
                }
                pstr_parse++;

            } else {                                            /* Invalid         char parsed (see Note #2a1C1a).      */
                done = DEF_YES;
            }

        } else {                                                /* Invalid OR NULL char parsed (see Note #2a1C1).       */
            done = DEF_YES;
        }
    }

    if (ovf == DEF_YES) {                                       /* If nbr ovf'd, ...                                    */
        nbr  = DEF_INT_32U_MAX_VAL;                             /* ... rtn max int val (see Note #2a3A1).               */
    }


    if (pstr_next != (CPU_CHAR **)0) {
        if (pstr_parse != pstr_parse_nbr) {                     /* If final parse str != init'l parse nbr str,       .. */
           *pstr_next   = pstr_parse;                           /* .. rtn   parse str's next char (see Note #2a2B2); .. */
        } else {
           *pstr_next   = pstr;                                 /* .. else rtn initial parse str  (see Note #2a2A2).    */
        }
    }

    if (pnbr_neg != (CPU_BOOLEAN *)0) {
       *pnbr_neg  = (CPU_BOOLEAN  )neg;                         /* Rtn neg nbr status.                                  */
    }


    return (nbr);
}

