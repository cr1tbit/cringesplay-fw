
#include "netUtils.h"
#include "secrets.h"
#include "elceder.h"
#include "commonFwUtils.h"

#include <WiFi.h>
#include <ArduinoJson.h>

#include <ArduinoOTA.h>


#define WIFI_RST_TIMEOUT_MS 1000*30
#define WIFI_ATTEMPT_TICK_RATE 500

void update_wifi_status()
{
  static bool recently_connected;
  uint32_t timeoutCounterMs = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    elceder_fill_row(1,"Connecting: %03ds",timeoutCounterMs/1000);
    timeoutCounterMs+=WIFI_ATTEMPT_TICK_RATE;
    vTaskDelay(WIFI_ATTEMPT_TICK_RATE);

    // if (timeoutCounterMs > WIFI_RST_TIMEOUT_MS){
    //   ESP.restart();
    // }
    recently_connected = true;
  }
  timeoutCounterMs = 0;
  if (recently_connected){
    elceder_fill_row(1,"%s    ",WiFi.localIP().toString().c_str());
    Serial.printf("My IP: %s",WiFi.localIP().toString().c_str());
    recently_connected = false;
  }
}

void wifi_task(void* params){
  WiFi.mode(WIFI_STA);
  WiFi.begin(IOT_WIFI_NAME, IOT_WIFI_PASSWD);

  update_wifi_status();
  begin_hspota();

  while(1){
    update_wifi_status();
    ArduinoOTA.handle();

    vTaskDelay(100);
    //requestWhois();
  }
}