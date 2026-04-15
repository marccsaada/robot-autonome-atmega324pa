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



Interpreter::Interpreter(Robot& robot)
    :robot(&robot),
     loopAddress_(0),
      loopCounter_(0),
      inLoop_(false)
{}

void Interpreter::startSequence() {

    for (uint8_t i = 0; i < 2; i++) {
        robot->led.red();
        _delay_ms(300);
        robot->led.off();
        _delay_ms(300);
    }
    for (uint8_t i = 0; i < 2; i++) {
        robot->led.green();
        _delay_ms(300);
        robot->led.off();
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

    //robot->motor.stop();
}



void Interpreter::confirm(){
    uint8_t  trInstr   = 0;
    uint8_t  attOp     = 0;
    uint16_t size      = memory_.getSize();

    robot->led.off();

    while (memory_.getAddress() < size) {
        uint8_t instr = memory_.readInstruction();
        uint8_t op    = memory_.readOperand();

    executeInstruction(instr, op);
    }

    _delay_ms(2000);

    if (trInstr != 0) executeInstruction(trInstr, 0);

    _delay_ms(2000);

    for (uint8_t i = 0; i < attOp; i++) {
        for (uint8_t j = 0; j < 83; j++)
            robot->led.amber();  
        robot->led.off();
        _delay_ms(500);          
    }
}

void Interpreter::executeInstruction(uint8_t instruction, uint8_t operand) {
       switch (instruction) {
            case SGO:
                if (robot->noteCount < 3) {
                    robot->note[robot->noteCount] = operand;
                    _delay_ms(125);
                      robot->sound.playSound(operand);
                     _delay_ms(250);
                    robot->sound.stopSound();
                    robot->noteCount++;
                }
                break;
            case TRD:
               robot->led.green();
                _delay_ms(2000);
                robot->led.off();
                robot->direction = 0; 
                break;
            case TRG:
                robot->led.red();
                _delay_ms(2000);
                robot->led.off();
                robot->direction = 1; 
                break;
            case ATT:
                robot->parkingOperand = operand;
                break;
            default:
                break;
        }
    
    }
