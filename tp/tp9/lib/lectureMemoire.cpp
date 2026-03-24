/*
Auteurs : Matthew Khouzam et Jerome Collin
Description : Implémentation de la lecture séquentielle de la mémoire EEPROM externe.
*/

#include "lectureMemoire.hpp"

Memory::Memory() {
    currentAddress_ = 2;
}

uint16_t Memory::readMemoryStart() {
    uint8_t high = 0, low  = 0;
    memory_.lecture(0, &high);
    memory_.lecture(1, &low);
    return ((uint16_t)high << 8) | low;
}

uint16_t Memory::getSize() {
    return readMemoryStart();
}

uint8_t Memory::readInstruction() {
    uint8_t octet = 0;
    memory_.lecture(currentAddress_, &octet);
    currentAddress_ += 2;
    return octet;
}

uint8_t Memory::readOperand() {
    uint8_t octet = 0;
    memory_.lecture(currentAddress_ - 1, &octet);
    return octet;
}

uint16_t Memory::getAddress() { return currentAddress_; }

void Memory::setAddress(uint16_t address) { currentAddress_ = address; }

