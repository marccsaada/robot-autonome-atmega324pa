/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Implémentation de l'interpréteur. Contient la séquence de démarrage, la
              boucle principale qui lit les instructions en mémoire, et la répartition des
              opcodes vers les actions du robot.
*/

#include "interpreter.hpp"
#include <util/delay.h>

static const uint8_t  LED_CONFIRM_CYCLES    = 2;
static const uint16_t LED_CONFIRM_BLINK_MS  = 300;
static const uint8_t  MAX_NOTE_COUNT        = 3;
static const uint16_t CONFIRM_DELAY_MS      = 2000;
static const uint8_t  AMBER_CYCLES_PER_SEC  = 83;
static const uint16_t AMBER_FLASH_INTERVAL_MS = 500;
static const uint16_t SOUND_PRE_DELAY_MS    = 125;
static const uint16_t SOUND_DURATION_MS     = 250;
static const uint16_t DIRECTION_LED_DURATION_MS = 2000;



Interpreter::Interpreter(Robot& robot)
    :robot(&robot),
     loopAddress_(0),
      loopCounter_(0),
      inLoop_(false)
{}

void Interpreter::startSequence() {

    for (uint8_t i = 0; i < LED_CONFIRM_CYCLES; i++) {
        robot->led.red();
        _delay_ms(LED_CONFIRM_BLINK_MS);
        robot->led.off();
        _delay_ms(LED_CONFIRM_BLINK_MS);
    }
    for (uint8_t i = 0; i < LED_CONFIRM_CYCLES; i++) {
        robot->led.green();
        _delay_ms(LED_CONFIRM_BLINK_MS);
        robot->led.off();
        _delay_ms(LED_CONFIRM_BLINK_MS);
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

}



void Interpreter::confirm(){
    uint8_t  trInstr   = 0;
    uint8_t  attOp     = 0;
    uint16_t size      = memory_.getSize();

    robot->led.off();

    while (memory_.getAddress() < size) {
        uint8_t instr = memory_.readInstruction();
        uint8_t op    = memory_.readOperand();

        if (instr == TRD || instr == TRG) trInstr = instr;
        if (instr == ATT)                 attOp   = op;

        if (instr == SGO) executeInstruction(instr, op);  
    }

    _delay_ms(CONFIRM_DELAY_MS);

    if (trInstr != 0) executeInstruction(trInstr, 0);

    _delay_ms(CONFIRM_DELAY_MS);

    for (uint8_t i = 0; i < attOp; i++) {
        for (uint8_t j = 0; j < AMBER_CYCLES_PER_SEC; j++)
            robot->led.amber();
        robot->led.off();
        _delay_ms(AMBER_FLASH_INTERVAL_MS);
    }
}

void Interpreter::executeInstruction(uint8_t instruction, uint8_t operand) {
       switch (instruction) {
            case SGO:
                if (robot->noteCount < MAX_NOTE_COUNT) {
                    _delay_ms(SOUND_PRE_DELAY_MS);
                    robot->sound.playSound(operand);
                    _delay_ms(SOUND_DURATION_MS);
                    robot->sound.stopSound();
                    robot->noteCount++;
                }
                break;
            case TRD:
                robot->led.green();
                _delay_ms(DIRECTION_LED_DURATION_MS);
                robot->led.off();
                robot->direction = 1;
                break;
            case TRG:
                robot->led.red();
                _delay_ms(DIRECTION_LED_DURATION_MS);
                robot->led.off();
                robot->direction = 0;
                break;
            case ATT:
                robot->parkingOperand = operand;
                break;
            default:
                break;
        }
    
    }
