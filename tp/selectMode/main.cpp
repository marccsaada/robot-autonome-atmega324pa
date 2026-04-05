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


Robot robot;
DistanceSensor distanceSensor;
Timer timer(Timer::TIMER1);

enum Mode  { INSTRUCTION, EXECUTION, RAPPORT };
enum State { INIT, GREEN, OFF1, RED, DONE };

volatile State    state        = INIT;
volatile uint16_t ticks        = 0;
volatile Mode     selectedMode = EXECUTION;

void handleModeSelection() {
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
}

ISR(TIMER1_COMPA_vect) {
    ticks++;
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

void printLine(UART& uart, const char* name, const int count, const char* type) {

    uart.UART_Transmission(name);

    if (count == 0) {
        uart.UART_Transmission("         oui\r\n");
    } else {
        uart.UART_Transmission("         non      ");
        uart.UART_Transmission(count + '0'); //count + '0' peut utiliser une fonction pour convert 
        uart.UART_Transmission(" ");
        uart.UART_Transmission(type);
        uart.UART_Transmission("\r\n");
    }
}

void modeRapport(){
   for (uint8_t i = 0; i < 8; i++) {
        robot.led.red();   
        _delay_ms(125);

        robot.led.off();
        _delay_ms(125);
    }

    UART uart;
    _delay_ms(3000); // laisse le temps de lancer serieViaUSB -l
    uart.UART_Transmission("Rapport de conformite\r\n\r\n");
    uart.UART_Transmission("Emplacement     conformite     detail\r\n");
    uart.UART_Transmission("--------------------------------------------------\r\n");

    printLine(uart, "Local A      ", distanceSensor.getObjectCounter(), "personne(s)"); // ask charger 
    printLine(uart, "Local B      ", distanceSensor.getObjectCounter(), "objet(s)");
    printLine(uart, "Local C      ", distanceSensor.getObjectCounter(), "objet(s)");
    printLine(uart, "Local D      ", distanceSensor.getObjectCounter(), "personne(s)");
    printLine(uart, "Couloir OUEST", distanceSensor.getObjectCounter(), "zones endommagée(s)");
    printLine(uart, "Couloir EST  ", distanceSensor.getObjectCounter(), "zones endommagée(s)");
}

void modeInstruction() {
    for (uint8_t i = 0; i < 8; i++) {
        robot.led.green();
        _delay_ms(125);

        robot.led.off();
        _delay_ms(125);
    }


    // debut de l'ecriture du bytecode en memoire externe 
    UART uart; 
    Memoire24CXXX memoire; 

    robot.led.red();

    uint8_t msb = uart.UART_Reception();
    uint8_t lsb = uart.UART_Reception();
    uint16_t taille = ((uint16_t)msb << 8) | lsb;

    memoire.ecriture(0, msb);
    _delay_ms(5);
    memoire.ecriture(1, lsb);
    _delay_ms(5);

    for (uint16_t i = 2; i < taille; i++) {
        uint8_t data = uart.UART_Reception();
        memoire.ecriture(i, data);
        _delay_ms(5);
    }
 
    // fin de l'ecriture du bytecode en memoire externe (il faut utiliser serieViaUSB pour envoyer le bytecode complet au robot)
    Interpreter interpreter;
    interpreter.confirm();
}

int main() {
    timer.setModeCTC(Timer::PRESCALE_64);
    timer.setOCRA(OCR1A_10MS);
    timer.startTimer();

    robot.button.init();
    sei();

    set_sleep_mode(SLEEP_MODE_IDLE);
    while (state != DONE) {
        sleep_mode();
        handleModeSelection();
    }

    switch (selectedMode) {
        case INSTRUCTION: modeInstruction(); break;
        case EXECUTION: { Interpreter interpreter; interpreter.execute(); break; }
        case RAPPORT: modeRapport();     break;
    }

    while (true) {}
}


