// Code to test and prototype with the GY-906 MLX90614 (surface temperature sensor)


/* Equation to read the data V_ir = A(T_o^4 - T_a^4)
 V_ir = infrared voltage?
 A = current?
 T_o = object temperature
 T_a = ambient temperature
*/

#include <Wire.h>
#include <Adafruit_MLX90614.h>

// ========== Infrared Temperature Sensor ==========
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

float currentTempC = mlx.readObjectTempC();

void setup() {

  while (!mlx.begin()){
    Serial.println("MLX90614 not found, retrying...");
    delay(1000);
  }


  // === Serial Initialization ===
  Serial.begin(9600);

  // === Initialize MLX90614 Sensor ===
  mlx.begin();
}

void loop() {
    // === Read Object Temperature Once ===
  currentTempC = mlx.readObjectTempC();
  
  // Edited to remove button // === Serial Output (Once per Button Press) ===
  Serial.println(F("=== Measurement Taken ==="));
  Serial.print(F("Object Temperature: "));
  Serial.print(currentTempC);
  Serial.println(F(" °C"));
  Serial.println();

  // Small delay to reduce loop frequency
  delay(100);
}