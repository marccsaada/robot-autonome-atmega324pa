#ifndef CAPTEUR_DISTANCE_H
#define CAPTEUR_DISTANCE_H

#include <avr/io.h>
#include <util/delay.h>
#include "can.h"
#include "robot.hpp"
#include "memoire_24.h"

static const uint16_t POTEAU_THRESHOLD = 100;
static const uint16_t SPIN_SPEED       = 95;
static const uint16_t FULL_ROTATION_MS = 2800;
static const uint8_t  SCAN_STEP_MS     = 10;
static const uint16_t NOTE_GAP_MS      = 125;
static const uint16_t NOTE_DURATION_MS = 25;
static const uint8_t  PORT_POSITION    = 5;

class DistanceSensor
{
public:

    DistanceSensor();

    ~DistanceSensor();

    uint16_t readADC();

    bool isObjectDetected(uint16_t threshold);

    void sortArray(uint16_t array[], uint8_t size);

    uint8_t getObjectCounter() {return personCounter_;}

    void scanRoom(Robot& robot);

    void evacuatePoteau(Robot& robot);

    void playConfirmSequence(Robot& robot);

    void blinkGreenClear(Robot& robot);

private:
    can can_;

    uint8_t personCounter_ = 0;

    static const uint8_t N_READINGS = 7;

    uint16_t readings[N_READINGS];

    static const uint16_t EEPROM_ADDR_COUNT = 0x0000;

    Memoire24CXXX eeprom_;
    bool objectPresent_ = false;
};

#endif /* CAPTEUR_DISTANCE_H */