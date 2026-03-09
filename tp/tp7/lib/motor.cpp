#include "motor.hpp"

// Motor::Motor() : leftWheel_(timer_, PD4, Wheel::OCR::A), rightWheel_(timer_, PD5, Wheel::OCR::B), timer_(Timer::Id::TIMER2) {
//     timer_.setModePWM(Timer::PWMMode::PHASE_CORRECT, Timer::Prescaler::PRESCALE_8);
// }
Motor::Motor() : leftWheel_(timer_, PD4, Wheel::OCR::A), rightWheel_(timer_, PD5, Wheel::OCR::B) {
    timer_.setModePWM(Timer::PWMMode::PHASE_CORRECT, Timer::Prescaler::PRESCALE_8);
}

void Motor::goForward(uint8_t speedValue1, uint8_t speedValue2) {
    leftWheel_.goForward(speedValue1);
    rightWheel_.goForward(speedValue2);
}

void Motor::goBackward(uint8_t speedValue1, uint8_t speedValue2) {
    leftWheel_.goBackward(speedValue1);
    rightWheel_.goBackward(speedValue2);
}

void Motor::stop() {
    leftWheel_.stop();
    rightWheel_.stop();
}

