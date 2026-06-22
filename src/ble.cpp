#include <NimBLEDevice.h>
#include "ble.h"
#include "records.h"

#define SERVICE_UUID "e52c30e1-0a0e-4fd9-8428-9a7ff5f67f2e"
#define CHARACTERISTIC_UUID "6ff149d0-ec5f-4c78-b8e5-d8e503a67057" // both random generated https://www.uuidgenerator.net/
#define METADATA_UUID "9d3a8c41-1b2e-4f7a-9c6d-2e5f8a0b1c3d" // readable sensor id -> name/unit registry (JSON)

NimBLECharacteristic *Characteristic;
NimBLECharacteristic *MetadataCharacteristic;
bool deviceConnected = 0;
uint16_t standardMTU = 23; // standard max BLE packet 23Bytes set to 517

uint32_t timestamp_notify;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *s, NimBLEConnInfo &info) override {
    deviceConnected = 1;
    Serial.printf("Phone connected\n");
  }
  void onDisconnect(NimBLEServer *s, NimBLEConnInfo &info, int reason) override {
    deviceConnected = 0;
    standardMTU = 23; // reset after DC
    Serial.printf("Phone disconnected, advertising to phone now\n");
    NimBLEDevice::startAdvertising();
  }
  void onMTUChange(uint16_t mtu, NimBLEConnInfo &info) override {
    standardMTU = mtu;
    Serial.printf("MTU now %u (notify payload soft limit %u bytes) with hard limit %d bytes payload\n", mtu, mtu - 3, 512); // -3 bytes header
  }
};

// class WriteCallbacks : public NimBLECharacteristicCallbacks { // needed to read data sent from phone
//   void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override {
//     Serial.printf("Received from phone: %s\n", c->getValue().c_str());
//   }
// };

void bleSetup(const char* deviceName) {
  timestamp_notify = millis();

  char name[MAX_DEVICE_NAME_LEN + 1];
  copyTruncated(name, deviceName, MAX_DEVICE_NAME_LEN);
  if (strlen(deviceName) > MAX_DEVICE_NAME_LEN)
    Serial.printf("bleSetup: name too long, truncated to '%s' (max %d chars)\n", name, MAX_DEVICE_NAME_LEN);

  NimBLEDevice::init(name);
  NimBLEDevice::setMTU(517); // request max MTU 517 = 512 value payload max

  NimBLEServer *Server = NimBLEDevice::createServer();
  Server->setCallbacks(new ServerCallbacks());

  NimBLEService *Service = Server->createService(SERVICE_UUID);

  Characteristic = Service->createCharacteristic(CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY); // | NIMBLE_PROPERTY::WRITE
  //Characteristic->setCallbacks(new WriteCallbacks()); // needed to read data sent from phone
  Characteristic->setValue("Hello from Mark esp32c6 :)");

  MetadataCharacteristic = Service->createCharacteristic(METADATA_UUID, NIMBLE_PROPERTY::READ); // damit python id's mit value und einheit einmalig lesen kann.
  MetadataCharacteristic->setValue(buildRegistryJson());

  NimBLEAdvertising *Advertising = NimBLEDevice::getAdvertising();
  Advertising->addServiceUUID(SERVICE_UUID);
  Advertising->setName(name);
  Advertising->start();

  Serial.printf("Advertising as %s", name);
}

void bleNotify() {
  if ((millis() - timestamp_notify >= 1000) || (len + 10 > MAX_PAYLOAD)) {
    if (len + 10 > MAX_PAYLOAD){
      Serial.printf("Notify 1/second is too slow now sending after %d milliseconds from previous notify. Buffer size: %u\n", millis() - timestamp_notify, len);
    }
    if (!deviceConnected){
      Serial.printf("Device not connected, throwing away recent data\n");

      len = 0;
      timestamp_notify = millis();
      return;
    }
    Characteristic->setValue(buffer, len);
    Characteristic->notify();
    //Serial.printf("Just notified via BLE, length: %u bytes, content: ", len);
    // for (size_t i=0; i<len; i++){
    //   Serial.printf("%02X", buffer[i]); // 02X each byte as 2 uppercase hex.
    // }
    // Serial.printf("\n");
    len = 0;
    timestamp_notify = millis();
  }
}

// send as bytearray with timestamp. 1mal/sek oder 0.5sek notify mit allen messwerten. millis() / micros()
// timestamp(uint64) + value(float)
// 1-2Mal/Sek notify: Array[timestamp, value]
