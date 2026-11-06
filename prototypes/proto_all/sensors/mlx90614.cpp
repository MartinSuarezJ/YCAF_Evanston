// functions for the mlx90614

// Dependencies
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include "sensors.h"

/* Wiring and setup (Board to Sensor)
-> 5V to VCC
-> GND to GND
-> TX to RX1
-> RX to TX1
-> add more, I'm almost certain that there should be more
*/
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // Sets mlx to an object of the Adafruit_MLX90614


/* Initialize
-> Check if the sensor is there
-> If the sensor is, move on
-> If the sensor is not, do something that allows the other functions to be called without a sensor being there
*/
bool MLX_init(MLX_sensor &d){
    if (!mlx.begin()){
        Serial.println("MLX90614 not found");
        d.status = false;
        return false;
    }
    d.status = true;
    Serial.println("MLX90614 found!");
    return true;
}


/* Measure
-> Take a measurement
-> Do something with the data, at the moment, just save to a variable
*/
bool MLX_measure(MLX_sensor &d){
    // Check if sensor is up 
    if(!d.status){
        Serial.println("MLX90614 still down, skipping measurement");
        d.status = false;
        return false;
    }

    // Save the data to the struct
    d.temp = mlx.readObjectTempC();
    d.ambient_temp = mlx.readAmbientTempC();
    return true;
}


/* Print
-> Print the data that was saved in measure to the terminal
-> More can be done, but this is all for now
*/
bool MLX_print(MLX_sensor &d){
    // skip if sensor is down
    if(!d.status) {
        Serial.println("MLX90614 is not functioning");
        return false;
    }

    Serial.print(F("Object Temperature: "));
    Serial.print(d.temp);
    return true;
}