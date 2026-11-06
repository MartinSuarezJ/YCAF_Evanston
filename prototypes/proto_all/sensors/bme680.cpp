// functions for the bme680

// Dependencies
#include <Arduino.h>
#include "Adafruit_BME680.h"
#include "sensors.h"

/* Wiring and setup
-> add more, I'm almost certain that there should be more
*/
Adafruit_BME680 bme; // for I2C (sets up the bme's default address of 0x77, can be changed to 0x76 by SDO to ground)

/* Initialize
-> Check if the sensor is there
-> If the sensor is, move on
-> If the sensor is not, do something that allows the other functions to be called without a sensor being there
*/
bool BME_init(BME_sensor &d){
    if (!bme.begin()){
        Serial.println("BME680 not found");
        d.status = false;
        return false;
    }
    d.status = true;
    Serial.println("BME680 found!");  
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setGasHeater(320, 150); // 320°C for 150 ms
    return true;
}


/* Measure
-> Take a measurement
-> Do something with the data, at the moment, just save to a variable
*/
bool BME_measure(BME_sensor &d){
    // Check if sensor is up 
    if(!bme.performReading()){
        Serial.println("BME680 failed to read");
        d.status = false;
        return false;
    }

    // Save the data to the struct
    d.temp = bme.temperature;
    d.humidity = bme.humidity;
    d.pressure = bme.pressure/100.0;
    d.gas = bme.gas_resistance/1000.0;
    return true;
}


/* Print
-> Print the data that was saved in measure to the terminal
-> More can be done, but this is all for now
*/
bool BME_print(BME_sensor &d){
    // skip if sensor is down
    if(!d.status) {
        Serial.println("BME680 is not functioning");
        return false;
    }

    Serial.print("Temperature = ");
    Serial.print(d.temp);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(d.pressure);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(d.humidity);
    Serial.println(" %");

    Serial.print("Gas = ");
    Serial.print(d.gas);
    Serial.println(" KOhms");

    return true;
}