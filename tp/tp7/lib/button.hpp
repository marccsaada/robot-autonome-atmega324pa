#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/**
 * @file button.hpp
 * @brief Gestion d'un bouton matériel avec interruptions.
 */

/**
 * @brief Représente un bouton physique pouvant se trouver sur différentes broches.
 */
class Button
{
public:
    /**
     * @brief Emplacements possibles pour le bouton.
     */
    enum Location {
        MOTHERBOARD,
        PD3_BUTTON,
        PB2_BUTTON
    };

    /**
     * @brief Modes d'interruption possibles pour le bouton.
     */
    enum Mode {
        RISING = (1 << ISC01) | (1 << ISC00),
        FALLING = (1 << ISC01),
        ANY = (1 << ISC00)
    };

    /**
     * @brief Construit un bouton et configure sa détection d'état.
     *
     * @param mode Mode de détection (montant/descendant/any).
     * @param location Emplacement physique du bouton.
     */
    Button(Mode mode, Location location_ = MOTHERBOARD);
    ~Button();
    /**
     * @brief Vérifie si le bouton est pressé (lecture de l'état logique).
     *
     * @return true si le bouton est pressé, false sinon.
     */
    bool isPressed();

    /**
     * @brief Initialise la broche et active l'interruption du bouton.
     */
    void init();

    /** @brief Active l'interruption du bouton. */
    void enableInterupt();

    /** @brief Désactive l'interruption du bouton. */
    void disableInterupt();

private:
    Mode mode_;
    Location location_;
};