Software UART example</h1>

A is a simple application that uses the software UART to echo characters
received back out the UART.  This example reuses the pins for the hardware
based UART0 and should work on most evaluation kits provided by Luminary
Micro.  The assumtion that is application makes is that the main crystal on
the board is 8MHz.  This can be changed by changing the value passed in to
the SysCtlClockSet() function to the correct value for the given board.

The default baud rate for the example is 19200 N-8-1.

-------------------------------------------------------------------------------

Copyright (c) 2009 Luminary Micro, Inc.  All rights reserved.

Software License Agreement

Luminary Micro, Inc. (LMI) is supplying this software for use solely and
exclusively on LMI's microcontroller products.

The software is owned by LMI and/or its suppliers, and is protected under
applicable copyright laws.  All rights are reserved.  You may not combine
this software with "viral" open-source software in order to form a larger
program.  Any use in violation of the foregoing restrictions may subject
the user to criminal sanctions under applicable laws, as well as to civil
liability for the breach of the terms and conditions of this license.

THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

This is part of revision 4049 of the Software UART Example.
