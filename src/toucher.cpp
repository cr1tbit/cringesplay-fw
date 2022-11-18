#include <Arduino.h>
#include "elceder.h"


#define CB_CNT 3

void (*toucher_evt_cb)() = {0};


// bool attach_cb(uint8_t evt_no, void* ptr){
//     if (evt_no < CB_CNT){
//         return false;
//     }
//     toucher_evt_cbs[evt_no] = ptr;
// }


void attach_cb(void (*ptr)()){
    toucher_evt_cb = ptr;
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
        //if the value is absurdely low, assume the code just started
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
          //elceder_fill_row(0,5000,"B%d %02d|%02d|%02d",i,(int)accs[i],(int)meases[i],(int)delta_touch);
          //elceder_fill_row(0,5000,"B%d %02d|%02d|%02d",i,(int)accs[i],(int)meases[i],(int)delta_touch);
          if (toucher_evt_cb != NULL){
              toucher_evt_cb();
          }
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