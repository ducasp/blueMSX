/*****************************************************************************
** File:
**      LanguageChineseTraditional.h
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
#ifndef LANGUAGE_CHINESE_TRADITIONAL_H
#define LANGUAGE_CHINESE_TRADITIONAL_H

#include "LanguageEnglish.h"

class LanguageChineseTraditional : public LanguageEnglish
{
public:
    LanguageChineseTraditional() {
        genericOk                   = "�T�w";
        genericCancel               = "����";

        toolbarResume               = "�}�l/�~��";
        toolbarPause                = "�������_";
        toolbarStop                 = "�����";
        toolbarRun                  = "���s�Ұ�";
        toolbarShowNext             = "��ܤU�@�ӳ��z��";
        toolbarStepIn               = "�v�B����";
        toolbarStepOver             = "���i�J�禡";
        toolbarStepOut              = "�����禡";
        toolbarRunTo                = "��������";
        toolbarBpToggle             = "�]�w/�������_�I";
        toolbarBpEnable             = "�ҥ�/���Τ��_�I";
        toolbarBpEnableAll          = "�ҥΩҦ����_�I";
        toolbarBpDisableAll         = "���ΩҦ����_�I";
        toolbarBpRemoveAll          = "�����Ҧ����_�I";

        menuFile                    = "�ɮ�";
        menuFileLoadSymbolFile      = "���J�Ÿ��ɮ�";
        menuFileSaveDisassembly     = "�x�s�ϲ�Ķ";
        menuFileSaveMemory          = "�x�s�O����";
        menuFileExit                = "����";

        menuDebug                   = "����";
        menuDebugStart              = "�}�l";
        menuDebugContinue           = "�~��";
        menuDebugBreakAll           = "�������_";
        menuDebugStop               = "�����";
        menuDebugRestart            = "���s�Ұ�";
        menuDebugStepIn             = "�v�B����";
        menuDebugStepOver           = "���i�J�禡";
        menuDebugStepOut            = "�����禡";
        menuDebugRunTo              = "��������";
        menuDebugShowSymbols        = "��ܲŸ���T";
        menuDebugGoto               = "�e��";
        menuDebugFind               = "�M��";
        menuDebugBpAdd              = "�[�J�s�����_�I";
        menuDebugBpToggle           = "�]�w/�������_�I";
        menuDebugEnable             = "�ҥ�/���Τ��_�I";
        menuDebugRemoveAll          = "�����Ҧ����_�I";
        menuDebugEnableAll          = "�ҥΩҦ����_�I";
        menuDebugDisableAll         = "���ΩҦ����_�I";
        menuDebugShowAssemblyFlags  = "��ܲզX�X��";
        menuDebugFastVram           = "Break on too fast VRAM access";

        menuWindow                  = "����";

        menuHelp                    = "����";
        menuHelpAbout               = "����";

        debuggerName                = "�����u��";
        windowDebugger              = "blueMSX - �����u��";
        windowDisassembly           = "�ϲ�Ķ�X";
        windowDisassemblyUnavail    = "�ϲ�Ķ�X���i�ϥΡC";
        windowCpuRegisters          = "CPU �Ȧs��";
        windowCpuRegistersFlags     = "�X��";
        windowStack                 = "���|";
        windowStackUnavail          = "���|���i�ϥΡC";
        windowCallstack             = "�I�s���|";
        windowCallstackUnavail      = "�I�s���|���i�ϥΡC";
        windowMemory                = "�O����";
        windowPeripheralRegisters   = "�P��Ȧs��";
        windowIoPorts               = "I/O �s����";

        memWindowRegisters          = "�Ȧs��:";

        memWindowMemory             = "�O����:";
        memWindowAddress            = "��}:";

        setBpWindowCaption          = "�[�J�s�����_�I";
        gotoWindowCaption           = "�e����}";
        gotoWindowText              = "��}�μ���:";

        symbolWindowCaption         = "�}�ҲŸ��ɮ�";
        symbolWindowText            = "���N�ª��Ÿ�";

        popupOverwrite              = "�ɮפw�g�s�b�C �z�T�w�n�мg���ܡH";
        
        aboutBuilt                  = "�ի�:";
        aboutVisit                  = "�гX�� http://www.bluemsx.com ���o��h�Ӹ`";
    }
};


#endif //LANGUAGE_CHINESE_TRADITIONAL_H
