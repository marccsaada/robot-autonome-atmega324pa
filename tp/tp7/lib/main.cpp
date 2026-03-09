
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <button.hpp>

volatile bool gEtat = false;

ISR(INT0_vect)
{
    gEtat = !gEtat;
    _delay_ms ( 30 );

    EIFR |= (1 << INTF0) ;

}

void greenLight()
{
    DDRA |= (1 << PA0) | (1 << PA1);
    PORTA |= (1 << PA0);
    PORTA &= ~(1 << PA1);
}

int main()
{
    cli();
    DDRA |= (1 << PA0) | (1 << PA1);
    Button button(Button::RISING);
    button.init();
    while (true) {
        
        if (gEtat){

            greenLight();

        }
        sei();

    }
    return 0;
}

