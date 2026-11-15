// function for pms5003

// functions for the bme680

// Dependencies
#include <Arduino.h>
#include "Adafruit_PM25AQI.h"
#include "sensors.h"

/* Wiring and setup (Board to Sensor)
-> 5V to VCC
-> GND to GND
-> TX to RX1
-> RX to TX1
-> add more, I'm almost certain that there should be more
*/
// static = private to this file, so these generic names can't clash with other sensor files
static Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
static PM25_AQI_Data data;

// aqi.read() returns false on partial frames (normal), so retry until timeout
static bool PMS_readFrame(uint32_t timeoutMs){
    uint32_t t0 = millis();
    while (millis() - t0 < timeoutMs) {
        if (aqi.read(&data)) return true;
        delay(10);
    }
    return false;
}

/* Initialize
-> Check if the sensor is there
-> If the sensor is, move on
-> If the sensor is not, do something that allows the other functions to be called without a sensor being there
*/
bool PMS_init(PMS_sensor &d){
    // Keep sensor awake (SET LOW = sleep)
    pinMode(A3, OUTPUT);
    digitalWrite(A3, HIGH);

    // The library does not start the UART itself; begin_UART() only stores the pointer
    Serial1.begin(9600, SERIAL_8N1, RX, TX);
    aqi.begin_UART(&Serial1);

    // Wait for sensor to boot up
    delay(10000);

    // Detect the sensor by getting a real, checksum-valid frame
    while (Serial1.available()) Serial1.read();
    if (!PMS_readFrame(3000)) {
        Serial.println("Could not find the PMS5003 sensor!");
        d.status = false;
        return false;
    }
    Serial.println("PMS5003 found!");
    d.status = true;
    return true;
}


/* Measure
-> Take a measurement
-> Do something with the data, at the moment, just save to a variable
*/
bool PMS_measure(PMS_sensor &d){
    
    // Check if sensor is up 
    while (Serial1.available()) Serial1.read();   // drop stale frames
    if(!PMS_readFrame(3000)){
        Serial.println("PMS5003 failed to read");
        d.status = false;
        return false;
    }
    d.status = true;   // recover after a previous failed read

    // Save the data to the struct
    // Standard (calibrated against a lab reference)
    d.pm10_standard = data.pm10_standard;
    d.pm25_standard = data.pm25_standard;
    d.pm100_standard = data.pm100_standard;
    // Environmental (calibrated for atmospheric conditions; use this for outdoor air!)
    d.pm10_env = data.pm10_env;
    d.pm25_env = data.pm25_env;
    d.pm100_env = data.pm100_env;
    // Particle counts of certain sizes per 0.1L of air
    d.particles_03um = data.particles_03um;
    d.particles_05um = data.particles_05um;
    d.particles_10um = data.particles_10um;
    d.particles_25um = data.particles_25um;
    d.particles_50um = data.particles_50um;
    d.particles_100um = data.particles_100um;
    // AQI (Air Quality Index) values for PM2.5 and PM10 using the US standards
    d.aqi_pm25_us = data.aqi_pm25_us;
    d.aqi_pm100_us = data.aqi_pm100_us;
    return true;
}


/* Print
-> Print the data that was saved in measure to the terminal
-> More can be done, but this is all for now
*/
bool PMS_print(PMS_sensor &d){
    // skip if sensor is down
    if(!d.status) {
        Serial.println("PMS5003 is not functioning");
        return false;
    }

    // Printing standard PM values
    Serial.println(F("---------------------------------------"));
    Serial.println(F("Concentration Units (standard)"));

    Serial.print(F("PM1.0: "));
    Serial.print(d.pm10_standard);
    Serial.println(F(" µg/m³"));
    
    Serial.print(F("PM2.5: "));
    Serial.print(d.pm25_standard);
    Serial.println(F(" µg/m³"));

    Serial.print(F("PM10: "));
    Serial.print(d.pm100_standard);
    Serial.println(F(" µg/m³"));

    // Printing environmental PM values
    Serial.println(F("---------------------------------------"));
    Serial.println(F("Concentration Units (environmental)"));

    Serial.print(F("PM1.0: "));
    Serial.print(d.pm10_env);
    Serial.println(F(" µg/m³"));

    Serial.print(F("PM2.5: "));
    Serial.print(d.pm25_env);
    Serial.println(F(" µg/m³"));

    Serial.print(F("PM10: "));
    Serial.print(d.pm100_env);
    Serial.println(F(" µg/m³"));

    // Printing particle counts
    Serial.println(F("---------------------------------------"));
    Serial.print(F("Particles > 0.3µm / 0.1L air: "));
    Serial.println(d.particles_03um);

    Serial.print(F("Particles > 0.5µm / 0.1L air: "));
    Serial.println(d.particles_05um);

    Serial.print(F("Particles > 1.0µm / 0.1L air: "));
    Serial.println(d.particles_10um);

    Serial.print(F("Particles > 2.5µm / 0.1L air: "));
    Serial.println(d.particles_25um);

    Serial.print(F("Particles > 5.0µm / 0.1L air: "));
    Serial.println(d.particles_50um);

    Serial.print(F("Particles > 10.0µm / 0.1L air: "));
    Serial.println(d.particles_100um);

    return true;
}