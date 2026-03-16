#include "debug.hpp"
#include "UART.hpp"

static UART uart;

void debugPrint(const char* message) {
    uart.UART_Transmission(message);
    uart.UART_Transmission("\n");
}

void debugPrint(uint16_t value) {
    char buffer[6]; 
    itoa(value, buffer, 10);
    uart.UART_Transmission(buffer);
    uart.UART_Transmission("\n");
}

void debugPrint(const char* message, uint16_t value) {
    uart.UART_Transmission(message);
    debugPrint(value);
}