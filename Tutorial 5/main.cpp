#include "mbed.h"
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cmath>

UnbufferedSerial pc(USBTX, USBRX, 115200);

void send_text(const char *msg) {
    pc.write(msg, strlen(msg));
}

void send_line(const char *msg) {
    pc.write(msg, strlen(msg));
    pc.write("\r\n", 2);
}

// ---------------- HARDWARE ----------------
DigitalOut row1(PB_3), row2(PB_5), row3(PC_7), row4(PA_15);
DigitalIn col1(PB_12, PullUp), col2(PB_13, PullUp), col3(PB_15, PullUp), col4(PC_6, PullUp);

AnalogIn pot(A0);
AnalogIn temp_sens(A1);
AnalogIn gas_sens(A2);

DigitalOut buzzer(PE_10);   // active-low buzzer
DigitalOut alarm_led(LED1);

// ---------------- DATA ----------------
struct EventLog {
    time_t timestamp;
    float temperature;
    float gasLevel;
};

EventLog eventHistory[5];
int eventCount = 0;

const char SECRET_CODE[] = "1235";
char enteredCode[5];
int enteredIndex = 0;

bool alarmActive = false;
bool alarmArmed = true;
bool showThresholds = true;

const char key_map[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// ---------------- BUZZER ----------------
void buzzer_on() {
    buzzer = 0;
}

void buzzer_off() {
    buzzer = 1;
}

// ---------------- LOGGING ----------------
void log_event(float temp, float gas) {
    for (int i = 4; i > 0; i--) {
        eventHistory[i] = eventHistory[i - 1];
    }

    eventHistory[0].timestamp = time(NULL);
    eventHistory[0].temperature = temp;
    eventHistory[0].gasLevel = gas;

    if (eventCount < 5) {
        eventCount++;
    }
}

void show_logs() {
    send_line("");
    send_line("----- EVENT LOG (5 MOST RECENT) -----");

    if (eventCount == 0) {
        send_line("No events stored.");
        send_line("-------------------------------------");
        return;
    }

    for (int i = 0; i < eventCount; i++) {
        char timeBuffer[32];
        char outBuffer[128];

        struct tm *timeInfo = localtime(&eventHistory[i].timestamp);

        if (timeInfo != NULL) {
            strftime(timeBuffer, sizeof(timeBuffer), "%d/%m %H:%M:%S", timeInfo);
        } else {
            strcpy(timeBuffer, "Unknown time");
        }

        snprintf(outBuffer, sizeof(outBuffer),
                 "%d) [%s] Temp: %d C | Gas: %d ppm\r\n",
                 i + 1,
                 timeBuffer,
                 (int)eventHistory[i].temperature,
                 (int)eventHistory[i].gasLevel);

        send_text(outBuffer);
    }

    send_line("-------------------------------------");
}

// ---------------- KEYPAD ----------------
void set_all_rows_high() {
    row1 = 1; row2 = 1; row3 = 1; row4 = 1;
}

char scan_keypad_raw() {
    set_all_rows_high();
    row1 = 0;
    if (col1 == 0) return key_map[0][0];
    if (col2 == 0) return key_map[0][1];
    if (col3 == 0) return key_map[0][2];
    if (col4 == 0) return key_map[0][3];

    set_all_rows_high();
    row2 = 0;
    if (col1 == 0) return key_map[1][0];
    if (col2 == 0) return key_map[1][1];
    if (col3 == 0) return key_map[1][2];
    if (col4 == 0) return key_map[1][3];

    set_all_rows_high();
    row3 = 0;
    if (col1 == 0) return key_map[2][0];
    if (col2 == 0) return key_map[2][1];
    if (col3 == 0) return key_map[2][2];
    if (col4 == 0) return key_map[2][3];

    set_all_rows_high();
    row4 = 0;
    if (col1 == 0) return key_map[3][0];
    if (col2 == 0) return key_map[3][1];
    if (col3 == 0) return key_map[3][2];
    if (col4 == 0) return key_map[3][3];

    return '\0';
}

char get_keypad_key() {
    static char stableKey = '\0';
    static char lastRawKey = '\0';

    char rawKey = scan_keypad_raw();

    if (rawKey != lastRawKey) {
        thread_sleep_for(50);
        rawKey = scan_keypad_raw();
    }

    lastRawKey = rawKey;

    if (rawKey != '\0' && stableKey == '\0') {
        stableKey = rawKey;
        return stableKey;
    }

    if (rawKey == '\0') {
        stableKey = '\0';
    }

    return '\0';
}

// ---------------- MAIN ----------------
int main() {
    set_time(1704067200);

    set_all_rows_high();
    alarm_led = 0;
    buzzer_off();
    memset(enteredCode, 0, sizeof(enteredCode));

    Timer thresholdTimer;
    thresholdTimer.start();

    float filteredTemp = 25.0f;
    float filteredGas = 0.0f;

    send_line("Smart Home Security System Active");
    send_line("PIN code: 1235");
    send_line("Press # to display event log");
    send_line("Press D to pause/resume threshold display");
    send_line("");

    while (true) {
        float p = pot.read();
        float tempThreshold = 25.0f + (p * 12.0f);
        float gasThreshold  = p * 800.0f;

        if (showThresholds && thresholdTimer.elapsed_time() >= 2s) {
            char buffer[100];
            snprintf(buffer, sizeof(buffer),
                     "Thresholds -> Temp: %d C | Gas: %d ppm\r\n",
                     (int)tempThreshold, (int)gasThreshold);
            send_text(buffer);
            thresholdTimer.reset();
        }

        float rawTemp = (temp_sens.read() * 3.3f) * 100.0f;
        float rawGas  = gas_sens.read() * 1000.0f;

        filteredTemp = 0.8f * filteredTemp + 0.2f * rawTemp;
        filteredGas  = 0.8f * filteredGas  + 0.2f * rawGas;

        bool tempSensorValid = (filteredTemp >= 0.0f && filteredTemp <= 80.0f);

        bool tempAlarm = tempSensorValid && (filteredTemp > tempThreshold);
        bool gasAlarm  = (filteredGas > gasThreshold);

        if (!alarmArmed) {
            bool tempSafe = (!tempSensorValid) || (filteredTemp < tempThreshold - 2.0f);
            bool gasSafe  = (filteredGas < gasThreshold - 80.0f);

            if (tempSafe && gasSafe) {
                alarmArmed = true;
                send_line("[SYSTEM] Alarm re-armed.");
            }
        }

        if (alarmArmed && !alarmActive && (tempAlarm || gasAlarm)) {
            alarmActive = true;
            alarm_led = 1;
            buzzer_on();

            enteredIndex = 0;
            memset(enteredCode, 0, sizeof(enteredCode));

            log_event(filteredTemp, filteredGas);

            send_line("");
            send_line("[ALERT] Enter 4-Digit Code to Deactivate");
        }

        char key = get_keypad_key();

        if (key != '\0') {
            // D works ANY TIME
            if (key == 'D') {
                showThresholds = !showThresholds;

                if (showThresholds) {
                    send_line("[SYSTEM] Threshold display ON");
                    thresholdTimer.reset();
                } else {
                    send_line("[SYSTEM] Threshold display OFF");
                }
            }
            // # only shows logs when alarm is not active
            else if (!alarmActive && key == '#') {
                show_logs();
            }
            // Alarm keypad handling
            else if (alarmActive) {
                if (key >= '0' && key <= '9') {
                    if (enteredIndex < 4) {
                        enteredCode[enteredIndex++] = key;
                        send_text("*");
                    }

                    if (enteredIndex == 4) {
                        enteredCode[4] = '\0';

                        if (strcmp(enteredCode, SECRET_CODE) == 0) {
                            alarmActive = false;
                            alarmArmed = false;
                            alarm_led = 0;
                            buzzer_off();
                            send_line("");
                            send_line("[SYSTEM] Alarm Cleared.");
                            send_line("[SYSTEM] Lower sensor values to re-arm.");
                        } else {
                            send_line("");
                            send_line("[ERROR] Incorrect Code.");
                        }

                        enteredIndex = 0;
                        memset(enteredCode, 0, sizeof(enteredCode));
                    }
                } else if (key == '*') {
                    enteredIndex = 0;
                    memset(enteredCode, 0, sizeof(enteredCode));
                    send_line("");
                    send_line("[SYSTEM] Code Cleared.");
                }
            }
        }

        thread_sleep_for(50);
    }
}
