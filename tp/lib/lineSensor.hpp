#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
class LineSensor {
public:
    LineSensor();
    ~LineSensor();
    
    bool isMiddle();
    bool isOffTrackLeft();
    bool isOffTrackRight();
    bool isOffTrack();

private:
    

};
