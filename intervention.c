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
#define CORE_TICK_RATE	        (SYS_FREQ/2/TOGGLES_PER_SEC)

// IOPORT bit masks can be found in ports.h
#define CONFIG          (CN_ON)
#define PINS            (CN16_ENABLE)
#define PULLUPS         (CN15_PULLUP_ENABLE | CN16_PULLUP_ENABLE)
#define INTERRUPT       (CHANGE_INT_ON | CHANGE_INT_PRI_2)

//0 -> Bottom, 1 -> Middle, 2 -> Top
//0 -> Red,    1 -> Yellow, 2 -> Green
unsigned int button_states[3]; //0: pressed 1: unpressed
unsigned int light_states[3]; //0: on 1: off
unsigned int flash_all; //1: do 0: don't
unsigned int green_mode; //1: green light condition 0: not

void init()
{
    //Set up button and light states
    int i;
    for (i = 0; i < 3; i++)
    {
        button_states[i] = 1;
        light_states[i] = 0;
    }
    flash_all = 1; //Start flashing
    green_mode = 0; // light isn't green

    // Make debugger do
    DBINIT();

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

void startFlashing()
{
    DBPRINTF("Start flashing!\n"); //DBPRINTF should be working
    green_mode = 0;
    flash_all = 1;
    OpenCoreTimer(CORE_TICK_RATE);
    mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0));
}

void stopFlashing()
{
    DBPRINTF("Stop flashing!\n");
    flash_all = 0;
}

void greenModeOn() {
    green_mode = 1;
    flash_all = 0;

    // turn green LED ON
    PORTSetBits(IOPORT_D, BIT_2);
    light_states[0] = 0;

    // ensure yellow LED OFF
    PORTClearBits(IOPORT_D, BIT_1);
    light_states[1] = 1;

    // ensure red LED OFF
    PORTClearBits(IOPORT_D, BIT_0);
    light_states[2] = 1;

    CloseCoreTimer();

    DBPRINTF("Green light mode on\n");
}

void greenModeOff() {
    green_mode = 0;

    // turn green LED OFF
    PORTClearBits(IOPORT_D, BIT_2);
    light_states[0] = 1;

    // ensure yellow LED OFF
    PORTClearBits(IOPORT_D, BIT_1);
    light_states[1] = 1;

    // ensure red LED OFF
    PORTClearBits(IOPORT_D, BIT_0);
    light_states[2] = 1;

    DBPRINTF("Green light mode off\n");
}

void watchButtons()
{
    //Polling for button change
    while(1)
    {
        // BUTTON C
        if(PORTDbits.RD6 == 0) // 0 = switch is pressed
        {
            if(button_states[2] == 1) //State just changed
            {
                button_states[2] = 0;
                if (flash_all)
                {
                    stopFlashing();
                }
                else
                {
                    startFlashing();
                }
            }
        }
        else // 1 = switch is not pressed
        {
            if(button_states[2] == 0) //State just changed
            {
                button_states[2] = 1;
            }
        }

        // BUTTON A
        if(PORTDbits.RD13 == 0) // 0 = switch is pressed
        {
            if(button_states[0] == 1) //State just changed
            {
                button_states[0] = 0;
                if (flash_all) // lights are flashing, stop flashing and set green condition
                {
                   stopFlashing();
                   greenModeOn();
                   
                }
                if (green_mode) {   // green condition, turn it off
                    greenModeOff();
                }
                else
                {
                    greenModeOn(); // start green condition
                }
            }
        }
        else // 1 = switch is not pressed
        {
            if(button_states[0] == 0) //State just changed
            {
                button_states[0] = 1;
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
    DBPRINTF("Interrupt Do\n");
    if (flash_all)
    {
        // Start next iteration
        UpdateCoreTimer(CORE_TICK_RATE);
    }

    if (flash_all || light_states[0])
    {
        DBPRINTF("Toggling lights from %d!\n", light_states[0]);
        // Toggle the LEDs
        mPORTDToggleBits(BIT_0 | BIT_1 | BIT_2);

        int i;
        if (light_states[0]) //Assuming that if A is on, B and C are too
        {
            for (i = 0; i < 3; i++)
            {
                light_states[i] = 0;
            }
        }
        else
        {
            for (i = 0; i < 3; i++)
            {
                light_states[i] = 1;
            }
        }
    }

    //Clear interrupt flag
    mCTClearIntFlag();
}
