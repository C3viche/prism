#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "config.h"
#include "networking.h"
#include "hardware.h"
#include "aws_logic.h"

String packageAwsJson(AWS_data data) {
    // Allocate a buffer. 512 bytes is plenty for this small struct.
    StaticJsonDocument<512> doc;
    
    // Create the nested structure: {"state": {"desired": { ... }}}
    JsonObject state = doc.createNestedObject("state");
    JsonObject desired = state.createNestedObject("desired");

    // Only add to the JSON if the string is not empty
    if (data.bars.length() > 0)          desired["num_bars"]      = data.bars;
    if (data.c1.length() > 0)            desired["color1"]        = data.c1;
    if (data.c2.length() > 0)            desired["color2"]        = data.c2;
    if (data.c3.length() > 0)            desired["color3"]        = data.c3;
    if (data.grav.length() > 0)          desired["gravity_shift"] = data.grav;
    if (data.rate.length() > 0)          desired["sampling_rate"] = data.rate;

    // Convert the object into a single String
    String output;
    serializeJson(doc, output);
    return output;
}


void unpackAwsJson(String json, AWS_data &data) {
    // 1. Allocate the buffer (1024 is safe for a full Shadow JSON)
    StaticJsonDocument<1024> doc;

    // 2. Parse the JSON
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("JSON Parse Failed: ");
        Serial.println(error.f_str());
        return;
    }

    // 3. Navigate to state -> desired
    // Using JsonObject allows us to check if the keys exist before reading
    JsonObject desired = doc["state"]["desired"];

    if (!desired.isNull()) {
        // Use as<String>() to safely extract values. 
        // If the key is missing in AWS, it returns an empty string "".
        data.bars  = desired["num_bars"].as<String>();
        data.c1    = desired["color1"].as<String>();
        data.c2    = desired["color2"].as<String>();
        data.c3    = desired["color3"].as<String>();
        data.grav  = desired["gravity_shift"].as<String>();
        data.rate  = desired["sampling_rate"].as<String>();
        
        Serial.println("--- Extracted Data from AWS ---");
        Serial.printf("Bars: %s, C1: %s, Rate: %s\n", data.bars.c_str(), data.c1.c_str(), data.rate.c_str());
    } else {
        Serial.println("Error: 'desired' state not found in JSON");
    }
}

void sendAwsData(AWS_data data) {
  WiFiClientSecure client;
  String ca = loadCert("/RootCA.pem");
  String cert = loadCert("/certificate.pem.crt");
  String key = loadCert("/private.pem.key");
  if (ca.length() == 0 || cert.length() == 0 || key.length() == 0) {
    Serial.println("Error: Certificates missing!");
    return;
  }
  client.setCACert(ca.c_str());
  client.setCertificate(cert.c_str());
  client.setPrivateKey(key.c_str());

  HTTPClient http;
  String url = "https://" + String(awsEndpoint)+ ":8443/things/" + thingName + "/shadow";
  String payload = packageAwsJson(data);
  // String payload = "{\"state\":{\"desired\":{\"cc3200_msg\":\"" + data.bars + "\"}}}";
  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.printf("AWS Sync Success: %d\n", httpResponseCode);
      blink_green();
    } else {
      Serial.printf("AWS Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  }


}





String getAwsState() {
  WiFiClientSecure client;
  String ca = loadCert("/RootCA.pem");
  String cert = loadCert("/certificate.pem.crt");
  String key = loadCert("/private.pem.key");

  if (ca.length() == 0 || cert.length() == 0 || key.length() == 0) {
    Serial.println("Error: Certificates missing!");
    return "";
  }

  client.setCACert(ca.c_str());
  client.setCertificate(cert.c_str());
  client.setPrivateKey(key.c_str());

  HTTPClient http;
  String url = "https://" + String(awsEndpoint)+ ":8443/things/" + thingName + "/shadow";
  // String payload = "{\"state\":{\"desired\":{\"cc3200_msg\":\"" + incomingData + "\"}}}";

  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.printf("AWS Get Success: %d\n", httpResponseCode);
      blink_green();
      String payload = http.getString();
      return payload;

    } else {
      Serial.printf("AWS Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  }
  return "";
}
