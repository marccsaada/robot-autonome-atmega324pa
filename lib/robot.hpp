/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Regroupe les sous-systèmes du robot et toute la logique d'exécution du projet.
              Les méthodes de déplacement et la machine à états sont privées ; l'interface
              publique se limite aux trois gestionnaires de mode et aux membres matériels.
*/

/**
 * @file robot.hpp
 * @brief Aggregates all robot subsystems and the top-level execution logic.
 */

#pragma once
#include "motor.hpp"
#include "LED.hpp"
#include "sound.hpp"
#include "button.hpp"
#include "lineSensor.hpp"
#include "memoryReader.hpp"
#include "distanceSensor.hpp"
#include "timer.hpp"
#include <avr/io.h>
#include <util/delay.h>

/**
 * @brief Central robot object that owns all hardware subsystems and execution modes.
 *
 * Hardware peripherals are exposed as public members so that the Interpreter and
 * other helpers can drive them directly. The navigation state machine and all
 * low-level movement helpers are private.
 */
class Robot {
public:
    /** @brief Initialises the LED on PB0/PB2, the falling-edge button, and Timer 1. */
    Robot() : led(PORTB, PB0, PB2),
              button(Button::FALLING, Button::MOTHERBOARD),
              timer(Timer::TIMER1) {}

    // ── Hardware subsystems ───────────────────────────────────────────────
    Motor          motor;          ///< Dual-wheel PWM motor controller.
    LED            led;            ///< Bi-colour status LED (red / green / amber).
    Sound          sound;          ///< Piezo buzzer for note playback.
    Button         button;         ///< User push-button with debounce.
    LineSensor     lineSensor;     ///< Five-channel line-detection array (PA0–PA4).
    DistanceSensor distanceSensor; ///< Infrared distance sensor for pole/object detection.
    Timer          timer;          ///< Hardware timer used for tick generation (Timer 1).
    Memoire24CXXX  eeprom;         ///< External I²C EEPROM for persistent storage.

    // ── State used by Interpreter ─────────────────────────────────────────
    uint8_t direction      = 1;  ///< Travel direction: 1 = right, 0 = left.
    uint8_t note[3]        = {}; ///< Up to three MIDI-like note indices to play.
    uint8_t parkingOperand = 0;  ///< ATT operand encoding the target parking slot.
    uint8_t noteCount      = 0;  ///< Number of notes loaded from EEPROM so far.

    // ── Public interface ──────────────────────────────────────────────────

    /**
     * @brief Busy-waits for a number of 10 ms timer ticks.
     * @param multiplicator Number of 10 ms ticks to wait.
     */
    void wait(uint16_t multiplicator);

    /** @brief Plays the notes stored in the @c note array via the buzzer. */
    void playEepromNotes();

    /** @brief Reads the direction, notes, and parking operand from external EEPROM. */
    void readEepromOperands();

    /** @brief Runs the instruction-recording mode (receives and stores a program). */
    void runInstruction();

    /** @brief Runs the report mode (reads EEPROM results and signals them via LED/sound). */
    void runReport();

    /** @brief Runs the full autonomous project execution. */
    void runProject();

private:
    // ── Execution state (reset each runProject call) ──────────────────────
    uint8_t  stepCount    = 0;
    bool     firstTime    = true;
    uint8_t  roomCount    = 0;
    uint8_t  parkingCount = 0;
    bool parkingReady = false;

    static constexpr uint8_t  LEFT_SPEED  = 110;
    static constexpr uint8_t  RIGHT_SPEED = 110;
    static constexpr uint16_t OCR1A_10MS  = 1250;

    // ── Internal enums ────────────────────────────────────────────────────
    enum class Action {
        PARKING, LEAVE_PARKING, AFTER_PARKING,
        FIRST_TURN,  FIRST_CORRIDOR,
        SECOND_TURN, SECOND_CORRIDOR,
        THIRD_TURN,  THIRD_CORRIDOR,
        PEOPLE_ROOM, OBJECT_ROOM,
        FOURTH_TURN, ENTER_PARKING, COMPLETE_PARKING, END,
    };
    enum class Alignment { DEFAULT, LEFT, RIGHT };
    enum class Speed     { SLOW, DEFAULT, FAST };

    // ── Movement helpers ──────────────────────────────────────────────────
    void turnLeft();
    void turnRight();
    void turn();
    void turnOpposite();
    void followPath(Alignment alignment, Speed speed);
    void followLine();
    void followLeftWall(Speed speed);
    void followLeftLine();
    void followRightWall(Speed speed);
    void followRightLine();
    void followWall(Speed speed);
    void alignToTurn();
    bool confirmTurn();
    void detectObject(EEPROMAddress addr);
    bool foundRoom();
    bool foundRoom2();
    void findRoom3();
    void turnCorner();

    // ── State-machine steps ───────────────────────────────────────────────
    void movementLogic(Action& current, Action& previous);
    void switchLogic(Action& current, Action& previous);
};
