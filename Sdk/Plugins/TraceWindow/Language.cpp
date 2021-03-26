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
//#include "LanguageFinnish.h"
#include "LanguageRussian.h"
#include "LanguageJapanese.h"
#include "LanguageKorean.h"
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
    case LID_RUSSIAN:
        ls = new LanguageRussian;
        break;
    case LID_KOREAN:
        ls = new LanguageKorean;
        break;
//    case LID_FINNISH:
//        ls = new LanguageFinnish;
//        break;
    case LID_ENGLISH:
    default:
        ls = new LanguageEnglish;
        break;
    }
}

const char* Language::traceWindowName;
const char* Language::traceWindowCaption;
const char* Language::openWindowCaption;

const char* Language::menuFile;
const char* Language::menuFileLogToFile;
const char* Language::menuFileStopLogToFile;
const char* Language::menuFileExit;

const char* Language::menuEdit;
const char* Language::menuEditSelectAll;
const char* Language::menuEditCopy;
const char* Language::menuEditClearWindow;

const char* Language::menuHelp;
const char* Language::menuHelpAbout;

const char* Language::aboutBuilt;
const char* Language::aboutVisit;
