#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>


class Timer {
public:
    enum class Id { 
        TIMER0, 
        TIMER1, 
        TIMER2          // Specifier quon peut pas utiliser TIMER2 si on utiliser PWM DANS LE RAPPORT
    };

    enum class PWMMode { 
        PHASE_CORRECT,
        FAST
    };

    enum class Prescaler {
        NO_PRESCALING,
        PRESCALE_8,
        PRESCALE_64,
        PRESCALE_256,
        PRESCALE_1024
    };

    Timer(Id id);

    void setModePWM(PWMMode mode, Prescaler prescaler);
    void setModeCTC(Prescaler prescaler);

    void setOCRA(uint16_t compareValue);
    void setOCRB(uint16_t compareValue);

    void startTimer();
    void stopTimer();

private:
    Id id_;
    volatile uint8_t* TCCRnA_ = nullptr;
    volatile uint8_t* TCCRnB_ = nullptr;
    volatile Prescaler prescaler_ = Prescaler::NO_PRESCALING;

    void setPrescaler_(Prescaler prescaler);
};