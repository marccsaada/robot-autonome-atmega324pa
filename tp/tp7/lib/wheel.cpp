#include "wheel.hpp"











































// // Helper: map OCR number to register address
// // 0 → OCR0A, 1 → OCR0B, 2 → OCR1A, 3 → OCR1B
// volatile uint8_t* Wheel::getOCRRegister() const {
//     switch (ocrType_) {
//         case OCR0A: return &OCR0A;
//         case OCR0B: return &OCR0B;
//         case OCR1A: return &OCR1A;
//         case OCR1B: return &OCR1B;
//         case OCR2A: return &OCR2A;
//         case OCR2B: return &OCR2B;
//         default: return &OCR0A;  // fallback
//     }
// }

// Wheel::Wheel(uint8_t OCR, uint8_t pinDirection)
//     : ocrType_(OCR), pinDirection_(pinDirection),
//       direction_(Direction::FORWARD), pwm_(0)
// {
//     // Configure direction pin as output on PORTD
//     DDRD |= (1 << pinDirection_);
//     // Initialize to FORWARD (pin high)
//     PORTD |= (1 << pinDirection_);
//     // Initialize PWM to 0%
//     setPwmPercentage(0);
// }

// void Wheel::changeDirection(Direction direction) {
//     direction_ = direction;
//     if (direction_ == Direction::FORWARD) {
//         PORTD |= (1 << pinDirection_);   // high = forward
//     } else {
//         PORTD &= ~(1 << pinDirection_);  // low = backward
//     }
// }

// void Wheel::changePwmPercentage(uint8_t percentage) {
//     setPwmPercentage(percentage);
// }

// uint8_t Wheel::getPwmPercentage() const {
//     return pwm_;
// }

// void Wheel::setPwmPercentage(uint8_t percentage) {
//     pwm_ = percentage;
//     // scale 0–100% to 0–255 for 8-bit timer
//     uint16_t duty = (uint16_t)percentage * 255 / 100;
//     *getOCRRegister() = duty;
// }
