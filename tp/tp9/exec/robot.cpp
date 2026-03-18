#include <libstatique.hpp>
#include <robot.hpp>

void Robot::avancer(uint8_t vitesse) {
    motor_.goForward(vitesse, vitesse);
}

void Robot::reculer(uint8_t vitesse) {
    motor_.goBackward(vitesse, vitesse);
}

void Robot::tournerDroite(u_int16_t angle) {
    motor_.spinRight(angle);
}

void Robot::tournerGauche(u_int16_t angle) {
    motor_.spinLeft(angle);
}

void Robot::arreter() {
    motor_.stop();
}

void Robot::allumerVert() {
    led_.green();
}

void Robot::allumerRouge() {
    led_.red();
}

void Robot::eteindreDEL() {
    led_.off();
}

void Robot::attendre(uint8_t multiplicateur) {
    for (uint8_t i = 0; i < multiplicateur; ++i) {
        _delay_ms(25); // Delay of 25 ms per unit of multiplicateur
    }
}