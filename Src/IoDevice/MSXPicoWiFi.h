//
//  MSMPicoWiFi.h
//
//  Created by Oduvaldo Pavan Junior September 9th 2024
//  For commercial usage contact author @ ducasp@gmail.com
//	Free for opensource / non-commercial usages
//

#ifndef MSXPicoWiFi_h
#define MSXPicoWiFi_h

#define MSXPicoWiFi_FIFO_Depth 4097
#include <windows.h>
#include "msxtypes.h"

enum MSXPicoWiFi_UART_Commands {
	UART_SPEED_859372 = 0,
	UART_SPEED_346520, //1
	UART_SPEED_231014, //2
	UART_SPEED_115200, //3
	UART_SPEED_57600, //4
	UART_SPEED_38400, //5
	UART_SPEED_31250, //6
	UART_SPEED_19200, //7
	UART_SPEED_9600, //8
	UART_SPEED_4800, //9
	UART_CLEAR_BUFFER = 20
};

void msxPicoWiFiCreate ();
void msxPicoWiFiDestroy ();
void msxPicoWiFiReset();
UInt8 msxPicoWiFiReadUARTStatus();
UInt8 msxPicoWiFiReadUARTBuffer();
UInt8 msxPicoWiFiReadF2();
void msxPicoWiFiWriteF2(UInt8 value);
void msxPicoWiFiWriteUARTBuffer(UInt8 value);
void msxPicoWiFiWriteUARTCommand(UInt8 value);


#endif /* MSXPicoWiFi_h */
