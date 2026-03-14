#include "config.h"
#include "hardware.h"
#include "networking.h"
#include "aws_logic.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>

// Define the externs from config.h here
const char* awsEndpoint = "a1tltfg29hzoaq-ats.iot.us-east-2.amazonaws.com";
const char* thingName = "Avolkov_CC3200_Board";
const char* ssid = "Ipod";
const char* password = "12345678";

void sendToCC3200(String bars, String c1, String c2, String c3, String grav, String rate) {
    // Construct the string: DATA<val1,val2,val3,val4,val5,val6>
    String output = "DATA<" + bars + "," + c1 + "," + c2 + "," + c3 + "," + grav + "," + rate + ">\n";
    
    // Send over Serial2 to the CC3200
    Serial2.print(output);
    
    Serial.print("Sent to CC3200: ");
    Serial.print(output);
    
}


void parseCC3200Data(String input) {
    input.trim();

    if (input.startsWith("SEND_AWS")) {
        int openBracket = input.indexOf('<');
        int closeBracket = input.indexOf('>');

        if (openBracket != -1 && closeBracket != -1 && closeBracket > openBracket) {
            String payload = input.substring(openBracket + 1, closeBracket);
            
            // Using a simple while-loop or manual index to find commas
            // num_bars, color1, color2, color3, gravity_shift, sampling_rate
            int comma1 = payload.indexOf(',');
            int comma2 = payload.indexOf(',', comma1 + 1);
            int comma3 = payload.indexOf(',', comma2 + 1);
            int comma4 = payload.indexOf(',', comma3 + 1);
            int comma5 = payload.indexOf(',', comma4 + 1);

            if (comma5 != -1) { // Ensure all 6 values are present
                String num_bars      = payload.substring(0, comma1);
                String color1        = payload.substring(comma1 + 1, comma2);
                String color2        = payload.substring(comma2 + 1, comma3);
                String color3        = payload.substring(comma3 + 1, comma4);
                String gravity_shift = payload.substring(comma4 + 1, comma5);
                String sampling_rate = payload.substring(comma5 + 1);

                AWS_data data = {
                  num_bars,
                  color1,
                  color2,
                  color3,
                  gravity_shift,
                  sampling_rate

                };


                Serial.println("--- AWS Payload Received ---");
                Serial.printf("Bars: %s | Colors: %s, %s, %s | Gravity: %s | Rate: %s\n", 
                               num_bars.c_str(), color1.c_str(), color2.c_str(), 
                               color3.c_str(), gravity_shift.c_str(), sampling_rate.c_str());
                
                // Construct JSON and send to AWS
                delay(1000); // Delay
                sendAwsData(data); 
                fast_blink();
            }
        }
    } 
    else if (input.startsWith("GET_AWS")) {
        // Assume you fetched these values from AWS variables
        String data = getAwsState();
        if (data == ""){
          return; // Failiure to get anything
        }

        AWS_data receivedData;
        unpackAwsJson(data, receivedData);

        
        sendToCC3200(receivedData.bars, receivedData.c1, receivedData.c2, 
                 receivedData.c3, receivedData.grav, receivedData.rate);
        // delay(1000);
        // sendToCC3200("12", "0x001F", "0x001F", 
        //          "0x001F", "12", "4000");
        fast_blink();
    }
    else if (input.startsWith("STATUS")){
      Serial2.print("ESPCON\n");
      Serial.println("Return Info sent ");
      fast_blink();
    }
    else{
       Serial.println("---        Unable to parse calls        ---");
       Serial.println("--- Must start with GET_AWS or SEND_AWS ---");
    }
}



void setup() {
    Serial.begin(115200); 
  pinMode(LED,OUTPUT);

  // DevKit V1 uses Serial2 for secondary UART
  Serial2.begin(CC3200_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Configure LED pins
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  
  setManualTime();
  if (!LittleFS.begin(false)) { // DevKit V1 LittleFS initialization
      Serial.println("Mounting failed!");
      return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    setRGB(32, 0, 0); // Red for connecting
    delay(500);
    Serial.print(".");
    digitalWrite(LED,HIGH); // Green ON
    delay(100); 
    digitalWrite(LED,LOW);   // All OFF
    delay(100); 
  }
  
  setRGB(0, 0, 0); // Clear LED after connect
  Serial.println("\nWiFi Connected.");
  slow_blink();
  digitalWrite(LED,HIGH); // STATUS: ON
  configTime(0, 0, "pool.ntp.org");
  // sendAwsMessage("TEST MESSAGE");
}

void loop() {
    // Use Serial2 instead of Serial1 for DevKit V1
  if (Serial2.available()) {
    String receivedMsg = Serial2.readStringUntil('\n');
    receivedMsg.trim();

    if (receivedMsg.length() > 0) {
      // if (strncmp( receivedMsg )     )
      Serial.print("Received from CC3200: ");
      Serial.println(receivedMsg);
      parseCC3200Data(receivedMsg);
      
      // sendAwsMessage(receivedMsg);
    }
  }
}