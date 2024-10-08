/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Memory/romMapperSfg05.c,v $
**
** $Revision: 1.14 $
**
** $Date: 2008-09-09 04:32:19 $
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
#include "romMapperSfg05.h"
#include "MediaDb.h"
#include "MidiIO.h"
#include "Switches.h"
#include "SlotManager.h"
#include "DeviceManager.h"
#include "DebugDeviceManager.h"
#include "IoPort.h"
#include "Board.h"
#include "ym2151.h"
#include "SaveState.h"
#include "Language.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "ArchEvent.h"


#define RX_QUEUE_SIZE 256

typedef struct {
    MidiIO*     midiIo;
    UInt8       command;
    UInt8       rxData;
    UInt32      status;
    UInt8       sendByte;
    UInt8       sendBuffer;
    int         txPending;
    UInt8       rxQueue[RX_QUEUE_SIZE];
    int         rxPending;
    int         rxHead;
    void*       semaphore;
    UInt32      charTime;
    UInt8       vector;
    BoardTimer* timerRecv;
    UInt32      timeRecv;
    BoardTimer* timerTrans;
    UInt32      timeTrans;
} YM2148;

#define STAT_TXRDY      0x01
#define STAT_RXRBSY     0x02
#define STAT_TXEMPTY    0x100
#define STAT_PE         0x10
#define STAT_OE         0x20
#define ST_INT          0x800

#define CMD_TXEN   0x01
#define CMD_TXINT  0x02
#define CMD_RXEN   0x04
#define CMD_RXINT  0x08
#define CMD_RSTER  0x10
#define CMD_RST    0x80

static void ym2148Reset(YM2148* midi);

static void midiInCallback(YM2148* midi, UInt8* buffer, UInt32 length)
{
    archSemaphoreWait(midi->semaphore, -1);
    if (midi->rxPending + length < RX_QUEUE_SIZE) {
        while (length--) {
            midi->rxQueue[midi->rxHead & (RX_QUEUE_SIZE - 1)] = *buffer++;
            midi->rxHead++;
            midi->rxPending++;
        }
    }
    archSemaphoreSignal(midi->semaphore);
}

static void onRecv(YM2148* midi, UInt32 time)
{
    midi->timeRecv = 0;

    if (midi->command & CMD_RXEN) {
	    if (midi->status & STAT_RXRBSY) {
            midi->status |= STAT_OE;
            if (midi->command & CMD_RSTER) {
                ym2148Reset(midi);
                return;
            }
	    } 
    
        if (midi->rxPending != 0) {
            archSemaphoreWait(midi->semaphore, -1);
            midi->rxData = midi->rxQueue[(midi->rxHead - midi->rxPending) & (RX_QUEUE_SIZE - 1)];
            midi->rxPending--;
            archSemaphoreSignal(midi->semaphore);
            midi->status |= STAT_RXRBSY;
            if (midi->command & CMD_RXINT) {
                boardSetDataBus(midi->vector, 0, 0);
                boardSetInt(0x800);
                midi->status |= ST_INT;
            }
        }
    }

    midi->timeRecv = boardSystemTime() + midi->charTime;
    boardTimerAdd(midi->timerRecv, midi->timeRecv);
}

static void onTrans(YM2148* midi, UInt32 time)
{
    midi->timeTrans  = 0;

    midiIoTransmit(midi->midiIo, midi->sendByte);
	if (midi->status &  STAT_TXRDY) {
		midi->status |= STAT_TXEMPTY;
        if (midi->command & CMD_TXINT) {
            boardSetDataBus(midi->vector, 0, 0);
            boardSetInt(0x800);
            midi->status |= ST_INT;
        }
	}
    else {
		midi->status |= STAT_TXRDY;
	    midi->status &= ~STAT_TXEMPTY;
	    midi->sendByte = midi->sendBuffer;

        midi->timeTrans = boardSystemTime() + midi->charTime;
        boardTimerAdd(midi->timerTrans, midi->timeTrans);
	}
}

static void ym2148Reset(YM2148* midi)
{
    midi->status = STAT_TXEMPTY;
    midi->txPending = 0;
    midi->rxPending = 0;
    midi->command = 0;
    midi->timeRecv = 0;
    midi->timeTrans = 0;
    midi->charTime = 10 * boardFrequency() / 31250;

    boardTimerRemove(midi->timerRecv);
    boardTimerRemove(midi->timerTrans);
    
    midi->timeRecv = boardSystemTime() + midi->charTime;
    boardTimerAdd(midi->timerRecv, midi->timeRecv);
}

static YM2148* ym2148Create()
{
    YM2148* midi = (YM2148*)calloc(1, sizeof(YM2148));

    midi->midiIo = midiIoCreate(midiInCallback, midi);
    midi->semaphore = archSemaphoreCreate(1);
    midi->timerRecv   = boardTimerCreate(onRecv, midi);
    midi->timerTrans  = boardTimerCreate(onTrans, midi);

    ym2148Reset(midi);

    return midi;
}

static void ym2148Destroy(YM2148* midi)
{
    midiIoDestroy(midi->midiIo);
    archSemaphoreDestroy(midi->semaphore);
}

static UInt8 ym2148ReadStatus(YM2148* midi)
{
    UInt8 val = midi->status;

    boardClearInt(0x800);
    midi->status &= ~ST_INT;

    return val;
}

static UInt8 ym2148ReadData(YM2148* midi)
{
    midi->status &= ~STAT_OE;
    midi->status &= ~STAT_RXRBSY;
    return midi->rxData;
}

static void ym2148SetVector(YM2148* midi, UInt8 value)
{
    midi->vector = value;
    boardSetDataBus(midi->vector, 0, 0);
}

static void ym2148WriteCommand(YM2148* midi, UInt8 value)
{
    UInt8 oldValue = midi->command;

    midi->command = value;
    
    if (value & CMD_RST) {
        ym2148Reset(midi);
        return;
    }

	if (value & CMD_TXINT) {
        if ((midi->status & STAT_TXEMPTY) && (value & CMD_TXEN)) {
                if (midi->command & CMD_TXINT) {
                boardSetDataBus(midi->vector, 0, 0);
                boardSetInt(0x800);
                midi->status |= ST_INT;
            }
        }
    }
}

static void ym2148WriteData(YM2148* midi, UInt8 value)
{
    if (!(midi->command & CMD_TXEN)) {
		return;
	}
	if (midi->status & STAT_TXEMPTY) {
	    midi->status &= ~STAT_TXEMPTY;
	    midi->sendByte = value;

        midi->timeTrans = boardSystemTime() + midi->charTime;
        boardTimerAdd(midi->timerTrans, midi->timeTrans);
	} 
    else {
		midi->sendBuffer = value;
		midi->status &= ~STAT_TXRDY;
	}
}


static void ym2148SaveState(YM2148* midi)
{
    SaveState* state = saveStateOpenForWrite("ym2148");

    saveStateSet(state, "command", midi->command);
    saveStateSet(state, "rxData", midi->rxData);
    saveStateSet(state, "status", midi->status);
    saveStateSet(state, "sendByte", midi->sendByte);
    saveStateSet(state, "sendBuffer", midi->sendBuffer);
    saveStateSet(state, "sendByte", midi->sendByte);
    saveStateSet(state, "txPending", midi->txPending);
    saveStateSetBuffer(state, "rxQueue", midi->rxQueue, sizeof(midi->rxQueue));
    saveStateSet(state, "rxHead", midi->rxHead);
    saveStateSet(state, "vector", midi->vector);
    
    saveStateClose(state);
}

static void ym2148LoadState(YM2148* midi)
{
    SaveState* state = saveStateOpenForRead("ym2148");
    
    midi->command = saveStateGet(state, "command", 0);
    midi->rxData = saveStateGet(state, "rxData", 0);
    midi->status = saveStateGet(state, "status", 0);
    midi->sendByte = saveStateGet(state, "sendByte", 0); 
    midi->sendBuffer = saveStateGet(state, "sendBuffer", 0);
    midi->sendByte = saveStateGet(state, "sendByte", 0);
    midi->txPending = saveStateGet(state, "txPending", 0);
    saveStateGetBuffer(state, "rxQueue", midi->rxQueue, sizeof(midi->rxQueue));
    midi->rxHead = saveStateGet(state, "rxHead", 0);
    midi->vector = saveStateGet(state, "vector", 0);

    saveStateClose(state);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



typedef struct {
    int deviceHandle;
    int debugHandle;
    YM2151* ym2151;
    YM2148* ym2148;
    UInt8* romData; 
    int slot;
    int sslot;
    int startPage;
    int sizeMask;
    MidiIO* ykIo; 
    UInt8 kbdLatch;
} RomMapperSfg05;

static int deviceCount = 0;

static void saveState(RomMapperSfg05* rm)
{
    SaveState* state = saveStateOpenForWrite("mapperSfg05");
    
    saveStateSet(state, "kbdLatch", rm->kbdLatch);
    
    saveStateClose(state);

    ym2151SaveState(rm->ym2151);
    ym2148SaveState(rm->ym2148);
}

static void loadState(RomMapperSfg05* rm)
{
    SaveState* state = saveStateOpenForRead("mapperSfg05");

    rm->kbdLatch = (UInt8)saveStateGet(state, "kbdLatch", 0);

    saveStateClose(state);
    
    ym2151LoadState(rm->ym2151);
    ym2148LoadState(rm->ym2148);
}

static void destroy(RomMapperSfg05* rm)
{
    deviceCount--;

    if (rm->ym2151 != NULL) {
        ym2151Destroy(rm->ym2151);
    }
    if (rm->ym2148 != NULL) {
        ym2148Destroy(rm->ym2148);
    }

    if (rm->ykIo != NULL) {
        ykIoDestroy(rm->ykIo);
    }

    slotUnregister(rm->slot, rm->sslot, rm->startPage);

    debugDeviceUnregister(rm->debugHandle);
    deviceManagerUnregister(rm->deviceHandle);

    if (rm->romData != NULL) {
        free(rm->romData);
    }
    free(rm);
}

#define YK01_KEY_START 37

static UInt8 getKbdStatus(RomMapperSfg05* rm)
{
    UInt8 val = 0xff;
    int row;

    for (row = 0; row < 8; row++) {
        if ((1 << row) & rm->kbdLatch) {
            val &= ykIoGetKeyState(rm->ykIo, YK01_KEY_START + row * 6 + 0) ? ~0x01 : 0xff;
            val &= ykIoGetKeyState(rm->ykIo, YK01_KEY_START + row * 6 + 1) ? ~0x02 : 0xff;
            val &= ykIoGetKeyState(rm->ykIo, YK01_KEY_START + row * 6 + 2) ? ~0x04 : 0xff;
            val &= ykIoGetKeyState(rm->ykIo, YK01_KEY_START + row * 6 + 3) ? ~0x10 : 0xff;
            val &= ykIoGetKeyState(rm->ykIo, YK01_KEY_START + row * 6 + 4) ? ~0x20 : 0xff;
            val &= ykIoGetKeyState(rm->ykIo, YK01_KEY_START + row * 6 + 5) ? ~0x40 : 0xff;
        }
    }

    return val;
}

static UInt8 read(RomMapperSfg05* rm, UInt16 address) 
{
    if (address < 0x3ff0 || address >= 0x3ff8) {
    	return rm->romData[address & rm->sizeMask];
    }

    switch (address & 0x3fff) {
    case 0x3ff0:
        return ym2151Read(rm->ym2151, 0);
    case 0x3ff1:
        return ym2151Read(rm->ym2151, 1);
    case 0x3ff2:
        return getKbdStatus(rm);
    case 0x3ff5:
        return ym2148ReadData(rm->ym2148);
    case 0x3ff6:
        return ym2148ReadStatus(rm->ym2148);
    }

    return 0xff;
}

static void reset(RomMapperSfg05* rm) 
{
    ym2151Reset(rm->ym2151);
    ym2148Reset(rm->ym2148);
    rm->kbdLatch = 0;
}

static void write(RomMapperSfg05* rm, UInt16 address, UInt8 value) 
{
    if (address < 0x3ff0 || address >= 0x3ff8) {
    	return;
    }
    
    switch (address & 0x3fff) {
    case 0x3ff0:
        ym2151Write(rm->ym2151, 0, value);
        break;
    case 0x3ff1:
        ym2151Write(rm->ym2151, 1, value);
        break;
    case 0x3ff2:
        rm->kbdLatch = value;
        break;
    case 0x3ff3:
        ym2148SetVector(rm->ym2148, value);
        break;
    case 0x3ff4:
        boardSetDataBus(value, value, 1);
        ym2151SetIrqVector(rm->ym2151, value);
        break;
    case 0x3ff5:
        ym2148WriteData(rm->ym2148, value);
        break;
    case 0x3ff6:
        ym2148WriteCommand(rm->ym2148, value);
        break;
    }
}


static void getDebugInfo(RomMapperSfg05* rm, DbgDevice* dbgDevice)
{
    ym2151GetDebugInfo(rm->ym2151, dbgDevice);
}

int romMapperSfg05Create(const char* filename, UInt8* romData, 
                            int size, int slot, int sslot, int startPage) 
{
    DeviceCallbacks callbacks = { destroy, reset, saveState, loadState };
    DebugCallbacks dbgCallbacks = { getDebugInfo, NULL, NULL, NULL };
    RomMapperSfg05* rm;
    int i;
    int pages = size / 0x2000;
    
    if (size != 0x4000 && size != 0x8000) {
        return 0;
    }

    rm = malloc(sizeof(RomMapperSfg05));

    rm->deviceHandle = deviceManagerRegister(pages == 2 ? ROM_YAMAHASFG01 : ROM_YAMAHASFG05, &callbacks, rm);
    rm->debugHandle = debugDeviceRegister(DBGTYPE_AUDIO, langDbgDevSfg05(), &dbgCallbacks, rm);

    slotRegister(slot, sslot, startPage, pages, read, read, write, destroy, rm);

    rm->romData = malloc(size);
    memcpy(rm->romData, romData, size);
    rm->slot  = slot;
    rm->sslot = sslot;
    rm->startPage  = startPage;
    rm->sizeMask = size - 1;

    for (i = 0; i < pages; i++) {
        slotMapPage(rm->slot, rm->sslot, rm->startPage + i, NULL, 0, 0);
    }

    rm->ym2151 = ym2151Create(boardGetMixer());
    rm->ym2148 = ym2148Create();
    rm->ykIo = ykIoCreate();

    reset(rm);

    return 1;
}
