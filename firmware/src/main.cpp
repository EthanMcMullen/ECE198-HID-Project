#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_MAX30102.h>

// --- initialize pulse oximeter object
DFRobot_MAX30102 particleSensor;

// --- set up accelerometer readings
// analog pin setup
const int pinX = A0;
const int pinY = A1;
const int pinZ = A2;

// ADXL335 calibration constants (need to adjust)
const float zeroG = 1.65;     // Zero-g bias voltage (approx.)
const float sensitivity = 0.300; // V per g for ADXL335

// 12-bit ADC conversion
const float ADC_RES = 4095.0;
const float VREF = 5.0;

// --- printing interval setup for vscode plotter
unsigned long lastPrint = 0;
const unsigned long printInterval = 50;  // 20 Hz output → good for the plotter

// --- helper function: convert analog reading to acceleration in g
float convertToG(int raw) {
    float voltage = (raw * VREF) / ADC_RES;
    return (voltage - zeroG) / sensitivity;
}

// --- setup
void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin();

  // check for sensor intialization
  while (!particleSensor.begin()) {
    Serial.println("MAX30102 not found!");
    delay(1000);
  }

  particleSensor.sensorConfiguration(
        60,              // LED brightness
        SAMPLEAVG_8,
        MODE_MULTILED,
        SAMPLERATE_400,
        PULSEWIDTH_411,
        ADCRANGE_16384
    );

    Serial.println("Sensors Initialized");
}


// --- main loop

void loop() {

  //print timing (only run through loop() at specified interval)
  unsigned long now = millis();
  if (now - lastPrint >= printInterval) {
    lastPrint = now;

    // ----- Read Accelerometer -----
    int rawX = analogRead(pinX);
    int rawY = analogRead(pinY);
    int rawZ = analogRead(pinZ);

    float ax = convertToG(rawX);
    float ay = convertToG(rawY);
    float az = convertToG(rawZ);

    // read pulse oximeter data
    int IRvalue = particleSensor.getIR();
    
    //print output for serial plotter
    Serial.print(">");
    Serial.print("ax:"); Serial.print(ax); Serial.print(",");
    Serial.print("ay:"); Serial.print(ay); Serial.print(",");
    Serial.print("az:"); Serial.print(az); Serial.print(",");
    Serial.print("IR:"); Serial.print(IRvalue);
    Serial.println();  // ends with \r\n automatically

    Serial.println(IRvalue);

  }

}
