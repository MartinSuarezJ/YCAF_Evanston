// Code to test and prototype with the PMS5003 (Air quality sensor)

#include <Arduino.h>
#include "Adafruit_PM25AQI.h"

// Wiring (Feather ESP32-S3 <-> PMS5003)
//   PMS TXD -> Feather RX (GPIO38)
//   PMS RXD -> Feather TX (GPIO39)
//   PMS SET -> A3 (driven HIGH here = run; LOW = sleep)
//   PMS VCC -> 5V (the fan needs 5V; logic is 3.3V)
#define PMS_RX_PIN  RX
#define PMS_TX_PIN  TX
#define PMS_SET_PIN A3

#define PMS_BOOT_MS        10000  // fan/laser spin-up before readings are stable
#define PMS_READ_TIMEOUT_MS 3000  // sensor sends a frame every ~1-2.3 s

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

// Poll the UART until a full, checksum-valid frame arrives or we time out.
// aqi.read() returns false for partial frames, which is normal, so retry.
bool readFrame(PM25_AQI_Data &data, uint32_t timeoutMs) {
    uint32_t t0 = millis();
    while (millis() - t0 < timeoutMs) {
        if (aqi.read(&data)) return true;
        delay(10);
    }
    return false;
}

void setup(){
    Serial.begin(115200);
    // USB CDC: wait (up to 5 s) for the monitor so the first prints aren't lost
    uint32_t t0 = millis();
    while (!Serial && millis() - t0 < 5000) delay(10);
    Serial.println("Adafruit PMS5003 Air Quality Sensor");

    // Make sure the sensor is awake (LOW on SET = sleep mode)
    pinMode(PMS_SET_PIN, OUTPUT);
    digitalWrite(PMS_SET_PIN, HIGH);

    // The library does NOT start the UART for you - this must come first.
    Serial1.begin(9600, SERIAL_8N1, PMS_RX_PIN, PMS_TX_PIN);
    aqi.begin_UART(&Serial1);  // only stores the pointer; can't detect the sensor

    Serial.println("Waiting for sensor to boot...");
    delay(PMS_BOOT_MS);

    // Detect the sensor by actually getting a valid frame
    while (Serial1.available()) Serial1.read();   // drop stale/partial bytes
    PM25_AQI_Data probe;
    if (!readFrame(probe, PMS_READ_TIMEOUT_MS)) {
        Serial.println("Could not find PM 2.5 sensor! Check TX/RX crossover, 5V power, SET pin.");
        Serial.printf("Bytes waiting on Serial1: %d\n", Serial1.available());
        while (1) delay(10);
    }
    Serial.println("PMS5003 found!");
}

void loop(){
    PM25_AQI_Data data;

    // Flush frames that piled up during the last delay so the reading is fresh
    while (Serial1.available()) Serial1.read();

    if (!readFrame(data, PMS_READ_TIMEOUT_MS)){
        Serial.println("Could not read from AQI (timeout)");
        return;
    }

    Serial.println("AQI reading success");

    Serial.println(F("---------------------------------------"));
    Serial.println(F("Concentration Units (standard)"));
    Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_standard);
    Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
    Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_standard);
    Serial.println(F("---------------------------------------"));
    Serial.println(F("Concentration Units (environmental)"));
    Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_env);
    Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_env);
    Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_env);
    Serial.println(F("---------------------------------------"));
    Serial.print(F("Particles > 0.3um / 0.1L air:")); Serial.println(data.particles_03um);
    Serial.print(F("Particles > 0.5um / 0.1L air:")); Serial.println(data.particles_05um);
    Serial.print(F("Particles > 1.0um / 0.1L air:")); Serial.println(data.particles_10um);
    Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
    Serial.print(F("Particles > 5.0um / 0.1L air:")); Serial.println(data.particles_50um);
    Serial.print(F("Particles > 10 um / 0.1L air:")); Serial.println(data.particles_100um);
    Serial.println(F("---------------------------------------"));
    Serial.println(F("AQI"));
    Serial.print(F("PM2.5 AQI US: ")); Serial.print(data.aqi_pm25_us);
    Serial.print(F("\tPM10  AQI US: ")); Serial.println(data.aqi_pm100_us);
    Serial.println(F("---------------------------------------"));
    Serial.println();

    delay(1000);
}
