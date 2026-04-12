#define F_CPU 8000000UL
#include "libstatique.hpp"

LED led1(PORTB, PB0, PB2);

// Ici je prends en compte quon utilise le mode qui detecte les zones sombres et non les zones claires.

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
    if (((PINA & (1 << sensor1)) /*|| (PINA & (1 << sensor2))*/) && !(PINA & (1 << sensor5))) {
        return true;
    }
    return false;
}

bool LineSensor::offTrackRight() {
    if (!(PINA & (1 << sensor1)) && (/*(PINA & (1 << sensor4)) || */(PINA & (1 << sensor5)))) {
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

// LEFT OFFSET : (PINA & (1 << sensor1) && ( (PINA & (1 << sensor4)) || (PINA & (1 << sensor5)) )
// RIGHT OFFSET : (PINA & (1 << sensor5)) && ( (PINA & (1 << sensor1)) || (PINA & (1 << sensor2)) )
// NO   OFFSET : (offTrackAmount() < 3) && ( (PINA & (1 << sensor2) || PINA & (1 << sensor3) || PINA & (1 << sensor4) )

bool LineSensor::findDamage() {
    bool damagePresent = !robotBumpLine() && (
        ((PINA & (1 << sensor1)) && ( (PINA & (1 << sensor4)) || (PINA & (1 << sensor5)) )) // LEFT OFFSET
     || ((PINA & (1 << sensor5)) && ( (PINA & (1 << sensor1)) || (PINA & (1 << sensor2)) )) // RIGHT OFFSET
     || ((offTrackAmount() < 3) && ( (PINA & (1 << sensor2)) || (PINA & (1 << sensor3)) || (PINA & (1 << sensor4)) )) // NO OFFSET
    );

    if (damagePresent) { 
        previousDamageState_ = true;
        led1.red();
        return true;
    }
    else if (!damagePresent && previousDamageState_) { 
        previousDamageState_ = false;
        _delay_ms(200);
        if (!robotBumpLine()) { 
            nDamage_++;
            led1.off();
            return true;
        }
    }
    return false;
}

void LineSensor::findObject(uint16_t EEPROM_ADDR_LOCAL) {
    bool objectPresent = robotBumpLine();

    if (objectPresent) { 
        previousObjectState_ = true;
        led1.red();
        return;
    }
    else if (!objectPresent && previousObjectState_) { 
        previousObjectState_ = false;
        _delay_ms(200);
        if (!robotBumpLine()) { 
            nObjects_++;
            eeprom_.ecriture(EEPROM_ADDR_LOCAL, nObjects_);
        }
    }
    led1.off();
    return;
}

uint8_t LineSensor::offTrackAmount() {
    // uint8_t amount = 0;
    // for (uint8_t i = 0; i < 5; i++) {
    //     if (PINA & (1 << sensors_[i])) {
    //         amount++;
    //     }
    // }

    // if (amount == 5)
    //     amount = 4;
    return 1;
}