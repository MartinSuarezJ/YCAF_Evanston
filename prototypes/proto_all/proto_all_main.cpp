// Code to run all sensor tests at once

// Dependencies
#include <Arduino.h>
#include "sensors.h"

// For BME680
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// For GY-906 MLX90614
#include <Wire.h>
#include <Adafruit_MLX90614.h>

// For PMS5003
#include "Adafruit_PM25AQI.h"


// Set up serial monitor
Serial.begin(115200);


/* Set up all of the sensors (only move on if all sensors are found -- alternatively: move on anyways, but state that one isn't found)
-> Find all sensors
-> Might be a good idea to set up the wiring and pins here too
-> Set up communication protocols
*/


/* Start of a continuous loop
-> Take measurements
-> Print data
-> Maybe check the status of the sensors, so that if one is down it doesn't ruin the whole thing? (could be simpler than the initialization of the sensors)
*/

    /* Take a measurement from each sensor
    -> They're kinda all already functions, almost no reason to make a function for each sensor...
    -> UNLESS: I want to take all of the measurements a sensor has to offer (some have a lot and it can get kind of messy)
    */

    /* Display all of the measurements
    -> Also not entirely necessary to be it's own function, but honestly not horrible
    */

    /* Check status of the sensors
    -> If a sensor is not found, don't look for a value on the next run
    -> Maybe try to re-initialize the sensor
    */

/* End of continuous loop
*/