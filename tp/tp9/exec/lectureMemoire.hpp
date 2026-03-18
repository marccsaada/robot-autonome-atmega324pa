#pragma once
#include "memoire_24.h"
class Memoire{
public: 
    Memoire();
    void lectureDebutMemoire();
    uint8_t lireInstruction();
    uint8_t lireOperande(); 



private:
    Memoire24CXXX memoire_;
    uint16_t adresseCourante_;

};