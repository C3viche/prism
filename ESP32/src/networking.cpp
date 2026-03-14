#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include "config.h"
#include "networking.h"

String loadCert(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) return "";
    String content = file.readString();
    file.close();
    return content;
}



void setManualTime() {
    struct timeval tv;
    tv.tv_sec = 1773139500; // March 2026
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    Serial.println("Manual time set to March 2026");
}

