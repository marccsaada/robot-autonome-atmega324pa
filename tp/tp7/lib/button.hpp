#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

class Button
{
public:
    // Selon si on utilise le bouton externe ou sur la carte mere, le fonctionnement est different.
    enum Location {
        MOTHERBOARD,
        PD3_BUTTON,
        PB2_BUTTON
    };

    enum Mode {
        RISING = (1 << ISC01) | (1 << ISC00),
        FALLING = (1 << ISC01),
        ANY = (1 << ISC00)
    };

    Button(Mode mode, Location location_ = MOTHERBOARD);
    bool isPressed();
    void init();
    void enableInterupt();
    void disableInterupt();

private:
    Mode mode_;
    Location location_;
};