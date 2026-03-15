/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah
Travail : TP2
Section # : 05
Équipe # : 111
Correcteur : Meriam Ben Rabia

Description du programme : Ce programme lit la valeur d'un capteur analogique connecté à l'entrée ADC0 du microcontrôleur ATmega324PA et contrôle une LED bicolore (rouge et verte) en fonction de la valeur lue. La LED s'allume en vert si la valeur est inférieure ou égale à 158, en rouge si elle est supérieure ou égale à 240, et en ambre (alternance rapide entre rouge et vert) pour les valeurs intermédiaires.
* ========== I/O IDENTIFICATION (CONNEXIONS SUR LE ROBOT) =============
* 
*
* ┌───────────────────────────────────────────────────────────────────┐
* │                    ATmega324PA - Pinout                           │
* ├───────────────────────────────────────────────────────────────────┤
* │                                                                   │
* │   PORTA            PORTB           PORTC          PORTD           │
* │  ┌───────┐        ┌──────┐         ┌──────┐       ┌──────┐        │   
* │  │ PA0 ●─│→ADC0   │PB0 ●─│→LED+    │PC0 ○ │       │PD0 ●─│→RXD    │
* │  │ PA1 ○ │        │PB1 ●─│→LED-    │PC1 ○ │       │PD1 ●─│→TXD    │
* │  │ PA2 ○ │        │PB2 ○ │         │PC2 ○ │       │PD2 ○ │        │
* │  │ PA3 ○ │        │PB3 ○ │         │PC3 ○ │       │PD3 ● │        │
* │  │ PA4 ○ │        │PB4 ●─│→PG+     │PC4 ○ │       │PD4 ○ │        │
* │  │ PA5 ○ │        │PB5 ●─│→PG+     │PC5 ○ │       │PD5 ○ │        │
* │  │ PA6 ○ │        │PB6 ●─│→PG+     │PC6 ○ │       │PD6 ○ │        │
* │  │ PA7 ○ │        │PB7 ●─│→PG+     │PC7 ○ │       │PD7 ○ │        │
* │  └───────┘        └──────┘         └──────┘       └──────┘        │
* │                                                                   │
* │  ● = Utilisé          ○ = Non utilisé                             │
* └───────────────────────────────────────────────────────────────────┘
* LEGENDE :
* PG : SPI (Programmeur)
* ADC0 : Entrée analogique 0 du convertisseur analogue-numérique 10 bits
* RXD/TXD : UART (debug)
* SCL/SDA : EEPROM externe
* B-INT0 : bouton
* LED+ : Green led
* LED- : Red led
*
* =====================================================================
*/

#include "button.hpp"
#include "debug.hpp"

Button::Button(Button::Mode mode, Button::Location location) : mode_(mode) , location_(location) {
    DEBUG_PRINT("Button constructor called");
}
Button::Button::~Button() {
    disableInterupt();
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
    DEBUG_PRINT("Enabling button interrupt");
    EIMSK |= (1 << INT0);
}

void Button::disableInterupt() {
    DEBUG_PRINT("Disabling button interrupt");
    EIMSK &= ~(1 << INT0);
}