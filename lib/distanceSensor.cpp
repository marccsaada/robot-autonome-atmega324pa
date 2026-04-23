/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Implémentation du capteur de distance. On fait une médiane sur plusieurs
              lectures pour réduire le bruit, puis on balaie le local jusqu'à retrouver
              la ligne et on enregistre le nombre d'objets détectés dans la mémoire externe.
*/

#define F_CPU 8000000UL
#include "libstatique.hpp"
#include "robot.hpp"
#include "distanceSensor.hpp"
#include "debug.hpp"

static const uint16_t SENSOR_INIT_DELAY_MS    = 40;
static const uint8_t  BLINK_COUNT             = 8;
static const uint16_t BLINK_DURATION_MS       = 125;
static const uint16_t EVACUATION_RETRY_DELAY_MS = 2000;
static const uint8_t  SCAN_FORWARD_SPEED      = 200;
static const uint8_t  SCAN_INITIAL_SPIN_SPEED = 180;
static const uint16_t SCAN_INIT_DELAY_MS      = 25;

DistanceSensor::DistanceSensor() : can_() {
    DDRA &= ~(1 << PA5);
    PORTA &= ~(1 << PA5);
   _delay_ms(SENSOR_INIT_DELAY_MS);
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

void DistanceSensor::blinkGreenClear(Robot& robot) {
    for (uint8_t i = 0; i < BLINK_COUNT; i++) {
        robot.led.green();
        _delay_ms(BLINK_DURATION_MS);
        robot.led.off();
        _delay_ms(BLINK_DURATION_MS);
    }
}

void DistanceSensor::evacuatePole(Robot& robot) {
    do {
        robot.playEepromNotes();
        _delay_ms(EVACUATION_RETRY_DELAY_MS);
    } while (readADC() >= POLE_THRESHOLD);

    blinkGreenClear(robot);
}

void DistanceSensor::scanRoom(Robot& robot, EEPROMAddress addr , Direction dir) { 

    uint8_t localCount = 0;
    bool startMoving = true;

    while (robot.direction == 0 ? !robot.lineSensor.getSensor(4) : !robot.lineSensor.getSensor(0)) {
        if (readADC() >= POLE_THRESHOLD) {
            robot.led.green();
            robot.motor.stop();
            if (!objectPresent_) {
                objectPresent_ = true;
                localCount++;
                
            }
            evacuatePole(robot);
            objectPresent_ = false;
            startMoving = true;
        } else {
            if (startMoving) {
                if (robot.direction == 0) {
                    robot.motor.goForward(SCAN_FORWARD_SPEED, 0);
                    _delay_ms(SCAN_INIT_DELAY_MS);
                    robot.motor.spinRightSpeed(SCAN_INITIAL_SPIN_SPEED);
                } else {
                    robot.motor.goBackward(SCAN_FORWARD_SPEED, 0);
                    _delay_ms(SCAN_INIT_DELAY_MS);
                    robot.motor.spinLeftSpeed(SCAN_INITIAL_SPIN_SPEED);
                }
                _delay_ms(SCAN_INIT_DELAY_MS);
                startMoving = false;
            }
            if (robot.direction == 0) robot.motor.spinRightSpeed(SPIN_SPEED);
            else robot.motor.spinLeftSpeed(SPIN_SPEED);
            _delay_ms(SCAN_STEP_MS);
        }
    }
    robot.motor.stop();
    eeprom_.ecriture(addr, localCount);
}






