#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include "Timer.hpp"

class Wheel {
public:
    enum class OCR{
        A,
        B
    };

    Wheel(Timer &timer, uint8_t pinDirection, OCR ocrType);

    void goForward(uint8_t speedValue);
    void goBackward(uint8_t speedValue);
    void adjustSpeedValue(uint8_t speedValue);
    void stop();
    
private:
    Timer *timer_;
    uint8_t pinDirection_;
    OCR ocrType_;
};