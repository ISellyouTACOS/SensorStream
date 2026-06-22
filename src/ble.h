#pragma once
#include <Arduino.h>

extern bool deviceConnected;
#define MAX_DEVICE_NAME_LEN 8

void bleSetup(const char* deviceName = "Mark esp");
void bleNotify();

