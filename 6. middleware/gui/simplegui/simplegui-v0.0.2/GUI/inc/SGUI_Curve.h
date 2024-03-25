#ifndef _INCLUDE_GUI_CURVE_H__
#define _INCLUDE_GUI_CURVE_H__
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "SGUI_Common.h"
#include "SGUI_Basic.h"

//=======================================================================//
//= Data type definition.                                               =//
//=======================================================================//
#ifdef __cplusplus
extern "C"{
#endif
typedef struct
{
    SGUI_INT                iA;
    SGUI_INT                iB;
}SGUI_CURVE_COEFFICIENT;

typedef struct _sgui_st_curve_point_
{
    struct _sgui_st_curve_point_* pstPrev;
    SGUI_POINT              stPoint;
    SGUI_POINT              stPosition;
    struct _sgui_st_curve_point_* pstNext;
}SGUI_CURVE_POINT;

typedef struct
{
    SGUI_CURVE_POINT*       pstHead;
    SGUI_CURVE_POINT*       pstEnd;
    SGUI_SIZE               sCount;
}SGUI_CURVE_POINT_LIST;

typedef struct
{
    SGUI_RECT               stLayout;
    SGUI_RANGE              stXRange;
    SGUI_RANGE              stYRange;
}SGUI_CURVE_PARAM;

typedef struct
{
    SGUI_CURVE_POINT_LIST   stPoints;
    SGUI_CURVE_POINT*       pstFocused;
}SGUI_CURVE_DATA;

typedef struct
{
    SGUI_CURVE_PARAM        stParam;
    SGUI_CURVE_DATA         stData;
}SGUI_CURVE_STRUCT;
#ifdef __cplusplus
}
#endif
//=======================================================================//
//= Public function declaration.                                        =//
//=======================================================================//
#ifdef __cplusplus
extern "C"{
#endif
void                SGUI_Curve_Initialize(SGUI_CURVE_STRUCT* pstObj, const SGUI_CURVE_PARAM* pcstInitParam);
void                SGUI_Curve_Repaint(SGUI_SCR_DEV* pstDeviceIF, const SGUI_CURVE_STRUCT* pstObj);
void                SGUI_Curve_InitializePoint(SGUI_CURVE_POINT* pstNewPoint, SGUI_INT iX, SGUI_INT iY);
void                SGUI_Curve_CalculatePointPosition(SGUI_CURVE_STRUCT* pstObj, SGUI_CURVE_POINT* pstPoint);
void                SGUI_Curve_AddPoint(SGUI_CURVE_STRUCT* pstObj, SGUI_CURVE_POINT* pstNewPoint);
SGUI_BOOL           SGUI_Curve_InsertBeforePoint(SGUI_CURVE_STRUCT* pstObj, SGUI_CURVE_POINT* pstNewPoint);
SGUI_BOOL           SGUI_Curve_InsertAfterPoint(SGUI_CURVE_STRUCT* pstObj, SGUI_CURVE_POINT* pstNewPoint);
SGUI_BOOL           SGUI_Curve_RemovePoint(SGUI_CURVE_STRUCT* pstObj, SGUI_CURVE_POINT* pstPoint);
SGUI_CURVE_POINT*   SGUI_Curve_FocusPoint(SGUI_CURVE_STRUCT* pstObj, SGUI_INT iIndex);
SGUI_BOOL           SGUI_Curve_UpdateFocusPoint(SGUI_CURVE_STRUCT* pstObj, SGUI_INT iX, SGUI_INT iY);
void                SGUI_Curve_HighlightFocus(SGUI_SCR_DEV* pstDeviceIF, SGUI_CURVE_STRUCT* pstObj);
SGUI_INT            SGUI_Curve_GetDependentValue(SGUI_CURVE_STRUCT* pstObj, SGUI_INT iArgument);
#ifdef __cplusplus
}
#endif

//=======================================================================//
//= Macro function.                                                     =//
//=======================================================================//
#define             SGUI_Curve_FocusedPoint(OBJ)        ((OBJ)->stData.pstFocused)

#endif // __INCLUDE_GUI_REAL_GRAPH_H__
