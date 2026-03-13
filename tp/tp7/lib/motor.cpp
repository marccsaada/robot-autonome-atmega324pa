#include "motor.hpp"
#include "debug.hpp"

Motor::Motor() : leftWheel_(timer_, PD4, Wheel::OCR::A), rightWheel_(timer_, PD5, Wheel::OCR::B) {
    DEBUG_PRINT("Initializing Motor");
    timer_.setModePWM(Timer::PWMMode::PHASE_CORRECT, Timer::Prescaler::PRESCALE_8);
    timer_.startTimer();
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

void Motor::stop() {
    DEBUG_PRINT("Motor::stop()");
    leftWheel_.stop();
    rightWheel_.stop();
}