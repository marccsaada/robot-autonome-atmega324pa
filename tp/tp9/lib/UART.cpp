#include "UART.hpp"

UART::UART(){
    UBRR0H = (0 >> 8);
    UBRR0L = 0xCF;
    UCSR0A = 0x00;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void UART::UART_Transmission(uint8_t donnee) {
    while ( !( UCSR0A & (1<<UDRE0)) )
    ;
    UDR0 = donnee;
}

void UART::UART_Transmission(const char text[]) {
    uint16_t i = 0;
    while(text[i] != '\0'){ 
        UART_Transmission(text[i++]);
    }
}

uint8_t UART::UART_Reception(void) {
    while ( !(UCSR0A & (1<<RXC0)) )
    ;
    return UDR0;
}