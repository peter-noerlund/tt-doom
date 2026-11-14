#pragma once

#include <stdint.h>

uint32_t* allocate_screen_buffer(void);
void set_active_screen_buffer(uint32_t* screen_buffer);
