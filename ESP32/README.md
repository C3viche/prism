# ESP32 AWS IoT Bridge

This project enables an ESP32 DevKit V1 to act as a secure gateway between a CC3200 board and AWS IoT Core. It handles WiFi connectivity, manages SSL/TLS certificates via LittleFS, and synchronizes Device Shadows using JSON.

## Project Structure

1. To build this project in PlatformIO, ensure your directory is organized as follows:

```text
.
├── data/                       # Filesystem folder (LittleFS)
│   ├── RootCA.pem              # AWS Root CA
│   ├── certificate.pem.crt     # Device Certificate
│   └── private.pem.key         # Private Key
├── src/
│   └── main.cpp                # Your source code
├── .gitignore                  # Keeps your certs off GitHub
└── platformio.ini              # Project configuration

---
```

And save this as `platformio.ini` in your project root.

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
board_build.filesystem = littlefs
lib_deps = 
    bblanchon/ArduinoJson @ ^6.21.3

```


Ensure that you flash the board with the partitions file before you attempt to upload the code. 