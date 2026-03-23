#include <avr/io.h>
#include "timer.hpp"

class Son {

public:
    Son();
    void jouer(uint8_t note);
    void arreter();

private:
    Timer timer_;
    uint16_t calculerOCR_(float frequence);
};

// #endif
