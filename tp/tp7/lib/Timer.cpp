
#define F_CPU 8000000UL
#include "Timer.hpp"

// Selon le timer choisi, on configure les registres correspondants (TIMSKn, TCCRnA, TCCRnB), ce qui inclut le masque, le type de timer et les bits de contrôle.
Timer::Timer(Id id) : id_(id) {
    switch(id_) {
        case Id::TIMER0:
            TIMSK0 = (1 << OCIE0A) | (1 << OCIE0B);
            TCCRnA_ = &TCCR0A;
            TCCRnB_ = &TCCR0B;
            // TCNTn_ = &TCNT0;
            break;
        case Id::TIMER1:
            TIMSK1 = (1 << OCIE1A) | (1 << OCIE1B);
            TCCRnA_ = &TCCR1A;
            TCCRnB_ = &TCCR1B;
            // TCNTn_ = &TCNT1;
            break;
        case Id::TIMER2:
            // TIMSK2 = (1 << OCIE2A) | (1 << OCIE2B);
            TCCRnA_ = &TCCR2A;
            TCCRnB_ = &TCCR2B;
            // TCNTn_ = &TCNT2;
            break;
    }
}

// En fonction du mode PWM choisi, on configure les bits de contrôle WGMn0, WGMn1 et WGMn2 pour les timers 0 et 2, et WGMn10, WGMn11, WGMn12 et WGMn13 pour le timer 1.
// On sait que les WGM0n et WGM2n contiennent les memes valeurs, donc on simplifie en utilisant les memes bits de controle pour les deux timers.
void Timer::setModePWM(PWMMode mode, Prescaler prescaler) {
    setPrescaler_(prescaler);

    switch(mode) {
        case PWMMode::PHASE_CORRECT:
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
    setPrescaler_(prescaler);
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
    // TCNTn_ = 0; // Reset du compteur
    switch(id_) {
        case Id::TIMER0: TCNT0 = 0; OCR0A = compareValue; break;
        case Id::TIMER1: TCNT1 = 0; OCR1A = compareValue; break;
        case Id::TIMER2: TCNT2 = 0; OCR2A = compareValue; break;
    }

    // switch(id_) {
    //     case Id::TIMER0:
    //         OCR0A = compareValue;
    //         break;
    //     case Id::TIMER1:
    //         OCR1A = compareValue;
    //         break;
    //     case Id::TIMER2:
    //         OCR2A = compareValue;
    //         break;
    // }
}
void Timer::setOCRB(uint16_t compareValue) {
    // TCNTn_ = 0; // Reset du compteur
    switch(id_) {
        case Id::TIMER0: TCNT0 = 0; OCR0B = compareValue; break;
        case Id::TIMER1: TCNT1 = 0; OCR1B = compareValue; break;
        case Id::TIMER2: TCNT2 = 0; OCR2B = compareValue; break;
    }

    // switch(id_) {
    //     case Id::TIMER0:
    //         OCR0B = compareValue;
    //         break;
    //     case Id::TIMER1:
    //         OCR1B = compareValue;
    //         break;
    //     case Id::TIMER2:
    //         OCR2B = compareValue;
    //         break;
    // }
}

// Arrêt du timer en mettant les bits de prescaler à 0
void Timer::stopTimer() {
    *TCCRnB_ &= ~((1 << CS00) | (1 << CS01) | (1 << CS02)); 
}