/*****************************************************************************
** File:        Breakpoints.cpp
**
** Author:      Daniel Vik
**
** Copyright (C) 2003-2012s Daniel Vik
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
#include "Breakpoints.h"
#include "Resource.h"
#include "Language.h"
#include "EditControls.h"
#include "InputDialogs.h"
#include <stdio.h>
#include <string>
#include <list>

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define TB_NEW_BREAKPOINT  37030
#define TB_NEW_WATCHPOINT  37031
#define TB_DELETE_BREAKPOINT  37032

extern void DebuggerUpdate();

namespace {



class BitmapIcons 
{
public:
    BitmapIcons(HINSTANCE hInstance, int id, int count) {
        HBITMAP hBitmap = (HBITMAP)LoadBitmap(hInstance, MAKEINTRESOURCE(id));
        hdcw = GetWindowDC(NULL);
        hMemDC = CreateCompatibleDC(hdcw);
        SelectObject(hMemDC, hBitmap);
        
        BITMAP bmp; 
        GetObject(hBitmap, sizeof(BITMAP), (PSTR)&bmp);
        width  = bmp.bmWidth / count;
        height = bmp.bmHeight;
    }

    ~BitmapIcons() {
        if (hMemDC) { DeleteDC(hMemDC); hMemDC=NULL; }
        if (hdcw) { ReleaseDC(NULL, hdcw); hdcw=NULL; }
    }

    void drawIcon(HDC hdc, int x, int y, int index) {
        BitBlt(hdc, x, y, width, height, hMemDC, width * index, 0, SRCCOPY);
    }

private:
    HDC hdcw;
    HDC hMemDC;
    int width;
    int height;
};

}


static Breakpoints* breakpointsInstance = NULL;
static BitmapIcons* bitmapIcons = NULL;


static LRESULT CALLBACK staticBreakpointsWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{
    if (breakpointsInstance != NULL) {
        return breakpointsInstance->breakpointsWndProc(hwnd, iMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void Breakpoints::updateWindowPositions()
{
    int toolHeight = 33;
    if (toolbar != NULL) {
        toolbar->updatePosition();
        toolHeight = toolbar->getHeight();
    }
    RECT r;
    GetClientRect(hwnd, &r);
    SetWindowPos(breakpointsHwnd, NULL, 0, toolHeight, r.right - r.left, r.bottom - r.top - toolHeight, SWP_NOZORDER);
}

static void updateTooltip(int id, char* str)
{
    switch (id) {
    case TB_NEW_BREAKPOINT:   sprintf(str, "New Breakpoint");        break;
    case TB_NEW_WATCHPOINT:   sprintf(str, "New Watchpoint");        break;
    case TB_DELETE_BREAKPOINT:   sprintf(str, "Delete Selected Breakpoint/Watchpoint");        break;
    }
}

LRESULT Breakpoints::wndProc(UINT iMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (iMsg) {
    case WM_CREATE:
        return 0;

    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        return 0;

    case WM_SIZE:
        updateWindowPositions();
        updateScroll();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case TB_NEW_BREAKPOINT:
            InputDialogs::NewBreakpoint();
            break;
        case TB_NEW_WATCHPOINT:
            InputDialogs::NewWatchpoint();
            break;
        case TB_DELETE_BREAKPOINT:
            if (GetEmulatorState() != EMULATOR_STOPPED) {
                if (selectedLine >= 0 && selectedLine < breakpoints.size()) {
                    clearBreakpoint(*breakpoints[selectedLine]);
                }
            }
            break;
        }
        break;

    case WM_NOTIFY:
        switch(((LPNMHDR)lParam)->code){
        case NM_CUSTOMDRAW:
            if (toolbar != NULL) {
                toolbar->onWmNotify(lParam);
            }
            return 0;

        case TTN_GETDISPINFO: 
            { 
                LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lParam; 
                lpttt->hinst = GetDllHinstance(); 
                updateTooltip(lpttt->hdr.idFrom, lpttt->szText);
            }
        }
        break;

    case WM_DESTROY:
        if (hBrushWhite) { DeleteObject(hBrushWhite); hBrushWhite=NULL; }
        if (hBrushLtGray) { DeleteObject(hBrushLtGray); hBrushLtGray=NULL; }
        if (hBrushDkGray) { DeleteObject(hBrushDkGray); hBrushDkGray=NULL; }
        if (hMemdc) { DeleteDC(hMemdc); hMemdc=NULL; }
        break;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

LRESULT Breakpoints::breakpointsWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (iMsg) {
    case WM_CREATE: {
        HDC hdc = GetDC(hwnd);
        hMemdc = CreateCompatibleDC(hdc);
        ReleaseDC(hwnd, hdc);
        colorBlack = RGB(0, 0, 0);
        colorGray  = RGB(128, 128, 128);
        colorRed   = RGB(255, 0, 0);
        colorWhite = RGB(255, 255, 255);
        SetBkMode(hMemdc, TRANSPARENT);
        hFont = CreateFont(-MulDiv(10, GetDeviceCaps(hMemdc, LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New");
        hFontBold = CreateFont(-MulDiv(10, GetDeviceCaps(hMemdc, LOGPIXELSY), 72), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New");
        
        hBrushWhite  = CreateSolidBrush(RGB(255, 255, 255));
        hBrushLtGray = CreateSolidBrush(RGB(239, 237, 222));
        hBrushDkGray = CreateSolidBrush(RGB(128, 128, 128));
        hBrushBlack  = CreateSolidBrush(RGB(200, 200, 255));

        SelectObject(hMemdc, hFont); 
        TEXTMETRIC tm;
        if (GetTextMetrics(hMemdc, &tm)) {
            textHeight = tm.tmHeight;
            textWidth = tm.tmMaxCharWidth;
        }
        
        return 0;
    }

    case WM_LBUTTONDOWN:
        {
            SetFocus(hwnd);

            SCROLLINFO si;

            si.cbSize = sizeof (si);
            si.fMask  = SIF_POS;
            GetScrollInfo (hwnd, SB_VERT, &si);

            int row = HIWORD(lParam) / textHeight;

            if (row + si.nPos  >= breakpoints.size()) {
                return 0;
            }

            if (GetEmulatorState() != EMULATOR_STOPPED) {
                selectedLine = row + si.nPos;
                if (LOWORD(lParam) < 22) {
                    toggleBreakpointEnable(*breakpoints[selectedLine]);
                }
                else {
                    invalidateContent();
                }
            }
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_SIZE:
        updateScroll();
        break;

    case WM_VSCROLL:
         scrollWindow(LOWORD(wParam));
         return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdcw = GetWindowDC(NULL);
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT r;
            GetClientRect(hwnd, &r);
            int top    = ps.rcPaint.top;
            int height = ps.rcPaint.bottom - ps.rcPaint.top;

            HBITMAP hBitmap = CreateCompatibleBitmap(hdcw, r.right, r.bottom);
            HBITMAP hBitmapOrig = (HBITMAP)SelectObject(hMemdc, hBitmap);
            
            SelectObject(hMemdc, hBrushLtGray);  
            PatBlt(hMemdc, 0, top, 21, height, PATCOPY);
            
            SelectObject(hMemdc, hBrushWhite); 
            PatBlt(hMemdc, 21, top, r.right - 21, height, PATCOPY);

            drawText(ps.rcPaint.top, ps.rcPaint.bottom);

            BitBlt(hdc, 0, top, r.right, height, hMemdc, 0, top, SRCCOPY);

            DeleteObject(SelectObject(hMemdc, hBitmapOrig));
            EndPaint(hwnd, &ps);
            ReleaseDC(NULL, hdcw);
        }
        return TRUE;

    case WM_DESTROY:
        if (hBrushWhite) { DeleteObject(hBrushWhite); hBrushWhite=NULL; }
        if (hBrushLtGray) { DeleteObject(hBrushLtGray); hBrushLtGray=NULL; }
        if (hBrushDkGray) { DeleteObject(hBrushDkGray); hBrushDkGray=NULL; }
        if (hBrushBlack) { DeleteObject(hBrushBlack); hBrushBlack=NULL; }
        if (hFont) { DeleteObject(hFont); hFont=NULL; }
        if (hFontBold) { DeleteObject(hFontBold); hFontBold=NULL; }
        if (hMemdc) { DeleteDC(hMemdc); hMemdc=NULL; }
        break;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void Breakpoints::initializeToolbar(HWND owner)
{
    if (toolbar != NULL) {
        delete toolbar;
    }
    toolbar = new Toolbar(GetDllHinstance(), owner, IDB_TOOLBAR, RGB(239, 237, 222));

    toolbar->addSeparator();
    toolbar->addButton(12,  TB_NEW_BREAKPOINT);
    toolbar->addButton(16,  TB_NEW_WATCHPOINT);
    toolbar->addSeparator();
    toolbar->addButton(17,  TB_DELETE_BREAKPOINT);
}

void Breakpoints::updateToolbar()
{
    if (toolbar == NULL) {
        return;
    }
    EmulatorState state = GetEmulatorState();

    toolbar->enableItem(1, state != EMULATOR_STOPPED);
    toolbar->enableItem(2, state != EMULATOR_STOPPED);
    toolbar->enableItem(4, state != EMULATOR_STOPPED && selectedLine >= 0);
}

Breakpoints::Breakpoints(HINSTANCE hInstance, HWND owner, SymbolInfo* symInfo) : 
    DbgWindow( hInstance, owner, 
        Language::windowBreakpoints, "Breakpoints Window", 23, 413, 507, 207, 0),
    toolbar(NULL), selectedLine(-1), runtoBreakpoint(-1),
    symbolInfo(symInfo)
{
    breakpointsInstance = this;

    static WNDCLASSEX wndClass;

    wndClass.cbSize         = sizeof(wndClass);
    wndClass.style          = 0;
    wndClass.lpfnWndProc    = staticBreakpointsWndProc;
    wndClass.cbClsExtra     = 0;
    wndClass.cbWndExtra     = 0;
    wndClass.hInstance      = hInstance;
    wndClass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BLUEMSX));
    wndClass.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BLUEMSX));
    wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground  = NULL;
    wndClass.lpszMenuName   = NULL;
    wndClass.lpszClassName  = "msxbreakpointsview";

    RegisterClassEx(&wndClass);

    init();

    if (bitmapIcons == NULL) {
        bitmapIcons = new BitmapIcons(hInstance, IDB_DASMICONS, 5);
    }

    initializeToolbar(hwnd);
    updateToolbar();
    toolbar->show();

    breakpointsHwnd = CreateWindow("msxbreakpointsview", "", 
                             WS_OVERLAPPED | WS_CLIPSIBLINGS | WS_CHILD, 
                             CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, hwnd, NULL, hInstance, NULL);

    
    ShowWindow(breakpointsHwnd, TRUE);

    updateWindowPositions();
    updateScroll();
    invalidateContent();
}

Breakpoints::~Breakpoints()
{
    breakpointsInstance = NULL;
    delete toolbar;
    toolbar = NULL;
}

void Breakpoints::disableEdit()
{
    DbgWindow::disableEdit();
}

void Breakpoints::updatePosition(RECT& rect)
{
    SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
}

void Breakpoints::invalidateContent()
{
    updateScroll();
    updateToolbar();

    InvalidateRect(breakpointsHwnd, NULL, TRUE);
}

void Breakpoints::updateContent()
{
    BreakpointInfo* bi;
    if (selectedLine >= 0 && selectedLine < breakpoints.size()) {
        bi = breakpoints[selectedLine];
    }

    selectedLine = -1;

    int firstHitIndex = -1;
    for (int i = 0; i < breakpoints.size(); ++i) {
        if (breakpoints[i]->breakpointHit) {
            firstHitIndex = i;
            break;
        }
        if (breakpoints[i] == bi) {
            selectedLine = i;
        }
    }
    if (firstHitIndex >= 0) {
        selectedLine = firstHitIndex;
    }
    updateToolbar();

    InvalidateRect(breakpointsHwnd, NULL, TRUE);
}

void Breakpoints::updateScroll() 
{
    RECT r;
    GetClientRect(breakpointsHwnd, &r);
    int visibleLines = r.bottom / textHeight;

    SCROLLINFO si;
    si.cbSize    = sizeof(SCROLLINFO);
    
    GetScrollInfo(breakpointsHwnd, SB_VERT, &si);
    int oldFirstLine = si.nPos;

    si.fMask     = SIF_PAGE | SIF_POS | SIF_RANGE;
    si.nMin      = 0;
    si.nMax      = breakpoints.size();
    si.nPage     = visibleLines;
    si.nPos      = 0;

    SetScrollInfo(breakpointsHwnd, SB_VERT, &si, TRUE);
    
    InvalidateRect(breakpointsHwnd, NULL, TRUE);
}

void Breakpoints::scrollWindow(int sbAction)
{
    SCROLLINFO si;

    si.cbSize = sizeof (si);
    si.fMask  = SIF_ALL;
    GetScrollInfo (breakpointsHwnd, SB_VERT, &si);
    int yPos = si.nPos;
    switch (sbAction) {
    case SB_TOP:
        si.nPos = si.nMin;
        break;
    case SB_BOTTOM:
        si.nPos = si.nMax;
        break;
    case SB_LINEUP:
        si.nPos -= 1;
        break;
    case SB_LINEDOWN:
        si.nPos += 1;
        break;
    case SB_PAGEUP:
        si.nPos -= si.nPage;
        break;
    case SB_PAGEDOWN:
        si.nPos += si.nPage;
        break;
    case SB_THUMBTRACK:
        si.nPos = si.nTrackPos;
        break;              
    default:
        break; 
    }

    si.fMask = SIF_POS;
    SetScrollInfo (breakpointsHwnd, SB_VERT, &si, TRUE);
    GetScrollInfo (breakpointsHwnd, SB_VERT, &si);
    if (si.nPos != yPos) {                    
        ScrollWindow(breakpointsHwnd, 0, textHeight * (yPos - si.nPos), NULL, NULL);
        UpdateWindow (breakpointsHwnd);
    }
}

void Breakpoints::drawText(int top, int bottom)
{
    SCROLLINFO si;
    
    si.cbSize = sizeof (si);
    si.fMask  = SIF_POS;
    GetScrollInfo (breakpointsHwnd, SB_VERT, &si);
    int yPos = si.nPos;
    int FirstLine = max (0, yPos + top / textHeight);
    int LastLine = min (breakpoints.size() - 1, yPos + bottom / textHeight);

    for (int i = FirstLine; i <= LastLine; i++) {
        if (i >= breakpoints.size()) {
            continue;
        }

        RECT r = { 28, textHeight * (i - yPos), 400, textHeight * (i + 1 - yPos) };
        if (i == selectedLine) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            SelectObject(hMemdc, hBrushBlack); 
            PatBlt(hMemdc, 21, r.top, rc.right - 21, r.bottom - r.top, PATCOPY);
        }

        if (breakpoints[i]->enabled) {
            bitmapIcons->drawIcon(hMemdc, 4, r.top, 1);
        }
        else {
            bitmapIcons->drawIcon(hMemdc, 4, r.top, 2);
        }
        
        char breakpointText[64];
        sprintf(breakpointText, "%s: ", breakpoints[i]->getTypeAsString());

        SetTextColor(hMemdc, i == selectedLine ? colorWhite : colorBlack);
        SelectObject(hMemdc, hFontBold);
        DrawText(hMemdc, breakpointText, strlen(breakpointText), &r, DT_LEFT);
        r.left  += strlen(breakpointText) * textWidth;

        char labelText[64];
        if (strlen(breakpoints[i]->label)) {
            sprintf(labelText, "%s (%.4Xh)", breakpoints[i]->label, breakpoints[i]->address);
        }
        else {
            sprintf(labelText, "%.4Xh", breakpoints[i]->address);
        }
        
        if (breakpoints[i]->type == BreakpointInfo::BREAKPOINT) {
            sprintf(breakpointText, "%s", labelText);
        }
        else {
            sprintf(breakpointText, "%s @ %s %s %s", breakpoints[i]->getSizeAsString(), labelText, 
                breakpoints[i]->getConditionAsString(),
                breakpoints[i]->getReferenceValueAsString());
        }

        SelectObject(hMemdc, hFont);
        SelectObject(hMemdc, hFontBold);
        DrawText(hMemdc, breakpointText, strlen(breakpointText), &r, DT_LEFT);
    }
}

void Breakpoints::SetBreakpoint(int address) {
    if (breakpointsInstance != NULL) {
        breakpointsInstance->setBreakpoint(MakeBreakpoint(address));
    }
}

void Breakpoints::ClearBreakpoint(int address) {
    if (breakpointsInstance != NULL) {
        breakpointsInstance->clearBreakpoint(MakeBreakpoint(address));
    }
}

void Breakpoints::ToggleBreakpointEnable(int address) {
    if (breakpointsInstance != NULL) {
        breakpointsInstance->toggleBreakpointEnable(MakeBreakpoint(address));
    }
}

bool Breakpoints::IsBreakpointUnset(int address) {
    if (breakpointsInstance == NULL) return true;
    BreakpointInfo* bi = breakpointsInstance->find(MakeBreakpoint(address));
    return bi == NULL;
}

bool Breakpoints::IsBreakpointSet(int address) {
    if (breakpointsInstance == NULL) return false;
    BreakpointInfo* bi = breakpointsInstance->find(MakeBreakpoint(address));
    return bi != NULL && bi->enabled;
}

bool Breakpoints::IsBreakpointDisabled(int address) {
    if (breakpointsInstance == NULL) return false;
    BreakpointInfo* bi = breakpointsInstance->find(MakeBreakpoint(address));
    return bi != NULL && !bi->enabled;
}

Breakpoints::BreakpointInfo* Breakpoints::find(const Breakpoints::BreakpointInfo& breakpointInfo) {
    for (std::vector<BreakpointInfo*>::iterator i = breakpoints.begin(); i != breakpoints.end(); ++i) {
        if ((*i)->address == breakpointInfo.address && (*i)->type == breakpointInfo.type) {
            return *i;
        }
    }
    return NULL;
}

const Breakpoints::BreakpointInfo& Breakpoints::MakeBreakpoint(int address) {
    static Breakpoints::BreakpointInfo breakpointInfo;
    breakpointInfo.type = BreakpointInfo::BREAKPOINT;
    breakpointInfo.address = address;
    return breakpointInfo;
}

void Breakpoints::setBreakpoint(const Breakpoints::BreakpointInfo& breakpointInfo) {
    BreakpointInfo* bi = find(breakpointInfo);
    if (bi == NULL) {
        bi = new BreakpointInfo(breakpointInfo);
        bi->enabled = false;
        // Insert new breakpoint sorted.
        std::vector<BreakpointInfo*>::iterator i = breakpoints.begin();
        for (; i != breakpoints.end(); ++i) {
            if (*(*i) > *bi) {
                break;
            }
        }
        breakpoints.insert(i, bi);
    }
    else if (bi->enabled) {
        return;
    }
    toggleBreakpointEnable(bi);
    DebuggerUpdate();
    updateScroll();
}

void Breakpoints::clearBreakpoint(const Breakpoints::BreakpointInfo& breakpointInfo) 
{
    BreakpointInfo* bi = find(breakpointInfo);
    if (bi == NULL) {
        return;
    }
    if (bi->enabled) {
        toggleBreakpointEnable(bi);
    }
    for (std::vector<BreakpointInfo*>::iterator i = breakpoints.begin(); i != breakpoints.end(); ++i) {
        if (*i == bi) {
            breakpoints.erase(i);
            break;
        }
    }

    DebuggerUpdate();
    updateScroll();
}

void Breakpoints::toggleBreakpointEnable(const Breakpoints::BreakpointInfo& breakpointInfo)
{
    BreakpointInfo* bi = find(breakpointInfo);
    if (bi != NULL) {
        toggleBreakpointEnable(bi);
        DebuggerUpdate();
    }
}

void Breakpoints::toggleBreakpointEnable(Breakpoints::BreakpointInfo* bi) 
{
    if (bi == NULL) {
        return;
    }

    if (bi->enabled) {
        bi->enabled = false;
        if (bi->type == BreakpointInfo::BREAKPOINT) {
            ::ClearBreakpoint(bi->address);
        }
        else {
            ClearWatchpoint(bi->getTypeAsDeviceType(), bi->address);
        }
    }
    else {
        bi->enabled = true;
        if (bi->type == BreakpointInfo::BREAKPOINT) {
            ::SetBreakpoint(bi->address);
        }
        else {
            SetWatchpoint(bi->getTypeAsDeviceType(), bi->address, bi->condition, bi->referenceValue, bi->size);
        }
    }
}

void Breakpoints::clearAllBreakpoints()
{
    while (!breakpoints.empty()) {
        delete breakpoints.front();
        breakpoints.erase(breakpoints.begin());
    }
    DebuggerUpdate();
}

int Breakpoints::getEnabledBpCount() {
    int count = 0;
    for (std::vector<BreakpointInfo*>::iterator i = breakpoints.begin(); i != breakpoints.end(); ++i) {
        if (!(*i)->enabled) {
            count++;
        }
    }
    return count;
}

int Breakpoints::getDisabledBpCount() {
    return breakpoints.size() - getEnabledBpCount();
}

void Breakpoints::enableAllBreakpoints()
{
    for (std::vector<BreakpointInfo*>::iterator i = breakpoints.begin(); i != breakpoints.end(); ++i) {
        if (!(*i)->enabled) {
            toggleBreakpointEnable(*i);
        }
    }
    DebuggerUpdate();
}

void Breakpoints::disableAllBreakpoints()
{
    for (std::vector<BreakpointInfo*>::iterator i = breakpoints.begin(); i != breakpoints.end(); ++i) {
        if ((*i)->enabled) {
            toggleBreakpointEnable(*i);
        }
    }
    DebuggerUpdate();
}

void Breakpoints::updateBreakpoints()
{
    for (std::vector<BreakpointInfo*>::iterator i = breakpoints.begin(); i != breakpoints.end(); ++i) {
        if ((*i)->enabled) {
            (*i)->enabled = false;
            toggleBreakpointEnable(*i);
        }
    }
    DebuggerUpdate();
}

void Breakpoints::setStepOutBreakpoint(const UInt8* memory, UInt16 address)
{
    char str[128];
    setRuntoBreakpoint((address + Disassembly::dasm(symbolInfo, memory, address, str)) & 0xffff);
}

bool Breakpoints::setStepOverBreakpoint(const UInt8* memory, UInt16 address)
{
    char str[128];
    // If call or rst instruction we need to set a runto breakpoint
    // otherwise its just a regular single step
    bool step = strncmp(str, "call", 4) != 0 && 
                strncmp(str, "ldir", 4) != 0 && 
                strncmp(str, "lddr", 4) != 0 && 
                strncmp(str, "cpir", 4) != 0 && 
                strncmp(str, "inir", 4) != 0 && 
                strncmp(str, "indr", 4) != 0 && 
                strncmp(str, "otir", 4) != 0 && 
                strncmp(str, "otdr", 4) != 0 && 
                strncmp(str, "rst",  3) != 0;
    if (!step) {
        setRuntoBreakpoint((address + Disassembly::dasm(symbolInfo, memory, address, str)) & 0xffff);
    }
    return step;
}

void Breakpoints::setRuntoBreakpoint(UInt16 address)
{
    if (address < 0) {
        return;
    }

    runtoBreakpoint = address;
    ::SetBreakpoint(runtoBreakpoint);
}

void Breakpoints::clearRuntoBreakpoint()
{
    if (runtoBreakpoint >= 0) {
        if (!IsBreakpointSet(runtoBreakpoint)) {
            ::ClearBreakpoint(runtoBreakpoint);
        }
        runtoBreakpoint = -1;
    }
}
