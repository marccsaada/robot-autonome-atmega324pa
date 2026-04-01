/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Point d'entrée principal. Sélection du mode (instruction / exécution / rapport)
              par séquence DEL + bouton, puis dispatche vers le mode approprié.
              Entièrement piloté par interruptions (Timer1 CTC + INT0). Aucune scrutation.
*/

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <libstatique.hpp>

static const uint16_t COLOR_DURATION_MS = 2000;
static const uint16_t OFF_DURATION_MS   = 1000;
static const uint16_t INIT_WAIT_MS      = 2000;
static const uint16_t TICK_MS           = 10;
static const uint16_t OCR1A_10MS        = 1249;

volatile uint16_t modeInstructionTicks = 0;
volatile uint8_t  blinkCount = 0;
volatile bool     ledState = false;
volatile bool     modeInstructionDone = false;

Robot robot;
Timer timer(Timer::TIMER1);

enum Mode  { INSTRUCTION, EXECUTION, RAPPORT };
enum State { INIT, GREEN, OFF1, RED, DONE };

volatile State    state        = INIT;
volatile uint16_t ticks        = 0;
volatile Mode     selectedMode = EXECUTION;

ISR(TIMER1_COMPA_vect) {
    ticks++;
    switch (state) {
        case INIT:
            if (ticks >= INIT_WAIT_MS / TICK_MS) {
                ticks = 0; state = GREEN; robot.led.green();
            }
            break;
        case GREEN:
            if (ticks >= COLOR_DURATION_MS / TICK_MS) {
                ticks = 0; state = OFF1; robot.led.off();
            }
            break;
        case OFF1:
            if (ticks >= OFF_DURATION_MS / TICK_MS) {
                ticks = 0; state = RED; robot.led.red();
            }
            break;
        case RED:
            if (ticks >= COLOR_DURATION_MS / TICK_MS) {
                ticks = 0; state = DONE; robot.led.off(); selectedMode = EXECUTION;
            }
            break;
        default:
            break;
    }

// en dessous pour mode interruption:
    if (selectedMode == INSTRUCTION && !modeInstructionDone) {
        modeInstructionTicks++;
    //chaque 125 ms (125 / 10 = 12.5 ≈ 13 ticks)
        if (modeInstructionTicks % 13 == 0) {
            ledState = !ledState;

            if (ledState)
                robot.led.green();
            else {
                robot.led.off();
                blinkCount++; 
            }
        }
        if (blinkCount >= 8) {
            robot.led.off();
            modeInstructionDone = true;
        }
    }
}

ISR(TIMER1_COMPB_vect) {}   // requis par setModeCTC

ISR(INT0_vect) {
    if (state == GREEN) {
        selectedMode = INSTRUCTION; state = DONE; robot.led.off();
    } 
    else if (state == RED) {
        selectedMode = RAPPORT; state = DONE; robot.led.off();
    }
    else if (state == OFF1) {
        selectedMode = EXECUTION; state = DONE; 
    }
}

void modeInstruction() {
    //DEBUG_PRINT("Instruction oui");
    // for (uint8_t i = 0; i < 8; i++) {  // 8 cycles, ns on veut 4hz, 4 cycles par seconde, 1 cycle est 250 ms. quand cèst pour 2 seconde, 8 cycle
    //     robot.led.green();
    //     _delay_ms(125); 

    //     robot.led.off();
    //     _delay_ms(125);  
    // }
    DEBUG_PRINT("Instruction oui");

    modeInstructionTicks = 0;
    blinkCount = 0;
    ledState = false;
    modeInstructionDone = false;

    while (!modeInstructionDone) {
        sleep_mode(); 
    }
 }

int main() {
    timer.setModeCTC(Timer::PRESCALE_64);
    timer.setOCRA(OCR1A_10MS);
    timer.startTimer();

    robot.button.init();
    sei();

    set_sleep_mode(SLEEP_MODE_IDLE);
    while (state != DONE)
        sleep_mode();

    switch (selectedMode) {
        case INSTRUCTION: modeInstruction(); break;
        case EXECUTION:   DEBUG_PRINT("Execution oui");   break;
        case RAPPORT:     DEBUG_PRINT("Rapport oui");     break;
    }

    while (true) {}
}


