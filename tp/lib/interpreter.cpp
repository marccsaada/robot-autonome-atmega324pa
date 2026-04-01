// /*
// Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
// Travail : TP9
// Section # : 05
// Équipe # : 107111
// Correcteur : Abdul-wahab Chaarani

// Description : Implémentation de l'interpréteur : séquence de démarrage et boucle d'exécution.
// */

#include "interpreter.hpp"
#include "debug.hpp"
#include <util/delay.h>


const uint8_t DBT = 0x01;
const uint8_t ATT = 0x02;
const uint8_t DAL = 0x44;
const uint8_t DET = 0x45;
const uint8_t SGO = 0x48;
const uint8_t SAR = 0x09;
const uint8_t MAR = 0x60;
const uint8_t MAR_1 = 0x61;
const uint8_t MAV = 0x62;
const uint8_t MRE = 0x63;
const uint8_t TRD = 0x64;
const uint8_t TRG = 0x65;
const uint8_t DBC = 0xC0;
const uint8_t FBC = 0xC1;
const uint8_t FIN = 0xFF;

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

void Interpreter::executeInstruction(uint8_t instruction, uint8_t operand) {
    switch (instruction) {

        case ATT:
            DEBUG_PRINT("Wait for ", operand * 25);
            robot_.wait(operand);
            break;

        case DAL:
            DEBUG_PRINT("Set LED to ", operand);
            if (operand == 1)      robot_.led.green();
            else if (operand == 2) robot_.led.red();
            break;

        case DET:
            DEBUG_PRINT("Turn LED off");
            robot_.led.off();
            break;

        case SGO:
            DEBUG_PRINT("Play sound ", operand);
            robot_.sound.playSound(operand);
            break;

        case SAR:
            DEBUG_PRINT("Stop sound");
            robot_.sound.stopSound();
            break;

        case MAR:
        case MAR_1:
            DEBUG_PRINT("Stop robot");
            robot_.motor.stop();
            break;

        case MAV:
            DEBUG_PRINT("Move forward by ", operand);
            robot_.motor.goForward(operand, operand);
            break;

        case MRE:
            DEBUG_PRINT("Move backward by ", operand);
            robot_.motor.goBackward(operand, operand);
            break;

        case TRD:
            DEBUG_PRINT("Turn right 90");
            robot_.motor.spinRight(90);
            break;

        case TRG:
            DEBUG_PRINT("Turn left 90");
            robot_.motor.spinLeft(90);
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