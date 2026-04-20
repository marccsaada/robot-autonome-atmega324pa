/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Contrôle d'une LED bicolore (rouge/verte) sur deux sorties GPIO.
*/

#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

/**
 * @file LED.hpp
 * @brief Control interface for a bicolor LED (red/green) on the board.
 */

/**
 * @brief Drives a bicolor LED using two GPIO outputs.
 */
class LED {
public:
    /**
     * @brief Constructor.
     *
     * @param port Reference to the PORTx register used for the LED.
     * @param pinIn Port pin corresponding to the red color.
     * @param pinOut Port pin corresponding to the green color.
     */
    LED(volatile uint8_t &port, uint8_t pinIn, uint8_t pinOut);
    /** @brief Turns off the LED and releases the GPIO pins. */
    ~LED();
    /** @brief Turns off the LED. */
    void off();
    /** @brief Turns on the LED in red. */
    void red();

    /** @brief Turns on the LED in green. */
    void green();

    /**
     * @brief Turns on the LED in amber (rapid alternation between red and green).
     *
     * The color is obtained by rapidly alternating between red and green.
     */
    void amber();

private:
    /** @brief Configures the pins as outputs. */
    void setDDR();
        volatile uint8_t* port_;
        volatile uint8_t* ddr_;
        uint8_t pinIn_, pinOut_;
};