#include "libstatique.hpp"
#include "lineSensor.hpp"

// LineSensor    lineSensor;
// Robot         robot;
// DistanceSensor distanceSensor;



// // Seuil ADC capteur de distance pour détecter un poteau (~20 cm, calibré)
// static const uint16_t POTEAU_THRESHOLD  = 100;

// // Angle de balayage (demi-arc gauche/droite) pour scanner le local
// static const uint16_t SPIN_SPEED   = 110;



// // ---------------------------------------------------------------------------
// // Enum état machine
// // ---------------------------------------------------------------------------
// enum class Action {
//     SCAN_FIRST_ROOM,
//     END,
// };


// // Joue la séquence d'alerte (même séquence que "Confirmation des instructions")
// void playConfirmSequence() {
//     for (uint8_t i = 0; i < N_CONFIRM_NOTES; i++) {
//         if (i > 0)
//             _delay_ms(NOTE_GAP_MS);
//         robot.sound.playSound(CONFIRM_NOTES[i]);
//         _delay_ms(NOTE_DURATION_MS);
//         robot.sound.stopSound();
//     }
// }

// // Fait clignoter la DEL en VERT à 4 Hz pendant 2 secondes (8 cycles)
// void blinkGreenClear() {
//     for (uint8_t i = 0; i < 8; i++) {
//         robot.led.green();
//         _delay_ms(125);
//         robot.led.off();
//         _delay_ms(125);
//     }
// }

// // ===========================================================================
// // Logique d'évacuation d'un poteau
// // ===========================================================================

// // Boucle d'alerte : joue la séquence, attend 2 s, re-vérifie; recommence
// // jusqu'à ce que le poteau soit retiré. Clignote en vert quand retiré.
// void evacuatePoteau() {
//     do {
//         playConfirmSequence();
//         _delay_ms(2000);
//     } while (distanceSensor.readADC() >= POTEAU_THRESHOLD);

//     blinkGreenClear();
//     }


// // ===========================================================================
// // Scan du local de travail
// // ===========================================================================

// // Temps pour un tour complet à SPIN_SPEED (à calibrer selon le robot)
// static const uint16_t FULL_ROTATION_MS = 4000;
// static const uint8_t  SCAN_STEP_MS     = 20;

// // Pivote à gauche sur 360°, évacue chaque poteau détecté au passage.
// // Le temps d'évacuation n'est pas compté dans la rotation.


// void scanRoom() {
//     uint16_t elapsed = 0;

//     while (elapsed < FULL_ROTATION_MS) {
//         if (distanceSensor.isObjectDetected(POTEAU_THRESHOLD)) {
//             robot.motor.stop();
//             evacuatePoteau();
//         } else {
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
    sensor.scanRoom(robot);
     
}