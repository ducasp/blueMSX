/*****************************************************************************
** File:
**      LanguageChineseSimplified.h
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
#ifndef LANGUAGE_CHINESE_SIMPLIFIED_H
#define LANGUAGE_CHINESE_SIMPLIFIED_H

#include "LanguageEnglish.h"

class LanguageChineseSimplified : public LanguageEnglish
{
public:
    LanguageChineseSimplified() {
        genericOk                   = "ȷ��";
        genericCancel               = "ȡ��";

        toolbarResume               = "��ʼ/����";
        toolbarPause                = "ȫ���ж�";
        toolbarStop                 = "ֹͣ����";
        toolbarRun                  = "����";
        toolbarShowNext             = "��ʾ��һ�����";
        toolbarStepIn               = "����ִ��";
        toolbarStepOver             = "����";
        toolbarStepOut              = "�����";
        toolbarRunTo                = "���е���괦";
        toolbarBpToggle             = "�趨/�Ƴ��ϵ�";
        toolbarBpEnable             = "����/���öϵ�";
        toolbarBpEnableAll          = "�������жϵ�";
        toolbarBpDisableAll         = "�������жϵ�";
        toolbarBpRemoveAll          = "�Ƴ����жϵ�";

        menuFile                    = "�ļ�";
        menuFileLoadSymbolFile      = "��ȡ�����ļ�";
        menuFileSaveDisassembly     = "���淴���";
        menuFileSaveMemory          = "�����ڴ�";
        menuFileExit                = "�˳�";

        menuDebug                   = "����";
        menuDebugStart              = "��ʼ";
        menuDebugContinue           = "����";
        menuDebugBreakAll           = "ȫ���ж�";
        menuDebugStop               = "ֹͣ����";
        menuDebugRestart            = "���¿�ʼ";
        menuDebugStepIn             = "����ִ��";
        menuDebugStepOver           = "����";
        menuDebugStepOut            = "�����";
        menuDebugRunTo              = "���е���괦";
        menuDebugShowSymbols        = "��ʾ������Ϣ";
        menuDebugGoto               = "ת��";
        menuDebugFind               = "����";
        menuDebugBpAdd              = "����¶ϵ�";
        menuDebugBpToggle           = "�趨/�Ƴ� ";
        menuDebugEnable             = "����/���öϵ�";
        menuDebugRemoveAll          = "�Ƴ����жϵ�";
        menuDebugEnableAll          = "�������жϵ�";
        menuDebugDisableAll         = "�������жϵ�";
        menuDebugShowAssemblyFlags  = "��ʾ����־";
        menuDebugFastVram           = "Break on too fast VRAM access";

        menuWindow                  = "����";

        menuHelp                    = "����";
        menuHelpAbout               = "����";

        debuggerName                = "���Թ���";
        windowDebugger              = "blueMSX - ���Թ���";
        windowDisassembly           = "�����";
        windowDisassemblyUnavail    = "����಻���á�";
        windowCpuRegisters          = "CPU�Ĵ�����";
        windowCpuRegistersFlags     = "���";
        windowStack                 = "ջ";
        windowStackUnavail          = "ջ�����á�";
        windowCallstack             = "����ջ";
        windowCallstackUnavail      = "����ջ�����á�";
        windowMemory                = "�ڴ�";
        windowPeripheralRegisters   = "��Χ�Ĵ���";
        windowIoPorts               = "I/O �˿�";

        memWindowRegisters          = "�Ĵ���:";

        memWindowMemory             = "�ڴ�:";
        memWindowAddress            = "��ַ:";

        setBpWindowCaption          = "����¶ϵ�";
        gotoWindowCaption           = "ת����ַ";
        gotoWindowText              = "��ַ���ǩ:";

        symbolWindowCaption         = "�򿪷����ļ�";
        symbolWindowText            = "���Ǿɵķ���";

        popupOverwrite              = "�ļ��Ѵ��ڡ�Ҫ����ԭ���ļ���";
        
        aboutBuilt                  = "����:";
        aboutVisit                  = "�����������½��www.bluemsx.com ";
    }
};


#endif //LANGUAGE_CHINESE_SIMPLIFIED_H
