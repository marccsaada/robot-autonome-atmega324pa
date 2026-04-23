/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Liste des adresses EEPROM utilisées pour sauvegarder les résultats de chaque
              zone (locaux A à D et couloirs ouest/est). Centralise les constantes pour
              éviter de les redéfinir un peu partout dans le code.
*/

/**
 * @file eepromAdresses.hpp
 * @brief EEPROM address map for all data recorded during project execution.
 */

#pragma once
#include <stdint.h>

/**
 * @brief Named EEPROM addresses for each measurement zone.
 *
 * Each enumerator maps to a fixed byte offset in the external EEPROM where
 * the corresponding count (people, objects, or damage events) is stored.
 */
enum EEPROMAddress : uint16_t {
    PERSON_A      = 100, ///< Number of people detected in room A.
    OBJECT_B      = 101, ///< Number of objects detected in room B.
    OBJECT_C      = 102, ///< Number of objects detected in room C.
    PERSON_D      = 103, ///< Number of people detected in room D.
    DAMAGE_WEST   = 104, ///< Number of damage zones found in the west corridor.
    DAMAGE_EST    = 105  ///< Number of damage zones found in the east corridor.
};