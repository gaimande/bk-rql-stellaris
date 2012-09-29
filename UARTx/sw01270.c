//*****************************************************************************
//
// sw01270.c - Sample application using software based UART.
//
// Copyright (c) 2009 Luminary Micro, Inc.  All rights reserved.
// 
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 4049 of the Software UART Example.
//
//*****************************************************************************

#include "inc/hw_ints.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "sw_uart_basic.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Software UART example</h1>
//!
//! A is a simple application that uses the software UART to echo characters
//! received back out the UART.  This example reuses the pins for the hardware
//! based UART0 and should work on most evaluation kits provided by Luminary
//! Micro.  The assumtion that is application makes is that the main crystal on
//! the board is 8MHz.  This can be changed by changing the value passed in to
//! the SysCtlClockSet() function to the correct value for the given board.
//!
//! The default baud rate for the example is 19200 N-8-1.
//
//*****************************************************************************

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//******************************************************************************
//
// The welcome string printed by the application.
//
//******************************************************************************
const char g_pucWelcomeStr[] = "Be Quyen nho be Thu lem";

//******************************************************************************
//
// A simple example application that uses the software UART that echos
// characters whenever one is received.
//
//******************************************************************************
int
main(void)
{
    int iIdx;
    long lChar;

    //
    // If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    // a workaround to allow the PLL to operate reliably.
    //
    if(REVISION_IS_A2)
    {
        SysCtlLDOSet(SYSCTL_LDO_2_75V);
    }

    //
    // Set the clocking to run directly from the crystal.
    //
    //SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
    //               SYSCTL_XTAL_8MHZ);
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_INT | SYSCTL_MAIN_OSC_DIS |
		SYSCTL_XTAL_16MHZ);
    //
    // Enable Interrupts and configure the UART.
    //
    IntMasterEnable();

    SW_UARTConfigSetExpClk(0, SysCtlClockGet(), 9600,
                           (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE));

    //
    // Enable the software UART.
    //
    SW_UARTEnable(TIMER0_BASE);

    //
    // Set new line, CR, LF
    //
    SW_UARTCharPut(0,'\n');
    SW_UARTCharPut(0,'\r');

    //
    // Display Welcome String
    //
    for(iIdx = 0; iIdx < sizeof(g_pucWelcomeStr); iIdx++)
    {
        SW_UARTCharPut(0, g_pucWelcomeStr[iIdx]);
    }

    //
    // Set new line, CR, LF
    //
    SW_UARTCharPut(0,'\n');
    SW_UARTCharPut(0,'\r');

    //
    // Echo characters received
    //
    while(1)
    {
        //
        // Get the next character from the UART.
        //
		SW_UARTCharPut(0,'\r');
        lChar = SW_UARTCharGetNonBlocking(0);

        //
        // If there was a non-zero character then echo it.
        //
        if(lChar > 0)
        {
            SW_UARTCharPut(0, (char)lChar);
        }
    }
}

