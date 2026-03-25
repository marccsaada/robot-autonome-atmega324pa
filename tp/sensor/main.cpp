#include <libstatique.hpp>
#include "lineSensor.hpp"

int main() {
    LineSensor lineSensor;
    UART uart;
    while (true) {
        if (lineSensor.robotMiddle()) {
            uart.UART_Transmission("Robot is in the middle of the line. \n");
        } else if (lineSensor.robotOffTrackLeft()) {
            uart.UART_Transmission("Robot is off track to the left. \n");
        } else if (lineSensor.robotOffTrackRight()) {
            uart.UART_Transmission("Robot is off track to the right. \n");
        } else if (lineSensor.robotBumpLine()) {
            uart.UART_Transmission("Robot has bumped into the line. \n");
        } else if (lineSensor.isLeftWall()) {
            uart.UART_Transmission("Robot is against the left wall. \n");
        } else if (lineSensor.isRightWall()) {
            uart.UART_Transmission("Robot is against the right wall. \n");
        } else if (lineSensor.foundDamage()) {
            uart.UART_Transmission("Robot has found damage. \n");
        }
    }
}