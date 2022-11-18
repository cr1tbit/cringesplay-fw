#include <Wire.h>
#include <LiquidCrystal.h>
#include <ArduinoLog.h>

#include <stdio.h>
#include <stdarg.h>

#include "elceder.h"
#include "pinDefs.h"

#include "cringesplayFwSetup.h"


#define CONTRAST 110

LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_4, PIN_5, PIN_6, PIN_7, 23, POSITIVE);

QueueHandle_t elceder_msg_queue = NULL;

#define LCD_ROW_COUNT 2
#define LCD_LENGTH 16

void elceder_init(){
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_5V_ENA, OUTPUT);

    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_5V_ENA, HIGH);

    lcd.backlight();
    lcd.begin(LCD_LENGTH, LCD_ROW_COUNT);
    lcd.home();

    lcd.createChar(1,(uint8_t* const)customCharArray[0]);
    lcd.createChar(2,(uint8_t* const)customCharArray[1]);

    Log.infoln("Elceder initialized...");

    elceder_msg_queue = xQueueCreate( LCD_ROW_COUNT, sizeof( elceder_msg_t ) );
    if (elceder_msg_queue == NULL){
        lcd.print("lcd queue error");
    }
}

void elcdeder_print_to_row(int row, char* const str_to_write){
    lcd.setCursor(0, row);
    lcd.print(str_to_write);

    int delta_chars = LCD_LENGTH - strnlen(str_to_write,LCD_LENGTH);
    for (int i = 0; i< delta_chars; i++){
        lcd.write(" "); //oferwrite previous stuff
    }
}

void elceder_queue_text(elceder_msg_t* msg){
    if (elceder_msg_queue != NULL){
        xQueueSend( elceder_msg_queue,
            (void *) msg, 0 );
    }    
}

void elceder_fill_row(int row, int message_timeout_ms, const char* fmt, ...){
    elceder_msg_t _msg;
    va_list args;

    va_start(args, fmt);

    vsnprintf(_msg.str, sizeof(_msg.str), fmt, args);
    _msg.row = row;
    _msg.message_timeout = message_timeout_ms;

    Log.infoln("LCD r%d: %s",row, _msg.str);

    elceder_queue_text(&_msg);
    va_end(args);
}

void elceder_task(void* params){
    elceder_init();
    elceder_msg_t msg;

    char default_row_buf[LCD_ROW_COUNT][32] = {0};
    char row_buf[LCD_ROW_COUNT][32] = {0};
    TickType_t row_timeout_tick[2];

    strncpy(
        default_row_buf[0],
        cfw_splash_text_l0,
        sizeof(default_row_buf[0])
    );

    if (strnlen(cfw_splash_text_l1,3) > 0){
        strncpy(
            default_row_buf[1],
            cfw_splash_text_l1,
            sizeof(default_row_buf[0])
        );
    }    

    elceder_fill_row(0,2000,"Hello world!");

    while(1){
        BaseType_t queue_succ = 
            xQueueReceive( elceder_msg_queue,
                &msg, 0 );                        

        if (queue_succ) {
            strncpy(row_buf[msg.row],msg.str,sizeof(row_buf[0]));
            elcdeder_print_to_row(msg.row,msg.str);
            if (msg.message_timeout == 0){
                //fill the default buf for non-ephemeral messages
                strncpy(default_row_buf[msg.row],msg.str,sizeof(default_row_buf[0]));
                row_timeout_tick[msg.row] = 0;
            } else {
                row_timeout_tick[msg.row] = xTaskGetTickCount() + msg.message_timeout;
            }
        }
        int scroll_tick = xTaskGetTickCount()/666;

        for (int i = 0; i<LCD_ROW_COUNT; i++){
            //display may be ordered to view message temporarily
            if (row_timeout_tick[i] != 0){
                if (xTaskGetTickCount() > row_timeout_tick[i]){
                    strncpy(row_buf[i],default_row_buf[i],sizeof(row_buf[0]));
                    row_timeout_tick[i] = 0;
                    elcdeder_print_to_row(i,default_row_buf[i]);
                }
            }
            int cur_buf_strlen = strlen(row_buf[i]);

            // display must be redrawn only if string is longer than its' width
            if (cur_buf_strlen > LCD_LENGTH){
                int print_scroll_offset = cur_buf_strlen - LCD_LENGTH;

                //offset sequence: 0, 0, 0, 1, 2 ... N-1, N, N, N, 0...
                int offset_this_tick = scroll_tick % (print_scroll_offset + 4) - 2;
                if (offset_this_tick < 0) 
                    offset_this_tick = 0;
                if (offset_this_tick > print_scroll_offset) 
                    offset_this_tick = print_scroll_offset;

                elcdeder_print_to_row(i,row_buf[i]+offset_this_tick);
            }
        }
        vTaskDelay(100);
    }    
}