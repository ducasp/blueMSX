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
#include <stdio.h>
#include "Win32Uart.h"
#include "ArchUart.h"
#include "Properties.h"

static HANDLE hComPort = INVALID_HANDLE_VALUE; 
static HANDLE hReadThread = NULL;
static HANDLE hStatusThread = NULL;
static BOOL bUartInitialized = FALSE;
static BOOL bMode2 = FALSE;
static BOOL bMode3 = FALSE;
static BYTE ucReceiveBuf = 0;
static void (*uartReceiveCallback)(BYTE value);
static void (*uartReceiveCallback2)(BYTE value,DWORD error);
static void (*uartControlCallback)(DWORD value);
static DWORD dwUartSpeed = CBR_9600;
static unsigned int uiParity = FALSE;
static unsigned int uiBits = 8;
static unsigned int uiParityScheme = NOPARITY;
static unsigned int uiStopBits = ONESTOPBIT;
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

static DWORD PortReadThread3 (LPVOID lpvoid)
{
	BYTE value;
	DWORD dwBytesTransferred;
	DWORD dwRes;
	DWORD dwErr;
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
					{
						dwBytesTransferred = 0;
						if (uartReceiveCallback2)
						{
							//ClearCommError seems to be de-asserting RTS and causing data to not be read
							ClearCommError(hComPort,&dwErr,NULL);
							(*uartReceiveCallback2) (value,dwErr);
						}
					}
					else if ((dwBytesTransferred)&&(uartReceiveCallback2 != NULL))
							(*uartReceiveCallback2) (value,0);
				}
			}
		}
		else // read immediatelly
			if (dwBytesTransferred)
			{
				//ClearCommError seems to be de-asserting RTS and causing data to not be read
				ClearCommError(hComPort,&dwErr,NULL);
				(*uartReceiveCallback2) (value,dwErr);
			}
	}

	CloseHandle(osReader.hEvent);

	return 0;
}

static DWORD PortReadStatus (LPVOID lpvoid)
{
	DWORD dwCommEvent,dwOvRes;
	DWORD dwRes;
	DWORD dwStoredFlags = EV_BREAK | EV_CTS | EV_DSR | EV_RING | EV_RLSD | EV_TXEMPTY ;
	OVERLAPPED osStatus = {0};

	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osStatus.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//SetCommMask(hComPort, dwStoredFlags);

	while (hComPort != INVALID_HANDLE_VALUE) 
	{
		if (!WaitCommEvent(hComPort, &dwCommEvent, &osStatus))
		{
            if (GetLastError() == ERROR_IO_PENDING)
			{
				dwRes = WaitForSingleObject(osStatus.hEvent, INFINITE);
				if(dwRes==WAIT_OBJECT_0)
				{
					if (GetOverlappedResult(hComPort, &osStatus, &dwOvRes, FALSE)&&(uartControlCallback))
						uartControlCallback(dwCommEvent);
				}
			}

        }
        else if (uartControlCallback)
			 //WaitCommEvent returned immediately.
			// Deal with status event as appropriate.
			uartControlCallback(dwCommEvent);
	}

	CloseHandle(osStatus.hEvent);

	return 0;
}

static BOOL uartCreate(void)
{
    Properties* pProperties = propGetGlobalProperties();

    COMMTIMEOUTS commTimeouts;
    DCB dcbConfig;
    DWORD dwThreadID;

	bMode2 = FALSE;
	bMode3 = FALSE;
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

static BOOL uartCreate3(void)
{
    Properties* pProperties = propGetGlobalProperties();
	COMMTIMEOUTS commTimeouts;
    DCB dcbConfig;
    DWORD dwThreadID;
	char myPortName[600];

	bMode2=TRUE;
	bMode3=TRUE;
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
		
    SetupComm(hComPort, 16, 16);
		
    // Change the DCB structure settings
    dcbConfig.BaudRate = dwUartSpeed;			// Current baud 
    dcbConfig.fBinary = TRUE;					// Binary mode; no EOF check 
    dcbConfig.fParity = uiParity;				// Disable parity checking 
    dcbConfig.fOutxCtsFlow = FALSE;				// No CTS output flow control 
    dcbConfig.fOutxDsrFlow = FALSE;				// No DSR output flow control 
    dcbConfig.fDtrControl = DTR_CONTROL_DISABLE;// DTR flow control type 
    dcbConfig.fDsrSensitivity = FALSE;			// DSR sensitivity 
    dcbConfig.fTXContinueOnXoff = FALSE;		// XOFF continues Tx 
    dcbConfig.fOutX = FALSE;					// No XON/XOFF out flow control 
    dcbConfig.fInX = FALSE;						// No XON/XOFF in flow control 
    dcbConfig.fErrorChar = FALSE;				// Disable error replacement 
    dcbConfig.fNull = FALSE;					// Disable null stripping 
    dcbConfig.fRtsControl = RTS_CONTROL_DISABLE;//Disable RTS flow control
    dcbConfig.fAbortOnError = TRUE;				// abort reads/writes on error
    dcbConfig.ByteSize = uiBits;				// Number of bits/byte, 4-8 
    dcbConfig.Parity = uiParityScheme;          // 0-4=no,odd,even,mark,space 
    dcbConfig.StopBits = uiStopBits;			// 0,1,2 = 1, 1.5, 2 
  
    // Configure the port according to the specifications of the DCB structure
	if (!SetCommState (hComPort, &dcbConfig)) {
		dwThreadID = GetLastError();
        return FALSE;
	}

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
                          (LPTHREAD_START_ROUTINE) PortReadThread3,
                          0, 
                          0, 
                          &dwThreadID);

	if (hReadThread == NULL)
		return FALSE;
	
	// Create a read thread for reading data from the communication port
	hStatusThread = CreateThread(NULL, 
                          0,
                          (LPTHREAD_START_ROUTINE) PortReadStatus,
                          0, 
                          0, 
                          &dwThreadID);

	if (hStatusThread == NULL)
		return FALSE;

    bUartInitialized = TRUE;

    return TRUE;
}

static void uartDestroy(void)
{
	DWORD dwError;
    if (!CloseHandle (hComPort))
		dwError = GetLastError();

	hComPort = INVALID_HANDLE_VALUE;

    if (hReadThread != NULL) {
        WaitForSingleObject(hReadThread, INFINITE);
	    CloseHandle(hReadThread);
        hReadThread = NULL;
    }

	if (hStatusThread != NULL) {
		WaitForSingleObject(hStatusThread, INFINITE);
	    CloseHandle(hStatusThread);
        hStatusThread = NULL;
    }

	if (bMode2)
	{
		if (osWrite.hEvent != NULL)
		{
			CloseHandle(osWrite.hEvent);
			osWrite.hEvent = NULL;
		}
		bMode2 = FALSE;
	}
   
	if (bMode3)
		bMode3 = FALSE;
    bUartInitialized = FALSE;
}

static BOOL uartSetLCR(DWORD dwSpeed)
{
	DCB dcbConfig;
	BOOL bReturn = FALSE;
	DWORD dwErr;

	dcbConfig.DCBlength = sizeof (DCB);
	if (!GetCommState(hComPort,&dcbConfig))
		dwErr = GetLastError();
	dwUartSpeed = dwSpeed;
	// Change the DCB structure settings
    dcbConfig.BaudRate = dwUartSpeed;       // Current baud 
    dcbConfig.fBinary = TRUE;               // Binary mode; no EOF check 
    dcbConfig.fParity = uiParity;           // Disable parity checking 
    dcbConfig.fOutxCtsFlow = FALSE;         // No CTS output flow control 
    dcbConfig.fOutxDsrFlow = FALSE;         // No DSR output flow control 
    dcbConfig.fDtrControl = DTR_CONTROL_DISABLE; // DTR flow control type 
    dcbConfig.fDsrSensitivity = FALSE;      // DSR sensitivity 
    dcbConfig.fTXContinueOnXoff = FALSE;    // XOFF continues Tx 
    dcbConfig.fOutX = FALSE;                // No XON/XOFF out flow control 
    dcbConfig.fInX = FALSE;                 // No XON/XOFF in flow control 
    dcbConfig.fErrorChar = FALSE;           // Disable error replacement 
    dcbConfig.fNull = FALSE;                // Disable null stripping 
    dcbConfig.fRtsControl = RTS_CONTROL_DISABLE; //Disable RTS flow control
    dcbConfig.fAbortOnError = TRUE;         // abort reads/writes on error
    dcbConfig.ByteSize = uiBits;            // Number of bits/byte, 4-8 
    dcbConfig.Parity = uiParityScheme;            // 0-4=no,odd,even,mark,space 
    dcbConfig.StopBits = uiStopBits;        // 0,1,2 = 1, 1.5, 2 
  
    // Configure the port according to the specifications of the DCB structure
	if (!SetCommState (hComPort, &dcbConfig)) {
		bReturn = FALSE;
		dwErr = GetLastError();
	}
	else
		bReturn = TRUE;

	return bReturn;


	//uartDestroy();
	//dwUartSpeed = dwSpeed;
	//return uartCreate3();
}

static BOOL uartSetSpeed(DWORD dwSpeed)
{
	BOOL bCreate3 = bMode3;

	if (bMode3)
		uartSetLCR(dwSpeed);

	if (dwSpeed!=dwUartSpeed)
	{
		uartDestroy();
		dwUartSpeed = dwSpeed;
		if (bCreate3)
			return uartCreate3();
		else
			return uartCreate2();
	}
	else
		return TRUE;
}

static void uartTransmit(BYTE value)
{
    DWORD dwNumBytesWritten = 0;

    WriteFile(hComPort, &value, 1, &dwNumBytesWritten, 0);
}

static void uartTransmit2(BYTE value)
{
	DWORD dwNumBytesWritten;
	if(!WriteFile(hComPort, &value, 1, &dwNumBytesWritten, &osWrite)) {
		dwNumBytesWritten = GetLastError();
	}
}

DWORD archUartGetModemStatus()
{
	DWORD dwModemStatus;
	GetCommModemStatus(hComPort,&dwModemStatus);
	return dwModemStatus;
}

void archUartEscapeCommFunction(unsigned long dwFunc)
{
	DWORD dwErr;
	if (!EscapeCommFunction(hComPort,dwFunc))
		dwErr=GetLastError();
}

void archUartSetLCR (unsigned char ucLCR)
{
unsigned int uiNParity = FALSE;
unsigned int uiNBits = 8;
unsigned int uiNParityScheme = NOPARITY;
unsigned int uiNStopBits = ONESTOPBIT;
	switch	(ucLCR&0x03)
	{
		case 0:
			uiNBits = 5;
			break;
		case 1:
			uiNBits = 6;
			break;
		case 2:
			uiNBits = 7;
			break;
		case 3:
			uiNBits = 8;
			break;
	}

	if (ucLCR&0x04)
	{
		if (uiNBits == 5)
			uiNStopBits = ONE5STOPBITS;
		else
			uiNStopBits = ONESTOPBIT; // some ports do not accept two stop bits
			//uiNStopBits = TWOSTOPBITS;
	}
	else
		uiNStopBits = ONESTOPBIT;

	if (ucLCR&0x08)
	{
		uiNParity = TRUE;
		if (ucLCR&0x20) // stick?
		{
			if (ucLCR&0x10)
				uiNParityScheme = MARKPARITY;
			else
				uiNParityScheme = SPACEPARITY;
		}
		else
		{
			if (ucLCR&0x10)
				uiNParityScheme = EVENPARITY;
			else
				uiNParityScheme = ODDPARITY;
		}
	}
	else
	{
		uiNParity = FALSE;
		uiNParityScheme = NOPARITY;
	}

	if ((uiNParity!=uiParity)||(uiNParityScheme!=uiParityScheme)||(uiNStopBits!=uiStopBits)||(uiNBits!=uiBits))
	{
		uiParity=uiNParity;
		uiParityScheme=uiNParityScheme;
		uiStopBits=uiNStopBits;
		uiBits=uiNBits;
		uartSetLCR(dwUartSpeed);
	}
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

int archUartCreate3(void (*archUartReceiveCallback) (BYTE, DWORD), void (*archUartControlCallback) (DWORD), DWORD dwCreationSpeed)
{
	uartReceiveCallback2 = archUartReceiveCallback;
	uartControlCallback = archUartControlCallback;
	dwUartSpeed = dwCreationSpeed;
	return uartCreate3();
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
	uartReceiveCallback2 = NULL;
	uartControlCallback = NULL;
    uartDestroy();
}
