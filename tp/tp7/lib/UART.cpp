#include "UART.hpp"

UART::UART(){
        // 2400 bauds. Nous vous donnons la valeur des deux

    // premiers registres pour vous éviter des complications.


    UBRR0H = (0 >> 8);
    UBRR0L = 0xCF;

    // permettre la réception et la transmission par le UART0

    UCSR0A = 0x00;

    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

    // Format des trames: 8 bits, 1 stop bits, sans parité

    UCSR0C = (1<<USBS0)|(3<<UCSZ00);

}

void UART::UART_Transmission(uint8_t donnee) {

    while ( !( UCSR0A & (1<<UDRE0)) )
    ;
    /* Put data into buffer, sends the data */
    UDR0 = donnee;

}

void UART::UART_Transmission(const char text[]) {
    uint16_t i = 0;
    while(text[i] != '\0'){ 
        UART_Transmission(text[i++]);
    }
}

void UART::UARTReception(uint8_t donnee) {
    while ( !(UCSR0A & (1<<RXC0)) )
    ;
    donnee = UDR0;
}