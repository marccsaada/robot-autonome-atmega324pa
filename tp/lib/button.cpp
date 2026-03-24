/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9 - Gestion du bouton
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Implémentation de la gestion du bouton avec configuration des interruptions.
*/

#include "button.hpp"
#include "debug.hpp"

Button::Button(Button::Mode mode, Button::Location location) : mode_(mode) , location_(location) {
    DEBUG_PRINT("Button constructor called");
}

Button::~Button() {
    disableInterupt();
    switch (location_) {
        case Button::MOTHERBOARD: EICRA &= ~((1 << ISC01) | (1 << ISC00)); break;
        case Button::PD3_BUTTON:  EICRA &= ~((1 << ISC11) | (1 << ISC10)); break;
        case Button::PB2_BUTTON:  EICRA &= ~((1 << ISC21) | (1 << ISC20)); break;
    }
}

bool Button::isPressed() {
    _delay_ms(30);
    switch (location_) {
        case Button::MOTHERBOARD: DEBUG_PRINT("MOTHERBOARD button is pressed"); return  (PIND & (1 << PD2));
        case Button::PD3_BUTTON:  DEBUG_PRINT("PD3 button is pressed");         return !(PIND & (1 << PD3));
        case Button::PB2_BUTTON:  DEBUG_PRINT("PB2 button is pressed");         return !(PINB & (1 << PB2));
        default: return false;
    }
}

void Button::init() {
    DEBUG_PRINT("Initializing button");

    switch (location_) {
        case Button::MOTHERBOARD:
            DDRD &= ~(1 << PD2);
            EIMSK |= (1 << INT0);
            EICRA = (EICRA & ~((1 << ISC01) | (1 << ISC00))) | (mode_ << 0);
            break;
        case Button::PD3_BUTTON:
            DDRD &= ~(1 << PD3);
            EIMSK |= (1 << INT1);
            EICRA = (EICRA & ~((1 << ISC11) | (1 << ISC10))) | (mode_ << 2);
            break;
        case Button::PB2_BUTTON:
            DDRB &= ~(1 << PB2);
            EIMSK |= (1 << INT2);
            EICRA = (EICRA & ~((1 << ISC21) | (1 << ISC20))) | (mode_ << 4);
            break;
    }
    sei();
}

void Button::enableInterupt() {
    DEBUG_PRINT("Enabling button interrupt");
    switch (location_) {
        case Button::MOTHERBOARD: EIMSK |= (1 << INT0); break;
        case Button::PD3_BUTTON:  EIMSK |= (1 << INT1); break;
        case Button::PB2_BUTTON:  EIMSK |= (1 << INT2); break;
    }
}

void Button::disableInterupt() {
    DEBUG_PRINT("Disabling button interrupt");
        switch (location_) {
        case Button::MOTHERBOARD: EIMSK &= ~(1 << INT0); break;
        case Button::PD3_BUTTON:  EIMSK &= ~(1 << INT1); break;
        case Button::PB2_BUTTON:  EIMSK &= ~(1 << INT2); break;
    }
}