#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

class Wheel
{
    public:
    Wheel(uint8_t OCR, uint8_t pinDirection);


    private:
}















// public:

//     // Rotation direction for the wheel
//     enum class Direction {
//         FORWARD,
//         BACKWARD
//     };

//     // Constructor: OCR is the register identifier (0=OCR0A, 1=OCR0B, 2=OCR1A, 3=OCR1B...)
//     // pinDirection is the PORTD pin number controlling direction
//     Wheel(uint8_t OCR, uint8_t pinDirection);


//     void changeDirection(Direction direction);
//     void changePwmPercentage(uint8_t percentage);

//     uint8_t getPwmPercentage() const;
//     void setPwmPercentage(uint8_t percentage);

// private:
//     uint8_t ocrType_;       // OCR register identifier
//     uint8_t pinDirection_;    // direction control pin on PORTD
//     Direction direction_;     // current direction state
//     uint8_t pwm_;             // PWM percentage (0–100)

//     // Helper: returns pointer to OCR register based on ocrType_
//     volatile uint8_t* getOCRRegister() const;
// };
