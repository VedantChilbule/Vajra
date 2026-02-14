# Project Vajra: Autonomous Non-Lethal Animal Deterrent System

Vajra is an intelligent, solar-powered solution designed to mitigate human-wildlife conflict in agriculture. It protects crops from damage caused by animals ranging from rats to elephants using a multimodal, adaptive approach.

## 🚀 Key Features
- **Multimodal Deterrence:** Combines strobe lights (150m range), 175 kHz ultrasonic sound, scent modules, and seismic stimuli.
- **Adaptive Height:** Integrated linear actuator extends the device up to 4 meters to prevent animal habituation and clear tall crops.
- **Intelligent Sensing:** Uses a Geophone to detect seismic vibrations up to 50m and a 360° rotating sensing head to eliminate blind spots.
- **Human-Animal Classification:** Software logic differentiates between human and animal movement to ensure safety.
- **Self-Sustaining:** Powered by a 20-watt solar panel for off-grid autonomous operation.

## 🛠️ Technology Stack
- **Hardware:** ESP32 Dev Board, Geophone (Seismic Sensor), PIR Sensor, Linear Actuator, 175 kHz Ultrasonic Transducer, 20W Solar Panel.
- **Software:** Arduino Framework, C++, I2C (Wire.h), Bluetooth A2DP.
- **Libraries:** WiFi.h, HTTPClient.h, BluetoothA2DPSource.h.

## 🔧 Installation
1. Clone the repository: `git clone https://github.com/yourusername/Project-Vajra.git`
2. Open the `/src` folder in Arduino IDE or VS Code (PlatformIO).
3. Install required libraries (Wire, BluetoothA2DP).
4. Flash the code to your ESP32.
