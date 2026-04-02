// /*
// Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
// Travail : TP9
// Section # : 05
// Équipe # : 107111
// Correcteur : Abdul-wahab Chaarani

// Description : Implémentation de l'interpréteur : séquence de démarrage et boucle d'exécution.
// */

#include "interpreter.hpp"
#include <util/delay.h>



Interpreter::Interpreter()
    : loopAddress_(0),
      loopCounter_(0),
      inLoop_(false)
{}

void Interpreter::startSequence() {

    for (uint8_t i = 0; i < 2; i++) {
        robot_.led.red();
        _delay_ms(300);
        robot_.led.off();
        _delay_ms(300);
    }
    for (uint8_t i = 0; i < 2; i++) {
        robot_.led.green();
        _delay_ms(300);
        robot_.led.off();
        _delay_ms(300);
    }
}

void Interpreter::execute() {
    uint16_t size = memory_.getSize();
    bool startFound = false;
    bool finished = false;

    while (!finished) {
        uint8_t instruction = memory_.readInstruction();
        uint8_t operand     = memory_.readOperand();

     
        if (!startFound) {
            if (instruction == DBT)
                startFound = true;
            else
                continue;
        }

     
        if (memory_.getAddress() > size) {
            finished = true;
            continue;
        }

        executeInstruction(instruction, operand);

        if (instruction == FIN)
            finished = true;
    }

    robot_.motor.stop();
}

void Interpreter::confirm() {
    uint8_t  noteCount = 0;
    uint8_t  trInstr   = 0;
    uint8_t  attOp     = 0;
    uint16_t size      = memory_.getSize();

    robot_.led.off();

    while (memory_.getAddress() < size) {
        uint8_t instr = memory_.readInstruction();
        uint8_t op    = memory_.readOperand();

        switch (instr) {
            case SGO:
                if (noteCount < 3) {
                    if (noteCount > 0) _delay_ms(125);
                    executeInstruction(SGO, op);
                    noteCount++;
                }
                break;
            case TRD:
            case TRG:
                if (trInstr == 0) trInstr = instr;
                break;
            case ATT:
                if (attOp == 0) attOp = op;
                break;
            default:
                break;
        }
    }

    _delay_ms(2000);

    if (trInstr != 0) executeInstruction(trInstr, 0);

    _delay_ms(2000);

    for (uint8_t i = 0; i < attOp; i++) {
        for (uint8_t j = 0; j < 83; j++)
            robot_.led.amber();  
        robot_.led.off();
        _delay_ms(500);          
    }
}

void Interpreter::executeInstruction(uint8_t instruction, uint8_t operand) {
    switch (instruction) {
        case ATT:
            robot_.wait(operand);
            break;
        case DAL:
            if (operand == 1)      robot_.led.green();
            else if (operand == 2) robot_.led.red();
            break;
        case DET:
            robot_.led.off();
            break;
        case SGO:
            robot_.sound.playSound(operand);
            _delay_ms(250);
            robot_.sound.stopSound();
            break;
        case SAR:
            robot_.sound.stopSound();
            break;
        case MAR:
            robot_.motor.stop();
            break;
        case MAV:
            robot_.motor.goForward(operand, operand);
            break;
        case MRE:
            robot_.motor.goBackward(operand, operand);
            break;
        case TRD:
            robot_.led.green();
            _delay_ms(2000);
            robot_.led.off();
            break;
        case TRG:
            robot_.led.red();
            _delay_ms(2000);
            robot_.led.off();
            break;
        case DBC:
            loopAddress_ = memory_.getAddress();
            loopCounter_ = operand;
            inLoop_      = true;
            break;
        case FBC:
            if (inLoop_ && loopCounter_ > 0) {
                memory_.setAddress(loopAddress_);
                loopCounter_--;
            } else {
                inLoop_ = false;
            }
            break;
        case FIN:
            robot_.motor.stop();
            break;
        default:
            break;
    }
}