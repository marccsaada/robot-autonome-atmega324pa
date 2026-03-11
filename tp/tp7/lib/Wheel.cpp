#include "Wheel.hpp"

Wheel::Wheel(Timer &timer, uint8_t pinDirection, OCR ocrType)
    : timer_(&timer), pinDirection_(pinDirection), ocrType_(ocrType) {
     if (pinDirection == PD4) {
        DDRD |= (1 << PD4) | (1<<PD6);
     } 
     else if (pinDirection == PD5) {
        DDRD |= (1 << PD5) | (1<<PD7); 
     }
}   

void Wheel::goForward(uint8_t speedValue) {
    PORTD &= ~(1 << pinDirection_); // Set direction pin low for forward
    adjustSpeedValue(speedValue);
}

void Wheel::goBackward(uint8_t speedValue) {
    PORTD |= (1 << pinDirection_); // Set direction pin high for backward
    adjustSpeedValue(speedValue);
}

void Wheel::stop() {
    adjustSpeedValue(0); 
}

void Wheel::adjustSpeedValue(uint8_t speedValue) {
    if (ocrType_ == OCR::A) {
        timer_->setOCRA(speedValue);
    }
    else if (ocrType_ == OCR::B) {
        timer_->setOCRB(speedValue);
    }    
}