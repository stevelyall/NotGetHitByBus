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
#define TOGGLES_PER_SEC			7
#define FLASH_RATE	        (SYS_FREQ/TOGGLES_PER_SEC)

// IOPORT bit masks can be found in ports.h
#define CONFIG          (CN_ON)
#define PINS            (CN16_ENABLE)
#define PULLUPS         (CN15_PULLUP_ENABLE | CN16_PULLUP_ENABLE)
#define INTERRUPT       (CHANGE_INT_ON | CHANGE_INT_PRI_2)

//0 -> Bottom, 1 -> Middle, 2 -> Top
//0 -> Red,    1 -> Yellow, 2 -> Green
unsigned int button_states[3]; //0: pressed 1: unpressed
unsigned int button_bounce[3];
unsigned int light_states[3]; //0: on 1: off
unsigned int flash_all; //1: do 0: don't
unsigned int green_mode; //1: green light condition 0: not

// 0: none
// 1: interrupt handler for flashing mode
// 2: interrupt handler for green  -> yellow
// 3: interrupt handler for yellow -> red
// 4: interrupt handler for red    -> green
unsigned int interrupt_mode; 

void init()
{
    //Set up button and light states
    int i;
    for (i = 0; i < 3; i++)
    {
        button_states[i] = 1;
        button_bounce[i] = 0;
        light_states[i] = 0;
    }

    flash_all = 1; //Start flashing
    interrupt_mode = 1; // tell interrupt handler to use flashing case
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
    OpenCoreTimer(FLASH_RATE);

    // set up the core timer interrupt with a prioirty of 2 and zero sub-priority
    mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0));

    // enable device multi-vector interrupts
    INTEnableSystemMultiVectoredInt();
}

void startFlashing()
{
    DBPRINTF("Start flashing!\n"); //DBPRINTF should be working
    // set flashing mode and interrupt handler for do flashing
    flash_all = 1;
    interrupt_mode = 1;
    greenModeOff(); // turn green light mode off
    OpenCoreTimer(FLASH_RATE);
    mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0));
}

void stopFlashing()
{
    DBPRINTF("Stop flashing!\n");
    flash_all = 0;

     //Assuming that if A is on, B and C are too
     if (light_states[0])
     {
         int i;
         for (i = 0; i < 3; i++) //all lights set off
          {
            light_states[i] = 0;
          }
          
          // turn all LEDs OFF
          PORTClearBits(IOPORT_D, BIT_0);
          PORTClearBits(IOPORT_D, BIT_1);
          PORTClearBits(IOPORT_D, BIT_2);
     }
}


void greenModeOn() {
    CloseCoreTimer();

    green_mode = 1;
    flash_all = 0;
    interrupt_mode = 0; // interrupts shouldn't do anything

    // turn green LED ON
    PORTSetBits(IOPORT_D, BIT_2);
    light_states[0] = 1;

    // ensure yellow LED OFF
    PORTClearBits(IOPORT_D, BIT_1);
    light_states[1] = 0;

    // ensure red LED OFF
    PORTClearBits(IOPORT_D, BIT_0);
    light_states[2] = 0;

    DBPRINTF("Green light mode on\n");
}

void greenModeOff() {
    green_mode = 0;

    int i;
         for (i = 0; i < 3; i++) //all lights set off
          {
            light_states[i] = 0;
          }

          // turn all LEDs OFF
          PORTClearBits(IOPORT_D, BIT_0);
          PORTClearBits(IOPORT_D, BIT_1);
          PORTClearBits(IOPORT_D, BIT_2);

    DBPRINTF("Green light mode off\n");
}

// turns green off and yellow on
void chgGreenToYellow() {
    // turn GREEN LED OFF
    PORTClearBits(IOPORT_D, BIT_2);
    light_states[0] = 0;
    // turn YELLOW LED ON
    PORTSetBits(IOPORT_D, BIT_1);
    light_states[1] = 1;
}

// turns yellow off and red on
void chgYellowToRed() {
    // turn yellow LED OFF
    PORTClearBits(IOPORT_D, BIT_1);
    light_states[1] = 0;
    // turn red LED ON
    PORTSetBits(IOPORT_D, BIT_0);
    light_states[2] = 1;
}

void watchButtons()
{
    //Polling for button change
    while(1)
    {
        int i;

        // BUTTON A for green mode
        if(PORTDbits.RD13 == 0) // 0 = switch is pressed
        {
            if(button_states[0] == 1 && button_bounce[0] == 0) //State just changed
            {
                button_states[0] = 0;
                button_bounce[0] = 300000;
                DBPRINTF("Green button pressed\n");
                if (flash_all) // lights are flashing, stop flashing and set green condition
                {
                    greenModeOn();
                    //stopFlashing();

                }
                else if (green_mode) {   // green condition, turn it off
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
                DBPRINTF("BUTTON UNPRESSED YO\n");
                button_states[0] = 1;
            }
        }

        // BUTTON B for yellow -> red -> green
        if(PORTDbits.RD7 == 0) // 0 = switch is pressed
        {
            if(button_states[1] == 1 && button_bounce[1] == 0) //State just changed
            {
                DBPRINTF("BUTTON B fa: %d|gm: %d\n", flash_all, green_mode);
                button_states[1] = 0;
                button_bounce[1] = 300000;
                
                if (flash_all)
                {
                    DBPUTS("No crossing while we're testing, Billy\n");
                    // don't do anything if lights are flashing
                }
                else if (green_mode) // begin change cycle from green mode
                {
                    DBPUTS("Change from green to yellow\n");
                    //greenModeOff();
                    green_mode = 0;

                    interrupt_mode = 2; // interrupt handler for green to yellow
                    //set core timer
                    OpenCoreTimer(FLASH_RATE*2);
                    mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0));

                }
                //else // not flashing, not green mode
                //{
                    //DBPUTS("Not green, WTF flash mode\n");
                    //startFlashing();
                //}
            }
        }
        else // 1 = switch is not pressed
        {
            if(button_states[1] == 0) //State just changed
            {
                button_states[1] = 1;
            }
        }

        // BUTTON C for flashing
        if(PORTDbits.RD6 == 0) // 0 = switch is pressed
        {
            if(button_states[2] == 1 && button_bounce[2] == 0) //State just changed
            {
                button_states[2] = 0;
                button_bounce[2] = 300000;
                
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

        for (i = 0; i < 3; i++)
            if (button_bounce[i] > 0)
                button_bounce[i]--;

       // if (button_bounce[0] != 0 && button_bounce[0] % 10 == 0)
            //DBPRINTF("is|%d\n", button_bounce[0]);
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
    //DBPRINTF("Interrupt Do\n");

    switch (interrupt_mode) {

        // 0: null interrupt handler
        case 0 : 
            DBPRINTF("...nothing\n");
            break;
            
        // 1: interrupt handler for flashing mode
        case 1 :

           // DBPRINTF("...for flashing mode\n");
            if (flash_all)
            {
                // Start next iteration
                UpdateCoreTimer(FLASH_RATE);
            }

            if (flash_all || light_states[0])
            {
                DBPRINTF("Toggling lights from %d!\n", light_states[0]);
                // Toggle the LEDs
                mPORTDToggleBits(BIT_0 | BIT_1 | BIT_2);

                int i;
                if (light_states[0]) //Assuming that if A is on, B and C are too
                    //Line for each state, off if on, on if off
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
            //mCTClearIntFlag();
            break;


        // 2: interrupt handler for green -> yellow
        case 2 :
            DBPRINTF("... going from green to yellow\n");
            chgGreenToYellow();
            
            DBPRINTF("Gone yellow yo\n");

            //CloseCoreTimer();
            interrupt_mode = 3;
            //set core timer for next mode
            UpdateCoreTimer(FLASH_RATE*5);
            //mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0));
            


            
            break;

        // 3: interrupt handler for yellow -> red
        case 3 :
            DBPRINTF("...going from yellow to red\n");
            chgYellowToRed();
            
            DBPRINTF("Gone red yo\n");
            interrupt_mode = 4;
            //set core timer for next mode
            UpdateCoreTimer(FLASH_RATE*10);
            
            break;

        // 4: interrupt handler for red -> green
        case 4 :
            DBPRINTF("...going from red to green\n");
            greenModeOn();
            DBPRINTF("Gone green yo\n");
     }

    mCTClearIntFlag();
}
