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
    Timer timer_ = Timer::Id::TIMER2;
    Wheel leftWheel_;
    Wheel rightWheel_;
};
