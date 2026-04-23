/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Point d'entrée principal. Sélection du mode (instruction / exécution / Rapport)
              par séquence DEL + bouton, puis redirige vers le mode approprié.
              Entièrement piloté par interruptions (Timer1 CTC + INT0). Aucune scrutation.
*/

/**
 * @file main.cpp
 * @brief Application entry point: mode selection via LED sequence + button, then dispatch.
 *
 * Timer 1 fires every 10 ms (CTC mode) to increment the global @c ticks counter.
 * INT0 (push-button) captures the user's mode choice during the LED colour sequence:
 *   - Button press during green → INSTRUCTION mode
 *   - Button press during red   → REPORT mode
 *   - No press (sequence finishes) → EXECUTION mode (default)
 */

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <libstatique.hpp>

static const uint16_t OCR1A_10MS     = 1249; ///< OCR1A value for a 10 ms CTC period at 8 MHz / 64.
static const uint16_t TICK_MS        = 10;   ///< Duration of one timer tick in milliseconds.
static const uint16_t COLOR_DURATION = 2000; ///< Time (ms) each LED colour is shown during selection.
static const uint16_t OFF_DURATION   = 1000; ///< Time (ms) the LED stays off between colours.
static const uint16_t INIT_WAIT      = 2000; ///< Settling delay (ms) before the selection sequence begins.

Robot            robot;         ///< Global robot instance owning all hardware subsystems.
volatile uint16_t ticks = 0;   ///< Incremented every 10 ms by Timer 1 COMPA ISR.

/** @brief Operating modes selectable at startup. */
enum Mode  { INSTRUCTION, ///< Receive and store an instruction program via serial.
             EXECUTION,   ///< Execute the stored program autonomously.
             REPORT       ///< Read EEPROM results and report them via LED/sound.
           };

/** @brief States of the LED-based mode-selection sequence. */
enum State { INIT,  ///< Waiting for the initial settling delay.
             GREEN, ///< LED is green; button press selects INSTRUCTION.
             OFF1,  ///< LED is off between green and red phases.
             RED,   ///< LED is red; button press selects REPORT.
             DONE   ///< Selection complete; proceed to execution.
           };

volatile State state        = INIT;      ///< Current phase of the selection sequence.
volatile Mode  selectedMode = EXECUTION; ///< Mode chosen by the user (default: EXECUTION).

/**
 * @brief Advances the LED mode-selection state machine based on elapsed ticks.
 *
 * Called from the main loop after each sleep cycle.  Transitions through INIT →
 * GREEN → OFF1 → RED → DONE, resetting @c ticks at each transition.
 */
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

/** @brief Timer 1 Compare-A ISR — increments the 10 ms tick counter. */
ISR(TIMER1_COMPA_vect) { ticks++; }

/** @brief Timer 1 Compare-B ISR — unused; present to satisfy the vector table. */
ISR(TIMER1_COMPB_vect) {}

/**
 * @brief INT0 ISR — captures the user's button press to select a mode.
 *
 * A press during the GREEN phase selects INSTRUCTION; a press during RED
 * selects REPORT.  In both cases the LED is turned off and the state
 * advances to DONE, ending the selection sequence immediately.
 */
ISR(INT0_vect) {
    if      (state == GREEN) { selectedMode = INSTRUCTION; state = DONE; robot.led.off(); }
    else if (state == RED)   { selectedMode = REPORT;      state = DONE; robot.led.off(); }
}

/**
 * @brief Application entry point.
 *
 * Configures Timer 1 for 10 ms CTC interrupts, enables INT0 for the button,
 * then sleeps in idle mode until the mode-selection sequence completes.
 * Afterwards it loads EEPROM operands and dispatches to the chosen mode handler.
 */
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
        case REPORT:     robot.runReport();       break;
    }
    while (true) {}
}
