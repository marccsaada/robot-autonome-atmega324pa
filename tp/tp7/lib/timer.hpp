#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>

/**
 * @file Timer.hpp
 * @brief AVR timer management for PWM and CTC mode.
 */

/**
 * @brief Abstraction of an AVR timer (Timer0/Timer1/Timer2).
 */
class Timer {
public:
    /**
     * @brief Hardware timer identifier.
     */
    enum Id {
        TIMER0,
        TIMER1,
        TIMER2
    };

    /**
     * @brief Supported PWM modes.
     */
    enum PWMMode {
        PHASE_CORRECT,
        FAST
    };

    /**
     * @brief Prescaler values for the timer.
     */
    enum Prescaler {
        NO_PRESCALING,
        PRESCALE_8,
        PRESCALE_64,
        PRESCALE_256,
        PRESCALE_1024
    };

    /**
     * @brief Constructs a timer for the specified identifier.
     *
     * @param id Timer identifier (TIMER0, TIMER1 or TIMER2).
     */
    Timer(Id id);
    ~Timer();

    /**
     * @brief Configures the timer in PWM mode.
     *
     * @param mode PWM mode (phase correct or fast).
     * @param prescaler Prescaler used for the timer.
     */
    void setModePWM(PWMMode mode, Prescaler prescaler);

    /**
     * @brief Configures the timer in CTC mode (Clear Timer on Compare).
     *
     * @param prescaler Prescaler used to enable the timer.
     */
    void setModeCTC(Prescaler prescaler);

    /**
     * @brief Sets the compare value for the OCRxA channel.
     *
     * @param compareValue Compare value.
     */
    void setOCRA(uint16_t compareValue);

    /**
     * @brief Sets the compare value for the OCRxB channel.
     *
     * @param compareValue Compare value.
     */
    void setOCRB(uint16_t compareValue);

    /** @brief Starts the timer (applies the configured prescaler). */
    void startTimer();

    /** @brief Stops the timer (disables the prescaler). */
    void stopTimer();

private:
    Id id_;
    volatile uint8_t* TCCRnA_ = nullptr;
    volatile uint8_t* TCCRnB_ = nullptr;
    volatile Prescaler prescaler_ = Prescaler::NO_PRESCALING;

    /**
     * @brief Applies the prescaler to the timer control register.
     *
     * @param prescaler Prescaler to apply.
     */
    void setPrescaler_(Prescaler prescaler);
};