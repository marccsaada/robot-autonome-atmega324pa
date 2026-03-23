/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9 - Programme 2 : lecture et interprétation des octets en mémoire externe
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Ce programme lit le programme robot stocké en mémoire EEPROM
              externe (24CXXX) et exécute les instructions sur le robot.
              Il doit être flashé EN SECOND, après le programme ecriture/.

Procédure :
  1. Flasher ecriture/ et attendre la LED verte (EEPROM écrite).
  2. Flasher CE programme (exec/).
  3. Brancher le robot — séquence de démarrage (2x rouge, 2x vert)
     puis exécution automatique du programme en mémoire.

Connexions (ATmega324PA) :
  PA0 : LED anode verte      PA1 : LED anode rouge
  PD4 : DIR moteur droit     PD5 : DIR moteur gauche
  PD6 : PWM moteur droit     PD7 : PWM moteur gauche
  PC0 : SDA (I2C EEPROM)     PC1 : SCL (I2C EEPROM)
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include "interpreteur.hpp"

int main() {
    Interpreteur interpreteur;
    interpreteur.sequenceDemarrage();
    interpreteur.executer();

    while (true) {}
}
