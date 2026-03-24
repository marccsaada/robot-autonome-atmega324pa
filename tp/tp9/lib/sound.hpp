/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9 - Synthèse sonore PWM
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Génère des sons via PWM (Timer0) en calculant la valeur OCR correspondant
              à chaque note MIDI (45–81) pour le buzzer.
*/

#pragma once
#include <avr/io.h>
#include "timer.hpp"

/**
 * @file sound.hpp
 * @brief PWM-based sound synthesis using Timer0 to play MIDI notes on the buzzer.
 */

/**
 * @brief Controls the buzzer via Timer0 in CTC mode.
 */
class Sound {
public:
    /** @brief Initializes the buzzer and configures Timer0 in fast PWM mode. */
    Sound();

    /**
     * @brief Plays a MIDI note on the buzzer.
     * @param note MIDI note number (45–81 inclusive).
     */
    void playSound(uint8_t note);

    /** @brief Stops sound generation. */
    void stopSound();

private:
    Timer timer_;

    /**
     * @brief Computes the OCR value for a given frequency.
     * @param frequency Target frequency in Hz.
     * @return Value to load into the OCRxA compare register.
     */
    uint16_t calculateOCR_(float frequency);
};
