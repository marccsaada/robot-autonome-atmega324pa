#include "libstatique.hpp"
#include "lineSensor.hpp"
#include <avr/interrupt.h>

Robot robot;
Timer timer(Timer::TIMER1);
DistanceSensor distanceSensor;

const uint8_t LEFT_DEFAULT_SPEED = 115;
const uint8_t RIGHT_DEFAULT_SPEED = 110;
uint8_t stepCount = 0;
bool completeTurn = false;
bool firstTime = true; // VARIABLE POUR LE VIRAGE DU PARKING
uint16_t afterParkingCount = 0;
uint8_t roomCount = 0;
uint8_t parkingCount = 0;
uint8_t turnDirection = 0; //0 = left, 1 = right


const uint16_t OCR1A_10MS = 1250;
volatile uint16_t ticks = 0;

enum class Action {
    PARKING,
    LEAVE_PARKING,
    AFTER_PARKING,
    FIRST_TURN,
    FIRST_CORRIDOR,
    SECOND_TURN,
    SECOND_CORRIDOR,
    THIRD_TURN,
    THIRD_CORRIDOR,
    PEOPLE_ROOM,
    OBJECT_ROOM,
    FOURTH_TURN,
    ENTER_PARKING,
    COMPLETE_PARKING,
    END,
};

ISR(TIMER1_COMPA_vect) {
    ticks++;
}

ISR(TIMER1_COMPB_vect) {
    // vide — nécessaire car setModeCTC active OCIE1B
}

void turnLeft() {
    uint8_t count = robot.lineSensor.offTrackAmount();

    // Virage à gauche proportionnel: plus de capteurs noirs = arc plus serré
    // Aucun delay → le robot lit les capteurs à chaque itération sans jamais dépasser le tape
    uint8_t leftSpeed;
    const uint8_t rightSpeed = RIGHT_DEFAULT_SPEED + 18;

    if      (count == 5 || robot.lineSensor.offTrackLeft()) leftSpeed = 0;    // pivot sur place (mur droit devant)
    else if (count == 4) leftSpeed = 0;
    else if (count == 3) leftSpeed = 0;
    else if (count == 2) leftSpeed = 95/*90*/;
    else if (count == 1) leftSpeed = 100/*94*/;
    else                 leftSpeed = LEFT_DEFAULT_SPEED/*-10*/;  // aucun capteur → tout droit

    robot.motor.goForward(leftSpeed, rightSpeed);
}

void turnRight() { 
    uint8_t count = robot.lineSensor.offTrackAmount();

    // Virage à gauche proportionnel: plus de capteurs noirs = arc plus serré
    // Aucun delay → le robot lit les capteurs à chaque itération sans jamais dépasser le tape
    uint8_t rightSpeed;
    const uint8_t leftSpeed = LEFT_DEFAULT_SPEED;

    if      (count == 5) rightSpeed = 0;    // pivot sur place (mur droit devant)
    else if (count == 4) rightSpeed = 0;
    else if (count == 3) rightSpeed = 0;
    else if (count == 2) rightSpeed = 65;
    else if (count == 1) rightSpeed = 75;
    else                 rightSpeed = RIGHT_DEFAULT_SPEED;  // aucun capteur → tout droit

    robot.motor.goForward(leftSpeed, rightSpeed);
}

void followPath() { 
    uint8_t leftWheelSpeed = LEFT_DEFAULT_SPEED-10;
    uint8_t rightWheelSpeed = RIGHT_DEFAULT_SPEED-10;
    
    if (robot.lineSensor.offTrackLeft()) { 
        leftWheelSpeed += robot.lineSensor.offTrackAmount() * 30;
        rightWheelSpeed -= robot.lineSensor.offTrackAmount() * 5;
    }
    else if (robot.lineSensor.offTrackRight()) { 
        rightWheelSpeed += robot.lineSensor.offTrackAmount() * 30;
        leftWheelSpeed -= robot.lineSensor.offTrackAmount() * 5;
    }
    robot.motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void followLine() { 
    if (robot.lineSensor.getSensor(1)) {
        robot.motor.goForward(/*LEFT_DEFAULT_SPEED*/110/1.3, /*RIGHT_DEFAULT_SPEED*/ 110);
    }
    else if (robot.lineSensor.getSensor(3)) {
        robot.motor.goForward(/*LEFT_DEFAULT_SPEED*/ 110, /*LEFT_DEFAULT_SPEED*/110/1.3);
    }
     else {
        robot.motor.goForward(/*LEFT_DEFAULT_SPEED*/ 110, /*RIGHT_DEFAULT_SPEED*/ 110);
    }
    // followPath();
}

void turn() {
    if (turnDirection == 0) turnLeft();
    else turnRight();
}

void turnOpposite() {
    if (turnDirection == 0) turnRight();
    else turnLeft();
}


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
    uint8_t leftWheelSpeed = LEFT_DEFAULT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_DEFAULT_SPEED;
    uint8_t amount = robot.lineSensor.offTrackAmount();

    if (robot.lineSensor.offTrackRight() && amount >= 1) {
        // Too close to right wall → turn left to move away
        leftWheelSpeed -= amount * 5;
        rightWheelSpeed += amount * 20;
    } else {
        // Right wall not detected → gentle right drift to re-acquire it
        rightWheelSpeed -= robot.lineSensor.offTrackAmount() * 5;
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
    _delay_ms(1000);
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
            while (firstTime) {
                if (turnDirection == 0) // Sort vers la droite, puis tourne à gauche
                    robot.motor.goForward(0, RIGHT_DEFAULT_SPEED - 10);
                
                else // Sort vers la gauche, puis tourne à droite
                    robot.motor.goForward(LEFT_DEFAULT_SPEED - 10, 0);
                if (robot.lineSensor.sensors345()) {
                    _delay_ms(50);
                    while (!robot.lineSensor.sensors345()){
                        if (turnDirection == 0) // Sort vers la droite, puis tourne à gauche
                            robot.motor.goForward(0, RIGHT_DEFAULT_SPEED - 10);
                
                        else // Sort vers la gauche, puis tourne à droite
                            robot.motor.goForward(LEFT_DEFAULT_SPEED - 10, 0);
    
                        firstTime = false;
                    }
                }
            }
            turn();
            break;
            
        case Action::AFTER_PARKING:
            if (afterParkingCount < 600)  {
                followWall();
                afterParkingCount++;
            } else followPath();
            break;

        case Action::FIRST_TURN:
            turn();
            break;

        case Action::FIRST_CORRIDOR:
                                                                                                            // PEUTETRE ENLEVER
            // if (robot.lineSensor.isLeftWall() && robot.lineSensor.isOnRightLine())
            //     robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED/1.5);
            //     _delay_ms(400);

            // if (robot.lineSensor.isRightWall() && robot.lineSensor.isOnLeftLine())
            //     robot.motor.goForward(LEFT_DEFAULT_SPEED/1.5, RIGHT_DEFAULT_SPEED);
            //     _delay_ms(400);


            // robot.lineSensor.findDamage(DAMAGE_EST);

            // followPath();
            timer.startTimer();
            while (ticks < 400) {
            if (robot.lineSensor.findDamage(DAMAGE_EST)) {
                followLine();
            }   
            else followPath();
            }
            timer.stopTimer();
            ticks = 0;

            while (!robot.lineSensor.robotBumpLine()) {
                followPath();
            }
            
            break;



        case Action::SECOND_TURN: // Second turn and prepare for the first room
            turn();
            break;
            
        case Action::SECOND_CORRIDOR:
            if (roomCount == 0) { // ROOM A OR D
                timer.startTimer();
                while (ticks < 245) { followPath(); }
                robot.motor.stop();
                _delay_ms(2000);
                timer.stopTimer();
                ticks = 0;
            }
            else if (roomCount == 1) { // ROOM B OR C
                timer.startTimer();
                while (ticks < 617) { followPath(); }
                timer.stopTimer();
                ticks = 0;
            }
            else if (roomCount == 2) { // ROOM B OR C
                timer.startTimer();
                while (ticks < 413) { followPath(); }
                timer.stopTimer();
                ticks = 0;
            }
            else if (roomCount == 3) { // ROOM A OR D
                timer.startTimer();
                while (ticks < 580) { followPath(); }
                robot.motor.stop();
                _delay_ms(2000);
                timer.stopTimer();
                ticks = 0;
            }
            break;



        case Action::PEOPLE_ROOM:
            while(!(robot.lineSensor.offTrackLeft() && robot.lineSensor.offTrackAmount() < 3)) {
                robot.motor.spinRightSpeed(100); 
            }

            robot.motor.stop();
            _delay_ms(500);

            while(!robot.lineSensor.robotMiddle()) {
                followPath();
            }

            break;

        case Action::OBJECT_ROOM:
            timer.startTimer();
            robot.motor.goForward(110, 110);

            while (ticks < 260) { // detect object for n seconds
                robot.lineSensor.findObject(OBJECT_B);
            }
            robot.led.off();

            while (!robot.lineSensor.robotBumpLine()) { // Once the timer is over, 180 and go forward till wall is found
                robot.motor.goForward(110, 110);
            }

            robot.motor.stop();
            _delay_ms(2000);
            if (turnDirection == 0) robot.motor.spinRight(195);
            else robot.motor.spinLeft(198);
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

                robot.motor.goForward(0, RIGHT_DEFAULT_SPEED);
                _delay_ms(700);

                while (!(robot.lineSensor.offTrackAmount() <= 3 && robot.lineSensor.offTrackRight())) {
                }
            
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
            if (previousAction != Action::THIRD_CORRIDOR) {
                timer.startTimer();
                while (ticks < 150) {
                    followPath();
                }
                timer.stopTimer();
                ticks = 0;
                previousAction = Action::THIRD_CORRIDOR;
            }
            
            timer.startTimer();
            while (ticks < 400) {
            if (robot.lineSensor.findDamage(DAMAGE_EST)) {
                followLine();
            }   
            else followPath();
            }
            timer.stopTimer();
            ticks = 0;
            
            while (!robot.lineSensor.robotBumpLine()) {
                followPath();
            }

            break;

        case Action::FOURTH_TURN: //completed
            turn();
            break;

        case Action::ENTER_PARKING:
            followPath();
            break;

        case Action::COMPLETE_PARKING:
            robot.led.green();
            if (turnDirection == 0) {
                while (!robot.lineSensor.isOnRightLine()) {
                    robot.led.red();
                    followPath();
                }
            } else {
                while (!robot.lineSensor.isOnLeftLine()) {
                    robot.led.red();
                    followPath();
                }
            }

            robot.led.green();
            timer.startTimer();

            if (turnDirection == 0)
                while (ticks < 300) { robot.motor.goForward(95, 160); }
            else
                while (ticks < 300) { robot.motor.goForward(160, 95); }
			timer.stopTimer();
			ticks = 0;

            while (!robot.lineSensor.robotBumpLine()) {  // Go forward till bumpline
                robot.motor.goForward(LEFT_DEFAULT_SPEED - 15, RIGHT_DEFAULT_SPEED - 15);
            }

			while (!robot.lineSensor.robotBumpLine()) { // If we passed the line, back up till we hit it again
				robot.motor.goBackward(LEFT_DEFAULT_SPEED - 15, RIGHT_DEFAULT_SPEED - 15);
			}

            timer.startTimer();
            while (robot.lineSensor.robotBumpLine() && ticks < 150) { // Stop touching the edge
                robot.motor.goBackward(LEFT_DEFAULT_SPEED - 15, RIGHT_DEFAULT_SPEED - 15);
			}
			timer.stopTimer();
			ticks = 0;
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
                _delay_ms(500);
                currentAction = Action::AFTER_PARKING;
            }
            break;
        case Action::AFTER_PARKING:
            if (robot.lineSensor.robotBumpLine()) {
                alignToTurn();
                currentAction = Action::FIRST_TURN;
            }
            break;

        case Action::FIRST_TURN:
            if (confirmTurn()) currentAction = Action::FIRST_CORRIDOR;
            break;

        case Action::FIRST_CORRIDOR:
            if (robot.lineSensor.robotBumpLine()) {
                alignToTurn();
                robot.lineSensor.setPreviousDamageState(false);
                robot.led.off();
                currentAction = Action::SECOND_TURN;
            }
            break;

        case Action::SECOND_TURN:
            if (confirmTurn()) currentAction = Action::SECOND_CORRIDOR;
            break;
        
        case Action::SECOND_CORRIDOR:
            if (roomCount == 0 || roomCount == 3) { // PEOPLE ROOM
                if (turnDirection == 0) {
                    robot.motor.spinLeft(135);
                    robot.motor.stop();
                    _delay_ms(800);
                }
                else robot.motor.spinRight(135);
                distanceSensor.scanRoom(robot, PERSON_A, turnDirection == 0 ? LEFT : RIGHT);
                currentAction = Action::PEOPLE_ROOM;
            }
            else if (roomCount == 1 || roomCount == 2) { // OBJECT ROOM
                _delay_ms(500);
                if (turnDirection == 0) robot.motor.spinLeft(105);
                else                    robot.motor.spinRight(90);
                robot.motor.stop();
                _delay_ms(1000);
                currentAction = Action::OBJECT_ROOM;
            }
            else if (roomCount == 4) {
                while (!robot.lineSensor.robotBumpLine()) { followPath(); }
                alignToTurn();
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
                alignToTurn();
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
                timer.startTimer();
                while (ticks < 18) {
                    robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
                }

                if (turnDirection == 0 ? robot.lineSensor.isOnRightLine() : robot.lineSensor.isOnLeftLine()) {
                    if (parkingCount == 1) {
                        currentAction = Action::COMPLETE_PARKING;
                    }
                    parkingCount++;
                }

                timer.stopTimer();
                ticks = 0;
            }
            break;

        case Action::COMPLETE_PARKING: //completed
            if (robot.lineSensor.robotBumpLine())
                currentAction = Action::END;
            break;

        case Action::END:
			while (true) {}
            break;
        }
}

 int main(){

    timer.setModeCTC(Timer::PRESCALE_64);
    timer.setOCRA(OCR1A_10MS);
    sei();

    _delay_ms(500);
    Action currentAction = Action::PARKING;
    Action previousAction = static_cast<Action>(-1);

    while (true) {
        movementLogic(currentAction, previousAction);
        switchLogic(currentAction, previousAction);
    }
}