/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Win32/Win32ScreenShot.h,v $
**
** $Revision: 1.7 $
**
** $Date: 2008-03-30 18:38:48 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik, Laurent Halter
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
#ifndef WIN32_SCREENSHOT_H
#define WIN32_SCREENSHOT_H

#include <windows.h>

#include "Properties.h"

void screenshotSetDirectory(char* directory, char* prefix);
int  ScreenShot(Properties* properties, HWND hwnd, int width, int height, int xOffset, int yOffset, int png);
void* ScreenShot2(void* src, int srcPitch, int width, int height, int* bitmapSize, int png);
void ScreenShot3(Properties* properties, void* src, int srcPitch, int width, int height, int png);
HBITMAP BitmapFromData(void* bmp);

#endif //WIN32_SCREENSHOT_H
