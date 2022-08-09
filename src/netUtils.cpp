
#include "netUtils.h"
#include "wifi_creds.h"
#include "elceder.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

WiFiClient client;

const int httpPort = 80;
/*
String requestWhois()
{
  String payload = "";
  const char *root_ca = \
    "  -----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----";

  HTTPClient http;
  http.begin("https://whois.at.hsp.sh/api/now", root_ca); //Specify the URL and certificate
  int httpCode = http.GET();                              //Make the request

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Requesting");
  if (httpCode > 0)
  { //Check for the returning code

    payload = http.getString();
    http.end(); //Free the resources
    lcd.setCursor(0, 1);
    lcd.print(httpCode + " " + payload);
  }
  else
  {
    Serial.println(F("Error on HTTP request"));
    lcd.print(httpCode + " error");
    delay(1000);
    return "";
  }
  http.end(); //Free the resources
  return payload;
}*/

#define WIFI_RST_TIMEOUT_MS 1000*30

void connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWD);
  
  uint32_t timeoutCounterMs = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    timeoutCounterMs+=100;
    vTaskDelay(100);

    // if (timeoutCounterMs > WIFI_RST_TIMEOUT_MS){
    //   ESP.restart();
    // }
  }

  //lcd.print(WiFi.localIP());
  //void elceder_fill_row(0,"%s",WiFi.localIP());
  elceder_fill_row(0,"%s",WiFi.localIP().toString().c_str());
  vTaskDelay(1000);

  //json = getWhois();
}



void wifi_task(void* params){
  connect();

  while(1){
    vTaskDelay(1000);
    //requestWhois();
  }
}



/*
void loop()
{
  if(WiFi.status() != WL_CONNECTED) connect();

  if (millis() - lastRequest >= 60 * 1000)
  {
    json = getWhois();
    lastRequest = millis();
  }

  static int counter = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People: ");
  lcd.print(json["headcount"].as<long>());
  if (json["users"].size() > 0)
  {
    int index = (counter++) % (json["users"].size());
    lcd.setCursor(0, 1);
    lcd.print(json["users"][index].as<String>());
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("Pool's Closed");
  }
  delay(1000);
}

*/
/*
StaticJsonDocument<512> getWhois()
{
  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, requestWhois());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Parsing");
  if (error != DeserializationError::Ok)
  {
    lcd.setCursor(0, 1);
    lcd.print(error.c_str());
    delay(1000);
  }
  return json;
}
*/
