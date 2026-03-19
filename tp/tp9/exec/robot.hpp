#pragma once
#include "motor.hpp"
#include "LED.hpp"
#include <avr/io.h>
#include <util/delay.h>

class Robot {
public:
    Robot() : motor_(), led_(PORTA, PA0, PA1) {};
    void avancer(uint8_t vitesse);
    void reculer(uint8_t vitesse);
    void tournerDroite(uint16_t angle);
    void tournerGauche(uint16_t angle);
    void arreter();

    
    void allumerVert();
    void allumerRouge();
    void eteindreDEL();

    void attendre(uint8_t multiplicateur); // 25ms * multiplicateur

private:
    Motor motor_;
    LED led_;
};