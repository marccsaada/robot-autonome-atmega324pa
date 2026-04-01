#include "libstatique.hpp"
#include "lineSensor.hpp"

LineSensor lineSensor;
Robot robot;
Timer timer(Timer::TIMER1);

// #define motor robot.motor

enum class State {
    MIDDLE,
    OFF_TRACK_LEFT,
    OFF_TRACK_RIGHT,
    BUMP_LINE,
    LEFT_WALL,
    RIGHT_WALL,
    DAMAGE
};

enum class Action {
    PARKING,
    FIRST_TURN,
    FIRST_CORRIDOR,
    ENTER_FIRST_ROOM,
    FIRST_ROOM,
    END,
};
void turnLeft() {

    uint8_t count = lineSensor.offTrackAmount();

    // Virage à gauche proportionnel: plus de capteurs noirs = arc plus serré
    // Aucun delay → le robot lit les capteurs à chaque itération sans jamais dépasser le tape
    uint8_t leftSpeed;
    const uint8_t rightSpeed = 110;

    if      (count == 5) leftSpeed = 0;    // pivot sur place (mur droit devant)
    else if (count == 4) leftSpeed = 0;
    else if (count == 3) leftSpeed = 80;
    else if (count == 2) leftSpeed = 80;
    else if (count == 1) leftSpeed = 85;
    else                 leftSpeed = 110;  // aucun capteur → tout droit

    robot.motor.goForward(leftSpeed, rightSpeed);
}

void followPath() { 

    uint8_t leftWheelSpeed = 100;
    uint8_t rightWheelSpeed = 100;
    
    if (lineSensor.offTrackLeft())
        rightWheelSpeed += lineSensor.offTrackAmount() * 15;
    else if (lineSensor.offTrackRight())
        leftWheelSpeed += lineSensor.offTrackAmount() * 15;

    robot.motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void movementLogic(Action& currentAction) {
    switch (currentAction) {

        case Action::FIRST_TURN:
            while (!lineSensor.robotMiddle()) {
                turnLeft();
                _delay_ms(50);
            }
            break;

        case Action::FIRST_CORRIDOR:

            followPath();

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
// Stop the robot // Turn off LEDs
            break;
    }
}

void switchLogic(Action& currentAction) {
    switch (currentAction) {
        // case Action::PARKING:
        //     if (lineSensor.robotBumpLine()) {
        //         currentAction = Action::FIRST_TURN;
        //     }
        //     break;

        case Action::FIRST_TURN:
            if (lineSensor.robotMiddle()) {
                currentAction = Action::FIRST_CORRIDOR;
            }
            break;

        case Action::FIRST_CORRIDOR:  
            if (lineSensor.robotBumpLine()) {
                currentAction = Action::END; // Transition to the second turn
            }
            break;

        // case Action::ENTER_FIRST_ROOM:
        //     _delay_ms(1000);
        //     currentAction = Action::FIRST_ROOM;

        case Action::END:

            break;
    }
}


int main() {

    // Action currentAction = Action::PARKING;
    // while (true) {
    //     movementLogic(currentAction);
    //     switchLogic(currentAction);
    // }
    // while (true) { 
    //     while (!lineSensor.robotMiddle()) {
    //         robot.motor.spinRight(lineSensor.offTrackAmount() * 20); // Example: Spin right at speed 150
    //         robot.motor.goForward(90, 90); // Move forward at speed 150
    //         _delay_ms(2000);
    //     }
    //     robot.motor.goForward(90, 90); // Move forward at speed 150
    // }
    Action currentAction = Action::FIRST_TURN;

    while (true) {
        movementLogic(currentAction);
        switchLogic(currentAction);
    }
}

