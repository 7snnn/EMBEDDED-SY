/* * Module: UFMFN3-30-2 Embedded Systems
 * Student Name: [Hassan aljadi]
 * Task: Tutorial 1 - Synchronized Triple LED Toggle
 */

#include "mbed.h"

// Initialize the three on-board LEDs available on the Nucleo board
DigitalOut led1(LED1); // Green
DigitalOut led2(LED2); // Blue
DigitalOut led3(LED3); // Red

int main() {
    // Initial debug message for the Serial Terminal
    printf("Task 1: Synchronized Toggle Initialized.\n");

    // Infinite loop to ensure the board stays active
    while (true) {
        
        // Toggle all three LEDs simultaneously using the Logical NOT operator (!)
        led1 = !led1;
        led2 = !led2;
        led3 = !led3;

        // 500ms blocking delay to hold the current state
        thread_sleep_for(500); 
    }
}
