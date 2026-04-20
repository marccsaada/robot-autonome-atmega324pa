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

static const uint16_t OCR1A_10MS     = 1249;
static const uint16_t TICK_MS        = 10;
static const uint16_t COLOR_DURATION = 2000;
static const uint16_t OFF_DURATION   = 1000;
static const uint16_t INIT_WAIT      = 2000;

Robot            robot;
volatile uint16_t ticks = 0;

enum Mode  { INSTRUCTION, EXECUTION, RAPPORT };
enum State { INIT, GREEN, OFF1, RED, DONE };

volatile State state        = INIT;
volatile Mode  selectedMode = EXECUTION;

void handleModeSelection() {
    switch (state) {
        case INIT:
            if (ticks >= INIT_WAIT / TICK_MS)
                { ticks = 0; state = GREEN; robot.led.green(); }
            break;
        case GREEN:
            if (ticks >= COLOR_DURATION / TICK_MS)
                { ticks = 0; state = OFF1; robot.led.off(); }
            break;
        case OFF1:
            if (ticks >= OFF_DURATION / TICK_MS)
                { ticks = 0; state = RED; robot.led.red(); }
            break;
        case RED:
            if (ticks >= COLOR_DURATION / TICK_MS)
                { ticks = 0; state = DONE; robot.led.off(); selectedMode = EXECUTION; }
            break;
        default: break;
    }
}

ISR(TIMER1_COMPA_vect) { ticks++; }
ISR(TIMER1_COMPB_vect) {}

ISR(INT0_vect) {
    if      (state == GREEN) { selectedMode = INSTRUCTION; state = DONE; robot.led.off(); }
    else if (state == RED)   { selectedMode = RAPPORT;     state = DONE; robot.led.off(); }
    // else if (state == OFF1)  { selectedMode = EXECUTION;   state = DONE; }
}
int main() {
    robot.timer.setModeCTC(Timer::PRESCALE_64);
    robot.timer.setOCRA(OCR1A_10MS);
    robot.timer.startTimer();
    robot.button.init();
    sei();
   
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (state != DONE) { sleep_mode(); handleModeSelection(); }
    robot.timer.stopTimer();
    ticks = 0;
    robot.readEepromOperands();
    switch (selectedMode) {
        case INSTRUCTION: robot.runInstruction(); break;
        case EXECUTION:   robot.runProject();     break;
        case RAPPORT:     robot.runRapport();     break;
    }
    while (true) {}
}
