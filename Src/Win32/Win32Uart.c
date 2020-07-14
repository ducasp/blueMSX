/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Win32/Win32Uart.c,v $
**
** $Revision: 1.8 $
**
** $Date: 2008-03-31 19:42:24 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik, Tomas Karlsson
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
#include <windows.h>
#include "Win32Uart.h"
#include "ArchUart.h"
#include "Properties.h"

static HANDLE hComPort = INVALID_HANDLE_VALUE; 
static HANDLE hReadThread = NULL;
static BOOL bUartInitialized = FALSE;
static BOOL bMode2 = FALSE;
static BYTE ucReceiveBuf = 0;
static void (*uartReceiveCallback)(BYTE value);
static void (*uartControlCallback)(BYTE value);
static DWORD dwUartSpeed = CBR_9600;
static OVERLAPPED osWrite = {0};

static DWORD PortReadThread (LPVOID lpvoid)
{
	BYTE value;
	DWORD dwCommModemStatus;
	DWORD dwBytesTransferred;	

	// Specify a set of events to be monitored for the port.
	SetCommMask (hComPort, EV_RXCHAR);

	while (hComPort != INVALID_HANDLE_VALUE) 
	{
		// Wait for an event to occur for the port.
		WaitCommEvent (hComPort, &dwCommModemStatus, 0);

		// Re-specify the set of events to be monitored for the port.
		SetCommMask (hComPort, EV_RXCHAR);

		if (dwCommModemStatus & EV_RXCHAR) 
		{
			// Read the data from the serial port.
			ReadFile(hComPort, &value, 1, &dwBytesTransferred, 0);

			// Display the data read.
			if (dwBytesTransferred == 1)
			{				
                if (uartReceiveCallback != NULL)
                    (*uartReceiveCallback) (value);
			}
		}

		// Retrieve modem control-register values.
		GetCommModemStatus (hComPort, &dwCommModemStatus);
	}

	return 0;
}

static DWORD PortReadThread2 (LPVOID lpvoid)
{
	BYTE value;
	DWORD dwBytesTransferred;
	DWORD dwRes;
	OVERLAPPED osReader = {0};

	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osReader.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	while (hComPort != INVALID_HANDLE_VALUE) 
	{
		dwBytesTransferred = 0;
		// Read the data from the serial port.
		if (!ReadFile(hComPort, &value, 1, &dwBytesTransferred, &osReader))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				dwRes = WaitForSingleObject(osReader.hEvent, INFINITE);
				if(dwRes==WAIT_OBJECT_0)
				{
					if (!GetOverlappedResult(hComPort, &osReader, &dwBytesTransferred, FALSE))
						dwBytesTransferred = 0;
				}
			}
		}

		// Display the data read.
		if ((dwBytesTransferred)&&(uartReceiveCallback != NULL))
            (*uartReceiveCallback) (value);
	}

	CloseHandle(osReader.hEvent);

	return 0;
}

static BOOL uartCreate(void)
{
    Properties* pProperties = propGetGlobalProperties();

    COMMTIMEOUTS commTimeouts;
    DCB dcbConfig;
    DWORD dwThreadID;

	bMode2 = FALSE;
    // Alread initialized?
    if (bUartInitialized)
        return TRUE;

    // Open the serial port
    hComPort = CreateFile(pProperties->ports.Com.portName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    dcbConfig.DCBlength = sizeof (DCB);

    // Get the default port setting information
    if (GetCommState(hComPort,&dcbConfig) == 0)
        return FALSE;
		
    SetupComm(hComPort, 32, 32);
		
    // Change the DCB structure settings
    dcbConfig.BaudRate = dwUartSpeed;       // Current baud 
    dcbConfig.fBinary = TRUE;               // Binary mode; no EOF check 
    dcbConfig.fParity = TRUE;               // Enable parity checking 
    dcbConfig.fOutxCtsFlow = FALSE;         // No CTS output flow control 
    dcbConfig.fOutxDsrFlow = FALSE;         // No DSR output flow control 
    dcbConfig.fDtrControl = DTR_CONTROL_ENABLE; // DTR flow control type 
    dcbConfig.fDsrSensitivity = TRUE;      // DSR sensitivity 
    dcbConfig.fTXContinueOnXoff = FALSE;     // XOFF continues Tx 
    dcbConfig.fOutX = FALSE;                // No XON/XOFF out flow control 
    dcbConfig.fInX = FALSE;                 // No XON/XOFF in flow control 
    dcbConfig.fErrorChar = FALSE;           // Disable error replacement 
    dcbConfig.fNull = FALSE;                // Disable null stripping 
    dcbConfig.fRtsControl = RTS_CONTROL_DISABLE; //Disable RTS flow control
    dcbConfig.fAbortOnError = FALSE;        // Do not abort reads/writes on error
    dcbConfig.ByteSize = 8;                 // Number of bits/byte, 4-8 
    dcbConfig.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space 
    dcbConfig.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2 
  
    // Configure the port according to the specifications of the DCB structure
    if (!SetCommState (hComPort, &dcbConfig))
        return FALSE;

    // Retrieve the time-out parameters for all read and write operations on the port
	GetCommTimeouts (hComPort, &commTimeouts);

    // Change the COMMTIMEOUTS structure settings
    commTimeouts.ReadIntervalTimeout = MAXDWORD;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
    commTimeouts.ReadTotalTimeoutConstant = 0;
    commTimeouts.WriteTotalTimeoutMultiplier = 0;
    commTimeouts.WriteTotalTimeoutConstant = 0;

    // Set the time-out parameters for all read and write operations on the port
    if (!SetCommTimeouts(hComPort, &commTimeouts))
        return FALSE;

    // Create a read thread for reading data from the communication port
    hReadThread = CreateThread(NULL, 
                          0,
                          (LPTHREAD_START_ROUTINE) PortReadThread,
                          0, 
                          0, 
                          &dwThreadID);

    if (hReadThread == NULL)
		return FALSE;

    bUartInitialized = TRUE;

    return TRUE;
}

static BOOL uartCreate2(void)
{
    Properties* pProperties = propGetGlobalProperties();
	COMMTIMEOUTS commTimeouts;
    DCB dcbConfig;
    DWORD dwThreadID;
	char myPortName[600];

	bMode2=TRUE;
    // Alread initialized?
    if (bUartInitialized)
        return TRUE;

	// A must for COM10 a beyond
	sprintf(myPortName,"\\\\.\\");
	strcat(myPortName,pProperties->ports.Com.portName);

    // Open the serial port
    hComPort = CreateFile(myPortName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (hComPort == INVALID_HANDLE_VALUE)
		return FALSE;

	// Create this write operation's OVERLAPPED structure's hEvent.
	osWrite.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (osWrite.hEvent == NULL)
		// error creating overlapped event handle
		return FALSE;

    dcbConfig.DCBlength = sizeof (DCB);

    // Get the default port setting information
    if (GetCommState(hComPort,&dcbConfig) == 0)
        return FALSE;
		
    SetupComm(hComPort, 4096, 4096);
		
    // Change the DCB structure settings
    dcbConfig.BaudRate = dwUartSpeed;       // Current baud 
    dcbConfig.fBinary = TRUE;               // Binary mode; no EOF check 
    dcbConfig.fParity = FALSE;               // Disable parity checking 
    dcbConfig.fOutxCtsFlow = FALSE;         // No CTS output flow control 
    dcbConfig.fOutxDsrFlow = FALSE;         // No DSR output flow control 
    dcbConfig.fDtrControl = DTR_CONTROL_DISABLE; // DTR flow control type 
    dcbConfig.fDsrSensitivity = FALSE;      // DSR sensitivity 
    dcbConfig.fTXContinueOnXoff = FALSE;     // XOFF continues Tx 
    dcbConfig.fOutX = FALSE;                // No XON/XOFF out flow control 
    dcbConfig.fInX = FALSE;                 // No XON/XOFF in flow control 
    dcbConfig.fErrorChar = FALSE;           // Disable error replacement 
    dcbConfig.fNull = FALSE;                // Disable null stripping 
    dcbConfig.fRtsControl = RTS_CONTROL_DISABLE; //Disable RTS flow control
    dcbConfig.fAbortOnError = TRUE;        // abort reads/writes on error
    dcbConfig.ByteSize = 8;                 // Number of bits/byte, 4-8 
    dcbConfig.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space 
    dcbConfig.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2 
  
    // Configure the port according to the specifications of the DCB structure
    if (!SetCommState (hComPort, &dcbConfig))
        return FALSE;

	// Change the COMMTIMEOUTS structure settings
    commTimeouts.ReadIntervalTimeout = MAXDWORD;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
    commTimeouts.ReadTotalTimeoutConstant = 0;
    commTimeouts.WriteTotalTimeoutMultiplier = 0;
    commTimeouts.WriteTotalTimeoutConstant = 0;

    // Set the time-out parameters for all read and write operations on the port
    if (!SetCommTimeouts(hComPort, &commTimeouts))
        return FALSE;

    // Create a read thread for reading data from the communication port
    hReadThread = CreateThread(NULL, 
                          0,
                          (LPTHREAD_START_ROUTINE) PortReadThread2,
                          0, 
                          0, 
                          &dwThreadID);

	if (hReadThread == NULL)
		return FALSE;

    bUartInitialized = TRUE;

    return TRUE;
}

static void uartDestroy(void)
{
    CloseHandle (hComPort);

	hComPort = INVALID_HANDLE_VALUE;

    if (hReadThread != NULL) {

        WaitForSingleObject(hReadThread, INFINITE);
	    CloseHandle(hReadThread);
        hReadThread = NULL;
    }

	if (bMode2)
	{
		if (osWrite.hEvent != NULL)
			CloseHandle(osWrite.hEvent);
	}
   
    bUartInitialized = FALSE;
}


static BOOL uartSetSpeed(DWORD dwSpeed)
{
	uartDestroy();
	dwUartSpeed = dwSpeed;
	return uartCreate2();
}

static void uartTransmit(BYTE value)
{
    DWORD dwNumBytesWritten = 0;

    WriteFile(hComPort, &value, 1, &dwNumBytesWritten, 0);
}

static void uartTransmit2(BYTE value)
{
	DWORD dwNumBytesWritten;
	WriteFile(hComPort, &value, 1, &dwNumBytesWritten, &osWrite);
}

int archUartSetSpeed(unsigned long dwNewSpeed)
{
	return uartSetSpeed(dwNewSpeed);
}

void archUartTransmit(BYTE value)
{
    if (bUartInitialized)
	{
		if (bMode2)
			uartTransmit2(value);
		else
			uartTransmit(value);
	}
}

int archUartCreate3(void (*archUartReceiveCallback) (BYTE), void (*archUartTransmitCallback) (void), void (*archUartControlCallback) (BYTE), DWORD dwCreationSpeed)
{
	uartReceiveCallback = archUartReceiveCallback;
	uartControlCallback = archUartControlCallback;
	dwUartSpeed = dwCreationSpeed;
	return uartCreate2();
}

int archUartCreate2(void (*archUartReceiveCallback) (BYTE), void (*archUartTransmitCallback) (void), DWORD dwCreationSpeed)
{
	uartControlCallback = NULL;
	uartReceiveCallback = archUartReceiveCallback;
	dwUartSpeed = dwCreationSpeed;
	return uartCreate2();
}

int archUartCreate(void (*archUartReceiveCallback) (BYTE))
{
	uartControlCallback = NULL;
    uartReceiveCallback = archUartReceiveCallback;
    return uartCreate();
}

void archUartDestroy(void)
{
    uartReceiveCallback = NULL;
    uartDestroy();
}
