/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Arch/ArchUart.h,v $
**
** $Revision: 1.4 $
**
** $Date: 2008-03-31 19:42:19 $
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
#ifndef ARCH_UART_H
#define ARCH_UART_H

#include "MsxTypes.h"

void archUartTransmit(UInt8 value);
int archUartSetSpeed(unsigned long dwNewSpeed);
void archUartSetLCR(unsigned char ucLCR);
void archUartEscapeCommFunction(unsigned long dwFunc);
unsigned long archUartGetModemStatus();
int archUartCreate(void (*archUartReceiveCallback) (UInt8));
int archUartCreate2(void (*archUartReceiveCallback) (UInt8), void (*archUartTransmitCallback) (void), unsigned long dwCreationSpeed);
int archUartCreate3(void (*archUartReceiveCallback) (UInt8,unsigned long), void (*archUartControlCallback) (unsigned long), unsigned long dwCreationSpeed);
void archUartDestroy(void);

#endif
