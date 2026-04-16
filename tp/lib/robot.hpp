/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Regroupe les sous-systèmes du robot et toute la logique d'exécution du projet.
              Les méthodes de déplacement et la machine à états sont privées ; l'interface
              publique se limite aux trois handlers de mode et aux membres matériels.
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

class Robot {
public:
    Robot() : led(PORTB, PB0, PB2),
              button(Button::FALLING, Button::MOTHERBOARD),
              timer(Timer::TIMER1) {}

    // ── Hardware subsystems ───────────────────────────────────────────────
    Motor          motor;
    LED            led;
    Sound          sound;
    Button         button;
    LineSensor     lineSensor;
    DistanceSensor distanceSensor;
    Timer          timer;

    // ── State used by Interpreter ─────────────────────────────────────────
    uint8_t direction      = 0;   // 0 = left, 1 = right
    uint8_t note[3]        = {};
    uint8_t parkingOperand = 0;
    uint8_t noteCount      = 0;

    // ── Public interface ──────────────────────────────────────────────────
    void wait(uint16_t multiplicator);
    void playEepromNotes();

    void runInstruction();
    void runRapport();
    void runProject();

private:
    // ── Execution state (reset each runProject call) ──────────────────────
    uint8_t  stepCount    = 0;
    bool     firstTime    = true;
    uint8_t  roomCount    = 0;
    uint8_t  parkingCount = 0;

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
    void followLeftWall();
    void followRightWall();
    void followWall();
    void alignToTurn();
    bool confirmTurn();
    void detectObject(EEPROMAddress addr);
    bool foundRoom();
    bool foundRoom2();

    // ── State-machine steps ───────────────────────────────────────────────
    void movementLogic(Action& current, Action& previous);
    void switchLogic(Action& current, Action& previous);
};
