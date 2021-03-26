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
#ifndef LANGUAGE_KOREAN_H
#define LANGUAGE_KOREAN_H

#include "LanguageEnglish.h"

class LanguageKorean : public LanguageEnglish
{
public:
    LanguageKorean() {
        trainerName             = "Ʈ���̳�";
        trainerCaption          = "blueMSX - Ʈ���̳�";
        saveCheatCaption        = "ġƮ �����ϱ�";
        loadCheatCaption        = "ġƮ �о����";
        pause                   = "�Ͻ�����";
        run                     = "����";
        truncated               = "����...";
        newCheat                = "���ο� ġƮ";
        address                 = "�ּ�";
        oldValue                = "���� ��";
        newValue                = "���ο� ��";
        change                  = "��ȭ";
        disable                 = "��Ȱ��";
        enable                  = "Ȱ��";
        description             = "����";
        value                   = "��";
        activeCheats            = "��밡�� ġƮ ";
        findCheats              = "ġƮ ã�� ";
        removeAll               = "��� ����";
        remove                  = "����";
        addCheat                = "ġƮ �߰�";
        cheatFile               = "ġƮ ����:";
        ok                      = "Ȯ��";
        cancel                  = "���";
        displayValueAs          = "�� ǥ�� ��� ";
        decimal                 = "10 ����";
        hexadecimal             = "16 ����";
        dataSize                = "������ ũ�� ";
        eightBit                = "8 ��Ʈ";
        sixteenBit              = "16 ��Ʈ";
        compareType             = "�� ��� ";
        equal                   = "����";
        notEqual                = "���� ����";
        lessThan                = "����";
        lessOrEqual             = "�۰ų� ����";
        greaterThan             = "ū";
        greaterOrEqual          = "ũ�ų� ����";
        display                 = "ǥ�� ��� ";
        compareNewValueWith     = "���ο� ���� �� ";
        specificValue           = "Ư�� ��: ";
        snapshot                = "������";
        search                  = "ã��";
        undo                    = "���� ���";
    }
};

#endif //LANGUAGE_KOREAN_H
