#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

class LED {
public:
    LED(volatile uint8_t &port, uint8_t pinIn, uint8_t pinOut);
    
    void off();
    void red();
    void green();
    void amber();

private:
        void setDDR();
        volatile uint8_t* port_;
        volatile uint8_t* ddr_;
        uint8_t pinIn_, pinOut_;
};