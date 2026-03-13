#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>

/**
 * @file Timer.hpp
 * @brief Gestion des timers AVR pour PWM et mode CTC.
 */

/**
 * @brief Abstraction d'un timer AVR (Timer0/Timer1/Timer2).
 */
class Timer {
public:
    /**
     * @brief Identifiant du timer matériel.
     */
    enum class Id { 
        TIMER0, 
        TIMER1, 
        TIMER2          /**< Timer2 utilisé pour PWM (ne pas utiliser si rapport). */
    };

    /**
     * @brief Modes PWM supportés.
     */
    enum class PWMMode { 
        PHASE_CORRECT,
        FAST
    };

    /**
     * @brief Valeurs de prescaler pour le timer.
     */
    enum class Prescaler {
        NO_PRESCALING,
        PRESCALE_8,
        PRESCALE_64,
        PRESCALE_256,
        PRESCALE_1024
    };

    /**
     * @brief Construit un timer pour l'identifiant spécifié.
     *
     * @param id Identifiant du timer (TIMER0, TIMER1 ou TIMER2).
     */
    Timer(Id id);

    /**
     * @brief Configure le timer en mode PWM.
     *
     * @param mode Mode PWM (phase correct ou fast).
     * @param prescaler Prescaler utilisé pour le timer.
     */
    void setModePWM(PWMMode mode, Prescaler prescaler);

    /**
     * @brief Configure le timer en mode CTC (Clear Timer on Compare).
     *
     * @param prescaler Prescaler utilisé pour activer le timer.
     */
    void setModeCTC(Prescaler prescaler);

    /**
     * @brief Définit la valeur de comparaison pour le canal OCRxA.
     *
     * @param compareValue Valeur de comparaison.
     */
    void setOCRA(uint16_t compareValue);

    /**
     * @brief Définit la valeur de comparaison pour le canal OCRxB.
     *
     * @param compareValue Valeur de comparaison.
     */
    void setOCRB(uint16_t compareValue);

    /** @brief Démarre le timer (applique le prescaler configuré). */
    void startTimer();

    /** @brief Arrête le timer (désactive le prescaler). */
    void stopTimer();

private:
    Id id_;
    volatile uint8_t* TCCRnA_ = nullptr;
    volatile uint8_t* TCCRnB_ = nullptr;
    volatile Prescaler prescaler_ = Prescaler::NO_PRESCALING;

    /**
     * @brief Applique le prescaler sur le registre de contrôle du timer.
     *
     * @param prescaler Prescaler à appliquer.
     */
    void setPrescaler_(Prescaler prescaler);
};