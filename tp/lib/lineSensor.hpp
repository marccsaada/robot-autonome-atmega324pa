#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>

#define sensor1 PA0
#define sensor2 PA1
#define sensor3 PA2
#define sensor4 PA3
#define sensor5 PA4

class LineSensor {
public:
    LineSensor();
    ~LineSensor();
    
    bool robotMiddle();         // Robot perfectly in the middle
    bool offTrackLeft();   // Robot offset track to the left
    bool offTrackRight();  // Robot offset track to the right
    bool robotBumpLine();       // Robot bumping into the line
    bool isLeftWall();          // Robot is against the left wall
    bool isOnLeftLine();    
    bool isRightWall();         // Robot is against the right wall
    bool isOnRightLine();    
    bool findDamage();          // Robot found damage
    void findObject();

    uint8_t offTrackAmount();   // Amount of sensors that are off track (0-5)

    uint8_t getNDamage() const { return nDamage_; } // Getter for nDamage_
    void incrementDamage() { nDamage_++; }
    void resetDamage() { nDamage_= 0; }

    bool getPreviousDamageState() const { return previousDamageState_; }
    void setPreviousDamageState(bool state) { previousDamageState_ = state; }
    
    
private:
    uint8_t sensors_[5] = {sensor1, sensor2, sensor3, sensor4, sensor5};
    uint8_t nDamage_ = 0; // Number of times the robot has found damage
    uint8_t nObjects_ = 0; // Number of times the robot has found an object

    bool previousDamageState_ = false; // Previous state of damage detection
    bool previousObjectState_ = false;
};
