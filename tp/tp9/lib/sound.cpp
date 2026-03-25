/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Implémentation de la synthèse sonore PWM via Timer0 et calcul OCR par fréquence.
*/

#define F_CPU 8000000UL
#include "sound.hpp"
#include "debug.hpp"

Sound::Sound() : timer_(Timer::Id::TIMER0) {
    DDRB |= (1 << PB3);  
    DDRB |= (1 << PB1);
    PORTB &= ~(1 << PB1);
    timer_.setModePWM(Timer::PWMMode::FAST_TOP, Timer::Prescaler::PRESCALE_256);
}

const float Sound::frequencies[] = {
    110, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81,
    174.61, 185, 196, 207.65, 220, 233.08, 246.94, 261.63,
    277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392,
    415.30, 440, 466.16, 493.88, 523.25, 554.37, 587.33,
    622.25, 659.26, 698.46, 739.99, 783.99, 830.61, 880
};

void Sound::playSound(uint8_t note) {
    if (note < 45 || note > 81)
        return;

    float f = frequencies[note - 45];
    uint16_t ocr = calculateOCR_(f);

    DEBUG_PRINT("note: ", note);
    DEBUG_PRINT("OCR: ", ocr);

    TCCR0A &= ~((1 << COM0A1) | (1 << COM0B1)); 
    TCCR0A |=  (1 << COM0A0); 
    
    timer_.setOCRA(ocr);
    timer_.startTimer();
}

void Sound::stopSound() {
    TCCR0A &= ~(1 << COM0A0);   
    timer_.stopTimer();
}

uint16_t Sound::calculateOCR_(float frequency) {
    return (F_CPU / (2 * 256 * frequency));
}