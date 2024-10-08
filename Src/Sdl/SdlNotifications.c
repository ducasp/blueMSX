/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Sdl/SdlNotifications.c,v $
**
** $Revision: 1.9 $
**
** $Date: 2008-03-31 19:42:23 $
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
#include "ArchNotifications.h"
#include <stdlib.h>

void* archScreenCapture(ScreenCaptureType type, int* bitmapSize, int onlyBmp) { return NULL; }

void archUpdateEmuDisplayConfig() {}

void archDiskQuickChangeNotify() {}
#if 0 // moved to bluemsxlite.c
void archEmulationStartNotification() {}
void archEmulationStopNotification() {}
void archEmulationStartFailure() {}
#endif

void archQuit() {}

void archThemeSetNext() {}
void archThemeUpdate(struct Theme* theme) {}

void archVideoOutputChange() {}
void archMinimizeMainWindow() {}

int archGetFramesPerSecond() { return 60; }

void* archWindowCreate(struct Theme* theme, int childWindow) { return NULL; }
void archWindowStartMove() {}
void archWindowMove() {}
void archWindowEndMove() {}

void archVideoCaptureSave() {}