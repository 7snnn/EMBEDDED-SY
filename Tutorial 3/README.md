Tutorial 3 – Main Task: Serial Communication & Command Interface
Module: UFMFN3-30-2 Embedded Systems
Hardware: STM32 Nucleo-F439ZI

1. Objective
The primary aim of this task was to establish robust, two-way serial communication between the microcontroller and a host PC. By migrating from simple printf statements to the UnbufferedSerial API, the objective was to build a "Smart Home Serial Interface" that processes asynchronous keyboard inputs to simulate sensor states (Gas, Temperature) and toggle system modes (Reset, Monitor).

2. Logic and Implementation
This system moves away from basic physical buttons and introduces terminal-based command parsing.

Unbuffered Serial: The UnbufferedSerial class is initialized on USBTX and USBRX with a baud rate of 115200. This provides low-level, non-blocking byte transmission, which is standard for professional embedded systems.

Non-Blocking Polling: Instead of pausing the system to wait for a user to type, the code uses if (pc.readable()). This ensures the main while(true) loop continues running at full speed until a byte actually arrives in the serial buffer.

Byte-Level Processing: When a keystroke is detected, pc.read() pulls exactly one byte (character) into the inputChar variable.

State Toggles & Visual Feedback: A switch/if-logic structure checks the character (e.g., '1'). If matched, it toggles a boolean state (gasAlarm = !gasAlarm). The Blue LED (ledBlue) is utilized as a physical "signature input feedback light" to visually confirm that the microcontroller successfully received the byte.

3. Core Task 3 Code Structure
C++
/* * Module: UFMFN3-30-2 Embedded Systems
 * Task: Tutorial 3 - Main Task (Serial Interface)
 */
#include "mbed.h"

// 1. HARDWARE SETUP
// Using 115200 baud rate as required by the lab sheet
UnbufferedSerial pc(USBTX, USBRX, 115200);

DigitalOut ledGreen(LED1);
DigitalOut ledBlue(LED2);  // Signature input feedback light
DigitalOut ledRed(LED3);   // Alarm indicator

// 2. SYSTEM STATE VARIABLES
bool gasAlarm = false;
bool tempAlarm = false;
bool continuousMonitoring = false;

Timer statusTimer; // Tracks the 2-second interval

int main() {
    statusTimer.start();

    // Initial Startup Message
    pc.write("Smart Home Serial Interface Ready\r\n", 36);
    pc.write("Commands: 1 (Gas), 4 (Temp), 5 (Reset), 6 (Monitor)\r\n", 53);

    while (true) {
        // --- PART A: KEYBOARD INPUT HANDLING ---
        // Only trigger the read function if data is actually in the buffer
        if (pc.readable()) {
            char inputChar = '\0';
            pc.read(&inputChar, 1); // Read exactly one byte from the PC

            // THE BLUE FLASH: Visual confirmation of keypress
            ledBlue = 1;

            // KEY 1: Toggle Gas Simulation
            if (inputChar == '1') {
                gasAlarm = !gasAlarm;
                
                if (gasAlarm) {
                    pc.write(" WARNING: GAS DETECTED\r\n", 25);
                } else {
                    pc.write("Gas Simulation: CLEAR\r\n", 23);
                }
            }
            
            // Note: The Blue LED must be turned off later in the loop 
            // or via a timer to complete the "flash" effect.
            
            // ... Logic continues for keys 4, 5, and 6 ...
        }
    }
}
4. Technical Reflection (Errors & Challenges)
Transitioning to direct serial buffer management introduced several specific communication challenges:

Baud Rate Mismatches: Initially, the serial monitor displayed unreadable "garbage" characters. I had to ensure that the terminal software (e.g., TeraTerm, Putty, or Mbed Studio) was explicitly configured to match the 115200 baud rate defined in the UnbufferedSerial setup.

Characters vs. Integers: A critical logic error occurred when I tried to evaluate the input using if (inputChar == 1). The program failed to recognize the keystroke because the terminal sends ASCII characters. I had to correct this to if (inputChar == '1') to match the actual byte value being transmitted.

Manual String Lengths: Moving away from printf meant I had to use pc.write(), which requires manually counting the exact number of characters in the string (including the carriage return \r and newline \n). Providing an incorrect length resulted in truncated messages or memory overflow artifacts on the screen.

Blocking Read Trap: If I had called pc.read() without wrapping it in the if (pc.readable()) condition, the entire microcontroller would have frozen indefinitely, waiting for a keyboard press. Using readable() was essential to maintain the asynchronous, non-blocking architecture required for the timers to keep running.

5. Conclusion
This task represented a major shift from hardware-level GPIO manipulation to software-level communication protocols. By successfully implementing a non-blocking serial interface, I demonstrated the ability to create a system that can be dynamically controlled and monitored from an external host PC without interrupting its internal real-time processes.
