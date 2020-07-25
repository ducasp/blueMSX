//
//  16550C.h
//
//  Created by Oduvaldo Pavan Junior July 10th 2020
//  For commercial usage contact author @ ducasp@gmail.com
//	Free for opensource / non-commercial usages
//

#ifndef U16550C_h
#define U16550C_h

#define U16550C_FIFO_Depth 17 // One byte is wasted
#include <windows.h>
#include "MsxTypes.h"

void CALLBACK U16550C_RXTimeOutCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
//Win32 Uart Functions
void U16550C_UartTransmit(BYTE value);
DWORD U16550C_PortReadStatus (LPVOID lpvoid);
BOOL U16550C_UartSetLCR (BOOL bForce);
void U16550C_UartDestroy(void);
BOOL U16550C_UartCreate(void);

//Below Functions are used by BlueMSX to handle our device
DWORD U16550C_PortReadThread (LPVOID lpvoid);
void U16550C_Create();
void U16550C_Destroy();
static unsigned char U16550C_ReadBaseAddress0(struct U16550C_STATE* state, unsigned short ioPort);
static unsigned char U16550C_ReadBaseAddress1(struct U16550C_STATE* state, unsigned short ioPort);
static unsigned char U16550C_ReadBaseAddress2(struct U16550C_STATE* state, unsigned short ioPort);
static unsigned char U16550C_ReadBaseAddress3(struct U16550C_STATE* state, unsigned short ioPort);
static unsigned char U16550C_ReadBaseAddress4(struct U16550C_STATE* state, unsigned short ioPort);
static unsigned char U16550C_ReadBaseAddress5(struct U16550C_STATE* state, unsigned short ioPort);
static unsigned char U16550C_ReadBaseAddress6(struct U16550C_STATE* state, unsigned short ioPort);
static unsigned char U16550C_ReadBaseAddress7(struct U16550C_STATE* state, unsigned short ioPort);
static void U16550C_WriteBaseAddress0(struct U16550C_STATE* state, unsigned short ioPort, unsigned char value);
static void U16550C_WriteBaseAddress1(struct U16550C_STATE* state, unsigned short ioPort, unsigned char value);
static void U16550C_WriteBaseAddress2(struct U16550C_STATE* state, unsigned short ioPort, unsigned char value);
static void U16550C_WriteBaseAddress3(struct U16550C_STATE* state, unsigned short ioPort, unsigned char value);
static void U16550C_WriteBaseAddress4(struct U16550C_STATE* state, unsigned short ioPort, unsigned char value);
static void U16550C_WriteBaseAddress5(struct U16550C_STATE* state, unsigned short ioPort, unsigned char value);
static void U16550C_WriteBaseAddress6(struct U16550C_STATE* state, unsigned short ioPort, unsigned char value);
static void U16550C_WriteBaseAddress7(struct U16550C_STATE* state, unsigned short ioPort, unsigned char value);

// Below functions are for internal use only
// Startup of UART
unsigned char U16550C_Reset();
// Modem Control Functions
void U16550C_AdjustModemControlParameters(unsigned char oldParam);
void U16550C_GetUartStatus();
void U16550C_ClearBreak();
void U16550C_SetBreak();
void U16550C_ClearDTR();
void U16550C_SetDTR();
void U16550C_ClearRTS();
void U16550C_SetRTS();
// RX Specific Functions
void U16550C_RxFIFOReset();
void U16550C_RxFIFOIn(unsigned char value, unsigned long error);
unsigned char U16550C_RxFIFOOut();
unsigned char U16550C_RxFIFOFull();
void U16550C_RxFIFOThresholdHit();
unsigned char U16550C_CheckFIFOForLineErrors();
void U16550C_RXTimeOutCancel();
void U16550C_RXTimeOutSchedule();
//TX Specific Functions
void U16550C_TxFIFOReset();
void U16550C_TxFIFOIn(unsigned char value);
unsigned char U16550C_TxFIFOOut();
unsigned char U16550C_TxFIFOFull();
static void U16550C_WriteUARTBuffer(unsigned char value);
void U16550C_SignalTXThreadToMove();
static unsigned long U16550C_TXThread (LPVOID lpvoid);
//Internal UART functionality functions
void U16550C_EvaluateIRQ();
void U16550C_FIFOStatusChange(unsigned long value);
void U16550C_FIFOStatusUpdate(unsigned long value);
void U16550C_AdjustLineControlParameters();
void U16550C_SetUartSpeed();


#endif /* U16550C_h */
