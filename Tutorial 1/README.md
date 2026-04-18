Tutorial 1: GPIO Basics & LED Control
Platform: Nucleo-F439ZI
## Overview
This project introduces the fundamentals of Embedded Systems using Mbed OS. The primary goal was to control the onboard LEDs of the Nucleo-F439ZI board through digital outputs and timing loops.

## Hardware Components
Microcontroller: STM32 Nucleo-F439ZI

Onboard LEDs: - LED1 (Green)

LED2 (Blue)

LED3 (Red)

## Development Tasks
### Task 1: The Single Blink
The first objective was to establish a basic "Heartbeat" for the board by blinking the Green LED at a 1-second interval.

Errors Encountered:

Legacy Commands: Tried using wait(), which is no longer supported. Corrected to thread_sleep_for().

Infinite Loop: Forgot the while(1) loop, causing the code to run once and stop.

### Task 2: Multiple LED Control
Expanded the code to control both the Green and Blue LEDs simultaneously.

Errors Encountered:

Naming Conflicts: Tried naming both DigitalOut objects as led. Learned that each output must have a unique identifier (e.g., led1, led2).

### Task 3: Synchronized Triple Toggle (Final)
The final task synchronized all three onboard LEDs (Green, Blue, and Red) to blink together using a 500ms delay.

## Final Source Code
C++
#include "mbed.h"

// Define the three built-in LEDs as Digital Outputs
DigitalOut led1(LED1); 
DigitalOut led2(LED2); 
DigitalOut led3(LED3); 

int main() {
    // Persistent loop to keep the board active
    while (true) {
        // Use the Logical NOT operator (!) to flip the LED states
        led1 = !led1;
        led2 = !led2;
        led3 = !led3;

        // 500ms delay (0.5 seconds)
        thread_sleep_for(500); 
    }
}
## Troubleshooting Checklist (Lessons Learned)
Case Sensitivity: Ensure DigitalOut is capitalized. Lowercase digitalout results in a compilation error.

The "!" Operator: The exclamation mark must be placed before the variable (!led) to invert the signal.

Baud Rate: When using printf, ensure the Serial Monitor is set to 9600 or 115200 to avoid "garbage" text.

Semicolons: Every line of logic must end with a ; or the build will fail.

Hard Reset: If the board doesn't respond after flashing, press the Black Reset Button to manually start the program.
