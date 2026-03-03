#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>


class Timer {
public:
    enum class Id { 
        TIMER0, 
        TIMER1, 
        TIMER2 
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

    // Fonction de démarrage du timer avec deux valeurs de comparaisons (deuxieme optionelle)
    void setOCRA(uint16_t compareValue);
    void setOCRB(uint16_t compareValue);

    void stopTimer();

private:
    Id id_;
    uint8_t TCCRnA_;
    uint8_t TCCRnB_;
    uint16_t TCNTn_;

    void setPrescaler_(Prescaler prescaler);
};
