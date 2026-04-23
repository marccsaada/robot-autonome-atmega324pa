/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Gestion des cinq capteurs de ligne du robot. Permet de savoir si le robot
              est centré, décalé, contre un mur ou s'il a rencontré une zone endommagée.
*/

#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include "memoire_24.h"
#include "eepromAdresses.hpp"

#define sensor1 PA0
#define sensor2 PA1
#define sensor3 PA2
#define sensor4 PA3
#define sensor5 PA4

class LineSensor {
public:
    LineSensor();
    ~LineSensor();
    
    bool robotMiddle();     
    bool offTrackLeft();   
    bool offTrackRight();  
    bool robotBumpLine();  
    bool isLeftWall();    
    bool isOnLeftLine();    
    bool isRightWall();   
    bool isOnRightLine();            
    bool sensors345(); 
    bool findDamage(EEPROMAddress addr);  
    void findObject(EEPROMAddress addr);

    uint8_t offTrackAmount(); 

    uint8_t getNDamage() const { return nDamage_; } 
    void incrementDamage() { nDamage_++; }
    void resetDamage() { nDamage_ = 0; previousDamageState_ = false; }

    bool getPreviousDamageState() const { return previousDamageState_; }
    void setPreviousDamageState(bool state) { previousDamageState_ = state; }

    void resetObjectCount() { nObjects_ = 0; previousDamageState_ = false;}
    uint8_t getNObjects() const { return nObjects_; }

    uint8_t getSensor(int index) const {
        if (index >= 0 && index < 5) {
            return (PINA & (1 << sensors_[index]));
        }
        return 0; 
    }
    
    
private:
    uint8_t sensors_[5] = {sensor1, sensor2, sensor3, sensor4, sensor5};
    uint8_t nDamage_ = 0; 
    uint8_t nObjects_ = 0; 

    bool     previousDamageState_ = false;
    bool     previousObjectState_ = false;
    bool     debouncing_          = false;
    uint16_t debounceStart_       = 0;
    Memoire24CXXX eeprom_;

};
