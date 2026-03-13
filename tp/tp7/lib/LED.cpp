
#define F_CPU 8000000UL
#include "LED.hpp"

const uint8_t RED_DELAY = 3;
const uint8_t GREEN_DELAY = 3;

LED::LED(volatile uint8_t &port, uint8_t pinIn, uint8_t pinOut)
    : port_(&port), pinIn_(pinIn), pinOut_(pinOut) {

    setDDR();
    
}

void LED::off() { 
    *port_ &= ~(1 << pinIn_);
    *port_ &= ~(1 << pinOut_);
}

void LED::red() {
    *port_ |= (1 << pinIn_);
    *port_ &= ~(1 << pinOut_);
}

void LED::green() {
    *port_ &= ~(1 << pinIn_);
    *port_ |= (1 << pinOut_);
}

void LED::amber() {
    LED::red();
    _delay_ms(RED_DELAY);
    LED::green();
    _delay_ms(GREEN_DELAY);
}
//c'est pas 
void LED::setDDR() {

    if (port_ == &PORTA) ddr_ = &DDRA;
else if (port_ == &PORTB) ddr_ = &DDRB;
else if (port_ == &PORTC) ddr_ = &DDRC;
else if (port_ == &PORTD) ddr_ = &DDRD;
*ddr_ |= (1 << pinIn_) | (1 << pinOut_);
}
