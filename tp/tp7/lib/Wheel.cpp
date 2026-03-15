#include "Wheel.hpp"
#include "debug.hpp"

Wheel::Wheel(Timer &timer, uint8_t pinDirection, OCR ocrType)
    : timer_(&timer), pinDirection_(pinDirection), ocrType_(ocrType) {
    DEBUG_PRINT(ocrType_ == OCR::A ? "Init Left wheel\n" : "Init Right wheel\n");
    DEBUG_PRINT("pinDirection : ", static_cast<uint16_t>(pinDirection));
    if (pinDirection == PD4) {
        DDRD |= (1 << PD4) | (1<<PD6);
    } else if (pinDirection == PD5) {
        DDRD |= (1 << PD5) | (1<<PD7); 
    }
}   

void Wheel::goForward(uint8_t speedValue) {
    DEBUG_PRINT(ocrType_ == OCR::A ? "Left wheel goForward with speedValue : " 
                                    : "Right wheel goForward with speedValue : ", static_cast<uint16_t>(speedValue));

    PORTD &= ~(1 << pinDirection_); // Set direction pin low for forward
    adjustSpeedValue(speedValue);
}

void Wheel::goBackward(uint8_t speedValue) {
    DEBUG_PRINT("Wheel goBackward with speedValue : ", static_cast<uint16_t>(speedValue));

    PORTD |= (1 << pinDirection_); // Set direction pin high for backward
    adjustSpeedValue(speedValue);
}

void Wheel::stop() {
    DEBUG_PRINT("Wheel stop");
    adjustSpeedValue(0); 
}

void Wheel::adjustSpeedValue(uint8_t speedValue) {
    DEBUG_PRINT(ocrType_ == OCR::A ? "Left wheel adjustSpeedValue with speedValue : "
                                : "Right wheel adjustSpeedValue with speedValue : ", static_cast<uint16_t>(speedValue));

    if (ocrType_ == OCR::A) {
        timer_->setOCRA(speedValue);
    }
    else if (ocrType_ == OCR::B) {
        timer_->setOCRB(speedValue);
    }    
}