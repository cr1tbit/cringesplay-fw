#include <Arduino.h>

#include "netUtils.h"
#include <Button2.h>


#include "stripper.h"
#include "elceder.h"

Button2 button;

void pressCB(Button2& btn) {
  Serial.println("1");
}

void spawn_tasks();

void setup()
{
  Serial.begin(115200);
  spawn_tasks();

  button.begin(35,INPUT,false, true);

  button.setTapHandler(pressCB);
}


void serial_read_tast(void* params){
  static int buf_ptr = 0;
  static char str_buf[16] = {0};

  Serial.setTimeout(0);

  while (true){
    button.loop();

    int byte = Serial.read();
    if (byte == 13){
      elceder_fill_row(0,"%-16s     ",str_buf);
      buf_ptr = 0;
      memset(str_buf,0x00,16);
      continue;
    }
    if (byte > 0){
      if (buf_ptr < 16){
        str_buf[buf_ptr++] = (char)byte;
      }
    } else {
      vTaskDelay(50);
    }
  }

}

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
  
  xTaskCreate(
    serial_read_tast,
    "serial task",
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


//this will print CPU usage stats at some point

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
