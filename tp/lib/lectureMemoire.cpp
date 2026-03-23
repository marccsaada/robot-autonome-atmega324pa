#include "lectureMemoire.hpp"

Memoire::Memoire() {
    adresseCourante_ = 2;
}

uint16_t Memoire::lectureDebutMemoire() {
    uint8_t haut = 0, bas = 0;
    memoire_.lecture(0, &haut);
    memoire_.lecture(1, &bas);
    return ((uint16_t)haut << 8) | bas;
}

uint16_t Memoire::getTaille() {
    return lectureDebutMemoire();
}

uint8_t Memoire::lireInstruction() {
    uint8_t octet = 0;
    memoire_.lecture(adresseCourante_, &octet);
    adresseCourante_ += 2;
    return octet;
}

uint8_t Memoire::lireOperande() {
    uint8_t octet = 0;
    memoire_.lecture(adresseCourante_ - 1, &octet);
    return octet;
}

uint16_t Memoire::getAdresse() { return adresseCourante_; }

void Memoire::setAdresse(uint16_t adresse) { adresseCourante_ = adresse; }






// void Memoire::lectureMemoire() {
//     Instruction inst;

//     uint16_t taille = getTaille();
//     adresseCourante_ = 2;

//     while (adresseCourante_ < taille) {

//         uint8_t opcode = lireOctet(adresseCourante_);
//         uint8_t operande = lireOctet(adresseCourante_ + 1);

//         inst.executer(opcode, operande);

//         if (opcode == 0xFF) { // fin
//             break;
//         }

//         adresseCourante_ += 2;
//     }
// }