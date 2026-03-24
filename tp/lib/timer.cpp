/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Implémentation de l'abstraction des timers AVR en modes PWM et CTC.
*/

#define F_CPU 8000000UL
#include "timer.hpp"
#include "debug.hpp"

Timer::Timer(Id id) : id_(id) {
    DEBUG_PRINT("Initializing Timer");

    switch(id_) {
        case TIMER0:
        DEBUG_PRINT("Selected TIMER0");
            TCCRnA_ = &TCCR0A;
            TCCRnB_ = &TCCR0B;
            break;
        case TIMER1:
        DEBUG_PRINT("Selected TIMER1");
            TCCRnA_ = &TCCR1A;
            TCCRnB_ = &TCCR1B;
            break;
        case TIMER2:
        DEBUG_PRINT("Selected TIMER2");
            TCCRnA_ = &TCCR2A;
            TCCRnB_ = &TCCR2B;
            break;
    }
}

Timer::~Timer() {
    DEBUG_PRINT("Timer destructor called");
    stopTimer();
    *TCCRnA_ = 0;
    *TCCRnB_ = 0;
}

void Timer::setModePWM(PWMMode mode, Prescaler prescaler) {
    DEBUG_PRINT("Setting Timer mode to PWM");
    DEBUG_PRINT("Setting Prescaler : ", static_cast<uint16_t>(prescaler));
    prescaler_ = prescaler;

    switch(mode) {
        case PHASE_CORRECT:
            DEBUG_PRINT("Setting PWM mode : Phase Correct");
            if (id_ == TIMER0 || id_ == TIMER2) {
                *TCCRnA_ |= (1 << WGM00);
                *TCCRnA_ &= ~(1 << WGM01);
                *TCCRnB_ &= ~(1 << WGM02);
            } else if (id_ == TIMER1) {
                *TCCRnA_ |= (1 << WGM10);
                *TCCRnA_ &= ~(1 << WGM11);
                *TCCRnB_ &= ~(1 << WGM12) & ~(1 << WGM13);
            }
            break;
        case FAST:
            DEBUG_PRINT("Setting PWM mode : Fast");
            if (id_ == TIMER0 || id_ == TIMER2) {
                *TCCRnA_ |= (1 << WGM00) | (1 << WGM01);
            } else if (id_ == TIMER1) {
                *TCCRnA_ |= (1 << WGM10) | (1 << WGM11);
                *TCCRnB_ &= ~(1 << WGM12) & ~(1 << WGM13);
            }
            break;
    }

    *TCCRnA_ |= (1 << COM2A1) | (1 << COM2B1);
    *TCCRnA_ &= ~((1 << COM2A0) | (1 << COM2B0));
}

void Timer::setModeCTC(Prescaler prescaler) {
    DEBUG_PRINT("Setting Timer mode to CTC");
    DEBUG_PRINT("Setting Prescaler : ", static_cast<uint16_t>(prescaler));
    prescaler_ = prescaler;

    if (id_ == TIMER0) {
        DEBUG_PRINT("Setting CTC Timer interrupts for TIMER0");
        TIMSK0 = (1 << OCIE0A) | (1 << OCIE0B);
    } else if (id_ == TIMER1) {
        DEBUG_PRINT("Setting CTC Timer interrupts for TIMER1");
        TIMSK1 = (1 << OCIE1A) | (1 << OCIE1B);
    } else if (id_ == TIMER2) {
        DEBUG_PRINT("Setting CTC Timer interrupts for TIMER2");
        TIMSK2 = (1 << OCIE2A) | (1 << OCIE2B);
    }

    switch(id_) {
        case TIMER0:
        case TIMER2:
            *TCCRnA_ &= ~(1 << WGM00);
            *TCCRnA_ |= (1 << WGM01);
            *TCCRnB_ &= ~(1 << WGM02);
            break;
        case TIMER1:
            *TCCRnA_ &= ~(1 << WGM10) & ~(1 << WGM11);
            *TCCRnB_ |= (1 << WGM12);
            *TCCRnB_ &= ~(1 << WGM13);
            break;
    }
}

void Timer::setPrescaler_(Prescaler prescaler) {
    if (id_ == TIMER2) {
        switch(prescaler) {
            case NO_PRESCALING:
                *TCCRnB_ |= (1 << CS00);
                *TCCRnB_ &= ~((1 << CS01) | (1 << CS02));
                break;
            case PRESCALE_8:
                *TCCRnB_ |= (1 << CS01);
                *TCCRnB_ &= ~((1 << CS00) | (1 << CS02));
                break;
            case PRESCALE_64:
                *TCCRnB_ |= (1 << CS02);
                *TCCRnB_ &= ~((1 << CS00) | (1 << CS01));
                break;
            case PRESCALE_256:
                *TCCRnB_ |= (1 << CS02) | (1 << CS01);
                *TCCRnB_ &= ~(1 << CS00);
                break;
            case PRESCALE_1024:
                *TCCRnB_ |= (1 << CS02) | (1 << CS01) | (1 << CS00);
                break;
            }
            return;
    }

    switch(prescaler) {
        case NO_PRESCALING:
            *TCCRnB_ |= (1 << CS00);
            *TCCRnB_ &= ~((1 << CS01) | (1 << CS02));
            break;
        case PRESCALE_8:
            *TCCRnB_ |= (1 << CS01);
            *TCCRnB_ &= ~((1 << CS00) | (1 << CS02));
            break;
        case PRESCALE_64:
            *TCCRnB_ |= (1 << CS01) | (1 << CS00);
            *TCCRnB_ &= ~(1 << CS02);
            break;
        case PRESCALE_256:
            *TCCRnB_ |= (1 << CS02);
            *TCCRnB_ &= ~((1 << CS00) | (1 << CS01));
            break;
        case PRESCALE_1024:
            *TCCRnB_ |= (1 << CS02) | (1 << CS00);
            *TCCRnB_ &= ~(1 << CS01);
            break;
        }
}

void Timer::setOCRA(uint16_t compareValue) {
    DEBUG_PRINT("Setting OCRA with compareValue : ", compareValue);
    switch(id_) {
        case TIMER0: 
            TCNT0 = 0;
            OCR0A = compareValue;
            break;
        case TIMER1:
            TCNT1 = 0;
            OCR1A = compareValue;
            break;
        case TIMER2:
            TCNT2 = 0;
            OCR2A = compareValue;
            break;
    }
}
void Timer::setOCRB(uint16_t compareValue) {
    DEBUG_PRINT("Setting OCRB with compareValue : ", compareValue);
    switch(id_) {
        case TIMER0:
            TCNT0 = 0;
            OCR0B = compareValue;
            break;
        case TIMER1:
            TCNT1 = 0;
            OCR1B = compareValue;
            break;
        case TIMER2:
            TCNT2 = 0;
            OCR2B = compareValue;
            break;
    }
}

void Timer::startTimer() {
    DEBUG_PRINT("Starting TIMER", static_cast<uint16_t>(id_));
    setPrescaler_(prescaler_);
}

void Timer::stopTimer() {
    DEBUG_PRINT("Stopping TIMER", static_cast<uint16_t>(id_));
    *TCCRnB_ &= ~((1 << CS00) | (1 << CS01) | (1 << CS02)); 
}