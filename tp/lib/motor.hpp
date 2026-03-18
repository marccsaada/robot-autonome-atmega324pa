#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include "wheel.hpp"

/**
 * @file motor.hpp
 * @brief Control of both motors via PWM.
 */

/**
 * @brief Represents a motor controller for the left and right wheels.
 */
class Motor {
public:
    /** @brief Constructs a motor controller. */
    Motor();
    /** @brief Stops both motors and releases resources. */
    ~Motor();

    /**
     * @brief Drives the robot forward by controlling each wheel.
     *
     * @param speedValue1 PWM applied to the left wheel (0-255).
     * @param speedValue2 PWM applied to the right wheel (0-255).
     */
    void goForward(uint8_t speedValue1, uint8_t speedValue2);

    /**
     * @brief Drives the robot backward by controlling each wheel.
     *
     * @param speedValue1 PWM applied to the left wheel (0-255).
     * @param speedValue2 PWM applied to the right wheel (0-255).
     */
    void goBackward(uint8_t speedValue1, uint8_t speedValue2);

    void spinLeft(uint16_t angle);

    void spinRight(uint16_t angle);

    /** @brief Stops both motors. */
    void stop();

private:
    Timer timer_ = Timer::Id::TIMER2;
    Wheel leftWheel_;
    Wheel rightWheel_;
};