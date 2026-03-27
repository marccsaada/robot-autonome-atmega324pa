#define F_CPU 8000000UL
#include "libstatique.hpp"
#include "distanceSensor.hpp"
#include "debug.hpp"

DistanceSensor::DistanceSensor() : can_() {
    DDRA &= ~(1 << PA5);
    PORTA &= ~(1 << PA5);
    _delay_ms(40);    //pas sur, a voir dans le datasheet.
}

DistanceSensor::~DistanceSensor() {};

void DistanceSensor::sortArray(uint16_t array[], uint8_t size)
{
    for (uint8_t i = 1; i < size; i++)
    {
        uint16_t key = array[i];  
        int8_t j = i - 1;

        while (j >= 0 && array[j] > key)
        {
            array[j + 1] = array[j];
            j--;
        }

        array[j + 1] = key;
    }
}


uint16_t DistanceSensor::readADC() {
    for (uint8_t i = 0; i < N_READINGS; i++) {
        readings[i] = can_.lecture(PORT_POSITION);
        DEBUG_PRINT("  reading[", i);
        _delay_ms(500);
        DEBUG_PRINT("]: ", readings[i]);
        _delay_ms(500);

    }

    sortArray(readings, N_READINGS);

    uint16_t median = readings[N_READINGS / 2];

    DEBUG_PRINT("ADC median: ", median);
    
    _delay_ms(500);
    return median;
}


bool DistanceSensor::isObjectDetected(uint16_t threshold)
{
    uint16_t adc = readADC();
    DEBUG_PRINT("threshold: ", threshold);

    if (adc >= threshold){
        objectCounter_ +=1;
        DEBUG_PRINT("DETECTED count: ", objectCounter_);
       _delay_ms(500);

        return true;
    }
    else {
        DEBUG_PRINT("not detected");
        _delay_ms(500);
        return false;
    }
}
