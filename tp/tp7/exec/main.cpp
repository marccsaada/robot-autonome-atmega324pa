
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <button.hpp>
#include <LED.hpp>
#include <motor.hpp>
#include <debug.hpp>
#include <UART.hpp>


// volatile bool gEtat = false;
LED led(PORTA, PA1, PA0);
Button button(Button::ANY, Button::PB2_BUTTON);

ISR(INT2_vect)
{
    if (button.isPressed())
        led.green();
    else
        led.red();

    EIFR |= (1 << INTF2);

}

int main()
{
  button.init();
  button.enableInterupt();
  led.green();
  _delay_ms(6000);
  led.~LED();
  while(true) {
  } 

}