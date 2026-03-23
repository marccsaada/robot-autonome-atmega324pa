#pragma once
#include "memoire_24.h"
#include <stdint.h>

class Memoire {
public:
    Memoire();
    uint16_t getTaille();
    uint8_t  lireInstruction();
    uint8_t  lireOperande();
    uint16_t getAdresse();
    void     setAdresse(uint16_t adresse);

private:
    Memoire24CXXX memoire_;
    uint16_t adresseCourante_;
    uint16_t lectureDebutMemoire();
};