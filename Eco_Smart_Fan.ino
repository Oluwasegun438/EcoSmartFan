/*
  Smart Fan Controller for ESP32
    - AUTO mode: Fan ON only when (PIR presence == true) AND (temperature >= TEMP_ON).
        Fan speed scales with temperature into 4 levels using 4 relays (only one relay active).
          - MANUAL mode: Web interface (AP mode) to turn fan OFF or select Low/Med/High/Max.
            - 16x2 I2C LCD displays Mode, Temp, Humidity, Presence.
              - BME280 for temp & humidity, HC-SR501 for PIR.

                  Libraries required:
                     - Adafruit BME280 Library
                        - Adafruit Unified Sensor
                           - LiquidCrystal_I2C
                              - WiFi, WebServer (ESP32 core)
*/

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>

// ---------- CONFIGURABLE PINS ----------
#define PIR_PIN        34   // HC-SR501 OUT
#define MODE_BTN_PIN   12   // Button to toggle AUTO/MANUAL (use internal pullup)
#define RELAY1_PIN     16   // Low speed (1.0uF)
#define RELAY2_PIN     17   // Medium-low
#define RELAY3_PIN     18   // Medium-high
#define RELAY4_PIN     19   // Direct / Max (no capacitor)
#define LED_AUTO_PIN   2    // Optional LED for mode indication

// ---------- SETTINGS ----------
const float TEMP_ON  = 26.0; // temp threshold to allow fan ON (AUTO)
const float TEMP_OFF = 24.0; // lower threshold to turn OFF (hysteresis)
const unsigned long SENSOR_INTERVAL_MS = 3000; // 3 seconds reading

// Web AP credentials (AP mode)
const char* apSSID = "SmartFan_AP";
const char* apPWD  = "fan12345"; // change if you want

// ---------- GLOBALS ----------
Adafruit_BME280 bme;                 // BME280 object
LiquidCrystal_I2C lcd(0x27, 16, 2);  // try 0x27 or 0x3F depending on your module
WebServer server(80);

enum Mode { MODE_AUTO, MODE_MANUAL };
enum FanSpeed { FAN_OFF = 0, FAN_LOW = 1, FAN_MED = 2, FAN_HIGH = 3, FAN_MAX = 4 };

Mode currentMode = MODE_AUTO;
FanSpeed manualSpeed = FAN_OFF;
FanSpeed autoSpeed = FAN_OFF;

bool presence = false;
float temperature = 0.0;
float humidity = 0.0;

unsigned long lastSensorMillis = 0;
unsigned long lastBtnMillis = 0;
const unsigned long DEBOUNCE_MS = 50;

// For button debouncing & edge detection
bool lastBtnState = HIGH; // using INPUT_PULLUP (active LOW)

// Hysteresis state
bool fanAllowedByTemp = false;

// ---------- HELPER FUNCTIONS ----------

void setAllRelaysOff() {
  digitalWrite(RELAY1_PIN, HIGH); // assuming relay is active LOW; adjust if yours is active HIGH
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);
}

void activateRelayForSpeed(FanSpeed s) {
  // Assumes active LOW relays (set LOW to energize). If your relay is active HIGH, flip levels.
  setAllRelaysOff();
  switch (s) {
    case FAN_LOW:  digitalWrite(RELAY1_PIN, LOW); break;
    case FAN_MED:  digitalWrite(RELAY2_PIN, LOW); break;
    case FAN_HIGH: digitalWrite(RELAY3_PIN, LOW); break;
    case FAN_MAX:  digitalWrite(RELAY4_PIN, LOW); break;
    default: /* all off */ break;
  }
}

FanSpeed mapTempToSpeed(float t) {
  if (t >= 32.0) return FAN_MAX;
  if (t >= 30.0) return FAN_HIGH;
  if (t >= 28.0) return FAN_MED;
  if (t >= 26.0) return FAN_LOW;
  return FAN_OFF;
}

String fanSpeedToText(FanSpeed s) {
  switch (s) {
    case FAN_LOW:  return "LOW";
    case FAN_MED:  return "MED";
    case FAN_HIGH: return "HIGH";
    case FAN_MAX:  return "MAX";
    default:       return "OFF";
  }
}

// ---------- WEB SERVER HANDLERS ----------
String htmlHeader = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'/><title>Smart Fan</title>"
                    "<style>body{font-family:Arial;margin:12px;}button{padding:10px;margin:6px;} .big{font-size:1.2em}</style></head><body>";

void handleRoot() {
  String html = htmlHeader;
  html += "<h2>Smart Fan Controller</h2>";
  html += "<p><strong>Mode:</strong> " + String(currentMode == MODE_AUTO ? "AUTO" : "MANUAL") + "</p>";
  html += "<p><strong>Temperature:</strong> " + String(temperature, 1) + " &deg;C<br>";
  html += "<strong>Humidity:</strong> " + String(humidity, 1) + " %<br>";
  html += "<strong>Presence:</strong> " + String(presence ? "Detected" : "Not Detected") + "</p>";
  html += "<p><strong>Active Speed:</strong> " + fanSpeedToText( (currentMode == MODE_MANUAL) ? manualSpeed : autoSpeed ) + "</p>";
  html += "<hr>";
  html += "<form action='/setmode' method='POST'><button name='mode' value='AUTO' type='submit'>Set AUTO</button>"
          "<button name='mode' value='MANUAL' type='submit'>Set MANUAL</button></form>";
  html += "<h3>Manual Controls</h3>";
  html += "<form action='/manual' method='POST'>"
          "<button name='speed' value='0' class='big'>OFF</button>"
          "<button name='speed' value='1' class='big'>LOW</button>"
          "<button name='speed' value='2' class='big'>MED</button>"
          "<button name='speed' value='3' class='big'>HIGH</button>"
          "<button name='speed' value='4' class='big'>MAX</button>"
          "</form>";
  html += "<hr><p>Connect to this AP (SSID: <b>" + String(apSSID) + "</b>) to control the fan.</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSetMode() {
  if (server.hasArg("mode")) {
    String m = server.arg("mode");
    if (m == "AUTO") {
      currentMode = MODE_AUTO;
    } else if (m == "MANUAL") {
      currentMode = MODE_MANUAL;
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleManualSet() {
  if (server.hasArg("speed")) {
    int s = server.arg("speed").toInt();
    if (s >= 0 && s <= 4) {
      manualSpeed = (FanSpeed)s;
      // If manual speed is off, deactivate all relays
      if (currentMode == MODE_MANUAL) {
        if (manualSpeed == FAN_OFF) setAllRelaysOff();
        else activateRelayForSpeed(manualSpeed);
      }
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(100);

  // Pin modes
  pinMode(PIR_PIN, INPUT);
  pinMode(MODE_BTN_PIN, INPUT_PULLUP); // button to ground when pressed (active LOW)
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  pinMode(LED_AUTO_PIN, OUTPUT);

  // default relays OFF (assuming active LOW)
  setAllRelaysOff();

  // init LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Fan Init...");

  // init BME280
  if (!bme.begin(0x76)) { // try 0x76 or 0x77 depending on your sensor
    Serial.println("BME280 not found at 0x76, trying 0x77...");
    if (!bme.begin(0x77)) {
      Serial.println("BME280 not detected - check wiring!");
      lcd.setCursor(0, 1);
      lcd.print("BME280 Error");
      // continue but temp/hum will be 0
    }
  }

  // Start WiFi AP
  WiFi.softAP(apSSID, apPWD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Setup web server pages
  server.on("/", handleRoot);
  server.on("/setmode", HTTP_POST, handleSetMode);
  server.on("/manual", HTTP_POST, handleManualSet);
  server.begin();
  Serial.println("HTTP server started");

  lastSensorMillis = millis() - SENSOR_INTERVAL_MS; // force immediate read
}

// ---------- MAIN LOOP ----------
void loop() {
  server.handleClient();

  unsigned long now = millis();

  // Read button (toggle mode) with debouncing
  bool btnReading = digitalRead(MODE_BTN_PIN);
  if (btnReading != lastBtnState) {
    lastBtnMillis = now;
  }
  if ((now - lastBtnMillis) > DEBOUNCE_MS) {
    // stable
    if (btnReading == LOW && lastBtnState == HIGH) {
      // button pressed (active LOW)
      // Toggle mode
      currentMode = (currentMode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
      Serial.println(String("Mode toggled to ") + (currentMode == MODE_AUTO ? "AUTO" : "MANUAL"));
      // If switching to AUTO, clear manual outputs
      if (currentMode == MODE_AUTO) {
        manualSpeed = FAN_OFF;
      }
    }
  }
  lastBtnState = btnReading;

  // Read sensors periodically
  if (now - lastSensorMillis >= SENSOR_INTERVAL_MS) {
    lastSensorMillis = now;
    // BME280
    if (bme.begin(0x76) || bme.begin(0x77) || true) { // using bme if already initialized - defensive
      // use safe reads with try/catch style (library doesn't throw)
      temperature = bme.readTemperature();
      humidity = bme.readHumidity();
    }
    // PIR
    presence = digitalRead(PIR_PIN) == HIGH;

    // Update intelligent control logic
    if (currentMode == MODE_AUTO) {
      // Check hysteresis for allowed-by-temp
      if (!fanAllowedByTemp) {
        if (temperature >= TEMP_ON) fanAllowedByTemp = true;
      } else {
        if (temperature <= TEMP_OFF) fanAllowedByTemp = false;
      }

      if (fanAllowedByTemp && presence) {
        // choose auto speed
        autoSpeed = mapTempToSpeed(temperature);
        if (autoSpeed == FAN_OFF) {
          setAllRelaysOff();
        } else {
          activateRelayForSpeed(autoSpeed);
        }
      } else {
        // Not allowed by temp or no presence -> fan OFF
        autoSpeed = FAN_OFF;
        setAllRelaysOff();
      }
    } else {
      // MANUAL mode => obey manualSpeed
      if (manualSpeed == FAN_OFF) {
        setAllRelaysOff();
      } else {
        activateRelayForSpeed(manualSpeed);
      }
    }

    // Update LCD
    lcd.clear();
    String modeText = (currentMode == MODE_AUTO) ? "AUTO" : "MANUAL";
    lcd.setCursor(0, 0);
    lcd.print(modeText);
    lcd.setCursor(6, 0);
    lcd.print(String(temperature, 1) + "C");

    lcd.setCursor(0, 1);
    lcd.print(String(humidity, 0) + "% ");
    lcd.setCursor(6, 1);
    lcd.print(presence ? "PRES" : "-----");
    // optionally show speed on LCD in small delay
    delay(50);

    // Debug prints
    Serial.print("Mode: "); Serial.print(currentMode == MODE_AUTO ? "AUTO" : "MANUAL");
    Serial.print(" | Temp: "); Serial.print(temperature, 1);
    Serial.print("C | Hum: "); Serial.print(humidity, 1);
    Serial.print("% | Presence: "); Serial.print(presence ? "Y" : "N");
    Serial.print(" | FanSpeed: "); Serial.println( (currentMode == MODE_AUTO) ? fanSpeedToText(autoSpeed) : fanSpeedToText(manualSpeed) );
  }

  // LED indicate AUTO mode
  digitalWrite(LED_AUTO_PIN, (currentMode == MODE_AUTO) ? HIGH : LOW);

  // small loop delay
  delay(10);
}