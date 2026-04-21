#include "mbed.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

void blink_five_times()
{
    for (int i = 0; i < 5; i++) {
        // all LEDs on
        led1 = 1;
        led2 = 1;
        led3 = 1;
        ThisThread::sleep_for(200ms);

        // all LEDs off
        led1 = 0;
        led2 = 0;
        led3 = 0;
        ThisThread::sleep_for(200ms);
    }

    // final state
    led1 = 1;
    led2 = 0;
    led3 = 0;
}

int main()
{
    blink_five_times();

    while (true) {
        ThisThread::sleep_for(1s);
    }
}
