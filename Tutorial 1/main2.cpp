#include "mbed.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

int main()
{
    while (true) {
        // toggle all LEDs together
        led1 = !led1;
        led2 = !led2;
        led3 = !led3;

        // wait 0.5 s
        ThisThread::sleep_for(500ms);
    }
}
