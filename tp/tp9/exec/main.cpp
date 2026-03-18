#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <libstatique.hpp>
#include "robot.hpp"
#include "lectureMemoire.hpp"
#include "interpreteur.hpp"

int main() {
    Robot robot;
    robot.avancer(150);
    _delay_ms(3000);
    robot.arreter();
}