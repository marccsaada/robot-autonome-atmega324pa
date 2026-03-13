#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>

/**
 * @file UART.hpp
 * @brief Interface de communication série UART.
 */

/**
 * @brief Fournit des fonctions de transmission et de réception UART.
 */
class UART {
public:
    /** @brief Initialise le périphérique UART (2400 bauds). */
    UART();
    /**
     * @brief Transmet un octet via UART.
     *
     * @param donnee Octet à transmettre.
     */
    void UART_Transmission(uint8_t donnee);
    /**
     * @brief Transmet une chaîne de caractères via UART.
     *
     * @param text Chaîne terminée par \0.
     */
    void UART_Transmission(const char text[]);

    /**
     * @brief Reçoit un octet via UART (bloquant).
     *
     * @return Octet reçu.
     */
    uint8_t UARTReception(void);
};