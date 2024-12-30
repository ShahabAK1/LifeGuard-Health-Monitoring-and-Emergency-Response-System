LifeGuard: Health Monitoring and Emergency Response System 🚑💓
"Empowering real-time health monitoring with intelligent emergency response."

LifeGuard is a cutting-edge, IoT-enabled health monitoring system designed to provide real-time insights and proactive emergency alerts for heart patients and accident scenarios. This system leverages advanced sensors, microcontrollers, and cloud integration to monitor critical health parameters, visualize data interactively, and initiate life-saving responses.

🌟 Key Features
1. Real-Time Health Monitoring
Tracks heart rate, oxygen saturation (SpO2), and motion data using:
MPU6050 Accelerometer (x2) for motion and fall detection.
MAX30102 Oximeter & Heart Rate Sensor for pulse and SpO2 monitoring.
Light Sensor for environmental analysis.
2. Emergency Alerts
Abnormal Heart Rate Detection: Instantly sends Telegram alerts to caregivers or emergency contacts.
Designed for critical heart patients and accident scenarios to ensure timely intervention.
3. Data Visualization
Integrates with Power BI for interactive dashboards to analyze real-time and historical data trends.
Easy-to-read insights for monitoring patient status over time.
4. IoT Integration
Uses ESP32 for seamless wireless communication to a remote server.
Flask API to manage data storage and communication.
5. Scalability and Future-Proofing
Designed for future enhancements:
GPS Integration for location tracking.
Automated Emergency Calls to nearby hospitals or police stations.
📊 Project Architecture
The LifeGuard system consists of three interconnected modules:

Data Collection:

STM32F401RE gathers sensor data from MPU6050 (x2) and Light Sensor, forwarding it to the ESP32 via UART.
ESP32 collects heart rate data from MAX30102.
Data Transmission and Storage:

ESP32 sends aggregated sensor data to a Flask server over Wi-Fi.
The server stores the data in an SQL database.
Data Visualization and Alerts:

Power BI Dashboards for monitoring trends and real-time metrics.
Telegram Alerts for emergency conditions.
💡 Use Cases
Healthcare for Heart Patients:
Early detection of heart abnormalities and timely alerts.
Accident Detection and Response:
Real-time fall or collision detection with immediate alerts.
Elderly Care:
Remote monitoring of elderly individuals' health and safety.
🚀 Technologies Used
Microcontrollers and Sensors:

STM32F401RE
ESP32
MAX30102 Oximeter & Heart Rate Sensor
MPU6050 Accelerometer (x2)
Light Sensor
Programming and Development:

C for STM32
Arduino IDE for ESP32
Python (Flask API)
Data Visualization:

Power BI for interactive dashboards
Communication Protocols:

UART for STM32 to ESP32
HTTP for ESP32 to Flask Server
Emergency Alerts:

Telegram API for instant notifications
🔧 Setup Instructions
Hardware Setup:

Connect sensors (MPU6050, MAX30102, Light Sensor) to STM32 and ESP32 as per the wiring diagram (see Docs/WiringDiagram.png).
Firmware Deployment:

Flash STM32 code (STM32_Code/main.c) to the STM32F401RE using STM32CubeIDE.
Flash ESP32 code (ESP32_Code/main.ino) using the Arduino IDE.
Server Setup:

Install Python dependencies:

pip install flask pyodbc
Run the Flask server:
python Server_Code/app.py
Power BI Setup:

Import PowerBI/Dashboard.pbix into Power BI Desktop to visualize data.
Testing:

Monitor the system using Power BI dashboards.
Verify Telegram alerts for abnormal heart rate conditions.
🛠️ Future Work
Add GPS integration for precise location tracking in emergencies.
Implement voice call functionality to contact emergency services automatically.
Expand data visualization capabilities with predictive analytics.
