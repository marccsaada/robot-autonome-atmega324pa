/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9 - Programme 2 : lecture et interprétation des octets en mémoire externe
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Ce programme lit le programme robot stocké en mémoire EEPROM
              externe (24CXXX) et exécute les instructions sur le robot.
              Il doit être flashé EN SECOND, après le programme ecriture/.
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include "interpreter.hpp"

int main() {
    Interpreter interpreter;
    interpreter.startSequence();
    interpreter.execute();

    while (true) {}
    
}
