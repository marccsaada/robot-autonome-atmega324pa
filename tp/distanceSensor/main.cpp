#include <avr/io.h>
#include <can.h>
#include "robot.hpp"
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

// Robot robot;

// int main() {


// for (uint8_t i = 0; i < robot.noteCount; i++) {
//     _delay_ms(125);
//     robot.sound.playSound(robot.note[i]);
//     _delay_ms(250);
//     robot.sound.stopSound();

// }

// }







//     DistanceSensor sensor; 
//     Robot robot;
//     UART uart;
//     uint16_t EEPROM_ADDR_BASE = 10;
//     static const uint8_t NB_ZONES = 6;

//     Memoire24CXXX eeprom;
//     for (uint8_t i = 0; i < 6; i++) {
//     eeprom.ecriture(10 + i, 0);
//     _delay_ms(5);
// }

//     sensor.scanRoom(robot, LEFT,10);
//     _delay_ms(2000);
//      sensor.scanRoom(robot, RIGHT,11);
//     _delay_ms(2000);
//      sensor.scanRoom(robot, LEFT,12);    
//     _delay_ms(2000);
//      sensor.scanRoom(robot, RIGHT,13);
//     _delay_ms(2000);
//      sensor.scanRoom(robot, LEFT,14);
//     _delay_ms(2000);

// Robot robot; 
// while(true){
// robot.motor.goBackward(255,255);
// }