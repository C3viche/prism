#include <Arduino.h>
#include "config.h"
#include "hardware.h"

void setRGB(int r, int g, int b) {
    analogWrite(LED_RED_PIN, r);
    analogWrite(LED_GREEN_PIN, g);
    analogWrite(LED_BLUE_PIN, b);
}

void blink_green(){
  for (int i = 0; i < 5; i++){
    setRGB(0, 128, 0); // Green ON
    delay(200); 
    setRGB(0, 0, 0);   // All OFF
    delay(200); 
  }
}

void fast_blink(){
  for (int i = 0; i < 5; i++){
    digitalWrite(LED,HIGH); // Green ON
    delay(100); 
    digitalWrite(LED,LOW);   // All OFF
    delay(100); 
  }
}

void slow_blink(){
  for (int i = 0; i < 3; i++){
    digitalWrite(LED,HIGH); // Green ON
    delay(500); 
    digitalWrite(LED,LOW);   // All OFF
    delay(500); 
  }
  
}

