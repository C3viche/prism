#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "config.h"
#include "networking.h"
#include "hardware.h"

String packageAwsJson(AWS_data data);
void unpackAwsJson(String json, AWS_data &data);
void sendAwsData(AWS_data data);
String getAwsState();