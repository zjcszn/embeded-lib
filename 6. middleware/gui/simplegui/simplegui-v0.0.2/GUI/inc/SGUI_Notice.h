#ifndef __INCLUDE_GUI_NOTICE_H__
#define __INCLUDE_GUI_NOTICE_H__
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "SGUI_Common.h"
#include "SGUI_Basic.h"
#include "SGUI_Text.h"

//=======================================================================//
//= Data type definition.                                               =//
//=======================================================================//
#ifdef __cplusplus
extern "C"{
#endif
typedef struct
{
    SGUI_CSZSTR             cszNoticeText;
    const SGUI_BMP_RES*     pstIcon;
    SGUI_RECT               stLayout;
}SGUI_NOTICT_BOX;
#ifdef __cplusplus
}
#endif

//=======================================================================//
//= Public function declaration.                                        =//
//=======================================================================//
#ifdef __cplusplus
extern "C"{
#endif
SGUI_SIZE   SGUI_Notice_Repaint(SGUI_SCR_DEV* pstDeviceIF, SGUI_NOTICT_BOX* pstObject, const SGUI_FONT_RES* pstFontRes, SGUI_INT uiTextOffset);
void        SGUI_Notice_FitArea(SGUI_SCR_DEV* pstDeviceIF, SGUI_RECT* pstFitArea);
#ifdef __cplusplus
}
#endif

#endif // __INCLUDE_GUI_NOTICE_H__
