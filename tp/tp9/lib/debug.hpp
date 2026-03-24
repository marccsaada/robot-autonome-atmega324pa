/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Macros et fonctions de débogage via UART, compilées uniquement si DEBUG est défini.
*/

#pragma once
#include "UART.hpp"
#include <stdlib.h>

/**
 * @file debug.hpp
 * @brief Debugging utilities via UART.
 */

/**
 * @brief Sends a debug message via UART.
 *
 * @param message String to transmit.
 */
void debugPrint(const char* message);

/**
 * @brief Sends a numeric value via UART.
 *
 * @param value Numeric value to display.
 */
void debugPrint(uint16_t value);

/**
 * @brief Sends a debug message followed by an integer value via UART.
 *
 * @param message String to transmit.
 * @param value Numeric value to display.
 */
void debugPrint(const char* message, uint16_t value);

#ifdef DEBUG
    #define DEBUG_PRINT(...) debugPrint(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...) do {} while(0)
#endif
