/* * Module: UFMFN3-30-2 Embedded Systems
 * Task: Tutorial 2 - Main Task (Button State Machine & Security System)
 * -------------------------------------------------------------------
 * How to demo this code:
 * 1. System starts in NORMAL (Green LED).
 * 2. Press Button 0 to trigger the WARNING state (Blue LED).
 * 3. In WARNING state, enter the passcode: Button 1, then 2, then 3, then 4.
 * - If correct, it returns to NORMAL.
 * - If wrong 3 times, it goes to LOCKDOWN (Flashing Red LED).
 * 4. In LOCKDOWN, press Button 5 to reset the system.
 */

#include "mbed.h"

// 1. SETUP BUTTONS (D2 to D7)
DigitalIn btn0(D2, PullDown);
DigitalIn btn1(D3, PullDown);
DigitalIn btn2(D4, PullDown);
DigitalIn btn3(D5, PullDown);
DigitalIn btn4(D6, PullDown);
DigitalIn btn5(D7, PullDown);

// 2. SETUP LEDS
DigitalOut ledGreen(LED1);
DigitalOut ledBlue(LED2);
DigitalOut ledRed(LED3);

// 3. SETUP STATES
const int NORMAL = 0;
const int WARNING = 1;
const int LOCKDOWN = 2;
int currentState = NORMAL;

// 4. SETUP VARIABLES
int guess1 = -1; 
int guess2 = -1;
int guess3 = -1; 
int guess4 = -1;
int clicks = 0;  
int fails = 0;

Timer blinkTimer;

// 5. SIMPLE INPUT FUNCTION
int getButton() {
    int press = -1;
    if (btn0.read() == 1) { press = 0; }
    if (btn1.read() == 1) { press = 1; }
    if (btn2.read() == 1) { press = 2; }
    if (btn3.read() == 1) { press = 3; }
    if (btn4.read() == 1) { press = 4; }
    if (btn5.read() == 1) { press = 5; }
    return press;
}

int main() {
    printf("--- SECURITY SYSTEM BOOTED ---\n");
    printf("State: NORMAL\n");
    blinkTimer.start();

    while (true) {
        // Read the buttons
        int press = getButton();

        // Simple Debounce: If a button is pressed, wait 250ms so it doesn't double-click
        if (press != -1) {
            thread_sleep_for(250); 
        }

        // --- STATE 0: NORMAL ---
        if (currentState == NORMAL) {
            ledGreen = 1; ledBlue = 0; ledRed = 0;

            // Trigger the alarm by pressing Button 0
            if (press == 0) {
                currentState = WARNING;
                clicks = 0; 
                fails = 0;
                printf("\nALERT: Sensor Triggered! Moving to WARNING.\n");
                printf("Enter 4-digit code to disarm.\n");
            }
        }
        
        // --- STATE 1: WARNING ---
        else if (currentState == WARNING) {
            ledGreen = 0; ledBlue = 1; ledRed = 0;

            if (press != -1) {
                clicks++;
                
                // Store the sequence of button presses
                if (clicks == 1) { guess1 = press; }
                else if (clicks == 2) { guess2 = press; }
                else if (clicks == 3) { guess3 = press; }
                else if (clicks == 4) { guess4 = press; }

                printf("Digit %d entered: [%d]\n", clicks, press);

                // Check the code once 4 buttons have been pressed
                if (clicks == 4) {
                    // The secret passcode is: Button 1, 2, 3, 4
                    if (guess1 == 1 && guess2 == 2 && guess3 == 3 && guess4 == 4) {
                        printf("PASSCODE ACCEPTED. Returning to NORMAL.\n\n");
                        currentState = NORMAL;
                    } else {
                        fails++;
                        printf("ERROR: Incorrect Passcode. Fails: %d/3\n", fails);
                        clicks = 0; // Reset clicks so they can try again
                        
                        // If they fail 3 times, lock the system
                        if (fails >= 3) {
                            printf("MAXIMUM ATTEMPTS REACHED. SYSTEM LOCKDOWN.\n\n");
                            currentState = LOCKDOWN;
                        }
                    }
                }
            }
        }

        // --- STATE 2: LOCKDOWN ---
        else if (currentState == LOCKDOWN) {
            ledGreen = 0; ledBlue = 0; 
            
            // Flash the Red LED using the timer (Non-blocking)
            if (chrono::duration_cast<chrono::milliseconds>(blinkTimer.elapsed_time()).count() > 250) {
                ledRed = !ledRed;
                blinkTimer.reset();
            }

            // Secret Admin Reset: Press Button 5 to reboot the system
            if (press == 5) {
                printf("ADMIN OVERRIDE. System Resetting...\n\n");
                currentState = NORMAL;
            }
        }

        // Small base delay to keep the loop stable
        thread_sleep_for(50);
    }
}
