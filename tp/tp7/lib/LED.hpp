#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

/**
 * @file LED.hpp
 * @brief Interface de contrôle pour une LED bicolore (rouge/vert) sur la carte.
 */

/**
 * @brief Pilote une LED bicolore en utilisant deux sorties GPIO.
 */
class LED {
public:
    /**
     * @brief Constructeur.
     *
     * @param port Référence vers le registre PORTx utilisé pour la LED.
     * @param pinIn Broche du port correspondant à la couleur rouge.
     * @param pinOut Broche du port correspondant à la couleur verte.
     */
    LED(volatile uint8_t &port, uint8_t pinIn, uint8_t pinOut);
    
    /** @brief Éteint la LED. */
    void off();
    /** @brief Allume la LED en rouge. */
    void red();

    /** @brief Allume la LED en vert. */
    void green();

    /**
     * @brief Allume la LED en ambre (alternance rouge / vert).
     *
     * Où la couleur est obtenue en alternant rapidement entre rouge et vert.
     */
    void amber();

private:
    /** @brief Configure les broches en sortie. */
    void setDDR();
        volatile uint8_t* port_;
        volatile uint8_t* ddr_;
        uint8_t pinIn_, pinOut_;
};