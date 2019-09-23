/*****************************************************************************
** File:        StatusBar.cpp
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
#include "StatusBar.h"
#include "Resource.h"
#include <windows.h>
#include <CommCtrl.h>


StatusBar::StatusBar(HINSTANCE hInstance, HWND owner, std::vector<int>& fieldWidthVec) :
    fieldWidth(fieldWidthVec)
{
    hwnd = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 
                          0, 0, 50, 50, owner, (HMENU) 443, hInstance, NULL);

    updatePosition();
}

StatusBar::~StatusBar()
{
    DestroyWindow(hwnd);
}

void StatusBar::setField(int fieldIndex, const char* text)
{
    SendMessage(hwnd, SB_SETTEXT, fieldIndex, (LPARAM)text);
}

void StatusBar::updatePosition()
{
    MoveWindow(hwnd, 0, 0, 0, 0, TRUE);

    RECT cr;
    GetClientRect(GetParent(hwnd), &cr);

    int segments = fieldWidth.size();
    int parts[64];

    parts[segments - 1] = cr.right;

    for (int i = segments - 1; i > 0; i--) {
        parts[i - 1] = parts[i] - fieldWidth[i];
    }
    
    SendMessage(hwnd, SB_SETPARTS, (WPARAM)segments, (LPARAM)parts); 
}

void StatusBar::show()
{
    ShowWindow(hwnd, true);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void StatusBar::hide()
{
    ShowWindow(hwnd, false);
}

int StatusBar::getHeight()
{
    if (!IsWindowVisible(hwnd)) {
        return 0;
    }
    RECT r;
    GetWindowRect(hwnd, &r);
    return r.bottom - r.top;
}
