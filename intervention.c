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

//  The following is used by the main application
#define SYS_FREQ		(80000000)
#define TOGGLES_PER_SEC			5
#define CORE_TICK_RATE	        64000000//(SYS_FREQ/2/TOGGLES_PER_SEC) MITCH

// IOPORT bit masks can be found in ports.h
#define CONFIG          (CN_ON)
#define PINS            (CN16_ENABLE)
#define PULLUPS         (CN15_PULLUP_ENABLE | CN16_PULLUP_ENABLE)
#define INTERRUPT       (CHANGE_INT_ON | CHANGE_INT_PRI_2)

unsigned int button_c_state = 1; //0: off, 1: on

void init()
{
    // Make debugger do
    DBINIT();
    DBPRINTF("CORE_TICK_RATE:%d\n", CORE_TICK_RATE); //By the way, this _is_ working for me

    // Configure the device for maximum performance, but do not change the PBDIV clock divisor.
    // Given the options, this function will change the program Flash wait states,
    // RAM wait state and enable prefetch cache, but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above.
    SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // configure IOPORTS PORTD.RD0, RD1, RD2 as outputs (LEDs)
    // could also use mPORTDSetPinsDigitalOut(BIT_6 | BIT_7);
    PORTSetPinsDigitalOut(IOPORT_D, BIT_0 | BIT_1 | BIT_2);

    // initialize the port pins states = output low
    PORTClearBits(IOPORT_D, BIT_0 | BIT_1 | BIT_2);

    // PORTD.RD6, RD7, 13 as inputs (these are the switches)
    // could also use mPORTDSetPinsDigitalIn(BIT_6 | BIT_7);
    PORTSetPinsDigitalIn(IOPORT_D, BIT_6 | BIT_7 | BIT_13);

    // configure the core timer roll-over rate (100msec)
    OpenCoreTimer(CORE_TICK_RATE);

    // set up the core timer interrupt with a prioirty of 2 and zero sub-priority
    mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0));

    // enable device multi-vector interrupts
    INTEnableSystemMultiVectoredInt();
}

void watchButtons()
{
    //Polling for button change
    while(1)
    {
        // BUTTON C
        if(PORTDbits.RD6 == 0) // 0 = switch is pressed
        {
            if(button_c_state == 1) //State just changed
            {
                DBPRINTF("BUTTON C has been pressed. \n");
                button_c_state = 0;
            }
        }
        else // 1 = switch is not pressed
        {
            if(button_c_state == 0) //State just changed
            {
                DBPRINTF("BUTTON C has been released. \n");
                button_c_state = 1;
                OpenCoreTimer(CORE_TICK_RATE);
            }
        }
    };
}

//  port_io application code
int main(void)
{
    init();
    watchButtons();
}

/*
* Makes all three lights flash
******************************************************************************
*	Change Notice Interrupt Service Routine
*
*   Note: Switch debouncing is not performed.
*   Code comes here if SW2 (CN16) PORTD.RD7 is pressed or released.
*   The user must read the IOPORT to clear the IO pin change notice mismatch
*	condition first, then clear the change notice interrupt flag.
******************************************************************************/
void __ISR(_CORE_TIMER_VECTOR, ipl2) CoreTimerHandler(void)
{
    DBPRINTF("Toggling?\n");
    // Toggle the LEDs
    mPORTDToggleBits(BIT_0 | BIT_1 | BIT_2);

    // update the period
    if (button_c_state == 1)
    {
        UpdateCoreTimer(CORE_TICK_RATE);
    }

    // clear the interrupt flag
    mCTClearIntFlag();
}
