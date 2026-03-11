#include "button.hpp"


Button::Button(Button::Mode mode, Button::Location location) : mode_(mode) , location_(location) {}

bool Button::isPressed() {
    _delay_ms(30);
    switch (location_) {
        case Button::MOTHERBOARD: return  (PIND & (1 << PD2));
        case Button::PD3_BUTTON:  return !(PIND & (1 << PD3));
        case Button::PB2_BUTTON:  return !(PINB & (1 << PB2));
    }
}

void Button::init() {
    EICRA = (EICRA & ~((1 << ISC01) | (1 << ISC00))) | mode_;

    switch (location_) {
        case Button::MOTHERBOARD:
            DDRD &= ~(1 << PD2);
            EIMSK |= (1 << INT0);
            break;
        case Button::PD3_BUTTON:
            DDRD &= ~(1 << PD3);
            EIMSK |= (1 << INT1);
            break;
        case Button::PB2_BUTTON:
            DDRB &= ~(1 << PB2);
            EIMSK |= (1 << INT2);
            break;
    }
    sei();
}

void Button::enableInterupt() {
    EIMSK |= (1 << INT0);
}

void Button::disableInterupt() {
    EIMSK &= ~(1 << INT0);
}