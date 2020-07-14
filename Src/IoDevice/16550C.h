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

void U16550C_Create ();
void U16550C_Destroy ();
void U16550C_Reset();

#endif /* U16550C_h */
