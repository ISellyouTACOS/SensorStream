// SensorStream example: stream sensor over BLE.
//
// Build target: any ESP32 with the Arduino framework + NimBLE-Arduino.
// Connect with Python/Bleak reader on pc

#include <Arduino.h>
#include <SensorStream.h>

  uint8_t id_temp;

  void setup() {
    id_temp = registerSensor("temp", "C"); // get a free id once
    bleSetup("adv-name");// start advertising. Do only once per ESP. Keep name short! overflow
  }

  void loop() {
    addRecord(id_temp, readTemperature());// push a value, sent over BLE
  }

  float readTemperature(){
    float temperature = 32.4;
    return temperature;
  }
