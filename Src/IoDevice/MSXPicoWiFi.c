//
//  MSXPicoWiFi.c
//
//  Created by Oduvaldo Pavan Junior September 4th 2024
//  For commercial usage contact author @ ducasp@gmail.com
//	Free for opensource / non-commercial usages
//
#include <windows.h>
#include "MSXPicoWiFi.h"
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
//#define MSXPICOWIFI_MUTEX
//#define MSXPICOWIFI_LOG
#ifdef MSXPICOWIFI_LOG
#include <stdio.h>

FILE *logfile = NULL;
unsigned char ucLogString[255];
unsigned char ucIsReadLog;
unsigned char ucIsFirstLog;
#endif
static unsigned char bPicoFirstReceiveAfterTx = 0;
static unsigned char bPicoBufferUnderrun = 0;
static unsigned char ucF2 = 0xff;
ULONGLONG ullTicks;
static const DWORD dwPICOUartSpeedTable[10] = {859372, 346520, 231014, 115200, 57600, 38400, 31250, 19200, 9600, 4800};
#ifdef MSXPICOWIFI_MUTEX
HANDLE ghMutex; 
static UInt8 Initialized = FALSE;
#endif

struct MSXPicoWiFi_STATE
{
	UInt8 UARTSpeed;
    UInt8 hasCommPort;
	UInt8 TXInProgress;
	unsigned int FIFOHead;
	unsigned int FIFOTail;
    unsigned char FIFOBuffer[MSXPicoWiFi_FIFO_Depth];
} state;

static UInt8 msxPicoWiFiReadF2()
{
	return ucF2;
}

static void msxPicoWiFiWriteF2(UInt8 value)
{
	ucF2 = value;
}

static void msxPicoWiFiFIFOReset()
{
	state.FIFOHead = 0;
	state.FIFOTail = 0;
	state.TXInProgress = 0;
}

static void msxPicoWiFiTXDone(void)
{
	state.TXInProgress = 0;
}

static void msxPicoWiFiFIFOIn(UInt8 value)
{
#ifdef MSXPICOWIFI_LOG
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
#ifdef MSXPICOWIFI_MUTEX
	WaitForSingleObject(ghMutex, INFINITE);
#endif
	if(!(state.FIFOHead == (( state.FIFOTail - 1 + MSXPicoWiFi_FIFO_Depth) % MSXPicoWiFi_FIFO_Depth)))
	{
		state.FIFOBuffer[state.FIFOHead] = value;
		state.FIFOHead = (state.FIFOHead+1) % MSXPicoWiFi_FIFO_Depth;
	}
#ifdef MSXPicoWIFI_LOG
	else
	{
		ucIsFirstLog = TRUE;
		sprintf(ucLogString,"\r\nERROR! FIFO FULL!",value);
		fwrite(ucLogString,strlen(ucLogString),1,logfile);
	}
#endif
#ifdef MSXPicoWIFI_MUTEX
	ReleaseMutex(ghMutex);
#endif
}

static UInt8 msxPicoWiFiFIFOOut()
{
	UInt8 value = 0xff; //If no data in FIFO, returns 0xff	
#ifdef MSXPICOWIFI_MUTEX
	WaitForSingleObject(ghMutex, INFINITE);
#endif
	if (state.FIFOTail == state.FIFOHead)
	{
		if (bPicoFirstReceiveAfterTx)
		{
			ullTicks = GetTickCount64() + 30;
			bPicoFirstReceiveAfterTx = 0;
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
		state.FIFOTail = (state.FIFOTail + 1) % MSXPicoWiFi_FIFO_Depth;
	}
	else
		bPicoBufferUnderrun = 1;
#ifdef MSXPicoWIFI_LOG
	else
	{
		ucIsFirstLog = TRUE;
		sprintf(ucLogString,"\r\nERROR! FIFO EMPTY!",value);
		fwrite(ucLogString,strlen(ucLogString),1,logfile);
	}
#endif
#ifdef MSXPicoWIFI_MUTEX
	ReleaseMutex(ghMutex);
#endif
	return value;
}

static UInt8 msxPicoWiFiReadUARTStatus()
{
	UInt8 currentStatus=0;
#ifdef MSXPICOWIFI_MUTEX
	WaitForSingleObject(ghMutex, INFINITE);
#endif
	if (state.FIFOTail != state.FIFOHead)
		currentStatus|=1; //set rx buffer has data
	if (state.TXInProgress)
		currentStatus|=2; //set tx in progress
	if (state.FIFOHead == (( state.FIFOTail - 1 + MSXPicoWiFi_FIFO_Depth) % MSXPicoWiFi_FIFO_Depth))
		currentStatus|=4; //set rx buffer is full
	currentStatus|=8; // Set support to quick receive
	if (bPicoBufferUnderrun)
	{
		currentStatus|=0x10; // Set buffer underrun
		bPicoBufferUnderrun = 0;
	}
#ifdef MSXPICOWIFI_MUTEX
	ReleaseMutex(ghMutex);
#endif
    return currentStatus;
}

static UInt8 msxPicoWiFiReadUARTBuffer()
{    
    return msxPicoWiFiFIFOOut();
}

static void msxPicoWiFiWriteUARTBuffer(UInt8 value)
{
#ifdef MSXPICOWIFI_LOG
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
	bPicoFirstReceiveAfterTx = 1;
	if ((state.hasCommPort))
	{
		archUartTransmit(value);
		//state->TXInProgress = 1;
	}
}

static void msxPicoWiFiWriteUARTCommand(UInt8 value)
{
	if(value<=UART_SPEED_4800) //Set UART Speed
	{
		state.UARTSpeed = value;
		//Set UART Speed
		archUartSetSpeed(dwPICOUartSpeedTable[value]);
	}
	else if (value == UART_CLEAR_BUFFER)
	{
		//Clear any data in UART buffers
		msxPicoWiFiFIFOReset();
	}
}

static void msxPicoWiFiCreate ()
{
	Properties* pProperties = propGetGlobalProperties();
	
#ifdef MSXPICOWIFI_LOG
	logfile = fopen("c:\\temp\\esplog.txt","w+");
	ucIsFirstLog = TRUE;
#endif
	msxPicoWiFiReset();
#ifdef MSXPICOWIFI_MUTEX
	ghMutex = CreateMutex( NULL, FALSE, NULL);
	Initialized = TRUE;
#endif
	if (pProperties->ports.Com.directuartio == 5)
		state.hasCommPort = archUartCreate2(msxPicoWiFiFIFOIn, msxPicoWiFiTXDone, dwPICOUartSpeedTable[state.UARTSpeed]);
	else
		state.hasCommPort = FALSE;
}

static void msxPicoWiFiDestroy ()
{
#ifdef MSXPICOWIFI_LOG
	fclose(logfile);
#endif
	archUartDestroy();
#ifdef MSXPICOWIFI_MUTEX
	CloseHandle(ghMutex);
	Initialized = FALSE;
#endif	
}

static void msxPicoWiFiReset()
{
#ifdef MSXPICOWIFI_MUTEX
	if (Initialized)
		WaitForSingleObject(ghMutex, INFINITE);
#endif
	state.UARTSpeed = 0x00; //start at fastest UART speed
	msxPicoWiFiFIFOReset();
#ifdef MSXPICOWIFI_MUTEX
	if (Initialized)
		ReleaseMutex(ghMutex);
#endif
}