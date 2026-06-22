#pragma once
#include "records.h"
#include "ble.h"

// SensorStream: register sensors, push float values, stream them over BLE.
//
//   #include <SensorStream.h>
//
//   uint8_t id_temp;
//
//   void setup() {
//     id_temp = registerSensor("temp", "C"); // get a free id once
//     bleSetup("adv-name");// start advertising. Do only once per ESP. Keep short -> overflow
//   }
//
//   void loop() {
//     addRecord(id_temp, readTemperature());// push a value, sent over BLE
//   }

