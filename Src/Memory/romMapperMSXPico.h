/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Memory/romMapperMSXPico.h,v $
**
** $Revision: 1.0 $
**
** $Date: 2024-09-04 $
**
** More info: http://www.github.com/ducasp
**
** Copyright (C) 2024- Oduvaldo Pavan Junior
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
#ifndef ROMMAPPER_MSXPICO_H
#define ROMMAPPER_MSXPICO_H
 
#include "MsxTypes.h"

int romMapperMSXPicoCreate(const char* filename, UInt8* romData, 
                           int size, int slot, int sslot, int startPage);

#endif
