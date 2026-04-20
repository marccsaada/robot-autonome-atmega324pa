/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Liste des adresses EEPROM utilisées pour sauvegarder les résultats de chaque
              zone (locaux A à D et couloirs ouest/est). Centralise les constantes pour
              éviter de les redéfinir un peu partout dans le code.
*/

#pragma once
#include <stdint.h>

enum EEPROMAddress : uint16_t {
    PERSON_A      = 100,
    OBJECT_B      = 101,
    OBJECT_C      = 102,
    PERSON_D      = 103, 
    DAMAGE_OUEST  = 104,
    DAMAGE_EST    = 105
};