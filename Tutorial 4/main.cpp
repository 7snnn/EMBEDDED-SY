#include "mbed.h"
#include <cstdio>
#include <cstring>

UnbufferedSerial pc(USBTX, USBRX, 115200);

// Inputs
AnalogIn potentiometerInput(A0);
AnalogIn tempSensorInput(A1);
AnalogIn gasSensorInput(A2);

// Outputs
DigitalOut buzzerOutput(PE_10);   // active-low buzzer
DigitalOut alarmLed(LED1);

void send_text(const char *msg) {
    pc.write(msg, strlen(msg));
}

void send_line(const char *msg) {
    pc.write(msg, strlen(msg));
    pc.write("\r\n", 2);
}

void buzzer_on() {
    buzzerOutput = 0;
}

void buzzer_off() {
    buzzerOutput = 1;
}

float lm35_to_celsius(float analogReading) {
    return analogReading * 330.0f;
}

int main() {
    buzzer_off();
    alarmLed = 0;

    send_line("Tutorial 4 - Main Task");
    send_line("A0 = Potentiometer | A1 = LM35 | A2 = MQ-2 AO");
    send_line("Calibrating temperature sensor...");
    send_line("");

    float startupSum = 0.0f;
    const int startupSamples = 20;

    for (int i = 0; i < startupSamples; i++) {
        startupSum += lm35_to_celsius(tempSensorInput.read());
        thread_sleep_for(100);
    }

    float startupAverage = startupSum / startupSamples;
    float tempOffset = startupAverage - 25.0f;

    char startMsg[100];
    snprintf(startMsg, sizeof(startMsg),
             "Temperature offset = %d C\r\n",
             (int)(tempOffset + 0.5f));
    send_text(startMsg);

    float filteredTemp = 25.0f;
    float filteredGas = 0.0f;

    while (true) {
        float potValue = potentiometerInput.read();
        float rawTemp = lm35_to_celsius(tempSensorInput.read());
        float rawGas = gasSensorInput.read() * 100.0f;

        float correctedTemp = rawTemp - tempOffset;
        if (correctedTemp < 0.0f) {
            correctedTemp = 0.0f;
        }

        filteredTemp = 0.8f * filteredTemp + 0.2f * correctedTemp;
        filteredGas  = 0.8f * filteredGas  + 0.2f * rawGas;

        float thresholdCounter = potValue * 100.0f;

        float tempThreshold = 30.0f + (thresholdCounter * 0.20f);  // 30 to 50 C
        float gasThreshold  = 15.0f + (thresholdCounter * 0.85f);  // 15 to 100

        bool temperatureWarning = (filteredTemp > tempThreshold);
        bool gasWarning = (filteredGas > gasThreshold);

        int thresholdInt = (int)(thresholdCounter + 0.5f);
        int tempInt = (int)(filteredTemp + 0.5f);
        int gasInt = (int)(filteredGas + 0.5f);

        char msg[220];

        if (temperatureWarning || gasWarning) {
            buzzer_on();
            alarmLed = 1;

            if (temperatureWarning && gasWarning) {
                snprintf(msg, sizeof(msg),
                         "Threshold: %d | Temp Counter: %d C | Gas Counter: %d | Buzzer ON - Cause: Temperature and Gas\r\n",
                         thresholdInt, tempInt, gasInt);
            } else if (temperatureWarning) {
                snprintf(msg, sizeof(msg),
                         "Threshold: %d | Temp Counter: %d C | Gas Counter: %d | Buzzer ON - Cause: Temperature\r\n",
                         thresholdInt, tempInt, gasInt);
            } else {
                snprintf(msg, sizeof(msg),
                         "Threshold: %d | Temp Counter: %d C | Gas Counter: %d | Buzzer ON - Cause: Gas\r\n",
                         thresholdInt, tempInt, gasInt);
            }
        } else {
            buzzer_off();
            alarmLed = 0;

            snprintf(msg, sizeof(msg),
                     "Threshold: %d | Temp Counter: %d C | Gas Counter: %d | System Normal\r\n",
                     thresholdInt, tempInt, gasInt);
        }

        send_text(msg);
        thread_sleep_for(1000);
    }
}
