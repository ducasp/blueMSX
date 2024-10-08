/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Linux/blueMSXlite/LinuxMidi.c,v $
**
** $Revision: 1.3 $
**
** $Date: 2008-03-31 19:42:21 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2004 Daniel Vikl, Tomas Karlsson
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
#include "ArchMidi.h"
#include <stdlib.h>

void archMidiEnable(int enable) {}
int  archMidiGetNoteOn() { return 0; }
void archMidiUpdateVolume(int left, int right) {}
void archMidiLoadState(void) {}
void archMidiSaveState(void) {}
ArchMidi* archMidiInCreate(int device, ArchMidiInCb cb, void* ref) { return NULL; }
void archMidiInDestroy(ArchMidi* archMidi) {}
int archMidiInGetNoteOn(ArchMidi* archMidi, int note) { return 0; }
ArchMidi* archMidiOutCreate(int device) { return NULL; }
void archMidiOutDestroy(ArchMidi* archMidi) {}
void archMidiOutTransmit(ArchMidi* archMidi, UInt8 value) {}


