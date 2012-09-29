//*****************************************************************************
//
// sw_uart_basic.c - Driver for the software based UART.
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

//*****************************************************************************
//
//! \addtogroup sw_uart_api
//! @{
//
//*****************************************************************************

#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "sw_uart_basic.h"

//*****************************************************************************
//
// Typedef and structure
//
//*****************************************************************************
typedef enum
{
    SWUART_PAR_NONE = 0,
    SWUART_PAR_EVEN,
    SWUART_PAR_ODD,
    SWUART_PAR_MARK,
    SWUART_PAR_SPACE
}
tParity;

typedef enum
{
    SWUART_STOP_ONE,
    SWUART_STOP_TWO
}
tStopBits;

typedef enum
{
    SWUART_START_BIT = 0,
    SWUART_DATA_1,
    SWUART_DATA_2,
    SWUART_DATA_3,
    SWUART_DATA_4,
    SWUART_DATA_5,
    SWUART_DATA_6,
    SWUART_DATA_7,
    SWUART_DATA_8,
    SWUART_DATA_9,
    SWUART_STOP_1,
    SWUART_STOP_2
}
tRxTxStates;

static struct
{
    tParity Parity;
    int iDataBits;
    tStopBits StopBits;
    unsigned long ulBaudRate;
}
g_sSoftwareUART;

//*****************************************************************************
//
// Place holder for Received character interrupt function
//
//*****************************************************************************
void (*g_pfnHandler)(void) = 0;

//*****************************************************************************
//
//! Receive buffer / Variables
//
//*****************************************************************************
#define RX_BUFFER_SIZE 8
static unsigned long g_ulRxBuffer[RX_BUFFER_SIZE];
static unsigned long *g_pulRxBufferHead, *g_pulRxBufferTail;
static unsigned long g_ulRxBufferCount;

//*****************************************************************************
//
// Transmit buffer / Variables
//
//*****************************************************************************
#define TX_BUFFER_SIZE 8
static unsigned long g_ulTxBuffer[TX_BUFFER_SIZE];
static unsigned long *g_pulTxBufferHead, *g_pulTxBufferTail;
static unsigned long g_ulTxBufferCount;

//*****************************************************************************
//
// Transmit / Receive state machine placeholders, variables
//
//*****************************************************************************
static tRxTxStates g_currentTxState;
static unsigned long g_ulTxBitMask;
static unsigned char g_ucTxLine = 1;
static unsigned long g_ulRxChar = 0;
static tRxTxStates g_currentRxState;
static unsigned long g_ulTxBaudTimerTickValue, g_ulRxBaudTimerTickValue;

//*****************************************************************************
//
// Transmit / Receive buffer management
//
//*****************************************************************************

//*****************************************************************************
//
//! Handles the adding of data to the transmit character buffer.
//!
//! \param ulDate is the character to be added to the TX buffer.
//!
//! Add data to the transmit buffer and increment place holder
//! This funciton is called by SW_UARTConfigSetExpClk.
//!
//! \return Returns \b true if the data is accepted, and \b false
//! if there is no space left in the buffer.
//
//*****************************************************************************
tBoolean
TxBufferAdd(unsigned long ulData)
{
    tBoolean bR;
    bR = false;

    //
    // Check for space in TX buffer.
    //
    if(g_ulTxBufferCount < TX_BUFFER_SIZE)
    {
        //
        // OK, space avaialble. Add data
        // SW_UARTGenarateParity(&ulData);
        //
        *g_pulTxBufferHead++ = ulData;

        //
        // Increment the buffer count
        //
        g_ulTxBufferCount += 1;

        //
        // Check for wrap around
        //
        if(g_pulTxBufferHead == (g_ulTxBuffer + TX_BUFFER_SIZE))
        {
            //
            // Wrap around
            //
            g_pulTxBufferHead = g_ulTxBuffer;
        }
        bR = true;
    }
    return bR;
}

//*****************************************************************************
//
//! Handles the removal of data to the transmit character buffer.
//!
//! \param None.
//!
//! Removes data to the transmit buffer and decrements place holder
//! This funciton is called by \b SW_UARTSendNextBit.
//!
//! \return none.
//
//*****************************************************************************
void
TxBufferRemove(void)
{
    //
    // Nothing to do if the buffer is already empty.
    //
    if(g_ulTxBufferCount > 0)
    {
        //
        // Increment the tail.
        //
        g_pulTxBufferTail++;

        //
        // Decrement count.
        //
        g_ulTxBufferCount--;

        //
        // Check for wrap around.
        if( g_pulTxBufferTail == (g_ulTxBuffer + TX_BUFFER_SIZE) )
        {
            //
            // Wrap around.
            //
            g_pulTxBufferTail = g_ulTxBuffer;
        }
    }
}

//*****************************************************************************
//
//! Handles the adding of data to the receive character buffer.
//!
//! \param ulDate is the character to be added to the RX buffer.
//!
//! Add data to the receive buffer and increment place holder
//! This funciton is called by \b SW_UARTRcvNextBit.
//!
//! \return Returns \b true if the data is accepted, and \b false
//! if there is no space left in the buffer.
//
//*****************************************************************************
tBoolean
RxBufferAdd(unsigned long ulData)
{
    tBoolean bRetCode;

    //
    // Default to failing unless there is space.
    //
    bRetCode = false;

    //
    // Check for space in TX buffer.
    //
    if( g_ulRxBufferCount < RX_BUFFER_SIZE)
    {
        //
        // There was space avaialble.
        //
        *g_pulRxBufferHead++ = ulData;

        //
        // Increment the buffer count.
        //
        g_ulRxBufferCount += 1;

        //
        // Check for wrap around.
        //
        if(g_pulRxBufferHead == (g_ulRxBuffer + RX_BUFFER_SIZE))
        {
            //
            // Wrap around.
            //
            g_pulRxBufferHead = g_ulRxBuffer;
        }
        bRetCode = true;
    }
    return(bRetCode);
}

//*****************************************************************************
//
//! Handles the removal of data to the receive character buffer.
//!
//! \param None.
//!
//! Removes data to the rceive buffer and decrements place holder
//! This funciton is called by \b SW_UARTSendNextBit.
//!
//! \return none.
//
//*****************************************************************************
long
RxBufferRemove(void)
{
    long lReturnChar;

    //
    // Default to no characters present.
    //
    lReturnChar = -1;

    //
    // Check is buffer is already empty.
    //
    if(g_ulRxBufferCount > 0)
    {
        //
        // Get character
        //
        lReturnChar = *(long*)g_pulRxBufferTail;

        //
        // Increment the tail.
        //
        g_pulRxBufferTail++;

        //
        // Decrement count.
        //
        g_ulRxBufferCount--;

        //
        // Check for wrap around.
        //
        if(g_pulRxBufferTail == (g_ulRxBuffer + RX_BUFFER_SIZE))
        {
            //
            // Wrap around.
            //
            g_pulRxBufferTail = g_ulRxBuffer;
        }
    }
    return lReturnChar;
}

//*****************************************************************************
//
//! Initialize the SW Uart GPIO.
//!
//! \param None.
//!
//! Initializes the GPIO used for TX and RX
//! This funciton is called by SW_UARTConfigSetExpClk.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTInitGPIO(void)
{
    unsigned long ulSysCtlPeripheral;
    unsigned long ulOffset;

    if((ulOffset = ((SWU_GPIO_PORT_BASE & 0x000fffff) / 0x1000)) < 8)
    {
        //
        // Either Port A - D
        //
        ulSysCtlPeripheral = (0x20000000 + ((unsigned)0x1<<(ulOffset - 0x4)));
    }
    else
    {
        //
        // Port E - H
        //
        ulSysCtlPeripheral = (0x20000000 + ((unsigned)0x10<<(ulOffset - 0x24)));
    }

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(ulSysCtlPeripheral);

    //
    // Configure the first  pin of GPIO port D to be the TX output
    //
    GPIOPinTypeGPIOOutput(SWU_GPIO_PORT_BASE, SWU_GPIO_TX);

    //
    // Set TX output high
    //
    GPIOPinWrite(SWU_GPIO_PORT_BASE, SWU_GPIO_TX, SWU_GPIO_TX);

    //
    // Configure the second  pin of GPIO 'g_ulGpioTxRxPortBase' to be the RX
    // input Interrupt, Falling Edge, Push_Pull, Weak pullup.
    //
    GPIOPinTypeGPIOInput(SWU_GPIO_PORT_BASE, SWU_GPIO_RX);
    GPIOPinIntEnable(SWU_GPIO_PORT_BASE, SWU_GPIO_RX);
    GPIOIntTypeSet(SWU_GPIO_PORT_BASE, SWU_GPIO_RX,GPIO_FALLING_EDGE);

    g_ucTxLine = SWU_GPIO_TX;
}

//*****************************************************************************
//
//! Initialize the SW Uart Timers.
//!
//! \param None.
//!
//! Initializes Timer0 as 2 16 bit timers
//! Calculates the timer values based on baud rate
//! This funciton is called by SW_UARTConfigSetExpClk.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTInitTxRxTimers(unsigned long ulClkFreq)
{
    //
    // Calculate tick times for RX and TX timers
    //
    g_ulTxBaudTimerTickValue = ulClkFreq / g_sSoftwareUART.ulBaudRate;
    g_ulRxBaudTimerTickValue = (g_ulTxBaudTimerTickValue / 2);

    //
    // Enable the peripherals clock for Timer0.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //
    // Set priority for Timer 0A and 0B Timer0B (RX) has higher priority.
    //
    IntPrioritySet(INT_TIMER0A, 0x20);
    IntPrioritySet(INT_TIMER0B, 0x10);

    //
    // Configure the Timer 0 for two 16-bit periodic timers.
    //
    TimerConfigure(TIMER0_BASE, (TIMER_CFG_16_BIT_PAIR |
                                 TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC));
    //
    // Used for TX.
    //
    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ulTxBaudTimerTickValue);

    //
    // Used for RX, 1uS offset to account for interrupt delays.
    //
    TimerLoadSet(TIMER0_BASE, TIMER_B, (g_ulRxBaudTimerTickValue -
                                        (ulClkFreq/1000000)));

    //
    //Intialize TX State machine
    //
    g_currentTxState = SWUART_START_BIT;
}

//*****************************************************************************
//
//! Detects a start condition on the RX line
//!
//! \param None.
//!
//! Detects a vaild \b Start condition via the edge interrupt
//! On validation the interrupt is disabled, the RX Statemchine is
//! intialized and the RX timer is enabledto allow correct reception
//! of a charater.
//! This funciton is called from the \b NVIC.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTStartBitDetect(void)
{
    unsigned long ulPortIntsStatus;

    ulPortIntsStatus = HWREG(SWU_GPIO_PORT_BASE + GPIO_O_MIS);

    //
    // Check for RX pin interrupt
    //
    if( ulPortIntsStatus & SWU_GPIO_RX)
    {
        //
        // Disable the Start Detect interrupt
        //
        HWREG(SWU_GPIO_PORT_BASE + GPIO_O_IM) &= ~(SWU_GPIO_RX);
        HWREG(SWU_GPIO_PORT_BASE + GPIO_O_ICR) = SWU_GPIO_RX;

        //
        // Set intial state of 'g_currentRxState' to 'SWUART_START_BIT'
        //
        g_currentRxState = SWUART_DATA_1;

        //
        // Enable the Receive time (4uS offset to account for
        // interrupt processing
        //
        HWREG(TIMER0_BASE + TIMER_O_TBILR) = ( g_ulTxBaudTimerTickValue +
                                             g_ulRxBaudTimerTickValue - 200 );

        //
        // Used for RX, 1uS offset to account for interrupt delays
        //
        HWREG(TIMER0_BASE + TIMER_O_CTL) |= TIMER_B & (TIMER_CTL_TAEN |
                                                       TIMER_CTL_TBEN);
    }
}

//*****************************************************************************
//
//! TX timer interrupt handler
//!
//! \param None.
//!
//! Statemachine to handle frame transmission.
//! Uses the 'g_sSoftwareUART' control structure to determine the number of
//! data bits.
//! This funciton is called from the NVIC.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTSendNextBit(void)
{
    //
    // Clear the timer interrupt.
    //
    HWREG(TIMER0_BASE + TIMER_O_ICR) = TIMER_TIMA_TIMEOUT;

    HWREG(SWU_GPIO_PORT_BASE + (GPIO_O_DATA + (SWU_GPIO_TX << 2))) = g_ucTxLine;

    switch(g_currentTxState)
    {
        case SWUART_START_BIT:
        {
            //
            // Check for characters to send
            //
            if(g_ulTxBufferCount == 0)
            {
                //
                // No characters. Disable interrrupts
                //
                break;
            }
            //
            // Charcter to transmit. Set RX line low
            //
            g_ucTxLine = 0;

            //
            // Intitalise the bitmask
            //
            g_ulTxBitMask = 0x00000001;

            //
            // Set next state
            //
            g_currentTxState++;

            break;
        }
        case SWUART_DATA_1:
        case SWUART_DATA_2:
        case SWUART_DATA_3:
        case SWUART_DATA_4:
        case SWUART_DATA_9:
        {
            //
            // Set TX IO to next bit value
            //
            g_ucTxLine = (*g_pulTxBufferTail & g_ulTxBitMask)?SWU_GPIO_TX:0;

            //
            // Set New bitmask
            //
            g_ulTxBitMask <<= 1;

            //
            // Set next state
            //
            g_currentTxState++;

            break;
        }
        case SWUART_DATA_5:
        case SWUART_DATA_6:
        case SWUART_DATA_7:
        case SWUART_DATA_8:
        {
            //
            // Set TX IO to next bit value
            //
            g_ucTxLine = (*g_pulTxBufferTail & g_ulTxBitMask) ? SWU_GPIO_TX : 0;

            //
            // Check for end of data
            //
            if( g_sSoftwareUART.iDataBits == g_currentTxState)
            {
                //
                // Last data bit detected
                //
                g_currentTxState = SWUART_STOP_1;
                break;
            }

            //
            // Set New bitmask
            //
            g_ulTxBitMask<<=1;

            //
            // Set next state
            //
            g_currentTxState++;

            break;
        }
        case SWUART_STOP_1:
        {
            //
            // Set RX line to STOP condition
            //
            g_ucTxLine = SWU_GPIO_TX;

            //
            // Check the number of STOP bits required
            //
            if(g_sSoftwareUART.StopBits == SWUART_STOP_ONE)
            {
                //
                // Only one STOP bit required, transmission complete so remove
                // character from TX buffer.
                //
                TxBufferRemove();

                //
                // Reset TX Statemachine
                //
                g_currentTxState = SWUART_START_BIT;

                break;
            }

            //
            // More than one STOP bit required
            // Set next state
            //
            g_currentTxState++;

            break;
        }
        case SWUART_STOP_2:
        {
            //
            // Transmission complete, remove character from TX buffer.
            //
            TxBufferRemove();

            //
            // Reset TX Statemachine
            //
            g_currentTxState = SWUART_START_BIT;

            break;
        }
        default:
        {
            break;
        }
    }
}

//*****************************************************************************
//
//! RX timer interrupt handler
//!
//! \param None.
//!
//! Statemachine to handle frame reception.
//! Uses the 'g_sSoftwareUART' control structure to determine the number of
//! data bits. Receives on a bit by bit bases.
//! This funciton is called from the receive trimer NVIC entry.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTRcvNextBit(void)
{
    unsigned long ulRcvIoValue;

    //
    // Clear the timer interrupt.
    //
    HWREG(TIMER0_BASE + TIMER_O_ICR) = TIMER_TIMB_TIMEOUT;

    //
    // If this is the START bit the timeout value has to be
    // changed to a complete bit width
    //
    if(g_currentRxState == SWUART_DATA_1)
    {
        //
        // Set RX timer tick to bit width
        //
        HWREG(TIMER0_BASE + TIMER_O_ICR) = TIMER_TIMB_TIMEOUT;
        HWREG(TIMER0_BASE + TIMER_O_TBILR) = g_ulTxBaudTimerTickValue;

        //
        //Reset the receive character value.
        //
        g_ulRxChar = 0;
    }

    //
    //Read the RX GPIO
    //
    ulRcvIoValue = HWREG(SWU_GPIO_PORT_BASE + (GPIO_O_DATA +
                                               (SWU_GPIO_RX << 2)));

    switch(g_currentRxState)
    {
        case SWUART_DATA_1:
        case SWUART_DATA_2:
        case SWUART_DATA_3:
        case SWUART_DATA_4:
        case SWUART_DATA_9:
        {
            //
            // Update current rx char with the bit value received
            //
            g_ulRxChar = g_ulRxChar +
                ((ulRcvIoValue) ? (1 << (g_currentRxState-1)) : 0);

            //
            // Set next state
            //
            g_currentRxState++;

            break;
        }
        case SWUART_DATA_5:
        case SWUART_DATA_6:
        case SWUART_DATA_7:
        case SWUART_DATA_8:
        {
            //
            // Update curretn rx char
            //
            g_ulRxChar = g_ulRxChar +
                ((ulRcvIoValue) ? (1<<(g_currentRxState-1)) : 0);

            //
            //Check for end of data
            //
            if( g_sSoftwareUART.iDataBits == g_currentRxState)
            {
                //
                //This was last data bit
                //
                g_currentRxState = SWUART_STOP_1;

                break;
            }

            //
            // Set next state
            //
            g_currentRxState++;

            break;
        }
        case SWUART_STOP_1:
        {
            //
            // Check for valid STOP bit
            //
            if(ulRcvIoValue)
            {
                //
                // STOP bit valid
                //
                if(g_sSoftwareUART.StopBits == SWUART_STOP_ONE)
                {
                    //
                    // Reception complete so add character to buffer.
                    //
                    RxBufferAdd(g_ulRxChar);

                    //
                    // Disable the receiver timer
                    //
                    HWREG(TIMER0_BASE + TIMER_O_CTL) &=
                        ~(TIMER_B & (TIMER_CTL_TAEN | TIMER_CTL_TBEN));

                    //
                    // Disable the Start Detect interrupt
                    //
                    HWREG(SWU_GPIO_PORT_BASE + GPIO_O_IM)|= SWU_GPIO_RX;
                    HWREG(SWU_GPIO_PORT_BASE + GPIO_O_ICR) = SWU_GPIO_RX;

                    //
                    // Set state for next character
                    //
                    g_currentRxState = SWUART_START_BIT;

                    break;
                }
                else
                {
                    //
                    // Set next state
                    //
                    g_currentRxState++;
                    break;
                }
            }
            //
            // Framing error
            //
            break;
        }
        case SWUART_STOP_2:
        {
            if( ulRcvIoValue )
            {
                //
                // STOP bit valid, so set state for next character
                //
                g_currentRxState = SWUART_START_BIT;

                break;
            }

            //
            // Framing error
            //
            break;
        }
        default:
        {
            break;
        }
    }
}

//*****************************************************************************
//
//! Populates a function call for the receive interrupt
//!
//! \param pfnHandler function call.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTReceiveIntRegister(void (*pfnHandler)(void))
{
    g_pfnHandler = pfnHandler;
}

//*****************************************************************************
//
//! Sets the configuration of a UART.
//!
//! \param ulBase is the base address of the UART port.
//! \param ulUARTClk is the rate of the clock supplied to the UART module.
//! \param ulBaud is the desired baud rate.
//! \param ulConfig is the data format for the port (number of data bits,
//! number of stop bits, and parity).
//!
//! This function will configure the UART for operation in the specified data
//! format.  The baud rate is provided in the \e ulBaud parameter and the data
//! format in the \e ulConfig parameter.
//!
//! The \e ulConfig parameter is the logical OR of three values: the number of
//! data bits, the number of stop bits, and the parity.  \b UART_CONFIG_WLEN_8,
//! \b UART_CONFIG_WLEN_7, \b UART_CONFIG_WLEN_6, and \b UART_CONFIG_WLEN_5
//! select from eight to five data bits per byte (respectively).
//! \b UART_CONFIG_STOP_ONE and \b UART_CONFIG_STOP_TWO select one or two stop
//! bits (respectively).  \b UART_CONFIG_PAR_NONE, \b UART_CONFIG_PAR_EVEN,
//! \b UART_CONFIG_PAR_ODD, \b UART_CONFIG_PAR_ONE, and \b UART_CONFIG_PAR_ZERO
//! select the parity mode (no parity bit, even parity bit, odd parity bit,
//! parity bit always one, and parity bit always zero, respectively).
//!
//! The peripheral clock will be the same as the processor clock.  This will be
//! the value returned by SysCtlClockGet(), or it can be explicitly hard coded
//! if it is constant and known (to save the code/execution overhead of a call
//! to SysCtlClockGet()).
//!
//! This function replaces the original UARTConfigSet() API and performs the
//! same actions.  A macro is provided in <tt>uart.h</tt> to map the original
//! API to this API.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTConfigSetExpClk(unsigned long ulBase, unsigned long ulUARTClk,
                       unsigned long ulBaud, unsigned long ulConfig)
{
    //
    // Check the arguments.
    //
    ASSERT(ulUARTClk >= (ulBaud * 16));
    ASSERT(ulBaud != 0);

    //
    // Set the baud rate
    //
    g_sSoftwareUART.ulBaudRate = ulBaud;

    //
    // Set the number of data bits
    //
    g_sSoftwareUART.iDataBits = ( (ulConfig & 0x00000060) >> 5) + 5;

    //
    // Set the number of stop bits
    //
    g_sSoftwareUART.StopBits =
        (ulConfig & 0x00000008) ? SWUART_STOP_TWO : SWUART_STOP_ONE;

    //
    // Initialse the timers used for TX and RX
    //
    SW_UARTInitTxRxTimers(ulUARTClk);

    //
    // Initialize the Transmit Buffer
    //
    g_pulTxBufferHead = g_pulTxBufferTail = g_ulTxBuffer;
    g_ulTxBufferCount = 0;

    //
    // Initialize the Receive Buffer
    //
    g_pulRxBufferHead = g_pulRxBufferTail = g_ulRxBuffer;
    g_ulRxBufferCount = 0;

    //
    // Initialize GPIO
    //
    SW_UARTInitGPIO();
}

//*****************************************************************************
//
//! Gets the current configuration of a UART.
//!
//! \param ulBase is the base address of the UART port.
//! \param ulUARTClk is the rate of the clock supplied to the UART module.
//! \param pulBaud is a pointer to storage for the baud rate.
//! \param pulConfig is a pointer to storage for the data format.
//!
//! The baud rate and data format for the UART is determined, given an
//! explicitly provided peripheral clock (hence the ExpClk suffix).  The
//! returned baud rate is the actual baud rate; it may not be the exact baud
//! rate requested or an ``official'' baud rate.  The data format returned in
//! \e pulConfig is enumerated the same as the \e ulConfig parameter of
//! UARTConfigSetExpClk().
//!
//! The peripheral clock will be the same as the processor clock.  This will be
//! the value returned by SysCtlClockGet(), or it can be explicitly hard coded
//! if it is constant and known (to save the code/execution overhead of a call
//! to SysCtlClockGet()).
//!
//! This function replaces the original UARTConfigGet() API and performs the
//! same actions.  A macro is provided in <tt>uart.h</tt> to map the original
//! API to this API.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTConfigGetExpClk(unsigned long ulBase, unsigned long ulUARTClk,
                    unsigned long *pulBaud, unsigned long *pulConfig)
{
    //
    // Not implemented
    //
}

//*****************************************************************************
//
//! Enables transmitting and receiving.
//!
//! \param ulBase is the base address of the UART port.
//!
//! Sets the UARTEN, TXE, and RXE bits, and enables the transmit and receive
//! FIFOs.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTEnable(unsigned long ulBase)
{
    //
    // Setup the interrupts for the timer timeouts.
    //
    IntEnable(INT_TIMER0A);
    IntEnable(INT_TIMER0B);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);

    //
    // Enable timer A only.
    // Timer B is enabled by the GPIO 'Start Detect' interrupt
    //
    TimerEnable(TIMER0_BASE, TIMER_A);

    //
    // Enable the RX Start Edge detect interrupt
    //
    GPIOPinIntClear(SWU_GPIO_PORT_BASE,0xFF);
    IntEnable(SWU_GPIO_INT_BASE);
}

//*****************************************************************************
//
//! Disables transmitting and receiving.
//!
//! \param ulBase is the base address of the UART port.
//!
//! Clears the UARTEN, TXE, and RXE bits, then waits for the end of
//! transmission of the current character, and flushes the transmit FIFO.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTDisable(unsigned long ulBase)
{
    //
    // Disable the Start Detect interrupt
    //
    GPIOPinIntDisable(SWU_GPIO_PORT_BASE, SWU_GPIO_RX);
    GPIOPinIntClear(SWU_GPIO_PORT_BASE, SWU_GPIO_RX);

    //
    // Disable timers for RX and TX
    //
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerDisable(TIMER0_BASE, TIMER_B);
}

//*****************************************************************************
//
//! Determines if there are any characters in the receive FIFO.
//!
//! \param ulBase is the base address of the UART port.
//!
//! This function returns a flag indicating whether or not there is data
//! available in the receive FIFO.
//!
//! \return Returns \b true if there is data in the receive FIFO, and \b false
//! if there is no data in the receive FIFO.
//
//*****************************************************************************
tBoolean
SW_UARTCharsAvail(unsigned long ulBase)
{
    return((g_ulRxBufferCount) ? true : false);
}

//*****************************************************************************
//
//! Determines if there is any space in the transmit FIFO.
//!
//! \param ulBase is the base address of the UART port.
//!
//! This function returns a flag indicating whether or not there is space
//! available in the transmit FIFO.
//!
//! \return Returns \b true if there is space available in the transmit FIFO,
//! and \b false if there is no space available in the transmit FIFO.
//
//*****************************************************************************
tBoolean
SW_UARTSpaceAvail(unsigned long ulBase)
{
    //
    // If the \b g_ulTxBufferCount is less than \b TX_BUFFER_SIZEspace is avail
    //
    return((g_ulTxBufferCount == TX_BUFFER_SIZE ) ? false : true);
}

//*****************************************************************************
//
//! Receives a character from the specified port.
//!
//! \param ulBase is the base address of the UART port.
//!
//! Gets a character from the receive FIFO for the specified port.
//!
//! This function replaces the original UARTCharNonBlockingGet() API and
//! performs the same actions.  A macro is provided in <tt>uart.h</tt> to map
//! the original API to this API.
//!
//! \return Returns the character read from the specified port, cast as a
//! \e long.  A \b -1 will be returned if there are no characters present in
//! the receive buffer.  //
//*****************************************************************************
long
SW_UARTCharGetNonBlocking(unsigned long ulBase)
{
    //
    // If there are any characters available.
    //
    return(RxBufferRemove());
}

//*****************************************************************************
//
//! Waits for a character from the specified port.
//!
//! \param ulBase is the base address of the UART port.
//!
//! Gets a character from the receive FIFO for the specified port.  If there
//! are no characters available, this function will wait until a character is
//! received before returning.
//!
//! \return Returns the character read from the specified port, cast as an
//! \e int.
//
//*****************************************************************************
long
SW_UARTCharGet(unsigned long ulBase)
{
    //
    // Check if there are any characters available
    //
    while(!SW_UARTCharsAvail(0))
    {
    }

    //
    // Return character
    //
    return(RxBufferRemove());
}

//*****************************************************************************
//
//! Sends a character to the specified port.
//!
//! \param ulBase is the base address of the UART port.
//! \param ucData is the character to be transmitted.
//!
//! Writes the character \e ucData to the transmit FIFO for the specified port.
//! This function does not block, so if there is no space available, then a
//! \b false is returned, and the application will have to retry the function
//! later.
//!
//! This function replaces the original UARTCharNonBlockingPut() API and
//! performs the same actions.  A macro is provided in <tt>uart.h</tt> to map
//! the original API to this API.
//!
//! \return Returns \b true if the character was successfully placed in the
//! transmit FIFO, and \b false if there was no space available in the transmit
//! FIFO.
//
//*****************************************************************************
tBoolean
SW_UARTCharPutNonBlocking(unsigned long ulBase, unsigned long ulData)
{
    //
    // Attempt to add data to TX buffer
    //
    return(TxBufferAdd(ulData));
}

//*****************************************************************************
//
//! Waits to send a character from the specified port.
//!
//! \param ulBase is the base address of the UART port.
//! \param ucData is the character to be transmitted.
//!
//! Sends the character \e ucData to the transmit FIFO for the specified port.
//! If there is no space available in the transmit FIFO, this function will
//! wait until there is space available before returning.
//!
//! \return None.
//
//*****************************************************************************
void
SW_UARTCharPut(unsigned long ulBase, unsigned char ucData)
{
    //
    // Wait until space is available.
    //
    while(!(SW_UARTCharPutNonBlocking(0,ucData)))
    {
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

