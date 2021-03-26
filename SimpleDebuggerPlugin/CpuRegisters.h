/*****************************************************************************
** File:
**      CpuRegisters.h
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
#ifndef CPU_REGISTERS_H
#define CPU_REGISTERS_H

#include <windows.h>
#include "DbgWindow.h"
#include "ToolInterface.h"
#include "EditControls.h"

class CpuRegisters : public DbgWindow {
public:
    enum FlagMode { FM_CPU, FM_ASM };

    CpuRegisters(HINSTANCE hInstance, HWND owner);
    ~CpuRegisters();

    void setFlagMode(FlagMode mode);
    FlagMode getFlagMode();
    
    virtual void disableEdit();
    
    void updateContent(RegisterBank* regBank);
    void invalidateContent();

    BOOL lookup(const char* name, WORD* addr);

    virtual LRESULT wndProc(UINT iMsg, WPARAM wParam, LPARAM lParam);

private:

    void scrollWindow(int sbAction);
    void updateScroll();
    void drawText(int top, int bottom);

    HDC    hMemdc;
    HFONT  hFont;
    HFONT  hFontBold;
    HBRUSH hBrushWhite;
    HBRUSH hBrushLtGray;
    HBRUSH hBrushDkGray;
    int    textHeight;
    int    textWidth;

    RegisterBank* currentRegBank;

    int    currentEditRegister;

    int    registersPerRow;
    int    lineCount;

    FlagMode flagMode;

    COLORREF colorBlack;
    COLORREF colorLtGray;
    COLORREF colorGray;
    COLORREF colorRed;

    int regValue[32];
    int refRegValue[32];
    
    HexInputDialog* dataInput2;
    HexInputDialog* dataInput4;
};

#endif //CPU_REGISTERS_H
