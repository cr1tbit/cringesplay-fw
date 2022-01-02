#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//#include "wifi_creds.h"

//String requestWhois();
//void connect();

void wifi_task(void* params);