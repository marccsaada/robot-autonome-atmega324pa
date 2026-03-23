#pragma once
#include "robot.hpp"
#include "lectureMemoire.hpp"

class Interpreteur {
public:
    Interpreteur();
    void sequenceDemarrage();
    void executer();

private:
    Robot   robot_;
    Memoire memoire_;

    uint16_t adresseBoucle_;
    uint8_t  compteurBoucle_;
    bool     enBoucle_;

    void executerInstruction(uint8_t instruction, uint8_t operande);
};