#include <Adafruit_GFX.h>         // For OLED display
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128          // OLED display width, in pixels
#define SCREEN_HEIGHT 64          // OLED display height, in pixels
#define OLED_RESET -1             // Reset pin (not used with I2C)
#define SSD1306_I2C_ADDRESS 0x3C  // OLED I2C address

#define RX_PIN 16  // RX pin for UART (adjust if needed)
#define TX_PIN 17  // TX pin for UART (adjust if needed)

#define WIFI_SSID "Internet_2.4GHz@unifi"
#define WIFI_PASSWORD "1to5&6to10"
#define SERVER_URL "http://192.168.0.101:5000/insertData"  // Replace with your server URL
#define TELEGRAM_TOKEN "7535459496:AAGkqansj70kNvMW_hz9qW0R8NcxOhxN1eA"
#define CHAT_ID "-4632205310"
 

// Declare beatsPerMinute and beatAvg globally
float beatsPerMinute = 0; // Current BPM
int beatAvg = 0;  

const byte RATE_SIZE = 4;  // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred

const int IR_THRESHOLD = 50000;  // Initial threshold for IR value detection
const int MIN_STABLE_IR_COUNT = 10;  // Minimum number of stable readings before detecting a valid finger
int stableIRCount = 0;
float irValueSmoothed = 0.0;
float heartRateFromSensor = 0; 
int lightIntensity = 0;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;


char uart_buffer[512];  // Increased buffer size to handle larger JSON
int uart_len = 0;

HardwareSerial mySerial(1);

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1000;

// Real-time sensor values (updated from UART)
float accelX1 = 0.0, accelY1 = 0.0, accelZ1 = 0.0;
float accelX2 = 0.0, accelY2 = 0.0, accelZ2 = 0.0;
float temp1 = 0.0, temp2 = 0.0; // Placeholder for temperature readings

void setup() {

    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
   }
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");



  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  sendTelegramMessage("Health Monitoring System is online.");
}

void loop() {
  monitorHeartRate();
  readUARTData();
  updateDisplay();

  if (millis() - lastSendTime >= sendInterval) {
    sendDataToServer();
    lastSendTime = millis();
  }
}

void monitorHeartRate() {
    long irValue = particleSensor.getIR();

    // Reduce smoothing for quicker reaction
    irValueSmoothed = 0.5 * irValueSmoothed + 0.5 * irValue;

    if (irValueSmoothed > IR_THRESHOLD) {
        stableIRCount++;  // Increment count if IR value is above threshold
    } else {
        stableIRCount = 0;  // Reset count if IR value is too low
    }

    // Lower the stable count threshold for faster detection
    if (stableIRCount >= 3) {  // Adjust this value for quicker reaction
        if (checkForBeat(irValueSmoothed)) {
            long delta = millis() - lastBeat;
            lastBeat = millis();

            beatsPerMinute = 60 / (delta / 1000.0);

            if (beatsPerMinute < 255 && beatsPerMinute > 20) {
                rates[rateSpot++] = (byte)beatsPerMinute;
                rateSpot %= RATE_SIZE;

                beatAvg = 0;
                for (byte x = 0; x < RATE_SIZE; x++) {
                    beatAvg += rates[x];
                }
                beatAvg /= RATE_SIZE;
            }
        }
    } else {
        // No stable IR value, reset BPM and average
        beatsPerMinute = 0;
        beatAvg = 0;
    }

    // Alert logic for abnormal BPM
    static bool alertSent = false;
    if (stableIRCount >= 3) {  // Only when a finger is detected
        if ((beatAvg < 50 || beatAvg > 120) && !alertSent) {
            sendTelegramMessage("Alert: Abnormal heart rate detected: " + String(beatAvg));
            alertSent = true;  // Avoid repeated messages
        } else if (beatAvg >= 50 && beatAvg <= 120) {
            alertSent = false;  // Reset alert flag for normal readings
        }
    } else {
        // No valid reading, reset alert flag
        alertSent = false;
    }
}


void readUARTData() {
  if (mySerial.available() > 0) {
    uart_len = mySerial.readBytesUntil('\n', uart_buffer, sizeof(uart_buffer) - 1);
    uart_buffer[uart_len] = '\0';
    Serial.println("Received UART data: " + String(uart_buffer));

    // Parse the received JSON data
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, uart_buffer);
    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Extract Sensor1 and Sensor2 data
    accelX1 = doc["Sensor1"]["Accel_X"];
    accelY1 = doc["Sensor1"]["Accel_Y"];
    accelZ1 = doc["Sensor1"]["Accel_Z"];
    temp1 = doc["Sensor1"]["Temp"];

    accelX2 = doc["Sensor2"]["Accel_X"];
    accelY2 = doc["Sensor2"]["Accel_Y"];
    accelZ2 = doc["Sensor2"]["Accel_Z"];
    temp2 = doc["Sensor2"]["Temp"];

    lightIntensity = doc["Light"];
  }
}


void sendDataToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Prepare the JSON payload
    DynamicJsonDocument doc(1024);
    doc["Sensor1_ID"] = 1;
    doc["AccelX1"] = accelX1;
    doc["AccelY1"] = accelY1;
    doc["AccelZ1"] = accelZ1;
    doc["Temp1"] = temp1;

    doc["Sensor2_ID"] = 2;
    doc["AccelX2"] = accelX2;
    doc["AccelY2"] = accelY2;
    doc["AccelZ2"] = accelZ2;
    doc["Temp2"] = temp2;

    doc["HeartRate_ID"] = 3;
    if (beatsPerMinute == 0) {
      doc["BeatsPerMinute"] = 0;
      doc["AverageBPM"] = 0;
    } else {
      doc["BeatsPerMinute"] = beatsPerMinute;
      doc["AverageBPM"] = beatAvg;
    }

    doc["LightSensor_ID"] = 4;
    doc["LightIntensity"] = lightIntensity;

    // Serialize the JSON to string
    String jsonStr;
    serializeJson(doc, jsonStr);

    // Send the POST request with JSON payload
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonStr);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}


void updateDisplay() {
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setCursor((SCREEN_WIDTH - 120) / 2, 0);  // Center the header
  display.println("Health Monitoring");

  // Sensor 1 Data (Accelerometer 1)
  display.setCursor(0, 13);
  display.printf("S1 X=%.2f Y=%.2f Z=%.2f", accelX1, accelY1, accelZ1);

  // Sensor 2 Data (Accelerometer 2)
  display.setCursor(0, 27);
  display.printf("S2 X=%.2f Y=%.2f Z=%.2f", accelX2, accelY2, accelZ2);

  // Sensor 1 Temperature
  display.setCursor(0, 45);
  display.printf("BPM=%.0f ", beatsPerMinute);

  // Sensor 2 Temperature
  display.setCursor(0, 55);  
  display.printf("Avg=%.0f", beatAvg);

  display.display();
}

void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String telegramUrl = "https://api.telegram.org/bot" TELEGRAM_TOKEN "/sendMessage?chat_id=" CHAT_ID "&text=" + message;

    http.begin(telegramUrl);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("Telegram message sent.");
    } else {
      Serial.println("Failed to send Telegram message.");
    }
    http.end();
  }
}
