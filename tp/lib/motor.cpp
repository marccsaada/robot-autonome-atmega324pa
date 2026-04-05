/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Implémentation du contrôleur de moteurs : avance, recul et rotations.
*/

#include "motor.hpp"
#include "debug.hpp"
#include <util/delay.h>

void Motor::delayFunction(uint8_t n) {
    for (uint8_t i = 0; i < n; ++i) {
        _delay_ms(10);
    }
}

Motor::Motor() : leftWheel_(timer_, PD5, Wheel::OCR::A), rightWheel_(timer_, PD4, Wheel::OCR::B) {
    DEBUG_PRINT("Initializing Motor");
    timer_.setModePWM(Timer::PWMMode::PHASE_CORRECT, Timer::Prescaler::PRESCALE_64);
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
    leftWheel_.goBackward(130);
    rightWheel_.goForward(130);
    delayFunction(angle);
    stop();
}

void Motor::spinRight(uint16_t angle) {
    DEBUG_PRINT("Motor::spinRight()");
    leftWheel_.goForward(130);
    rightWheel_.goBackward(130);
    delayFunction(angle+angle/7);
    stop();
}

void Motor::spinRightSpeed(uint8_t speed) {
    DEBUG_PRINT("Motor::spinRightSpeed()");
    leftWheel_.goForward(speed);
    rightWheel_.goBackward(speed);
}

void Motor::spinLeftSpeed(uint8_t speed) {
    DEBUG_PRINT("Motor::spinLeftSpeed()");
    leftWheel_.goBackward(speed);
    rightWheel_.goForward(speed);
}

void Motor::stop() {
    DEBUG_PRINT("Motor::stop()");
    leftWheel_.stop();
    rightWheel_.stop();
}