#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include "Wheel.hpp"

class Motor {
public:
    Motor();
    void goForward(uint8_t speedValue1, uint8_t speedValue2);
    void goBackward(uint8_t speedValue1, uint8_t speedValue2);
    void stop();
    //ajoute peut etre fct spin

private:
    Wheel leftWheel_;
    Wheel rightWheel_;
    Timer timer_;
};
