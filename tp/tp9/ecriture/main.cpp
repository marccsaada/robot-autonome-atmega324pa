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

    uint8_t msb = uart.UART_Reception();
    uint8_t lsb = uart.UART_Reception();
    uint16_t taille = ((uint16_t)msb << 8) | lsb;

    for (uint16_t i = 2; i < taille && i < taille; i++) {
        uint8_t data = uart.UART_Reception();
        memoire.ecriture(i, data);
        _delay_ms(5);
    }

    led.green();

    while (true) {}
}
