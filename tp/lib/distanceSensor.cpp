#include "distanceSensor.hpp"

DistanceSensor::DistanceSensor(uint8_t pin) : pin_(pin) {
}

DistanceSensor::~DistanceSensor() {
}

uint16_t DistanceSensor::readADC() {
    return converter_.lecture(pin_);
}

uint8_t DistanceSensor::getDistance() {
    uint16_t adc = readADC();

    if (adc < 80)
        return 80;
    if (adc > 500)
        return 10;

    // formule basee sur la courbe datasheet
   // uint8_t distance 

    if (distance < 10)
        return 10;
    if (distance > 80)
        return 80;

    return distance;
}

bool DistanceSensor::detect(uint8_t maxDistanceCm) {
    return getDistance() <= maxDistanceCm;
}
