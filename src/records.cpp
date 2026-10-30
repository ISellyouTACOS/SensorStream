#include "records.h"
#include "ble.h"

uint8_t recordBuffer[MAX_PAYLOAD]; // one packet with records
size_t recordLen = 0;
static uint32_t starting_timestamp; // static, only used in this file.
static uint32_t current_timestamp = 0;
static bool isFirstRecord = 1;
static bool registryPublished = 0;


struct SensorMeta {
  char name[MAX_NAME_LEN + 1];
  char unit[MAX_UNIT_LEN + 1];
};

static SensorMeta registry[MAX_SENSORS];
static uint8_t sensorCount = 0;

void copyTruncated(char* dest, const char* src, uint8_t maxLen) {
  uint8_t i = 0;
  while (src[i] != '\0' && i < maxLen) { // stop at end of src or when full
    
    //--------------------edge case
    char c = src[i];
    if (c == '"' || c == '\\' || c == '|')
    c = '_';
    //----------------------

    dest[i] = c;
    i++;
  }
  dest[i] = '\0'; // always terminate
}

uint8_t registerSensor(const char* name, const char* unit) {
  if (registryPublished)
    Serial.printf("[WARNING] you have to registerSensor() before bleSetup(), receiver won't know the sensor's name\n");
  if (sensorCount >= MAX_SENSORS) {
    Serial.printf("registerSensor: registry full (max %d), %s ignored\n", MAX_SENSORS, name);
    return TS_MARKER;
  }
  uint8_t id = sensorCount++;
  copyTruncated(registry[id].name, name, MAX_NAME_LEN);
  copyTruncated(registry[id].unit, unit, MAX_UNIT_LEN);
  return id;
}

const char* buildRegistryJson() {
  registryPublished = 1;
  static String json;
  json = "[";
  for (uint8_t i = 0; i < sensorCount; i++) {
    if (i > 0) json += ",";
    json += "\"" + String(registry[i].name) + "|" + String(registry[i].unit) + "\"";
  }
  json += "]";
  return json.c_str();
}

void addRecord(uint8_t id, float value) { // timestamp, id, value, id, value, id, value, timestamp etc.

  if (id >= sensorCount){ // id 0xFF prohibited
    return;
  }

  bleNotify(); // With very slow sensors bleNotify() should be called in loop
  if (isFirstRecord){ 
    isFirstRecord = 0;
    starting_timestamp = millis();
  }
  uint32_t now = millis() - starting_timestamp;

  if (recordLen > 0 && current_timestamp == now){ // same timestamp more often-> use 1 timestamp per group
    #if SENSORSTREAM_DEBUG
    Serial.printf("addingRecord: ID: %u, value: %f\n", id, value); // 250ms to 330ms between each notify, good.
    #endif
    recordBuffer[recordLen] = id;
    recordLen ++;
    memcpy(recordBuffer + recordLen, &value, 4);
    recordLen += 4;
    return;
  }

  #if SENSORSTREAM_DEBUG
  Serial.printf("adding Record with timestamp: ID: %u, timestamp: %lu, value: %f\n", id, (unsigned long)now, value);
  #endif

  current_timestamp = now;

  recordBuffer[recordLen] = TS_MARKER;
  recordLen++;
  memcpy(recordBuffer + recordLen, &now, 4); //4bytes timestamp
  recordLen += 4;
  recordBuffer[recordLen] = id; //1byte id
  recordLen ++;
  memcpy(recordBuffer + recordLen, &value, 4); // 4 bytes value
  recordLen +=4;
  return;
}

// Question: Timestamp calculation in python instead of here? more efficient.