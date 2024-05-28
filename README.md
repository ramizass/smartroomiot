# Introduction
Smart Room is an intelligent solution that uses advanced technology to monitor temperature, air quality and fire potential in real time to improve the safety and comfort of occupants under various conditions.
1. The temperature is high, it doesn't detect a fire and the air quality is still good so only the fan is on.
2. Low temperature, no fire detection and poor air quality means the fan will turn on and the window will open.
3. High temperature, detecting fire, and poor air quality then only the window is open and the buzzer is on.

# Tools
The microcontroller used is the ESP8266 which is integrated with WiFi which functions to control and process various components.
1. Temperature and Humidity Sensor (DHT22)
2. Fire Sensor
3. Air Quality Sensor (MQ-2)
4. Motor Servo
5. Mini Fan
6. Buzzer
7. LCD

# Block Diagram
![image](https://github.com/ramizass/smartroomiot/assets/88464165/7d4163f1-e12b-41b8-b1ef-8452bd7d320c)

1. Temperature and Humidity Sensors will continue to monitor environmental conditions. When the temperature rises above a certain limit, the sensor detects this change.
2. The Fire Sensor will continue to monitor for fires or abnormal temperatures. When the sensor detects a fire or temperature that has the potential to cause a fire, a signal will be sent to the system.
3. Air quality sensors will be used to monitor the quality of the surrounding air. If air quality worsens, the sensor will provide warnings or information necessary so that action can be taken according to the conditions that are occurring.
4. Data Processing Process: Signals from these sensors will be processed by the microcontroller installed in the system. The processor will evaluate the data received and make decisions based on temperature conditions, air quality and also fire detection.
5. Servo Settings: If the two DHT22 and MQ-2 sensors detect a temperature above 30°C and there is a fire, the servo will move to direct something and the buzzer will sound. The example used in this tool is that the servo will open the window automatically.
6. Fan Settings: If the temperature sensor is above 30°C without a fire being detected or detects poor air quality without a fire being detected, the fan will be activated to help cool the surrounding environment.
7. Entering Data into the Database: The results of the data processing process will be entered into the database using Firebase which uses WiFi connectivity to be processed and displayed in the application.
8. Display in Application: will use an application which will display temperature & humidity, air quality and fire detection in Real-Time and will display a notification if it detects fire.
