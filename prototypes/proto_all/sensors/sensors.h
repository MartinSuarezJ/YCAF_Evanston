#pragma once

// BME680
struct BME_sensor { bool status; float temp, humidity, pressure, gas; };
// Custom functions
bool BME_init(BME_sensor &d);
bool BME_measure(BME_sensor &d);
bool BME_print(BME_sensor &d);

// mlx90614
struct MLX_sensor { bool status; float temp, ambient_temp; };
// Custom functions
bool MLX_init(MLX_sensor &d);
bool MLX_measure(MLX_sensor &d);
bool MLX_print(MLX_sensor &d);

//PMS5003
// Custom Functions
struct PMS_sensor { bool status; float pm10_standard, pm25_standard, pm100_standard, pm10_env, pm25_env, pm100_env, particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um, aqi_pm25_us, aqi_pm100_us; };
bool PMS_init(PMS_sensor &d);
bool PMS_measure(PMS_sensor &d);
bool PMS_print(PMS_sensor &d);