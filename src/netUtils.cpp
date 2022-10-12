
#include "netUtils.h"
#include "secrets.h"
#include "elceder.h"
#include "commonFwUtils.h"

#include <WiFi.h>

#include <ArduinoOTA.h>
#include <LeifHomieLib.h>


#define WIFI_RST_TIMEOUT_MS 1000*30
#define WIFI_ATTEMPT_TICK_RATE 500

void update_wifi_status()
{
  static bool recently_connected;
  uint32_t timeoutCounterMs = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    elceder_fill_row(1,0,"Connecting: %03ds",timeoutCounterMs/1000);
    timeoutCounterMs+=WIFI_ATTEMPT_TICK_RATE;
    vTaskDelay(WIFI_ATTEMPT_TICK_RATE);
    // if (timeoutCounterMs > WIFI_RST_TIMEOUT_MS){
    //   ESP.restart();
    // }
    recently_connected = true;
  }
  timeoutCounterMs = 0;
  if (recently_connected){
    elceder_fill_row(1,0,"%s",WiFi.localIP().toString().c_str());
    Serial.printf("My IP: %s",WiFi.localIP().toString().c_str());
    recently_connected = false;
  }
}

HomieDevice homie;

void init_homie_stuff(HomieDevice* pHomie){
  pHomie->strFriendlyName = "Cringesplay demo";
  pHomie->strID = "cringesplay";
  pHomie->strID.toLowerCase();

  pHomie->strMqttServerIP = "192.168.0.44";
	// pHomie->strMqttUserName = MQTT_USERNAME;
	// pHomie->strMqttPassword = MQTT_PASSWD;
  pHomie->Init();
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