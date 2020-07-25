//
//  16550C.c
//
//  Created by Oduvaldo Pavan Junior July 10th 2020
//  For commercial usage contact author @ ducasp@gmail.com
//	Free for opensource / non-commercial usages
//
#include <windows.h>
#include <MMSYSTEM.h>
#include "16550C.h"
#include "IoPort.h"
#include "Board.h"
#include "SaveState.h"
#include "DebugDeviceManager.h"
#include "Language.h"
#include "Properties.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INT_U16550C	0x1000
#define UART_CLOCK_16X	1843200//0

UInt8 b16550CHoldStatusRegister = FALSE;
UInt8 b16550CHoldRTSUntilEmpty = FALSE;
UInt8 b16550CTXThreadRunning = FALSE;
UInt8 b16550CTXThreadWorking = FALSE;
UInt8 b16550CTXThreadGotMutex = FALSE;
ULONGLONG ull16550CTicks;
unsigned long ul16550CBaudRate;
HANDLE hTXThread = NULL;
HANDLE ghMutex;
HANDLE ghTimerMutex;
HANDLE ghTimerMutex2;
HANDLE hSerialPort = INVALID_HANDLE_VALUE; 
HANDLE hRxThread = NULL;
HANDLE hStsThread = NULL;
OVERLAPPED os_Write = {0};
unsigned int ui_Parity = FALSE;
unsigned int ui_Bits = 8;
unsigned int ui_ParityScheme = NOPARITY;
unsigned int ui_StopBits = ONESTOPBIT;
MMRESULT timerId = 0;

struct U16550C_STATE
{
    UInt8 hasCommPort;
	UInt8 ReceiveBufferRegister;
	UInt8 TransmitterHoldingRegister;
	UInt8 InterruptEnableRegister;
	UInt8 InterruptIdentificationRegister;
	UInt8 FIFOControlRegister;
	UInt8 LineControlRegister;
	UInt8 ModemControlRegister;
	UInt8 LineStatusRegister;
	UInt8 ModemStatusRegister;
	UInt8 ScratchRegister;
	UInt8 DivisorLatchLSB;
	UInt8 DivisorLatchMSB;
	UInt8 RXDataInt;
	UInt8 RXDataToutInt;
	UInt8 TXEmptyInt;
	UInt8 LSRInt;
	UInt8 MSRInt;
	UInt8 TXEmpty;
	UInt8 RXEmpty;
	unsigned int FIFORxLevel;
	unsigned int FIFORxHead;
	unsigned int FIFORxTail;
	unsigned int FIFOTxHead;
	unsigned int FIFOTxTail;
    unsigned char FIFORxBuffer[U16550C_FIFO_Depth];
	unsigned long FIFORxError[U16550C_FIFO_Depth];
	unsigned char FIFOTxBuffer[U16550C_FIFO_Depth];
} state;

// Win32 Uart Code

// Ok, there is a catch here...
// On real serial ports this should not happen, but USB converters or BlueTooth adapters are what
// make most of the current COMM ports on PCs. Both USB and Bluetooth have latency to some degree
// (i.e.: FTDI USB -> RS232 has a minimum 1ms latency), so this causes a few unwanted effects:
//
// 1 - Control Signals might take a few more ms to arrive at the port / other end, so device might
//     think it is ok to send data, while the "virtual UART" is full and RTS was not cleared on the
//     physical port
//
// 2 - Instead of receiving data on a steady pace driving by the baudrate, we are going to receive
//     a burst of more than what our FIFO can handle before even having the chance to clear RTS or
//     MSX retrieving it. (I.e.: 128 bytes are sent over serial port, but due to the latency, by the
//     time the driver retrieves something from USB->Serial converter, the converter own FIFO already
//     has 64 bytes or more and pack it on a single USB report, so, we go from no data to 64 bytes all
//     of sudden, and this of course will cause a "virtual baud rate" for those 64 bytes that popped up
//     immediately much higher than anything that a MSX can handle, so FIFO will OVERRUN....
//
// Given that, at expense of detecting real OVERRUNS for hardware that use a REAL UART on an interface,
// this emulation will wait up to 100ms every time FIFO is FULL, before overruning it. That is why we
// set-up a big buffer when opening the serial connection. The other end won't stop at the byte or the next
// byte after RTS is cleared, it will take time to propagate to the physical port and at the same time, more
// data might come in packets. On real, physical 16550 on PCs, that should not be an issue, but those are
// scarce and don't want to require it for proper working.
DWORD U16550C_PortReadThread (LPVOID lpvoid)
{
	BYTE value;
	DWORD dwBytesTransferred;
	DWORD dwRes;
	DWORD dwErr;
	OVERLAPPED osReader = {0};
	ULONGLONG ullTicks;

	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osReader.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	while (hSerialPort != INVALID_HANDLE_VALUE) 
	{
		dwBytesTransferred = 0;
		// Read the data from the serial port.
		if (!ReadFile(hSerialPort, &value, 1, &dwBytesTransferred, &osReader))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				dwRes = WaitForSingleObject(osReader.hEvent, INFINITE);
				if(dwRes==WAIT_OBJECT_0)
				{
					ullTicks = GetTickCount64() + 100;
					while (U16550C_RxFIFOFull()&&(ullTicks>GetTickCount64()))
						Sleep(0);
					if (!GetOverlappedResult(hSerialPort, &osReader, &dwBytesTransferred, FALSE))
					{
						dwBytesTransferred = 0;
						ClearCommError(hSerialPort,&dwErr,NULL);
						U16550C_RxFIFOIn(value,dwErr);
					}
					else if (dwBytesTransferred)
							U16550C_RxFIFOIn(value,0);
				}
			}
		}
		else // read immediatelly
			if (dwBytesTransferred)
			{
				ullTicks = GetTickCount64() + 100;
				while (U16550C_RxFIFOFull()&&(ullTicks>GetTickCount64()))
						Sleep(0);
				U16550C_RxFIFOIn(value,0);
			}
	}

	CloseHandle(osReader.hEvent);

	return 0;
}

// This is the easiest way to indicate TX shift register is empty, when real UART is empty
// Also, this allow us to detect CTS, DSR, RING and DCD basically for free, no need to
// poll for changes...
// Finally, works for BREAK detection, not sure if that is detected by virtual UARTs, but
// real UARTs should work fine :)
DWORD U16550C_PortReadStatus (LPVOID lpvoid)
{
	DWORD dwCommEvent,dwOvRes;
	DWORD dwRes;
	DWORD dwStoredFlags = EV_BREAK | EV_CTS | EV_DSR | EV_RING | EV_RLSD | EV_TXEMPTY ;
	OVERLAPPED osStatus = {0};

	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osStatus.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	SetCommMask(hSerialPort, dwStoredFlags);

	while (hSerialPort != INVALID_HANDLE_VALUE) 
	{
		if (!WaitCommEvent(hSerialPort, &dwCommEvent, &osStatus))
		{
            if (GetLastError() == ERROR_IO_PENDING)
			{
				dwRes = WaitForSingleObject(osStatus.hEvent, INFINITE);
				if(dwRes==WAIT_OBJECT_0)
				{
					if (GetOverlappedResult(hSerialPort, &osStatus, &dwOvRes, FALSE))
						U16550C_FIFOStatusChange(dwCommEvent);
				}
			}

        }
        else
			 //WaitCommEvent returned immediately.
			// Deal with status event as appropriate.
			U16550C_FIFOStatusChange(dwCommEvent);
	}

	CloseHandle(osStatus.hEvent);

	return 0;
}

// Abstraction to send a single byte over serial port
// Need to do it overlapped, otherwise will lock until byte is sent, which will
// severely impact performance, specifically on USB->Serial Converters due to
// the latency
void U16550C_UartTransmit(BYTE value)
{
	DWORD dwNumBytesWritten;
	if(!WriteFile(hSerialPort, &value, 1, &dwNumBytesWritten, &os_Write)) {
		dwNumBytesWritten = GetLastError();
	}
}

// Change related Line Parameters, using the UART registers
// Will set the changes in the UART only if they are different OR if bForce is TRUE
// bForce is TRUE mostly at UART Creation. :)
BOOL U16550C_UartSetLCR(BOOL bForce)
{
unsigned int uiNParity = FALSE;
unsigned int uiNBits = 8;
unsigned int uiNParityScheme = NOPARITY;
unsigned int uiNStopBits = ONESTOPBIT;
DCB dcbConfig;
DWORD dwErr;
BOOL bRet = TRUE;

	switch	(state.LineControlRegister&0x03)
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

	if (state.LineControlRegister&0x04)
	{
		if (uiNBits == 5)
			uiNStopBits = ONE5STOPBITS;
		else
			uiNStopBits = ONESTOPBIT; // some ports do not accept two stop bits
			//uiNStopBits = TWOSTOPBITS;
	}
	else
		uiNStopBits = ONESTOPBIT;

	if (state.LineControlRegister&0x08)
	{
		uiNParity = TRUE;
		if (state.LineControlRegister&0x20) // stick?
		{
			if (state.LineControlRegister&0x10)
				uiNParityScheme = MARKPARITY;
			else
				uiNParityScheme = SPACEPARITY;
		}
		else
		{
			if (state.LineControlRegister&0x10)
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

	if ((uiNParity!=ui_Parity)||(uiNParityScheme!=ui_ParityScheme)||(uiNStopBits!=ui_StopBits)||(uiNBits!=ui_Bits)||(bForce))
	{
		ui_Parity=uiNParity;
		ui_ParityScheme=uiNParityScheme;
		ui_StopBits=uiNStopBits;
		ui_Bits=uiNBits;
		if ((state.hasCommPort)||(bForce))
		{
			dcbConfig.DCBlength = sizeof (DCB);
			if (!GetCommState(hSerialPort,&dcbConfig))
			{
				dwErr = GetLastError();
				bRet = FALSE;
			}
			else
			{
				dcbConfig.BaudRate = ul16550CBaudRate;
				dcbConfig.fBinary = TRUE;
				dcbConfig.fParity = ui_Parity;
				dcbConfig.fOutxCtsFlow = FALSE;
				dcbConfig.fOutxDsrFlow = FALSE;
				dcbConfig.fDtrControl = DTR_CONTROL_DISABLE;
				dcbConfig.fDsrSensitivity = FALSE;
				dcbConfig.fTXContinueOnXoff = FALSE;
				dcbConfig.fOutX = FALSE;
				dcbConfig.fInX = FALSE;
				dcbConfig.fErrorChar = FALSE;
				dcbConfig.fNull = FALSE;
				dcbConfig.fRtsControl = RTS_CONTROL_DISABLE;
				dcbConfig.fAbortOnError = FALSE;
				dcbConfig.ByteSize = ui_Bits;
				dcbConfig.Parity = ui_ParityScheme;
				dcbConfig.StopBits = ui_StopBits;
				// Configure the port according to the specifications of the DCB structure
				if (!SetCommState (hSerialPort, &dcbConfig)) {
					bRet = FALSE;
					dwErr = GetLastError();
				}
			}
		}
		else
			bRet = FALSE;
	}

	return bRet;
}

// This will allocate a UART on the host system for our virtual UART
BOOL U16550C_UartCreate(void)
{
    Properties* pProperties = propGetGlobalProperties();
	COMMTIMEOUTS commTimeouts;
    DWORD dwThreadID;
	char myPortName[600];

    // Alread initialized?
    if (state.hasCommPort)
        return TRUE;

	// A must for COM10 a beyond
	sprintf(myPortName,"\\\\.\\");
	strcat(myPortName,pProperties->ports.Com.portName);

    // Open the serial port
    hSerialPort = CreateFile(myPortName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (hSerialPort == INVALID_HANDLE_VALUE)
	{
		state.hasCommPort = FALSE;
		return FALSE;
	}

	// Create this write operation's OVERLAPPED structure's hEvent.
	os_Write.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (os_Write.hEvent == NULL) {
		CloseHandle(hSerialPort);
		hSerialPort = INVALID_HANDLE_VALUE;
		state.hasCommPort = FALSE;
		// error creating overlapped event handle
		return FALSE;
	}

	if (!U16550C_UartSetLCR(TRUE))
	{
		CloseHandle(os_Write.hEvent);
		os_Write.hEvent = NULL;
		CloseHandle(hSerialPort);
		hSerialPort = INVALID_HANDLE_VALUE;
		state.hasCommPort = FALSE;
        return FALSE;
	}
		
    SetupComm(hSerialPort, 16, 16);
		    
	// Change the COMMTIMEOUTS structure settings
	// Do not want time outs, so overlapped will signal only when data was sent or received
	commTimeouts.ReadIntervalTimeout = 0;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
    commTimeouts.ReadTotalTimeoutConstant = 0;
    commTimeouts.WriteTotalTimeoutMultiplier = 0;
    commTimeouts.WriteTotalTimeoutConstant = 0;

    // Set the time-out parameters for all read and write operations on the port
    if (!SetCommTimeouts(hSerialPort, &commTimeouts))
	{
		CloseHandle(os_Write.hEvent);
		os_Write.hEvent = NULL;
		CloseHandle(hSerialPort);
		hSerialPort = INVALID_HANDLE_VALUE;
		state.hasCommPort = FALSE;
        return FALSE;
	}

    // Create a read thread for reading data from the communication port
    hRxThread = CreateThread(NULL, 
                          0,
                          (LPTHREAD_START_ROUTINE) U16550C_PortReadThread,
                          0, 
                          0, 
                          &dwThreadID);

	if (hRxThread == NULL)
	{
		CloseHandle(os_Write.hEvent);
		os_Write.hEvent = NULL;
		CloseHandle(hSerialPort);
		hSerialPort = INVALID_HANDLE_VALUE;
		state.hasCommPort = FALSE;
		return FALSE;
	}
	
	// Create a read thread for reading status from the communication port
	hStsThread = CreateThread(NULL, 
                          0,
                          (LPTHREAD_START_ROUTINE) U16550C_PortReadStatus,
                          0, 
                          0, 
                          &dwThreadID);

	if (hStsThread == NULL)
	{
		WaitForSingleObject(hRxThread, INFINITE);
	    CloseHandle(hRxThread);
        hRxThread = NULL;
		CloseHandle(os_Write.hEvent);
		os_Write.hEvent = NULL;
		CloseHandle(hSerialPort);
		hSerialPort = INVALID_HANDLE_VALUE;
		state.hasCommPort = FALSE;
		return FALSE;
	}

    state.hasCommPort = TRUE;

    return TRUE;
}

// Release the resources used to control UART and Virtual UART
void U16550C_UartDestroy(void)
{
	DWORD dwError;
	if (hSerialPort!=INVALID_HANDLE_VALUE)
	{
		if (!CloseHandle (hSerialPort))
			dwError = GetLastError();
	}

	hSerialPort = INVALID_HANDLE_VALUE;

    if (hRxThread != NULL) {
        WaitForSingleObject(hRxThread, INFINITE);
	    CloseHandle(hRxThread);
        hRxThread = NULL;
    }

	if (hStsThread != NULL) {
		WaitForSingleObject(hStsThread, INFINITE);
	    CloseHandle(hStsThread);
        hStsThread = NULL;
    }

	if (os_Write.hEvent != NULL)
	{
		CloseHandle(os_Write.hEvent);
		os_Write.hEvent = NULL;
	}
   
	state.hasCommPort = FALSE;
}

// Functions that are used by blueMSX core

// This is our very beginning... And if the directuartio is set to 2, let's create our device
void U16550C_Create()
{
	Properties* pProperties = propGetGlobalProperties();
	DWORD dwThreadID;	

	if (pProperties->ports.Com.directuartio == 2)
	{
		
		// Reset our emulated uart
		if (U16550C_Reset())
		{
			ghTimerMutex = CreateMutex(NULL,           // default security attributes
									FALSE,             // initially not owned
									NULL);             // unnamed mutex
			if (ghTimerMutex == NULL) 
			{
				U16550C_UartDestroy();
				state.hasCommPort = FALSE;
				return;
			}
			ghTimerMutex2 = CreateMutex(NULL,           // default security attributes
									FALSE,             // initially not owned
									NULL);             // unnamed mutex
			if (ghTimerMutex2 == NULL) 
			{
				CloseHandle(ghTimerMutex);
				U16550C_UartDestroy();
				state.hasCommPort = FALSE;
				return;
			}
			// This mutex is for our TX Thread
			ghMutex = CreateMutex( 	NULL,              // default security attributes
									TRUE,              // initially owned
									NULL);             // unnamed mutex
			if (ghMutex == NULL) 
			{
				CloseHandle(ghTimerMutex);
				CloseHandle(ghTimerMutex2);
				U16550C_UartDestroy();
				state.hasCommPort = FALSE;
			}
			else
			{
				// Create a TX thread to handle FIFO TX Buffer
				hTXThread = CreateThread(NULL, 
								  0,
								  (LPTHREAD_START_ROUTINE) U16550C_TXThread,
								  0, 
								  0, 
								  &dwThreadID);

				if (hTXThread == NULL) {
					CloseHandle(ghMutex);
					CloseHandle(ghTimerMutex);
					CloseHandle(ghTimerMutex2);
					U16550C_UartDestroy();
					state.hasCommPort = FALSE;
				}
			}
		}
	}
	else
		state.hasCommPort = FALSE;

	// Register IO no matter what
    ioPortRegister(0x80, U16550C_ReadBaseAddress0, U16550C_WriteBaseAddress0,  (void*) &state);
    ioPortRegister(0x81, U16550C_ReadBaseAddress1, U16550C_WriteBaseAddress1,  (void*) &state);
	ioPortRegister(0x82, U16550C_ReadBaseAddress2, U16550C_WriteBaseAddress2,  (void*) &state);
	ioPortRegister(0x83, U16550C_ReadBaseAddress3, U16550C_WriteBaseAddress3,  (void*) &state);
	ioPortRegister(0x84, U16550C_ReadBaseAddress4, U16550C_WriteBaseAddress4,  (void*) &state);
	ioPortRegister(0x85, U16550C_ReadBaseAddress5, U16550C_WriteBaseAddress5,  (void*) &state);
	ioPortRegister(0x86, U16550C_ReadBaseAddress6, U16550C_WriteBaseAddress6,  (void*) &state);
	ioPortRegister(0x87, U16550C_ReadBaseAddress7, U16550C_WriteBaseAddress7,  (void*) &state);
}

// It is time to say good-bye
void U16550C_Destroy ()
{
	// Unregister IO ports
    ioPortUnregister(0x80);
    ioPortUnregister(0x81);
	ioPortUnregister(0x82);
	ioPortUnregister(0x83);
	ioPortUnregister(0x84);
	ioPortUnregister(0x85);
	ioPortUnregister(0x86);
	ioPortUnregister(0x87);

	// If TXThread exists, means we are working and have to de-allocate resources
	if (hTXThread != NULL) {
		// Stop running
		b16550CTXThreadRunning = FALSE;
		// Release mutex so if thread is waiting, it runs again
		ReleaseMutex(ghMutex);		
		// release UART
		U16550C_UartDestroy();
		// Wait thread to die before killing mutex
        WaitForSingleObject(hTXThread, INFINITE);
		// Release mutex as well
		if (ghMutex)
			CloseHandle(ghMutex);
		if (ghTimerMutex)
			CloseHandle(ghTimerMutex);
		if (ghTimerMutex2)
		CloseHandle(ghTimerMutex2);
		// Close thread resources
	    CloseHandle(hTXThread);
        hTXThread = NULL;		
    }	
}

// Address 0 reading: Receiver Buffer Register or if DLAB set in Line Control Register read Divisor Latch LSB
// Address 0 writing: Transmitter Holding Register or if DLAB set in Line Control Register set Divisor Latch LSB
// How do we work with it?
static UInt8 U16550C_ReadBaseAddress0(struct U16550C_STATE* state, UInt16 ioPort)
{
	if (state->LineControlRegister&0x80) //DLAB?
		return state->DivisorLatchLSB; //Just return it
	else
		return U16550C_RxFIFOOut(); //Otherwise let the FIFO handle what should be returned
}

static void U16550C_WriteBaseAddress0(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	if (state->LineControlRegister&0x80) //DLAB?
		state->DivisorLatchLSB = value; //Just write the new value of it
	else
		U16550C_TxFIFOIn(value); //Otherwise, put the data in the input fifo for transmission
}

// Address 1 reading: Interrupt Enable Register or if DLAB set in Line Control Register read Divisor Latch MSB
// Address 1 writing: Interrupt Enable Register or if DLAB set in Line Control Register set Divisor Latch MSB
static UInt8 U16550C_ReadBaseAddress1(struct U16550C_STATE* state, UInt16 ioPort)
{
	if (state->LineControlRegister&0x80) //DLAB?
		return state->DivisorLatchMSB; //Just return it
	else
		return state->InterruptEnableRegister; //Otherwise return the IER
}

static void U16550C_WriteBaseAddress1(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	if (state->LineControlRegister&0x80) //DLAB?
		state->DivisorLatchMSB = value; //Just write the new value of it
	else
	{
		state->InterruptEnableRegister = value&0x0f; //Otherwise, save the new IER value
		// Clear Any Interrupt that was disabled 
		if ((state->InterruptEnableRegister&1)==0)
		{
			state->RXDataInt = FALSE;
			U16550C_RXTimeOutCancel();
			state->RXDataToutInt = FALSE;
		}
		if ((state->InterruptEnableRegister&2)==0)
			state->TXEmptyInt = FALSE;
		if ((state->InterruptEnableRegister&4)==0)
			state->LSRInt = FALSE;
		if ((state->InterruptEnableRegister&8)==0)
			state->MSRInt = FALSE;
		// And let the IRQ Evaluation be done again
		U16550C_EvaluateIRQ();
	}
}

// Address 2 reading: Interrupt Identification Register
// Address 2 writing: FIFO Control Register
static UInt8 U16550C_ReadBaseAddress2(struct U16550C_STATE* state, UInt16 ioPort)
{
	// If it is TXEmpty being currently identified, that means the other higher
	// priority Ints being false
	if((!state->LSRInt)&&(!state->RXDataInt)&&(!state->RXDataToutInt))
	{
		// It is cleared upon reading IIR, page 28 of TI's data sheet
		state->TXEmptyInt = FALSE;
		// And let IRQ be re-evaluated if needed
		U16550C_EvaluateIRQ();
	}
	return state->InterruptIdentificationRegister;
}

static void U16550C_WriteBaseAddress2(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	UInt8 oldFCR = state->FIFOControlRegister; // save current FCR

	state->FIFOControlRegister = value & 0xCF; //make sure reserved bits are kept 0

	// If FIFO Enabled can evaluate
	if(state->FIFOControlRegister&0x01)
	{
		state->TXEmpty = TRUE;
		state->RXEmpty = TRUE;
		state->InterruptIdentificationRegister|=0xC0; //Sets bit 6 and 7 of IIR
		if((state->FIFOControlRegister&0x02)||((oldFCR&0x01)==0)) // Reset RX Fifo?
			U16550C_RxFIFOReset();
		if((state->FIFOControlRegister&0x04)||((oldFCR&0x01)==0)) // Reset TX Fifo?
			U16550C_TxFIFOReset();
	}
	else if((state->FIFOControlRegister&0x01)==0)
	{
		if (oldFCR&0x01) // was in FIFO mode?
		{
			state->TXEmpty = TRUE; //make sure this is correct
			state->RXEmpty = TRUE;
			state->LineStatusRegister&=0xFE; // Clear bit 0
			state->LineStatusRegister|=0x60; // Set bits 5 and 6
		}
		state->InterruptIdentificationRegister&=0x3F; //Clears bit 6 and 7 of IIR
	}
}

// Address 3: Line Control Register
// How do we work with it?
// Writting set the new communication parameters, and we set those and restart the port
// Reading will pretty much read what was written before (or existed since reset)
static void U16550C_WriteBaseAddress3(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	UInt8 oldLCR = state->LineControlRegister;
	// 3 is the Line Control Register
	state->LineControlRegister = value;
	if ((oldLCR&0x7f)!=(state->LineControlRegister&0x7f))
		U16550C_AdjustLineControlParameters();
	// Ok, clearing the DLAB? If so, set new speed
	if ((oldLCR&0x80)&&((state->LineControlRegister&0x80)==0))
		U16550C_SetUartSpeed();
	// Entering Break?
	if ((!oldLCR&0x40)&&(state->LineControlRegister&0x40))
		U16550C_SetBreak();
	// Exiting Break?
	if ((oldLCR&0x40)&&(!state->LineControlRegister&0x40))
		U16550C_ClearBreak();
	return;
}

static UInt8 U16550C_ReadBaseAddress3(struct U16550C_STATE* state, UInt16 ioPort)
{
	// 3 is the line control Register
	return state->LineControlRegister;
}

// Address 4: Modem Control Register
// How do we work with it?
// Modem control register starts with DTR and RTS and AFE disabled (and we ensure that at reset)
// When DTR or RTS is Enabled, bit will be automatically set
// Same when disabled
// AFE can be set (or cleared again) when writting to MCR, then the related functions will adapt properly
static void U16550C_WriteBaseAddress4(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	UInt8 oldLCR = state->ModemControlRegister;
	// 4 is the modem control register
	state->ModemControlRegister = value & 0x3F; // bits 6 and 7 are always zero
	if (oldLCR!=state->ModemControlRegister)
		U16550C_AdjustModemControlParameters(oldLCR);
	return;
}

static UInt8 U16550C_ReadBaseAddress4(struct U16550C_STATE* state, UInt16 ioPort)
{
	// 4 is the modem control Register
	return state->ModemControlRegister;
}

// Address 5: Line Status Register, this guy is mostly read only
// How do we work with it? 
// Data Ready is handled by U16550C_RxFIFOIn and U16550C_RxFIFOOut, clear when rx fifo is empty (or just 1 byte when fifo is off)
// Overrun Error is handled by U16550C_RxFIFOIn and U16550C_RxFIFOOut, clear when read
// Parity Error is handled by U16550C_RxFIFOIn and U16550C_RxFIFOOut, clear when read
// Framing Error is handled by U16550C_RxFIFOIn and U16550C_RxFIFOOut, clear when read
// Break Condition is handled by U16550C_RxFIFOIn and U16550C_RxFIFOOut, clear when read
// THRE indicator is checked on the fly
// TEMPT is handled by TX Fifo Thread, set when thread is idle, otherwise clear....
// Error In Rcv FIFO is handled by U16550C_RxFIFOIn and U16550C_RxFIFOOut, clear when read or if FIFO is off
static void U16550C_WriteBaseAddress5(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	// 5 is the Line Status Register, can't write to it
	return;
}

static UInt8 U16550C_ReadBaseAddress5(struct U16550C_STATE* state, UInt16 ioPort)
{
	state->LSRInt = FALSE;
	U16550C_EvaluateIRQ(); //Reset Line Status Int if needed

	// This checks whether FIFO is on, and if on, if there is any error on any byte stored in it
	if (!U16550C_CheckFIFOForLineErrors())
		state->LineStatusRegister &= 0x61; //Clear error bits and FIFO has error bit if there are no errors or not in FIFO mode
	else
	{
		state->LineStatusRegister &= 0xe1; //Clear error bits
		state->LineStatusRegister |= 0x80; //Set Error in RCVR FIFO
	}

	// Check THR on the fly...
	if (state->FIFOControlRegister&0x01) // FIFO Enabled?
	{
		// Yes, does it have data?
		if (state->FIFOTxHead==state->FIFOTxTail)
			state->LineStatusRegister |= 0x20; 
		else
			state->LineStatusRegister &= 0xDF; 
	}
	else
	{
		// No, has unsent byte?
		if (!state->TXEmpty) 
			// Yes
			state->LineStatusRegister |= 0x20; 
		else
			// No
			state->LineStatusRegister &= 0xDF; 
	}
	

	return state->LineStatusRegister;
}

// Address 6: Modem Status Register, this guy is mostly read only
// How do we work with it? ArchUart / Win32Uart has a thread that do overlapped operation
// waiting for status changes, so U16550C_FIFOStatusChange will deal with those changes, and
// U16550C_GetUartStatus get the initial values, so here we just return what is in memory
// and clear deltas after reading
static void U16550C_WriteBaseAddress6(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	// 6 is the modem status register, can't write to it
	return;
}

static UInt8 U16550C_ReadBaseAddress6(struct U16550C_STATE* state, UInt16 ioPort)
{
	UInt8 returnValue = state->ModemStatusRegister;

	state->MSRInt = FALSE;	
	// 6 is the modem status register
	b16550CHoldStatusRegister = TRUE;	

	state->ModemStatusRegister &= 0xf4; //Clear delta indication but leave TERI
	b16550CHoldStatusRegister = FALSE;
	U16550C_EvaluateIRQ(); //Reset Modem Status Int if needed
	return returnValue;
}

//Address 7: Scratch register, not much to do other than read or write value into it
static void U16550C_WriteBaseAddress7(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	// 7 is the scratch register, update it
	state->ScratchRegister = value;
}

static UInt8 U16550C_ReadBaseAddress7(struct U16550C_STATE* state, UInt16 ioPort)
{
	// 7 is the scratch register, return its current value
	return state->ScratchRegister;
}

// Internal use Functions

// Reset: basically start the FIFO as close as possible to what TI Datasheet specifies after a Reset
unsigned char U16550C_Reset()
{
	unsigned int uiDivisorLatch;

	uiDivisorLatch = 1;
	ul16550CBaudRate = UART_CLOCK_16X / (uiDivisorLatch*16); // Start at highest speed, whatever, not defined

	// No interrupt at all
	boardClearInt(INT_U16550C);	
	// Reset FIFOs
	U16550C_RxFIFOReset();
	U16550C_TxFIFOReset();
	// Page 23 of TI data sheet define most of the following
	state.InterruptEnableRegister = 0;
	state.InterruptIdentificationRegister = 1;
	state.FIFOControlRegister = 0;
	state.LineControlRegister = 0x03; //8N1, default should be 5N1 after a reset but some ports don't like it
	state.ModemControlRegister = 0;
	state.LineStatusRegister = 0x60;
	state.ModemStatusRegister = 0;
	// This is undefined, so I'm using 1
	state.DivisorLatchLSB = 1;
	state.DivisorLatchMSB = 0;
	// No int indication
	state.RXDataInt = FALSE;
	state.RXDataToutInt = FALSE;
	state.TXEmptyInt = FALSE;
	state.LSRInt = FALSE;
	state.MSRInt = FALSE;
	state.TXEmpty = TRUE;
	state.RXEmpty = TRUE;
	// We need UART to notify Received Bytes and Comm Status of those bytes, need to receive Modem Status Change as well
	if (U16550C_UartCreate())
	{
		U16550C_AdjustLineControlParameters();
		U16550C_GetUartStatus();
		// No Break Condition
		U16550C_ClearBreak();
		// DTR and RTS are High, meaning cleared
		U16550C_ClearDTR();
		U16550C_ClearRTS();
	}
	return state.hasCommPort;
}

// Modem Control Functions
void U16550C_AdjustModemControlParameters(UInt8 oldParam)
{
	// DTR Always handled
	state.ModemControlRegister&1 ? U16550C_SetDTR() : U16550C_ClearDTR();
	// RTS is handled here only if AFE RTS is disabled
	if ((state.ModemControlRegister&0x22)!=0x22)
		(state.ModemControlRegister&2) ? U16550C_SetRTS() : U16550C_ClearRTS();
	// Auto Flow changing?
	else if ((oldParam&0x22)!=(state.ModemControlRegister&0x22))
	{
		if ((state.ModemControlRegister&0x22)==0x22) // Enabling Auto Flow RTS
		{
			//Ok, enable or disable RTS according to FIFO status
			U16550C_RxFIFOThresholdHit();
		}
		else // Disabling Auto Flow, no need to do anything, RTS set before
		{
			//state.ModemControlRegister&2 ? U16550C_SetRTS() : U16550C_ClearRTS();
		}
	}
}

// Just used at UART emulation start up, to get the initial values
// All other changes are received in a callback
void U16550C_GetUartStatus()
{
	DWORD dwSts;
	if (state.hasCommPort)
	{
		GetCommModemStatus(hSerialPort,&dwSts);
		
		if (dwSts&MS_CTS_ON)
			state.ModemStatusRegister|=0x10; // Set bit 4
		else
			state.ModemStatusRegister&=0xEF; // Clear bit 4

		if (dwSts&MS_DSR_ON)
			state.ModemStatusRegister|=0x20; // Set bit 5
		else
			state.ModemStatusRegister&=0xDF; // Clear bit 5

		if (dwSts&MS_RING_ON)
			state.ModemStatusRegister|=0x40; // Set bit 6
		else
			state.ModemStatusRegister&=0xBF; // Clear bit 6

		if (dwSts&MS_RLSD_ON)
			state.ModemStatusRegister|=0x80; // Set bit 7
		else
			state.ModemStatusRegister&=0x7F; // Clear bit 7
	}
}

// Clear Break Condition on real UART
void U16550C_ClearBreak()
{
	if (state.hasCommPort)
		EscapeCommFunction(hSerialPort,CLRBREAK);
}

// Set Break Condition on real UART
void U16550C_SetBreak()
{
	if (state.hasCommPort)
		EscapeCommFunction(hSerialPort,SETBREAK);
}

// Clear DTR on real UART
void U16550C_ClearDTR()
{
	EscapeCommFunction(hSerialPort,CLRDTR);
}

// Set DTR on real UART
void U16550C_SetDTR()
{
	EscapeCommFunction(hSerialPort,SETDTR);
}

// Clear RTS on real UART
void U16550C_ClearRTS()
{	
	EscapeCommFunction(hSerialPort,CLRRTS);
}

// Set RTS on real UART
void U16550C_SetRTS()
{
	EscapeCommFunction(hSerialPort,SETRTS);
}

// RX Specific Functions

// Clear FIFO data and restore pointers
void U16550C_RxFIFOReset()
{
	state.FIFORxHead = 0;
	state.FIFORxTail = 0;
	memset(state.FIFORxBuffer,0,U16550C_FIFO_Depth);
}

// There are two slightly different behaviors:
// 1 - FIFO is enabled: will check for FIFO being full while writting, it is, mark OE
// 2 - FIFO is disabled: will check if the last byte received was read, if not, mark OE
//
// Common behavior:
// Set Data Ready, first byte of of Line Status Register
// Write the data to next available position, if using FIFO, or the last FIFO position if FIFO full. If not FIFO, write to the first FIFO position, always
// In case FIFO is on, increase the FIFORxLevel indicator (used by IRQ Evaluation function)
void U16550C_RxFIFOIn(UInt8 value, DWORD error)
{	
	U16550C_RXTimeOutCancel();
	state.RXDataToutInt = FALSE; // Data entering resets Time Out Interrupt
	state.LineStatusRegister|=0x1; // Set bit 1, not empty anymore, faster than checking if need to set, just set
	if (state.FIFOControlRegister&0x01) // Fifo Enabled?
	{		
		state.FIFORxBuffer[state.FIFORxHead] = value; //write value
		// Overrun?
		if(U16550C_RxFIFOFull())
		{
			// Yep, buffer full
			state.FIFORxError[state.FIFORxHead] = error|CE_OVERRUN; // Mark OE, and write the rest of the error information for it
			state.LineStatusRegister|=0x02; // Set Bit 2, OE
		}		
		else
		{
			// No Overrun			
			state.FIFORxError[state.FIFORxHead] = error; //and the error information for it
			state.FIFORxHead = (state.FIFORxHead+1) % U16550C_FIFO_Depth; // advance head, leap if needed
		}		
	}
	else // FIFO not enabled
	{
		if (!state.RXEmpty) //Overrun?
		{
			state.LineStatusRegister|=0x02; // Set Bit 2, OE
			state.FIFORxError[0] = error|CE_OVERRUN; // Mark OE
		}
		else
			state.RXEmpty = FALSE; //since we've already checked it, set it just here
		state.FIFORxBuffer[0] = value; // write or overwrite it				
		
	}

	U16550C_RxFIFOThresholdHit(); // will check if need to disable RTS or not
	U16550C_EvaluateIRQ(); // and will check IRQ and priorities
	U16550C_RXTimeOutSchedule(); // and check if need to schedule a RX timeout for interrupt
}

// There are two different behaviors:
// 1 - FIFO is enabled: 
//		Will clear RXData Time Out Interrup
//		Check if FIFO has data, and if it has:
//			Return value is taken from fifo, that will also make adjustments to line error status
//			Adjust fifo RxFifoLevel, Head and Tail pointers
//		If FIFO ends-up empty, clear Data Ready in Line Status Register
//		Call RXTimeOutSchedule, that will check if a timeout check need to be scheduled or cancelled
// 2 - FIFO is disabled:
//		Return Value is always the first byte of FIFO
//		Error Line status also is taken from there
//		Empty indicator is set to signal data was already READ
//		Will check if the last byte received was read, if not, mark OE
//		Clear Data Ready in Line Status Register
//
// Common behavior:
// Check if RTS was held and need to be released
// Call the function that Evaluates IRQ status and notifications
UInt8 U16550C_RxFIFOOut()
{
	UInt8 value = 0xff; //If no data in Rx FIFO, returns 0xff	

	U16550C_RXTimeOutCancel();
	state.RXDataToutInt = FALSE; // Reading data resets Time Out Interrupt		
	if (state.FIFOControlRegister&0x01) // Fifo Enabled?
	{
		// Yes, we are working with a FIFO		
		if (state.FIFORxTail != state.FIFORxHead) // Data available in FIFO?
		{
			value = state.FIFORxBuffer[state.FIFORxTail]; // Copy value
			U16550C_FIFOStatusUpdate(state.FIFORxError[state.FIFORxTail]); // update line status with byte status
			state.FIFORxTail = (state.FIFORxTail + 1) % U16550C_FIFO_Depth; // move Tail
		}
		if (state.FIFORxTail==state.FIFORxHead) //Empty?
			state.LineStatusRegister&=0xFE; // Clear Data Ready, bit 1 of Line Status		
	}
	else
	{
		// Nope, FIFO Disabled
		value = state.FIFORxBuffer[0]; // Just copy the value
		U16550C_FIFOStatusUpdate(state.FIFORxError[0]); // and update line status with that byte status
		state.RXEmpty = TRUE; // Signal empty
		state.LineStatusRegister&=0xFE; // Clear Data Ready, bit 1 of Line Status
	}
	U16550C_RxFIFOThresholdHit(); // This will set/reset RTS as needed
	U16550C_EvaluateIRQ(); // Evaluate if IRQ status and registers need update
	U16550C_RXTimeOutSchedule(); // and adjust timeout timers accordingly
	return value; // And return byte
}

// Check if Head is just behind tail, meaning full
unsigned char U16550C_RxFIFOFull()
{
	if (state.FIFOControlRegister&0x01)
	{
		//A 100 byte fifo will have pointers moving up to 99, and then, 0 (that is why calculate the remainder)
		//This fifo "wastes" 1 byte, so it is not allowed to have Head = Tail unless it is empty
		if(!(state.FIFORxHead == (( state.FIFORxTail - 1 + U16550C_FIFO_Depth) % U16550C_FIFO_Depth)))
			return FALSE;
		else
			return TRUE;
	}
	else
		return !state.RXEmpty;
}

// Ok, that is a tricky one
// TI Documentation about RX FifoThreshold enabling an interrupt says in pages 24-25:
//	-	The received data available interrupt is issued to the microprocessor when the FIFO has reached its
//		programmed trigger level. It is cleared when the FIFO drops below its programmed trigger level.
//	-	The IIR receive data available indication also occurs when the FIFO trigger level is reached, and like the
//		interrupt, it is cleared when the FIFO drops below the trigger level.
// And levels are described as:
//		00 - 1
//		01 - 4
//		10 - 8
//		11 - 14 bytes
//
// On the other hand, for Auto Flow the behavior (God knows why) for threshold is different:
// Pages 3 - 4:
// Figure 3 tells us that for level set to 1/4/8 the behavior is:
// -	Once threshold bytes are stored in FIFO, RTS is disabled
// -	There is a chance that an extra byte is sent (so 2, 5 or 9) and stored
// -	RTS is enabled only when FIFO is empty!!!
// Figure 4 tells us that for level set to 14, a different scenario occurs:
// -	Once the 16th byte start bit is received, RTS is disabled (so we have 16 bytes stored in FIFO)
// -	As soon as just one byte is read, it will enable RTS again
//
// Love those designs that have different meanings and behaviors on the same set of functionality... :P
//
// So this function will handle just AUTO FLOW behavior of thresholds:
void U16550C_RxFIFOThresholdHit()
{
	unsigned char rtsClear = FALSE;	// by default, RTS enabled
	unsigned char TriggerLevel = ((state.FIFOControlRegister>>6)&0x03); // easier to switch
	unsigned char bAFE_RTS = ((state.ModemControlRegister&0x22)==0x22); // Auto Flow Enabled for RTS?

	// Id no RTS AutoFlow, just return, nothing to see here
	if (!bAFE_RTS)
		return;

	// Fifo Enabled?
	if(state.FIFOControlRegister&0x01)
	{
		// Are we holding RTS until empty?
		if (b16550CHoldRTSUntilEmpty)
		{
			if (state.FIFORxTail!=state.FIFORxHead) //if still has data and is holding
				return; // Done
			else
			{
				//Can re-enable it
				b16550CHoldRTSUntilEmpty = FALSE; // Clear hold flag
				U16550C_SetRTS(); // Set RTS to allow for more data to come in
				return; // Done
			}
		}
		// If not holding RTS, let's evaluate if trigger occurred
		else if (!b16550CHoldRTSUntilEmpty)
		{
			if (state.FIFORxHead>=state.FIFORxTail)
				state.FIFORxLevel=state.FIFORxHead-state.FIFORxTail;
			else
				state.FIFORxLevel=(U16550C_FIFO_Depth - 1 - state.FIFORxTail) + state.FIFORxHead;
			switch (TriggerLevel)
			{
				case 0:
					if (state.FIFORxLevel>=1)
					{
						rtsClear = TRUE;	// RTS Disabled
						b16550CHoldRTSUntilEmpty = TRUE; // Trigger occurred and will hold RTS disabled until fifo is empty
					}
					break;
				case 1:
					if (state.FIFORxLevel>=4)
					{
						rtsClear = TRUE;	// RTS Disabled
						b16550CHoldRTSUntilEmpty = TRUE; // Trigger occurred and will hold RTS disabled until fifo is empty
					}
					break;
				case 2:
					if (state.FIFORxLevel>=8)
					{
						rtsClear = TRUE;	// RTS Disabled
						b16550CHoldRTSUntilEmpty = TRUE; // Trigger occurred and will hold RTS disabled until fifo is empty
					}
					break;
				case 3:
					if (state.FIFORxLevel>=16)
					{
						b16550CHoldRTSUntilEmpty = FALSE; // will not hold
						rtsClear =  TRUE;  // Trigger occurred 
					}
					break;
			}
		}
	}
	else // No FIFO, autoflow?
	{
		// No FIFO so, if no unread byte, RTS enabled, return false
		// If there is an unread byte, RTS disabled, return true
		rtsClear = !state.RXEmpty;
		b16550CHoldRTSUntilEmpty = FALSE;
	}

	if (rtsClear)
		U16550C_ClearRTS();	// If yes, clear RTS to stop the other end from sending data
	else
		U16550C_SetRTS(); // Otherwise set RTS to allow for more data to come in
}

// Line Status Register bit 7 needs to be set when read if at least one unread FIFO byte has an error
UInt8 U16550C_CheckFIFOForLineErrors()
{
	unsigned int uiI;
	unsigned char ucRet = FALSE;

	// If FIFO is in use and not Empty
	if ((state.FIFOControlRegister&0x01)&&(state.FIFORxHead!=state.FIFORxTail))
	{
		// FIFO is in use, need to check the Rx FIFO for errors, if any error found, return true
		for (uiI=(state.FIFORxTail+1); uiI!=state.FIFORxHead;uiI = (uiI + 1) % U16550C_FIFO_Depth)
		{
			// If any error found
			if (state.FIFORxError[uiI]!=0)
			{
				// no need to look further
				ucRet = TRUE;
				break;
			}
		}
		// if Head = Tail, need to check that as well, last FIFO byte
		// of course, as long as ucRet is not true...
		if ((uiI==state.FIFORxHead)&&(!ucRet))
			if (state.FIFORxError[uiI]!=0)
				ucRet = TRUE;

		// clear so it is no longer read, page 30 of TI Datasheet:
		//	LSR7 is set when there is at least one parity, framing, or break error in the FIFO. It is
		//	cleared when the microprocessor reads the LSR and there are no subsequent errors in the FIFO.
		// My understanding is that the error for the current byte in FIFO is no longer relevant for that
		// so just clear it to not be evaluated again
		state.FIFORxError[state.FIFORxTail] = 0;
	}

	return ucRet;
}

// This callback is called whenever a programmed RX TimeOut occurs
void CALLBACK U16550C_RXTimeOutCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	// Will evaluate this time-out only if data receiving interrupt and FIFO are enabled
	if ((state.InterruptEnableRegister&1)&&(state.FIFOControlRegister&0x01)&&(state.FIFORxTail!=state.FIFORxHead)&&(state.RXDataInt==0))
		// Still has data in FIFO, so flag this interruption
		state.RXDataToutInt = TRUE;
	else
		// Not needed, make sure it is disabled
		state.RXDataToutInt = FALSE;

	// And let the IRQ evaluation function handle the rest
	U16550C_EvaluateIRQ();
	timerId = 0; // clear variable
}

// This function will cancel a RX Timeout Timer, if needed
void U16550C_RXTimeOutCancel()
{
	WaitForSingleObject(ghTimerMutex,INFINITE);
	// If there is an existing timer
	if (timerId)
	{
		timeKillEvent(timerId);			
		timerId = 0; // clear variable
	}
	ReleaseMutex(ghTimerMutex);
}
// This function will schedule a RX Timeout Timer, if needed
void U16550C_RXTimeOutSchedule()
{
	int iCalculatedTimer;	

	WaitForSingleObject(ghTimerMutex2,INFINITE);
	U16550C_RXTimeOutCancel();
	// Will evaluate if needed to schedule only if data receiving interrupt and FIFO are enabled
	if ((state.FIFOControlRegister&0x01)&&(state.InterruptEnableRegister&0x01)&&(state.FIFORxTail!=state.FIFORxHead))
	{
		// Find out the equivalent to 12 * Baud Clock  * 5 characters 
		iCalculatedTimer = 12*1000*5/(ul16550CBaudRate);
		// Yeah, that pretty much sucks, but for speeds higher than 60000bps we are not able to have intercharacter timeout as fast as it should
		// Option would be to hammer high performance counters, but that would increase CPU usage a lot, so, if there are a lot of small packets
		// the performance of emulated 16550C will be lower than the real one if whoever is using it base it on interrupts
		if (!iCalculatedTimer)
			iCalculatedTimer = 1;		
		timerId = timeSetEvent(iCalculatedTimer, iCalculatedTimer, U16550C_RXTimeOutCallback, 0, TIME_ONESHOT|TIME_CALLBACK_FUNCTION|TIME_KILL_SYNCHRONOUS);		
	}
	else
	{
		// Not needed, make sure it is disabled
		state.RXDataToutInt = FALSE;
		state.RXDataToutInt = FALSE;
		// And let the IRQ evaluation function handle the rest
		U16550C_EvaluateIRQ();
	}
	ReleaseMutex(ghTimerMutex2);
}

// TX Specific Functions

// Clear FIFO data and restore pointers
void U16550C_TxFIFOReset()
{	
	state.FIFOTxHead = 0;
	state.FIFOTxTail = 0;
	memset(state.FIFOTxBuffer,0,U16550C_FIFO_Depth);
}

// Two possible scenarios:
//
// 1 - FIFO is Enabled: push data in TxFIFO if it is not full, and adjust Head pointer
// 2 - FIFO is disabled: push data in first byte of TxFIFO and flag as unsent
// 
// Common Behavior: signal TXThread to start moving FIFO Data if it is idle
void U16550C_TxFIFOIn(UInt8 value)
{
	state.TXEmptyInt = FALSE; // This interrupt should no longer be enabled as this is not empty now... :)
	U16550C_EvaluateIRQ(); // Evaluate IRQ condtions again
	if (state.FIFOControlRegister&0x01) // Fifo Enabled?
	{
		// TI Doesn't document what happens when TxFIFO is Full, I'm discarding it
		// Perhaps it overwrites the last entry on the FIFO? Not sure
		if(!U16550C_TxFIFOFull())
		{
			// Update value
			state.FIFOTxBuffer[state.FIFOTxHead] = value;
			// And move FIFO Head
			state.FIFOTxHead = (state.FIFOTxHead+1) % U16550C_FIFO_Depth;
		}
	}
	else
	{
		// TI Doesn't document what happens when Tx is still loading the shift register
		// I'm overwriting just because it is easier on code, perhaps it discards? (most likely?) Not sure
		state.FIFOTxBuffer[0] = value;
		state.TXEmpty = FALSE;
	}
	// And if TxThread is idle, begin working :)
	U16550C_SignalTXThreadToMove();
}

// TX Thread is the only one using this to pop-out data
UInt8 U16550C_TxFIFOOut()
{
	UInt8 value = 0xff; //If no data in Tx FIFO, returns 0xff	

	if (state.FIFOControlRegister&0x01) // Fifo Enabled?
	{
		// Yes, there is date in it?
		if (state.FIFOTxTail != state.FIFOTxHead)
		{
			// Yes, get it and adjust Tail
			value = state.FIFOTxBuffer[state.FIFOTxTail];
			state.FIFOTxTail = (state.FIFOTxTail + 1) % U16550C_FIFO_Depth;
		}
	}
	else
	{
		// No, just return date in first buffer position
		value = state.FIFOTxBuffer[0];
		// And it is empty now :)
		state.TXEmpty = TRUE;
	}

	return value;
}

unsigned char U16550C_TxFIFOFull()
{
	//A 100 byte fifo will have pointers moving up to 99, and then, 0 (that is why calculate the remainder)
	//This fifo "wastes" 1 byte, so it is not allowed to have Head = Tail unless it is empty
	if(!(state.FIFOTxHead == (( state.FIFOTxTail - 1 + U16550C_FIFO_Depth) % U16550C_FIFO_Depth)))
		return FALSE;
	else
		return TRUE;
}

// Send a byte through the real UART
static void U16550C_WriteUARTBuffer(UInt8 value)
{
	if ((state.hasCommPort))	
		U16550C_UartTransmit(value);
}

// If Thread is Idle and there is data to send, Wake-Up thread
void U16550C_SignalTXThreadToMove()
{
	if (!b16550CTXThreadWorking)
	{
		// Fifo Enabled and TX FiFO Not Empty?
		if ( ((state.FIFOControlRegister&0x01)&&(state.FIFOTxTail != state.FIFOTxHead))||\
		// Or Fifo Disabled and Unsent Byte?
			 (((state.FIFOControlRegister&0x01)==0)&&(!state.TXEmpty)) )
		{
			// If thread is idle, mutex is ours, release it so thread will wake up
			ReleaseMutex(ghMutex);
			// Wait thread release the Mutex before working
			while(!b16550CTXThreadGotMutex);
			// And now get the Mutext again to make sure thread won't work again
			WaitForSingleObject(ghMutex,INFINITE);
			// And this finally release thread to work, this avoid false trigger of thread
			// If somehow we are slow to take ownership of Mutex
			b16550CTXThreadGotMutex = FALSE;
		}
	}
}

// If we have that to send, send it, otherwise, do nothing
static DWORD U16550C_TXThread(LPVOID lpvoid)
{	
	DWORD dwWaitResult; 

	// This allow an external proccess to stop thread
	b16550CTXThreadRunning = TRUE;

	// While no one tells us to stop
	while (b16550CTXThreadRunning)
	{
		// Wait Our Mutex
		dwWaitResult = WaitForSingleObject(ghMutex, INFINITE);
		if ((dwWaitResult==WAIT_OBJECT_0)&&(b16550CTXThreadRunning))
		{
			b16550CTXThreadWorking = TRUE; // Thread is or is going to move data
			// If Mutex was released  and no one asked us to keep running
			ReleaseMutex(ghMutex); //release it
			b16550CTXThreadGotMutex = TRUE; // Indicate it is ours
			while (b16550CTXThreadGotMutex); // wait mutex to be held again by main thread, avoid false triggering
			if (state.hasCommPort)
			{
				state.LineStatusRegister&=0x9F; // Clear bit 6, not empty, moving data
				if (state.ModemControlRegister&0x20) // Auto Flow ?
					// If yes, hold here until CTS is set or someone ask the thread to stop
					while ((!(state.ModemStatusRegister&0x10))&&(b16550CTXThreadRunning));
				// If thread still running...
				if (b16550CTXThreadRunning)
				{
					// Ok, data to send, using FIFO?
					if (state.FIFOControlRegister&0x01)
						// Yes, so while TX Fifo is not Empty (Head != Tail)
						while (state.FIFOTxHead!=state.FIFOTxTail)
							// Get the byte out of FIFO and send it
							U16550C_WriteUARTBuffer(U16550C_TxFIFOOut());
					else
						// No fifo, so just send the position 0 of TxFifo
						U16550C_WriteUARTBuffer(U16550C_TxFIFOOut());
				}
			}
		}
		state.LineStatusRegister|=0x40; // Set bit 6, empty, not moving data
		b16550CTXThreadWorking = FALSE; // No longer working			
	}

	return 0;
}

//Internal UART Functionality

// Ok, that is a tricky one
// TI Documentation about RX FifoThreshold enabling an interrupt says in pages 24-25:
//	-	The received data available interrupt is issued to the microprocessor when the FIFO has reached its
//		programmed trigger level. It is cleared when the FIFO drops below its programmed trigger level.
//	-	The IIR receive data available indication also occurs when the FIFO trigger level is reached, and like the
//		interrupt, it is cleared when the FIFO drops below the trigger level.
// And levels are described as:
//		00 - 1
//		01 - 4
//		10 - 8
//		11 - 14 bytes
//
// On the other hand, for Auto Flow the behavior (God knows why) for threshold is different:
// Pages 3 - 4:
// Figure 3 tells us that for level set to 1/4/8 the behavior is:
// -	Once threshold bytes are stored in FIFO, RTS is disabled
// -	There is a chance that an extra byte is sent (so 2, 5 or 9) and stored
// -	RTS is enabled only when FIFO is empty!!!
// Figure 4 tells us that for level set to 14, a different scenario occurs:
// -	Once the 16th byte start bit is received, RTS is disabled (so we have 16 bytes stored in FIFO)
// -	As soon as just one byte is read, it will enable RTS again
//
// Love those designs that have different meanings and behaviors on the same set of functionality... :P
//
// So this function will handle RX thresolds only related to IRQ
// And also handle the other interrupt behaviors
void U16550C_EvaluateIRQ()
{
	unsigned char TriggerLevel = ((state.FIFOControlRegister>>6)&0x03); // Easier to switch 

	// Interrupts Enabled for received data?
	if (state.InterruptEnableRegister&1)
	{
		//Yes, but is FIFO enabled?
		if(state.FIFOControlRegister&0x01)
		{
			if (state.FIFORxHead>=state.FIFORxTail)
				state.FIFORxLevel=state.FIFORxHead-state.FIFORxTail;
			else
				state.FIFORxLevel=(U16550C_FIFO_Depth - 1 - state.FIFORxTail) + state.FIFORxHead;
			// Yes
			switch (TriggerLevel)
			{
				case 0:
					if (state.FIFORxLevel>=1)
					{
						state.RXDataInt = TRUE;
					}
					else
						state.RXDataInt = FALSE;
					break;
				case 1:
					if (state.FIFORxLevel>=4)
					{
						state.RXDataInt = TRUE;
					}
					else
						state.RXDataInt = FALSE;
					break;
				case 2:
					if (state.FIFORxLevel>=8)
					{
						state.RXDataInt = TRUE;
					}
					else
						state.RXDataInt = FALSE;
					break;
				case 3:
					if (state.FIFORxLevel>=16)
					{
						state.RXDataInt = TRUE;
					}
					else
						state.RXDataInt = FALSE;
					break;
			}
		}
		else
		{
			// No, so interrupt is active when there is an unread byte
			state.RXDataInt = !state.RXEmpty;
		}
	}
	else
		// No, so this is not generating an interrupt
		state.RXDataInt = FALSE;

	// Interrupts Enabled for Transmitter Buffer?
	if (state.InterruptEnableRegister&2)
	{
		// Yes, but FIFO is enabled?
		if (state.FIFOControlRegister&0x01)
		{
			// Yes, is TxFIFO empty?
			if (state.FIFOTxHead!=state.FIFOTxTail)
				state.TXEmptyInt = FALSE; // No, so no interrupt
			else
				state.TXEmptyInt = TRUE; // Yes, so interrupt
		}
		else
		{
			// If there is an untransmitted byte, do not interrupt, if there is not, interrupt
			state.TXEmptyInt = state.TXEmpty;
		}
	}
	else
		// No, so interrupt is not active
		state.TXEmptyInt = FALSE;
	
	// Line Status interrupt is evaluated in FIFOStatusChange and FIFOStatusUpdate functions
	// Just force off if Line Status interrupt is disabled
	if ((state.InterruptEnableRegister&4)==0)
		state.LSRInt = FALSE;

	// Modem Status interrupt is evaluated in FIFOStatusChange function that receive updates to those inputs
	// Just force off if Modem Status interrupt is disabled
	if ((state.InterruptEnableRegister&8)==0)
		state.MSRInt = FALSE;

	// Parse through priority of IRQs, just one is show in IIR
	// Line Status is the highest priority if it occurred
	if (state.LSRInt)
	{
		state.InterruptIdentificationRegister = (state.InterruptIdentificationRegister&0xf0)|0x06;
		boardSetInt(INT_U16550C);
	}
	// Otherwise, then RX Buffer Threshold Hit
	else if (state.RXDataInt)
	{
		state.InterruptIdentificationRegister = (state.InterruptIdentificationRegister&0xf0)|0x04;
		boardSetInt(INT_U16550C);
		state.RXDataToutInt = 0;
	}
	// Otherwise, then RX Buffer TimeOut
	else if (state.RXDataToutInt)
	{
		state.InterruptIdentificationRegister = (state.InterruptIdentificationRegister&0xf0)|0x0c;
		boardSetInt(INT_U16550C);
	}
	// Otherwise, then TX Buffer Empty
	else if (state.TXEmptyInt)
	{
		state.InterruptIdentificationRegister = (state.InterruptIdentificationRegister&0xf0)|0x02;
		boardSetInt(INT_U16550C);
	}
	// Finally, last check then, Modem Status
	else if (state.MSRInt)
	{
		state.InterruptIdentificationRegister = (state.InterruptIdentificationRegister&0xf0);
		boardSetInt(INT_U16550C);
	}
	// Nope, no interrupt at all
	else
	{
		// Clear IIR
		state.InterruptIdentificationRegister = (state.InterruptIdentificationRegister&0xf0)|0x01;
		// Clear Interrupt signal
		boardClearInt(INT_U16550C);
	}
}

// This is a function that adjust UART status according to the status of each received byte
void U16550C_FIFOStatusChange(DWORD value)
{
	UInt8 oldMSR = state.ModemStatusRegister;
	UInt8 oldLSR = state.LineStatusRegister;
	// Receive the result of WaitCommEvent, we are monitoring all status not RX related
	// Rx related we deal in the RX Thread

	if (value&EV_BREAK) //BREAK Occurred
		state.LineStatusRegister|=0x10; // Set bit 4
	else
		state.LineStatusRegister&=0xEF; // Clear bit 4

	if (value&EV_TXEMPTY) //TXBuffer Empty
		if (state.LineStatusRegister&0x20); // if our internal fifo is empty
			state.LineStatusRegister |= 0x40; //TRANSMITTER now empty...

	if (value&EV_CTS) //CTS changed state
	{
		if (oldMSR&0x10) // was it set?
		{
			//will need to reset and CTS set delta CTS, so set bit 0 and clear bit 4
			state.ModemStatusRegister&=0xEF; // Clear bit 4
			state.ModemStatusRegister|=0x01; // Set bit 0
		}
		else //it was reset
		{
			//will need to set CTS and set delta CTS, so set bit 0 and clear bit 4
			state.ModemStatusRegister|=0x10; // Set bit 4
			state.ModemStatusRegister|=0x01; // Set bit 0
		}
	}

	if (value&EV_DSR) //DSR changed state
	{
		if (oldMSR&0x20) // was it set?
		{
			//will need to reset DSR and set delta DSR, so set bit 1 and clear bit 5
			state.ModemStatusRegister&=0xDF; // Clear bit 5
			state.ModemStatusRegister|=0x02; // Set bit 1
		}
		else //it was reset
		{
			//will need to set DSR and set delta DSR, so set bit 1 and clear bit 5
			state.ModemStatusRegister|=0x20; // Set bit 5
			state.ModemStatusRegister|=0x02; // Set bit 1
		}
	}

	if (value&EV_RLSD) //DCD?
	{
		if (oldMSR&0x80) // was it set?
		{
			//will need to reset DCD and set delta DCD, so set bit 3 and clear bit 7
			state.ModemStatusRegister&=0x7F; // Clear bit 7
			state.ModemStatusRegister|=0x08; // Set bit 3
		}
		else //it was reset
		{
			//will need to set DCD and set delta DCD, so set bit 1 and clear bit 5
			state.ModemStatusRegister|=0x80; // Set bit 7
			state.ModemStatusRegister|=0x08; // Set bit 3
		}
	}

	if (value&EV_RING) //RING detected
	{
		//will set RING and clear TERI, so set bit 6 and clear bit 2
		state.ModemStatusRegister|=0x40; // Set bit 6
		state.ModemStatusRegister&=0xFB; // Clear bit 2
	}
	else
	{
		state.ModemStatusRegister&=0xBF; // Clear bit 6
		if (oldMSR&0x40) // was it set?
			//yes, set the TERI bit
			state.ModemStatusRegister|=0x04; // Set bit 2
	}

	if (((oldLSR&0x1e)^(state.LineStatusRegister&0x1e))&&(state.InterruptEnableRegister&0x04))
		state.LSRInt = TRUE;

	if ((state.ModemStatusRegister&0x0f)&&(state.InterruptEnableRegister&0x08))
	{
		state.MSRInt = TRUE;
		U16550C_EvaluateIRQ();
	}
	else
	{
		state.MSRInt = FALSE;
		U16550C_EvaluateIRQ();
	}
}

// This is a callback that receive the real UART status changes and adjust the emulated UART statuses
void U16550C_FIFOStatusUpdate(DWORD value)
{
	UInt8 oldLSR = state.LineStatusRegister;
	// Receive the result of WaitCommEvent, we are monitoring all status not RX related
	// Rx related we deal in the RX Thread

	if (value&CE_BREAK) //BREAK Occurred
		state.LineStatusRegister|=0x10; // Set bit 4
	else
		state.LineStatusRegister&=0xEF; // Clear bit 4

	if (value&CE_FRAME) //Framing error occurred
		state.LineStatusRegister|=0x08; // Set bit 3
	else
		state.LineStatusRegister&=0xF7; // Clear bit 3

	if ((value&CE_OVERRUN)||(value&CE_RXOVER)) //Overrun
		state.LineStatusRegister|=0x02; // Set bit 1
	else
		state.LineStatusRegister&=0xFD; // Clear bit 1

	if (value&CE_RXPARITY) //Parity error occurred
		state.LineStatusRegister|=0x04; // Set bit 2
	else
		state.LineStatusRegister&=0xFB; // Clear bit 2

	if ((oldLSR&0x1e)^(state.LineStatusRegister&0x1e))
	{
		state.LSRInt = TRUE;
		U16550C_EvaluateIRQ();
	}
}

void U16550C_AdjustLineControlParameters()
{
	if (state.hasCommPort)
		U16550C_UartSetLCR(FALSE);
}

void U16550C_SetUartSpeed()
{
	unsigned int uiDivisorLatch = state.DivisorLatchLSB + (state.DivisorLatchMSB * 0x100);
	unsigned long ul16550CBaudRateN;
	if (state.hasCommPort)
	{
		if (!uiDivisorLatch)
			uiDivisorLatch = 1;
		ul16550CBaudRateN = UART_CLOCK_16X / (uiDivisorLatch*16);
		if (ul16550CBaudRateN!=ul16550CBaudRate)
		{
			ul16550CBaudRate = ul16550CBaudRateN;
			U16550C_UartSetLCR(TRUE);
		}
	}
}