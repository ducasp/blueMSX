/*****************************************************************************
** File:        DbgWindow.cpp
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
#include "DbgWindow.h"
#include "Resource.h"
#include "IniFileParser.h"
#include <map>

using namespace std;

#define MAX_WINDOWS 16

typedef map<HWND, DbgWindow*> WindowMap;

static WindowMap windows;
static DbgWindow* isCreating = NULL;

static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{
    if (isCreating != NULL && iMsg == WM_CREATE )
    {
        isCreating->hwnd = hwnd;
        windows[hwnd] = isCreating;
    }

    WindowMap::iterator i = windows.find(hwnd);
    if (i != windows.end()) {
        DbgWindow* window = i->second;
        if( iMsg == WM_WINDOWPOSCHANGED ) {
            window->updateWindowPos((WINDOWPOS*)lParam);
        }
        return window->wndProc(iMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

DbgWindow::DbgWindow(HINSTANCE hInst, HWND wndOwner, const std::string& name, const std::string& ininame,
                     int defX, int defY, int defW, int defH, int defV) : 
    hInstance(hInst), owner(wndOwner), editEnabled(false), iniName(ininame), winName(name)
{
    static WNDCLASSEX wndClass;

    wndClass.cbSize         = sizeof(wndClass);
    wndClass.style          = CS_VREDRAW;
    wndClass.lpfnWndProc    = staticWndProc;
    wndClass.cbClsExtra     = 0;
    wndClass.cbWndExtra     = 0;
    wndClass.hInstance      = hInstance;
    wndClass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BLUEMSX));
    wndClass.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BLUEMSX));
    wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground  = NULL;
    wndClass.lpszMenuName   = NULL;
    wndClass.lpszClassName  = "msxdbgsub";

    RegisterClassEx(&wndClass);
    
    x       = iniFileGetInt( iniName.c_str(), "x", defX );
    y       = iniFileGetInt( iniName.c_str(), "y", defY );
    width   = iniFileGetInt( iniName.c_str(), "width", defW );
    height  = iniFileGetInt( iniName.c_str(), "height", defH );
    visible = iniFileGetInt( iniName.c_str(), "visible", defV );
}

void DbgWindow::init()
{
    isCreating = this;
    hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, "msxdbgsub", winName.c_str(), 
                          WS_OVERLAPPED | WS_CLIPSIBLINGS | WS_CHILD | WS_BORDER | WS_THICKFRAME | WS_DLGFRAME, 
                          x, y, width, height, owner, NULL, hInstance, NULL);
    isCreating = NULL;
    if( visible ) {
        show();
    }
}

DbgWindow::~DbgWindow()
{
    iniFileWriteInt( iniName.c_str(), "x",       x);
    iniFileWriteInt( iniName.c_str(), "y",       y);
    iniFileWriteInt( iniName.c_str(), "width",   width);
    iniFileWriteInt( iniName.c_str(), "height",  height);
    iniFileWriteInt( iniName.c_str(), "visible", visible);

    for (WindowMap::iterator i = windows.begin(); i != windows.end(); ++i) {
        if (i->first == hwnd) {
            windows.erase(i);
            break;
        }
    }
}

void DbgWindow::show()
{
    visible = 1;
    ShowWindow(hwnd, true);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void DbgWindow::hide()
{
    visible = 0;
    ShowWindow(hwnd, false);
}

bool DbgWindow::isVisible()
{
    return visible != 0;
}

void DbgWindow::enableEdit()
{
    editEnabled = true;
}

void DbgWindow::disableEdit()
{
    editEnabled = false;
}

void DbgWindow::updateWindowPos(WINDOWPOS* windowPos) 
{
    x       = windowPos->x;
    y       = windowPos->y;
    width   = windowPos->cx;
    height  = windowPos->cy;
}

