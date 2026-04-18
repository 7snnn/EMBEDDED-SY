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

4. Technical Reflection (Errors & Challenges)
Integrating the analog hardware introduced a unique set of physical challenges that severely affected the expected output:

Hardware Degradation (The Burned Sensor): During the initial wiring, my LM35 sensor became damaged (likely due to a temporary short or reverse polarity). As a result, the sensor's baseline readings were abnormally high, sitting constantly near or above the standard room temperature threshold.

The Faulty Buzzer: The buzzer provided for the circuit was also faulty, producing an inconsistent tone rather than a clean beep.

Troubleshooting Success: Despite the LM35 being damaged and reading artificially high, the underlying logic of my code was still correct. By applying an external heat source, the sensor did successfully register the temperature spike, cross the upper threshold, and successfully trigger the faulty buzzer to sound the alarm. This proved that the ADC conversion and threshold logic were perfectly functional, even if the physical components were degraded.

5. Conclusion
This task successfully demonstrated the principles of reading analog signals and converting them into human-readable data. Dealing with a burned sensor and a faulty buzzer provided a highly realistic hardware debugging scenario, proving that robust code can still execute its intended safety protocols (triggering the alarm) even when physical components are compromised.
