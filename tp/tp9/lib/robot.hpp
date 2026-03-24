/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9 - Abstraction du robot
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Regroupe les sous-systèmes moteurs, LED et son en une interface unifiée
              utilisée par l'interpréteur.
*/

#pragma once
#include "motor.hpp"
#include "LED.hpp"
#include "sound.hpp"
#include <avr/io.h>
#include <util/delay.h>

/**
 * @file robot.hpp
 * @brief High-level robot abstraction grouping motor, LED, and sound control.
 */

/**
 * @brief Unified robot interface: movement, LED signaling, and audio.
 */
class Robot {
public:
    Robot() : motor_(), led_(PORTA, PA0, PA1) {};

    /**
     * @brief Drives the robot forward.
     * @param speed PWM value (0–255).
     */
    void goForward(uint8_t speed);

    /**
     * @brief Drives the robot backward.
     * @param speed PWM value (0–255).
     */
    void goBackward(uint8_t speed);

    /**
     * @brief Turns the robot right by the given angle.
     * @param angle Angle in degrees.
     */
    void turnRight(uint16_t angle);

    /**
     * @brief Turns the robot left by the given angle.
     * @param angle Angle in degrees.
     */
    void turnLeft(uint16_t angle);

    /** @brief Stops both motors. */
    void stopMotor();

    /** @brief Turns on the green LED. */
    void turnOnGreen();
    /** @brief Turns on the red LED. */
    void turnOnRed();
    /** @brief Turns off the LED. */
    void turnOffLED();

    /** @brief Stops the buzzer. */
    void stopSound();

    /**
     * @brief Plays a MIDI note on the buzzer.
     * @param note MIDI note number (45–81 inclusive).
     */
    void playSound(uint8_t note);

    /**
     * @brief Waits for a multiple of 25 ms.
     * @param multiplicator Number of 25 ms slots.
     */
    void wait(uint16_t multiplicator);

private:
    Motor motor_;
    LED led_;
    Sound sound_;
};
