Smart Home Security System (STM32 + Mbed OS)
Overview
This project implements a Smart Home Security System using the STM32 NUCLEO-F429ZI board and Mbed OS.
The system monitors temperature and gas levels using sensors and triggers an alarm when thresholds are exceeded. The alarm can be deactivated using a keypad-based PIN system. Events are logged with timestamps and can be viewed through the serial interface.

Features


4x4 keypad PIN authentication (default code: 1235)


Alarm system triggered by temperature or gas levels


Buzzer and LED activation during alarm


Dynamic threshold adjustment using a potentiometer


Temperature: 25°C to 37°C


Gas: 0 to 800 ppm




Real-Time Clock (RTC) event logging


Storage of the 5 most recent events


Serial monitor interface for system feedback


User controls:


Enter PIN to deactivate alarm


# to display event logs


D to toggle threshold display


* to clear entered code





Hardware Setup
Components


STM32 NUCLEO-F429ZI


4x4 Matrix Keypad


LM35 Temperature Sensor


MQ-2 Gas Sensor


Potentiometer


Active Buzzer


Breadboard and external power supply (MB102)



Pin Configuration
ComponentPinPotentiometerA0LM35A1MQ-2 (Analog)A2BuzzerPE_10LEDLED1
Keypad Connections
RowsColumnsPB_3, PB_5, PC_7, PA_15PB_12, PB_13, PB_15, PC_6
Important:


The MQ-2 sensor must use the analog output (AO) connected to A2.


The digital output (DO) is not used in this task.



System Behaviour
Alarm Activation
The alarm is triggered when:


Temperature exceeds the threshold
or


Gas level exceeds the threshold


Alarm Response


LED turns on


Buzzer activates


User is prompted to enter a 4-digit code


Alarm Deactivation


Correct PIN (1235) turns off the alarm


System waits for safe conditions before re-arming



Event Logging


The system stores the last 5 alarm events


Each event includes:


Timestamp (RTC)


Temperature


Gas level




Press # to display logs in the serial monitor



Example Output
Smart Home Security System ActivePIN code: 1235Press # to display event logThresholds -> Temp: 28 C | Gas: 210 ppm[ALERT] Enter 4-Digit Code to Deactivate****[SYSTEM] Alarm Cleared.

Notes


The MQ-2 gas sensor requires warm-up time and may show fluctuating values.


Sensor readings are smoothed in software to improve stability.


The buzzer is active-low (0 = ON, 1 = OFF).


Threshold display can be paused using the D key to improve usability during PIN entry.



Author
Hassan Aljadi
University of the West of England (UWE)
Electronic and Electrical Engineering
