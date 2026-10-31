// Code to test and prototype with the PMS5003 (Air quality sensor)

#include "Adafruit_PM25AQI.h"

// For documentation purposes, our ESP has hardware UART

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();


// Sets up the sensor
void setup(){
    // Wait for serial monitor to open
    Serial.begin(115200);
    while(!Serial) delay(10);

    Serial.println("Adafruit PMS5003 Air Quality Sensor");
    
    // Wait for sensor to boot up
    delay(10000);

    // Set up communication/connectivity?
    if (! aqi.begin_UART(&Serial1)) {
        Serial.println("Could not find PM 2.5 sensor!");
        while (1) delay(10);
    }
    Serial.print("PMS5003 found!");
}

void loop(){
    PM25_AQI_Data data;

    if (! aqi.read(&data)){
        Serial.println("Could not read from AQI");

        delay(500);
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
    //  Serial.print(F("PM2.5 AQI China: ")); Serial.print(data.aqi_pm25_china);
    //  Serial.print(F("\tPM10  AQI China: ")); Serial.println(data.aqi_pm100_china);
    Serial.println(F("---------------------------------------"));
    Serial.println();

    delay(1000);
}