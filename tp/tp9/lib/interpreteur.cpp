#include "interpreteur.hpp"
#include "debug.hpp"
#include <util/delay.h>

// Codes binaires des instructions
const uint8_t DBT = 0x01;
const uint8_t ATT = 0x02;
const uint8_t DAL = 0x44;
const uint8_t DET = 0x45;
const uint8_t SGO = 0x48;
const uint8_t SAR = 0x09;
const uint8_t MAR_0 = 0x60;
const uint8_t MAR_1 = 0x61;
const uint8_t MAV = 0x62;
const uint8_t MRE = 0x63;
const uint8_t TRD = 0x64;
const uint8_t TRG = 0x65;
const uint8_t DBC = 0xC0;
const uint8_t FBC = 0xC1;
const uint8_t FIN = 0xFF;

Interpreteur::Interpreteur()
    : adresseBoucle_(0),
      compteurBoucle_(0),
      enBoucle_(false)
{}

void Interpreteur::sequenceDemarrage() {
    // 2x rouge, 2x vert comme demandé dans le TP
    for (uint8_t i = 0; i < 2; i++) {
        robot_.allumerRouge();
        _delay_ms(300);
        robot_.eteindreDEL();
        _delay_ms(300);
    }
    for (uint8_t i = 0; i < 2; i++) {
        robot_.allumerVert();
        _delay_ms(300);
        robot_.eteindreDEL();
        _delay_ms(300);
    }
}

void Interpreteur::executer() {
    uint16_t taille = memoire_.getTaille();
    bool debutTrouve = false;
    bool fini = false;

    while (!fini) {
        uint8_t instruction = memoire_.lireInstruction();
        uint8_t operande    = memoire_.lireOperande();

        // On ignore tout avant dbt
        if (!debutTrouve) {
            if (instruction == DBT)
                debutTrouve = true;
            else
                continue;
        }

        // On arrête si on dépasse la taille
        if (memoire_.getAdresse() > taille) {
            fini = true;
            continue;
        }

        executerInstruction(instruction, operande);

        if (instruction == FIN)
            fini = true;
    }

    robot_.arreter();
}

void Interpreteur::executerInstruction(uint8_t instruction, uint8_t operande) {
    switch (instruction) {

        case ATT:
            DEBUG_PRINT("Attendre pendant ", operande * 25);
            robot_.attendre(operande);
            break;

        case DAL:
            DEBUG_PRINT("Allumer DEL ", operande);
            if (operande == 1)      robot_.allumerVert();
            else if (operande == 2) robot_.allumerRouge();
            break;

        case DET:
            DEBUG_PRINT("Eteindre DEL");
            robot_.eteindreDEL();
            break;

        case SGO:
            DEBUG_PRINT("Jouer son ", operande);
            robot_.jouerSon(operande);
            break;

        case SAR:
            DEBUG_PRINT("Arreter son");
            robot_.arreterSon();
            break;

        case MAR_0:
        case MAR_1:
            DEBUG_PRINT("Arreter robot");
            robot_.arreter();
            break;

        case MAV:
            DEBUG_PRINT("Avancer de ", operande);
            robot_.avancer(operande);
            break;

        case MRE:
            DEBUG_PRINT("Reculer de ", operande);
            robot_.reculer(operande);
            break;

        case TRD:
            DEBUG_PRINT("Tourner à droite de ", operande);
            robot_.tournerDroite(operande);
            break;

        case TRG:
            DEBUG_PRINT("Tourner à gauche de ", operande);
            robot_.tournerGauche(operande);
            break;

        case DBC:
            adresseBoucle_  = memoire_.getAdresse();
            compteurBoucle_ = operande;
            enBoucle_       = true;
            break;

        case FBC:
            if (enBoucle_ && compteurBoucle_ > 0) {
                memoire_.setAdresse(adresseBoucle_);
                compteurBoucle_--;
            } else {
                enBoucle_ = false;
            }
            break;

        case FIN:
            robot_.arreter();
            break;

        default:
            break; 
    }
}