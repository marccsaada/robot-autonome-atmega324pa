#define F_CPU 8000000UL
#include "libstatique.hpp"
#include "robot.hpp"
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
        readings[i] = can_.lecture(5);
        DEBUG_PRINT("  reading[", i);
        DEBUG_PRINT("]: ", readings[i]);

    }

    sortArray(readings, N_READINGS);

    uint16_t median = readings[N_READINGS / 2];

    DEBUG_PRINT("ADC median: ", median);
    return median;
}



void DistanceSensor::playConfirmSequence(Robot& robot) {
    for (uint8_t i = 0; i < robot.noteCount; i++) {
        _delay_ms(125);
        robot.sound.playSound(robot.note[i]);
        _delay_ms(250);
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

void DistanceSensor::scanRoom(Robot& robot, EEPROMAddress addr , Direction dir) { //marc (adresse 10(A)-13(D))

    uint16_t elapsed = 0;
    uint8_t localCount = 0;
    uint16_t totalRotationMs = FULL_ROTATION_MS;
    bool justStopped = false;

    while (elapsed < totalRotationMs) {
        if (readADC() >= POTEAU_THRESHOLD) {
            robot.led.green();
            robot.motor.stop();
            if (!objectPresent_) {
                objectPresent_ = true;
                localCount++;
                eeprom_.ecriture(addr, localCount);
            }
            evacuatePoteau(robot);
            objectPresent_ = false;
            // justStopped = true;
        } else {
            // if (justStopped) {
            //     robot.motor.spinRightSpeed(IMPULSE_SPEED);
            //     _delay_ms(IMPULSE_MS);
            //     justStopped = false;
            // }
            robot.motor.spinRightSpeed(SPIN_SPEED);
            _delay_ms(SCAN_STEP_MS);
            elapsed += SCAN_STEP_MS;
        }
    }
    robot.motor.stop();
}






