#include "debug.hpp"
#include "UART.hpp"

static UART uart;

void debugPrint(const char* message) {
    uart.UART_Transmission(message);
}

void debugPrint(const char* message, const char* value) {
    uart.UART_Transmission(message);
    uart.UART_Transmission(value);
    uart.UART_Transmission("\n");
}

void debugPrint(const char* message, uint8_t value) {
    uart.UART_Transmission(message);
    uart.UART_Transmission('0' + value); // Conversion ASCII
    uart.UART_Transmission("\n");
}