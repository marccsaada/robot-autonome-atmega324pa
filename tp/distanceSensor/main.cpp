#include "libstatique.hpp"
#include "lineSensor.hpp"

LineSensor    lineSensor;
Robot         robot;
DistanceSensor distanceSensor;

// ---------------------------------------------------------------------------
// Notes MIDI pour la séquence "Confirmation des instructions"
// Ajuster ces valeurs pour correspondre aux notes de la section correspondante
// ---------------------------------------------------------------------------
static const uint8_t  CONFIRM_NOTES[]   = {69, 72, 76}; // A4, C5, E5
static const uint8_t  N_CONFIRM_NOTES   = 3;
static const uint16_t NOTE_DURATION_MS  = 250;
static const uint16_t NOTE_GAP_MS       = 125;

// Seuil ADC capteur de distance pour détecter un poteau (~20 cm, calibré)
static const uint16_t POTEAU_THRESHOLD  = 100;

// Angle de balayage (demi-arc gauche/droite) pour scanner le local
static const uint16_t SCAN_HALF_ANGLE   = 45;

// Vitesses de déplacement
static const uint8_t  FORWARD_SPEED     = 110;

// ---------------------------------------------------------------------------
// Enum état machine
// ---------------------------------------------------------------------------
enum class Action {
    SCAN_FIRST_ROOM,
    END,
};

// ===========================================================================
// Fonctions utilitaires – son / LED
// ===========================================================================

// Joue la séquence d'alerte (même séquence que "Confirmation des instructions")
void playConfirmSequence() {
    for (uint8_t i = 0; i < N_CONFIRM_NOTES; i++) {
        if (i > 0)
            _delay_ms(NOTE_GAP_MS);
        robot.sound.playSound(CONFIRM_NOTES[i]);
        _delay_ms(NOTE_DURATION_MS);
        robot.sound.stopSound();
    }
}

// Fait clignoter la DEL en VERT à 4 Hz pendant 2 secondes (8 cycles)
void blinkGreenClear() {
    for (uint8_t i = 0; i < 8; i++) {
        robot.led.green();
        _delay_ms(125);
        robot.led.off();
        _delay_ms(125);
    }
}

// ===========================================================================
// Fonctions utilitaires – déplacement
// ===========================================================================

// Virage proportionnel vers la gauche selon les capteurs de ligne
void turnLeft() {
    uint8_t count = lineSensor.offTrackAmount();

    uint8_t leftSpeed;
    const uint8_t rightSpeed = FORWARD_SPEED;

    if      (count == 5) leftSpeed = 0;
    else if (count == 4) leftSpeed = 0;
    else if (count == 3) leftSpeed = 80;
    else if (count == 2) leftSpeed = 80;
    else if (count == 1) leftSpeed = 85;
    else                 leftSpeed = FORWARD_SPEED;

    robot.motor.goForward(leftSpeed, rightSpeed);
}

// Suit la ligne en ajustant les vitesses proportionnellement
void followPath() {
    uint8_t leftSpeed  = FORWARD_SPEED;
    uint8_t rightSpeed = FORWARD_SPEED;

    if (lineSensor.robotOffTrackLeft())
        rightSpeed += lineSensor.offTrackAmount() * 15;
    else if (lineSensor.robotOffTrackRight())
        leftSpeed  += lineSensor.offTrackAmount() * 15;

    robot.motor.goForward(leftSpeed, rightSpeed);
}

// ===========================================================================
// Logique d'évacuation d'un poteau
// ===========================================================================

// Boucle d'alerte : joue la séquence, attend 2 s, re-vérifie; recommence
// jusqu'à ce que le poteau soit retiré. Clignote en vert quand retiré.
void evacuatePoteau() {
    do {
        playConfirmSequence();
        _delay_ms(2000);
    } while (distanceSensor.isObjectDetected(POTEAU_THRESHOLD));

    blinkGreenClear();
    }


// Vérifie si un poteau est détecté dans la direction actuelle et l'évacue.
void checkAndEvacuateIfNeeded() {
    if (distanceSensor.isObjectDetected(POTEAU_THRESHOLD)) {
        robot.motor.stop();
        evacuatePoteau();
    }
}

// ===========================================================================
// Scan du local de travail
// ===========================================================================

// Balaye le local : commence côté SUD-EST, puis pivote vers SUD, puis OUEST,
// puis revient au centre. Évacue chaque poteau trouvé.
void scanRoom() {
    // Positionnement initial côté SUD-EST
    robot.motor.spinRight(SCAN_HALF_ANGLE);
    checkAndEvacuateIfNeeded();

    // Balayage côté SUD (centre)
    robot.motor.spinLeft(SCAN_HALF_ANGLE);
    checkAndEvacuateIfNeeded();

    // Balayage côté OUEST
    robot.motor.spinLeft(SCAN_HALF_ANGLE);
    checkAndEvacuateIfNeeded();

    // Retour au centre
    robot.motor.spinRight(SCAN_HALF_ANGLE);
}

// ===========================================================================
// Machine à états – mouvement
// ===========================================================================

void movementLogic(Action& currentAction) {
    switch (currentAction) {

        // case Action::ENTER_FIRST_ROOM:
        //     // Avance jusqu'à l'entrée du local (ligne rouge / bump line)
        //     robot.motor.goForward(FORWARD_SPEED, FORWARD_SPEED);
        //     break;

        case Action::SCAN_FIRST_ROOM:
            robot.motor.stop();
            scanRoom();
            break;

        case Action::END:
            robot.motor.stop();
            robot.led.off();
            break;
    }
}

// ===========================================================================
// Machine à états – transitions
// ===========================================================================

void switchLogic(Action& currentAction) {
    switch (currentAction) {

        // case Action::ENTER_FIRST_ROOM:
        //     // Avance brièvement pour se positionner à l'entrée, puis scanne
        //     _delay_ms(300);
        //     currentAction = Action::SCAN_FIRST_ROOM;
        //     break;

        case Action::SCAN_FIRST_ROOM:
            currentAction = Action::END;
            break;

        case Action::END:
            break;
    }
}


int main() {
    // Action currentAction = Action::SCAN_FIRST_ROOM;

    // while (true) {
    //     movementLogic(currentAction);
    //     switchLogic(currentAction);
    // }

    while(true){
        scanRoom();  
    }
    // while(true){
    //     DistanceSensor interpreter;
    //     // interpreter.isObjectDetected(100);
    //     }
}
