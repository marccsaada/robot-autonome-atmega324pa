/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis
Ben Boudaoud Travail : TP Final Section # : 05 Équipe # : 107111

Description : Implémentation de l'interface unifiée du robot : mouvement, LED,
son, et logique complète d'exécution du projet (machine à états).
*/

/*
 * ========================= MACHINE A ETATS (runProject) =========================
 *
 * La boucle principale appelle movementLogic() (action continue dans l'état courant)
 * puis switchLogic() (test de transition vers l'état suivant). Les entrées sont des
 * conditions de capteurs (ligne, distance, bouton), les sorties sont des commandes
 * envoyées aux moteurs et à la DEL.
 *
 * +------------------+----------------------+------------------+------------------------------+
 * |                  |        INPUT         |                  |           OUTPUT             |
 * |  État actuel     +----------------------+   État suivant   +------------------------------+
 * |                  |  Condition capteur   |                  |         Action robot         |
 * +------------------+----------------------+------------------+------------------------------+
 * | PARKING          | !robotBumpLine()     | PARKING          | Recule en diagonale          |
 * | PARKING          | robotBumpLine()      | LEAVE_PARKING    | Arrêt (délai 500 ms)         |
 * +------------------+----------------------+------------------+------------------------------+
 * | LEAVE_PARKING    | !robotMiddle()       | LEAVE_PARKING    | Manoeuvre vers la ligne      |
 * | LEAVE_PARKING    | robotMiddle()        | AFTER_PARKING    | Arrêt (délai 500 ms)         |
 * +------------------+----------------------+------------------+------------------------------+
 * | AFTER_PARKING    | !robotBumpLine()     | AFTER_PARKING    | Suit le mur (vitesse SLOW)   |
 * | AFTER_PARKING    | robotBumpLine()      | FIRST_TURN       | Arrêt                        |
 * +------------------+----------------------+------------------+------------------------------+
 * | FIRST_TURN       | !confirmTurn()       | FIRST_TURN       | Tourne (turnYBB)             |
 * | FIRST_TURN       | confirmTurn()        | FIRST_CORRIDOR   | (transition directe)         |
 * +------------------+----------------------+------------------+------------------------------+
 * | FIRST_CORRIDOR   | toujours             | SECOND_TURN      | DEL verte, suit le couloir   |
 * +------------------+----------------------+------------------+------------------------------+
 * | SECOND_TURN      | toujours             | SECOND_CORRIDOR  | (transition directe)         |
 * +------------------+----------------------+------------------+------------------------------+
 * | SECOND_CORRIDOR  | roomCount = 0 ou 3   | PEOPLE_ROOM      | Avance vers le local         |
 * | SECOND_CORRIDOR  | roomCount = 1 ou 2   | OBJECT_ROOM      | Avance vers le local         |
 * | SECOND_CORRIDOR  | roomCount = 4        | THIRD_TURN       | Suit le couloir (SLOW)       |
 * +------------------+----------------------+------------------+------------------------------+
 * | PEOPLE_ROOM      | !robotMiddle()       | PEOPLE_ROOM      | Scan IR, cherche poteaux     |
 * | PEOPLE_ROOM      | robotMiddle()        | SECOND_CORRIDOR  | roomCount++                  |
 * +------------------+----------------------+------------------+------------------------------+
 * | OBJECT_ROOM      | !robotMiddle()       | OBJECT_ROOM      | Suit ligne, cherche objets   |
 * | OBJECT_ROOM      | robotMiddle()        | SECOND_CORRIDOR  | roomCount++                  |
 * +------------------+----------------------+------------------+------------------------------+
 * | THIRD_TURN       | !confirmTurn()       | THIRD_TURN       | Tourne                       |
 * | THIRD_TURN       | confirmTurn()        | THIRD_CORRIDOR   | (transition directe)         |
 * +------------------+----------------------+------------------+------------------------------+
 * | THIRD_CORRIDOR   | !robotBumpLine()     | THIRD_CORRIDOR   | Suit le mur                  |
 * | THIRD_CORRIDOR   | robotBumpLine()      | FOURTH_TURN      | Arrêt                        |
 * +------------------+----------------------+------------------+------------------------------+
 * | FOURTH_TURN      | !confirmTurn()       | FOURTH_TURN      | Tourne                       |
 * | FOURTH_TURN      | confirmTurn()        | ENTER_PARKING    | (transition directe)         |
 * +------------------+----------------------+------------------+------------------------------+
 * | ENTER_PARKING    | parkingCount < op    | ENTER_PARKING    | Suit mur, compte les lignes  |
 * | ENTER_PARKING    | parkingCount = op    | COMPLETE_PARKING | (transition directe)         |
 * +------------------+----------------------+------------------+------------------------------+
 * | COMPLETE_PARKING | toujours             | END              | (transition directe)         |
 * +------------------+----------------------+------------------+------------------------------+
 * | END              | toujours             | END              | Boucle infinie               |
 * +------------------+----------------------+------------------+------------------------------+
 *
 * Note : "op" désigne parkingOperand, lu dans la mémoire EEPROM externe par
 *        readEepromOperands() au début du runProject().
 * ================================================================================
 */

#include <libstatique.hpp>
#include <robot.hpp>

// ticks est défini volatile dans le main (project.cpp) et incrémenté par l'ISR.
extern volatile uint16_t ticks;

// ── Helper fichier-local
// ──────────────────────────────────────────────────────

void printLine(UART& uart, const char* name, int count, const char* type) {
    uart.UART_Transmission(name);
    if (count == 0) {
        uart.UART_Transmission("         oui\r\n");
    } else {
        uart.UART_Transmission("         non      ");
        uart.UART_Transmission(count + '0');
        uart.UART_Transmission(type);
        uart.UART_Transmission("\r\n");
    }
}

void Robot::wait(uint16_t multiplicator) {
    for (uint16_t i = 0; i < multiplicator; ++i) _delay_ms(25);
}

void Robot::playEepromNotes() {
    Memory mem;
    uint16_t size = mem.getSize();
    uint8_t found = 0;

    for (uint16_t i = 0; i < size; i += 2) {
        uint8_t instr = mem.readInstruction();
        uint8_t operand = mem.readOperand();

        if (instr == SGO && found < 3) {
            _delay_ms(NOTE_GAP_MS);
            sound.playSound(operand);
            _delay_ms(NOTE_DURATION_MS);
            sound.stopSound();
            found++;
        }
    }
}

void Robot::readEepromOperands() {
    Memory mem;
    uint16_t size = mem.getSize();

    while (mem.getAddress() < size) {
        uint8_t instr = mem.readInstruction();
        uint8_t operand = mem.readOperand();
        if (instr == TRD) direction = 1;
        if (instr == TRG) direction = 0;
        if (instr == ATT) {
            if (direction == 0) {
                parkingOperand = operand;
            } else if (direction == 1) {
                parkingOperand = 5 - operand;
            }
        }
    }
}

void Robot::runRapport() {
    for (uint8_t i = 0; i < 8; i++) {
        led.red();
        _delay_ms(125);
        led.off();
        _delay_ms(125);
    }

    UART uart;
    Memoire24CXXX eeprom;
    static const uint8_t NB_ZONES = 6;
    static const uint16_t EEPROM_ADDR_BASE = 100;

    const char* noms[NB_ZONES] = {
        "Local A      ", "Local B      ", "Local C      ",
        "Local D      ", "Couloir OUEST", "Couloir EST  ",
    };
    const char* types[NB_ZONES] = {" personne(s)",
                                   " objet(s)",
                                   " objet(s)",
                                   " personne(s)",
                                   " zone(s) endommagee(s)",
                                   " zone(s) endommagee(s)"};
    uint8_t valeurs[NB_ZONES] = {0};

    for (uint8_t i = 0; i < NB_ZONES; i++)
        eeprom.lecture(EEPROM_ADDR_BASE + i, &valeurs[i]);

    uart.UART_Transmission("Rapport de conformite\r\n\r\n");
    uart.UART_Transmission("Emplacement     conformite     detail\r\n");
    uart.UART_Transmission(
        "--------------------------------------------------\r\n");
    for (uint8_t i = 0; i < NB_ZONES; i++)
        printLine(uart, noms[i], valeurs[i], types[i]);
}

void Robot::runInstruction() {
    for (uint8_t i = 0; i < 8; i++) {
        led.green();
        _delay_ms(125);
        led.off();
        _delay_ms(125);
    }

    UART uart;
    Memoire24CXXX memoire;
    led.red();

    uint8_t msb = uart.UART_Reception();
    uint8_t lsb = uart.UART_Reception();
    uint16_t taille = ((uint16_t)msb << 8) | lsb;

    memoire.ecriture(0, msb);
    _delay_ms(5);
    memoire.ecriture(1, lsb);
    _delay_ms(5);

    for (uint16_t i = 2; i < taille; i++) {
        memoire.ecriture(i, uart.UART_Reception());
        _delay_ms(5);
    }

    Interpreter interpreter(*this);
    interpreter.confirm();
}

void Robot::turnLeft() {
    uint8_t       count      = lineSensor.offTrackAmount();
    uint8_t       leftSpeed;
    const uint8_t rightSpeed = RIGHT_SPEED + 18;

    if      (count == 5 || count == 0) leftSpeed = 0;
    else if (count == 4) leftSpeed = 0;
    else if (count == 3) leftSpeed = 0;
    else if (count == 2) leftSpeed = 95;
    else if (count == 1) leftSpeed = 100;
    else                 leftSpeed = LEFT_SPEED;

    motor.goForward(leftSpeed, rightSpeed);
}

void Robot::turnRight() {
    uint8_t       count      = lineSensor.offTrackAmount();
    uint8_t       rightSpeed;
    const uint8_t leftSpeed  = LEFT_SPEED + 18;

    if      (count == 5 || count == 0) rightSpeed = 0;
    else if (count == 4) rightSpeed = 0;
    else if (count == 3) rightSpeed = 0;
    else if (count == 2) rightSpeed = 95;
    else if (count == 1) rightSpeed = 100;
    else                 rightSpeed = RIGHT_SPEED;

    motor.goForward(leftSpeed, rightSpeed);
}

void Robot::turn()         {
    if (direction == 0) turnLeft();  else turnRight(); }
void Robot::turnOpposite() { if (direction == 0) turnRight(); else turnLeft();  }


void Robot::followPath(Alignment alignment, Speed speed) {
    uint8_t leftWheelSpeed = 0;
    uint8_t rightWheelSpeed = 0;
    if (direction == 0) {
        uint8_t leftWheelSpeed = LEFT_SPEED + 9;
        uint8_t rightWheelSpeed = RIGHT_SPEED;
    } else {
        uint8_t leftWheelSpeed = LEFT_SPEED;
        uint8_t rightWheelSpeed = RIGHT_SPEED;
    }

    if (speed == Speed::SLOW) {
        leftWheelSpeed = LEFT_SPEED - 20;
        rightWheelSpeed = RIGHT_SPEED - 20;
    } else if (speed == Speed::DEFAULT) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (speed == Speed::FAST) {
        leftWheelSpeed = LEFT_SPEED + 30;
        rightWheelSpeed = RIGHT_SPEED + 30;
    }

    if (alignment == Alignment::LEFT) {
        leftWheelSpeed = LEFT_SPEED - 10;
        rightWheelSpeed = RIGHT_SPEED + 7;
    } else if (alignment == Alignment::RIGHT) {
        leftWheelSpeed = LEFT_SPEED + 7;
        rightWheelSpeed = RIGHT_SPEED - 10;
    }

    if (lineSensor.offTrackLeft()) {
        leftWheelSpeed += lineSensor.offTrackAmount() * 30;
        rightWheelSpeed -= lineSensor.offTrackAmount() * 5;
    } else if (lineSensor.offTrackRight()) {
        rightWheelSpeed += lineSensor.offTrackAmount() * 30;
        leftWheelSpeed -= lineSensor.offTrackAmount() * 5;
    }
    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followLine() {
    if (lineSensor.getSensor(1))
        motor.goForward(LEFT_SPEED / 1.3, RIGHT_SPEED);
    else if (lineSensor.getSensor(3))
        motor.goForward(LEFT_SPEED, RIGHT_SPEED / 1.3);
    else
        motor.goForward(LEFT_SPEED, RIGHT_SPEED);
}

void Robot::followLeftLine() {
    uint8_t leftWheelSpeed;
    uint8_t rightWheelSpeed;

    bool s1 = lineSensor.getSensor(0);
    bool s2 = lineSensor.getSensor(1);
    bool s3 = lineSensor.getSensor(2);
    bool s4 = lineSensor.getSensor(3);

    if (s1 && s2) {
        leftWheelSpeed = LEFT_SPEED + 33;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s2 && s3) {
        leftWheelSpeed = LEFT_SPEED + 33;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s1) {
        leftWheelSpeed = LEFT_SPEED - 25;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s2 || s3 || s4) {
        leftWheelSpeed = LEFT_SPEED + 25;
        rightWheelSpeed = RIGHT_SPEED;
    }

    else {
        leftWheelSpeed = LEFT_SPEED - 23;
        rightWheelSpeed = RIGHT_SPEED;
    }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followRightLine() {
    uint8_t leftWheelSpeed;
    uint8_t rightWheelSpeed;

    bool s2 = lineSensor.getSensor(1);
    bool s3 = lineSensor.getSensor(2);
    bool s4 = lineSensor.getSensor(3);
    bool s5 = lineSensor.getSensor(4);

    if (s4 && s5) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + 31;
    } else if (s3 && s4) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + 31;
    } else if (s5) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED - 25;
    } else if (s2 || s3 || s4) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + 25;
    } else {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED - 23;
    }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followRightWall(Speed speed) {
    uint8_t leftWheelSpeed = LEFT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_SPEED;

    if (speed == Speed::SLOW) {
        leftWheelSpeed = LEFT_SPEED - 35;
        rightWheelSpeed = RIGHT_SPEED - 35;
    } else if (speed == Speed::DEFAULT) {
        leftWheelSpeed = LEFT_SPEED - 20;
        rightWheelSpeed = RIGHT_SPEED - 20;
    } else if (speed == Speed::FAST) {
        leftWheelSpeed = LEFT_SPEED - 10;
        rightWheelSpeed = RIGHT_SPEED - 10;
    }

    bool s3 = lineSensor.getSensor(2);
    bool s4 = lineSensor.getSensor(3);
    bool s5 = lineSensor.getSensor(4);

    if (s4 && s5) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED - 5;
    } else if (s5) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + 50;
    } else if (s3 && s4) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + 50;
    } else {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED - 55;
    }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}
void Robot::followLeftWall(Speed speed) {
    uint8_t leftWheelSpeed = LEFT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_SPEED;

    if (speed == Speed::SLOW) {
        leftWheelSpeed = LEFT_SPEED - 35;
        rightWheelSpeed = RIGHT_SPEED - 35;
    } else if (speed == Speed::DEFAULT) {
        leftWheelSpeed = LEFT_SPEED - 20;
        rightWheelSpeed = RIGHT_SPEED - 20;
    } else if (speed == Speed::FAST) {
        leftWheelSpeed = LEFT_SPEED - 10;
        rightWheelSpeed = RIGHT_SPEED - 10;
    }

    bool s1 = lineSensor.getSensor(0);
    bool s2 = lineSensor.getSensor(1);
    bool s3 = lineSensor.getSensor(2);

    if (s1 && s2) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s1) {
        leftWheelSpeed = LEFT_SPEED + 55;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s2 && s3) {
        leftWheelSpeed = LEFT_SPEED + 55;
        rightWheelSpeed = RIGHT_SPEED;
    } else {
        leftWheelSpeed = LEFT_SPEED - 30;
        rightWheelSpeed = RIGHT_SPEED;
    }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}
void Robot::followWall(Speed speed) {
    if (direction == 0)
        followRightWall(speed);
    else
        followLeftWall(speed);
}

void Robot::alignToTurn() {
    motor.stop();
    _delay_ms(300);

    if (direction == 0)
        motor.goForward(0, RIGHT_SPEED - 10);
    else
        motor.goForward(LEFT_SPEED - 10, 0);
    _delay_ms(1750);
}

bool Robot::confirmTurn() {
    if ((direction == 0 ? lineSensor.offTrackRight()
                        : lineSensor.offTrackLeft()) &&
        lineSensor.offTrackAmount() <= 2)
        return false;
    else
        return true;
}

void Robot::detectObject(EEPROMAddress addr) {
    _delay_ms(500);
    lineSensor.resetObjectCount();

    timer.startTimer();
    motor.goForward(255, 255);
    _delay_ms(80);
    while (direction == 0 ? ticks < 198 : ticks < 196) {
        motor.goForward(LEFT_SPEED, RIGHT_SPEED);
        lineSensor.findObject(addr);
        if (lineSensor.offTrackLeft()) {
            motor.goForward(LEFT_SPEED,
                            RIGHT_SPEED + lineSensor.offTrackAmount());
        } else if (lineSensor.offTrackRight()) {
            motor.goForward(LEFT_SPEED, RIGHT_SPEED);
        }

        led.off();
    }

    eeprom.ecriture(addr, lineSensor.getNObjects());
    timer.stopTimer();
    ticks = 0;
}

void Robot::findRoom3() {
    if (direction == 0) {
        while (lineSensor.getSensor(0) || lineSensor.getSensor(1) ||
               lineSensor.getSensor(2))
            followLeftLine();
    } else {
        while (lineSensor.getSensor(2) || lineSensor.getSensor(3) ||
               lineSensor.getSensor(4))
            followRightLine();
    }
    timer.startTimer();

    ticks = 0;
    if (roomCount == 1) {
        while ((lineSensor.robotMiddle() ||
                (direction == 0 ? lineSensor.getSensor(4)
                                : lineSensor.getSensor(0))) &&
               ticks < 175) {
            followPath(direction == 0 ? Alignment::RIGHT : Alignment::DEFAULT, Speed::DEFAULT);
        }
    } else if (roomCount == 2) {
        while ((lineSensor.robotMiddle() ||
                (direction == 0 ? lineSensor.getSensor(4)
                                : lineSensor.getSensor(0))) &&
               ticks < 180) {
            followPath(direction == 0 ? Alignment::RIGHT : Alignment::DEFAULT, Speed::DEFAULT);
        }
    } else {
        while ((lineSensor.robotMiddle() ||
                (direction == 0 ? lineSensor.getSensor(4)
                                : lineSensor.getSensor(0))) &&
               ticks < 175) {
            followPath(direction == 0 ? Alignment::RIGHT : Alignment::DEFAULT, Speed::DEFAULT);
        }
    }

    timer.stopTimer();
    ticks = 0;

    _delay_ms(250);
    motor.stop();
    _delay_ms(250);

    if (direction == 0) {
        timer.startTimer();
        while (ticks < 90) {
            if (lineSensor.offTrackLeft() && lineSensor.offTrackAmount() < 2)
                break;
            motor.spinLeftSpeed(85);
        }

        timer.stopTimer();
        ticks = 0;
        while (
            !(lineSensor.offTrackLeft() && lineSensor.offTrackAmount() < 2)) {
            motor.spinRightSpeed(85);
        }
    } else {
        timer.startTimer();
        while (ticks < 90) {
            if (lineSensor.offTrackRight() && lineSensor.offTrackAmount() < 2)
                break;
            motor.spinRightSpeed(85);
        }

        timer.stopTimer();
        ticks = 0;

        while (
            !(lineSensor.offTrackRight() && lineSensor.offTrackAmount() < 2)) {
            motor.spinLeftSpeed(85);
        }
    }

    motor.stop();
    _delay_ms(250);
}

void Robot::turnCorner() {
    motor.goForward(LEFT_SPEED, RIGHT_SPEED);
    _delay_ms(700);

    while (direction == 0 ? !lineSensor.offTrackLeft()
                          : !lineSensor.offTrackRight()) {
        if (direction == 0)
            motor.goForward(0, RIGHT_SPEED);
        else
            motor.goForward(LEFT_SPEED, 0);
    }
    motor.stop();
    while (direction == 0 ? !lineSensor.offTrackRight()
                          : !lineSensor.offTrackLeft()) {
        if (direction == 0)
            motor.goForward(0, RIGHT_SPEED);
        else
            motor.goForward(LEFT_SPEED, 0);
    }

    while (!lineSensor.robotMiddle())
        followPath(Alignment::DEFAULT, Speed::DEFAULT);

    motor.stop();
    _delay_ms(500);
}

void Robot::movementLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {
        case Action::PARKING:
            if (stepCount == 0) {
                if (direction == 0)
                    motor.goBackward(LEFT_SPEED, 0);
                else
                    motor.goBackward(0, RIGHT_SPEED);
                _delay_ms(2380);
                motor.stop();
                stepCount++;
            }
            if (stepCount == 1) {
                if (direction == 0)
                    motor.goForward(0, RIGHT_SPEED);
                else
                    motor.goForward(LEFT_SPEED, 0);
                _delay_ms(1380);
                while (lineSensor.offTrackAmount() < 4) {
                }
                motor.stop();
                stepCount++;
            }
            break;

        case Action::LEAVE_PARKING:

            if (direction == 0)
                motor.goForward(0, RIGHT_SPEED - 10);

            else
                motor.goForward(LEFT_SPEED - 10, 0);
            _delay_ms(1800);
            while (true) {
                if ((direction == 0 ? lineSensor.offTrackRight()
                                    : lineSensor.offTrackLeft()) &&
                    lineSensor.offTrackAmount() < 3) {
                    while (!lineSensor.robotMiddle()) {
                    }

                    _delay_ms(50);
                    while (direction == 0 ? !lineSensor.isOnRightLine()
                                          : !lineSensor.isOnLeftLine()) {
                        if (direction == 0)
                            motor.goForward(LEFT_SPEED + 18, RIGHT_SPEED);
                        else
                            motor.goForward(LEFT_SPEED, RIGHT_SPEED + 10);
                        led.red();
                    }
                    led.off();

                    while (!lineSensor.robotMiddle()) {
                        if (direction == 0)
                            motor.goForward(72, RIGHT_SPEED);
                        else
                            motor.goForward(LEFT_SPEED, 72);
                    }
                    if (direction == 0)
                        motor.goForward(LEFT_SPEED, RIGHT_SPEED - 10);
                    else
                        motor.goForward(LEFT_SPEED - 10, RIGHT_SPEED);
                    break;
                }
            }
            break;

        case Action::AFTER_PARKING:
            timer.startTimer();
            while (ticks < 350 && !lineSensor.robotBumpLine()) {
                followWall(Speed::DEFAULT);
                if (direction == 0 ? lineSensor.isOnRightLine()
                                   : lineSensor.isOnLeftLine()) {
                    ticks = 0;
                }
            }
            timer.stopTimer();
            ticks = 0;

            while (!lineSensor.robotBumpLine()) {
                if (direction == 0)
                    followPath(Alignment::RIGHT, Speed::SLOW);
                else
                    followPath(Alignment::LEFT, Speed::SLOW);
            }
            break;

        case Action::FIRST_TURN:
            motor.stop();
            _delay_ms(500);
            turnCorner();
            break;

        case Action::FIRST_CORRIDOR:
            led.green();
            lineSensor.resetDamage();
            timer.startTimer();

            while (ticks < 150) followPath(Alignment::DEFAULT, Speed::DEFAULT);
            ticks = 0;

            while (ticks < 400 && !lineSensor.robotBumpLine()) {
                if (lineSensor.findDamage(direction == 0 ? DAMAGE_EST
                                                         : DAMAGE_OUEST))
                    followLine();
                else
                    followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;

            while (!lineSensor.robotBumpLine()) {
                if (direction == 0)
                    followPath(Alignment::RIGHT, Speed::SLOW);
                else
                    followPath(Alignment::LEFT, Speed::SLOW);
            }
            break;

        case Action::SECOND_TURN:
            motor.stop();
            _delay_ms(500);
            turnCorner();

            timer.startTimer();
            while (ticks < 130)
                followPath(direction == 0 ? Alignment::RIGHT : Alignment::LEFT,
                           Speed::DEFAULT);
            timer.stopTimer();
            ticks = 0;
            motor.stop();
            break;

        case Action::SECOND_CORRIDOR: {
            if (roomCount == 0) {
                timer.startTimer();
                while (direction == 0 ? ticks < 135 : ticks < 140) {
                    followPath(direction == 0 ? Alignment::LEFT : Alignment::RIGHT, Speed::SLOW);
                }
                timer.stopTimer();
                ticks = 0;

                motor.stop();
                _delay_ms(500);

                if (direction == 0)
                    motor.spinLeft(115);
                else
                    motor.spinRight(115);
                motor.stop();
                _delay_ms(800);

            } else if ((roomCount == 1) || (roomCount == 2)) {
                motor.stop();
                _delay_ms(500);

                while (!(direction == 0 ? (lineSensor.getSensor(0) || lineSensor.getSensor(1) || lineSensor.getSensor(2)) : 
                                          (lineSensor.getSensor(2) || lineSensor.getSensor(3) || lineSensor.getSensor(4)))) {
                    if (direction == 0)
                        followLeftLine();
                    else
                        followRightLine();
                }

                findRoom3();

                if (direction == 0)
                    motor.spinLeft(90);
                else
                    motor.spinRight(90);

            } else if (roomCount == 3) {
                motor.stop();
                _delay_ms(500);

                while (!(direction == 0 ? lineSensor.getSensor(0) || lineSensor.getSensor(1) || lineSensor.getSensor(2) :
                                          lineSensor.getSensor(2) || lineSensor.getSensor(3) || lineSensor.getSensor(4))) {
                    if (direction == 0)
                        followLeftLine();
                    else
                        followRightLine();
                }

                findRoom3();
                timer.startTimer();
                while (ticks < 40) {
                    motor.goForward(LEFT_SPEED, RIGHT_SPEED);
                }
                timer.stopTimer();
                ticks = 0;
                if (direction == 0)
                    motor.spinLeft(105);
                else
                    motor.spinRight(105);
                motor.stop();
                _delay_ms(800);
            }
            break;
        }

        case Action::PEOPLE_ROOM:
            motor.setPrescaler(Timer::PRESCALE_1024);
            if (roomCount == 0)
                distanceSensor.scanRoom(*this,
                                        direction == 0 ? PERSON_D : PERSON_A,
                                        direction == 0 ? LEFT : RIGHT);
            else if (roomCount == 3)
                distanceSensor.scanRoom(*this,
                                        direction == 0 ? PERSON_A : PERSON_D,
                                        direction == 0 ? LEFT : RIGHT);

            if (direction == 0)
                motor.spinRightSpeed(90);
            else
                motor.spinLeftSpeed(90);
            if (direction == 0) {
                while (!(lineSensor.offTrackLeft() &&
                         lineSensor.offTrackAmount() < 3)) {
                }
            } else {
                while (!(lineSensor.offTrackRight() &&
                         lineSensor.offTrackAmount() < 3)) {
                }
            }
            while (!(lineSensor.robotMiddle())) {
            }

            motor.resetPrescaler();

            while (!lineSensor.robotMiddle())
                followPath(Alignment::DEFAULT, Speed::DEFAULT);
            break;

        case Action::OBJECT_ROOM:
            if (roomCount == 1)
                detectObject(direction == 0 ? OBJECT_C : OBJECT_B);
            else if (roomCount == 2)
                detectObject(direction == 0 ? OBJECT_B : OBJECT_C);

            while (!lineSensor.robotBumpLine()) {
                motor.goForward(LEFT_SPEED, RIGHT_SPEED);
            }

            motor.stop();
            _delay_ms(500);

            if (direction == 0)
                motor.spinRight(150);
            else
                motor.spinLeft(145);

            motor.stop();
            _delay_ms(500);

            if (lineSensor.getNObjects() == 0) led.green();

            _delay_ms(500);

            timer.stopTimer();
            ticks = 0;

            motor.goForward(255, 255);
            _delay_ms(80);
            while (!lineSensor.robotBumpLine())
                motor.goForward(LEFT_SPEED, RIGHT_SPEED);
            motor.stop();
            _delay_ms(500);

            if (!lineSensor.robotBumpLine()) {
                motor.goBackward(200, 200);
                _delay_ms(100);
                while (!lineSensor.robotBumpLine())
                    motor.goBackward(LEFT_SPEED, RIGHT_SPEED);
                motor.stop();
            }


            if (direction == 0)
                motor.goForward(0, RIGHT_SPEED);
            else
                motor.goForward(LEFT_SPEED, 0);
            _delay_ms(700);                         // CHANGED old 700

            if (direction == 0)
                while (!(lineSensor.offTrackAmount() < 2 &&
                         lineSensor.offTrackRight())) {
                }
            else
                while (!(lineSensor.offTrackAmount() < 2 &&
                         lineSensor.offTrackLeft())) {
                }

            while (!lineSensor.robotMiddle()) turn();
            previousAction = Action::OBJECT_ROOM;
            break;

        case Action::THIRD_TURN:
            motor.stop();
            _delay_ms(500);

            turnCorner();

            break;

        case Action::THIRD_CORRIDOR:
            lineSensor.resetDamage();
            timer.startTimer();
            while (ticks < 150) followPath(Alignment::DEFAULT, Speed::DEFAULT);
            ticks = 0;
            while (ticks < 400 && !lineSensor.robotBumpLine()) {
                if (lineSensor.findDamage(direction == 0 ? DAMAGE_OUEST
                                                         : DAMAGE_EST))
                    followLine();
                else
                    followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;

            while (!lineSensor.robotBumpLine())
                followPath(direction == 0 ? Alignment::RIGHT : Alignment::LEFT,
                           Speed::SLOW);
            break;

        case Action::FOURTH_TURN:
            motor.stop();
            _delay_ms(500);
            turnCorner();
            break;

        case Action::ENTER_PARKING:
            break;

        case Action::COMPLETE_PARKING:
            timer.startTimer();
            if (direction == 0) while (ticks < 240) motor.goForward(95,  160);
            else                while (ticks < 240) motor.goForward(170, 95);
            timer.stopTimer();
            ticks = 0;
            while (lineSensor.robotMiddle()) {
                motor.goForward(LEFT_SPEED - 40, RIGHT_SPEED - 40);
            }
            if (lineSensor.offTrackRight()){
                motor.goBackward(LEFT_SPEED - 40, RIGHT_SPEED - 10);
            }
            else if (lineSensor.offTrackLeft()){
                motor.goBackward(LEFT_SPEED - 10, RIGHT_SPEED - 40);
            } else if (lineSensor.robotBumpLine()) {
                motor.goBackward(LEFT_SPEED - 40, RIGHT_SPEED - 40);
            }
            _delay_ms(900);

            break;

        case Action::END:
            motor.stop();
            sound.playSound(46);
            _delay_ms(1000);
            sound.stopSound();
            while (true) {
            }
            break;
    }
}

void Robot::switchLogic(Action& currentAction, Action& previousAction) {
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
            if (lineSensor.robotBumpLine()) currentAction = Action::FIRST_TURN;
            break;

        case Action::FIRST_TURN:
            if (confirmTurn()) currentAction = Action::FIRST_CORRIDOR;
            break;

        case Action::FIRST_CORRIDOR:
            lineSensor.setPreviousDamageState(false);
            led.off();
            currentAction = Action::SECOND_TURN;
            break;

        case Action::SECOND_TURN:
            currentAction = Action::SECOND_CORRIDOR;
            break;

        case Action::SECOND_CORRIDOR:
            if (roomCount == 0 || roomCount == 3)
                currentAction = Action::PEOPLE_ROOM;
            else if (roomCount == 1 || roomCount == 2)
                currentAction = Action::OBJECT_ROOM;
            else if (roomCount == 4) {
                while (!lineSensor.robotBumpLine())
                    followPath(
                        direction == 0 ? Alignment::RIGHT : Alignment::LEFT,
                        Speed::SLOW);
                currentAction = Action::THIRD_TURN;
            }
            break;

        case Action::PEOPLE_ROOM:
            if (lineSensor.robotMiddle()) {
                roomCount++;
                currentAction = Action::SECOND_CORRIDOR;
            }
            break;

        case Action::OBJECT_ROOM:
            if (lineSensor.robotMiddle()) {
                roomCount++;
                currentAction = Action::SECOND_CORRIDOR;
            }
            break;

        case Action::THIRD_TURN:
            if (confirmTurn()) currentAction = Action::THIRD_CORRIDOR;
            break;

        case Action::THIRD_CORRIDOR:
            if (lineSensor.robotBumpLine()) {
                lineSensor.setPreviousDamageState(false);
                led.off();
                currentAction = Action::FOURTH_TURN;
            }
            break;

        case Action::FOURTH_TURN:
            if (confirmTurn()) currentAction = Action::ENTER_PARKING;
            break;

        case Action::ENTER_PARKING:
            timer.startTimer();
            while (ticks < 350) {
                followWall(Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;

            while (parkingCount != parkingOperand) {
                followWall(Speed::DEFAULT);
                if (direction == 0 ? lineSensor.isOnRightLine()
                                   : lineSensor.isOnLeftLine()) {
                    parkingCount++;

                    timer.startTimer();

                    if (direction == 0) {
                        while (ticks < 50) followWall(Speed::DEFAULT);
                    } else {
                        while (ticks < 20) followWall(Speed::DEFAULT);
                    }

                    timer.stopTimer();
                    ticks = 0;
                }
            }
            currentAction = Action::COMPLETE_PARKING;
            break;

        case Action::COMPLETE_PARKING:
            currentAction = Action::END;
            break;

        case Action::END:
            while (true) {
            }
            break;
    }
}

void Robot::runProject() {
    timer.setModeCTC(Timer::PRESCALE_64);
    timer.setOCRA(OCR1A_10MS);
    sei();

    _delay_ms(500);

    stepCount = 0;
    firstTime = true;
    roomCount = 0;
    parkingCount = 0;

    for (uint8_t i = 0; i < 6; i++) {
        eeprom.ecriture(100 + i, 0);
        _delay_ms(5);
    }

    readEepromOperands();

    Action currentAction = Action::PARKING;
    Action previousAction = static_cast<Action>(-1);

    while (true) {
        movementLogic(currentAction, previousAction);
        switchLogic(currentAction, previousAction);
    }
}
