/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Capteur de distance infrarouge. Sert à détecter les poteaux pendant le balayage
              d'un local, à compter les objets trouvés et à écrire le résultat en EEPROM.
*/

/**
 * @file distanceSensor.hpp
 * @brief Infrared distance sensor for pole detection and room scanning.
 */

#pragma once

#include <avr/io.h>
#include <util/delay.h>
#include "can.h"
#include "memoire_24.h"
#include "eepromAdresses.hpp"

static const uint16_t POLE_THRESHOLD     = 100;  ///< ADC value below which an object is considered detected.
static const uint16_t SPIN_SPEED         = 80;   ///< PWM speed used while spinning during a room scan.
static const uint16_t FULL_ROTATION_MS   = 3150; ///< Time (ms) for a full 360° rotation at SPIN_SPEED.
static const uint16_t STOP_COMPENSATION_MS = 350;///< Extra delay (ms) added after stopping to compensate motor inertia.
static const uint8_t  IMPULSE_SPEED      = 255;  ///< Full-power PWM for a brief re-alignment impulse.
static const uint8_t  IMPULSE_MS         = 40;   ///< Duration (ms) of the re-alignment impulse.
static const uint8_t  SCAN_STEP_MS       = 50;   ///< Delay (ms) between consecutive ADC readings during a scan.
static const uint16_t NOTE_GAP_MS        = 125;  ///< Silence gap (ms) between successive confirmation notes.
static const uint16_t NOTE_DURATION_MS   = 250;  ///< Duration (ms) of each confirmation note.
static const uint8_t  PORT_POSITION      = 5;    ///< ADC channel (PA5) connected to the distance sensor.

class Robot;

/** @brief Scan direction for @c scanRoom(). */
enum Direction { LEFT, ///< Spin left during room scan.
                 RIGHT ///< Spin right during room scan.
               };

/**
 * @brief Manages the infrared distance sensor for object detection and room scanning.
 *
 * Uses the ADC (channel PA5) to read reflected IR intensity.  During a room scan the
 * robot spins a full revolution while this class samples the sensor and writes the
 * detected object count to external EEPROM.
 */
class DistanceSensor
{
public:
    /** @brief Initialises the ADC and sensor state. */
    DistanceSensor();
    /** @brief Default destructor. */
    ~DistanceSensor();

    /**
     * @brief Reads and returns the current ADC value from the distance sensor.
     * @return Raw 10-bit ADC reading (0–1023).
     */
    uint16_t readADC();

    /**
     * @brief Returns true if the median of several ADC readings is below @p threshold.
     * @param threshold Detection threshold; lower ADC values mean a closer object.
     * @return @c true if an object is within range.
     */
    bool isObjectDetected(uint16_t threshold);

    /**
     * @brief Sorts an array of uint16_t values in ascending order (in-place).
     * @param array Pointer to the array to sort.
     * @param size  Number of elements.
     */
    void sortArray(uint16_t array[], uint8_t size);

    /**
     * @brief Spins the robot a full revolution, counts detected objects, and
     *        writes the result to EEPROM.
     * @param robot Reference to the Robot for motor and sound access.
     * @param addr  EEPROM address where the object count is stored.
     * @param dir   Spin direction (LEFT by default).
     */
    void scanRoom(Robot& robot, EEPROMAddress addr, Direction dir = LEFT);

    /**
     * @brief Backs the robot away from a pole it has collided with.
     * @param robot Reference to the Robot for motor access.
     */
    void evacuatePole(Robot& robot);

    /**
     * @brief Blinks the LED green and plays a confirmation tone when the room is clear.
     * @param robot Reference to the Robot for LED and sound access.
     */
    void blinkGreenClear(Robot& robot);

private:
    can can_;                          ///< ADC driver for sensor readings.

    static const uint8_t N_READINGS = 7; ///< Number of ADC samples taken per detection.

    uint16_t readings[N_READINGS];     ///< Scratch buffer for the most recent N_READINGS samples.

    Memoire24CXXX eeprom_;             ///< Handle to the external I²C EEPROM.

    bool objectPresent_ = false;       ///< True while an object is currently detected.
};
