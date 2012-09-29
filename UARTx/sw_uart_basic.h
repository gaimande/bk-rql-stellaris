//*****************************************************************************
//
// sw_uart_basic.h - Defines and Macros for the Software UART.
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

#ifndef __SW_UART_BASIC_H__
#define __SW_UART_BASIC_H__

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// The GPIO port and pins to use for the software UART, the defaults are to
// use the pins for the hardware UART that is present on all devices.  This
// allows the application to use existing hardware to connect to another devices
// UART.  These should be changed to the GPIO port and pins that the application
// wishes to use.
//
//*****************************************************************************
#define SWU_GPIO_INT_BASE       INT_GPIOD
#define SWU_GPIO_PORT_BASE      GPIO_PORTD_BASE
#define SWU_GPIO_RX             GPIO_PIN_2
#define SWU_GPIO_TX             GPIO_PIN_3

//*****************************************************************************
//
// Values that can be passed to UARTConfigSetExpClk as the ulConfig parameter
// and returned by UARTConfigGetExpClk in the pulConfig parameter.
// Additionally, the UART_CONFIG_PAR_* subset can be passed to
// UARTParityModeSet as the ulParity parameter, and are returned by
// UARTParityModeGet.
//
//*****************************************************************************
#define UART_CONFIG_WLEN_9      0x00000080  // 8 bit data
#define UART_CONFIG_WLEN_8      0x00000060  // 8 bit data
#define UART_CONFIG_WLEN_7      0x00000040  // 7 bit data
#define UART_CONFIG_WLEN_6      0x00000020  // 6 bit data
#define UART_CONFIG_WLEN_5      0x00000000  // 5 bit data
#define UART_CONFIG_STOP_ONE    0x00000000  // One stop bit
#define UART_CONFIG_STOP_TWO    0x00000008  // Two stop bits
#define UART_CONFIG_PAR_NONE    0x00000000  // No parity
#define UART_CONFIG_PAR_EVEN    0x00000006  // Even parity
#define UART_CONFIG_PAR_ODD     0x00000002  // Odd parity
#define UART_CONFIG_PAR_ONE     0x00000086  // Parity bit is one
#define UART_CONFIG_PAR_ZERO    0x00000082  // Parity bit is zero

#define BITSET(x, b)            (x = (x | (0x00000001 << (b))))
#define BITCLR(x, b)            (x = (x & ~(0x00000001 << (b))))

extern void SW_UARTConfigSetExpClk(unsigned long ulBase,
                                   unsigned long ulUARTClk,
                                   unsigned long ulBaud,
                                   unsigned long ulConfig);
extern void SW_UARTConfigGetExpClk(unsigned long ulBase,
                                   unsigned long ulUARTClk,
                                   unsigned long *pulBaud,
                                   unsigned long *pulConfig);
extern void SW_UARTEnable(unsigned long ulBase);
extern void SW_UARTDisable(unsigned long ulBase);
extern tBoolean SW_UARTCharsAvail(unsigned long ulBase);
extern tBoolean SW_UARTSpaceAvail(unsigned long ulBase);
extern long SW_UARTCharGetNonBlocking(unsigned long ulBase);
extern long SW_UARTCharGet(unsigned long ulBase);
extern tBoolean SW_UARTCharPutNonBlocking(unsigned long ulBase,
                                          unsigned long ulData);
extern void SW_UARTCharPut(unsigned long ulBase, unsigned char ucData);
extern void SW_UARTSendNextBit(void);

#ifdef __cplusplus
}
#endif

#endif
