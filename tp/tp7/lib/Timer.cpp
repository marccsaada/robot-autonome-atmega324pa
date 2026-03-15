
#define F_CPU 8000000UL
#include "Timer.hpp"
#include "debug.hpp"

// Selon le timer choisi, on configure les registres correspondants (TIMSKn, TCCRnA, TCCRnB), ce qui inclut le masque, le type de timer et les bits de contrôle.
Timer::Timer(Id id) : id_(id) {
    DEBUG_PRINT("Initializing Timer");

    switch(id_) {
        case Id::TIMER0:
        DEBUG_PRINT("Selected TIMER0");
            TCCRnA_ = &TCCR0A;
            TCCRnB_ = &TCCR0B;
            break;
        case Id::TIMER1:
        DEBUG_PRINT("Selected TIMER1");
            TCCRnA_ = &TCCR1A;
            TCCRnB_ = &TCCR1B;
            break;
        case Id::TIMER2:                             // !!! ATTENTION !!! : TIMSK2 = (1 << OCIE2A) | (1 << OCIE2B); Cette ligne a ete effacer car nous nutilisons jamais dinterruptions avec le tiemr 2
        DEBUG_PRINT("Selected TIMER2");
            TCCRnA_ = &TCCR2A;
            TCCRnB_ = &TCCR2B;
            break;
    }
}

// En fonction du mode PWM choisi, on configure les bits de contrôle WGMn0, WGMn1 et WGMn2 pour les timers 0 et 2, et WGMn10, WGMn11, WGMn12 et WGMn13 pour le timer 1.
// On sait que les WGM0n et WGM2n contiennent les memes valeurs, donc on simplifie en utilisant les memes bits de controle pour les deux timers.
void Timer::setModePWM(PWMMode mode, Prescaler prescaler) {
    DEBUG_PRINT("Setting Timer mode to PWM");
    DEBUG_PRINT("Setting Prescaler : ", static_cast<uint16_t>(prescaler));
    prescaler_ = prescaler;

    switch(mode) {
        case PWMMode::PHASE_CORRECT:
            DEBUG_PRINT("Setting PWM mode : Phase Correct");
            if (id_ == Id::TIMER0 || id_ == Id::TIMER2) {
                *TCCRnA_ |= (1 << WGM00);
                *TCCRnA_ &= ~(1 << WGM01);
                *TCCRnB_ &= ~(1 << WGM02);
            } else if (id_ == Id::TIMER1) {
                *TCCRnA_ |= (1 << WGM10);
                *TCCRnA_ &= ~(1 << WGM11);
                *TCCRnB_ &= ~(1 << WGM12) & ~(1 << WGM13);
            }
            break;
        case PWMMode::FAST:
            DEBUG_PRINT("Setting PWM mode : Fast");
            if (id_ == Id::TIMER0 || id_ == Id::TIMER2) {
                *TCCRnA_ |= (1 << WGM00) | (1 << WGM01);
                *TCCRnB_ &= ~(1 << WGM02);
            } else if (id_ == Id::TIMER1) {
                *TCCRnA_ |= (1 << WGM10) | (1 << WGM11);
                *TCCRnB_ &= ~(1 << WGM12) & ~(1 << WGM13);
            }
            break;
    }
    // bits de COM
    *TCCRnA_ |= (1 << COM2A1) | (1 << COM2B1);
    *TCCRnA_ &= ~((1 << COM2A0) | (1 << COM2B0));
}

void Timer::setModeCTC(Prescaler prescaler) {
    DEBUG_PRINT("Setting Timer mode to CTC");
    DEBUG_PRINT("Setting Prescaler : ", static_cast<uint16_t>(prescaler));
    prescaler_ = prescaler;

    // Ajustement des masques
    if (id_ == Id::TIMER0) {
        DEBUG_PRINT("Setting CTC Timer interrupts for TIMER0");
        TIMSK0 = (1 << OCIE0A) | (1 << OCIE0B);
    } else if (id_ == Id::TIMER1) {
        DEBUG_PRINT("Setting CTC Timer interrupts for TIMER1");
        TIMSK1 = (1 << OCIE1A) | (1 << OCIE1B);
    } else if (id_ == Id::TIMER2) {
        DEBUG_PRINT("Setting CTC Timer interrupts for TIMER2");
        TIMSK2 = (1 << OCIE2A) | (1 << OCIE2B);
    }

    switch(id_) {
        case Id::TIMER0:
        case Id::TIMER2:
            *TCCRnA_ &= ~(1 << WGM00);
            *TCCRnA_ |= (1 << WGM01);
            *TCCRnB_ &= ~(1 << WGM02);
            break;
        case Id::TIMER1:
            *TCCRnA_ &= ~(1 << WGM10) & ~(1 << WGM11);
            *TCCRnB_ |= (1 << WGM12);
            *TCCRnB_ &= ~(1 << WGM13);
            break;
    }
}

void Timer::setPrescaler_(Prescaler prescaler) {
    switch(prescaler) {
    case Prescaler::NO_PRESCALING:
        *TCCRnB_ |= (1 << CS00);
        *TCCRnB_ &= ~((1 << CS01) | (1 << CS02));
        break;
    case Prescaler::PRESCALE_8:
        *TCCRnB_ |= (1 << CS01);
        *TCCRnB_ &= ~((1 << CS00) | (1 << CS02));
        break;
    case Prescaler::PRESCALE_64:
        *TCCRnB_ |= (1 << CS01) | (1 << CS00);
        *TCCRnB_ &= ~(1 << CS02);
        break;
    case Prescaler::PRESCALE_256:
        *TCCRnB_ |= (1 << CS02);
        *TCCRnB_ &= ~((1 << CS00) | (1 << CS01));
        break;
    case Prescaler::PRESCALE_1024:
        *TCCRnB_ |= (1 << CS02) | (1 << CS00);
        *TCCRnB_ &= ~(1 << CS01);
        break;
    }
}

void Timer::setOCRA(uint16_t compareValue) {
    DEBUG_PRINT("Setting OCRA with compareValue : ", compareValue);
    switch(id_) {
        case Id::TIMER0: 
            TCNT0 = 0;
            OCR0A = compareValue;
            break;
        case Id::TIMER1:
            TCNT1 = 0;
            OCR1A = compareValue;
            break;
        case Id::TIMER2:
            TCNT2 = 0;
            OCR2A = compareValue;
            break;
    }
}
void Timer::setOCRB(uint16_t compareValue) {
    DEBUG_PRINT("Setting OCRB with compareValue : ", compareValue);
    switch(id_) {
        case Id::TIMER0:
            TCNT0 = 0;
            OCR0B = compareValue;
            break;
        case Id::TIMER1:
            TCNT1 = 0;
            OCR1B = compareValue;
            break;
        case Id::TIMER2:
            TCNT2 = 0;
            OCR2B = compareValue;
            break;
    }
}

void Timer::startTimer() {
    DEBUG_PRINT("Starting TIMER", static_cast<uint16_t>(id_));
    setPrescaler_(prescaler_);
}

// Arrêt du timer en mettant les bits de prescaler à 0
void Timer::stopTimer() {
    DEBUG_PRINT("Stopping TIMER", static_cast<uint16_t>(id_));
    *TCCRnB_ &= ~((1 << CS00) | (1 << CS01) | (1 << CS02)); 
}