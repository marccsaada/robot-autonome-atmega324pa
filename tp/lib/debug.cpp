/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Implémentation des fonctions de débogage via UART.
*/

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