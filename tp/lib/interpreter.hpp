// /*
// Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
// Travail : TP9
// Section # : 05
// Équipe # : 107111
// Correcteur : Abdul-wahab Chaarani

// Description : Lit le bytecode stocké en EEPROM externe et dispatch chaque instruction
//               vers le contrôleur Robot.
// */

#pragma once
#include "robot.hpp"
#include "memoryReader.hpp"

const uint8_t DBT   = 0x01;
const uint8_t ATT   = 0x02;
const uint8_t DAL   = 0x44;
const uint8_t DET   = 0x45;
const uint8_t SGO   = 0x48;
const uint8_t SAR   = 0x09;
const uint8_t MAR   = 0x60;
const uint8_t MAR_1 = 0x61;
const uint8_t MAV   = 0x62;
const uint8_t MRE   = 0x63;
const uint8_t TRD   = 0x64;
const uint8_t TRG   = 0x65;
const uint8_t DBC   = 0xC0;
const uint8_t FBC   = 0xC1;
const uint8_t FIN   = 0xFF;

/**
 * @file interpreter.hpp
 * @brief Bytecode interpreter reading instructions from external EEPROM.
 */

/**
 * @brief Reads instructions from memory and executes them on the robot.
 */
class Interpreter {
public:
    /** @brief Constructs the interpreter. */
    Interpreter();
    /** @brief Plays the startup LED sequence (2x red, 2x green). */
    void startSequence();
    /** @brief Reads and executes all instructions until the FIN instruction. */
    void execute();
    /** @brief Confirms reception: plays notes, signals tr direction, blinks amber attOp times. */
    void confirm();

private:
    Robot   robot_;
    Memory memory_;

    uint16_t loopAddress_;
    uint8_t  loopCounter_;
    bool     inLoop_;

    /**
     * @brief Dispatches a single instruction to the robot.
     * @param instruction Instruction byte.
     * @param operand     Operand byte.
     */
    void executeInstruction(uint8_t instruction, uint8_t operand);
};
