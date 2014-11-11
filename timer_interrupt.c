/***********************************************************************************
				
				PIC32MX Starter Kit Example Code - Timer Interrupt

 ***********************************************************************************
 * FileName:        timer_interrupt.c
 * Dependencies:    None
 * Company:         Microchip Technology, Inc.
 *
 * Copyright (c) 2008 Microchip Technology, Inc.
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC32 Microcontroller is intended
 * and supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC32 Microcontroller products.
 * The software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ***************************************************************************
 * Description:
 *			The example code demonstrates using PIC32 core timer, core timer 
 *          interrupt and interrupt service routine to blink LED1 10 times a second.
 *
 * Additional Info: 
 *			Refer to readme.txt
 *
 * Platforms:
 *			PIC32MX Starter Kit DM320001
 *			PIC32MX USB Starter Kit DM320003
 *
 * Tools:
 *			1. MPLAB IDE 8.20 or higher
 *			2. MPLAB C Compiler for PIC32 v1.04 or higher
 *			3. General Purpose Starter Kit DM320001 or USB Starter board DM320003
 *			4. USB Cable
 *
 * Debug Print Library: SUPPORTED IN DEBUG MODE ONLY
 *			The Debug Print Library adds user debugging input/output capability
 *          to the MPLAB IDE Starter Kit console window using DBPRINTF, DPUTS,
 *          DBGETC and DBGETS.
 *
 *          To use the Debug Print Library with the PIC32 Starter Kit, do the following:
 *			1. Add '#include "db_utils.h"' to the source code.
 *			   Note: you may need to copy db_utils.h into your working directory or
 *			   specify	a path to its location.
 *			   It is located in "<install dir>\PIC32 Starter Kits\Microchip\Include".
 *			2. Add 'db_utils.a' to the library branch of MPLAB IDE project tree.
 *			   It is located in "<install dir>\PIC32 Starter Kits\Microchip\Common"
 *			3. Add "PIC32_STARTER_KIT" macro definition to the project build options
 *			   for the C compiler.
 *			4. Be sure to call DBINIT() first before any other Debug Print Library macros.
 *
 * Starter Board Resources:
 *			LED1 (RED)		= PORTD.RD0
 *			LED2 (YELLOW)	= PORTD.RD1
 *			LED3 (GREEN)	= PORTD.RD2
 *			SW1				= PORTD.RD6  (change notice pin CN15)
 *			SW2				= PORTD.RD7	 (change notice pin CN16)
 *			SW3				= PORTD.RD13 (change notice pin CN19)
 *			JTAG.TMS		= PORTA.RA0  (used by debugger)
 *			JTAG.TCK		= PORTA.RA1  (used by debugger)
 *			JTAG.TDO		= PORTA.RA5  (used by debugger)
 *			JTAG.TDI		= PORTA.RA4  (used by debugger)
 *			PGC2			= PORTB.RB6	 (used by debugger)
 *			PGD2			= PORTB.RB7  (used by debugger)
 *
 * Starter Board Notes:
 *			1.	There are no pullup resistors on SW1, SW2 or SW3.
 *				To enable the PORTD pullups for SW1, SW2 and SW3, use the following:
 *				CNPUE = (CN15_PULLUP_ENABLE | CN16_PULLUP_ENABLE | CN19_PULLUP_ENABLE);
 *			2.  Do not disable the PIC32MX JTAG.  This will prevent the PIC32MX Starter Kit
 *				debugger(PIC18F4550) from communicating with the PIC32MX device.
 *			3.  Do not configure the SYSTEM CLOCK to operate faster than 80MHz.
 *************************************************************************************
 * Change History:
 * ID			Date            Changes
 * C11842		12/01/2008		Modified for 80MHz operation, Updated comments.
 *************************************************************************************/

// Adds support for PIC32 Peripheral library functions and macros
#include <plib.h>

// Configuration Bits
#pragma config FNOSC    = PRIPLL        // Oscillator Selection
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider (PIC32 Starter Kit: use divide by 2 only)
#pragma config FPLLMUL  = MUL_20        // PLL Multiplier
#pragma config FPLLODIV = DIV_1         // PLL Output Divider
#pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
#pragma config FWDTEN   = OFF           // Watchdog Timer
#pragma config WDTPS    = PS1           // Watchdog Timer Postscale
#pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
#pragma config OSCIOFNC = OFF           // CLKO Enable
#pragma config POSCMOD  = XT            // Primary Oscillator
#pragma config IESO     = OFF           // Internal/External Switch-over
#pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable
#pragma config CP       = OFF           // Code Protect
#pragma config BWP      = OFF           // Boot Flash Write Protect
#pragma config PWP      = OFF           // Program Flash Write Protect
#pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select
#pragma config DEBUG    = OFF           // Debugger Disabled for Starter Kit

// Let compile time pre-processor calculate the CORE_TICK_PERIOD
#define SYS_FREQ 				(80000000)
#define TOGGLES_PER_SEC			10
#define CORE_TICK_RATE	        (SYS_FREQ/2/TOGGLES_PER_SEC)

// timer_interrupt application
int main(void)
{
    // Configure the device for maximum performance, but do not change the PBDIV clock divisor.
	// Given the options, this function will change the program Flash wait states,
	// RAM wait state and enable prefetch cache, but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above.
	SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    //Initialize the DB_UTILS IO channel
	DBINIT();
	
	// Display a message
	DBPRINTF("Welcome to the PIC32 Timer Interrupt example. \n");
	DBPRINTF("The build date and time is ... (" __DATE__ "," __TIME__ ") \n");
	DBPRINTF("This example demonstrates how to configure the PIC32MX Core Timer to generate an interrupt %d times a second. \n", TOGGLES_PER_SEC);

    // configure the core timer roll-over rate (100msec)
    OpenCoreTimer(CORE_TICK_RATE);

    // set up the core timer interrupt with a prioirty of 2 and zero sub-priority
    mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0));

    // enable device multi-vector interrupts
    INTEnableSystemMultiVectoredInt();

	// configure PORTD.RD1 = output
    mPORTDSetPinsDigitalOut(BIT_0);

    while(1);

}

void __ISR(_CORE_TIMER_VECTOR, ipl2) CoreTimerHandler(void)
{
    // .. things to do

	// .. Toggle the LED
    mPORTDToggleBits(BIT_0);

    // update the period
    UpdateCoreTimer(CORE_TICK_RATE);

    // clear the interrupt flag
    mCTClearIntFlag();
}
