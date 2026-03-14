#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include "config.h"

void setManualTime();
String loadCert(const char* path);