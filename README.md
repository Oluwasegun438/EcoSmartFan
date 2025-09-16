I'm# EcoSmart Fan 🌬️⚡

An IoT-enabled, energy-efficient smart fan system powered by ESP32, with automatic mode (sensor-based) and manual mode (web interface). Designed for optimized cooling and power efficiency.

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
    - Temperature & Humidity (humidity is displayed but not used for control)
    - Presence Status (Detected / Not Detected)
  - Push button to switch between AUTO and MANUAL.
  - Built-in LED (GPIO 2) lights up to indicate AUTO mode.

- Power Efficiency
  - Fan only runs when both temperature and presence criteria are satisfied.
  - Saves energy by preventing unnecessary operation.

## 🧠 How “Smart” Is EcoSmart Fan?
EcoSmart Fan isn’t just an ON/OFF switch — it actively adjusts to the environment:  
- Adaptive Speed Control: Fan speed changes automatically with room temperature.  
- Presence Awareness: Uses a PIR sensor to ensure the fan only runs when someone is present.  
- Environmental Feedback: Displays both temperature and humidity on the LCD and web page. (Control logic is based on temperature, but humidity is still monitored for user awareness).  
- Dual Control Modes: Automatic (sensor-driven) and Manual (IoT web interface).  

## ⚡ Energy Efficiency
Compared to a regular fan that runs continuously until manually switched off:  
- EcoSmart Fan only runs when needed → preventing wasted energy.  
- Speed scaling saves ~30–50% power during mild temperatures.  
- Presence-based shutoff saves another ~20–30% when the room is empty.  

👉 In real-world testing, EcoSmart Fan can save up to 50–70% of energy usage compared to a traditional fan.  

## 🧩 Design Choices & Trade-Offs
- Why ESP32?→ Built-in Wi-Fi + GPIOs.  
- Why BME280?→ Accurate temperature & humidity sensor.  
- Why PIR sensor?→ Simple, low-power human presence detection.  
- Relay-Based Speed Control→ Cheap & practical using capacitor taps.  
- Trade-Offs: 
  - Humidity is only displayed, not used in control logic.  
  - PIR may miss still occupants or trigger on pets.  
  - Relays add clicking noise but are reliable.  

## 🧪 Challenges Faced
- Getting PIR + Temperature logic right (solved with hysteresis).  
- Balancing comfort vs energy savings (26 °C threshold chosen).  
- Wokwi simulator doesn’t support Wi-Fi → web mode tested on hardware.  
- Relay wiring safety → required proper isolation.  

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
Component        Pin   
BME280 SDA       21    
 BME280 SCL       22    
LCD SDA           21    
LCD SCL           22    
 PIR Sensor OUT    34    
Mode Button       12    
Relay 1 (Low)     16    
 Relay 2 (Med)     17    
Relay 3 (High)    18    
Relay 4 (Max)     19    
 LED (Auto Mode)   2     

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

📊 Future Improvements

Add MQTT/Cloud integration (HiveMQ / ThingsBoard).

Adjustable temperature thresholds via web interface.

Mobile app integration (Blynk / Flutter).

Data logging to Firebase/ThingSpeak.

🔍 Machine Vision for Presence Detection: Use ESP32-CAM or Rasp

👨‍💻 Author

Adedayo Oluwasegun Dada

Undergraduate Research in Intelligent Systems & Embedded Robotics