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
        DEBUG_PRINT("]: ", readings[i]);

    }

    sortArray(readings, N_READINGS);

    uint16_t median = readings[N_READINGS / 2];

    DEBUG_PRINT("ADC median: ", median);
    
    _delay_ms(500);
    return median;
}


// bool DistanceSensor::isObjectDetected(uint16_t threshold) // defaut
// {
//     uint16_t adc = readADC();
//     DEBUG_PRINT("threshold: ", threshold);

//     if (adc >= threshold){
//         objectCounter_ +=1;
//         DEBUG_PRINT("DETECTED count: ", objectCounter_);
//         return true;
//     }
//     else {
//         DEBUG_PRINT("not detected");
//         return false;
//     }
// }

bool DistanceSensor::isObjectDetected(uint16_t threshold) // update 
{
    uint16_t adc = readADC();

    // Cas 1 : objet détecté pour la première fois
    if (adc >= threshold && !objectPresent_) {
        objectPresent_ = true;
        objectCounter_ +=1;
        DEBUG_PRINT("OBJECT DETECTED (NEW)");
        DEBUG_PRINT("DETECTED count: ", objectCounter_);

        return true;
    }

    // Cas 2 : objet encore là → ne rien faire
    if (adc >= threshold && objectPresent_) {
        DEBUG_PRINT("OBJECT STILL PRESENT");
        DEBUG_PRINT("DETECTED count: ", objectCounter_);
        return false;
    }

    // Cas 3 : objet disparu → reset
    if (adc < threshold && objectPresent_) {
        objectPresent_ = false;
        DEBUG_PRINT("OBJECT REMOVED");
        DEBUG_PRINT("DETECTED count: ", objectCounter_);
    }

    return false;
}