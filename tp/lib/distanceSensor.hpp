#pragma once
#include <avr/io.h>
#include <stdint.h>
#include "can.h"

class DistanceSensor {
public:
    DistanceSensor(uint8_t pin);
    ~DistanceSensor();

    uint16_t readADC();
    uint8_t getDistance();
    bool detect(uint8_t maxDistanceCm);

private:
    can converter_;
    uint8_t pin_;
};
