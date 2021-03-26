//
//  SMXWiFi.h
//
//  Created by Oduvaldo Pavan Junior May 27th 2020
//  For commercial usage contact author @ ducasp@gmail.com
//	Free for opensource / non-commercial usages
//

#ifndef SMXWiFi_h
#define SMXWiFi_h

#define SMXWiFi_FIFO_Depth 4097
#include <windows.h>

enum SMXWiFi_UART_Commands {
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

void smxWiFiCreate ();
void smxWiFiDestroy ();
void smxWiFiReset();

#endif /* SMXWiFi_h */
