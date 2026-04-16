/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Point d'entrée principal. Sélection du mode (instruction / exécution / rapport)
              par séquence DEL + bouton, puis dispatche vers le mode approprié.
              Entièrement piloté par interruptions (Timer1 CTC + INT0). Aucune scrutation.
*/

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <libstatique.hpp>

// ── Constantes de sélection de mode ──────────────────────────────────────────

static const uint16_t OCR1A_10MS     = 1249;
static const uint16_t TICK_MS        = 10;
static const uint16_t COLOR_DURATION = 2000;
static const uint16_t OFF_DURATION   = 1000;
static const uint16_t INIT_WAIT      = 2000;

// ── Globaux ───────────────────────────────────────────────────────────────────

Robot            robot;
volatile uint16_t ticks = 0;

// ── Machine à états de sélection ─────────────────────────────────────────────

enum Mode  { INSTRUCTION, EXECUTION, RAPPORT };
enum State { INIT, GREEN, OFF1, RED, DONE };

volatile State state        = INIT;
volatile Mode  selectedMode = EXECUTION;

void handleModeSelection() {
    switch (state) {
        case INIT:
            if (ticks >= INIT_WAIT / TICK_MS)
                { ticks = 0; state = GREEN; robot.led.green(); }
            break;
        case GREEN:
            if (ticks >= COLOR_DURATION / TICK_MS)
                { ticks = 0; state = OFF1; robot.led.off(); }
            break;
        case OFF1:
            if (ticks >= OFF_DURATION / TICK_MS)
                { ticks = 0; state = RED; robot.led.red(); }
            break;
        case RED:
            if (ticks >= COLOR_DURATION / TICK_MS)
                { ticks = 0; state = DONE; robot.led.off(); selectedMode = EXECUTION; }
            break;
        default: break;
    }
}

// ── ISR ───────────────────────────────────────────────────────────────────────

ISR(TIMER1_COMPA_vect) { ticks++; }
ISR(TIMER1_COMPB_vect) {}

ISR(INT0_vect) {
    if      (state == GREEN) { selectedMode = INSTRUCTION; state = DONE; robot.led.off(); }
    else if (state == RED)   { selectedMode = RAPPORT;     state = DONE; robot.led.off(); }
    else if (state == OFF1)  { selectedMode = EXECUTION;   state = DONE; }
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    robot.timer.setModeCTC(Timer::PRESCALE_64);
    robot.timer.setOCRA(OCR1A_10MS);
    robot.timer.startTimer();
    robot.button.init();
    sei();

    set_sleep_mode(SLEEP_MODE_IDLE);
    while (state != DONE) { sleep_mode(); handleModeSelection(); }
    robot.timer.stopTimer();
    ticks = 0;

void followLeftWall() {
    uint8_t leftWheelSpeed = LEFT_DEFAULT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_DEFAULT_SPEED;
    
    if (!robot.lineSensor.isLeftWall()) { 
        leftWheelSpeed -= robot.lineSensor.offTrackAmount() * 10;//30;
        rightWheelSpeed += robot.lineSensor.offTrackAmount() * 2;//30;
    }
    else if (robot.lineSensor.isLeftWall()) { 
        rightWheelSpeed = RIGHT_DEFAULT_SPEED;
        leftWheelSpeed = LEFT_DEFAULT_SPEED;
    }
    robot.motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void followRightWall() {
    uint8_t leftWheelSpeed;
    uint8_t rightWheelSpeed;

    bool s3 = robot.lineSensor.getSensor(2); //s3
    bool s4 = robot.lineSensor.getSensor(3); // sensor4
    bool s5 = robot.lineSensor.getSensor(4); // sensor5

    if (s4 && s5) {
        leftWheelSpeed = LEFT_DEFAULT_SPEED;
        rightWheelSpeed = RIGHT_DEFAULT_SPEED ;
    } else if (s5) {
        // Sensor 5 seul sur le bord → parfait, tout droit
        leftWheelSpeed = LEFT_DEFAULT_SPEED;
        rightWheelSpeed = RIGHT_DEFAULT_SPEED+32;
    } else if (s3 && s4){
        leftWheelSpeed = LEFT_DEFAULT_SPEED;
        rightWheelSpeed = RIGHT_DEFAULT_SPEED+32;
    } else {
        // Aucun capteur droit → on perd le mur, virer à droite pour le retrouver
        leftWheelSpeed = LEFT_DEFAULT_SPEED + 17;
        rightWheelSpeed = RIGHT_DEFAULT_SPEED;
    }

    robot.motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void followWall() {
    if (turnDirection == 0) followRightWall();
    else followLeftWall();
}

// At a corridor end: back up onto the intersection line if needed, then do a short
// blind pivot in the turn direction. After this, turn() takes over in the turn state.
void alignToTurn() {
    robot.motor.stop();
    _delay_ms(1000);

    if (!robot.lineSensor.robotBumpLine()) {
        while (!robot.lineSensor.robotBumpLine())
            robot.motor.goBackward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
        robot.motor.stop();
    }


    

    if (turnDirection == 0)
        robot.motor.goForward(0, RIGHT_DEFAULT_SPEED);
    else
        robot.motor.goForward(LEFT_DEFAULT_SPEED, 0);
    _delay_ms(1500);
}

// Returns true when a turn is genuinely complete.
// Security 1: if the robot overshot (bumpLine after robotMiddle), back up to the line.
// Security 2: spin for 700 ms in the turn direction before confirming, to filter false detections.
bool confirmTurn() {
    if (!robot.lineSensor.robotMiddle()) return false;

    if (robot.lineSensor.robotBumpLine()) {
        while (!robot.lineSensor.robotBumpLine())
            robot.motor.goBackward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
        robot.motor.stop();
        return false;
    }
    return true;
}

void detectObject(EEPROMAddress addr) {
    robot.lineSensor.resetObjectCount();
    timer.startTimer();
    robot.motor.goForward(110, 110);
    while (ticks < 230) {
        robot.lineSensor.findObject(addr);
    }
    robot.led.off();
}

bool foundRoom() { 
    while (!(turnDirection == 0 ? 
        (robot.lineSensor.getSensor(1) || robot.lineSensor.getSensor(2) || robot.lineSensor.getSensor(3)) : 
        (robot.lineSensor.getSensor(3) || robot.lineSensor.getSensor(4) || robot.lineSensor.getSensor(5)))) { 
        followPath(turnDirection == 0 ? Alignment::LEFT : Alignment::RIGHT, Speed::DEFAULT);
    }
    _delay_ms(333);
    return true;
}

bool foundRoom2() {
    bool bumpRoom = turnDirection == 0 ? 
        (robot.lineSensor.getSensor(1) || robot.lineSensor.getSensor(2) || robot.lineSensor.getSensor(3)) : 
        (robot.lineSensor.getSensor(3) || robot.lineSensor.getSensor(4) || robot.lineSensor.getSensor(5));

    if (turnDirection == 0 ? robot.lineSensor.offTrackLeft() : robot.lineSensor.offTrackRight()) {
        ticks = 0; // wall still detected → reset the no-wall window
        return false;
    }
    if (ticks >= 200 || bumpRoom) {
        timer.stopTimer();
        ticks = 0;
        return true;
    }
    return false;
}

void movementLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {
        case Action::PARKING:   //parking a swit
            if (stepCount == 0) {
                if (turnDirection == 0)
                    robot.motor.goBackward(LEFT_DEFAULT_SPEED, 0);
                else
                    robot.motor.goBackward(0, RIGHT_DEFAULT_SPEED);
                _delay_ms(2380);
                robot.motor.stop();
                stepCount++;
            }
            if (stepCount == 1) {
                if (turnDirection == 0)
                    robot.motor.goForward(0, RIGHT_DEFAULT_SPEED);
                else
                    robot.motor.goForward(LEFT_DEFAULT_SPEED, 0);
                _delay_ms(1380);
                while (robot.lineSensor.offTrackAmount() < 4) {}
                robot.motor.stop();
                stepCount++;
            }
            break;
        
        case Action::LEAVE_PARKING:
            
                if (turnDirection == 0) // Sort vers la droite, puis tourne à gauche
                    robot.motor.goForward(0, RIGHT_DEFAULT_SPEED - 10);
                
                else robot.motor.goForward(LEFT_DEFAULT_SPEED - 10, 0); // Sort vers la gauche, puis tourne à droite
                _delay_ms(1200);
                while (true) {
                    if (robot.lineSensor.robotMiddle()) {
                        _delay_ms(50);
                        while (!robot.lineSensor.isOnRightLine()) {
                            robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
                            // followRightWall();
                            robot.led.green();
                        }
                        while (!robot.lineSensor.robotMiddle()) {
                            robot.motor.goForward(72, RIGHT_DEFAULT_SPEED);
                        }
                        robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED - 10);
                        break;
                        // while (true) followRightWall();
                            // if (turnDirection == 0) // Sort vers la droite, puis tourne à gauche
                            //     robot.motor.goForward(0, RIGHT_DEFAULT_SPEED - 10);
                    
                            // else // Sort vers la gauche, puis tourne à droite
                            //     robot.motor.goForward(LEFT_DEFAULT_SPEED - 10, 0);
                            // firstTime = false;
                    }
                }
            
            break;
            
        case Action::AFTER_PARKING:
            timer.startTimer();
            // if (afterParkingCount < 600)  {
            //     followWall();
            //     afterParkingCount++;
            // }
            // else followPath(Alignment::DEFAULT);
            while (ticks < 250){
                followRightWall();
                if (robot.lineSensor.isOnRightLine()){
                    ticks = 0;
                }
            }
            timer.stopTimer();
            ticks = 0;

            timer.startTimer();
            while (ticks < 150 && !robot.lineSensor.robotBumpLine()) {
                followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;
            while (!robot.lineSensor.robotBumpLine()) {
                followPath(Alignment::DEFAULT, Speed::SLOW);
            }

            break;

        case Action::FIRST_TURN:
            turn();
            break;

        case Action::FIRST_CORRIDOR:
            robot.lineSensor.resetDamage();
            timer.startTimer();
            while (ticks < 400 && !robot.lineSensor.robotBumpLine()) {
            if (robot.lineSensor.findDamage(DAMAGE_EST)) {
                followLine();
            }   
            else followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;


            while (!robot.lineSensor.robotBumpLine()) {
                followPath(Alignment::DEFAULT, Speed::SLOW);
            }
            
            break;



        case Action::SECOND_TURN: // Second turn and prepare for the first room
            turn();
            break;
            
        case Action::SECOND_CORRIDOR:

            if (roomCount == 0) { // PEOPLE ROOM A
                timer.startTimer();
                while (ticks < 245) { followPath(Alignment::DEFAULT, Speed::DEFAULT); }
                robot.motor.stop();
                _delay_ms(500);
                timer.stopTimer();
                ticks = 0;
                if (turnDirection == 0) robot.motor.spinLeft(135);
                else                    robot.motor.spinRight(135);
                robot.motor.stop();
                _delay_ms(800);
            }
            else if (roomCount == 1 || roomCount == 2) { // OBJECT ROOM B ou C
                // if (foundRoom()) {
                //     robot.motor.stop();
                //     _delay_ms(150);
                //     if (turnDirection == 0) robot.motor.spinLeft(90);
                //     else                    robot.motor.spinRight(90);
                //     robot.motor.stop();
                //     _delay_ms(800);
                // }
                ticks = 0;
                timer.startTimer();
                while (!foundRoom2()) {followPath(turnDirection == 0 ? Alignment::LEFT : Alignment::RIGHT, Speed::DEFAULT); }
                robot.motor.stop();
                _delay_ms(150);
                if (turnDirection == 0) robot.motor.spinLeft(roomCount == 1 ? 80 : 74);
                else                    robot.motor.spinRight(roomCount == 1 ? 80 : 74);
                robot.motor.stop();
                _delay_ms(800);
            }
            else if (roomCount == 3) { // PEOPLE ROOM D
                // if (foundRoom()) {
                //     robot.motor.stop();
                //     _delay_ms(150);
                //     if (turnDirection == 0) robot.motor.spinLeft(123);
                //     else                    robot.motor.spinRight(123);
                //     robot.motor.stop();
                //     _delay_ms(800);
                // }
                ticks = 0;
                timer.startTimer();
                while (!foundRoom2()) {followPath(turnDirection == 0 ? Alignment::LEFT : Alignment::RIGHT, Speed::DEFAULT); }
                robot.motor.stop();
                _delay_ms(150);
                if (turnDirection == 0) robot.motor.spinLeft(110);
                else                    robot.motor.spinRight(110);
                robot.motor.stop();
                _delay_ms(800);
            }
            break;

        case Action::PEOPLE_ROOM:
            if (roomCount == 0) { 
                distanceSensor.scanRoom(robot, turnDirection == 0 ? PERSON_D : PERSON_A, turnDirection == 0 ? LEFT : RIGHT);
            } else if (roomCount == 3) {
                distanceSensor.scanRoom(robot, turnDirection == 0 ? PERSON_A : PERSON_D, turnDirection == 0 ? LEFT : RIGHT);
            }

            while(!(robot.lineSensor.offTrackLeft() && robot.lineSensor.offTrackAmount() < 3)) {
                robot.motor.spinRightSpeed(100); 
            }

            robot.motor.stop();
            _delay_ms(500);

            while(!robot.lineSensor.robotMiddle()) {
                followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }

            break;

        case Action::OBJECT_ROOM:
            if (roomCount == 1)
                detectObject(turnDirection == 0 ? OBJECT_C : OBJECT_B);
            else if (roomCount == 2)
                detectObject(turnDirection == 0 ? OBJECT_B : OBJECT_C);

            while (!robot.lineSensor.robotBumpLine()) { // Once the timer is over, 180 and go forward till wall is found
                robot.motor.goForward(110, 110);
            }

            robot.motor.stop();
            _delay_ms(2000);
            if (turnDirection == 0) robot.motor.spinRight(180);
            else robot.motor.spinLeft(180);
            robot.motor.stop();
            _delay_ms(1000);

            timer.stopTimer();
            ticks = 0;
            
             while (!robot.lineSensor.robotBumpLine()) {
                robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
            }
            robot.motor.stop();

            _delay_ms(500);

            if (!robot.lineSensor.robotBumpLine()) {
                robot.motor.goBackward(200, 200);
                _delay_ms(100);
                while (!robot.lineSensor.robotBumpLine()) {
                    robot.motor.goBackward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);

                }
                robot.motor.stop();
            }

                if (turnDirection == 0)
                    robot.motor.goForward(0, RIGHT_DEFAULT_SPEED);
                else
                    robot.motor.goForward(LEFT_DEFAULT_SPEED, 0);
                _delay_ms(700);

                if (turnDirection == 0)
                    while (!(robot.lineSensor.offTrackAmount() <= 3 && robot.lineSensor.offTrackRight())) {}
                else
                    while (!(robot.lineSensor.offTrackAmount() <= 3 && robot.lineSensor.offTrackLeft())) {}
            
            // _delay_ms(500); 

            while (!robot.lineSensor.robotMiddle()) {
                turn();
            }
        // }
            previousAction = Action::OBJECT_ROOM;
            break;
        
        case Action::THIRD_TURN: //completed
            turn();
            break;

        case Action::THIRD_CORRIDOR: //completed
            // if (previousAction != Action::THIRD_CORRIDOR) {   OUTDATED CODE
            //     timer.startTimer();
            //     while (ticks < 150) {
            //         followPath(Alignment::DEFAULT);
            //     }
            //     timer.stopTimer();
            //     ticks = 0;
            //     previousAction = Action::THIRD_CORRIDOR;
            // }
            robot.lineSensor.resetDamage();

            timer.startTimer();
            while (ticks < 400 && !robot.lineSensor.robotBumpLine()) {
            if (robot.lineSensor.findDamage(DAMAGE_OUEST)) {
                followLine();
            }   
            else followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;
            
            while (!robot.lineSensor.robotBumpLine()) {
                followPath(Alignment::DEFAULT, Speed::SLOW);
            }

            break;

        case Action::FOURTH_TURN: //completed
            turn();
            break;

        case Action::ENTER_PARKING:
            // followPath(Alignment::RIGHT);
            followRightWall();
            break;

        case Action::COMPLETE_PARKING:

            // if (turnDirection == 0) {
            //     while (!robot.lineSensor.isOnRightLine()) {
            //         robot.led.red();
            //         followPath(Alignment::RIGHT);
            //     }
            // } else {
            //     while (!robot.lineSensor.isOnLeftLine()) {
            //         robot.led.red();
            //         followPath(Alignment::RIGHT);
            //     }
            // }

            robot.led.green();
            timer.startTimer();

            if (turnDirection == 0)
                while (ticks < 240) { robot.motor.goForward(95, 160); }
            else
                while (ticks < 240) { robot.motor.goForward(160, 95); }
			timer.stopTimer();
			ticks = 0;

            robot.motor.goForward(LEFT_DEFAULT_SPEED - 40, RIGHT_DEFAULT_SPEED - 40);
            _delay_ms(400);

            while (robot.lineSensor.offTrackAmount() > 0){
                robot.motor.goBackward(LEFT_DEFAULT_SPEED - 40, RIGHT_DEFAULT_SPEED - 40);
            }

                    
            // while (!robot.lineSensor.robotBumpLine()) {  // Go forward till bumpline
            //     robot.motor.goForward(LEFT_DEFAULT_SPEED - 40, RIGHT_DEFAULT_SPEED - 40);
            // }

			// while (!robot.lineSensor.robotBumpLine()) { // If we passed the line, back up till we hit it again
			// 	robot.motor.goBackward(LEFT_DEFAULT_SPEED - 15, RIGHT_DEFAULT_SPEED - 15);
			// }

            // timer.startTimer();
            // while (robot.lineSensor.robotBumpLine() && ticks < 270) { // Stop touching the edge
            //     robot.motor.goBackward(LEFT_DEFAULT_SPEED - 15, RIGHT_DEFAULT_SPEED - 15);
			// }
			// timer.stopTimer();
			// ticks = 0;
            
            break;

        case Action::END:
            robot.motor.stop();
            for (uint8_t i = 0; i < robot.lineSensor.getNDamage(); i++){
                robot.led.green();
                _delay_ms(1000);
                robot.led.off();
                _delay_ms(1000);
            }
            break;
    }
}

void switchLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {
        case Action::PARKING:
            if (robot.lineSensor.robotBumpLine()) {

                _delay_ms(500);
                currentAction = Action::LEAVE_PARKING;
            }
            break;
        
        case Action::LEAVE_PARKING:
            if (robot.lineSensor.robotMiddle()) {
                        // if (afterParkingCount < 600)  {
            //     followWall();
            //     afterParkingCount++;
            // }
            // else followPath(Alignment::DEFAULT);    robot.led.red();
                _delay_ms(500);
                currentAction = Action::AFTER_PARKING;
            }
            break;
        case Action::AFTER_PARKING:
            if (robot.lineSensor.robotBumpLine()) {
                // alignToTurn();
                currentAction = Action::FIRST_TURN;
            }
            break;

        case Action::FIRST_TURN:
            if (confirmTurn()) currentAction = Action::FIRST_CORRIDOR;
            break;

        case Action::FIRST_CORRIDOR:
            if (robot.lineSensor.robotBumpLine()) {
                // alignToTurn();
                robot.lineSensor.setPreviousDamageState(false);
                robot.led.off();
                currentAction = Action::SECOND_TURN;
            }
            break;

        case Action::SECOND_TURN:
            if (confirmTurn()) currentAction = Action::SECOND_CORRIDOR;
            break;
        
        case Action::SECOND_CORRIDOR:
            if (roomCount == 0 || roomCount == 3)
                currentAction = Action::PEOPLE_ROOM;
            else if (roomCount == 1 || roomCount == 2)
                currentAction = Action::OBJECT_ROOM;
            else if (roomCount == 4) {
                while (!robot.lineSensor.robotBumpLine()) { followPath(Alignment::DEFAULT, Speed::SLOW); }
                // alignToTurn();
                currentAction = Action::THIRD_TURN;
            }
            break;

        case Action::PEOPLE_ROOM: //COMPLET
            // if (robot.lineSensor.robotBumpLine()) {
            //     roomCount++;
            //     _delay_ms(500);
            //     currentAction = Action::SECOND_CORRIDOR;
            // }
            if (robot.lineSensor.robotMiddle()) {
                roomCount++;
                // _delay_ms(500);
                currentAction = Action::SECOND_CORRIDOR;
            }
            break;

        case Action::OBJECT_ROOM:  
            if (robot.lineSensor.robotMiddle()) {
                roomCount++;
                currentAction = Action::SECOND_CORRIDOR; // Transition to the second turn
            }
            break;
        
        case Action::THIRD_TURN:
            if (confirmTurn()) currentAction = Action::THIRD_CORRIDOR;
            break;

        case Action::THIRD_CORRIDOR:
            if (robot.lineSensor.robotBumpLine()) {
                // alignToTurn();
                robot.lineSensor.setPreviousDamageState(false);
                robot.led.off();
                currentAction = Action::FOURTH_TURN;
            }
            break;

        case Action::FOURTH_TURN:
            if (confirmTurn()) currentAction = Action::ENTER_PARKING;
            break;
            
        case Action::ENTER_PARKING: //completed
            if (turnDirection == 0 ? robot.lineSensor.isOnRightLine() : robot.lineSensor.isOnLeftLine()) {
                parkingCount++;
                if (parkingCount == 2) {
                    currentAction = Action::COMPLETE_PARKING;
                    break;
                }
                
                robot.led.red();
                timer.startTimer();
                while (ticks < 40) {
                    // robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
                    followPath(Alignment::RIGHT, Speed::DEFAULT);
                }
                robot.led.off();
                timer.stopTimer();
                ticks = 0;
            }
            break;

        case Action::COMPLETE_PARKING: //completed
            currentAction = Action::END;
            break;

        case Action::END:
			while (true) {}
            break;
        }
}

 void runProject(){
    //     Memoire24CXXX eeprom;                Pour reset adresses 10 à 15 du eeprom à 0
    //     for (uint8_t i = 0; i < 6; i++) {
    //     eeprom.ecriture(10 + i, 0);
    //     _delay_ms(5);
    // }
    timer.setModeCTC(Timer::PRESCALE_64);
    timer.setOCRA(OCR1A_10MS);
    sei();

    _delay_ms(500);
    Action currentAction = Action::PARKING;
    Action previousAction = static_cast<Action>(-1);

    switch (selectedMode) {
        case INSTRUCTION: robot.runInstruction(); break;
        case EXECUTION:   robot.runProject();     break;
        case RAPPORT:     robot.runRapport();     break;
    }
    while (true) {}
}
