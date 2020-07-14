//
//  16550C.c
//
//  Created by Oduvaldo Pavan Junior July 10th 2020
//  For commercial usage contact author @ ducasp@gmail.com
//	Free for opensource / non-commercial usages
//
#include <windows.h>
#include "16550C.h"
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

#define INT_U16550C	0x1000

bool bHoldStatusRegister = FALSE;
unsigned char bFirstReceiveAfterTx = 0;
unsigned char bBufferUnderrun = 0;
ULONGLONG ullTicks;
const DWORD dwUartSpeedTable[10] = {859372, 346520, 231014, 115200, 57600, 38400, 31250, 19200, 9600, 4800};

struct U16550C_STATE
{
	UInt8 UARTSpeed;
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
	unsigned int FIFORxHead;
	unsigned int FIFORxTail;
	unsigned int FIFOTxHead;
	unsigned int FIFOTxTail;
    unsigned char FIFORxBuffer[U16550C_FIFO_Depth];
	unsigned char FIFOTxBuffer[U16550C_FIFO_Depth];
} state;

void U16550C_TxFIFOReset()
{
	state.FIFOTxHead = 0;
	state.FIFOTxTail = 0;
	memset(FIFOTxBuffer,0,U16550C_FIFO_Depth);
}

void U16550C_RxFIFOReset()
{
	state.FIFORxHead = 0;
	state.FIFORxTail = 0;
	memset(FIFORxBuffer,0,U16550C_FIFO_Depth);
}

void U16550C_TXDone(void)
{
	state.TXInProgress = 0;
}

void U16550C_RxFIFOIn(UInt8 value)
{
	if(!(state.FIFORxHead == (( state.FIFORxTail - 1 + U16550C_FIFO_Depth) % U16550C_FIFO_Depth)))
	{
		state.FIFORxBuffer[state.FIFORxHead] = value;
		state.FIFORxHead = (state.FIFORxHead+1) % U16550C_FIFO_Depth;
	}
}

UInt8 U16550C_RxFIFOOut()
{
	UInt8 value = 0xff; //If no data in Rx FIFO, returns 0xff	

	if (state.FIFORxTail == state.FIFORxHead)
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
		while ((GetTickCount64()<ullTicks)&&(state.FIFORxTail == state.FIFORxHead));
	}
	if (state.FIFORxTail != state.FIFORxHead)
	{
		value = state.FIFORxBuffer[state.FIFORxTail];
		state.FIFORxTail = (state.FIFORxTail + 1) % U16550C_FIFO_Depth;
	}
	else
		bBufferUnderrun = 1;
	return value;
}

static UInt8 U16550C_ReadUARTStatus(struct U16550C_STATE* state, UInt16 ioPort)
{
	UInt8 currentStatus=0;

	if (state->FIFORxTail != state->FIFORxHead)
		currentStatus|=1; //set rx buffer has data
	if (state->TXInProgress)
		currentStatus|=2; //set tx in progress
	if (state->FIFORxHead == (( state->FIFORxTail - 1 + U16550C_FIFO_Depth) % U16550C_FIFO_Depth))
		currentStatus|=4; //set rx buffer is full
	currentStatus|=8; // Set support to quick receive
	if (bBufferUnderrun)
	{
		currentStatus|=0x10; // Set buffer underrun
		bBufferUnderrun = 0;
	}
    return currentStatus;
}

static UInt8 U16550C_ReadUARTBuffer(struct U16550C_STATE* state, UInt16 ioPort)
{    
    return U16550C_RxFIFOOut();
}

static void U16550C_WriteUARTBuffer(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	bFirstReceiveAfterTx = 1;
	if ((state->hasCommPort))
	{
		archUartTransmit(value);
		//state->TXInProgress = 1;
	}
}

static void U16550C_WriteUARTCommand(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	if(value<=9) //Set UART Speed
	{
		state->UARTSpeed = value;
		//Set UART Speed
		archUartSetSpeed(dwUartSpeedTable[value]);
	}
	else if (value == 0x20)
	{
		//Clear any data in UART buffers
		U16550C_RxFIFOReset();
		U16550C_TxFIFOReset();
	}
}

static Uint8 U16550C_ReadBaseAddress0(struct U16550C_STATE* state, UInt16 ioPort)
{
}

static Uint8 U16550C_ReadBaseAddress1(struct U16550C_STATE* state, UInt16 ioPort)
{
}

static Uint8 U16550C_ReadBaseAddress2(struct U16550C_STATE* state, UInt16 ioPort)
{
}

static Uint8 U16550C_ReadBaseAddress3(struct U16550C_STATE* state, UInt16 ioPort)
{
}

static Uint8 U16550C_ReadBaseAddress4(struct U16550C_STATE* state, UInt16 ioPort)
{
}

bool U16550C_CheckFIFOForLineErrors()
{
	return false;
}

void U16550C_AdjustLineControlParameters()
{
}

static void U16550C_WriteBaseAddress3(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	UInt8 oldLCR = state->LineControlRegister;
	// 3 is the Line Control Register
	state->LineControlRegister = value;
	if ((oldLCR&0x7f)!=(state->LineControlRegister&0x7f))
		U16550C_AdjustLineControlParameters();
	return;
}

static Uint8 U16550C_ReadBaseAddress3(struct U16550C_STATE* state, UInt16 ioPort)
{
	// 3 is the modem control Register
	return state->LineControlRegister();
}

void U16550C_AdjustModemControlParameters()
{
}

static void U16550C_WriteBaseAddress4(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	UInt8 oldLCR = state->ModemControlRegister;
	// 4 is the modem control register
	state->ModemControlRegister = value & 0x3F; // bits 6 and 7 are always zero
	if (oldLCR!=state->ModemControlRegister)
		U16550C_AdjustModemControlParameters()
	return;
}

static Uint8 U16550C_ReadBaseAddress4(struct U16550C_STATE* state, UInt16 ioPort)
{
	// 6 is the modem control Register
	return state->ModemControlRegister;
}

static void U16550C_WriteBaseAddress5(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	// 5 is the Line Status Register, can't write to it
	return;
}

static Uint8 U16550C_ReadBaseAddress5(struct U16550C_STATE* state, UInt16 ioPort)
{
	// 5 is the Line Status Register
	Uint8 returnValue = state->LineStatusRegister;

	if (!U16550C_CheckFIFOForLineErrors())
		state->LineStatusRegister &= 0x63; //Clear error bits and FIFO has error bit if there are no errors or not in FIFO mode
	else
		state->LineStatusRegister &= 0xe3; //Clear error bits

	return returnValue;
}

static void U16550C_WriteBaseAddress6(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	// 6 is the modem status register, can't write to it
	return;
}

static Uint8 U16550C_ReadBaseAddress6(struct U16550C_STATE* state, UInt16 ioPort)
{
	// 6 is the modem status register
	bHoldStatusRegister = TRUE;
	Uint8 returnValue = state->ModemStatusRegister;

	state->ModemStatusRegister &= 0xf0; //Clear delta indication
	bHoldStatusRegister = FALSE;
	return returnValue;
}

static void U16550C_WriteBaseAddress7(struct U16550C_STATE* state, UInt16 ioPort, UInt8 value)
{
	// 7 is the scratch register
	state->ScratchRegister = value;
}

static Uint8 U16550C_ReadBaseAddress7(struct U16550C_STATE* state, UInt16 ioPort)
{
	// 7 is the scratch register
	return state->ScratchRegister;
}

void U16550C_Create ()
{
	Properties* pProperties = propGetGlobalProperties();
	
	U16550C_Reset();

	if (pProperties->ports.Com.directuartio == 1)
		state.hasCommPort = archUartCreate2(U16550C_RxFIFOIn, U16550C_TXDone, dwUartSpeedTable[state.UARTSpeed]);
	else
		state.hasCommPort = FALSE;

    ioPortRegister(0x80, U16550C_ReadBaseAddress0, U16550C_WriteBaseAddress0,  (void*) &state);
    ioPortRegister(0x81, U16550C_ReadBaseAddress1, U16550C_WriteBaseAddress1,  (void*) &state);
	ioPortRegister(0x82, U16550C_ReadBaseAddress2, U16550C_WriteBaseAddress2,  (void*) &state);
	ioPortRegister(0x83, U16550C_ReadBaseAddress3, U16550C_WriteBaseAddress3,  (void*) &state);
	ioPortRegister(0x84, U16550C_ReadBaseAddress4, U16550C_WriteBaseAddress4,  (void*) &state);
	ioPortRegister(0x85, U16550C_ReadBaseAddress5, U16550C_WriteBaseAddress5,  (void*) &state);
	ioPortRegister(0x86, U16550C_ReadBaseAddress6, U16550C_WriteBaseAddress6,  (void*) &state);
	ioPortRegister(0x87, U16550C_ReadBaseAddress7, U16550C_WriteBaseAddress7,  (void*) &state);
}

void U16550C_Destroy ()
{
    ioPortUnregister(0x06);
    ioPortUnregister(0x07);
	archUartDestroy();
}

void U16550C_Reset()
{
	state.UARTSpeed = 0x00; //start at fastest UART speed
	boardClearInt(INT_U16550C);
	//boardSetInt(INT_U16550C);
	U16550C_RxFIFOReset();
	U16550C_TxFIFOReset();
	InterruptEnableRegister = 0;
	InterruptIdentificationRegister = 1;
	FIFOControlRegister = 0;
	LineControlRegister = 0;
	ModemControlRegister = 0;
	LineStatusRegister = 0x60;
	ModemStatusRegister = 0;
}