#ifndef __INCLUDE_GUI_REAL_GRAPH_H__
#define __INCLUDE_GUI_REAL_GRAPH_H__
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "SGUI_Common.h"
#include "SGUI_Basic.h"

//=======================================================================//
//= Macro definition.                                                   =//
//=======================================================================//

//=======================================================================//
//= Data type definition.                                               =//
//=======================================================================//
#ifdef __cplusplus
extern "C"{
#endif
typedef struct
{
    SGUI_INT                yAxisMax; // Max value on screen display.
    SGUI_INT                yAxisMin; // Min value on screen display.
    SGUI_BOOL               EnableBaseline; // Enable to show zero-point line.
    SGUI_INT                xAxisStepPixel;
    SGUI_INT                ValueArea; // Display value range, auto calculate when initialize.
}SGUI_RTGRAPH_CONTROL;

typedef struct
{
    SGUI_INT                iValue;
    SGUI_INT                iYCoord;
}SGUI_RTGRAPH_POINT;

typedef struct
{
    SGUI_RTGRAPH_POINT*     arrValue;
    SGUI_INT                iSize;
}SGUI_RTGRAPH_RECORDER;

typedef struct
{
    SGUI_RTGRAPH_RECORDER   stRecorder;
    SGUI_INT                BaseLineValue; // Base line value on graph map.
}SGUI_RTGRAPH_DATA;

typedef struct
{
    SGUI_RECT               stLayout;
    SGUI_RTGRAPH_DATA       stData;
    SGUI_RTGRAPH_CONTROL    stControl;
}SGUI_RTGRAPH;
#ifdef __cplusplus
}
#endif

//=======================================================================//
//= Public function declaration.                                        =//
//=======================================================================//
#ifdef __cplusplus
extern "C"{
#endif
void    SGUI_RealtimeGraph_Initialize(SGUI_RTGRAPH* pstRTGraph);
void    SGUI_RealtimeGraph_Repaint(SGUI_SCR_DEV* pstDeviceIF, SGUI_RTGRAPH* pstRTGraph);
void    SGUI_RealtimeGraph_PushRear(SGUI_RTGRAPH* pstRTGraph, SGUI_INT iNewValue);
void    SGUI_RealtimeGraph_PushFront(SGUI_RTGRAPH* pstRTGraph, SGUI_INT iNewValue);
void    SGUI_RealtimeGraph_Cleanup(SGUI_RTGRAPH* pstRTGraph);
#ifdef __cplusplus
}
#endif

#endif // __INCLUDE_GUI_REAL_GRAPH_H__
