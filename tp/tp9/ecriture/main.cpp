/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9 - Programme 1 : écriture du bytecode en mémoire externe via RS-232
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Ce programme reçoit le bytecode compilé par progmem via RS-232
              (serieViaUSB -e -f programme.bin) et l'écrit dans la mémoire
              EEPROM externe (24CXXX) via le bus I2C (TWI).
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "memoire_24.h"

#include <libstatique.hpp>

int main() {
    
    LED led(PORTA, PA0, PA1);
    UART uart;
    Memoire24CXXX memoire;

    
    led.red();

    
    const uint16_t MAX_TAILLE = 256;
    uint8_t buffer[MAX_TAILLE];

    buffer[0] = uart.UART_Reception();
    buffer[1] = uart.UART_Reception();
    uint16_t taille = ((uint16_t)buffer[0] << 8) | buffer[1];

    for (uint16_t i = 2; i < taille && i < MAX_TAILLE; i++)
        buffer[i] = uart.UART_Reception();

    for (uint16_t i = 0; i < taille && i < MAX_TAILLE; i++) {
        memoire.ecriture(i, buffer[i]);
        _delay_ms(5);
    }

    led.green();

    while (true) {}
}
