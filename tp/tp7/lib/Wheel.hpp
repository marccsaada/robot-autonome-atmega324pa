#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include "Timer.hpp"

/**
 * @file Wheel.hpp
 * @brief Gestion d'une roue motorisée via PWM et direction.
 */

/**
 * @brief Contrôle une roue individuelle (gauche ou droite).
 */
class Wheel {
public:
    /**
     * @brief Sélectionne le canal OCR utilisé pour le PWM.
     */
    enum class OCR{
        A,
        B
    };

    /**
     * @brief Construit une roue en configurant la direction et le canal PWM.
     *
     * @param timer Référence au Timer qui génère le signal PWM.
     * @param pinDirection Broche utilisée pour définir le sens de rotation.
     * @param ocrType Canal OCR (A ou B) utilisé pour le PWM.
     */
    Wheel(Timer &timer, uint8_t pinDirection, OCR ocrType);

    /**
     * @brief Fait avancer la roue.
     *
     * @param speedValue Valeur PWM (0‑255).
     */
    void goForward(uint8_t speedValue);
    /**
     * @brief Fait reculer la roue.
     *
     * @param speedValue Valeur PWM (0‑255).
     */
    void goBackward(uint8_t speedValue);

    /**
     * @brief Ajuste la valeur PWM sans changer la direction.
     *
     * @param speedValue Valeur PWM (0‑255).
     */
    void adjustSpeedValue(uint8_t speedValue);

    /** @brief Arrête la roue (PWM à 0). */
    void stop();
    
private:
    Timer *timer_;
    uint8_t pinDirection_;
    OCR ocrType_;
};