#pragma once
// #include "libstatique.hpp"
#include "memoire_24.h"
class Memoire {
public: 
    Memoire() : adresseCourante_(2) {}
    uint16_t lectureDebutMemoire();
    uint8_t lireInstruction();
    uint8_t lireOperande(); 



private:
    Memoire24CXXX memoire_;
    uint16_t adresseCourante_;
    uint16_t getAdresse();
    void setAdresse(uint16_t adresse);


};