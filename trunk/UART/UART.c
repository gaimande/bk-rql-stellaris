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
unsigned char truck_bmp[1024] = {
   0,  0,  0,  0,  0,248,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12,
  12, 10, 10, 10, 10, 10, 10,  9,  9,  9,  9,  9,  9,  9,  9,  9,
   9,  9,  9,  9,  9,  9,  9,  9,  9,  9,137,137,137,137,137,137,
 137,137,137,137,137,137,137,  9,  9,  9,  9,  9,  9,  9,  9,  9,
   9,  9, 13,253, 13,195,  6,252,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
 240,240,240,240,240,224,224,240,240,240,240,240,224,192,192,224,
 240,240,240,240,240,224,192,  0,  0,  0,255,255,255,255,255,195,
 195,195,195,195,195,195,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,255,240, 79,224,255, 96, 96, 96, 32, 32, 32, 32, 32,
  32, 32, 32, 32, 32, 32, 32, 32, 64, 64, 64, 64,128,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
 255,255,255,255,255,  0,  0,  0,  0,255,255,255,255,255,  0,  0,
   0,  0,255,255,255,255,255,  0,  0,  0,255,255,255,255,255,129,
 129,129,129,129,129,129,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,255,  1,248,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
   8,  8,  8,  8, 16,224, 24, 36,196, 70,130,130,133,217,102,112,
 160,192, 96, 96, 32, 32,160,160,224,224,192, 64, 64,128,128,192,
  64,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0, 63, 96, 96, 96,224, 96, 96, 96, 96, 96, 96,
  99, 99, 99, 99, 99, 96, 96, 96, 96, 99, 99, 99, 99, 99, 96, 96,
  96, 96, 99, 99, 99, 99, 99, 96, 96, 96, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 96, 96, 96, 96, 96, 96, 96, 64, 64,
  64,224,224,255,246,  1, 14,  6,  6,  2,  2,  2,  2,  2,  2,  2,
   2,  2,  2,  2,130, 67,114, 62, 35, 16, 16,  0,  7,  3,  3,  2,
   4,  4,  4,  4,  4,  4,  4, 28, 16, 16, 16, 17, 17,  9,  9, 41,
 112, 32, 67,  5,240,126,174,128, 56,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,
   1,  1,127,127,127,127,255,255,247,251,123,191, 95, 93,125,189,
 189, 63, 93, 89,177,115,243,229,207, 27, 63,119,255,207,191,255,
 255,255,255,255,255,255,255,127,127,127,127,127,127,127,127,255,
 255,255,127,127,125,120,120,120,120,120,248,120,120,120,120,120,
 120,248,248,232,143,  0,  0,  0,  0,  0,  0,  0,  0,128,240,248,
 120,188,220, 92,252, 28, 28, 60, 92, 92, 60,120,248,248, 96,192,
 143,168,216,136, 49, 68, 72, 50,160, 96,  0,  0,  0,  0,  0,  0,
   0,  0,  0,128,192,248,248,248,248,252,254,254,254,254,254,254,
 254,254,254,254,254,255,255,255,255,255,246,239,208,246,174,173,
 169,128,209,208,224,247,249,255,255,252,220,240,127,255,223,255,
 255,255,255,255,255,254,254,255,255,255,255,255,255,255,254,255,
 255,255,255,255,255,255,254,254,254,254,254,254,254,254,254,254,
 254,254,254,254,255,255,255,255,255,255,254,255,190,255,255,253,
 240,239,221,223,254,168,136,170,196,208,228,230,248,127,126,156,
 223,226,242,242,242,242,242,177, 32,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  1,  1,  1,  1,  3,  3,  3,  7,  7,  7,  7,  7, 15,
  15, 15,  7, 15, 15, 15,  7,  7, 15, 14, 15, 13, 15, 47, 43, 43,
  43, 43, 43, 47,111,239,255,253,253,255,254,255,255,255,255,255,
 191,191,239,239,239,191,255,191,255,255,255,255,255,255,255,255,
 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
 255,255,255,255,127,127,127,127,255,255,191,191,191,191,255,254,
 255,253,255,255,255,251,255,255,255,127,125, 63, 31, 31, 31, 31,
  31, 31, 63, 15, 15,  7,  7,  3,  3,  3,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,
   1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
   1,  1,  1,  1,  3,  3,  3, 11, 11, 11, 11,  7,  3, 14,  6,  6,
   6,  2, 18, 19, 19,  3, 23, 21, 21, 17,  1, 19, 19,  3,  6,  6,
  14, 15, 15,  7, 15, 15, 15, 11,  2,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

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
	//GLCD_FILL(0);
	GLCD_IMAGE(truck_bmp);
    //GLCD_DRW_REC_SOLID(45,45,20,20,1);
	//GLCD_CHAR_SET(0, 0, basic_font, 'A');
	//GLCD_OUT_STR(40,1,"Minh THu MT",1);
	//GLCD_DRW_LINE(2,2,8,8,1);
	//GLCD_OUT_DEC(60, 0 ,1234 ,4, 1);
    GLCD_DISPLAY();

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
	while(1)
	{
		 key_return = ScanKeyMatrix();
		 if(key_return != 0xFF)
		 	ROM_UARTCharPutNonBlocking(UART1_BASE,key_return);
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
