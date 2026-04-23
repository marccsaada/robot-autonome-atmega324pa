/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Gestion des cinq capteurs de ligne du robot. Permet de savoir si le robot
              est centré, décalé, contre un mur ou s'il a rencontré une zone endommagée.
*/

/**
 * @file lineSensor.hpp
 * @brief Five-channel line sensor array for path following and event detection.
 */

#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include "memoire_24.h"
#include "eepromAdresses.hpp"

/** @brief PA0 — leftmost sensor (sensor 1). */
#define sensor1 PA0
/** @brief PA1 — second sensor from the left (sensor 2). */
#define sensor2 PA1
/** @brief PA2 — centre sensor (sensor 3). */
#define sensor3 PA2
/** @brief PA3 — second sensor from the right (sensor 4). */
#define sensor4 PA3
/** @brief PA4 — rightmost sensor (sensor 5). */
#define sensor5 PA4

/**
 * @brief Reads and interprets the five reflective line sensors on port A.
 *
 * Sensors are active-high: a logic 1 means the sensor is over a dark line.
 * The class tracks damage zone and object counts and can write them to EEPROM.
 */
class LineSensor {
public:
    /** @brief Configures PA0–PA4 as inputs. */
    LineSensor();
    /** @brief Default destructor. */
    ~LineSensor();

    /**
     * @brief Returns true when none of the five sensors detects a line.
     *
     * Indicates the robot is positioned between lines (open floor).
     */
    bool robotMiddle();

    /**
     * @brief Returns true when the leftmost sensor (sensor 1) detects a line.
     *
     * Used to determine that the robot has drifted to the left of the path.
     */
    bool offTrackLeft();

    /**
     * @brief Returns true when the rightmost sensor (sensor 5) detects a line.
     *
     * Used to determine that the robot has drifted to the right of the path.
     */
    bool offTrackRight();

    /**
     * @brief Returns true when all five sensors simultaneously detect a line.
     *
     * Indicates the robot has crossed a perpendicular intersection line.
     */
    bool robotBumpLine();

    /**
     * @brief Returns true when sensor 1 (PA0) detects a line.
     *
     * Equivalent to @c offTrackLeft(); used in wall-following contexts.
     */
    bool isLeftWall();

    /**
     * @brief Returns true when sensors 1 and 2 both detect a line.
     *
     * Indicates the robot is riding the left boundary line.
     */
    bool isOnLeftLine();

    /**
     * @brief Returns true when sensor 5 (PA4) detects a line.
     *
     * Equivalent to @c offTrackRight(); used in wall-following contexts.
     */
    bool isRightWall();

    /**
     * @brief Returns true when sensors 4 and 5 both detect a line.
     *
     * Indicates the robot is riding the right boundary line.
     */
    bool isOnRightLine();

    /**
     * @brief Returns true when at least one of sensors 3, 4, or 5 detects a line.
     */
    bool sensors345();

    /**
     * @brief Detects a damage zone crossing and writes the running count to EEPROM.
     *
     * A damage zone is identified by an asymmetric sensor pattern that is not
     * a full bump-line event. The count is incremented on the trailing edge and
     * stored at @p addr in external EEPROM.
     *
     * @param addr EEPROM address where the damage count is persisted.
     * @return @c true while a damage pattern is actively detected or during debounce.
     */
    bool findDamage(EEPROMAddress addr);

    /**
     * @brief Tracks objects that cause sensors to go off-track and counts distinct events.
     * @param addr EEPROM address where the object count is persisted.
     */
    void findObject(EEPROMAddress addr);

    /**
     * @brief Returns the number of sensors currently reading a line (0–5).
     *
     * Returns @c MAX_OFFTRACK_AMOUNT (4) when all five sensors are active to
     * distinguish a full bump-line from a heavily off-track situation.
     */
    uint8_t offTrackAmount();

    /** @brief Returns the cumulative number of damage zones recorded. */
    uint8_t getNDamage() const { return nDamage_; }

    /** @brief Increments the damage counter by one. */
    void incrementDamage() { nDamage_++; }

    /** @brief Resets the damage counter and clears the previous-state flag. */
    void resetDamage() { nDamage_ = 0; previousDamageState_ = false; }

    /** @brief Returns the previous damage detection state (true = was detecting). */
    bool getPreviousDamageState() const { return previousDamageState_; }

    /**
     * @brief Overrides the previous damage detection state.
     * @param state New state value.
     */
    void setPreviousDamageState(bool state) { previousDamageState_ = state; }

    /** @brief Resets the object counter and clears the previous-damage-state flag. */
    void resetObjectCount() { nObjects_ = 0; previousDamageState_ = false; }

    /** @brief Returns the cumulative number of objects recorded. */
    uint8_t getNObjects() const { return nObjects_; }

    /**
     * @brief Returns the raw PINA bit for the sensor at the given index (0–4).
     * @param index Sensor index (0 = leftmost, 4 = rightmost).
     * @return Non-zero if the sensor detects a line, 0 otherwise.  Returns 0 for out-of-range indices.
     */
    uint8_t getSensor(int index) const {
        if (index >= 0 && index < 5) {
            return (PINA & (1 << sensors_[index]));
        }
        return 0;
    }

private:
    uint8_t sensors_[5] = {sensor1, sensor2, sensor3, sensor4, sensor5}; ///< Ordered pin numbers for sensors 1–5.
    uint8_t nDamage_  = 0; ///< Running count of detected damage zones.
    uint8_t nObjects_ = 0; ///< Running count of detected objects.

    bool     previousDamageState_ = false; ///< True while a damage pattern was active on the last call.
    bool     previousObjectState_ = false; ///< True while an object was detected on the last call.
    bool     debouncing_          = false; ///< True while waiting for a debounce delay to expire.
    uint16_t debounceStart_       = 0;     ///< Tick value at which the current debounce started.
    Memoire24CXXX eeprom_;                 ///< Handle to the external I²C EEPROM.
};
