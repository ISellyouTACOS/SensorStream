#include "records.h"
#include "ble.h"

uint8_t buffer[MAX_PAYLOAD]; // one packet with records
size_t len = 0;
uint32_t starting_timestamp;
uint32_t current_timestamp = 0;
bool isFirstRecord = 1;

struct SensorMeta {
  char name[MAX_NAME_LEN + 1];
  char unit[MAX_UNIT_LEN + 1];
};

static SensorMeta registry[MAX_SENSORS];
static uint8_t sensorCount = 0;

void copyTruncated(char* dest, const char* src, uint8_t maxLen) {
  uint8_t i = 0;
  while (src[i] != '\0' && i < maxLen) { // stop at end of src or when full
    dest[i] = src[i];
    i++;
  }
  dest[i] = '\0'; // always terminate
}

uint8_t registerSensor(const char* name, const char* unit) {
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

  if (isFirstRecord){ // first measurement will have timestamp = 0;
    isFirstRecord = 0;
    starting_timestamp = millis();
  }

  if (len + 5 > MAX_PAYLOAD)
    bleNotify();


  if (current_timestamp == millis() - starting_timestamp){ // same timestamp more often-> use 1 timestamp per group
    Serial.printf("addingRecord: ID: %u, value: %f\n", id, value); // 250ms to 330ms between each notify, good.
    buffer[len] = id;
    len ++;
    memcpy(buffer + len, &value, 4);
    len += 4;
    return;
  }

  if (len + 10 > MAX_PAYLOAD)
    bleNotify(); 

  current_timestamp = millis() - starting_timestamp;

  Serial.printf("adding Record with timestamp: ID: %u, timestamp: %u, value: %f\n", id, current_timestamp, value);

  buffer[len] = TS_MARKER;
  len++;
  memcpy(buffer + len, &current_timestamp, 4); //4bytes timestamp
  len += 4;
  buffer[len] = id; //1byte id
  len ++;
  memcpy(buffer + len, &value, 4); // 4 bytes value
  len +=4;
}

// Question: Timestamp calculation in python instead of here? more efficient.