#pragma once
#include "UART.hpp"
#include <stdlib.h>

/**
 * @file debug.hpp
 * @brief Utilitaires de débogage via UART.
 */

/**
 * @brief Envoie un message de débogage via UART.
 *
 * @param message Chaîne de caractères à transmettre.
 */
void debugPrint(const char* message);

/**
 * @brief Envoie un message de débogage suivi d'une valeur entière via UART.
 *
 * @param message Chaîne de caractères à transmettre.
 * @param value Valeur numérique à afficher.
 */
void debugPrint(const char* message, uint16_t value);

#ifdef DEBUG
    #define DEBUG_PRINT(...) debugPrint(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...) do {} while(0)
#endif
