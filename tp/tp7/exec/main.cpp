/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP7
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description du programme : Ce programme est un programme de tests pour toutes les classes de la librairie.
Les tests sont organisés en deux phases parallèles :
  Phase 1 — LED + Bouton + Timer1 CTC démarrent ensemble : la séquence de couleurs LED se déroule pendant
            que Timer1 compte 3 s et que l'interruption INT2 est armée.
  Phase 2 — Motor : chaque phase de mouvement est minutée par Timer1 (remplace _delay_ms), ce qui teste
            simultanément les moteurs et le mécanisme CTC du timer.


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
volatile uint8_t gTimerTarget = 3;
volatile bool    gTimerDone   = false;
volatile bool    gButtonEvent = false;

ISR(TIMER1_COMPA_vect) {
    gTimerCount++;
    DEBUG_PRINT("Timer1 tick : ", gTimerCount);

    if (gTimerCount >= gTimerTarget) {
        timer1.stopTimer();
        gTimerCount = 0;
        gTimerDone  = true;
    }
}

ISR(TIMER1_COMPB_vect) {}

// ISR INT2_vect déclenché par le bouton, change l'état de la LED et signale l'événement
ISR(INT2_vect) {
    _delay_ms(30);  // anti-rebond : délai de 30 ms
    gButtonEvent = true;
    if (button.isPressed())
        led.green();
    else
        led.red();

    EIFR  |=  (1 << INTF2);
    EIMSK |=  (1 << INT2);
}

// Lance une minuterie Timer1 CTC de n secondes
static void startTimerWait(uint8_t ticks) {
    gTimerDone   = false;
    gTimerCount  = 0;
    gTimerTarget = ticks;
    timer1.setModeCTC(Timer::Prescaler::PRESCALE_1024);
    timer1.setOCRA(7812);   // 8 MHz / 1024 / 7812 ≈ 1 Hz
    timer1.startTimer();
}

int main() {
    UART uart;
    uart.UART_Transmission("\n=== DEBUT DES TESTS ===\n");

    // Phase 1
    uart.UART_Transmission("\n[1+2+4] LED / Button / Timer1 — en parallele\n");

    // Armer le bouton (INT2) et demarrer Timer1 CTC 3 s
    gButtonEvent = false;
    button.init();
    startTimerWait(3);

    // --- Test LED (Timer1 tourne en arriere-plan) ---
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

    uart.UART_Transmission("[2] Button — appuyer le bouton\n");
    for (uint8_t i = 0; i < 10 && !gButtonEvent && !gTimerDone; i++) {
        led.red();  _delay_ms(250);
        led.off();  _delay_ms(250);
    }

    // Attendre Timer1 si pas encore fini (affiche amber)
    if (!gTimerDone) {
        while (!gTimerDone) { led.amber(); }
    }

    led.green(); _delay_ms(500); led.off(); _delay_ms(200);

    uart.UART_Transmission("[1] LED OK\n");
    uart.UART_Transmission(gButtonEvent
        ? "[2] Button OK\n"
        : "[2] Button TIMEOUT (aucun appui)\n");
    uart.UART_Transmission("[4] Timer1 CTC OK\n");

    // Phase 2
    uart.UART_Transmission("\n[3] Motor (Timer1 comme minuterie)\n");

    // goForward(150, 150) 2 s
    uart.UART_Transmission("  motor.goForward(150, 150)\n");
    startTimerWait(2);
    motor.goForward(150, 150);
    led.green();
    while (!gTimerDone) {}
    motor.stop(); led.off(); _delay_ms(300);

    // goBackward(150, 150) 2 s
    uart.UART_Transmission("  motor.goBackward(150, 150)\n");
    startTimerWait(2);
    motor.goBackward(150, 150);
    led.red();
    while (!gTimerDone) {}
    motor.stop(); led.off(); _delay_ms(300);

    // Virage gauche goForward(64, 200) 1 s
    uart.UART_Transmission("  motor.goForward(64, 200)\n");
    startTimerWait(1);
    motor.goForward(64, 200);
    led.green();
    while (!gTimerDone) {}

    // Virage droite goForward(200, 64) 1 s
    uart.UART_Transmission("  motor.goForward(200, 64)\n");
    startTimerWait(1);
    motor.goForward(200, 64);
    while (!gTimerDone) {}

    motor.stop(); led.off(); _delay_ms(300);
    uart.UART_Transmission("[3] Motor OK\n");

    uart.UART_Transmission("\n=== TESTS TERMINES ===\n");

    cli();
    EIMSK &= ~(1 << INT2);
    led.green();

    while (true) {}
}