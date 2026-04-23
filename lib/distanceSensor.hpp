/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Capteur de distance infrarouge. Sert à détecter les poteaux pendant le balayage
              d'un local, à compter les objets trouvés et à écrire le résultat en EEPROM.
*/

#pragma once

#include <avr/io.h>
#include <util/delay.h>
#include "can.h"
#include "memoire_24.h"
#include "eepromAdresses.hpp"
static const uint16_t POLE_THRESHOLD     = 100;
static const uint16_t SPIN_SPEED           = 80;
static const uint16_t FULL_ROTATION_MS     = 3150;
static const uint16_t STOP_COMPENSATION_MS = 350;
static const uint8_t  IMPULSE_SPEED        = 255;
static const uint8_t  IMPULSE_MS           = 40;
static const uint8_t  SCAN_STEP_MS         = 50;
static const uint16_t NOTE_GAP_MS          = 125;
static const uint16_t NOTE_DURATION_MS     = 250;
static const uint8_t  PORT_POSITION        = 5;
class Robot;
enum Direction { LEFT, RIGHT };

class DistanceSensor
{
public:

    DistanceSensor();
    ~DistanceSensor();

    uint16_t readADC();
    bool isObjectDetected(uint16_t threshold);
    void sortArray(uint16_t array[], uint8_t size);
    void scanRoom(Robot& robot, EEPROMAddress addr ,Direction dir = LEFT);
    void evacuatePole(Robot& robot);
    void blinkGreenClear(Robot& robot);

private:
    can can_;

    static const uint8_t N_READINGS = 7;

    uint16_t readings[N_READINGS];

    Memoire24CXXX eeprom_;

    bool objectPresent_ = false;
};
