#include <Wire.h>
#include <LiquidCrystal.h>

#include <stdio.h>
#include <stdarg.h>

#include "elceder.h"
#include "pinDefs.h"

#define CONTRAST 110

LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_4, PIN_5, PIN_6, PIN_7, 23, POSITIVE);

QueueHandle_t elceder_msg_queue;

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

    char row_buf[LCD_ROW_COUNT][32] = {0};

    while(1){
        BaseType_t queue_succ = 
            xQueueReceive( elceder_msg_queue,
                &msg, 0 );                        

        if (queue_succ) {
            // memset(row_buf[msg.row],0x00,sizeof(row_buf[0]));
            strncpy(row_buf[msg.row],msg.str,sizeof(row_buf[0]));
            elcdeder_print_to_row(msg.row,msg.str);
        }
        int scroll_tick = xTaskGetTickCount()/666;

        for (int i = 0; i<LCD_ROW_COUNT; i++){
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