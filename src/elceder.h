#pragma once

#define ELCEDER_NO_TIMEOUT

typedef struct {
    char str[32];
    uint8_t row;
    uint32_t message_timeout;
} elceder_msg_t;

void elceder_task(void* params);
void elceder_queue_text(elceder_msg_t* msg);
void elceder_fill_row(int row, int message_timeout_ms, const char* fmt, ...);