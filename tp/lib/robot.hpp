/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Regroupe les sous-systèmes moteurs, LED et son en une interface unifiée
              utilisée par l'interpréteur.
*/

#pragma once
#include "motor.hpp"
#include "LED.hpp"
#include "sound.hpp"
#include "button.hpp"
#include <avr/io.h>
#include <util/delay.h>

/**
 * @file robot.hpp
 * @brief High-level robot abstraction grouping motor, LED, and sound control.
 */

/**
 * @brief Unified robot interface: movement, LED signaling, and audio.
 */
class Robot {
public:
    Robot() : led(PORTC, PC0, PC1), button(Button::FALLING, Button::MOTHERBOARD) {};

    Motor motor;
    LED led;
    Sound sound;
    Button button;

    void wait(uint16_t multiplicator);




};
