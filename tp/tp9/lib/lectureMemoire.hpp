/*
Auteurs : Matthew Khouzam et Jerome Collin
Description : Implémentation de la lecture séquentielle de la mémoire EEPROM externe.
*/

#pragma once
#include "memoire_24.h"
#include <stdint.h>

/**
 * @file lectureMemoire.hpp
 * @brief Sequential read interface for the external EEPROM memory.
 */

/**
 * @brief Provides sequential read access to the external EEPROM.
 *
 * Reads the program size from the first two bytes, then steps through
 * instruction/operand pairs starting at address 2.
 */
class Memory {
public:
    /** @brief Initializes the memory and sets the read cursor to address 2. */
    Memory();

    /**
     * @brief Returns the total number of bytes in the stored program.
     * @return Program size read from addresses 0–1.
     */
    uint16_t getSize();

    /**
     * @brief Reads the next instruction byte and advances the cursor.
     * @return Instruction byte at the current address.
     */
    uint8_t  readInstruction();

    /**
     * @brief Reads the next operand byte and advances the cursor.
     * @return Operand byte at the current address.
     */
    uint8_t  readOperand();

    /**
     * @brief Returns the current read cursor address.
     * @return Current address in EEPROM.
     */
    uint16_t getAddress();

    /**
     * @brief Sets the read cursor to an arbitrary address.
     * @param address Target EEPROM address.
     */
    void     setAddress(uint16_t address);

private:
    Memoire24CXXX memory_;
    uint16_t currentAddress_;

    /**
     * @brief Reads the program start address stored at address 0 (2 bytes, big-endian).
     * @return Program size / start offset.
     */
    uint16_t readMemoryStart();
};