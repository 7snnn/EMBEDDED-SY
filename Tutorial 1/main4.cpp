#include "mbed.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

void ping_pong_sequence()
{
    DigitalOut* sequence[] = {&led1, &led2, &led3, &led2, &led1};

    while (true) {
        for (int i = 0; i < 5; i++) {
            // turn current LED on
            *sequence[i] = 1;
            ThisThread::sleep_for(200ms);

            // turn current LED off
            *sequence[i] = 0;
            ThisThread::sleep_for(200ms);
        }
    }
}

int main()
{
    ping_pong_sequence();
}
