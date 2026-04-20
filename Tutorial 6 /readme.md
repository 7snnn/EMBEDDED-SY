Smart Home Security System – Tutorial 6

Module: Embedded Systems (UFMFN3-30-2)
Platform: STM32 Nucleo-F439ZI (Mbed OS)

Overview

This project implements a Smart Home Security System using analog sensors, a keypad interface, an LCD display, and an audible alarm.

The system continuously monitors temperature and gas levels, triggers an alarm when thresholds are exceeded, and allows the user to silence the alarm using a secure code (18888).

Features
Real-time temperature monitoring (LM35 sensor)
Real-time gas detection (MQ-2 sensor)
Automatic alarm activation when unsafe conditions occur
Secure keypad input to deactivate alarm
LCD interface with multiple screens:
System status
Gas monitoring
Temperature monitoring
Code entry
Alarm state
Visual feedback (LED) and audible buzzer
Sensor calibration and noise reduction filtering
Hardware Components
STM32 Nucleo-F439ZI
LM35 Temperature Sensor (Analog)
MQ-2 Gas Sensor (Analog Output)
4x4 Matrix Keypad
I2C 20x4 LCD (PCF8574 module)
Buzzer (Active-low)
LED (Alarm indicator)
Pin Configuration
Component	Pin
LM35	A1
MQ-2 AO	A2
LCD SDA	PB_9
LCD SCL	PB_8
Buzzer	PE_10
LED	LED1
Keypad	Mixed GPIO (Rows/Columns)
System Behaviour
Normal Operation
Displays temperature and gas values
System remains in safe state
Buzzer is OFF
Warning Condition

Triggered when:

Temperature > 40°C
Gas level ≥ 20

System response:

Buzzer activates
LED turns ON
LCD switches to warning/code entry screen
Keypad Controls
Key	Function
4	View Gas Screen
5	View Temperature Screen
#	Return to main screen
*	Clear entered code
0–9	Enter code digits
Alarm Deactivation
Enter the correct code: 18888
LCD displays ***** during input
If correct:
Alarm stops
System enters silenced mode
If incorrect:
Error message displayed
Sensor Processing
Temperature
ADC scaled to Celsius
Startup calibration applied
Multiple samples averaged
Sudden spikes filtered
Gas
Analog readings scaled
Immediate threshold detection (≥20)
Additional smoothing for display only
LCD Interface

The LCD uses:

I2C communication
Selective updates (only when values change)
Timed refresh (250 ms)

This minimizes:

Flickering
Noise
Unstable display behaviour
Code Structure

The system is implemented in a modular and readable format:

Sensor reading and filtering
Keypad scanning with debouncing
State-based UI system
Alarm logic control
LCD update management
Known Improvements (Future Work)
Add EEPROM storage for user-defined codes
Adjustable thresholds via keypad
Logging of events (alarm history)
Mobile or wireless integration
Conclusion

This project demonstrates a complete embedded system integrating:

Analog sensing
Real-time decision making
User interaction
Hardware control

The system is stable, responsive, and suitable for safety monitoring applications
