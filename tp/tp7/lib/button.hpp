#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

class Button
{
public:
    enum Mode {
        RISING = (1 << ISC01) | (1 << ISC00),
        FALLING = (1 << ISC01),
        ANY = (1 << ISC00)
    };

    Button(Mode mode);
    bool isPressed();
    void init();
    void enableInterupt();
    void disableInterupt();

private:
    Mode mode_;
};