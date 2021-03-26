/*****************************************************************************
** File:
**      LanguageRussian.h
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
#ifndef LANGUAGE_RUSSIAN_H
#define LANGUAGE_RUSSIAN_H

#include "Language.h"

class LanguageRussian : public Language
{
public:
    LanguageRussian() {
        genericOk                   = "OK";
        genericCancel               = "������";

        toolbarResume               = "������/����������";
        toolbarPause                = "������������� ���";
        toolbarStop                 = "���������� �������";
        toolbarRun                  = "����������";
        toolbarShowNext             = "�������� ���������� ���������";
        toolbarStepIn               = "��������� ���";
        toolbarStepOver             = "��������� ��� � ���������� ����";
        toolbarStepOut              = "����� � ���������� ��������";
        toolbarRunTo                = "��������� �������� �� �������";
        toolbarBpToggle             = "���������/������ ����������";
        toolbarBpEnable             = "��������/��������� ����������";
        toolbarBpEnableAll          = "�������� ��� ����������";
        toolbarBpDisableAll         = "��������� ��� ����������";
        toolbarBpRemoveAll          = "������ ��� ����������";

        menuFile                    = "����";
        menuFileLoadSymbolFile      = "������� ���������� ����";
        menuFileSaveDisassembly     = "��������� ������������������";
        menuFileSaveMemory          = "��������� ������";
        menuFileExit                = "�����";

        menuDebug                   = "�������";
        menuDebugStart              = "������";
        menuDebugContinue           = "����������";
        menuDebugBreakAll           = "������������� ���";
        menuDebugStop               = "���������� �������";
        menuDebugRestart            = "����������";
        menuDebugStepIn             = "��������� ���";
        menuDebugStepOver           = "��������� ��� � ���������� ����";
        menuDebugStepOut            = "����� � ���������� ��������";
        menuDebugRunTo              = "��������� �������� �� �������";
        menuDebugShowSymbols        = "�������� ���������� ����������";
        menuDebugGoto               = "������� �";
        menuDebugFind               = "�����";
        menuDebugBpAdd              = "�������� ����� ����������";
        menuDebugBpToggle           = "���������/������ ����������";
        menuDebugEnable             = "��������/��������� ����������";
        menuDebugRemoveAll          = "������ ��� ����������";
        menuDebugEnableAll          = "�������� ��� ����������";
        menuDebugDisableAll         = "��������� ��� ����������";
        menuDebugShowAssemblyFlags  = "�������� ����� ����������";
        menuDebugFastVram           = "Break on too fast VRAM access";

        menuWindow                  = "����";

        menuHelp                    = "������";
        menuHelpAbout               = "� ���������";

        debuggerName                = "��������";
        windowDebugger              = "blueMSX - ��������";
        windowDisassembly           = "������������������";
        windowDisassemblyUnavail    = "������������������ ����������.";
        windowCpuRegisters          = "CPU ��������";
        windowCpuRegistersFlags     = "�����";
        windowStack                 = "����";
        windowStackUnavail          = "���� ����������.";
        windowCallstack             = "���� �������";
        windowCallstackUnavail      = "���� ������� ����������.";
        windowMemory                = "������";
        windowPeripheralRegisters   = "�������� ������������";
        windowIoPorts               = "����� �����/������";

        memWindowRegisters          = "��������:";

        memWindowMemory             = "������:";
        memWindowAddress            = "�����:";

        setBpWindowCaption          = "�������� ����� ����������";
        gotoWindowCaption           = "������� � ������";
        gotoWindowText              = "����� ��� �����:";
        findWindowCaption           = "�����";
        findWindowText              = "������ ��� ��������:";

        symbolWindowCaption         = "������� ���������� ����";
        symbolWindowText            = "�������� ������ �������";
        
        popupOverwrite              = "���� ��� ����������. ������ ������������ ���?";
        
        aboutBuilt                  = "������:";
        aboutVisit                  = "�������� www.bluemsx.com ��� ������������";
    }
};


#endif //LANGUAGE_RUSSIAN_H
