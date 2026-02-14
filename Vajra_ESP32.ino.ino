#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "BluetoothA2DPSource.h"

// CONFIGURATION 
const char* ssid = "O";
const char* password = "qwertyuiop";
const char* apiKey = "cd_glu_050226_ep-Ve_";
const char* templateID = "103"; 
const char* mobileNumber = "917972354266";

// PINS
int IN1 = 12; int IN2 = 13; // LEDs
int IN3 = 14; int IN4 = 27; // Motors
int pirPin = 33;

#define ADXL345_ADDR 0x53
#define POWER_CTL    0x2D
#define DATA_FORMAT  0x31
#define DATAX0       0x32

// State Variables
bool sequenceRunning = false;
unsigned long sequenceStart = 0;
unsigned long lastSmsTime = 0;
const unsigned long smsCooldown = 60000; // 1 minute cooldown to avoid 429 error

float prevX = 0, prevY = 0, prevZ = 0;
float vibrationThreshold = 0.15; // Slightly higher to avoid motor noise interference
bool playSound = false;

BluetoothA2DPSource a2dp_source;

//  AUDIO GENERATOR
int32_t get_sound_data(Frame* frame, int32_t frame_count) {
  static uint32_t t = 0;
  int16_t sample;
  for (int i = 0; i < frame_count; i++) {
    sample = (playSound && (t % 200 < 100)) ? 10000 : 0;
    frame[i].channel1 = frame[i].channel2 = sample;
    t++;
  }
  return frame_count;
}

//  SMS ALERT FUNCTION 
void sendAlertSMS() {
  // Prevent 429 Error (Too Many Requests)
  if (millis() - lastSmsTime < smsCooldown && lastSmsTime != 0) {
    Serial.println("⏳ Cooldown: Skipping SMS to avoid API rate limit.");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = "http://www.circuitdigest.cloud/api/v1/send_sms?ID=" + String(templateID);
    http.begin(serverPath);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", apiKey);

    String jsonPayload = "{\"mobiles\":\"" + String(mobileNumber) + 
                         "\",\"var1\":\"VAJRA_DEVICE\",\"var2\":\"Farm\"}";

    Serial.println("📡 Attempting to send SMS...");
    int responseCode = http.POST(jsonPayload);
    Serial.print("📡 Gateway Response: "); Serial.println(responseCode);
    
    if (responseCode == 200) {
      lastSmsTime = millis(); // Set cooldown start
    }
    http.end();
  }
}

//  SETUP 
void setup() {
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n--- VAJRA BOOTING UP ---");

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(pirPin, INPUT);

  WiFi.begin(ssid, password);
  
  Wire.begin(21, 22);
  Wire.beginTransmission(ADXL345_ADDR);
  if (Wire.endTransmission() == 0) {
    Wire.beginTransmission(ADXL345_ADDR); Wire.write(POWER_CTL); Wire.write(0x08); Wire.endTransmission();
    Wire.beginTransmission(ADXL345_ADDR); Wire.write(DATA_FORMAT); Wire.write(0x08); Wire.endTransmission();
    Serial.println("ADXL345 Detected.");
  } else {
    Serial.println("ADXL345 Error.");
  }

  a2dp_source.start("Target SB-88", get_sound_data);
  Serial.println("--- SYSTEM ARMED ---");
}

//  MAIN LOOP 
void loop() {
  // Only check sensors if the deterrent is NOT currently running
  if (!sequenceRunning) {
    bool pirTriggered = digitalRead(pirPin);

    // Read ADXL345
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(DATAX0);
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345_ADDR, 6, true);
    
    if (Wire.available() == 6) {
      int16_t xRaw = Wire.read() | (Wire.read() << 8);
      int16_t yRaw = Wire.read() | (Wire.read() << 8);
      int16_t zRaw = Wire.read() | (Wire.read() << 8);
      float x = xRaw * 0.0039; float y = yRaw * 0.0039; float z = zRaw * 0.0039;
      float vibration = abs(x - prevX) + abs(y - prevY) + abs(z - prevZ);
      prevX = x; prevY = y; prevZ = z;

      if (vibration > vibrationThreshold || pirTriggered) {
        Serial.println("⚠️ TRIGGER DETECTED!");
        startSequence();
      }
    }
  }

  // Handle the 50-second Deterrent Sequence
  if (sequenceRunning) {
    unsigned long elapsed = millis() - sequenceStart;

    // 1. LED Strobe (Runs full 50s)
    if (elapsed < 50000) {
      if ((elapsed / 200) % 2 == 0) {
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
      } else {
        digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
      }
    }

    // 2. Motor Return Logic
    if (elapsed < 25000) {
      // CW for 25s
      digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    } 
    else if (elapsed < 50000) {
      // CCW for 25s
      digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    } 
    else {
      // STOP & RESET
      digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
      playSound = false;
      sequenceRunning = false;
      Serial.println("--- RESETTING SENSORS ---");
    }
  }
  delay(50);
}

void startSequence() {
  sequenceRunning = true;
  sequenceStart = millis();
  playSound = true;
  sendAlertSMS();
}