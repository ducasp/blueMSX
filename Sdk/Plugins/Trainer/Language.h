/*****************************************************************************
** File:
**      Language.h
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
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "ToolInterface.h"

class Language 
{
public:
    static void  SetLanguage(LanguageId langId);
    virtual ~Language() {}

    static const char* trainerName;
    static const char* trainerCaption;

    static const char* saveCheatCaption;
    static const char* loadCheatCaption;

    static const char* pause;
    static const char* run;
    static const char* truncated;
    static const char* newCheat;
    static const char* address;
    static const char* oldValue;
    static const char* newValue;
    static const char* change;
    static const char* disable;
    static const char* enable;
    static const char* description;
    static const char* value;
    static const char* activeCheats;
    static const char* findCheats;
    static const char* removeAll;
    static const char* remove;
    static const char* addCheat;
    static const char* cheatFile;
    static const char* ok;
    static const char* cancel;
    static const char* displayValueAs;
    static const char* decimal;
    static const char* hexadecimal;
    static const char* dataSize;
    static const char* eightBit;
    static const char* sixteenBit;
    static const char* compareType;
    static const char* equal;
    static const char* notEqual;
    static const char* lessThan;
    static const char* lessOrEqual;
    static const char* greaterThan;
    static const char* greaterOrEqual;
    static const char* display;
    static const char* compareNewValueWith;
    static const char* specificValue;
    static const char* snapshot;
    static const char* search;
    static const char* undo;

    Language() {}
};

#endif //LANGUAGE_H
