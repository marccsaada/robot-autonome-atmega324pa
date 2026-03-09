#include "button.hpp"


Button::Button(Button::Mode mode) : mode_(mode) {}

bool Button::isPressed() {
    if (PIND & (1 << PD2)) {
        _delay_ms(30);
        if (PIND & (1 << PD2)) {
            return true;
        }
    }

    return false;
}

void Button::init() {
    DDRD &= ~(1 << PD2);
    EICRA = (EICRA & ~((1 << ISC01) | (1 << ISC00))) | mode_;
    EIMSK |= (1 << INT0);
    sei();
}

void Button::enableInterupt() {
    EIMSK |= (1 << INT0);
}

void Button::disableInterupt() {
    EIMSK &= ~(1 << INT0);
}