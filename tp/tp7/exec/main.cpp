
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
//   DDRA |= (1 << PA0) | (1 << PA1);
  LED led(PORTA, PA0, PA1);
  led.green();
  while(true) {
  } 
}