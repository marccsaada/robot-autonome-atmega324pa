#include "motor.hpp"
#include "debug.hpp"
#include <util/delay.h>

void tempDelayFunc(uint8_t n) {
    for (uint8_t i = 0; i < n; ++i) {
        _delay_ms(10); // Delay of 10 ms per unit of n
    }
}

Motor::Motor() : leftWheel_(timer_, PD5, Wheel::OCR::B), rightWheel_(timer_, PD4, Wheel::OCR::A) {
    DEBUG_PRINT("Initializing Motor");
    timer_.setModePWM(Timer::PWMMode::PHASE_CORRECT, Timer::Prescaler::PRESCALE_8);
    timer_.startTimer();
}

Motor::~Motor() {
    DEBUG_PRINT("Motor destructor called");
    stop();
    timer_.stopTimer();
}

void Motor::goForward(uint8_t speedValue1, uint8_t speedValue2) {
    DEBUG_PRINT("Motor::goForward()");
    leftWheel_.goForward(speedValue1);
    rightWheel_.goForward(speedValue2);
}

void Motor::goBackward(uint8_t speedValue1, uint8_t speedValue2) {
    DEBUG_PRINT("Motor::goBackward()");
    leftWheel_.goBackward(speedValue1);
    rightWheel_.goBackward(speedValue2);
}

void Motor::spinLeft(uint16_t angle) {
    DEBUG_PRINT("Motor::spinLeft()");
    leftWheel_.goBackward(145);
    rightWheel_.goForward(145);
    tempDelayFunc(angle); // Adjust delay based on angle (example: 10ms per degree)
    stop();
}

void Motor::spinRight(uint16_t angle) {
    DEBUG_PRINT("Motor::spinRight()");
    leftWheel_.goForward(145); // Half speed forward
    rightWheel_.goBackward(145); // Half speed backward
    tempDelayFunc(angle); // Adjust delay based on angle (example: 10ms per degree)
    stop();
}

void Motor::stop() {
    DEBUG_PRINT("Motor::stop()");
    leftWheel_.stop();
    rightWheel_.stop();
}