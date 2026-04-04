#include "libstatique.hpp"
#include "lineSensor.hpp"

Robot robot;
Timer timer(Timer::TIMER1);

const uint8_t LEFT_DEFAULT_SPEED = 138;
const uint8_t RIGHT_DEFAULT_SPEED = 133;
uint8_t stepCount = 0;
bool firstTime = true; // VARIABLE POUR LE VIRAGE DU PARKING
uint16_t afterParkingCount = 0;

// #define motor robot.motor

enum class Action {
    PARKING,
    LEAVE_PARKING,
    AFTER_PARKING,
    FIRST_TURN,
    FIRST_CORRIDOR,
    SECOND_TURN,
    SECOND_CORRIDOR,
    ENTER_FIRST_ROOM,
    FIRST_ROOM,
    END,
};

void turnLeft() {
    uint8_t count = robot.lineSensor.offTrackAmount();

    // Virage à gauche proportionnel: plus de capteurs noirs = arc plus serré
    // Aucun delay → le robot lit les capteurs à chaque itération sans jamais dépasser le tape
    uint8_t leftSpeed;
    const uint8_t rightSpeed = RIGHT_DEFAULT_SPEED;

    if      (count == 5 || robot.lineSensor.offTrackLeft()) leftSpeed = 0;    // pivot sur place (mur droit devant)
    else if (count == 4) leftSpeed = 0;
    else if (count == 3) leftSpeed = 0;
    else if (count == 2) leftSpeed = 95;
    else if (count == 1) leftSpeed = 110;
    else                 leftSpeed = LEFT_DEFAULT_SPEED-20;  // aucun capteur → tout droit

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
    else if (count == 3) rightSpeed = 80;
    else if (count == 2) rightSpeed = 80;
    else if (count == 1) rightSpeed = 85;
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
        leftWheelSpeed -= robot.lineSensor.offTrackAmount()   * 5;
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
        leftWheelSpeed -= robot.lineSensor.offTrackAmount() * 30;
        rightWheelSpeed += robot.lineSensor.offTrackAmount() * 5;
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

uint8_t findObject() { // Returns the position of the object if found. else, return -1;
    for (uint8_t i = 1; i <= 3; i++) { 
        move3Inches();
        if (robot.lineSensor.robotBumpLine()) { 
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
                _delay_ms(1980);
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
            _delay_ms(25);
            break;
            
        case Action::AFTER_PARKING:
            if (afterParkingCount < 600)  {
                followRightWall();
                afterParkingCount++;
            } else followPath();
            _delay_ms(25);
            break;

        case Action::FIRST_TURN:
            turnLeft();
            _delay_ms(25);
            break;

        case Action::FIRST_CORRIDOR:
                                                                                                            // PEUTETRE ENLEVER
            // if (robot.lineSensor.isLeftWall() && robot.lineSensor.isOnRightLine())
            //     robot.motor.goForward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED/1.5);

            // if (robot.lineSensor.isRightWall() && robot.lineSensor.isOnLeftLine())
            //     robot.motor.goForward(LEFT_DEFAULT_SPEED/1.5, RIGHT_DEFAULT_SPEED);

            // robot.lineSensor.foundDamage();

            // followPath();
            // _delay_ms(25);


            followPath();
            robot.lineSensor.foundDamage();
            _delay_ms(25);
            break;

        case Action::SECOND_CORRIDOR:
            followPath();
            
            _delay_ms(25);
            break;

        case Action::END:
            robot.motor.stop();
            for (uint8_t i = 0; i < robot.lineSensor.getNDamage(); i++){
                robot.led.green();
                _delay_ms(1000);
                robot.led.off();
                _delay_ms(1000);
            }
            _delay_ms(25);
            while (true) {};
            break;
    }
}

void switchLogic(Action& currentAction) {
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
        
                robot.motor.stop();
                _delay_ms(2000);
                while (!robot.lineSensor.robotBumpLine()){
                    robot.motor.goBackward(LEFT_DEFAULT_SPEED, RIGHT_DEFAULT_SPEED);
                    _delay_ms(25);
                    if (robot.lineSensor.robotBumpLine()){
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
                currentAction = Action::END; // Transition to the second turn
            }
            break;

        case Action::SECOND_TURN:
            if (robot.lineSensor.robotMiddle()) {
                _delay_ms(500);
                currentAction = Action::FIRST_CORRIDOR;
            }
            break;
        
        case Action::SECOND_CORRIDOR:  
            if (robot.lineSensor.robotBumpLine()) {
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
    _delay_ms(500);
    Action currentAction = Action::PARKING;

    while (true) {
        movementLogic(currentAction);
        switchLogic(currentAction);
    }
}