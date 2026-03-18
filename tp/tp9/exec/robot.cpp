#include <libstatique.hpp>
#include <robot.hpp>

void Robot::avancer(uint8_t vitesse) {
    motor_.goForward(vitesse, vitesse);
}

void Robot::reculer(uint8_t vitesse) {
    motor_.goBackward(vitesse, vitesse);
}

