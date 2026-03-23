/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9 - Programme 1 : écriture du bytecode en mémoire externe via RS-232
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Ce programme reçoit le bytecode compilé par progmem via RS-232
              (serieViaUSB -e -f programme.bin) et l'écrit dans la mémoire
              EEPROM externe (24CXXX) via le bus I2C (TWI).

Procédure :
  1. Flasher CE programme (ecriture/) dans le microcontrôleur.
  2. Brancher le robot — la LED rouge s'allume, le robot attend les données.
  3. Envoyer le bytecode : serieViaUSB -e -f programme.bin
  4. Attendre que la LED passe au vert (écriture terminée).
  5. Reflasher le programme de lecture/interprétation (exec/).

Format du fichier binaire reçu (identique au format EEPROM) :
  Octets 0-1 : taille totale du fichier en octets (big-endian)
  Octets 2+  : paires (instruction, opérande) consécutives
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "memoire_24.h"
// #include "LED.hpp"
// #include "UART.hpp"
#include <libstatique.hpp>

int main() {
    // LED bicolore : PA0 = rouge, PA1 = vert
    LED led(PORTA, PA0, PA1);
    UART uart;
    Memoire24CXXX memoire;

    // Signal d'attente : LED rouge pendant la réception
    led.red();

    // Buffer RAM : on reçoit TOUT d'abord, puis on écrit en EEPROM.
    // (Le buffer UART est 1 octet; l'EEPROM prend ~15ms/octet → on perdrait
    //  des octets si on intercalait réception et écriture.)
    const uint16_t MAX_TAILLE = 256;
    uint8_t buffer[MAX_TAILLE];

    // Lire les 2 premiers octets : taille totale (big-endian)
    buffer[0] = uart.UART_Reception();
    buffer[1] = uart.UART_Reception();
    uint16_t taille = ((uint16_t)buffer[0] << 8) | buffer[1];

    // Recevoir le reste dans le buffer (sans délai EEPROM)
    for (uint16_t i = 2; i < taille && i < MAX_TAILLE; i++)
        buffer[i] = uart.UART_Reception();

    // Écrire le buffer complet en EEPROM
    for (uint16_t i = 0; i < taille && i < MAX_TAILLE; i++) {
        memoire.ecriture(i, buffer[i]);
        _delay_ms(5);
    }

    // Signal de fin : LED verte — l'EEPROM est prête
    led.green();

    while (true) {}
}
