/*
Auteurs : Matthew Khouzam et Jérôme Collin
Description : Implémentation de la lecture séquentielle de la mémoire EEPROM externe.
*/

#include "memoryReader.hpp"

static const uint16_t INITIAL_ADDRESS   = 2;
static const uint8_t  HIGH_BYTE_SHIFT   = 8;
static const uint8_t  INSTRUCTION_SIZE  = 2;

Memory::Memory() {
    currentAddress_ = INITIAL_ADDRESS;
}

uint16_t Memory::readMemoryStart() {
    uint8_t high = 0, low  = 0;
    memory_.lecture(0, &high);
    memory_.lecture(1, &low);
    return ((uint16_t)high << HIGH_BYTE_SHIFT) | low;
}

uint16_t Memory::getSize() {
    return readMemoryStart();
}

uint8_t Memory::readInstruction() {
    uint8_t byte = 0;
    memory_.lecture(currentAddress_, &byte);
    currentAddress_ += INSTRUCTION_SIZE;
    return byte;
}

uint8_t Memory::readOperand() {
    uint8_t byte = 0;
    memory_.lecture(currentAddress_ - 1, &byte);
    return byte;
}

uint16_t Memory::getAddress() { return currentAddress_; }

void Memory::setAddress(uint16_t address) { currentAddress_ = address; }
