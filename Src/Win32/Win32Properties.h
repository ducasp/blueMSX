/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Win32/Win32Properties.h,v $
**
** $Revision: 1.9 $
**
** $Date: 2008-03-30 18:38:48 $
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
#ifndef WIN32_PROPERTIES_H
#define WIN32_PROPERTIES_H

#include <windows.h>
#include "Properties.h"
#include "AudioMixer.h"
#include "VideoRender.h"
#include "Language.h"
#include "MediaDb.h"
#include "Win32Common.h"

int showProperties(Properties* pProperties, HWND hwndOwner, PropPage startPage, Mixer* mixer, Video* video);

#endif
