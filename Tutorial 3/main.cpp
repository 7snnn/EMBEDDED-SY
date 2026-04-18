/* * Module: UFMFN3-30-2 Embedded Systems
 * Task: Tutorial 3 - Main Task (Serial Interface)
 * -------------------------------------------------------------------
 * How to demo this code:
 * 1. Flash the board.
 * 2. Open your Serial Terminal (Tera Term, Putty, or Mbed Studio).
 * 3. IMPORTANT: Set your Baud Rate to 115200.
 * 4. Press the keys 1, 4, 5, or 6 on your keyboard to interact.
 */

#include "mbed.h"

// 1. HARDWARE SETUP
// Using 115200 baud rate as required by the lab sheet
UnbufferedSerial pc(USBTX, USBRX, 115200);

DigitalOut ledGreen(LED1); // System Safe indicator
DigitalOut ledBlue(LED2);  // Signature input feedback light (flashes on keypress)
DigitalOut ledRed(LED3);   // Alarm indicator

// 2. SYSTEM STATE VARIABLES
bool gasAlarm = false;
bool tempAlarm = false;
bool continuousMonitoring = false;

Timer statusTimer; // Tracks the 2-second interval for continuous monitoring

int main() {
    // Start the timer
    statusTimer.start();

    // Initial Startup Message
    // Note: pc.write requires the exact string length as the second parameter
    pc.write("Smart Home Serial Interface Ready\r\n", 36);
    pc.write("Commands: 1 (Gas), 4 (Temp), 5 (Reset), 6 (Monitor)\r\n", 53);

    // Turn on the Green LED to show the system is active and safe
    ledGreen = 1;

    while (true) {
        
        // --- PART A: KEYBOARD INPUT HANDLING ---
        // Only trigger if there is actual data waiting in the serial buffer
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
                    pc.write(" Gas Simulation: CLEAR\r\n", 24);
                }
            }
            
            // KEY 4: Toggle Temp Simulation
            else if (inputChar == '4') {
                tempAlarm = !tempAlarm;
                if (tempAlarm) {
                    pc.write(" WARNING: TEMP HIGH\r\n", 22);
                } else {
                    pc.write(" Temp Simulation: CLEAR\r\n", 25);
                }
            }
            
            // KEY 5: Reset All Alarms
            else if (inputChar == '5') {
                gasAlarm = false;
                tempAlarm = false;
                pc.write(" System RESET. All clear.\r\n", 28);
            }
            
            // KEY 6: Toggle Continuous Monitoring Mode
            else if (inputChar == '6') {
                continuousMonitoring = !continuousMonitoring;
                if (continuousMonitoring) {
                    pc.write(" Monitoring: ACTIVE\r\n", 21);
                    statusTimer.reset(); // Reset timer so it starts counting from 0 now
                } else {
                    pc.write(" Monitoring: PAUSED\r\n", 21);
                }
            }

            // Quick delay and turn off the Blue LED to complete the "Flash" effect
            thread_sleep_for(50);
            ledBlue = 0;
        }

        // --- PART B: CONTINUOUS MONITORING LOGIC ---
        // If monitoring is active, print a status message every 2 seconds
        if (continuousMonitoring) {
            // Check if 2000 milliseconds (2 seconds) have passed
            if (chrono::duration_cast<chrono::milliseconds>(statusTimer.elapsed_time()).count() >= 2000) {
                pc.write(" [Status] System Nominal...\r\n", 29);
                statusTimer.reset(); // Reset the timer to count the next 2 seconds
            }
        } else {
            // Keep the timer at zero if we aren't monitoring
            statusTimer.reset(); 
        }

        // --- PART C: HARDWARE ALARM LOGIC ---
        // Update the Green and Red LEDs based on the alarm states
        if (gasAlarm || tempAlarm) {
            ledGreen = 0;
            ledRed = 1;
        } else {
            ledGreen = 1;
            ledRed = 0;
        }
    }
}
