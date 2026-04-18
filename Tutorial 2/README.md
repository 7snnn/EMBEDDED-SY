Tutorial 2: Main Task – State Machine and Button Logic
Module: UFMFN3-30-2 Embedded Systems
Hardware: STM32 Nucleo-F439ZI

1. Objective
The objective of this main task was to implement a robust Finite State Machine (FSM) using external button inputs. The system was designed to handle different operational modes (NORMAL, WARNING, and LOCKDOWN) by processing user inputs through six digital buttons and managing hardware responses via a keypad-style "guess" system and timers.

2. Logic and Implementation
The system utilizes a structured state-based approach rather than complex nested loops. I implemented a custom input function, getButton(), to poll six buttons (D2 to D7) configured with PullDown resistors. This ensures that the pins read a stable Logic 0 when not pressed.

Key logic components include:

State Management: Using const int values to represent NORMAL (0), WARNING (1), and LOCKDOWN (2). This makes the code readable and allows for easy transitions between system behaviors.

Input Processing: A 4-digit "guess" system was initialized (guess1 to guess4) to store button sequences.

Timing: The use of Timer objects (stateTimer and blinkTimer) allows for non-blocking time management, enabling the system to track how long it has been in a specific state or to handle LED blinking intervals without stopping code execution.

Hardware Interfacing: Six DigitalIn buttons and three DigitalOut LEDs provide the physical interface for the security-style logic.

3. Final Main Task Code (Structure)
C++
/* * Module: UFMFN3-30-2 Embedded Systems
 * Task: Tutorial 2 - Main Task (Button State Machine)
 */
#include "mbed.h"

// 1. SETUP BUTTONS
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
int guess1 = -1; int guess2 = -1;
int guess3 = -1; int guess4 = -1;
int clicks = 0;  int fails = 0;

Timer stateTimer;
Timer blinkTimer;

// 5. SIMPLE INPUT FUNCTION
int getButton() {
    int press = -1;
    if (btn0.read() == 1) { press = 0; }
    if (btn1.read() == 1) { press = 1; }
    if (btn2.read() == 1) { press = 2; }
    // ... logic continues for other buttons
    return press;
}

int main() {
    printf("Tutorial 2 Main Task: State Machine Initialized\n");
    while (true) {
        // State Machine Logic would follow here
    }
}
4. Technical Reflection (Errors & Challenges)
Developing this complex input system revealed several engineering hurdles:

Floating Inputs: My biggest error initially was forgetting to set the buttons to PullDown mode. This caused "ghost presses" where the system would register inputs randomly because the pins were "floating." Adding the PullDown internal resistor fixed this by pulling the signal to ground when the button is open.

Input Debouncing: I realized that a single physical button press often registers as multiple "clicks" in code because the metal contacts bounce. I had to consider how to handle the clicks variable to ensure one press only counted as one input.

Memory Management: Initializing the guess variables to -1 was an important step. This allowed me to distinguish between "Button 0" (value 0) and "No Button Pressed" (value -1), preventing the system from automatically thinking a code had been entered.

5. Conclusion
The Main Task for Tutorial 2 demonstrated the transition from simple timing to event-driven programming. By utilizing a state machine and a dedicated input function, I created a scalable system that can react differently to the same hardware (buttons) depending on the current operational state of the microcontroller.
