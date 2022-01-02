
#include <Adafruit_NeoPixel.h>


#define SCREEN_LEDS 10
#define PIN 12

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(Adafruit_NeoPixel* strip, byte WheelPos) {
  if (WheelPos < 85) {
    return strip->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip->Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip->Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


void stripper_task(void * parameter){
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(SCREEN_LEDS, PIN, NEO_RGB + NEO_KHZ800);
    int j = 0;

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 15;
    xLastWakeTime = xTaskGetTickCount();

    strip.begin();

    while (1){
        for (int i=0;i<SCREEN_LEDS;i++){
            strip.setPixelColor(
                i,
                Wheel(&strip,((i * 256 / strip.numPixels()) + j) & 255)
            );
        }
        strip.show();
        j++;
        vTaskDelayUntil(&xLastWakeTime,xFrequency);
    }
}