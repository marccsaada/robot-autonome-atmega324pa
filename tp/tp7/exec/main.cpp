/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP7
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description du programme : Ce programme est un programme de tests pour toutes les classes de la librairie. Il teste successivement : l'affichage 
des couleurs de la LED(rouge,vert, ambre, éteint), la détection d'un appui sur le bouton externe (via INT2),le contrôle des moteurs (avance, 
recule, virages) via PWM sur Timer2, et finalement le Timer1 en mode CTC pour mesurer un délai de 3 secondes. Les résultats sont transmis par UART.


* ========== I/O IDENTIFICATION (CONNEXIONS SUR LE ROBOT) =============
* 
* ┌───────────────────────────────────────────────────────────────────┐
* │                    ATmega324PA - Pinout                           │
* ├───────────────────────────────────────────────────────────────────┤
* │                                                                   │
* │   PORTA            PORTB           PORTC          PORTD           │
* │  ┌───────┐        ┌──────┐         ┌──────┐       ┌──────┐        │   
* │  │ PA0 ●-│→ LED+  │PB0 ○ │         │PC0 ○ │       │PD0 ●-│→ RXD   │
* │  │ PA1 ●-│→ LED-  │PB1 ○ │         │PC1 ○ │       │PD1 ●-│→ TXD   │
* │  │ PA2 ○ │        │PB2 ●-│→B-INT2  │PC2 ○ │       │PD2 ○ │        │
* │  │ PA3 ○ │        │PB3 ○ │         │PC3 ○ │       │PD3 ○ │        │
* │  │ PA4 ○ │        │PB4 ○ │         │PC4 ○ │       │PD4 ●-│→ DIR-R │
* │  │ PA5 ○ │        │PB5 ○ │         │PC5 ○ │       │PD5 ●-│→ DIR-L │
* │  │ PA6 ○ │        │PB6 ○ │         │PC6 ○ │       │PD6 ●-│→OC2A-R │
* │  │ PA7 ○ │        │PB7 ○ │         │PC7 ○ │       │PD7 ●-│→OC2B-L │
* │  └───────┘        └──────┘         └──────┘       └──────┘        │
* │                                                                   │
* │  ● = Utilisé          ○ = Non utilisé                             │
* └───────────────────────────────────────────────────────────────────┘
* LEGENDE :
* RXD/TXD : UART (debug, 2400 baud)
* B-INT2  : bouton externe (INT2 sur PB2)
* LED+    : anode LED verte  (PA0)
* LED-    : anode LED rouge  (PA1)
* DIR-R   : direction roue droite  (PD4, Timer2 OCR2A)
* DIR-L   : direction roue gauche  (PD5, Timer2 OCR2B)
* OC2A-R  : PWM roue droite  (PD6, OC2A)
* OC2B-L  : PWM roue gauche  (PD7, OC2B)
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <libstatique.hpp>

LED    led(PORTA, PA1, PA0);
Button button(Button::ANY, Button::PB2_BUTTON);
Timer  timer1(Timer::TIMER1);
Motor  motor;

volatile uint8_t gTimerCount  = 0;
volatile bool    gTimerDone   = false;
volatile bool    gButtonEvent = false;

// ISR TIMER1_COMPA_vect — compte les ticks du timer pour mesurer 3 secondes
ISR(TIMER1_COMPA_vect)
{
    gTimerCount++;
    DEBUG_PRINT("Timer1 tick : ", gTimerCount);

    if (gTimerCount >= 3)   // 3 x 1s = 3 secondes
    {
        timer1.stopTimer();
        gTimerCount = 0;
        gTimerDone  = true;
    }
}

// ISR TIMER1_COMPB_vect — vide, car nous n'utilisons pas ce canal de comparaison pour les interruptions
ISR(TIMER1_COMPB_vect) {}

// ISR INT2_vect — déclenché par le bouton, change l'état de la LED et signale l'événement
ISR(INT2_vect)
{
    EIMSK &= ~(1 << INT2);  // anti-rebond : bloque re-declenchement
    EIFR  |=  (1 << INTF2);

    // _delay_ms(30);  // anti-rebond : délai de 30 ms
    gButtonEvent = true;
    if (button.isPressed())
        led.green();
    else
        led.red();

    EIFR  |=  (1 << INTF2);
    EIMSK |=  (1 << INT2);
}

int main() {
    UART uart;
    uart.UART_Transmission("\n=== DEBUT DES TESTS ===\n");

    // Tests pour la LED
    uart.UART_Transmission("\n[1] LED\n");

    uart.UART_Transmission("  led.red()\n");
    led.red();
    _delay_ms(800);

    uart.UART_Transmission("  led.green()\n");
    led.green();
    _delay_ms(800);

    uart.UART_Transmission("  led.amber()\n");
    for (uint8_t i = 0; i < 50; i++)
        led.amber();

    uart.UART_Transmission("  led.off()\n");
    led.off();
    _delay_ms(500);

    // Tests pour le bouton externe sur PB2
    uart.UART_Transmission("\n[2] Button — appuyer le bouton\n");

    gButtonEvent = false;
    button.init();

    // Signal d'attente : LED clignote rouge pendant 5 s
    for (uint8_t i = 0; i < 10; i++) {
        led.red();   _delay_ms(250);
        led.off();   _delay_ms(250);
        if (gButtonEvent) break;
    }

    if (gButtonEvent)
        uart.UART_Transmission("[2] Button OK\n");
    else
        uart.UART_Transmission("[2] Button TIMEOUT (aucun appui)\n");

    led.off();
    _delay_ms(500);

    // Tests pour les classes Motor et Wheel
    uart.UART_Transmission("\n[3] Motor\n");

    uart.UART_Transmission("  motor.goForward(150, 150)\n");
    led.green();
    motor.goForward(150, 150);
    _delay_ms(2000);

    uart.UART_Transmission("  motor.stop()\n");
    motor.stop();
    led.off();
    _delay_ms(500);

    uart.UART_Transmission("  motor.goBackward(150, 150)\n");
    led.red();
    motor.goBackward(150, 150);
    _delay_ms(2000);

    uart.UART_Transmission("  motor.stop()\n");
    motor.stop();
    led.off();
    _delay_ms(500);

    uart.UART_Transmission("  motor.goForward(64, 200)\n");
    led.green();
    motor.goForward(64, 200);
    _delay_ms(1500);

    uart.UART_Transmission("  motor.goForward(200, 64)\n");
    motor.goForward(200, 64);
    _delay_ms(1500);

    uart.UART_Transmission("  stop\n");
    motor.stop();
    led.off();
    _delay_ms(500);
    uart.UART_Transmission("[3] Motor OK\n");

    // Tests pour le timer en mode CTC
    uart.UART_Transmission("\n[4] Timer1 CTC 3 s\n");

    gTimerDone  = false;
    gTimerCount = 0;
    timer1.setModeCTC(Timer::Prescaler::PRESCALE_1024);
    timer1.setOCRA(7812);
    timer1.startTimer();

    led.amber();
    while (!gTimerDone) {
        led.amber();
    }

    led.green();
    _delay_ms(500);
    led.off();


    uart.UART_Transmission("\n=== TESTS TERMINES ===\n");

    cli();
    EIMSK &= ~(1 << INT2);
    led.green();

    while (true) {}
}