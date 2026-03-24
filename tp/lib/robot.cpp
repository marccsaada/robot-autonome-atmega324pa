/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Implémentation de l'interface unifiée du robot (moteurs, LED, son).
*/

#include <libstatique.hpp>
#include <robot.hpp>

void Robot::goForward(uint8_t speed) {
    motor_.goForward(speed, speed);
}

void Robot::goBackward(uint8_t speed) {
    motor_.goBackward(speed, speed);
}

void Robot::turnRight(uint16_t angle) {
    motor_.spinRight(angle);
}

void Robot::turnLeft(uint16_t angle) {
    motor_.spinLeft(angle);
}

void Robot::stopMotor() {
    motor_.stop();
}

void Robot::turnOnGreen() {
    led_.green();
}

void Robot::turnOnRed() {
    led_.red();
}

void Robot::turnOffLED() {
    led_.off();
}

void Robot::playSound(uint8_t note) {
    sound_.playSound(note);
}

void Robot::stopSound() {
    sound_.stopSound();
}

void Robot::wait(uint16_t multiplicator) {
    for (uint16_t i = 0; i < multiplicator; ++i) {
        _delay_ms(25); 
    }
}