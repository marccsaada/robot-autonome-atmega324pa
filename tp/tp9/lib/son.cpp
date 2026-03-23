#define F_CPU 8000000UL
#include "son.hpp"

Son::Son() : timer_(Timer::Id::TIMER2) {
    DDRD |= (1 << PD6);
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

    timer_.setModeCTC(Timer::Prescaler::PRESCALE_8);

    uint16_t ocr = calculerOCR_(f);

    timer_.setOCRA(ocr);

    TCCR2A |= (1 << COM2A0);
}

void Son::arreter() {
    TCCR2A &= ~(1 << COM2A0);

    timer_.stopTimer();
}

uint16_t Son::calculerOCR_(float frequence) {
    return (F_CPU / (2 * 8 * frequence)) - 1;
}
