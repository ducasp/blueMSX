/*****************************************************************************
** File:
**      LanguageKorean.h
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
#ifndef LANGUAGE_KOERAN_H
#define LANGUAGE_KOERAN_H

#include "LanguageEnglish.h"

class LanguageKorean : public LanguageEnglish
{
public:
    LanguageKorean() {
        genericOk                   = "Ȯ��";
        genericCancel               = "���";

        toolbarResume               = "����/���";
        toolbarPause                = "��� �ߴ�";
        toolbarStop                 = "����� ����";
        toolbarRun                  = "�ٽ� ����";
        toolbarShowNext             = "���� ���� �����ֱ�";
        toolbarStepIn               = "���ν��� ������";
        toolbarStepOver             = "���ν��� ���� ����";
        toolbarStepOut              = "���ν��� ������";
        toolbarRunTo                = "Ŀ������ ����";
        toolbarBpToggle             = "�ߴ��� ����/����";
        toolbarBpEnable             = "�ߴ��� Ȱ��/��Ȱ��";
        toolbarBpEnableAll          = "��� �ߴ��� Ȱ��";
        toolbarBpDisableAll         = "��� �ߴ��� ��Ȱ��";
        toolbarBpRemoveAll          = "��� �ߴ��� ����";

        menuFile                    = "����";
        menuFileLoadSymbolFile      = "�ɺ� ���� �ε�";
        menuFileSaveDisassembly     = "�� ����� ����";
        menuFileSaveMemory          = "�޸� ����";
        menuFileExit                = "������";

        menuDebug                   = "�����";
        menuDebugStart              = "����";
        menuDebugContinue           = "���";
        menuDebugBreakAll           = "��� �ߴ�";
        menuDebugStop               = "����� ����";
        menuDebugRestart            = "�ٽ� ����";
        menuDebugStepIn             = "���ν��� ������";
        menuDebugStepOver           = "���ν��� ���� ����";
        menuDebugStepOut            = "���ν��� ������";
        menuDebugRunTo              = "Ŀ������ ����";
        menuDebugShowSymbols        = "�ɺ� ���� �����ֱ�";
        menuDebugGoto               = "�����ּҷ� ����";
        menuDebugFind               = "ã��";
        menuDebugBpAdd              = "�ߴ��� �߰�";
        menuDebugBpToggle           = "�ߴ��� ����/����";
        menuDebugEnable             = "�ߴ��� Ȱ��/��Ȱ��";
        menuDebugRemoveAll          = "��� �ߴ��� ����";
        menuDebugEnableAll          = "��� �ߴ��� Ȱ��";
        menuDebugDisableAll         = "��� �ߴ��� ��Ȱ��";
        menuDebugShowAssemblyFlags  = "����� �÷��� �����ֱ�";
        menuDebugFastVram           = "Break on too fast VRAM access";

        menuWindow                  = "������";

        menuHelp                    = "����";
        menuHelpAbout               = "Debugger�� ���Ͽ�";

        debuggerName                = "Debugger";
        windowDebugger              = "blueMSX - Debugger";
        windowDisassembly           = "�𽺾����";
        windowDisassemblyUnavail    = "Disassembly unavailable.";
        windowCpuRegisters          = "CPU ��������";
        windowCpuRegistersFlags     = "Flags";
        windowStack                 = "����";
        windowStackUnavail          = "Stack unavailable.";
        windowCallstack             = "ȣ�� ����";
        windowCallstackUnavail      = "Callstack unavailable.";
        windowMemory                = "�޸�";
        windowPeripheralRegisters   = "�ֺ���ġ ��������";
        windowIoPorts               = "����� ��Ʈ";

		memWindowRegisters          = "Registers:";

        memWindowMemory             = "Memory:";
        memWindowAddress            = "Address:";

        setBpWindowCaption          = "���ο� �ߴ��� �߰�";
        gotoWindowCaption           = "�ּҷ� ����";
        gotoWindowText              = "�ּ� Ȥ�� ���̺�:";

        symbolWindowCaption         = "�ɺ� ���� ����";
        symbolWindowText            = "���� �ɺ� ��ü";

        popupOverwrite              = "������ �̹� �ֽ��ϴ�. ���� ���ðڽ��ϱ�?";
        
        aboutBuilt                  = "����:";
        aboutVisit                  = "�ڼ��� ���� http://www.bluemsx.com �� �湮�ϼ���.";
    }
};


#endif //LANGUAGE_KOERAN_H
