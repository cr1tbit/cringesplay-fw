
#include <FastLED.h>

#define SCREEN_LEDS 50
#define PIN 12

CRGBArray<SCREEN_LEDS> leds;


void stripper_task(void * parameter){
    // Adafruit_NeoPixel strip = Adafruit_NeoPixel(SCREEN_LEDS, PIN, NEO_RGB + NEO_KHZ800);
    FastLED.addLeds<NEOPIXEL,PIN>(leds, SCREEN_LEDS);
    int j = 0;

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;
    xLastWakeTime = xTaskGetTickCount();
    // strip.begin();

    while (1){
        for (int i=0;i<SCREEN_LEDS;i++){
            leds[i] = CHSV((i * 256 / 24) + j ,0xFF,0xFF);
        }
        FastLED.show();
        j++;
        vTaskDelayUntil(&xLastWakeTime,xFrequency);
    }
}