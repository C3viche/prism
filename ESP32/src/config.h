
#include <Arduino.h>

#ifndef CONFIG_H
#define CONFIG_H

// --- WiFi & AWS ---
extern const char* awsEndpoint;
extern const char* thingName;
extern const char* ssid;
extern const char* password;

// --- Pins ---
#define RX_PIN 16  
#define TX_PIN 17  
#define CC3200_BAUD 115200
#define LED_RED_PIN   27 
#define LED_GREEN_PIN 25 
#define LED_BLUE_PIN  26 
#define LED 2

// --- Data Structure ---
struct AWS_data {
  String bars;
  String c1;
  String c2;
  String c3;
  String grav;
  String rate;
};

#endif