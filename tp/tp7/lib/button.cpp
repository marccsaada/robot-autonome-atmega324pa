#include "button.hpp"


Button::Button(Button::Mode mode) : mode_(mode)
{
    cli();
    EICRA |= mode_;
    sei();
}

bool Button::isPressed()
{
    if (PIND & (1 << PD2))
    {
        _delay_ms(30);
        if (PIND & (1 << PD2))
        {
            return true;
        }
    }
    return false;
}
