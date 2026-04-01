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
