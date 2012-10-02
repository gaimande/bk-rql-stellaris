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
#include "utils/uartstdio.h"
#include "driverlib/adc.h"

//Global Variable
extern unsigned char Welcome01[1024];
extern unsigned char Welcome02[1024];
extern unsigned char menu_status[1024];
extern unsigned char menu_setup[1024];
extern unsigned char menu_history[1024];
extern unsigned char menu_factory[1024];
extern unsigned char setup_forward[1024];
extern unsigned char setup_elchem_frd[1024];
extern unsigned char setup_elchem_rsv[1024];
extern unsigned char setup_elchem_pressure[1024];
extern unsigned char setup_exit[1024];
extern unsigned char setup_preservation[1024];
extern unsigned char setup_pressure[1024];
extern unsigned char setup_reverse[1024];
const char keyPadMatrix[] = 
{ 
    '1','2','3','U',
    '4','5','6','D',
    '7','8','9','<',
    'T','0','C','@',
    0xFF
};

char ScanKeyMatrix()
{
    // This routine returns the first key found to be
    // pressed during the scan.
    char key = 0;
	int row;
	
    for( row = 0x10; row < 0x100; row <<= 1 )
    {     
        {   // turn on row output
            //row1port = row.0;
            //row2port = row.1;
            //row3port = row.2;
            //row4port = row.3;
			GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,row);
        }   

        // read colums - break when key press detected
		
        if( GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_0 ) )
            break;
        key++;
        if( GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_1 ) )
            break;
        key++;
        if( GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_2 ) )
            break;
        key++;
		if( GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_3 ) )
            break;
        key++;
    }
	
	GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,0);
        
    return keyPadMatrix[key]; 
}

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

char i,j;
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
		switch(UARTCharGetNonBlocking(UART0_BASE)){
			case 'P':
				//Start pump
				GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,~GPIO_PIN_7);
				break;
			case 'F':
				//Turn off pump
				GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,GPIO_PIN_7);	
				break;
			case 'H':
				//Turn off pump for fill Hemoclean from Mixing Module
				GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,GPIO_PIN_7);	
				break;	
			case 'S':
				//Turn off pump for waiting Hemoclean
				GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,GPIO_PIN_7);		
				break;
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
        ROM_UARTCharPut(UART1_BASE, *pucBuffer++);
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
	char key_return;
	int row = 2;
	unsigned long ADC_DATA;
	char menu_index,setup_index,ok_set;
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
	
	//Matrix keypad configure
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7); //Configure ROWs are output

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3); //Configure COLUMNs are input	
	GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    //
    // Enable the (non-GPIO) peripherals used by this example. 
    // already enabled GPIO Port A.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	
	/*
	//
    // Configure the ADC to get value from conductivity sensor
    //
    ADCSequenceConfigure(ADC_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0,
                             ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 0);
	*/
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
    // Configure UARTs for 9600, 8-N-1 operation.
    //
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
	ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
	UARTStdioInit(1);
    //
    // Enable UARTs interrupt.
    //
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
	ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
   	
	//
    // Initialize the display driver.
    //
    GLCD_INIT();
	GLCD_IMAGE(Welcome01);
    GLCD_DISPLAY();
	SysCtlDelay(5000000);	
	GLCD_IMAGE(Welcome02);
	GLCD_DISPLAY();
	SysCtlDelay(5000000);
	GLCD_IMAGE(menu_status);
    GLCD_DISPLAY();
	/*
	SysCtlDelay(5000000);
	GLCD_IMAGE(menu_setup);
    GLCD_DISPLAY();
	SysCtlDelay(5000000);
	GLCD_IMAGE(menu_history);
    GLCD_DISPLAY();
	SysCtlDelay(5000000);
	GLCD_IMAGE(menu_factory);
    GLCD_DISPLAY();
	SysCtlDelay(5000000);
	GLCD_IMAGE(setup_forward);
    GLCD_DISPLAY();
	SysCtlDelay(5000000);
	GLCD_IMAGE(setup_elchem_frd);
    GLCD_DISPLAY();
	/*
	//Send notification
	UARTSend((unsigned char *)"Automated Dialyzer and Bloodline Washing System", 47);

	//GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,0x20);
	//while(1);
	
	UARTprintf("\nBe Quyen la be Quyen leu leu Be Thu %d",4664);
	
	/*
	//
    // Read the data from the ADC.
    //
	//
	// Trigger the sample sequence.
	//
	ADCProcessorTrigger(ADC_BASE, 0);
	
	//
	// Wait until the sample sequence has completed.
	//
	while(!ADCIntStatus(ADC_BASE, 0, false)){}
	//
	// Read the value from the ADC.
	//
	ADCSequenceDataGet(ADC_BASE, 0, &ADC_DATA);
	UARTprintf("Testing ADC0 seq 0 on DMA channel %d\n",ADC_DATA);
	*/
/*	
	while(1)
	{
		key_return = ScanKeyMatrix();
		if(key_return != 0xFF)
			ROM_UARTCharPutNonBlocking(UART1_BASE,key_return);
	}	
	*/	
	menu_index = 1;
	ok_set = 0;
	
	main_menu:
	while(1)
	{
		switch(ScanKeyMatrix())
		{
			case 'U':
				if(menu_index==1)
				{
					menu_index = 4;
					break;
				}
				else
				menu_index--;
				break;
			case 'D':	
				if(menu_index==4)
				{
					menu_index = 1;
					break;
				}
				else
				menu_index++;
				break;
			case '@':
				ok_set = 1;
				break;
		}
		switch(menu_index)
		{
			case 1:				
				GLCD_IMAGE(menu_status);
				GLCD_DISPLAY();
				break;
			case 2:	
				if(ok_set)
				{
					GLCD_IMAGE(setup_forward);
					GLCD_DISPLAY();
					ok_set = 0;
					setup_index = 1;
					goto setup_menu;
				}				
				GLCD_IMAGE(menu_setup);
				GLCD_DISPLAY();
				break;
			case 3:	
				GLCD_IMAGE(menu_history);
				GLCD_DISPLAY();
				break;	
			case 4:	
				GLCD_IMAGE(menu_factory);
				GLCD_DISPLAY();
				break;					
		}

	}
	
	setup_menu:
	while(1)
	{
		switch(ScanKeyMatrix())
		{
			case 'U':
				if(setup_index==1)
					break;
				else
				setup_index--;
				break;
			case 'D':	
				if(setup_index==8)
					break;
				else
				setup_index++;
				break;
			case '@':
				ok_set = 1;
				break;
		}
		switch(setup_index)
		{
			case 1:				
				GLCD_IMAGE(setup_forward);
				GLCD_DISPLAY();
				break;
			case 2:				
				GLCD_IMAGE(setup_reverse);
				GLCD_DISPLAY();
				break;
			case 3:	
				GLCD_IMAGE(setup_pressure);
				GLCD_DISPLAY();
				break;	
			case 4:	
				GLCD_IMAGE(setup_preservation);
				GLCD_DISPLAY();
				break;		
			case 5:	
				GLCD_IMAGE(setup_elchem_frd);
				GLCD_DISPLAY();
				break;
			case 6:	
				GLCD_IMAGE(setup_elchem_rsv);
				GLCD_DISPLAY();
				break;
			case 7:	
				GLCD_IMAGE(setup_elchem_pressure);
				GLCD_DISPLAY();
				break;
			case 8:	
				if(ok_set)
				{
					menu_index = 2;
					setup_index = 0;
					ok_set = 0;
					goto main_menu;	
				}	
				GLCD_IMAGE(setup_exit);
				GLCD_DISPLAY();
				break;
		}
		
	}
	
	
	
	
	while(1)
	{
		if( GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_3 ) )
		{
			SysCtlDelay(500000);	//delay 40ms	
			if( GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_3 ) )			
				UARTSend((unsigned char *)"\nPin 3 is turned on", 19);
		}
	}
	while(1)
	{
		UARTCharPut(UART0_BASE, 'A');
		for(ulLoop = 0; ulLoop < 10000000; ulLoop++)
        {
        }
	}

}
