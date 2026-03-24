/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Contrôle individuel d'une roue via un signal PWM et un pin de direction.
*/

#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include "timer.hpp"

/**
 * @file Wheel.hpp
 * @brief Management of a motorized wheel via PWM and direction control.
 */

/**
 * @brief Controls an individual wheel (left or right).
 */
class Wheel {
public:
    /**
     * @brief Selects the OCR channel used for PWM.
     */
    enum class OCR{
        A,
        B
    };

    /**
     * @brief Constructs a wheel by configuring the direction and PWM channel.
     *
     * @param timer Reference to the Timer that generates the PWM signal.
     * @param pinDirection Pin used to define the rotation direction.
     * @param ocrType OCR channel (A or B) used for PWM.
     */
    Wheel(Timer &timer, uint8_t pinDirection, OCR ocrType);
    /** @brief Stops the wheel and releases resources. */
    ~Wheel();

    /**
     * @brief Drives the wheel forward.
     *
     * @param speedValue PWM value (0-255).
     */
    void goForward(uint8_t speedValue);
    /**
     * @brief Drives the wheel backward.
     *
     * @param speedValue PWM value (0-255).
     */
    void goBackward(uint8_t speedValue);

    /**
     * @brief Adjusts the PWM value without changing the direction.
     *
     * @param speedValue PWM value (0-255).
     */
    void adjustSpeedValue(uint8_t speedValue);

    /** @brief Stops the wheel (PWM set to 0). */
    void stop();
    
private:
    Timer *timer_;
    uint8_t pinDirection_;
    OCR ocrType_;
};