#pragma once
#include "UART.hpp"
#include <stdlib.h>

void debugPrint(const char* message);
void debugPrint(const char* message, const char* value);
void debugPrint(const char* message, uint8_t value);

#ifdef DEBUG
    #define DEBUG_PRINT(...) debugPrint(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...) do {} while(0)
#endif
