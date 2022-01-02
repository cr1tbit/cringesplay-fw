#include <Arduino.h>


//#include "wifi_creds.h"
#include "netUtils.h"

#include "stripper.h"
#include "elceder.h"


//unsigned long lastRequest = 0;
//StaticJsonDocument<512> json;
//StaticJsonDocument<512> getWhois();

void spawn_tasks();

void setup()
{
  Serial.begin(115200);
  spawn_tasks();
}

/*
void diagnostics_task(void * parameter){
  char diag_buff[256];


  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 100;
  xLastWakeTime = xTaskGetTickCount();

  while(1){
    vTaskGetRunTimeStats((char*)&diag_buff);
    //Serial.println((char*)&diag_buff);
    Serial.println("test");

    vTaskDelayUntil(&xLastWakeTime,xFrequency);

  }
}
*/

void spawn_tasks(){
  xTaskCreate(
    stripper_task,
    "stripper task",
    1000,
    NULL,
    6,
    NULL
  );
  
  xTaskCreate(
    elceder_task,
    "elceder task",
    1000,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    wifi_task,
    "wifi task",
    10000,
    NULL,
    3,
    NULL
  );
  
}

void loop(){
  static int test = 0;
  elceder_fill_row(1,"test %d",test++);
  vTaskDelay(1000);
}
