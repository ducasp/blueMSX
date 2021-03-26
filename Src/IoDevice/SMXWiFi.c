//
//  SMXWiFi.c
//
//  Created by Oduvaldo Pavan Junior May 27th 2020
//  For commercial usage contact author @ ducasp@gmail.com
//	Free for opensource / non-commercial usages
//
#include <windows.h>
#include "SMXWiFi.h"
#include "IoPort.h"
#include "Board.h"
#include "SaveState.h"
#include "DebugDeviceManager.h"
#include "Language.h"
#include "ArchUart.h"
#include "Properties.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#define SMXWIFI_MUTEX
//#define SMXWIFI_LOG
#ifdef SMXWIFI_LOG
#include <stdio.h>

FILE *logfile = NULL;
unsigned char ucLogString[255];
unsigned char ucIsReadLog;
unsigned char ucIsFirstLog;
#endif
unsigned char bFirstReceiveAfterTx = 0;
unsigned char bBufferUnderrun = 0;
ULONGLONG ullTicks;
const DWORD dwUartSpeedTable[10] = {859372, 346520, 231014, 115200, 57600, 38400, 31250, 19200, 9600, 4800};
#ifdef SMXWIFI_MUTEX
HANDLE ghMutex; 
static UInt8 Initialized = FALSE;
#endif

struct SMXWiFi_STATE
{
	UInt8 UARTSpeed;
    UInt8 hasCommPort;
	UInt8 TXInProgress;
	unsigned int FIFOHead;
	unsigned int FIFOTail;
    unsigned char FIFOBuffer[SMXWiFi_FIFO_Depth];
} state;

void smxWiFiFIFOReset()
{
	state.FIFOHead = 0;
	state.FIFOTail = 0;
	state.TXInProgress = 0;
}

void smxWiFiTXDone(void)
{
	state.TXInProgress = 0;
}

void smxWiFiFIFOIn(UInt8 value)
{
#ifdef SMXWIFI_LOG
	if((ucIsFirstLog)||(!ucIsReadLog))
	{
		ucIsFirstLog = FALSE;
		ucIsReadLog = TRUE;
		sprintf(ucLogString,"\r\n<- %02x ",value);
	}
	else
		sprintf(ucLogString,"%02x ",value);
	fwrite(ucLogString,strlen(ucLogString),1,logfile);
#endif
#ifdef SMXWIFI_MUTEX
	WaitForSingleObject(ghMutex, INFINITE);
#endif
	if(!(state.FIFOHead == (( state.FIFOTail - 1 + SMXWiFi_FIFO_Depth) % SMXWiFi_FIFO_Depth)))
	{
		state.FIFOBuffer[state.FIFOHead] = value;
		state.FIFOHead = (state.FIFOHead+1) % SMXWiFi_FIFO_Depth;
	}
#ifdef SMXWIFI_LOG
	else
	{
		ucIsFirstLog = TRUE;
		sprintf(ucLogString,"\r\nERROR! FIFO FULL!",value);
		fwrite(ucLogString,strlen(ucLogString),1,logfile);
	}
#endif
#ifdef SMXWIFI_MUTEX
	ReleaseMutex(ghMutex);
#endif
}

UInt8 smxWiFiFIFOOut()
{
	UInt8 value = 0xff; //If no data in FIFO, returns 0xff	
#ifdef SMXWIFI_MUTEX
	WaitForSingleObject(ghMutex, INFINITE);
#endif
	if (state.FIFOTail == state.FIFOHead)
	{
		if (bFirstReceiveAfterTx)
		{
			ullTicks = GetTickCount64() + 30;
			bFirstReceiveAfterTx = 0;
		}
		else if (ullTicks > (GetTickCount64() +30)) //safe guard for uninitialized state
			ullTicks = GetTickCount64() + 30;
		do
		{
			Sleep(0);
		}
		while ((GetTickCount64()<ullTicks)&&(state.FIFOTail == state.FIFOHead));
	}
	if (state.FIFOTail != state.FIFOHead)
	{
		value = state.FIFOBuffer[state.FIFOTail];
		state.FIFOTail = (state.FIFOTail + 1) % SMXWiFi_FIFO_Depth;
	}
	else
		bBufferUnderrun = 1;
#ifdef SMXWIFI_LOG
	else
	{
		ucIsFirstLog = TRUE;
		sprintf(ucLogString,"\r\nERROR! FIFO EMPTY!",value);
		fwrite(ucLogString,strlen(ucLogString),1,logfile);
	}
#endif
#ifdef SMXWIFI_MUTEX
	ReleaseMutex(ghMutex);
#endif
	return value;
}

static UInt8 smxWiFiReadUARTStatus(struct SMXWiFi_STATE* state, UInt16 ioPort)
{
	UInt8 currentStatus=0;
#ifdef SMXWIFI_MUTEX
	WaitForSingleObject(ghMutex, INFINITE);
#endif
	if (state->FIFOTail != state->FIFOHead)
		currentStatus|=1; //set rx buffer has data
	if (state->TXInProgress)
		currentStatus|=2; //set tx in progress
	if (state->FIFOHead == (( state->FIFOTail - 1 + SMXWiFi_FIFO_Depth) % SMXWiFi_FIFO_Depth))
		currentStatus|=4; //set rx buffer is full
	currentStatus|=8; // Set support to quick receive
	if (bBufferUnderrun)
	{
		currentStatus|=0x10; // Set buffer underrun
		bBufferUnderrun = 0;
	}
#ifdef SMXWIFI_MUTEX
	ReleaseMutex(ghMutex);
#endif
    return currentStatus;
}

static UInt8 smxWiFiReadUARTBuffer(struct SMXWiFi_STATE* state, UInt16 ioPort)
{    
    return smxWiFiFIFOOut();
}

static void smxWiFiWriteUARTBuffer(struct SMXWiFi_STATE* state, UInt16 ioPort, UInt8 value)
{
#ifdef SMXWIFI_LOG
	if((ucIsFirstLog)||(ucIsReadLog))
	{
		ucIsFirstLog = FALSE;
		ucIsReadLog = FALSE;
		sprintf(ucLogString,"\r\n-> %02x ",value);
	}
	else
		sprintf(ucLogString,"%02x ",value);
	fwrite(ucLogString,strlen(ucLogString),1,logfile);
#endif
	bFirstReceiveAfterTx = 1;
	if ((state->hasCommPort))
	{
		archUartTransmit(value);
		//state->TXInProgress = 1;
	}
}

static void smxWiFiWriteUARTCommand(struct SMXWiFi_STATE* state, UInt16 ioPort, UInt8 value)
{
	if(value<=UART_SPEED_4800) //Set UART Speed
	{
		state->UARTSpeed = value;
		//Set UART Speed
		archUartSetSpeed(dwUartSpeedTable[value]);
	}
	else if (value == UART_CLEAR_BUFFER)
	{
		//Clear any data in UART buffers
		smxWiFiFIFOReset();
	}
}

void smxWiFiCreate ()
{
	Properties* pProperties = propGetGlobalProperties();
	
#ifdef SMXWIFI_LOG
	logfile = fopen("c:\\temp\\esplog.txt","w+");
	ucIsFirstLog = TRUE;
#endif
	smxWiFiReset();
#ifdef SMXWIFI_MUTEX
	ghMutex = CreateMutex( NULL, FALSE, NULL);
	Initialized = TRUE;
#endif
	if (pProperties->ports.Com.directuartio == 1)
		state.hasCommPort = archUartCreate2(smxWiFiFIFOIn, smxWiFiTXDone, dwUartSpeedTable[state.UARTSpeed]);
	else
		state.hasCommPort = FALSE;
    ioPortRegister(0x06, smxWiFiReadUARTBuffer, smxWiFiWriteUARTCommand,  (void*) &state);
    ioPortRegister(0x07, smxWiFiReadUARTStatus, smxWiFiWriteUARTBuffer,  (void*) &state);
}

void smxWiFiDestroy ()
{
#ifdef SMXWIFI_LOG
	fclose(logfile);
#endif
    ioPortUnregister(0x06);
    ioPortUnregister(0x07);
	archUartDestroy();
#ifdef SMXWIFI_MUTEX
	CloseHandle(ghMutex);
	Initialized = FALSE;
#endif	
}

void smxWiFiReset()
{
#ifdef SMXWIFI_MUTEX
	if (Initialized)
		WaitForSingleObject(ghMutex, INFINITE);
#endif
	state.UARTSpeed = 0x00; //start at fastest UART speed
	smxWiFiFIFOReset();
#ifdef SMXWIFI_MUTEX
	if (Initialized)
		ReleaseMutex(ghMutex);
#endif
}