#pragma once

// BME680
struct BME_sensor { bool status=false; float temp=0, humidity=0, pressure=0, gas=0; };
// Custom functions
bool BME_init(BME_sensor &d);
bool BME_measure(BME_sensor &d);
bool BME_print(const BME_sensor &d);

// mlx90614
struct MLX_sensor { bool status=false; float temp=0, ambient_temp=0; };
// Custom functions
bool MLX_init(MLX_sensor &d);
bool MLX_measure(MLX_sensor &d);
bool MLX_print(MLX_sensor &d);

//PMS5003
struct PMS_sensor { bool status=false; float pm10_standard=0, pm25_standard=0, pm100_standard=0, pm10_env=0, pm25_env=0, pm100_env=0, particles_03um=0, particles_05um=0, particles_10um=0, particles_25um=0, particles_50um=0, particles_100um=0, aqi_pm25_us=0, aqi_pm100_us=0; };
// Custom Functions
bool PMS_init(PMS_sensor &d);
bool PMS_measure(PMS_sensor &d);
bool PMS_print(PMS_sensor &d);