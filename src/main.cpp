#include <Arduino.h>
#include <Button2.h>
#include <ArduinoLog.h>

#include "pinDefs.h"
#include "stripper.h"
#include "toucher.h"
#include "elceder.h"
#include "netUtils.h"
#include "commonFwUtils.h"

Button2 button;

void pressCB(Button2& btn) {
  Serial.println("1");
}
void spawn_tasks();

void touchbut_cb(){
    elceder_fill_row(1,2000,"DINGDONG");  
}

void setup()
{
  Serial.begin(115200);

  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.infoln("Initalizing board...");

  spawn_tasks();

  button.begin(PIN_BUT,INPUT,false);
  button.setTapHandler(pressCB);
  pinMode(PIN_LED, OUTPUT);

  attach_cb(touchbut_cb);
}

void loop(){
  handle_io_pattern(PIN_LED,PATTERN_HBEAT);
  button.loop();
  vTaskDelay(150);
}

void serial_read_task(void* params){
  static int buf_ptr = 0;
  static char str_buf[32] = {0};

  Serial.setTimeout(0);

  while (true){

    int byte = Serial.read();
    if (byte == 13){
      elceder_fill_row(0,0,"%s",str_buf);
      buf_ptr = 0;
      memset(str_buf,0x00,32);
      continue;
    } else if (byte == 9){
      elceder_fill_row(1,0,"%s",str_buf);
      buf_ptr = 0;
      memset(str_buf,0x00,32);
      continue;
    }

    if (byte > 0){
      if (buf_ptr < 32){
        str_buf[buf_ptr++] = (char)byte;
      }
    } else {
      vTaskDelay(50);
    }
  }
}


void diagnostics_task(void * parameter);

TaskHandle_t task_handles[10] = {0};

void spawn_tasks(){
  xTaskCreate( stripper_task, "stripper task",
    1500, NULL, 6, &task_handles[0] );
    
  xTaskCreate( elceder_task, "elceder task",
    3000, NULL, 2, &task_handles[1] );

  xTaskCreate( wifi_task, "wifi task",
    5000, NULL, 3, &task_handles[2] ); 

  xTaskCreate( serial_read_task, "serial task",
    2000,NULL, 3, &task_handles[3] );

  xTaskCreate( touch_task, "touch task",
    2000, NULL, 3, &task_handles[4] );

  xTaskCreate( diagnostics_task, "diag task",
    3000, NULL, 3, &task_handles[5] );
}

// this will maybe print CPU usage stats at some point
// but it would require us to switch to espidf framework
// as freertos kernel for aduino is baked in.

void diagnostics_task(void * parameter){
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 30 * 1000;
  xLastWakeTime = xTaskGetTickCount();

  while(1){
    int uptime_sec = xTaskGetTickCount()/1000;
    Serial.printf(
      "\nBoard uptime: %dm %02ds \n\r"
      "Free task heap:\n\r",
      uptime_sec/60, uptime_sec%60
    );
    for (int i = 0; i< 10;i++){
      if (task_handles[i] != 0){
        Serial.printf(
           "%-16s%d\n\r", 
           pcTaskGetTaskName(task_handles[i]),
           (int)uxTaskGetStackHighWaterMark( task_handles[i] ));
      }      
    }
    vTaskDelayUntil(&xLastWakeTime,xFrequency);
  }
}
