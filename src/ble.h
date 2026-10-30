#pragma once
#include <Arduino.h>

#define MAX_DEVICE_NAME_LEN 29 // 31bytes - 2 header

void bleSetup(const char* deviceName = "Mark esp");
void bleNotify();
size_t blePayLoadLimit();

