#include <Arduino.h>

#include "netUtils.h"
#include <Button2.h>

#include "pinDefs.h"
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

  button.begin(PIN_BUT,INPUT,false, true);
  button.setTapHandler(pressCB);
  pinMode(PIN_LED, OUTPUT);  
}

void loop(){
  static int test = 0;
  static uint8_t heartbeat_pattern[] = {1,0,0,1,0,0,0,0,0,0,0,0,0};

  // elceder_fill_row(1,"test %d",test++);
  
  uint8_t pattern_index = (test++)% sizeof(heartbeat_pattern);
  digitalWrite(PIN_LED, heartbeat_pattern[pattern_index]);

  vTaskDelay(150);
}

void serial_read_task(void* params){
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
    } else if (byte == 9){
      elceder_fill_row(1,"%-16s     ",str_buf);
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

const float aver_ratio = 100.0f;
const float addition_factor = 1.0f/aver_ratio;
const float sum_factor = 1.0f-addition_factor;

const int cbuts[3] = {33, 27, 14};

void touch_task (void* params){
  float accs[3] = {0,0,0};
  float meases[3] = {0,0,0};
  uint32_t counter = 0;

  while (true){
    for (int i=0;i<3;i++){
      meases[i] = (float)touchRead(cbuts[i]);

      accs[i] = 
        accs[i]*sum_factor + 
        meases[i]*addition_factor;
    }

    elceder_fill_row(
      0,"%03d/%03d/%03d/%04d",
      (int)meases[0],
      (int)meases[1],
      (int)meases[2],
      counter%10000
    );
      
    elceder_fill_row(
      1,"%03d/%03d/%03d       ",
      (int)(accs[0]-meases[0]),
      (int)(accs[1]-meases[1]),
      (int)(accs[2]-meases[2])
    );
    counter++;
    vTaskDelay(100);
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

  // xTaskCreate(
  //   wifi_task,
  //   "wifi task",
  //   10000,
  //   NULL,
  //   3,
  //   NULL
  // );
  
  // xTaskCreate(
  //   serial_read_task,
  //   "serial task",
  //   10000,
  //   NULL,
  //   3,
  //   NULL
  // );

xTaskCreate(
    touch_task,
    "touch task",
    10000,
    NULL,
    3,
    NULL
  );

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
