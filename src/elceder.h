

typedef struct {
    char str[17];
    uint8_t row;
} elceder_msg_t;

void elceder_task(void* params);
void elceder_queue_text(elceder_msg_t* msg);
void elceder_fill_row(int row, const char* fmt, ...);