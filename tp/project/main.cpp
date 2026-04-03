#include "libstatique.hpp"
#include "lineSensor.hpp"

LineSensor lineSensor;
Robot robot;
Timer timer(Timer::TIMER1);

const uint8_t LEFT_DEFAULT_SPEED = 130;
const uint8_t RIGHT_DEFAULT_SPEED = 125;
uint8_t stepCount = 0;

// #define motor robot.motor

enum class Action {
    PARKING,
    LEAVE_PARKING,
    AFTER_PARKING,
    FIRST_TURN,
    FIRST_CORRIDOR,
    SECOND_CORRIDOR,
    ENTER_FIRST_ROOM,
    FIRST_ROOM,
    END,
};

// void exitParking() {
//     while (!lineSensor.robotMiddle())
// }

void turnLeft() {
    uint8_t count = lineSensor.offTrackAmount();

    // Virage à gauche proportionnel: plus de capteurs noirs = arc plus serré
    // Aucun delay → le robot lit les capteurs à chaque itération sans jamais dépasser le tape
    uint8_t leftSpeed;
    const uint8_t rightSpeed = RIGHT_DEFAULT_SPEED;

    if      (count == 5) leftSpeed = 0;    // pivot sur place (mur droit devant)
    else if (count == 4) leftSpeed = 0;
    else if (count == 3) leftSpeed = 80;
    else if (count == 2) leftSpeed = 80;
    else if (count == 1) leftSpeed = 85;
    else                 leftSpeed = LEFT_DEFAULT_SPEED;  // aucun capteur → tout droit

    robot.motor.goForward(leftSpeed, rightSpeed);
}

void turnRight() { 
    uint8_t count = lineSensor.offTrackAmount();

    // Virage à gauche proportionnel: plus de capteurs noirs = arc plus serré
    // Aucun delay → le robot lit les capteurs à chaque itération sans jamais dépasser le tape
    uint8_t rightSpeed;
    const uint8_t leftSpeed = LEFT_DEFAULT_SPEED;

    if      (count == 5) rightSpeed = 0;    // pivot sur place (mur droit devant)
    else if (count == 4) rightSpeed = 0;
    else if (count == 3) rightSpeed = 80;
    else if (count == 2) rightSpeed = 80;
    else if (count == 1) rightSpeed = 85;
    else                 rightSpeed = RIGHT_DEFAULT_SPEED;  // aucun capteur → tout droit

    robot.motor.goForward(leftSpeed, rightSpeed);
}

void followPath() { 
    uint8_t leftWheelSpeed = LEFT_DEFAULT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_DEFAULT_SPEED;
    
    if (lineSensor.offTrackLeft()) { 
        leftWheelSpeed += lineSensor.offTrackAmount() * 30;
        rightWheelSpeed -= lineSensor.offTrackAmount() * 5;
    }
    else if (lineSensor.offTrackRight()) { 
        rightWheelSpeed += lineSensor.offTrackAmount() * 30;
        leftWheelSpeed -= lineSensor.offTrackAmount() * 5;
    }
    robot.motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void followLeftWall(){
    uint8_t leftWheelSpeed = LEFT_DEFAULT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_DEFAULT_SPEED;
    
    if (!lineSensor.isLeftWall()) { 
        leftWheelSpeed -= lineSensor.offTrackAmount() * 30;
        rightWheelSpeed += lineSensor.offTrackAmount() * 5;
    }
    else if (lineSensor.isLeftWall()) { 
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

uint8_t findObject() { // Returns the position of the object if found. else, return -1;
    for (uint8_t i = 1; i <= 3; i++) { 
        move3Inches();
        if (lineSensor.robotBumpLine()) { 
            return i;
        }
    }
    return -1;
}

void movementLogic(Action& currentAction) {
    switch (currentAction) {
        case Action::PARKING:
            _delay_ms(25);
            if (stepCount == 0) {
                robot.motor.goBackward(LEFT_DEFAULT_SPEED, 0);
                _delay_ms(2450);
                robot.motor.stop();
                stepCount++;
            }
            if (stepCount == 1) {
                robot.motor.goForward(0, RIGHT_DEFAULT_SPEED);
                _delay_ms(1380);
                while (lineSensor.offTrackAmount() < 4) {}
                robot.motor.stop();
                stepCount++;
            }
            

            break;
        
        case Action::LEAVE_PARKING:
            turnLeft();
            _delay_ms(25);
            break;
            
        case Action::AFTER_PARKING:
            followPath();
            _delay_ms(25);
            break;

        case Action::FIRST_TURN:
            turnLeft();
            //turnRight();
            _delay_ms(25);
            break;

        case Action::FIRST_CORRIDOR:
            followPath();
            _delay_ms(25);
            break;

        case Action::SECOND_CORRIDOR:
            followPath();
            
            _delay_ms(25);
            break;

        // case Action::ENTER_FIRST_ROOM:
        //     robot.motor.goForward(120, 120);
        //     break;
        // case Action::FIRST_ROOM:
        //     robot.motor.goForward(120, 120);
        //     _delay_ms(1000);

        //     // IMPLEMENTER LOGIQUE
        //     break;
            
        case Action::END:
        _delay_ms(25);
            break;
    }
}

void switchLogic(Action& currentAction) {
    switch (currentAction) {
        case Action::PARKING:
            if (lineSensor.robotBumpLine()) {
                _delay_ms(500);
                currentAction = Action::LEAVE_PARKING;
            }
            break;
        
        case Action::LEAVE_PARKING:
            if (lineSensor.robotMiddle()) {
                _delay_ms(500);
                currentAction = Action::AFTER_PARKING;
            }
                
            break;
        case Action::AFTER_PARKING:
            if (lineSensor.robotBumpLine()) {
        
                robot.motor.stop();
                _delay_ms(2000);
                while (!lineSensor.robotBumpLine()){
                    robot.motor.goBackward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
                    _delay_ms(25);
                    if (lineSensor.robotBumpLine()){
                        break;
                    }
                    robot.motor.stop();
                }
                currentAction = Action::FIRST_TURN;
            }
            break;

        case Action::FIRST_TURN:
            if (lineSensor.robotMiddle()) {
                _delay_ms(500);
                currentAction = Action::FIRST_CORRIDOR;
            }
            break;

        case Action::FIRST_CORRIDOR:  
            if (lineSensor.robotBumpLine()) {
                currentAction = Action::END; // Transition to the second turn
            }
            break;
        
        case Action::SECOND_CORRIDOR:  
            if (lineSensor.robotBumpLine()) {
                currentAction = Action::END; // Transition to the second turn
            }
            break;

        // case Action::ENTER_FIRST_ROOM:
        //     _delay_ms(1000);
        //     currentAction = Action::FIRST_ROOM;

        case Action::END:
            robot.motor.stop();
            break;
    }
}


int main() {
    Action currentAction = Action::PARKING;

    while (true) {
        movementLogic(currentAction);
        switchLogic(currentAction);
    }
}