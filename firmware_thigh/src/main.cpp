#include <Arduino.h>
#include <Wire.h>
#include <MAX30105.h>
#include "heartRate.h"

#include <WiFi.h>
#include <HTTPClient.h>

#include "time.h"


// ---------------------------------------
// --- set up pulse oximiter
// ---------------------------------------
// MAX30105 particleSensor;     // create object

// const byte RATE_SIZE = 4;         // number of readings to average
// byte rates[RATE_SIZE];            // array of heart rate values
// byte rateSpot = 0;

// // heartbeat sample timing
// unsigned long lastIRsample = 0;
// const unsigned long IR_SAMPLE_INTERVAL_MS = 10; 
// long lastIRvalue = 0;

// long lastBeat = 0;                // time of last beat
// float beatsPerMinute = 0;
// int beatAvg = 0;
// const unsigned long BPM_TIMEOUT_MS = 3000; // reset bpm after 3s of no bpm


// ---------------------------------------
// --- set up accelerometer readings
// ---------------------------------------
// analog pin setup
const int pinX = 1;
const int pinY = 3;
const int pinZ = 0;

// ADXL335 calibration constants
const float zeroG_X = 1.37f;    // zero-g voltages for all axes
const float zeroG_Y = 1.37f;
const float zeroG_Z = 1.42f;

const float sensX = 0.275f;    // v per g for all axes
const float sensY = 0.270f;
const float sensZ = 0.285f;

// 12-bit ADC conversion
const float ADC_RES = 4095.0;
const float VREF = 3.3;

// --- printing interval setup for vscode plotter
unsigned long lastRead = 0;
const unsigned long readInterval = 100;  // 20 hz output

// ---------------------------------------
// --- helper functions: 
// ---------------------------------------

// convert raw adc to voltage
inline float adcToVoltage(int raw) {
    return (raw * VREF) / ADC_RES;
}

// axis conversion
float convertX(int raw) { return (adcToVoltage(raw) - zeroG_X) / sensX; }
float convertY(int raw) { return (adcToVoltage(raw) - zeroG_Y) / sensY; }
float convertZ(int raw) { return (adcToVoltage(raw) - zeroG_Z) / sensZ; }


// --- standing detection algorithm
bool isStanding = false;

void updateStandingState(float gx, float gy, float gz) {
    float gMag = sqrtf(gx*gx + gy*gy + gz*gz);
    float gzNorm = gz / gMag;

    const float STAND_THRESH = -0.75; // entering standing
    const float SIT_THRESH   = -0.65; // leaving standing

    if (isStanding) {
        if (gx > SIT_THRESH) {
            isStanding = false;
        }
    } else {
        if (gx < STAND_THRESH) {
            isStanding = true;
        }
    }
}

// --- standing percentage tracking ---
// 5 minutes = 300000ms
// 100 ms per sample = 3000 samples
const int WINDOW_SIZE = 50;
bool standingHistory[WINDOW_SIZE];
int historyIndex = 0;
int standingCount = 0;      // number of true values in window

unsigned long lastSample = 0;
const unsigned long SAMPLE_INTERVAL = 100; // ms (same as print interval)


// -- update window of standing data
void updateStandingWindow() {
  unsigned long now = millis();
  if (now - lastSample < SAMPLE_INTERVAL) return;
  lastSample = now;

  // Remove old value from count
  if (standingHistory[historyIndex]) {
      standingCount--;
  }

  // Insert new value
  standingHistory[historyIndex] = isStanding;
  if (isStanding) {
      standingCount++;
  }

  // Move index forward circularly
  historyIndex++;
  if (historyIndex >= WINDOW_SIZE) {
      historyIndex = 0;
  }
}

// --- compute standing percentage
float getStandingPercentage() {
    return (standingCount * 100.0f) / WINDOW_SIZE;
}


// timer for sending to backend
unsigned long lastSend = 0;
// const unsigned long SEND_INTERVAL = 5UL * 60UL * 1000UL; // 5 min
const unsigned long SEND_INTERVAL =  5UL * 1000UL; // 5 seconds

// -- wifi setup
const char* ssid = "Ethans phone";
const char* password = "ceeitch123";

String endpoint = "http://172.20.10.3:3000/patients/69164212a337ec14e57f5b1d/data";

void connectWiFi() {
    Serial.println("\n[WiFi] Connecting...");

    WiFi.mode(WIFI_STA);           // MUST be before anything else on ESP32-C3
    WiFi.setHostname("esp32c3_1");   // Prevents stuck state on some hotspots

    WiFi.disconnect();             // soft disconnect, NOT WiFi.disconnect(true)
    delay(200);

    WiFi.begin(ssid, password);

    unsigned long startAttempt = millis();
    const unsigned long WIFI_TIMEOUT = 8000; // 8 sec max

    // while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_TIMEOUT) {
    //   Serial.print(".");
    //   delay(500);
    // }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WiFi] Connected!");
      Serial.print("[WiFi] IP: ");
      Serial.println(WiFi.localIP());
      return;
    }

    // FAILED
    Serial.println("\n[WiFi] Failed. Resetting WiFi stack...");

    WiFi.mode(WIFI_OFF);   // HARD reset the WiFi hardware
    delay(300);
    WiFi.mode(WIFI_STA);
    delay(100);

    // Retry once more
    WiFi.begin(ssid, password);

    startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_TIMEOUT) {
      Serial.print(".");
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WiFi] Connected after reset!");
      return;
    }

    Serial.println("\n[WiFi] Still failed. Will retry from sendStandingToBackend().");
}

// time function for sending to backend
String getTimestampISO8601() {
    time_t now;
    time(&now);

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);  // Local time based on configTime offset

    char buffer[40];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000", &timeinfo);

    return String(buffer) + "Z";
}

// backend function
void sendStandingToBackend(float pct) {

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] Lost connection! Reconnecting...");
      connectWiFi();
    }

    HTTPClient http;
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{";
    jsonPayload += "\"patientId\":\"69164212a337ec14e57f5b1d\",";
    jsonPayload += "\"avgHeartRate\":-1,";
    jsonPayload += "\"motionPercent\":" + String(pct, 2) + ",";
    jsonPayload += "\"timestamp\":\"" + getTimestampISO8601() + "\"";
    jsonPayload += "}";

    Serial.println("Sending:");
    Serial.println(jsonPayload);
    
    int httpResponseCode = http.POST(jsonPayload);

    Serial.print("Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      Serial.println(http.getString()); // response body
    }

    http.end();
}


// --- setup
void setup() {
  Serial.begin(115200);
  delay(500);

  // -- wifi setup
  connectWiFi();

  // sync time ()
  const long EST_OFFSET_SEC = -5 * 3600;  // UTC-5
  const int  DST_OFFSET_SEC = 0;       // if daylight savings needed (can be 0)
  configTime(EST_OFFSET_SEC, DST_OFFSET_SEC, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time");
  time_t nowSec = time(nullptr);
  while (nowSec < 100000) {       // wait until time is valid
      delay(500);
      Serial.print(".");
      nowSec = time(nullptr);
      connectWiFi();
      
  }
  Serial.println("\nTime synced!");


  //set adc resolution to 12 bits
  analogReadResolution(12);

  Serial.println("Sensors Initialized");
  delay(100);
}


// --- main loop

void loop() {

  //print timing (only run through loop() at specified interval)
  unsigned long now = millis();

  
  if (now - lastRead >= readInterval) {
    lastRead = now;

    // ----- Read Accelerometer -----
    int rawX = analogRead(pinX);
    int rawY = analogRead(pinY);
    int rawZ = analogRead(pinZ);

    // covert to Gs
    float ax = convertX(rawX);
    float ay = convertY(rawY);
    float az = convertZ(rawZ);

    //update standing state
    updateStandingState(ax, ay, az);
    updateStandingWindow();

    if (millis() - lastSend >= SEND_INTERVAL) {
      lastSend = millis();

      float pct = getStandingPercentage();

      sendStandingToBackend(pct); 
    }



    // -------- output ------------------

    // raw voltage outputs for calibration
    // Serial.print("rawX:"); Serial.print(rawX * VREF / ADC_RES);
    // Serial.print("    rawY:"); Serial.print(rawY * VREF / ADC_RES);
    // Serial.print("    rawZ:"); Serial.println(rawZ * VREF / ADC_RES);

    //standing state + heart rate
    // Serial.print("Standing: ");
    // Serial.println(isStanding ? "YES" : "NO");
    // Serial.print("   IR:");
    // Serial.print(lastIRvalue);
    // Serial.print("   BPM:");
    // Serial.print((int)beatsPerMinute);
    // Serial.print("   Avg BPM:");
    // Serial.println(beatAvg);

    // //print output for serial plotter
    // Serial.print(">");
    // Serial.print("ax:"); Serial.print(ax); Serial.print(",");
    // Serial.print("ay:"); Serial.print(ay); Serial.print(",");
    // Serial.print("az:"); Serial.print(az); Serial.print(",");
    // // // Serial.print("IR:"); Serial.print(IRvalue);
    // Serial.println();

  }

}
