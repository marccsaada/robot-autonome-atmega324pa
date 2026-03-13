#include "debug.hpp"
#include "UART.hpp"

static UART uart;

void debugPrint(const char* message) {
    uart.UART_Transmission(message);
}

void debugPrint(const char* message, uint16_t value) {
    uart.UART_Transmission(message);
    char buffer[6]; // 5 digits + null terminator
    itoa(value, buffer, 10); // Convertit le nombre en chaîne de caractères
    uart.UART_Transmission(buffer);
    uart.UART_Transmission("\n");
}