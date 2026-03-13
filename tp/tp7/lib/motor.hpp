#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include "Wheel.hpp"

/**
 * @file motor.hpp
 * @brief Contrôle des deux moteurs via PWM.
 */

/**
 * @brief Représente un contrôleur de moteurs pour les roues gauche et droite.
 */
class Motor {
public:
    /** @brief Construit un contrôleur de moteurs. */
    Motor();

    /**
     * @brief Fait avancer le robot en contrôlant chaque roue.
     *
     * @param speedValue1 PWM appliqué à la roue gauche (0‑255).
     * @param speedValue2 PWM appliqué à la roue droite (0‑255).
     */
    void goForward(uint8_t speedValue1, uint8_t speedValue2);

    /**
     * @brief Fait reculer le robot en contrôlant chaque roue.
     *
     * @param speedValue1 PWM appliqué à la roue gauche (0‑255).
     * @param speedValue2 PWM appliqué à la roue droite (0‑255).
     */
    void goBackward(uint8_t speedValue1, uint8_t speedValue2);

    /** @brief Arrête les deux moteurs. */
    void stop();
    //ajoute peut etre fct spin

private:
    Timer timer_ = Timer::Id::TIMER2;
    Wheel leftWheel_;
    Wheel rightWheel_;
};