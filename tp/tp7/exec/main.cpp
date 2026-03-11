
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <button.hpp>
#include <LED.hpp>
#include <motor.hpp>


// volatile bool gEtat = false;
// LED led(&PORTA, PA1, PA0);

// ISR(INT0_vect)
// {
//     gEtat = !gEtat;
//     if (gEtat)
//         led.red();
//     else
//         led.green();

//     _delay_ms ( 30 );

//     EIFR |= (1 << INTF0) ;

// }

int main()
{
  
    Motor moteur;
    // moteur.goForward(200, 200);
    // _delay_ms(4000);
    // moteur.goBackward(200, 200);
    // _delay_ms(4000);
    moteur.goForward(75, 75);
    _delay_ms(4000);
    moteur.stop();
    return 0;


}