// Code to test and prototype with the GY-906 MLX90614 (surface temperature sensor)


/* Equation to read the data V_ir = A(T_o^4 - T_a^4)
 V_ir = infrared voltage?
 A = current?
 T_o = object temperature
 T_a = ambient temperature
*/

#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

float currentTempC = 0;

void setup() {
  Serial.begin(115200);
  delay(1500);  // give USB serial time to connect

  while (!mlx.begin()) {
    Serial.println(F("MLX90614 not found, retrying..."));
    delay(1000);
  }
  Serial.println(F("MLX90614 ready"));
}

void loop() {
  currentTempC = mlx.readObjectTempC();

  Serial.print(F("Object Temperature: "));
  Serial.print(currentTempC);
  Serial.println(F(" °C"));

  delay(500);
}