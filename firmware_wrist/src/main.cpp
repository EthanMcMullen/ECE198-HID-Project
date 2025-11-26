#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

MAX30105 particleSensor;

// Custom I2C pins for ESP32-C3
#define I2C_SDA 8
#define I2C_SCL 9

#define MINIMUM_SPO2 80
#define FINGER_TIMEOUT 8000
#define READING_INTERVAL 10000  // Print reading every 10 seconds

// Auto-calibration variables
uint32_t baselineIR = 0;
uint32_t maxIR = 0;
uint32_t minIR = 999999;
bool calibrated = false;

uint32_t irBuffer[100];
uint32_t redBuffer[100];

int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

byte readLED = 2;

long lastBeat = 0;
long lastPrint = 0;
long fingerDetectedTime = 0;
long readingStartTime = 0;

float beatsPerMinute;
const byte RATE_SIZE = 10;  // Store more beats for better averaging
byte rates[RATE_SIZE];
byte rateSpot = 0;
int beatAvg = 0;
int spo2Avg = 0;

// Extended averaging for display
const byte DISPLAY_AVG_SIZE = 20;
int displayBpmHistory[DISPLAY_AVG_SIZE];
int displaySpo2History[DISPLAY_AVG_SIZE];
byte displayHistoryIndex = 0;
int displayBpmAvg = 0;
int displaySpo2Avg = 0;
bool hasValidReading = false;


// timer for sending to backend
unsigned long lastSend = 0;
// const unsigned long SEND_INTERVAL = 5UL * 60UL * 1000UL; // 5 min
const unsigned long SEND_INTERVAL =  10UL * 1000UL; // 10 seconds

// -- wifi setup
const char* ssid = "Ethans phone";
const char* password = "ceeitch123";

String endpoint = "http://172.20.10.3:3000/patients/69164212a337ec14e57f5b1d/data";

void connectWiFi() {
    Serial.println("\n[WiFi] Connecting...");

    WiFi.mode(WIFI_STA);           // MUST be before anything else on ESP32-C3
    WiFi.setHostname("esp32c3_2");   // Prevents stuck state on some hotspots

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

    Serial.println("\n[WiFi] Still failed. Will retry from sendBpmToBackend().");
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
void sendBpmToBackend(float bpm) {

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] Lost connection! Reconnecting...");
      connectWiFi();
    }

    HTTPClient http;
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{";
    jsonPayload += "\"patientId\":\"69164212a337ec14e57f5b1d\",";
    jsonPayload += "\"avgHeartRate\":" + String(bpm, 2) + ",";
    jsonPayload += "\"motionPercent\":-1,";
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




void setup()
{ 
  Wire.begin(I2C_SDA, I2C_SCL);
  pinMode(readLED, OUTPUT);
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║  MAX30102 Pulse Oximeter - ESP32-C3  ║");
  Serial.println("║   10-Second Averaging Mode            ║");
  Serial.println("╚═══════════════════════════════════════╝\n");
  
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("ERROR: MAX30102 not found!");
    while (1);
  }

  Serial.println("Sensor initialized");
  
  // Configuration for maximum sample rate
  byte ledBrightness = 0x1F;
  byte sampleAverage = 1;
  byte ledMode = 2;
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 16384;
  
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);
  particleSensor.clearFIFO();
  
  // Initialize history arrays
  for (int i = 0; i < DISPLAY_AVG_SIZE; i++) {
    displayBpmHistory[i] = 0;
    displaySpo2History[i] = 0;
  }

  // connect to wifi
  connectWiFi();

  // sync time for backend data transfer
  const long EST_OFFSET_SEC = -5 * 3600;  // UTC-5
  const int  DST_OFFSET_SEC = 0;       // if daylight savings needed (can be 0)
  configTime(EST_OFFSET_SEC, DST_OFFSET_SEC, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time");
  time_t nowSec = time(nullptr);
  while (nowSec < 100000) {       // wait until time is valid
      delay(1500);
      Serial.println("NTP time syncing...");
      nowSec = time(nullptr);
        connectWiFi();
  }
  Serial.println("\nTime synced!");

  
  Serial.println("\n Place your finger on sensor");
  Serial.println("   Readings will display every 10 seconds\n");
  Serial.println("═══════════════════════════════════════\n");
}

void calibrateSensor() {
  Serial.println(" Calibrating sensor range...");
  
  for (int i = 0; i < 50; i++) {
    while (particleSensor.available() == false)
      particleSensor.check();
    
    uint32_t ir = particleSensor.getIR();
    particleSensor.nextSample();
    
    if (ir > maxIR) maxIR = ir;
    if (ir < minIR) minIR = ir;
    baselineIR += ir;
    
    delay(10);
  }
  
  baselineIR /= 50;
  calibrated = true;
  
  Serial.println("Calibration complete!");
  Serial.print("   IR Range: ");
  Serial.print(minIR);
  Serial.print(" - ");
  Serial.println(maxIR);
  Serial.println("\n Collecting data for 10 seconds...\n");
}

void printDetailedStatus() {
  long currentIR = particleSensor.getIR();
  
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║         HEALTH METRICS REPORT         ║");
  Serial.println("╠═══════════════════════════════════════╣");
  
  // Signal Quality
  Serial.print("║ Signal Strength: ");
  if (currentIR >= baselineIR * 0.95) {
    Serial.println("Excellent      ║");
  } else if (currentIR >= baselineIR * 0.85) {
    Serial.println("Good           ║");
  } else {
    Serial.println("Fair           ║");
  }
  
  Serial.print("║    IR Value: ");
  Serial.print(currentIR);
  Serial.print("                    ║\n");
  Serial.println("╠═══════════════════════════════════════╣");
  
  // Heart Rate
  if (displayBpmAvg > 0 && hasValidReading) {
    Serial.print("║ Heart Rate: ");
    Serial.print(displayBpmAvg);
    Serial.print(" BPM");
    
    if (displayBpmAvg < 60) {
      Serial.println("                ║");
      Serial.println("║    Status: Low (Bradycardia)       ║");
    } else if (displayBpmAvg > 100) {
      Serial.println("               ║");
      Serial.println("║    Status: High (Tachycardia)      ║");
    } else {
      Serial.println("               ║");
      Serial.println("║    Status: Normal                  ║");
    }
  } else {
    Serial.println("║ Heart Rate: -- BPM              ║");
    Serial.println("║    Status: Stabilizing...          ║");
  }
  
  Serial.println("╠═══════════════════════════════════════╣");
  
  // SpO2
  if (displaySpo2Avg >= 80 && displaySpo2Avg <= 100 && hasValidReading) {
    Serial.print("║   Blood Oxygen (SpO2): ");
    Serial.print(displaySpo2Avg);
    Serial.println(" %       ║");
    
    if (displaySpo2Avg >= 95) {
      Serial.println("║    Status: Normal                  ║");
    } else if (displaySpo2Avg >= 90) {
      Serial.println("║    Status: Low (Monitor)           ║");
    } else {
      Serial.println("║    Status: Very Low (Seek help)    ║");
    }
  } else {
    Serial.println("║   Blood Oxygen (SpO2): -- %       ║");
    Serial.println("║    Status: Calculating...          ║");
  }
  
  Serial.println("╚═══════════════════════════════════════╝");
  Serial.println("\n   Next reading in 10 seconds...\n");
}

void loop()
{
  bufferLength = 100;
  
  long irValue = particleSensor.getIR();
  uint32_t fingerThreshold = calibrated ? baselineIR / 2 : 7000;
  
  if (irValue < fingerThreshold) {
    if (millis() - lastPrint > 3000) {
      Serial.print(" Waiting for finger... (IR: ");
      Serial.print(irValue);
      Serial.println(")\n");
      lastPrint = millis();
    }
    
    // Reset everything
    beatAvg = 0;
    spo2Avg = 0;
    displayBpmAvg = 0;
    displaySpo2Avg = 0;
    hasValidReading = false;
    fingerDetectedTime = 0;
    readingStartTime = 0;
    calibrated = false;
    
    for (byte i = 0; i < RATE_SIZE; i++) rates[i] = 0;
    for (int i = 0; i < DISPLAY_AVG_SIZE; i++) {
      displayBpmHistory[i] = 0;
      displaySpo2History[i] = 0;
    }
    
    delay(100);
    return;
  }
  
  if (fingerDetectedTime == 0) {
    fingerDetectedTime = millis();
    Serial.print("Finger detected! (IR: ");
    Serial.print(irValue);
    Serial.println(")\n");
  }
  
  if (!calibrated) {
    calibrateSensor();
    readingStartTime = millis();
  }
  
  // Collect 100 samples
  for (byte i = 0; i < bufferLength; i++)
  {
    while (particleSensor.available() == false)
      particleSensor.check();
  
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
    
    if (i % 25 == 0) {
      digitalWrite(readLED, !digitalRead(readLED));
    }
  }
  
  // Initial calculation
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  
  // Main loop
  while (1)
  {
    irValue = particleSensor.getIR();
    if (irValue < fingerThreshold) {
      Serial.println("\n  Finger removed\n");
      fingerDetectedTime = 0;
      calibrated = false;
      hasValidReading = false;
      break;
    }
    
    // Shift buffer
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
  
    // Get 25 new samples
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false)
        particleSensor.check();
    
      digitalWrite(readLED, !digitalRead(readLED));
    
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();

      long irValue = irBuffer[i];

      // Beat detection
      if (checkForBeat(irValue) == true)
      {
        long delta = millis() - lastBeat;
        lastBeat = millis();
      
        beatsPerMinute = 60 / (delta / 1000.0);
        
        if (beatsPerMinute > 40 && beatsPerMinute < 180) {
          rates[rateSpot++] = (byte)beatsPerMinute;
          rateSpot %= RATE_SIZE;
          
          beatAvg = 0;
          byte count = 0;
          for (byte x = 0; x < RATE_SIZE; x++) {
            if (rates[x] > 0) {
              beatAvg += rates[x];
              count++;
            }
          }
          if (count > 0) {
            beatAvg /= count;
            
            // Add to display history
            displayBpmHistory[displayHistoryIndex] = beatAvg;
          }
        }
      }
      
      if (millis() - lastBeat > FINGER_TIMEOUT) {
        beatAvg = 0;
        for (byte j = 0; j < RATE_SIZE; j++) rates[j] = 0;
      }
    }
  
    // Recalculate SpO2
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    
    if (validSPO2 == 1 && spo2 >= MINIMUM_SPO2 && spo2 <= 100) {
      if (spo2Avg == 0) {
        spo2Avg = spo2;
      } else {
        spo2Avg = (spo2Avg * 3 + spo2) / 4;
      }
      
      // Add to display history
      displaySpo2History[displayHistoryIndex] = spo2Avg;
    }

    // Print detailed report + send data to backend every 10 seconds
    if (millis() - readingStartTime >= READING_INTERVAL) {
      // Calculate display averages
      displayBpmAvg = 0;
      displaySpo2Avg = 0;
      int bpmCount = 0;
      int spo2Count = 0;
      
      for (int i = 0; i < DISPLAY_AVG_SIZE; i++) {
        if (displayBpmHistory[i] > 0) {
          displayBpmAvg += displayBpmHistory[i];
          bpmCount++;
        }
        if (displaySpo2History[i] > 0) {
          displaySpo2Avg += displaySpo2History[i];
          spo2Count++;
        }
      }
      
      if (bpmCount > 0) displayBpmAvg /= bpmCount;
      if (spo2Count > 0) displaySpo2Avg /= spo2Count;
      
      if (bpmCount >= 3 && spo2Count >= 3) {
        hasValidReading = true;
      }
      
      printDetailedStatus();

      sendBpmToBackend(displayBpmAvg); 
      
      readingStartTime = millis();
      displayHistoryIndex = (displayHistoryIndex + 1) % DISPLAY_AVG_SIZE;
    }
  }
}