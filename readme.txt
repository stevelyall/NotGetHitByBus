PIC32 Starter Kit: Core Timer demo
---------------------------------------------------------
 
Required Development Resources:
-------------------------------
a. PIC32 Starter Kit (DM320001)
b. MPLAB IDE.
c. C Compiler.


Compiling and Downloading the Demo:
-----------------------------------
1. Load the demo code into MPLAB by double clicking the *.mcp project file.
2. Connect the mini-B debugger port on-board the Starter board to an USB port on the development computer using the USB cable provided in the kit.
3. Choose the PIC32 Starter Kit debugger tool in MPLAB IDE by selecting Debugger>Select Tool and then click on PIC32 Starter Kit.
4. Build the project by selecting Project>Build All.
5. Download your code into the evaluation board microcontroller by selecting Debugger>Programming>Program All Memories.
6. Run the demo code by selecting Debugger>Run.

Running the Demo:
-----------------
This demonstration uses the PIC32 Peripheral Library macros to configure the PIC32 32-bit core timer to generate interrupts and illustrates the syntax for declaring an interrupt ISR.

The program generates a core timer interrupt once every 10 seconds. The core timer ISR updates the core timer compare register and toggles LED1 (PORTD.RD0).

Also demonstrated is the use of the Debug Print Library macros to send user messages to the MPLAB IDE Starter Kit console window.
