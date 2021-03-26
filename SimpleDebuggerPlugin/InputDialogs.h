/*****************************************************************************
** File:
**      InputDialogs.h
**
** Copyright (C) 2003-2012 Daniel Vik
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
#ifndef INPUT_DIALOGS_H
#define INPUT_DIALOGS_H

#include "Disassembly.h"
#include "SymbolInfo.h"
#include "CpuRegisters.h"
#include "DbgWindow.h"
#include "Memory.h"
#include <windows.h>

class InputDialogs {
public:
    InputDialogs(HINSTANCE hInstance, HWND rootHwnd, Disassembly* disassembly,
        SymbolInfo* symbolInfo, CpuRegisters* cpuRegisters, Memory* memory, Breakpoints* breakpoints);
    ~InputDialogs();
    
    static void NewBreakpoint();
    static void NewWatchpoint();
    static void GotoAddress();
    static void FindText();
    static void FindNextText();
    
    BOOL addressDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
    BOOL watchpointDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
    BOOL findDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

private:
    void newBreakpoint();
    void newWatchpoint();
    void gotoAddress();
    void findText();

    bool showAddressDialog(const char* caption, int& address);
    bool showWatchpointDialog(const char* caption, Breakpoints::BreakpointInfo& breakpointInfo);
    bool showFindTextDialog(const char* caption, char* enteredText);

    void setButtonCheck(HWND hDlg, int id, int check, int enable);
    void updateWatchpointControls(HWND hDlg, const Breakpoints::BreakpointInfo& bi);

    HINSTANCE hInstance;
    HWND rootHwnd;
    Disassembly* disassembly;
    SymbolInfo* symbolInfo;
    CpuRegisters* cpuRegisters;
    Memory* memory;
    Breakpoints* breakpoints;

    HexInputDialog* addressInput;
    HexInputDialog* refValueInput;

    char currentSearchText[512];
};

#endif //InputDialogs
