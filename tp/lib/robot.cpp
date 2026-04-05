// /*
// Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
// Travail : TP9
// Section # : 05
// Équipe # : 107111
// Correcteur : Abdul-wahab Chaarani

// Description : Implémentation de l'interface unifiée du robot (moteurs, LED, son).
// */

#include <libstatique.hpp>
#include <robot.hpp>

void Robot::wait(uint16_t multiplicator) {
    for (uint16_t i = 0; i < multiplicator; ++i) {
        _delay_ms(25); 
    }
}