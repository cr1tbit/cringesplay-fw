

#include <Wire.h>
#include <LiquidCrystal.h>

#include <stdio.h>
#include <stdarg.h>

#include "elceder.h"
#include "pinDefs.h"

#define CONTRAST 110

LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_4, PIN_5, PIN_6, PIN_7, 23, POSITIVE);

QueueHandle_t elceder_msg_queue;


void elceder_init(){

    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_5V_ENA, OUTPUT);


    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_5V_ENA, HIGH);

    lcd.backlight();
    lcd.begin(16, 2); // initialize the lcd

    lcd.home(); // go home

    elceder_msg_queue = xQueueCreate( 2, sizeof( elceder_msg_t ) );
    if (elceder_msg_queue == NULL){
        lcd.print("queue failed");
    }
}

void elceder_queue_text(elceder_msg_t* msg){
    if (elceder_msg_queue != NULL){
        xQueueSend( 
            elceder_msg_queue,
            ( void * ) msg,
            0 
        );
    }
    
}

void elceder_fill_row(int row, const char* fmt, ...){
    elceder_msg_t test_msg;
    va_list args;

    va_start(args, fmt);

    vsnprintf(test_msg.str, sizeof(test_msg.str), fmt, args);
    test_msg.row = row;

    elceder_queue_text(&test_msg);
    va_end(args);
}

void elceder_task(void* params){
    elceder_init();
    elceder_msg_t msg;

    while(1){
        xQueueReceive( 
            elceder_msg_queue,
            &msg,
            portMAX_DELAY
        );                        

        lcd.setCursor(0, msg.row);
        lcd.print(msg.str);
    }
}