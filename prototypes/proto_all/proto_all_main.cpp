// Code to run all sensor tests at once

// Dependencies
#include <Arduino.h>
#include "sensors/sensors.h"

/* Define all the sensors
-> Make an instance of each sensor based on how you coded them in the individual components
*/
BME_sensor bme_inst;
MLX_sensor mlx_inst;
PMS_sensor pms_inst;

// Set up
void setup(){
    // Set up the Serial Monitor
    Serial.begin(115200);

    /* Set up all of the sensors (only move on if all sensors are found -- alternatively: move on anyways, but state that one isn't found)
    -> Find all sensors
    -> Might be a good idea to set up the wiring and pins here too
    -> Set up communication protocols
    */
    BME_init(bme_inst);
    MLX_init(mlx_inst);
    PMS_init(pms_inst);
}


void loop(){
    /* Start of a continuous loop
    -> Take measurements
    -> Print data
    -> Maybe check the status of the sensors, so that if one is down it doesn't ruin the whole thing? (could be simpler than the initialization of the sensors)
    */
    // This should already be handled by the function files//

    /* Take a measurement from each sensor
    -> They're kinda all already functions, almost no reason to make a function for each sensor...
    -> UNLESS: I want to take all of the measurements a sensor has to offer (some have a lot and it can get kind of messy)
    */
    BME_measure(bme_inst);
    MLX_measure(mlx_inst);
    PMS_measure(pms_inst);

    /* Display all of the measurements
    -> Also not entirely necessary to be it's own function, but honestly not horrible
    */
    BME_print(bme_inst);
    MLX_print(mlx_inst);
    PMS_print(pms_inst);

    /* Check status of the sensors
    -> If a sensor is not found, don't look for a value on the next run
    -> Maybe try to re-initialize the sensor
    */
    if (!bme_inst.status){
        BME_init(bme_inst);
    }
    if (!mlx_inst.status){
        MLX_init(mlx_inst);
    }
    if (!pms_inst.status){
        PMS_init(pms_inst);
    }
/* End of continuous loop
*/
}