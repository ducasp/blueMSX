/*****************************************************************************
** File:        Toolbar.cpp
**
** Author:      Daniel Vik
**
** Copyright (C) 2003-2004 Daniel Vik
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/
#include "ToolBar.h"

#ifndef BTNS_BUTTON
#define BTNS_BUTTON     TBSTYLE_BUTTON
#endif
#ifndef BTNS_DROPDOWN
#define BTNS_DROPDOWN   TBSTYLE_DROPDOWN
#endif
#ifndef BTNS_SEP
#define BTNS_SEP        TBSTYLE_SEP
#endif

Toolbar::Toolbar(HINSTANCE hInstance, HWND owner, int bitmapId, COLORREF transparentColor, int backgroundId) :
    hBackground(NULL)
{
    INITCOMMONCONTROLSEX icex;
    HBITMAP   hBtn;
 
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    if (backgroundId != -1) {
        hBackground = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(backgroundId), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    }

    hImglBtn = ImageList_Create(18, 18, ILC_COLOR24 | ILC_MASK , 3, 1);
    hBtn = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(bitmapId), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    ImageList_AddMasked(hImglBtn, hBtn, transparentColor);
    DeleteObject(hBtn);

    hwnd = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL, 
        WS_CHILD | TBSTYLE_TOOLTIPS | WS_BORDER | TBSTYLE_FLAT | CCS_ADJUSTABLE, 
        0, 0, 0, 0, owner, (HMENU)12029, hInstance, NULL); 

    SendMessage(hwnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0); 
    SendMessage(hwnd, TB_SETSTYLE, 0, SendMessage(hwnd, TB_GETSTYLE, 0,0 ) & ~TBSTYLE_TRANSPARENT);

    SendMessage(hwnd, TB_SETBITMAPSIZE, 0, MAKELONG(18,18));
    SendMessage(hwnd, TB_SETBUTTONSIZE, 0, MAKELONG(18,18));
    SendMessage(hwnd, TB_AUTOSIZE, 0, 0L);
    
    SendMessage(hwnd, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)hImglBtn);
    SendMessage(hwnd, TB_SETEXTENDEDSTYLE, (WPARAM)0, TBSTYLE_EX_DRAWDDARROWS );
}

Toolbar::~Toolbar()
{
    if (hBackground != NULL) {
        DeleteObject(hBackground);
    }
    ImageList_Destroy(hImglBtn);
    DestroyWindow(hwnd);
}

void Toolbar::addButton(int bitmap, int command, int dropdown, int insertBefore)
{
    if (insertBefore == -1) {
        TBBUTTON button;
        button.iBitmap = bitmap; 
        button.idCommand = command; 
        button.fsState = TBSTATE_ENABLED; 
        button.fsStyle = dropdown ? BTNS_DROPDOWN : 0; 
        buttons.push_back(button);
        SendMessage(hwnd, TB_INSERTBUTTON, buttons.size() - 1, (LPARAM)(LPTBBUTTON)&button);
    }
}

void Toolbar::addSeparator(int insertBefore)
{
    if (insertBefore == -1) {
        TBBUTTON button;
        button.iBitmap = 0; 
        button.idCommand = 0; 
        button.fsState = TBSTATE_ENABLED; 
        button.fsStyle = BTNS_SEP; 
        buttons.push_back(button);
        SendMessage(hwnd, TB_INSERTBUTTON, buttons.size() - 1, (LPARAM)(LPTBBUTTON)&button);
    }
}

void Toolbar::show()
{
    ShowWindow(hwnd, true);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void Toolbar::hide()
{
    ShowWindow(hwnd, false);
}

int Toolbar::getHeight()
{
    if (!IsWindowVisible(hwnd)) {
        return 0;
    }
    RECT r;
    GetWindowRect(hwnd, &r);
    return r.bottom - r.top - 1;
}

void Toolbar::updatePosition()
{
    RECT parentRect;
    RECT rect;

    GetClientRect(GetParent(hwnd), &parentRect);
    GetWindowRect(hwnd, &rect);

    SetWindowPos(hwnd, NULL, 0, 0, parentRect.left - parentRect.right, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

void Toolbar::enableItem(int item, bool enable)
{
    SendMessage(hwnd, TB_DELETEBUTTON, item, 0);

    buttons[item].fsState = enable ? TBSTATE_ENABLED : 0;
    SendMessage(hwnd, TB_INSERTBUTTON, item, (LPARAM)(LPTBBUTTON)&(buttons[item]));
}

void Toolbar::disableItem(int item)
{
    enableItem(item, false);
}

void Toolbar::onWmNotify(LPARAM lParam)
{
    if (hBackground == NULL) {
        return;
    }

    LPNMTBCUSTOMDRAW lptbcd = (NMTBCUSTOMDRAW *)lParam;

    if (lptbcd->nmcd.dwDrawStage == CDDS_PREPAINT) {
        HDC hdc = lptbcd->nmcd.hdc;
        BITMAP bm;
        HBITMAP hBitmap;
        RECT r;

        HDC hMemDC = CreateCompatibleDC(hdc);
        hBitmap = (HBITMAP)SelectObject(hMemDC, hBackground);
        GetObject(hBackground, sizeof(BITMAP), (PSTR)&bm);

        GetClientRect(hwnd, &r);
        int width = r.right - r.left;
        int height = r.bottom - r.top;
        for (int i = 0; i < width; i += bm.bmWidth) {
            BitBlt(hdc, i, 0, bm.bmWidth, height, hMemDC, 0, 0, SRCCOPY);
        }
        SelectObject(hMemDC, hBitmap);
        DeleteDC(hMemDC);
    }
}
