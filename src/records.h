#pragma once
#include <Arduino.h>

#ifndef SENSORSTREAM_DEBUG
#define SENSORSTREAM_DEBUG 1 // 1 for debug Serial prints. 0 for no prints
#endif

#define MAX_PAYLOAD 512 // max mtu, packet size
#define TS_MARKER 0xFF

#define MAX_METADATA_BYTES 512
#define MAX_NAME_LEN 8
#define MAX_UNIT_LEN 6
// worst-case bytes per entry
#define MAX_SENSORS ((MAX_METADATA_BYTES - 1) / (MAX_NAME_LEN + MAX_UNIT_LEN + 4)) // = 28

extern uint8_t recordBuffer[MAX_PAYLOAD]; // one packet with records
extern size_t recordLen;

uint8_t registerSensor(const char* name, const char* unit = "");
const char* buildRegistryJson();
void addRecord(uint8_t id, float value);
void copyTruncated(char* dest, const char* src, uint8_t maxLen); // copy src into dest, cut off at maxLen chars
