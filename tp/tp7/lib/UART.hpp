#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>

/**
 * @file UART.hpp
 * @brief UART serial communication interface.
 */

/**
 * @brief Provides UART transmission and reception functions.
 */
class UART {
public:
    /** @brief Initializes the UART peripheral (2400 baud). */
    UART();
    /**
     * @brief Transmits a byte via UART.
     *
     * @param donnee Byte to transmit.
     */
    void UART_Transmission(uint8_t donnee);
    /**
     * @brief Transmits a null-terminated string via UART.
     *
     * @param text Null-terminated string.
     */
    void UART_Transmission(const char text[]);

    /**
     * @brief Receives a byte via UART (blocking).
     *
     * @return Received byte.
     */
    uint8_t UARTReception(void);
};