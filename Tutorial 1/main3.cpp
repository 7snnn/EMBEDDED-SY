#include "mbed.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

int counter = 0;

int main()
{
    while (true) {
        // LED2 changes every loop
        led2 = !led2;

        // LED1 changes every 2 loops
        if (counter % 2 == 0) {
            led1 = !led1;
        }

        // LED3 changes every 4 loops
        if (counter % 4 == 0) {
            led3 = !led3;
        }

        counter++;
        ThisThread::sleep_for(500ms);
    }
}
