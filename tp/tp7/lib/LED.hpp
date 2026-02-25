#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

class LED {

public:

    LED(uint8_t* port, uint8_t pinIn, uint8_t pinOut);

    ~LED();

    void off();

    void red();

    void green();

    void amber();

private:
        uint8_t* port_;
        uint8_t pinIn_, pinOut_;
};