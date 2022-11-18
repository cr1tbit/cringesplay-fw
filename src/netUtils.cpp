#include "netUtils.h"
#include "secrets.h"
#include "elceder.h"
#include "commonFwUtils.h"

#include <WiFi.h>

#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <LeifHomieLib.h>
#include <ArduinoLog.h>
#include "cringesplayFwSetup.h"


#define WIFI_RST_TIMEOUT_MS 1000*30
#define WIFI_ATTEMPT_TICK_RATE 500

void update_wifi_status()
{
  static bool recently_connected;
  uint32_t timeoutCounterMs = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    elceder_fill_row(1,2000,"Connecting: %03ds",timeoutCounterMs/1000);
    timeoutCounterMs+=WIFI_ATTEMPT_TICK_RATE;
    vTaskDelay(WIFI_ATTEMPT_TICK_RATE);
    // if (timeoutCounterMs > WIFI_RST_TIMEOUT_MS){
    //   ESP.restart();
    // }
    recently_connected = true;
  }
  timeoutCounterMs = 0;
  if (recently_connected){
    elceder_fill_row(1,4000,"%s",WiFi.localIP().toString().c_str());
    Serial.printf("My IP: %s",WiFi.localIP().toString().c_str());
    recently_connected = false;
  }
}

HomieDevice homie;

void init_homie_stuff(HomieDevice* pHomie){
  pHomie->strFriendlyName = "Cringesplay demo";
  pHomie->strID = "cringesplay";
  pHomie->strID.toLowerCase();


  IPAddress mqttServerIp((uint32_t)0);
  if(!WiFiGenericClass::hostByName(mqttHost, mqttServerIp)){
    Log.errorln("Couldn't fetch IP by hostname: %s !",mqttHost);
    return;
  }
  pHomie->strMqttServerIP = mqttServerIp.toString();

  Log.noticeln("MQTT server IP: %s",pHomie->strMqttServerIP.c_str());

	// pHomie->strMqttUserName = MQTT_USERNAME;
	// pHomie->strMqttPassword = MQTT_PASSWD;
  pHomie->Init();
}


String getJsonFromAddr(String addr, const char* root_ca){
  if (WiFi.status() != WL_CONNECTED){
    return "error";
  }
  String payload = "";

  HTTPClient http;

  if (root_ca == NULL){
    http.begin(addr);
  } else {
    http.begin(addr, root_ca);
  }
  
  int httpCode = http.GET();

  if (httpCode > 0) { 
    return http.getString();
  } else {
    Serial.println("Error on HTTP request");
    return "error";
  }
  http.end(); //Free the resources
  return payload;
}

void wifi_task(void* params){
  WiFi.mode(WIFI_STA);
  WiFi.begin(IOT_WIFI_NAME, IOT_WIFI_PASSWD);

  update_wifi_status();
  begin_hspota();

  HomieNode *pNode = homie.NewNode();
  pNode->strID = "properties";
  pNode->strFriendlyName = "Properties";

  init_homie_stuff(&homie);  

  while(1){
    update_wifi_status();
    ArduinoOTA.handle();
    homie.Loop();

    vTaskDelay(100);
  }
}

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

lcd.print(json["headcount"].as<long>());
  if (json["users"].size() > 0)
  {
    int index = (counter++) % (json["users"].size());
    lcd.setCursor(0, 1);
    lcd.print(json["users"][index].as<String>());
  }


    lcd.print("Pool's Closed");


*/


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
  }
  delay(1000);
}

*/
