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
}

ISR(TIMER1_COMPB_vect) {}   // requis par setModeCTC

ISR(INT0_vect) {
    if (state == GREEN) {
        selectedMode = INSTRUCTION; state = DONE; robot.led.off();
    } else if (state == RED) {
        selectedMode = RAPPORT; state = DONE; robot.led.off();
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
        case INSTRUCTION: DEBUG_PRINT("Instruction oui"); break;
        case EXECUTION:   DEBUG_PRINT("Execution oui");   break;
        case RAPPORT:     DEBUG_PRINT("Rapport oui");     break;
    }

    while (true) {}
}


/*

    Description du programme: Ce programme implémente une séquence lumineuse sur une DEL bicolore à l'aide d'un bouton poussoir.
    Lorsqu'un bouton est enfoncé, un compteur s'incrémente à un rythme spécifique et contrôle la couleur
    et le clignotement de la DEL. La séquence varie en fonction de la durée de l'appui sur le bouton.


    Identification matérielle :
    Broche d'entrée (PD2)
    Broches de sortie (PB0, PB1)
    La DEL libre est connecté aux broches de sortie (soit PB0, PB1) par l'entremise d'un fil femelle-femelle.
    Un fil mâle-femelle est connecté aux GND et VCC du PORTA (entrée du circuit du «breadboard»)
    Un fil mâle-femelle est connecté à PD2 pour INT0 (sortie du circuit du «breadboard»)




#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t gMinuterieExpiree;
volatile uint8_t gBoutonPoussoir = 0;
volatile uint8_t compteur = 0;
volatile uint16_t dureeCompteur = 783;

const uint8_t DUREE_MAXIMALE_COMPTEUR = 120;
const uint16_t DELAI_VERT_INITIALE = 500;
const uint16_t DELAI_ETEINT_INITIALE = 2000;
const uint8_t DELAI_CLIGNOTE = 250;
const uint16_t DELAI_VERT_FINAL = 1000;
const uint8_t INCREMENT_PAR_2 = 2;


void partirMinuterie()
{
    TCNT1 = 0;

    OCR1A = dureeCompteur;

    TCCR1A |= (1 << COM1A0);

    TCCR1B |= (1 << CS10) | (1 << CS12) | (1 << WGM12);

    TCCR1C = 0;

    TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{

    compteur++;
}

ISR(TIMER1_COMPB_vect) {}

ISR(INT0_vect)
{
    _delay_ms(30);
    if (!(PIND & (1 << PD2)))
    {
        gBoutonPoussoir = 1;
        partirMinuterie();
    }
    else
    {
        gBoutonPoussoir = 0;
    }
}

void initialisation(void)
{

    cli();

    DDRD &= ~(1 << PD2);
    DDRB |= (1 << PB1) | (1 << PB0);

    EIMSK |= (1 << INT0);

    EICRA |= (1 << ISC00);

    sei();
}
void arreterMinuterie()
{
    TCCR1B = 0;
    TIMSK1 = 0;
    compteur = 0;
}


int main()
{

    initialisation();

    while (true)
    {
        if (gBoutonPoussoir)
        {
            do
            {

            } while ((gBoutonPoussoir != 0) & (compteur != DUREE_MAXIMALE_COMPTEUR));

            ouvrirVert();

            _delay_ms(DELAI_VERT_INITIALE);

            eteint();

            _delay_ms(DELAI_ETEINT_INITIALE);

            for (int i = 0; i <= compteur; i += INCREMENT_PAR_2)
            {
                ouvrirRouge();
                _delay_ms(DELAI_CLIGNOTE);
                eteint();
                _delay_ms(DELAI_CLIGNOTE);
            }
            ouvrirVert();
            _delay_ms(DELAI_VERT_FINAL);
            eteint();
            arreterMinuterie();
        }
    }
}
« Précédent12Suivant »(2-2/2)
Powered by Redmine © 2006-2024 Jean-Philippe Lang


*/
