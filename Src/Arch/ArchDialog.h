/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Arch/ArchDialog.h,v $
**
** $Revision: 1.14 $
**
** $Date: 2008-03-30 18:38:39 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#ifndef ARCH_DIALOG_H
#define ARCH_DIALOG_H

#include "Properties.h"

void archShowPropertiesDialog(PropPage page);
void archShowLanguageDialog();
void archShowHelpDialog();
void archShowAboutDialog();
void archShowCassettePosDialog();
void archShowShortcutsEditor();
void archShowKeyboardEditor();
void archShowMixer();
void archShowDebugger();
void archShowTrainer();
void archShowMachineEditor();

void archShowNoRomInZipDialog();
void archShowNoDiskInZipDialog();
void archShowNoCasInZipDialog();
void archShowStartEmuFailDialog();

void archMaximizeWindow();
void archMinimizeWindow();
void archCloseWindow();

#endif
