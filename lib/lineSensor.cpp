/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Implémentation des capteurs de ligne. Lit les entrées du port A, gère le
              comptage des zones endommagées et des objets, et écrit les résultats en EEPROM.
*/

#define F_CPU 8000000UL
#include "libstatique.hpp"

static const uint8_t  SENSOR_COUNT         = 5;
static const uint8_t  MAX_OFFTRACK_AMOUNT  = 4;
static const uint16_t DAMAGE_DEBOUNCE_MS   = 30;
static const uint16_t OBJECT_DEBOUNCE_MS   = 200;

extern volatile uint16_t ticks;

LED led1(PORTB, PB0, PB2);

LineSensor::LineSensor() {
    DDRA &= ~((1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3) | (1 << PA4));
};

LineSensor::~LineSensor() {};

bool LineSensor::robotMiddle() {
    if (!((PINA & (1 << sensor1)) || (PINA & (1 << sensor2)) || (PINA & (1 << sensor3)) || (PINA & (1 << sensor4)) || (PINA & (1 << sensor5)))) {
        return true;
    }
    return false;
}

bool LineSensor::offTrackLeft() {
    if ((PINA & (1 << sensor1))) {
        return true;
    }
    return false;
}

bool LineSensor::offTrackRight() {
    if ((PINA & (1 << sensor5))) {
        return true;
    }
    return false;
}

bool LineSensor::robotBumpLine() {
    if ((PINA & (1 << sensor1)) && (PINA & (1 << sensor2)) && (PINA & (1 << sensor3)) && (PINA & (1 << sensor4)) && (PINA & (1 << sensor5))) {
        return true;
    }
    return false;
}

bool LineSensor::isLeftWall() {
    if ((PINA & (1 << sensor1))) {
        return true;
    }
    return false;
}

bool LineSensor::isOnLeftLine() {
    if ((PINA & (1 << sensor1)) && (PINA & (1 << sensor2))) {
        return true;
    }
    return false;
}

bool LineSensor::isRightWall() {
    if ((PINA & (1 << sensor5))) {
        return true;
    }
    return false;
}
bool LineSensor::isOnRightLine() {
    if ((PINA & (1 << sensor4)) && (PINA & (1 << sensor5))) {
        return true;
    }
    return false;
}

bool LineSensor::findDamage(EEPROMAddress addr) { 
    bool damagePresent = !robotBumpLine() && (
        ((PINA & (1 << sensor1)) && ( (PINA & (1 << sensor4)) || (PINA & (1 << sensor5)) )) 
     || ((PINA & (1 << sensor5)) && ( (PINA & (1 << sensor1)) || (PINA & (1 << sensor2)) )) 
     || ((offTrackAmount() < 3) && ( (PINA & (1 << sensor2)) || (PINA & (1 << sensor3)) || (PINA & (1 << sensor4)) )) 
    );

    if (damagePresent) { 
        previousDamageState_ = true;
        led1.red();
        return true;
    }
    else if (!damagePresent && previousDamageState_) { 
        previousDamageState_ = false;
        _delay_ms(DAMAGE_DEBOUNCE_MS);
        if (!robotBumpLine()) {
            nDamage_++;
            led1.off();
            eeprom_.ecriture(addr, nDamage_); 
            return true;
        }
    }

    previousDamageState_ = false;
    led1.off();
        
    return false;
}


void LineSensor::findObject(EEPROMAddress addr) { 
    bool objectPresent = offTrackAmount() >= 1;

    if (objectPresent) { 
        previousObjectState_ = true;
        led1.red();
        return;
    }
    else if (!objectPresent && previousObjectState_) { 
        previousObjectState_ = false;
        _delay_ms(OBJECT_DEBOUNCE_MS);
        if (!robotBumpLine()) {
            nObjects_++;
        }
        return;
    }
    led1.off();
    return;
}

uint8_t LineSensor::offTrackAmount() {
    uint8_t amount = 0;
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        if (PINA & (1 << sensors_[i])) {
            amount++;
        }
    }

    if (amount == SENSOR_COUNT)
        amount = MAX_OFFTRACK_AMOUNT;
    return amount;
}



