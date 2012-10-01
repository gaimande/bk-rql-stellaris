//*****************************************************************************
//
// uart_echo.c - Example for reading data from and writing data to the UART in
//               an interrupt driven fashion.
//
// Copyright (c) 2008-2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 9107 of the DK-LM3S9B96 Firmware Package.
//
//*****************************************************************************
#include "inc/lm3s9790.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "grlib/grlib.h"
#include "glcd.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>UART Echo (uart_echo)</h1>
//!
//! This example application utilizes the UART to echo text.  The first UART
//! (connected to the FTDI virtual serial port on the evaluation board) will be
//! configured in 115,200 baud, 8-n-1 mode.  All characters received on the
//! UART are transmitted back to the UART.
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

//DELAY FUNCTION
unsigned long g_ulCounter = 0;
void
SysTickIntHandler(void)
{
    // Update the Systick interrupt counter.
    g_ulCounter++;
}
//this called from outside 
void delay_ms (unsigned int milliseconds)
{ 
   // Initialize the interrupt counter.
   g_ulCounter = 0;

   // Set up the period for the SysTick timer.  The SysTick timer period will
   // be equal to the system clock, resulting in a period of 1 second.
   //here I will divide by 1000 so Interrupt at 1 ms
   ROM_SysTickPeriodSet(ROM_SysCtlClockGet()/1000);

   // Enable interrupts to the processor.
   ROM_IntMasterEnable();

   // Enable the SysTick Interrupt.
   ROM_SysTickIntEnable();

   // Enable SysTick.
   ROM_SysTickEnable();

   // Loop  while the SysTick count g_ulCounter is less than milliseconds.
   while(g_ulCounter < milliseconds);

} 

//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************
void
UARTIntHandler(void)
{
    unsigned long ulStatus;
	volatile unsigned long ulLoop;
    //
    // Get the interrrupt status.
    //
    ulStatus = ROM_UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART0_BASE, ulStatus);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(ROM_UARTCharsAvail(UART0_BASE))
    {
        //
        // Read the next character from the UART and write it back to the UART.
        //
        //ROM_UARTCharPutNonBlocking(UART0_BASE,
        //                           UARTCharGetNonBlocking(UART0_BASE));
		if(UARTCharGetNonBlocking(UART0_BASE)=='A')
		{
			GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,~GPIO_PIN_7);	
			for(ulLoop = 0; ulLoop < 10000000; ulLoop++)
        	{
        	}
			GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,GPIO_PIN_7);	
			for(ulLoop = 0; ulLoop < 10000000; ulLoop++)
        	{
        	}
		}
		ROM_UARTCharPutNonBlocking(UART1_BASE,UARTCharGetNonBlocking(UART0_BASE));
    }
}

//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
void
UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        //
        // Write the next character to the UART.
        //
        ROM_UARTCharPutNonBlocking(UART1_BASE, *pucBuffer++);
    }
}

//*****************************************************************************
//
// This example demonstrates how to send a string of data to the UART.
//
//*****************************************************************************
int
main(void)
{
	volatile unsigned long ulLoop;
    tRectangle sRect;

    //
	// Set the clocking to run directly from the internal crystal/oscillator.
	//
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_INT | SYSCTL_MAIN_OSC_DIS |
		SYSCTL_XTAL_16MHZ);
	// Enable IO port for UARTs
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_7);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinConfigure(GPIO_PD2_U1RX);
	GPIOPinConfigure(GPIO_PD3_U1TX);
    //
    // Enable the (non-GPIO) peripherals used by this example.  PinoutSet()
    // already enabled GPIO Port A.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Set UART pins.
    //
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	ROM_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    //
    // Configure the UART for 9600, 8-N-1 operation.
    //
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
	ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    //
    // Enable the UART interrupt.
    //
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
	ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
   	
	//
    // Initialize the display driver.
    //
    GLCD_INIT();
	GLCD_FILL(0);
    GLCD_DRW_REC_SOLID(45,45,20,20,1);
	GLCD_OUT_STR(40,1,"Minh THu MT",1);
	GLCD_DRW_LINE(2,2,8,8,1);
    GLCD_DISPLAY();
    ROM_UARTCharPutNonBlocking(UART1_BASE,'D');
    //
    // Loop forever echoing data through the UART.
    //	   
	while(1)
	{
		GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,~GPIO_PIN_7);
		//UARTCharPut(UART0_BASE, 'A');
		//for(ulLoop = 0; ulLoop < 10000000; ulLoop++)
        //{
        //}	
	}
	while(1)
	{
		UARTCharPut(UART0_BASE, 'A');
		for(ulLoop = 0; ulLoop < 10000000; ulLoop++)
        {
        }
	}
    while(1)
    {
		UARTSend((unsigned char *)"Enter text: ", 12);
		//
        // Delay for a bit.
        //
        for(ulLoop = 0; ulLoop < 10000000; ulLoop++)
        {
        }
		//delay_ms(1);
    }
}
