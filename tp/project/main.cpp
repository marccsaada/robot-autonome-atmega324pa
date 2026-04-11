#include "libstatique.hpp"
#include "lineSensor.hpp"
#include <avr/interrupt.h>

Robot robot;
Timer timer(Timer::TIMER1);

const uint8_t LEFT_DEFAULT_SPEED = 105;
const uint8_t RIGHT_DEFAULT_SPEED = 100;
uint8_t stepCount = 0;
bool completeTurn = false;
bool firstTime = true; // VARIABLE POUR LE VIRAGE DU PARKING
uint16_t afterParkingCount = 0;
uint8_t roomCount = 0;
uint8_t parkingCount = 0;

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
    const uint8_t rightSpeed = RIGHT_DEFAULT_SPEED;

    if      (count == 5 || robot.lineSensor.offTrackLeft()) leftSpeed = 0;    // pivot sur place (mur droit devant)
    else if (count == 4) leftSpeed = 0;
    else if (count == 3) leftSpeed = 0;
    else if (count == 2) leftSpeed = 90;
    else if (count == 1) leftSpeed = 94;
    else                 leftSpeed = LEFT_DEFAULT_SPEED-10;  // aucun capteur → tout droit

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
    robot.led.red();
    uint8_t leftWheelSpeed = LEFT_DEFAULT_SPEED /*-10*/;
    uint8_t rightWheelSpeed = RIGHT_DEFAULT_SPEED/*-10*/;
    
    if (robot.lineSensor.offTrackLeft()) { 
        leftWheelSpeed += robot.lineSensor.offTrackAmount() * 30;//30;
        rightWheelSpeed -= robot.lineSensor.offTrackAmount() * 5;//* 5;
    }
    else if (robot.lineSensor.offTrackRight()) { 
        rightWheelSpeed += robot.lineSensor.offTrackAmount() * 30;//30;
        leftWheelSpeed -= robot.lineSensor.offTrackAmount()  * 5;//30;
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

// SALLES B ET C
void move3Inches() { 
    robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
    _delay_ms(150);
    robot.motor.stop();
}

// void findObject() { // Returns the position of the object if found. else, return -1;
//     if (robot.lineSensor.robotBumpLine()) {
        
//     }
// }

void movementLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {
        case Action::PARKING:
            // // _delay_ms(5);
            if (stepCount == 0) {
                robot.motor.goBackward(LEFT_DEFAULT_SPEED, 0);
                _delay_ms(2380);
                robot.motor.stop();
                stepCount++;
            }
            if (stepCount == 1) {
                robot.motor.goForward(0, RIGHT_DEFAULT_SPEED);
                _delay_ms(1380);
                while (robot.lineSensor.offTrackAmount() < 4) {}
                robot.motor.stop();
                stepCount++;
            }
            break;
        
        case Action::LEAVE_PARKING:
            if (firstTime) {
                robot.motor.goForward(0, RIGHT_DEFAULT_SPEED - 10);                               // NOUVEL AJOUT POUR CONTRER LE FAIT QUIL DEPASSERAIT LA LIGNE
                firstTime = false;
                _delay_ms(800);
            }
            turnLeft();
            break;
            
        case Action::AFTER_PARKING:
            if (afterParkingCount < 600)  {
                followRightWall();
                afterParkingCount++;
            } else followPath();
            break;

        case Action::FIRST_TURN:
            turnLeft();
            break;

        case Action::FIRST_CORRIDOR:
                                                                                                            // PEUTETRE ENLEVER
            if (robot.lineSensor.isLeftWall() && robot.lineSensor.isOnRightLine())
                robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED/1.5);

            if (robot.lineSensor.isRightWall() && robot.lineSensor.isOnLeftLine())
                robot.motor.goForward(LEFT_DEFAULT_SPEED/1.5, RIGHT_DEFAULT_SPEED);

            robot.lineSensor.findDamage();

            followPath();
            // _delay_ms(5);

            // followPath();
            // robot.lineSensor.foundDamage();
            // // _delay_ms(5);
            break;

        case Action::SECOND_TURN: // Second turn and prepare for the first room
            turnLeft();
            // _delay_ms(5);

            if (robot.lineSensor.robotMiddle()) {
                timer.startTimer();
                while (ticks < 100) { // Wait a set amount of time to align with the first room
                    followPath();
                }
                robot.motor.stop();
                _delay_ms(200);
                timer.stopTimer();
                ticks = 0;
                currentAction = Action::SECOND_CORRIDOR;
            }
            break;
            
        case Action::SECOND_CORRIDOR:
            if (roomCount == 0) { // ROOM A OR D
                timer.startTimer();
                while (ticks < 600) { followPath(); }
                timer.stopTimer();
                ticks = 0;
            }
            else if (roomCount == 1) { // ROOM B OR C
                timer.startTimer();
                while (ticks < 600) { followPath(); }
                timer.stopTimer();
                ticks = 0;
            }
            else if (roomCount == 2) { // ROOM B OR C
                timer.startTimer();
                while (ticks < 600) { followPath(); }
                timer.stopTimer();
                ticks = 0;
            }
            else if (roomCount == 3) { // ROOM A OR D
                // timer.startTimer();
                // while (ticks < 300) { followPath(); }
                // timer.stopTimer();
                // ticks = 0;
                followPath();
            }
            else if (roomCount == 4) {
                currentAction = Action::THIRD_TURN; // Transition to the second turn
            }
            break;



        case Action::PEOPLE_ROOM:
            if (previousAction != Action::PEOPLE_ROOM) { // Enter the room, scan the room
                robot.motor.spinLeft(45);
                // scan room
                previousAction = Action::PEOPLE_ROOM;
            }
            
            while (!robot.lineSensor.offTrackRight()) // Exit the room
                robot.motor.spinRightSpeed(100);
            break;

        case Action::OBJECT_ROOM:
            if (previousAction != Action::OBJECT_ROOM) { // Turn towards the room
                robot.motor.spinLeft(90);
                robot.motor.stop();
                _delay_ms(50);

                timer.startTimer();     // Go forward for the timers time to detect the objects
                robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
                while (ticks < 600) {   // detect object for n seconds
                    robot.lineSensor.findObject();
                }
                robot.motor.stop();
                timer.stopTimer();
                ticks = 0;

                previousAction = Action::OBJECT_ROOM;
            }
            
            _delay_ms(50);

            robot.motor.spinLeft(180);
            while (!robot.lineSensor.robotBumpLine()) { // Once the timer is over, 180 and go forward till wall is found
                robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
            }
            turnLeft();


            break;
        
        case Action::THIRD_TURN: //completed
            turnLeft();
            // _delay_ms(5);
            break;

        case Action::THIRD_CORRIDOR: //completed
                                                                                                            // PEUTETRE ENLEVER
            if (robot.lineSensor.isLeftWall() && robot.lineSensor.isOnRightLine())
                robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED/1.5);

            if (robot.lineSensor.isRightWall() && robot.lineSensor.isOnLeftLine())
                robot.motor.goForward(LEFT_DEFAULT_SPEED/1.5, RIGHT_DEFAULT_SPEED);

            robot.lineSensor.findDamage();

            // followPath();
            // // _delay_ms(5);

            followPath();
            robot.lineSensor.findDamage();
            // _delay_ms(5);
            break;

        case Action::FOURTH_TURN: //completed
             turnLeft();
            // _delay_ms(5);
            break;

        case Action::ENTER_PARKING:
            followPath();
            // _delay_ms(5);
            break;

        case Action::COMPLETE_PARKING:
            robot.led.green();
            while (!robot.lineSensor.isOnRightLine()) {
                robot.led.red();
                followPath();
            }
            robot.led.green();
            timer.startTimer();
            while (ticks < 200) { 
                // if (robot.lineSensor.robotBumpLine()) { 
                    // robot.motor.stop();
                    // _delay_ms(50);
                    // break;
                // }
                    
                robot.motor.goForward(95, 160);
            }

            while (!robot.lineSensor.robotBumpLine()) { 
                robot.motor.goForward(LEFT_DEFAULT_SPEED - 15, RIGHT_DEFAULT_SPEED - 15);
            }
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
            break;
        
        case Action::LEAVE_PARKING:
            if (robot.lineSensor.robotMiddle()) {
                _delay_ms(500);
                currentAction = Action::AFTER_PARKING;
            }
            break;
        
            case Action::AFTER_PARKING:
            if (robot.lineSensor.robotBumpLine()) {
        
                robot.motor.stop();
                _delay_ms(2000);
                while (!robot.lineSensor.robotBumpLine()) {
                    robot.motor.goBackward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
                    // _delay_ms(5);
                    if (robot.lineSensor.robotBumpLine()) {
                        break;
                    }
                    robot.motor.stop();
                }
                currentAction = Action::FIRST_TURN;
            }
            break;

        case Action::FIRST_TURN:
            if (robot.lineSensor.robotMiddle()) {
                _delay_ms(500);
                currentAction = Action::FIRST_CORRIDOR;
            }
            break;

        case Action::FIRST_CORRIDOR:  
            if (robot.lineSensor.robotBumpLine()) {
                currentAction = Action::SECOND_TURN; // Transition to the second turn
            }
            break;

        case Action::SECOND_TURN:
            // if (robot.lineSensor.robotMiddle()) {
            //     _delay_ms(500);
            //     currentAction = Action::SECOND_CORRIDOR;
            // }
            break;
        
        case Action::SECOND_CORRIDOR:  
            if (roomCount == 0) { // ROOM A OR D
                currentAction = Action::PEOPLE_ROOM; // Transition to the second turn
            }
            else if (roomCount == 1) { // ROOM B OR C
                currentAction = Action::OBJECT_ROOM; // Transition to the second turn
            }
            else if (roomCount == 2) { // ROOM B OR C
                currentAction = Action::OBJECT_ROOM; // Transition to the second turn
            }
            else if (roomCount == 3) { // ROOM A OR D
                currentAction = Action::PEOPLE_ROOM; // Transition to the second turn
            }
            else if (roomCount == 4) {
                while (!robot.lineSensor.robotBumpLine()) { followPath(); }
                currentAction = Action::THIRD_TURN; // Transition to the second turn
            }
            break;

        case Action::PEOPLE_ROOM:  
            if (robot.lineSensor.offTrackRight()) {
                roomCount++;
                currentAction = Action::SECOND_CORRIDOR; // Transition to the second turn
            }
            break;

        case Action::OBJECT_ROOM:  
            if (robot.lineSensor.robotMiddle()) {
                roomCount++;
                currentAction = Action::SECOND_CORRIDOR; // Transition to the second turn
            }
            break;
        
        case Action::THIRD_TURN: //completed
            if (robot.lineSensor.robotMiddle()) {
                _delay_ms(500);
                currentAction = Action::THIRD_CORRIDOR;
            }
            break;

        case Action::THIRD_CORRIDOR: //completed
            if (robot.lineSensor.robotBumpLine()) {
                currentAction = Action::FOURTH_TURN; // Transition to the second turn
            }
            break;

        case Action::FOURTH_TURN: //completed
            if (robot.lineSensor.robotMiddle()) {
                _delay_ms(500);
                currentAction = Action::ENTER_PARKING;
            }
            break;
            
        case Action::ENTER_PARKING: //completed
            if (robot.lineSensor.isOnRightLine()) {
                timer.startTimer();
                while (ticks < 18) {
                    robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
                }
                
                if (robot.lineSensor.isOnRightLine()) {
                    parkingCount++;
                    if (parkingCount == 1) {
                        currentAction = Action::COMPLETE_PARKING;
                    }
                }
                
                // if (robot.lineSensor.isRightWall()) {
                //     parkingCount++;
                //     if (parkingCount == 1) {
                //         currentAction = Action::COMPLETE_PARKING;
                //     }
                // }
                timer.stopTimer();
                ticks = 0;
            }
            break;

        case Action::COMPLETE_PARKING: //completed
            if (robot.lineSensor.robotBumpLine())
                currentAction = Action::END;
            break;

        case Action::END:
            break;
    }
}
}

 int main(){

    timer.setModeCTC(Timer::PRESCALE_64);
    timer.setOCRA(OCR1A_10MS);
    sei();

    _delay_ms(500);
    Action currentAction = Action::OBJECT_ROOM;
    Action previousAction = static_cast<Action>(-1);

    while (true) {
        movementLogic(currentAction, previousAction);
        switchLogic(currentAction, previousAction);
        // followPath();
        // _delay_ms(25);
    }
}