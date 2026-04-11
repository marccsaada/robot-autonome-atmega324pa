#define F_CPU 8000000UL
#include "libstatique.hpp"
#include "distanceSensor.hpp"
#include "debug.hpp"

DistanceSensor::DistanceSensor() : can_() {
    DDRA &= ~(1 << PA0);
    PORTA &= ~(1 << PA0);
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
        readings[i] = can_.lecture(7);
        DEBUG_PRINT("  reading[", i);
        DEBUG_PRINT("]: ", readings[i]);

    }

    sortArray(readings, N_READINGS);

    uint16_t median = readings[N_READINGS / 2];

    DEBUG_PRINT("ADC median: ", median);
    return median;
    return can_.lecture(7);
}



void DistanceSensor::playConfirmSequence(Robot& robot) {
    for (uint8_t i = 0; i < 3 ; i++) {
        _delay_ms(NOTE_GAP_MS);
        robot.sound.playSound(70);
        _delay_ms(NOTE_DURATION_MS);
        robot.sound.stopSound();
    }
}

void DistanceSensor::blinkGreenClear(Robot& robot) {
    for (uint8_t i = 0; i < 8; i++) {
        robot.led.green();
        _delay_ms(125);
        robot.led.off();
        _delay_ms(125);
    }
}

void DistanceSensor::evacuatePoteau(Robot& robot) {
    do {
        playConfirmSequence(robot);
        _delay_ms(2000);
    } while (readADC() >= POTEAU_THRESHOLD);

    blinkGreenClear(robot);
}

void DistanceSensor::scanRoom(Robot& robot) {

    uint16_t elapsed = 0;

    while (elapsed < FULL_ROTATION_MS) {
        if (readADC() >= POTEAU_THRESHOLD) {
            robot.motor.stop();
            if (!objectPresent_) {
                objectPresent_ = true;
                personCounter_++;
                eeprom_.ecriture(EEPROM_ADDR_COUNT, personCounter_);
            }
            evacuatePoteau(robot);
            objectPresent_ = false;
        } else {
            robot.motor.spinLeftSpeed(SPIN_SPEED);
            _delay_ms(SCAN_STEP_MS);
            elapsed += SCAN_STEP_MS;
        }
    }
    robot.motor.stop();
}






