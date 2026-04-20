#include "mbed.h"
#include <cstdio>
#include <cstring>

// ============================================================
// SERIAL
// ============================================================
UnbufferedSerial pc(USBTX, USBRX, 115200);

void send_text(const char *msg)
{
    pc.write(msg, strlen(msg));
}

void send_line(const char *msg)
{
    send_text(msg);
    send_text("\r\n");
}

// ============================================================
// LCD I2C (PCF8574 backpack)
// SDA = PB_9, SCL = PB_8
// 8-bit address = 0x4E
// ============================================================
I2C lcd_i2c(PB_9, PB_8);

static const int LCD_ADDR = 0x4E;
static const char LCD_BACKLIGHT = 0x08;
static const char LCD_ENABLE    = 0x04;
static const char LCD_RS        = 0x01;

void lcd_write_byte(char data)
{
    lcd_i2c.write(LCD_ADDR, &data, 1);
}

void lcd_pulse_enable(char data)
{
    lcd_write_byte(data | LCD_ENABLE);
    wait_us(1);
    lcd_write_byte(data & ~LCD_ENABLE);
    wait_us(50);
}

void lcd_write4bits(char nibble, char mode)
{
    char data = 0x00;
    data |= LCD_BACKLIGHT;
    data |= mode;
    data |= (nibble & 0xF0);
    lcd_write_byte(data);
    lcd_pulse_enable(data);
}

void lcd_send(char value, char mode)
{
    lcd_write4bits(value & 0xF0, mode);
    lcd_write4bits((value << 4) & 0xF0, mode);
}

void lcd_cmd(char cmd)
{
    lcd_send(cmd, 0x00);
}

void lcd_data(char data)
{
    lcd_send(data, LCD_RS);
}

void lcd_init()
{
    thread_sleep_for(50);

    lcd_write4bits(0x30, 0x00);
    thread_sleep_for(5);
    lcd_write4bits(0x30, 0x00);
    thread_sleep_for(5);
    lcd_write4bits(0x30, 0x00);
    thread_sleep_for(1);
    lcd_write4bits(0x20, 0x00);

    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
    thread_sleep_for(2);
}

void lcd_set_cursor(int row, int col)
{
    static const int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    lcd_cmd(0x80 | (col + row_offsets[row]));
}

void lcd_print_line(int row, const char *text)
{
    char buffer[21];
    std::snprintf(buffer, sizeof(buffer), "%-20.20s", text);
    lcd_set_cursor(row, 0);

    for (int i = 0; i < 20; i++) {
        lcd_data(buffer[i]);
    }
}

// ============================================================
// KEYPAD
// ============================================================
DigitalOut row1(PB_3), row2(PB_5), row3(PC_7), row4(PA_15);
DigitalIn  col1(PB_12, PullUp), col2(PB_13, PullUp), col3(PB_15, PullUp), col4(PC_6, PullUp);

const char key_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

void set_all_rows_high()
{
    row1 = 1;
    row2 = 1;
    row3 = 1;
    row4 = 1;
}

char scan_keypad_raw()
{
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

char get_keypad_key()
{
    char k1 = scan_keypad_raw();
    if (k1 == '\0') {
        return '\0';
    }

    thread_sleep_for(40);
    char k2 = scan_keypad_raw();
    if (k2 != k1) {
        return '\0';
    }

    while (scan_keypad_raw() != '\0') {
        thread_sleep_for(10);
    }

    thread_sleep_for(40);
    return k1;
}

// ============================================================
// INPUTS / OUTPUTS
// ============================================================
AnalogIn tempSensor(A1);
AnalogIn gasSensor(A2);
DigitalOut buzzer(PE_10);
DigitalOut alarmLed(LED1);

void buzzer_on()
{
    buzzer = 0;   // active-low
}

void buzzer_off()
{
    buzzer = 1;
}

// ============================================================
// SETTINGS
// ============================================================
const char SECRET_CODE[] = "18888";
char enteredCode[6];
int enteredIndex = 0;

const float TEMP_LIMIT_C = 40.0f;
const float GAS_LIMIT    = 20.0f;

float filteredTemp = 25.0f;
float filteredGas  = 0.0f;
float tempOffset   = 0.0f;

bool alarmActive = false;
bool silencedUntilSafe = false;

// ============================================================
// SCREEN MODE
// ============================================================
enum ScreenMode {
    SCREEN_STATUS,
    SCREEN_GAS_INFO,
    SCREEN_TEMP_INFO,
    SCREEN_CODE_ENTRY,
    SCREEN_CLEARED,
    SCREEN_WRONG,
    SCREEN_ALARM_STATE
};

ScreenMode currentScreen = SCREEN_STATUS;

char lastLine0[21] = "";
char lastLine1[21] = "";
char lastLine2[21] = "";
char lastLine3[21] = "";

void update_line_if_changed(int row, const char *text, char *cache)
{
    char padded[21];
    std::snprintf(padded, sizeof(padded), "%-20.20s", text);

    if (std::strncmp(cache, padded, 20) != 0) {
        lcd_print_line(row, padded);
        std::strncpy(cache, padded, 20);
        cache[20] = '\0';
    }
}

void lcd_show_screen(const char *l0, const char *l1, const char *l2, const char *l3)
{
    update_line_if_changed(0, l0, lastLine0);
    update_line_if_changed(1, l1, lastLine1);
    update_line_if_changed(2, l2, lastLine2);
    update_line_if_changed(3, l3, lastLine3);
}

void make_stars(char *out)
{
    for (int i = 0; i < 5; i++) {
        out[i] = (i < enteredIndex) ? '*' : ' ';
    }
    out[5] = '\0';
}

void show_status_screen()
{
    char line1[21];
    char line2[21];

    std::snprintf(line1, sizeof(line1), "T:%2dC G:%2d",
                  (int)(filteredTemp + 0.5f),
                  (int)(filteredGas + 0.5f));

    if (silencedUntilSafe) {
        std::snprintf(line2, sizeof(line2), "Silenced");
    } else if (alarmActive) {
        std::snprintf(line2, sizeof(line2), "Warning Active");
    } else {
        std::snprintf(line2, sizeof(line2), "System Safe");
    }

    lcd_show_screen(
        "Tutorial 6",
        line1,
        line2,
        "4=Gas 5=Temp"
    );
}

void show_gas_info_screen(bool gasWarning)
{
    char line1[21];
    std::snprintf(line1, sizeof(line1), "Gas Level:%2d", (int)(filteredGas + 0.5f));

    lcd_show_screen(
        "Gas Detector",
        gasWarning ? "Gas: DETECTED" : "Gas: SAFE",
        line1,
        "#=Back"
    );
}

void show_temp_info_screen(bool tempWarning)
{
    char line1[21];
    std::snprintf(line1, sizeof(line1), "Temp:%2d C", (int)(filteredTemp + 0.5f));

    lcd_show_screen(
        "Temp Detector",
        tempWarning ? "Temp: HIGH" : "Temp: NORMAL",
        line1,
        "#=Back"
    );
}

void show_code_entry_screen(bool gasWarning, bool tempWarning)
{
    char line1[21];
    char line2[21];
    char stars[6];

    make_stars(stars);

    if (gasWarning && tempWarning) {
        std::snprintf(line1, sizeof(line1), "Gas & Temp High");
    } else if (gasWarning) {
        std::snprintf(line1, sizeof(line1), "Gas Warning");
    } else if (tempWarning) {
        std::snprintf(line1, sizeof(line1), "Temp Warning");
    } else {
        std::snprintf(line1, sizeof(line1), "Enter Code");
    }

    std::snprintf(line2, sizeof(line2), "Code:%s", stars);

    lcd_show_screen(
        "Enter Code",
        line1,
        line2,
        "*=Clear #=Back"
    );
}

void show_cleared_screen()
{
    lcd_show_screen(
        "Alarm Cleared",
        "Code Accepted",
        "System Safe",
        "Returning..."
    );
}

void show_wrong_screen()
{
    lcd_show_screen(
        "Wrong Code",
        "Try Again",
        "Need 18888",
        "Returning..."
    );
}

void show_alarm_state_screen()
{
    lcd_show_screen(
        "Alarm State",
        alarmActive ? "ALARM ON" : "ALARM OFF",
        silencedUntilSafe ? "Silenced" : "Monitoring",
        "#=Back"
    );
}

// ============================================================
// MAIN
// ============================================================
int main()
{
    lcd_i2c.frequency(100000);
    buzzer_off();
    alarmLed = 0;
    set_all_rows_high();
    std::memset(enteredCode, 0, sizeof(enteredCode));

    send_line("Tutorial 6 starting...");
    send_line("Code = 18888");
    send_line("Calibrating temperature sensor...");

    lcd_init();

    // startup calibration
    float startupSum = 0.0f;
    const int startupSamples = 60;

    for (int i = 0; i < startupSamples; i++) {
        float rawTemp = tempSensor.read() * 330.0f;
        startupSum += rawTemp;
        thread_sleep_for(50);
    }

    float startupAverage = startupSum / startupSamples;
    tempOffset = startupAverage - 25.0f;

    char calMsg[80];
    std::snprintf(calMsg, sizeof(calMsg), "Temp offset = %d C",
                  (int)(tempOffset + 0.5f));
    send_line(calMsg);

    Timer messageTimer;
    messageTimer.start();

    Timer alarmStateTimer;
    alarmStateTimer.start();

    Timer lcdRefreshTimer;
    lcdRefreshTimer.start();

    while (true) {
        // average multiple temp samples each loop
        float tempSum = 0.0f;
        const int tempLoopSamples = 8;
        for (int i = 0; i < tempLoopSamples; i++) {
            tempSum += tempSensor.read() * 330.0f;
            wait_us(2000);
        }
        float rawTemp = (tempSum / tempLoopSamples) - tempOffset;

        // average gas samples each loop
        float gasSum = 0.0f;
        const int gasLoopSamples = 5;
        for (int i = 0; i < gasLoopSamples; i++) {
            gasSum += gasSensor.read() * 100.0f;
            wait_us(2000);
        }
        float rawGas = gasSum / gasLoopSamples;

        if (rawTemp < 0.0f) {
            rawTemp = 0.0f;
        }

        // reject impossible temperature jumps
        if (rawTemp > filteredTemp + 8.0f) {
            rawTemp = filteredTemp + 8.0f;
        }
        if (rawTemp < filteredTemp - 8.0f) {
            rawTemp = filteredTemp - 8.0f;
        }

        // smoother temperature
        filteredTemp = 0.95f * filteredTemp + 0.05f * rawTemp;

        // smoother gas for display
        filteredGas = 0.75f * filteredGas + 0.25f * rawGas;

        bool tempWarning = (filteredTemp > TEMP_LIMIT_C);
        bool gasWarning  = (rawGas >= GAS_LIMIT);

        if (silencedUntilSafe) {
            buzzer_off();
            alarmLed = 0;
            alarmActive = false;

            if (!tempWarning && !gasWarning) {
                silencedUntilSafe = false;
            }
        } else {
            if (tempWarning || gasWarning) {
                alarmActive = true;
                buzzer_on();
                alarmLed = 1;
                currentScreen = SCREEN_CODE_ENTRY;
            } else {
                alarmActive = false;
                buzzer_off();
                alarmLed = 0;
            }
        }

        if (alarmStateTimer.elapsed_time() >= 60s) {
            currentScreen = SCREEN_ALARM_STATE;
            messageTimer.reset();
            alarmStateTimer.reset();
        }

        // refresh LCD only every 250 ms to reduce noise
        if (lcdRefreshTimer.elapsed_time() >= 250ms) {
            switch (currentScreen) {
                case SCREEN_STATUS:
                    show_status_screen();
                    break;
                case SCREEN_GAS_INFO:
                    show_gas_info_screen(gasWarning);
                    break;
                case SCREEN_TEMP_INFO:
                    show_temp_info_screen(tempWarning);
                    break;
                case SCREEN_CODE_ENTRY:
                    show_code_entry_screen(gasWarning, tempWarning);
                    break;
                case SCREEN_CLEARED:
                    show_cleared_screen();
                    if (messageTimer.elapsed_time() >= 1500ms) {
                        currentScreen = SCREEN_STATUS;
                    }
                    break;
                case SCREEN_WRONG:
                    show_wrong_screen();
                    if (messageTimer.elapsed_time() >= 1500ms) {
                        currentScreen = SCREEN_CODE_ENTRY;
                    }
                    break;
                case SCREEN_ALARM_STATE:
                    show_alarm_state_screen();
                    if (messageTimer.elapsed_time() >= 2000ms) {
                        currentScreen = alarmActive ? SCREEN_CODE_ENTRY : SCREEN_STATUS;
                    }
                    break;
            }
            lcdRefreshTimer.reset();
        }

        char key = get_keypad_key();

        if (key != '\0') {
            char dbg[40];
            std::snprintf(dbg, sizeof(dbg), "Key: %c", key);
            send_line(dbg);

            if (key == '4') {
                currentScreen = SCREEN_GAS_INFO;
                messageTimer.reset();
            }
            else if (key == '5') {
                currentScreen = SCREEN_TEMP_INFO;
                messageTimer.reset();
            }
            else if (key == '#') {
                currentScreen = SCREEN_STATUS;
                messageTimer.reset();
            }
            else if (key == '*') {
                enteredIndex = 0;
                std::memset(enteredCode, 0, sizeof(enteredCode));
                currentScreen = SCREEN_CODE_ENTRY;
                send_line("Code cleared");
            }
            else if ((key >= '0' && key <= '9') || key == 'A') {
                if (alarmActive) {
                    if (enteredIndex < 5) {
                        enteredCode[enteredIndex++] = key;
                    }

                    // immediately refresh stars
                    currentScreen = SCREEN_CODE_ENTRY;
                    show_code_entry_screen(gasWarning, tempWarning);

                    if (enteredIndex == 5) {
                        enteredCode[5] = '\0';
                        send_line("Checking code...");

                        if (std::strcmp(enteredCode, SECRET_CODE) == 0) {
                            enteredIndex = 0;
                            std::memset(enteredCode, 0, sizeof(enteredCode));
                            silencedUntilSafe = true;
                            alarmActive = false;
                            buzzer_off();
                            alarmLed = 0;
                            currentScreen = SCREEN_CLEARED;
                            messageTimer.reset();
                            send_line("Correct code. Alarm silenced.");
                        } else {
                            enteredIndex = 0;
                            std::memset(enteredCode, 0, sizeof(enteredCode));
                            currentScreen = SCREEN_WRONG;
                            messageTimer.reset();
                            send_line("Incorrect code.");
                        }
                    }
                }
            }
        }

        thread_sleep_for(60);
    }
}
