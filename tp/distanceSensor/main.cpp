#include <avr/io.h>
#include <can.h>
#include "distanceSensor.hpp"
#include "LED.hpp"
#include "motor.hpp"
#include "UART.hpp"
#include "memoire_24.h"


// LED led(PORTA, PA4, PA5);

// can can_; 
// uint8_t personCounter_ = 0;

// static const uint8_t N_READINGS = 7;

// uint16_t readings[N_READINGS];


// Robot robot; 

// void sortArray(uint16_t array[], uint8_t size)
// {
//     for (uint8_t i = 1; i < size; i++)
//     {
//         uint16_t key = array[i];  
//         int8_t j = i - 1;

//         while (j >= 0 && array[j] > key)
//         {
//             array[j + 1] = array[j];
//             j--;
//         }

//         array[j + 1] = key;
//     }
// }

// uint16_t readADC() {
//     for (uint8_t i = 0; i < N_READINGS; i++) {
//         readings[i] = can_.lecture(7);

//     }

//     sortArray(readings, N_READINGS);

//     uint16_t median = readings[N_READINGS / 2];
//     return median;
//     return can_.lecture(7);
// }

// void playConfirmSequence(Robot& robot) {
//     for (uint8_t i = 0; i < 3 ; i++) {
//         _delay_ms(NOTE_GAP_MS);
//         robot.sound.playSound(70);
//         _delay_ms(NOTE_DURATION_MS);
//         robot.sound.stopSound();
//     }
// }

// void blinkGreenClear(Robot& robot) {
//     for (uint8_t i = 0; i < 8; i++) {
//         robot.led.green();
//         _delay_ms(125);
//         robot.led.off();
//         _delay_ms(125);
//     }
// }



// void evacuatePoteau(Robot& robot) {
//     do {
//         playConfirmSequence(robot);
//         _delay_ms(2000);
//     } while (readADC() >= POTEAU_THRESHOLD);

//     blinkGreenClear(robot);
// }

// void scanRoom(Robot& robot) { 
//     uint16_t elapsed = 0;

//     while (elapsed < FULL_ROTATION_MS) {
//         if (readADC() >= POTEAU_THRESHOLD) {
//             robot.led.red();
//             robot.motor.stop();
//             evacuatePoteau(robot);
//         } else {
//             robot.led.green();
//             robot.motor.spinLeftSpeed(SPIN_SPEED);
//             _delay_ms(SCAN_STEP_MS);
//             elapsed += SCAN_STEP_MS;
//         }
//     }
//     robot.motor.stop();
// }


int main() {
    DistanceSensor sensor; 
    Robot robot;
    UART uart;
    // Memoire24CXXX eeprom;
    // uint8_t count = 0;
    // eeprom.lecture(0x0000, &count);

      sensor.scanRoom(robot); 

    // uart.UART_Transmission(count);
    // _delay_ms(2000);
}