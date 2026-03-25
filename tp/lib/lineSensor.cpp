#pragma once
#define F_CPU 8000000UL
#include "libstatique.hpp"

#define sensor1 PA0
#define sensor2 PA1
#define sensor3 PA2
#define sensor4 PA3
#define sensor5 PA4

LineSensor::LineSensor() {
    DDRA |= (1 << sensor1) | (1 << sensor2) | (1 << sensor3) | (1 << sensor4) | (1 << sensor5);
};

LineSensor::~LineSensor() {};

bool LineSensor::isMiddle() {
    
}
bool LineSensor::isOffTrackLeft() {

}

bool LineSensor::isOffTrackRight() {

}



