
#include "netUtils.h"
#include "secrets.h"
#include "elceder.h"
#include "commonFwUtils.h"

#include <WiFi.h>

#include <ArduinoOTA.h>
#include <LeifHomieLib.h>


#define WIFI_RST_TIMEOUT_MS 1000*30
#define WIFI_ATTEMPT_TICK_RATE 500

HomieProperty* shitton = NULL;

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
    Serial.printf("My IP: %s",WiFi.localIP().toString().c_str());
    recently_connected = false;
  }
}

HomieDevice homie;

void init_homie_stuff(HomieDevice* pHomie){
  pHomie->strFriendlyName = "domohsp-alpha";
  pHomie->strID = "domohsp-alpha";
  pHomie->strID.toLowerCase();

  pHomie->strMqttServerIP = "192.168.88.170";
	pHomie->strMqttUserName = MQTT_USERNAME;
	pHomie->strMqttPassword = MQTT_PASSWD;
  pHomie->Init();
}


HomieProperty* init_shitton(HomieNode* pNode, int pin_num, String id){
  // Button* pButton = new Button(pin_num, INPUT);
  HomieProperty *pProperty = pNode->NewProperty();

  pProperty->strFriendlyName = id;
  pProperty->strID = id;
  pProperty->datatype = homieBool;
  pProperty->SetBool(true);
  pProperty->strFormat = "";
  // pButton->onChange([pButton,pProperty]() {
  //   pProperty->SetBool(pButton->isPressed());
  // });
  return pProperty;
}

void wifi_task(void* params){
  WiFi.mode(WIFI_STA);
  WiFi.begin(IOT_WIFI_NAME, IOT_WIFI_PASSWD);

  update_wifi_status();
  begin_hspota();

  HomieNode *pNode = homie.NewNode();
  pNode->strID = "properties";
  pNode->strFriendlyName = "Properties";

  shitton = init_shitton(pNode, 0, "shitton");

  init_homie_stuff(&homie);  

  int wifi_loop_cnt = 0;

  while(1){
    update_wifi_status();
    ArduinoOTA.handle();
    homie.Loop();

    if ((wifi_loop_cnt%100) == 0){
      elceder_fill_row(1,0,"%s",WiFi.localIP().toString().c_str());
    } else if ((wifi_loop_cnt%50) == 0){
      elceder_fill_row(1,0,"MQTT status: %s",homie.IsConnected()? "yes" : "no");
    }

    wifi_loop_cnt++;
    vTaskDelay(100);
  }
}