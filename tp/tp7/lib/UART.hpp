#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>

class UART {
public:
    UART();
    void UART_Transmission(uint8_t donnee);
    void UART_Transmission(const char text[]);
    uint8_t UARTReception(void);
};