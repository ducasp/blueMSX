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

    static const char* traceWindowName;
    static const char* traceWindowCaption;
    static const char* openWindowCaption;

    static const char* menuFile;
    static const char* menuFileLogToFile;
    static const char* menuFileStopLogToFile;
    static const char* menuFileExit;
    
    static const char* menuEdit;
    static const char* menuEditSelectAll;
    static const char* menuEditCopy;
    static const char* menuEditClearWindow;

    static const char* menuHelp;
    static const char* menuHelpAbout;

    static const char* aboutBuilt;
    static const char* aboutVisit;

    Language() {}
};

#endif //LANGUAGE_H
