#include "son.hpp"
#include <avr/interrupt.h>

static volatile bool sonActif_ = false;

ISR(TIMER2_COMPA_vect) {
    if (sonActif_)
        PIND |= (1 << PD6); // toggle PD6 opposite to PD7
}

Son::Son() : timer_(Timer::Id::TIMER2) {
    DDRD |= (1 << PD7) | (1 << PD6);
}

void Son::jouer(uint8_t note) {
    if (note < 45 || note > 81)
        return;

    static const float frequences[] = {
        110,116.54,123.47,130.81,138.59,146.83,155.56,164.81,
        174.61,185,196,207.65,220,233.08,246.94,261.63,
        277.18,293.66,311.13,329.63,349.23,369.99,392,
        415.30,440,466.16,493.88,523.25,554.37,587.33,
        622.25,659.26,698.46,739.99,783.99,830.61,880
    };

    float f = frequences[note - 45];

    TIFR2 |= (1 << OCF2B);  // clear pending flag before setModeCTC enables OCIE2B
    timer_.setModeCTC(Timer::Prescaler::PRESCALE_256);
    TIMSK2 &= ~(1 << OCIE2B); // disable unused compare B interrupt

    uint16_t ocr = calculerOCR_(f);
    timer_.setOCRA(ocr);

    // Force OC2A latch LOW for consistent phase on every call
    TCCR2A = (TCCR2A & ~(1 << COM2A0)) | (1 << COM2A1); // clear-on-match mode
    TCCR2B |= (1 << FOC2A);                               // force clear → OC2A = LOW
    TCCR2A = (TCCR2A & ~(1 << COM2A1)) | (1 << COM2A0); // toggle mode

    // PD7 (OC2A) = LOW, PD6 = HIGH → differential drive
    PORTD |= (1 << PD6);
    sonActif_ = true;

    timer_.startTimer();
    sei();
}

void Son::arreter() {
    sonActif_ = false;
    TCCR2A &= ~(1 << COM2A0);
    PORTD &= ~((1 << PD7) | (1 << PD6));
    timer_.stopTimer();
}

uint16_t Son::calculerOCR_(float frequence) {
    return (F_CPU / (2 * 256 * frequence)) - 1;
}
