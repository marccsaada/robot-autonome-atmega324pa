#ifndef CAPTEUR_DISTANCE_H
#define CAPTEUR_DISTANCE_H

#include <avr/io.h>
#include <util/delay.h>
#include "can.h"


class DistanceSensor
{
public:

    DistanceSensor();

    ~DistanceSensor();

    uint16_t readADC();

    bool isObjectDetected(uint16_t threshold);

    void sortArray(uint16_t array[], uint8_t size);

    uint8_t getObjectCounter() {return objectCounter_;}

private:
    can can_;

    uint8_t objectCounter_ = 0; 

    static const uint8_t N_READINGS = 7;

    uint16_t readings[N_READINGS];

    static const uint8_t PORT_POSITION = 5;

    static const uint16_t DETECTION_THESHOLD = 471;    
    
};

#endif /* CAPTEUR_DISTANCE_H */