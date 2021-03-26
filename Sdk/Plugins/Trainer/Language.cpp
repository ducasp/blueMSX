/*****************************************************************************
** File:        Language.c
**
** Author:      Daniel Vik
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
#include "Language.h"
#include "LanguageChineseSimplified.h"
#include "LanguageChineseTraditional.h"
#include "LanguageEnglish.h"
#include "LanguageJapanese.h"
#include "LanguageKorean.h"
#include "LanguageGerman.h"
#include "LanguageItalian.h"
#include "LanguageFrench.h"
#include "LanguagePortuguese.h"
#include "LanguageRussian.h"
#include "LanguageSpanish.h"
#include "LanguageCatalan.h"
#include <stdlib.h>

namespace {
    Language* ls = new LanguageEnglish;
}


void Language::SetLanguage(LanguageId langId) 
{
    delete ls;
    switch (langId) {
    case LID_CHINESETRAD:
        ls  = new LanguageChineseTraditional;
        break;
    case LID_CHINESESIMP:
        ls  = new LanguageChineseSimplified;
        break;
    case LID_JAPANESE:
        ls = new LanguageJapanese;
        break;
    case LID_KOREAN:
        ls = new LanguageKorean;
        break;
    case LID_GERMAN:
        ls = new LanguageGerman;
        break;
    case LID_ITALIAN:
        ls = new LanguageItalian;
        break;
    case LID_FRENCH:
        ls = new LanguageFrench;
        break;
    case LID_SPANISH:
        ls = new LanguageSpanish;
        break;
    case LID_PORTUGUESE:
        ls = new LanguagePortuguese;
        break;
    case LID_RUSSIAN:
        ls = new LanguageRussian;
        break;
    case LID_CATALAN:
        ls = new LanguageCatalan;
        break;
    case LID_ENGLISH:
    default:
        ls = new LanguageEnglish;
        break;
    }
}

const char* Language::trainerName;
const char* Language::trainerCaption;

const char* Language::saveCheatCaption;
const char* Language::loadCheatCaption;

const char* Language::pause;
const char* Language::run;
const char* Language::truncated;
const char* Language::newCheat;
const char* Language::address;
const char* Language::oldValue;
const char* Language::newValue;
const char* Language::change;
const char* Language::disable;
const char* Language::enable;
const char* Language::description;
const char* Language::value;
const char* Language::activeCheats;
const char* Language::findCheats;
const char* Language::removeAll;
const char* Language::remove;
const char* Language::addCheat;
const char* Language::cheatFile;
const char* Language::ok;
const char* Language::cancel;
const char* Language::displayValueAs;
const char* Language::decimal;
const char* Language::hexadecimal;
const char* Language::dataSize;
const char* Language::eightBit;
const char* Language::sixteenBit;
const char* Language::compareType;
const char* Language::equal;
const char* Language::notEqual;
const char* Language::lessThan;
const char* Language::lessOrEqual;
const char* Language::greaterThan;
const char* Language::greaterOrEqual;
const char* Language::display;
const char* Language::compareNewValueWith;
const char* Language::specificValue;
const char* Language::snapshot;
const char* Language::search;
const char* Language::undo;

