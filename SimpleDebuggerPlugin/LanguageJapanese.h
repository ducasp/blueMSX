/*****************************************************************************
** File:
**      LanguageJapanese.h
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
#ifndef LANGUAGE_JAPANESE_H
#define LANGUAGE_JAPANESE_H

#include "LanguageEnglish.h"

class LanguageJapanese : public LanguageEnglish
{
public:
    LanguageJapanese() {
        genericOk                   = "OK";
        genericCancel               = "�L�����Z��";

        toolbarResume               = "�J�n/���s";
        toolbarPause                = "�S�Ē�~";
        toolbarStop                 = "�f�o�b�O���~";
        toolbarRun                  = "���X�^�[�g";
        toolbarShowNext             = "���̃X�e�[�g�����g�\\��";
        toolbarStepIn               = "�X�e�b�v �C��";
        toolbarStepOver             = "�X�e�b�v �I�[�o�[";
        toolbarStepOut              = "�X�e�b�v �A�E�g";
        toolbarRunTo                = "�J�[�\\���܂Ŏ��s";
        toolbarBpToggle             = "�u���C�N�|�C���g/����";
        toolbarBpEnable             = "�u���C�N�|�C���g�L/��";
        toolbarBpEnableAll          = "�u���C�N�|�C���g�S�L����";
        toolbarBpDisableAll         = "�u���C�N�|�C���g�S������";
        toolbarBpRemoveAll          = "�u���C�N�|�C���g�S����";

        menuFile                    = "�t�@�C��";
        menuFileLoadSymbolFile      = "�V���{���t�@�C���̃��[�h";
        menuFileSaveDisassembly     = "Save Disassembly";
        menuFileSaveMemory          = "Save Memory";
        menuFileExit                = "�I��";

        menuDebug                   = "�f�o�b�O";
        menuDebugStart              = "�J�n";
        menuDebugContinue           = "���s";
        menuDebugBreakAll           = "�S�Ē�~";
        menuDebugStop               = "�f�o�b�O���~";
        menuDebugRestart            = "���X�^�[�g";
        menuDebugStepIn             = "�X�e�b�v �C��";
        menuDebugStepOver           = "�X�e�b�v �I�[�o�[";
        menuDebugStepOut            = "�X�e�b�v �A�E�g";
        menuDebugRunTo              = "�J�[�\\���܂Ŏ��s";
        menuDebugShowSymbols        = "�V���{�����\\��";
        menuDebugGoto               = "�ړ�";
        menuDebugBpAdd              = "�u���C�N�|�C���g��ǉ�";
        menuDebugBpToggle           = "�u���C�N�|�C���g�̃Z�b�g/����";
        menuDebugEnable             = "�u���C�N�|�C���g�L/��";
        menuDebugRemoveAll          = "�u���C�N�|�C���g�S����";
        menuDebugEnableAll          = "�u���C�N�|�C���g�S�L����";
        menuDebugDisableAll         = "�u���C�N�|�C���g�S������";
        menuDebugShowAssemblyFlags  = "�A�Z���u���t���b�O�\\��";
        menuDebugFastVram           = "Break on too fast VRAM access";

        menuWindow                  = "�E�B���h�E";

        menuHelp                    = "�w���v";
        menuHelpAbout               = "�f�o�b�K�ɂ���";

        debuggerName                = "�f�o�b�K";
        windowDebugger              = "blueMSX - �f�o�b�K";
        windowDisassembly           = "�t�A�Z���u��";
        windowDisassemblyUnavail    = "Disassembly unavailable.";
        windowCpuRegisters          = "CPU ���W�X�^";
        windowCpuRegistersFlags     = "Flags";
        windowStack                 = "�X�^�b�N";
        windowStackUnavail          = "Stack unavailable.";
        windowCallstack             = "�R�[���X�^�b�N";
        windowCallstackUnavail      = "Callstack unavailable.";
        windowMemory                = "������";
        windowPeripheralRegisters   = "Peripheral Registers";
        windowIoPorts               = "I/O Ports";

        memWindowRegisters          = "Registers:";

        memWindowMemory             = "������:";
        memWindowAddress            = "�A�h���X:";

        setBpWindowCaption          = "�u���C�N�|�C���g��ǉ�";
        gotoWindowCaption           = "�A�h���X�Ɉړ�";
        gotoWindowText              = "�A�h���X�܂��̓��x��:";

        symbolWindowCaption         = "�V���{���t�@�C���̃��[�h";
        symbolWindowText            = "�V���{���̓���ւ�";
        
        popupOverwrite              = "File already exists. Do you want to overwrite it?";
        
        aboutBuilt                  = "�r���h:";
        aboutVisit                  = "�ڂ������� http://www.bluemsx.com �܂�";
    }
};

#endif //LANGUAGE_JAPANESE_H
