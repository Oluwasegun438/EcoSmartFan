# EcoSmartFan
An IoT-enabled, energy-efficient smart fan system

# EcoSmart Fan 🌬️⚡

An IoT-enabled, energy-efficient smart fan system powered by ESP32, with automatic mode (sensor-based) and manual mode(web interface). Designed for optimized cooling and power efficiency.

## ✨ Features
- Dual Mode Operation
  - Automatic Mode: 
    - Fan turns ON only when human presence is detected (PIR sensor) AND temperature ≥ threshold.
    - Fan speed scales automatically with temperature:
      - 26–28 °C → Low
      - 28–30 °C → Medium
      - 30–32 °C → High
      - ≥ 32 °C → Max
    - Fan turns OFF when temperature ≤ 24 °C (hysteresis).
  - Manual Mode (IoT):
    - ESP32 runs a Wi-Fi Access Point with a web server.
    - Control fan remotely (OFF / Low / Med / High / Max).
    - Toggle between Auto and Manual modes via web page.

- User Interface
  - 16x2 LCD displays:
    - Current Mode (AUTO / MANUAL)
    - Temperature & Humidity
    - Presence Status (Detected / Not Detected)
  - Push button to switch between AUTO and MANUAL.

- Power Efficiency
  - Fan only runs when both temperature and presence criteria are satisfied.
  - Saves energy by preventing unnecessary operation.

## 🛠️ Components Used
- ESP32 DevKit V1
- BME280 Temperature & Humidity Sensor
- HC-SR501 PIR Sensor (Human Presence)
- 16x2 I2C LCD Display
- Push Button
- 4 Relays (for fan speed control) / 4 LEDs (for simulation)
- AC Fan (100–240V, 70W) – real hardware
- Jumper Wires, Veroboard, Power Supply

## 🔌 Pin Configuration (ESP32)
Component                 Pin   
BME280 SDA               21    
 BME280 SCL              22    
LCD SDA                  21    
LCD SCL                  22    
 PIR Sensor OUT          34    
Mode Button              12    
Relay 1 (Low)            16    
 Relay 2 (Med)           17    
Relay 3 (High)           18    
 Relay 4 (Max)           19    

## 📱 Web Interface
- Hosted on ESP32 (AP mode).
- Connect to Wi-Fi:
  - SSID: SmartFan_AP
  - Password: fan12345
- Open browser at `http://192.168.4.1`

Web Page Layout:
- Displays Mode, Temperature, Humidity, Presence, Active Fan Speed
- Buttons to switch Mode (AUTO/MANUAL)
- Manual Controls: OFF / Low / Med / High / Max

2. Open in Arduino IDE or PlatformIO.

3. Install required libraries:

Adafruit BME280
Adafruit Unified Sensor
LiquidCrystal_I2C
ESP32 WiFi

4. Upload code to ESP32.

5. Connect hardware

📊 Future Improvements

Add MQTT/Cloud integration (HiveMQ / ThingsBoard).
Adjustable temperature thresholds via web interface.
Mobile app integration (Blynk / Flutter).
Data logging to Firebase/ThingSpeak.

👨‍💻 Author

Adedayo Oluwasegun Dada

Graduate Researcher in Intelligent Systems & Embedded Robotics
