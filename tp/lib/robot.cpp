// /*
// Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
// Travail : TP9
// Section # : 05
// Équipe # : 107111
// Correcteur : Abdul-wahab Chaarani

// Description : Implémentation de l'interface unifiée du robot (moteurs, LED, son).
// */

#include <libstatique.hpp>
#include <robot.hpp>

void Robot::wait(uint16_t multiplicator) {
    for (uint16_t i = 0; i < multiplicator; ++i) {
        _delay_ms(25); 
    }
}

void Robot::playEepromNotes() {

    Memory  mem;
    uint16_t size  = mem.getSize();
    uint8_t  notes[3];
    uint8_t  found = 0;

    for (uint16_t i = 0; i < size && found < 3; i += 2) {
        uint8_t instr   = mem.readInstruction();
        uint8_t operand = mem.readOperand();
        if (instr == SGO)
            notes[found++] = operand;
    }

    for (uint8_t i = 0; i < found; i++) {
        if (i > 0)
            _delay_ms(NOTE_GAP_MS);
        sound.playSound(notes[i]);
        _delay_ms(NOTE_DURATION_MS);
        sound.stopSound();
    }
}