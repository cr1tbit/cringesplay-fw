#include <Arduino.h>

#include "netUtils.h"
#include <Button2.h>

#include "pinDefs.h"
#include "stripper.h"
#include "elceder.h"

#include "commonFwUtils.h"

Button2 button;

void pressCB(Button2& btn) {
  Serial.println("1");
}
void spawn_tasks();

void setup()
{
  Serial.begin(115200);
  spawn_tasks();

  button.begin(PIN_BUT,INPUT,false, true);
  button.setTapHandler(pressCB);
  pinMode(PIN_LED, OUTPUT);  
}

void loop(){
  handle_io_pattern(PIN_LED,PATTERN_HBEAT);
  vTaskDelay(150);
}

void serial_read_task(void* params){
  static int buf_ptr = 0;
  static char str_buf[32] = {0};

  Serial.setTimeout(0);

  while (true){
    button.loop();

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

const float aver_ratio = 1000.0f;
const float addition_factor = 1.0f/aver_ratio;
const float sum_factor = 1.0f-addition_factor;

// the value read from cap is in range 0-100
// for example let it be 80
// by trial and error, touching a cap button
// increases the read value by about 10%
// so I assume 7% increase for now.
const float trigger_factor = 0.07f;

//sometimes captouch glitches - ignore those values
const float glitch_factor = 0.20f;

const int cbuts[3] = {33, 27, 14};

void touch_task (void* params){
  float accs[3] = {0,0,0};
  float meases[3] = {0,0,0};
  uint32_t counter = 0;

  bool button_pressed[3] = {0,0,0};

  while (true){
    vTaskPrioritySet(NULL,1);  //block task switch cause it messes with touchRead

    for (int i=0;i<3;i++){
      meases[i] = (float)touchRead(cbuts[i]);

      //update averaging buffers
      if (accs[i] < 30){
        //on first run skip averaging
        accs[i] = meases[i];
      } else {
        accs[i] = 
          accs[i]*sum_factor + 
          meases[i]*addition_factor;
      }

      float delta_touch = accs[i]-meases[i];
      if ((delta_touch > (accs[i] * trigger_factor))&&(delta_touch < (accs[i] * glitch_factor))){
        if (button_pressed[i] == false){
          button_pressed[i] = true;
          elceder_fill_row(0,5000,"B%d %02d|%02d|%02d",i,(int)accs[i],(int)meases[i],(int)delta_touch);
        }
      } else {
        button_pressed[i] = false;
      }
    }
    counter++;
    vTaskPrioritySet(NULL,3);
    vTaskDelay(100);
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
    for (int i = 0; i< 10;i++){
      if (task_handles[i] != 0){
        Serial.printf(
           "%s: %d\n\r", 
           pcTaskGetTaskName(task_handles[i]),
           (int)uxTaskGetStackHighWaterMark( task_handles[i] ));
      }      
    }
    vTaskDelayUntil(&xLastWakeTime,xFrequency);
  }
}
