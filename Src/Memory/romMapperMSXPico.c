/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Memory/romMapperMSXPico.c,v $
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
#include "romMapperMSXPico.h"
#include "MediaDb.h"
#include "SlotManager.h"
#include "DeviceManager.h"
#include "Board.h"
#include "MSXPicoWiFi.c"
#include <stdlib.h>
#include <string.h>


typedef struct {
    int deviceHandle;
	UInt8* romData;
    int slot;
    int sslot;
    int startPage;
} RomMapperMSXPico;


static void destroy(RomMapperMSXPico* rm)
{
    msxPicoWiFiDestroy();
    slotUnregister(rm->slot, rm->sslot, rm->startPage);
    deviceManagerUnregister(rm->deviceHandle);

	free(rm->romData);
    free(rm);
}

static void reset(RomMapperMSXPico* rm)
{
    msxPicoWiFiReset();
}

static UInt8 read(RomMapperMSXPico* rm, UInt16 address) 
{
	if (address == 0x3F06) { 
        return msxPicoWiFiReadUARTBuffer();
    }

	if (address == 0x3F07) { 
        return msxPicoWiFiReadUARTStatus();
    }

	if (address == 0x3FF2) { 
        return msxPicoWiFiReadF2();
    }

    if (address < 0x4000) {
        // This is reads to 0x4000-0x7FFF, rest are directly mapped
        return rm->romData[address];
    }

    return 0xff;
}

static UInt8 peek(RomMapperMSXPico* rm, UInt16 address) 
{
    
    if (address == 0x3F06) return 0xff;
	if (address == 0x3F07) return 0xff;
	if (address == 0x3FF2) return 0xff;
    return read(rm, address);
}

static void write(RomMapperMSXPico* rm, UInt16 address, UInt8 value) 
{
	if (address == 0x3F06) { 
        msxPicoWiFiWriteUARTCommand(value);
    }

	if (address == 0x3F07) { 
        msxPicoWiFiWriteUARTBuffer(value);
    }

	if (address == 0x3FF2) { 
        msxPicoWiFiWriteF2(value);
    }
}

int romMapperMSXPicoCreate(const char* filename, UInt8* romData, 
                           int size, int slot, int sslot, int startPage) 
{
    DeviceCallbacks callbacks = { destroy, reset, NULL, NULL };
    RomMapperMSXPico* rm;
	
	if (size > 0x4000) {
        return 0;
    }

    rm = malloc(sizeof(RomMapperMSXPico));

	rm->romData = malloc(0x4000);
    memset(rm->romData, 0xff, 0x4000);
    memcpy(rm->romData, romData, size);

    rm->deviceHandle = deviceManagerRegister(ROM_MSXPICOWIFI_16K, &callbacks, rm);
    slotRegister(slot, sslot, startPage, 4, read, peek, write, destroy, rm);

    rm->slot  = slot;
    rm->sslot = sslot;
    rm->startPage  = startPage;

    msxPicoWiFiCreate();

    slotMapPage(rm->slot, rm->sslot, rm->startPage + 0, rm->romData, 1, 0);
    slotMapPage(rm->slot, rm->sslot, rm->startPage + 1, NULL, 0, 0);
    slotMapPage(rm->slot, rm->sslot, rm->startPage + 2, NULL, 0, 0);
    slotMapPage(rm->slot, rm->sslot, rm->startPage + 3, NULL, 0, 0);

    reset(rm);

    return 1;
}