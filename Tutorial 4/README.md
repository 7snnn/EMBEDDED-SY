Tutorial 4: Main Task – Analog Sensors & Audible Alarms
Module: UFMFN3-30-2 Embedded Systems

Hardware: STM32 Nucleo-F439ZI, LM35 Temperature Sensor, Buzzer

1. Objective
The objective of this task was to interface an analog environmental sensor (LM35 Temperature Sensor) with the microcontroller and implement an automated hardware response. The system was designed to continuously read analog voltage, convert it into a Celsius temperature value, and trigger an audible alarm (Buzzer) when the temperature exceeded a defined safety threshold.

2. Logic and Implementation
This task shifted the focus from digital "On/Off" inputs to processing continuous analog signals.

Analog to Digital Conversion (ADC): The AnalogIn class was used to read the raw voltage from the LM35 sensor. Because the microcontroller uses a 3.3V reference, the raw reading was mathematically scaled (multiplied by 3.3 and then by 100) to convert the 10mV/°C output into a readable Celsius format.

Threshold Logic: A continuous if/else polling system was implemented to compare the calculated temperature against a hardcoded threshold (e.g., 35.0°C).

Audible Output: A DigitalOut object mapped to the buzzer was driven high (Logic 1) whenever the threshold was breached to simulate a safety alarm.

3. System Architecture (Code Summary)
Instead of relying on basic digital states, the architecture utilized the following Mbed OS classes to handle the continuous data stream:

AnalogIn tempSensor(A0); - To capture the fluctuating voltage from the LM35.

DigitalOut buzzer(D8); - To actuate the hardware alarm.

thread_sleep_for(500); - Used as a 500ms sampling delay to prevent terminal flooding while maintaining real-time responsiveness.

4. Hardware Testing & Validation
To ensure the reliability of the embedded system, physical stress tests were conducted on the individual components to validate the code's logic and the ADC (Analog-to-Digital Converter) conversions.

Potentiometer Calibration Test: To verify the analog reading logic independently of the environmental sensors, a potentiometer was temporarily connected to the analog input. Adjusting the dial successfully yielded a smooth, scaled data range from 55 to 67 in the terminal. This proved that the ADC scaling multipliers in the code were functioning correctly before connecting the real sensors.

Temperature Sensor (LM35) Test: The temperature threshold logic was validated by physically pressing/holding the LM35 sensor to apply body heat. The serial terminal successfully registered the real-time temperature spike. Once the readings crossed the hardcoded upper threshold, the microcontroller successfully engaged the DigitalOut pin, sounding the hardware buzzer.

Gas Sensor (MQ-2) Test: The gas monitoring loop was tested by exposing the MQ-2 sensor to a safe, simulated gas source. The sensor successfully detected the change in air quality and pulled the input pin to Logic 0. The system immediately recognized the state change and triggered the audible buzzer alarm, confirming the emergency response protocol works under real-world conditions.

5. Conclusion
This task successfully demonstrated the principles of reading both continuous analog signals and binary digital inputs, converting them into actionable, human-readable data. By rigorously testing the potentiometer, applying physical heat to the LM35, and triggering the MQ-2, the system proved it can reliably execute its safety protocols (terminal warnings and buzzer alarms) in real-time.
