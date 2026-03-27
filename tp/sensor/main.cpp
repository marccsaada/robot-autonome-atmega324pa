/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Point d'entrée principal. Sélection du mode (instruction / exécution / rapport)
              par séquence DEL + bouton, puis dispatche vers le mode approprié.
              La temporisation utilise le Timer1 matériel en mode CTC (tick = 10 ms).
*/

#include <avr/interrupt.h>
#include <libstatique.hpp>


static const uint16_t COLOR_DURATION_MS = 2000;
static const uint16_t OFF_DURATION_MS   = 1000;
static const uint16_t INIT_WAIT_MS      = 2000;


// F_CPU = 8 MHz, prescaler = 64  →  tick = 64 / 8 000 000 * (1249+1) = 10 ms
static const uint16_t TICK_MS    = 10;
static const uint16_t OCR1A_10MS = 1249;
Robot robot;

Timer timer(Timer::TIMER1);
volatile uint16_t ticks = 0;

ISR(TIMER1_COMPA_vect) {
    ticks++;
}

// ISR vide pour éviter un reset quand OCIE1B est activé par setModeCTC
ISR(TIMER1_COMPB_vect) {}


static bool waitWithButton(uint16_t numTicks) {
    uint16_t start = ticks;
    while (static_cast<uint16_t>(ticks - start) < numTicks) {
        if (robot.button.isPressed())
            return true;
    }
    return false;
}

// Attend numTicks ticks sans surveiller le bouton
static void waitTicks(uint16_t numTicks) {
    uint16_t start = ticks;
    while (static_cast<uint16_t>(ticks - start) < numTicks) {}
}

enum Mode { INSTRUCTION, EXECUTION, RAPPORT };

Mode selectMode() {
    waitTicks(INIT_WAIT_MS / TICK_MS);

    robot.led.green();
    if (waitWithButton(COLOR_DURATION_MS / TICK_MS)) {
        robot.led.off();
        return INSTRUCTION;
    }

    robot.led.off();
    waitTicks(OFF_DURATION_MS / TICK_MS);

    robot.led.red();
    if (waitWithButton(COLOR_DURATION_MS / TICK_MS)) {
        robot.led.off();
        return RAPPORT;
    }

    robot.led.off();
    waitTicks(OFF_DURATION_MS / TICK_MS);

    
    return EXECUTION;
}


int main() {
    timer.setModeCTC(Timer::PRESCALE_64);
    timer.setOCRA(OCR1A_10MS);
    timer.startTimer();
    sei();

    Mode mode = selectMode();

    switch (mode) {
        case INSTRUCTION: DEBUG_PRINT("Instruction oui"); break;
        case EXECUTION:   DEBUG_PRINT("Execution oiu");   break;
        case RAPPORT:     DEBUG_PRINT("Rapport oui");     break;
    }

    while (true) {
    }
}
