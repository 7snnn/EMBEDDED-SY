#include "mbed.h"
#include <cstdio>
#include <cstring>

// ================= SERIAL =================
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

// ================= LCD I2C =================
// LCD SDA -> D14 (PB_9)
// LCD SCL -> D15 (PB_8)
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

// ================= RELAY / MOTOR =================
DigitalOut relayIN1(PF_2);
DigitalOut relayIN2(PE_3);

// TRY THIS VERSION NOW
const int RELAY_ON  = 0;
const int RELAY_OFF = 1;

// ================= INTERRUPTS =================
InterruptIn intOpen(PF_9, PullUp);
InterruptIn intClose(PF_8, PullUp);
InterruptIn pirSensor(PF_10, PullDown);

// ================= KEYPAD =================
DigitalOut row1(PB_3), row2(PB_5), row3(PC_7), row4(PA_15);
DigitalIn  col1(PB_12, PullUp), col2(PB_13, PullUp), col3(PB_15, PullUp), col4(PC_6, PullUp);

const char keyMap[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// ================= LEDS =================
DigitalOut ledOpening(LED1);
DigitalOut ledClosing(LED2);
DigitalOut ledAlarm(LED3);

// ================= STATES =================
enum MotorState {
    MOTOR_STOPPED = 0,
    MOTOR_OPENING,
    MOTOR_CLOSING
};

enum ModeState {
    MODE_MANUAL = 0,
    MODE_AUTO
};

volatile bool openFlag = false;
volatile bool closeFlag = false;
volatile bool pirFlag = false;

MotorState motorState = MOTOR_STOPPED;
ModeState modeState = MODE_MANUAL;

bool alarmActive = false;
bool autoCycleActive = false;
bool eventFlash = false;

const char *lastEvent = "NONE";

// ================= LCD CACHE =================
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

void lcd_show(const char *l0, const char *l1, const char *l2, const char *l3)
{
    update_line_if_changed(0, l0, lastLine0);
    update_line_if_changed(1, l1, lastLine1);
    update_line_if_changed(2, l2, lastLine2);
    update_line_if_changed(3, l3, lastLine3);
}

// ================= TEXT =================
const char* motor_text()
{
    switch (motorState) {
        case MOTOR_OPENING: return "OPENING";
        case MOTOR_CLOSING: return "CLOSING";
        default: return "STOPPED";
    }
}

const char* mode_text()
{
    return (modeState == MODE_AUTO) ? "AUTO" : "MANUAL";
}

// ================= MOTOR CONTROL =================
void motor_stop()
{
    relayIN1 = RELAY_OFF;
    relayIN2 = RELAY_OFF;
    motorState = MOTOR_STOPPED;
    ledOpening = 0;
    ledClosing = 0;
}

void motor_open()
{
    relayIN1 = RELAY_ON;
    relayIN2 = RELAY_OFF;
    motorState = MOTOR_OPENING;
    ledOpening = 1;
    ledClosing = 0;
}

void motor_close()
{
    relayIN1 = RELAY_OFF;
    relayIN2 = RELAY_ON;
    motorState = MOTOR_CLOSING;
    ledOpening = 0;
    ledClosing = 1;
}

// ================= INTERRUPTS =================
void open_isr()
{
    openFlag = true;
    lastEvent = "INT OPEN";
    eventFlash = true;
}

void close_isr()
{
    closeFlag = true;
    lastEvent = "INT CLOSE";
    eventFlash = true;
}

void pir_isr()
{
    pirFlag = true;
    lastEvent = "PIR ALERT";
    eventFlash = true;
}

// ================= KEYPAD =================
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
    if (col1 == 0) return keyMap[0][0];
    if (col2 == 0) return keyMap[0][1];
    if (col3 == 0) return keyMap[0][2];
    if (col4 == 0) return keyMap[0][3];

    set_all_rows_high();
    row2 = 0;
    if (col1 == 0) return keyMap[1][0];
    if (col2 == 0) return keyMap[1][1];
    if (col3 == 0) return keyMap[1][2];
    if (col4 == 0) return keyMap[1][3];

    set_all_rows_high();
    row3 = 0;
    if (col1 == 0) return keyMap[2][0];
    if (col2 == 0) return keyMap[2][1];
    if (col3 == 0) return keyMap[2][2];
    if (col4 == 0) return keyMap[2][3];

    set_all_rows_high();
    row4 = 0;
    if (col1 == 0) return keyMap[3][0];
    if (col2 == 0) return keyMap[3][1];
    if (col3 == 0) return keyMap[3][2];
    if (col4 == 0) return keyMap[3][3];

    return '\0';
}

char get_keypad_key()
{
    static char stableKey = '\0';
    static char lastRaw = '\0';

    char raw = scan_keypad_raw();

    if (raw != lastRaw) {
        thread_sleep_for(30);
        raw = scan_keypad_raw();
    }

    lastRaw = raw;

    if (raw != '\0' && stableKey == '\0') {
        stableKey = raw;
        return raw;
    }

    if (raw == '\0') {
        stableKey = '\0';
    }

    return '\0';
}

// ================= DISPLAY + SERIAL =================
void update_display()
{
    char line1[21];
    char line2[21];
    char line3[21];

    std::snprintf(line1, sizeof(line1), "Motor: %-10s", motor_text());
    std::snprintf(line2, sizeof(line2), "Mode : %-10s", mode_text());
    std::snprintf(line3, sizeof(line3), "Event: %-10s", lastEvent);

    lcd_show(
        "Tutorial 7 System ",
        line1,
        line2,
        line3
    );
}

void print_serial_status()
{
    char buffer[180];
    std::snprintf(buffer, sizeof(buffer),
                  "Motor=%s | Mode=%s | LastEvent=%s | Alarm=%s\r\n",
                  motor_text(),
                  mode_text(),
                  lastEvent,
                  alarmActive ? "ON" : "OFF");
    send_text(buffer);
}

// ================= MAIN =================
int main()
{
    lcd_i2c.frequency(100000);

    relayIN1 = RELAY_OFF;
    relayIN2 = RELAY_OFF;

    ledOpening = 0;
    ledClosing = 0;
    ledAlarm = 0;

    set_all_rows_high();

    intOpen.fall(&open_isr);
    intClose.fall(&close_isr);
    pirSensor.rise(&pir_isr);

    lcd_init();

    send_line("Tutorial 7 - Main Task");
    send_line("A = Open | B = Close | C = Stop | D = Mode");
    send_line("LCD SDA -> D14, LCD SCL -> D15");
    send_line("Relay logic = ACTIVE LOW");
    send_line("");

    motor_stop();
    update_display();

    Timer lcdTimer;
    Timer serialTimer;
    Timer autoTimer;
    Timer eventTimer;

    lcdTimer.start();
    serialTimer.start();
    eventTimer.start();

    while (true) {

        if (openFlag) {
            openFlag = false;
            motor_open();
            autoCycleActive = false;
            eventTimer.reset();
        }

        if (closeFlag) {
            closeFlag = false;
            motor_close();
            autoCycleActive = false;
            eventTimer.reset();
        }

        if (pirFlag) {
            pirFlag = false;
            alarmActive = true;
            ledAlarm = 1;
            eventTimer.reset();

            if (modeState == MODE_AUTO) {
                motor_open();
                autoTimer.reset();
                autoTimer.start();
                autoCycleActive = true;
            }
        }

        char key = get_keypad_key();

        if (key != '\0') {
            if (key == 'A') {
                motor_open();
                lastEvent = "KEY OPEN";
                autoCycleActive = false;
                eventFlash = true;
                eventTimer.reset();
            }
            else if (key == 'B') {
                motor_close();
                lastEvent = "KEY CLOSE";
                autoCycleActive = false;
                eventFlash = true;
                eventTimer.reset();
            }
            else if (key == 'C') {
                motor_stop();
                lastEvent = "KEY STOP";
                autoCycleActive = false;
                eventFlash = true;
                eventTimer.reset();
            }
            else if (key == 'D') {
                modeState = (modeState == MODE_MANUAL) ? MODE_AUTO : MODE_MANUAL;
                lastEvent = "KEY MODE";
                eventFlash = true;
                eventTimer.reset();
            }
            else if (key == '#') {
                alarmActive = false;
                ledAlarm = 0;
                lastEvent = "ALARM CLR";
                eventFlash = true;
                eventTimer.reset();
            }
        }

        if (autoCycleActive) {
            if (motorState == MOTOR_OPENING && autoTimer.elapsed_time() >= 3s) {
                motor_stop();
                thread_sleep_for(300);
                motor_close();
                autoTimer.reset();
            }
            else if (motorState == MOTOR_CLOSING && autoTimer.elapsed_time() >= 3s) {
                motor_stop();
                autoCycleActive = false;
            }
        }

        if (lcdTimer.elapsed_time() >= 250ms) {
            update_display();
            lcdTimer.reset();
        }

        if (serialTimer.elapsed_time() >= 1s) {
            print_serial_status();
            serialTimer.reset();
        }

        if (eventFlash && eventTimer.elapsed_time() >= 2s) {
            lastEvent = "NONE";
            eventFlash = false;
        }

        thread_sleep_for(20);
    }
}
