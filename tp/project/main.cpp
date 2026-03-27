#include "libstatique.hpp"
#include "lineSensor.hpp"

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
    SECOND_TURN,
    END
};

int main() {
    LineSensor lineSensor;
    Robot robot;
    Action currentAction = Action::PARKING;
    while (true) {
        switch (currentAction) {
            case Action::PARKING:
                robot.goBackward(180);
                _delay_ms(1000);
                robot.stopMotor();
                _delay_ms(1000);
                robot.turnRight(90); // Turn left 180 degrees to face the line
                robot.stopMotor();
                _delay_ms(1000);
                robot.goForward(200); // Move forward at speed 200 until bumping the line
                _delay_ms(700);

                currentAction = Action::FIRST_TURN; // Transition to the first turn
                break;

            case Action::FIRST_TURN:
                // Implement logic for the first turn
                while (!lineSensor.robotBumpLine()) {
                }
                robot.turnLeft(90); // Example: Turn right 90 degrees
                robot.goForward(200); // Move forward at speed 200
                // Transition to SECOND_TURN based on conditions
                if (lineSensor.robotBumpLine()) {
                    currentAction = Action::SECOND_TURN; // Transition to the second turn
                }
                break;

            case Action::SECOND_TURN:
                // Implement logic for the second turn
                robot.turnLeft(90); // Example: Turn right 90 degrees
                robot.goForward(200); // Move forward at speed 200
                if (lineSensor.foundDamage()) {
                    robot.playSound(60); // Play a sound to indicate damage found
                    robot.turnOnGreen(); // Turn on green LED to indicate damage found
                }
                // Transition back to PARKING or other states based on conditions
                if (lineSensor.robotBumpLine()) {
                    currentAction = Action::END; // Transition to the second turn
                }
                break;

            case Action::END:
                robot.stopMotor(); // Stop the robot
                robot.turnOffLED(); // Turn off LEDs
                break;
        }
    }
    

}