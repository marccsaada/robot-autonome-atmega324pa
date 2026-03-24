#define F_CPU 8000000UL
#include "son.hpp"
#include "debug.hpp"

Son::Son() : timer_(Timer::Id::TIMER0) {
    DDRB |= (1 << PB3);  
    DDRB |= (1 << PB5);
    PORTB &= ~(1<<PB5);
    timer_.setModePWM(Timer::PWMMode::FAST, Timer::Prescaler::PRESCALE_256);
}

void Son::jouer(uint8_t note) {
    if (note < 45 || note > 81)
        return;

    static const float frequences[] = {
        110, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81,
        174.61, 185, 196, 207.65, 220, 233.08, 246.94, 261.63,
        277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392,
        415.30, 440, 466.16, 493.88, 523.25, 554.37, 587.33,
        622.25, 659.26, 698.46, 739.99, 783.99, 830.61, 880
    };

    float f = frequences[note - 45];
    uint16_t ocr = calculerOCR_(f);

    DEBUG_PRINT("note: ", note);
    DEBUG_PRINT("OCR: ", ocr);

    // Fast PWM mode 7 (WGM02:WGM01:WGM00 = 1:1:1), TOP = OCR0A
    // setModePWM(FAST) configure WGM00=1, WGM01=1 et COM0A1=1 (non-inverseur)
    // On ajoute WGM02=1 pour le mode 7 et on corrige COM pour obtenir le toggle
    // timer_.setModePWM(Timer::PWMMode::FAST, Timer::Prescaler::PRESCALE_256);
    TCCR0B |= (1 << WGM02);                       // Mode 7 : TOP = OCR0A
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0B1));  // Deconnecter OC0B (PB4) mis par setModePWM
    TCCR0A |=  (1 << COM0A0);                     // Toggle OC0A (PB3) -> onde carree 50%
    TIMSK0 = 0;
    timer_.setOCRA(ocr);
    timer_.startTimer();
}

void Son::arreter() {
    TCCR0A &= ~(1 << COM0A0);    // Deconnecter OC0A
    timer_.stopTimer();
}

uint16_t Son::calculerOCR_(float frequence) {
    return (F_CPU / (2 * 256 * frequence)) - 1;
}
