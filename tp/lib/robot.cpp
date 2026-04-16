/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP Final
Section # : 05
Équipe # : 107111

Description : Implémentation de l'interface unifiée du robot : mouvement, LED, son,
              et logique complète d'exécution du projet (machine à états).
*/

#include <libstatique.hpp>
#include <robot.hpp>

// ticks est défini volatile dans le main (project.cpp) et incrémenté par l'ISR.
extern volatile uint16_t ticks;

// ── Helper fichier-local ──────────────────────────────────────────────────────

static void printLine(UART& uart, const char* name, int count, const char* type) {
    uart.UART_Transmission(name);
    if (count == 0) {
        uart.UART_Transmission("         oui\r\n");
    } else {
        uart.UART_Transmission("         non      ");
        uart.UART_Transmission(count + '0');
        uart.UART_Transmission(" ");
        uart.UART_Transmission(type);
        uart.UART_Transmission("\r\n");
    }
}

// ── Méthodes existantes ───────────────────────────────────────────────────────

void Robot::wait(uint16_t multiplicator) {
    for (uint16_t i = 0; i < multiplicator; ++i)
        _delay_ms(25);
}

void Robot::playEepromNotes() {
    Memory   mem;
    uint16_t size  = mem.getSize();
    uint8_t  notes[3];
    uint8_t  found = 0;

    for (uint16_t i = 0; i < size && found < 3; i += 2) {
        uint8_t instr   = mem.readInstruction();
        uint8_t operand = mem.readOperand();
        if (instr == SGO)
            notes[found++] = operand;
    }

    for (uint8_t i = 0; i < found; i++) {
        if (i > 0) _delay_ms(NOTE_GAP_MS);
        sound.playSound(notes[i]);
        _delay_ms(NOTE_DURATION_MS);
        sound.stopSound();
    }
}

// ── Handlers de mode ─────────────────────────────────────────────────────────

void Robot::runRapport() {
    for (uint8_t i = 0; i < 8; i++) {
        led.red();   _delay_ms(125);
        led.off();   _delay_ms(125);
    }

    UART          uart;
    Memoire24CXXX eeprom;
    static const uint8_t  NB_ZONES         = 6;
    static const uint16_t EEPROM_ADDR_BASE = 10;

    const char* noms[NB_ZONES] = {
        "Local A      ", "Local B      ", "Local C      ",
        "Local D      ", "Couloir OUEST", "Couloir EST  ",
    };
    const char* types[NB_ZONES] = {
        "personne(s)", "objet(s)", "objet(s)",
        "personne(s)", "zone(s) endommagee(s)", "zone(s) endommagee(s)"
    };
    uint8_t valeurs[NB_ZONES] = {0};

    for (uint8_t i = 0; i < NB_ZONES; i++)
        eeprom.lecture(EEPROM_ADDR_BASE + i, &valeurs[i]);

    uart.UART_Transmission("Rapport de conformite\r\n\r\n");
    uart.UART_Transmission("Emplacement     conformite     detail\r\n");
    uart.UART_Transmission("--------------------------------------------------\r\n");
    for (uint8_t i = 0; i < NB_ZONES; i++)
        printLine(uart, noms[i], valeurs[i], types[i]);
}

void Robot::runInstruction() {
    for (uint8_t i = 0; i < 8; i++) {
        led.green(); _delay_ms(125);
        led.off();   _delay_ms(125);
    }

    UART          uart;
    Memoire24CXXX memoire;
    led.red();

    uint8_t  msb    = uart.UART_Reception();
    uint8_t  lsb    = uart.UART_Reception();
    uint16_t taille = ((uint16_t)msb << 8) | lsb;

    memoire.ecriture(0, msb); _delay_ms(5);
    memoire.ecriture(1, lsb); _delay_ms(5);

    for (uint16_t i = 2; i < taille; i++) {
        memoire.ecriture(i, uart.UART_Reception());
        _delay_ms(5);
    }

    Interpreter interpreter(*this);
    interpreter.confirm();
}

// ── Helpers de mouvement (privés) ─────────────────────────────────────────────

void Robot::turnLeft() {
    uint8_t       count      = lineSensor.offTrackAmount();
    uint8_t       leftSpeed;
    const uint8_t rightSpeed = RIGHT_SPEED + 18;

    if      (count == 5 || lineSensor.offTrackLeft()) leftSpeed = 0;
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

    if      (count == 5 || lineSensor.offTrackLeft()) rightSpeed = 0;
    else if (count == 4) rightSpeed = 0;
    else if (count == 3) rightSpeed = 0;
    else if (count == 2) rightSpeed = 95;
    else if (count == 1) rightSpeed = 100;
    else                 rightSpeed = RIGHT_SPEED;

    motor.goForward(leftSpeed, rightSpeed);
}

void Robot::turn()         { if (direction == 0) turnLeft();  else turnRight(); }
void Robot::turnOpposite() { if (direction == 0) turnRight(); else turnLeft();  }

void Robot::followPath(Alignment alignment, Speed speed) {
    uint8_t leftWheelSpeed  = LEFT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_SPEED;

    if      (speed == Speed::SLOW)    { leftWheelSpeed = LEFT_SPEED  - 20; rightWheelSpeed = RIGHT_SPEED - 20; }
    else if (speed == Speed::DEFAULT) { leftWheelSpeed = LEFT_SPEED;       rightWheelSpeed = RIGHT_SPEED;      }
    else if (speed == Speed::FAST)    { leftWheelSpeed = LEFT_SPEED  + 30; rightWheelSpeed = RIGHT_SPEED + 30; }

    if      (alignment == Alignment::LEFT)  { leftWheelSpeed = LEFT_SPEED - 10; rightWheelSpeed = RIGHT_SPEED + 4;  }
    else if (alignment == Alignment::RIGHT) { leftWheelSpeed = LEFT_SPEED + 4;  rightWheelSpeed = RIGHT_SPEED - 10; }

    if (lineSensor.offTrackLeft()) {
        leftWheelSpeed  += lineSensor.offTrackAmount() * 30;
        rightWheelSpeed -= lineSensor.offTrackAmount() * 5;
    } else if (lineSensor.offTrackRight()) {
        rightWheelSpeed += lineSensor.offTrackAmount() * 30;
        leftWheelSpeed  -= lineSensor.offTrackAmount() * 5;
    }
    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followLine() {
    if      (lineSensor.getSensor(1)) motor.goForward(LEFT_SPEED / 1.3, RIGHT_SPEED);
    else if (lineSensor.getSensor(3)) motor.goForward(LEFT_SPEED, RIGHT_SPEED / 1.3);
    else                              motor.goForward(LEFT_SPEED, RIGHT_SPEED);
}

void Robot::followLeftWall() {
    uint8_t leftWheelSpeed  = LEFT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_SPEED;

    if (!lineSensor.isLeftWall()) {
        leftWheelSpeed  -= lineSensor.offTrackAmount() * 10;
        rightWheelSpeed += lineSensor.offTrackAmount() * 2;
    }
    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followRightWall() {
    uint8_t leftWheelSpeed;
    uint8_t rightWheelSpeed;

    bool s3 = lineSensor.getSensor(2);
    bool s4 = lineSensor.getSensor(3);
    bool s5 = lineSensor.getSensor(4);

    if      (s4 && s5) { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED;      }
    else if (s5)        { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED + 32; }
    else if (s3 && s4)  { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED + 32; }
    else                { leftWheelSpeed = LEFT_SPEED + 17; rightWheelSpeed = RIGHT_SPEED;      }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followWall() {
    if (direction == 0) followRightWall();
    else                followLeftWall();
}

void Robot::alignToTurn() {
    motor.stop();
    _delay_ms(1000);

    if (!lineSensor.robotBumpLine()) {
        while (!lineSensor.robotBumpLine())
            motor.goBackward(LEFT_SPEED, RIGHT_SPEED);
        motor.stop();
    }

    if (direction == 0) motor.goForward(0, RIGHT_SPEED);
    else                motor.goForward(LEFT_SPEED, 0);
    _delay_ms(1500);
}

bool Robot::confirmTurn() {
    if (!lineSensor.robotMiddle()) return false;

    if (lineSensor.robotBumpLine()) {
        while (!lineSensor.robotBumpLine())
            motor.goBackward(LEFT_SPEED, RIGHT_SPEED);
        motor.stop();
        return false;
    }
    return true;
}

void Robot::detectObject(EEPROMAddress addr) {
    lineSensor.resetObjectCount();
    timer.startTimer();
    motor.goForward(110, 110);
    while (ticks < 230)
        lineSensor.findObject(addr);
    led.off();
}

bool Robot::foundRoom() {
    while (!(direction == 0
        ? (lineSensor.getSensor(1) || lineSensor.getSensor(2) || lineSensor.getSensor(3))
        : (lineSensor.getSensor(3) || lineSensor.getSensor(4) || lineSensor.getSensor(5)))) {
        followPath(direction == 0 ? Alignment::LEFT : Alignment::RIGHT, Speed::DEFAULT);
    }
    _delay_ms(333);
    return true;
}

bool Robot::foundRoom2() {
    bool bumpRoom = direction == 0
        ? (lineSensor.getSensor(1) || lineSensor.getSensor(2) || lineSensor.getSensor(3))
        : (lineSensor.getSensor(3) || lineSensor.getSensor(4) || lineSensor.getSensor(5));

    if (direction == 0 ? lineSensor.offTrackLeft() : lineSensor.offTrackRight()) {
        ticks = 0;
        return false;
    }
    if (ticks >= 200 || bumpRoom) {
        timer.stopTimer();
        ticks = 0;
        return true;
    }
    return false;
}

// ── Machine à états ───────────────────────────────────────────────────────────

void Robot::movementLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {
        case Action::PARKING:
            if (stepCount == 0) {
                if (direction == 0) motor.goBackward(LEFT_SPEED, 0);
                else                motor.goBackward(0, RIGHT_SPEED);
                _delay_ms(2380);
                motor.stop();
                stepCount++;
            }
            if (stepCount == 1) {
                if (direction == 0) motor.goForward(0, RIGHT_SPEED);
                else                motor.goForward(LEFT_SPEED, 0);
                _delay_ms(1380);
                while (lineSensor.offTrackAmount() < 4) {}
                motor.stop();
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
            while (ticks < 250){
                followRightWall();
                if (lineSensor.isOnRightLine()){
                    ticks = 0;
                }
            }
            timer.stopTimer();
            ticks = 0;

            timer.startTimer();
            while (ticks < 150 && !lineSensor.robotBumpLine()) {
                followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;
            while (!lineSensor.robotBumpLine()) {
                followPath(Alignment::DEFAULT, Speed::SLOW);
            }

            break;


        case Action::FIRST_TURN:
            turn();
            break;

        case Action::FIRST_CORRIDOR:
            lineSensor.resetDamage();
            timer.startTimer();
            while (ticks < 400 && !lineSensor.robotBumpLine()) {
                if (lineSensor.findDamage(DAMAGE_EST)) followLine();
                else                                   followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;

            while (!lineSensor.robotBumpLine())
                followPath(Alignment::DEFAULT, Speed::SLOW);
            break;

        case Action::SECOND_TURN:
            turn();
            break;

        case Action::SECOND_CORRIDOR:
            if (roomCount == 0) {
                timer.startTimer();
                while (ticks < 245) followPath(Alignment::DEFAULT, Speed::DEFAULT);
                motor.stop();
                _delay_ms(500);
                timer.stopTimer();
                ticks = 0;
                if (direction == 0) motor.spinLeft(135);
                else                motor.spinRight(135);
                motor.stop();
                _delay_ms(800);
            } else if (roomCount == 1 || roomCount == 2) {
                ticks = 0;
                timer.startTimer();
                while (!foundRoom2())
                    followPath(direction == 0 ? Alignment::LEFT : Alignment::RIGHT, Speed::DEFAULT);
                motor.stop();
                _delay_ms(150);
                if (direction == 0) motor.spinLeft(roomCount == 1 ? 80 : 74);
                else                motor.spinRight(roomCount == 1 ? 80 : 74);
                motor.stop();
                _delay_ms(800);
            } else if (roomCount == 3) {
                ticks = 0;
                timer.startTimer();
                while (!foundRoom2())
                    followPath(direction == 0 ? Alignment::LEFT : Alignment::RIGHT, Speed::DEFAULT);
                motor.stop();
                _delay_ms(150);
                if (direction == 0) motor.spinLeft(110);
                else                motor.spinRight(110);
                motor.stop();
                _delay_ms(800);
            }
            break;

        case Action::PEOPLE_ROOM:
            if      (roomCount == 0)
                distanceSensor.scanRoom(*this, direction == 0 ? PERSON_D : PERSON_A, direction == 0 ? LEFT : RIGHT);
            else if (roomCount == 3)
                distanceSensor.scanRoom(*this, direction == 0 ? PERSON_A : PERSON_D, direction == 0 ? LEFT : RIGHT);

            while (!(lineSensor.offTrackLeft() && lineSensor.offTrackAmount() < 3))
                motor.spinRightSpeed(100);
                motor.stop();
               _delay_ms(500);

            while (!lineSensor.robotMiddle())
                followPath(Alignment::DEFAULT, Speed::DEFAULT);

            break;

        case Action::OBJECT_ROOM:
            if      (roomCount == 1) detectObject(direction == 0 ? OBJECT_C : OBJECT_B);
            else if (roomCount == 2) detectObject(direction == 0 ? OBJECT_B : OBJECT_C);

            while (!lineSensor.robotBumpLine())
                motor.goForward(110, 110);
            motor.stop();
            _delay_ms(2000);

            if (direction == 0) motor.spinRight(180);
            else                motor.spinLeft(180);
            motor.stop();
            _delay_ms(1000);

            timer.stopTimer();
            ticks = 0;

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

            if (direction == 0) motor.goForward(0, RIGHT_SPEED);
            else                motor.goForward(LEFT_SPEED, 0);
            _delay_ms(700);

            if (direction == 0)
                while (!(lineSensor.offTrackAmount() <= 3 && lineSensor.offTrackRight())) {}
            else
                while (!(lineSensor.offTrackAmount() <= 3 && lineSensor.offTrackLeft()))  {}

            while (!lineSensor.robotMiddle()) turn();
            previousAction = Action::OBJECT_ROOM;
            break;

        case Action::THIRD_TURN:
            turn();
            break;

        case Action::THIRD_CORRIDOR:
            lineSensor.resetDamage();
            timer.startTimer();
            while (ticks < 400 && !lineSensor.robotBumpLine()) {
                if (lineSensor.findDamage(DAMAGE_OUEST)) followLine();
                else                                     followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;

            while (!lineSensor.robotBumpLine())
                followPath(Alignment::DEFAULT, Speed::SLOW);
            break;

        case Action::FOURTH_TURN:
            turn();
            break;

        case Action::ENTER_PARKING:
            followRightWall();
            break;

        case Action::COMPLETE_PARKING:
            led.green();
            timer.startTimer();
            if (direction == 0) while (ticks < 240) motor.goForward(95,  160);
            else                while (ticks < 240) motor.goForward(160, 95);
            timer.stopTimer();
            ticks = 0;

            motor.goForward(LEFT_SPEED - 40, RIGHT_SPEED - 40);
            _delay_ms(400);

            while (lineSensor.offTrackAmount() > 0)
                motor.goBackward(LEFT_SPEED - 40, RIGHT_SPEED - 40);
            break;

        case Action::END:
            motor.stop();
            for (uint8_t i = 0; i < lineSensor.getNDamage(); i++) {
                led.green(); _delay_ms(1000);
                led.off();   _delay_ms(1000);
            }
            break;
    }
}

void Robot::switchLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {
        case Action::PARKING:
            if (lineSensor.robotBumpLine()) { _delay_ms(500); currentAction = Action::LEAVE_PARKING; }
            break;

        case Action::LEAVE_PARKING:
            if (lineSensor.robotMiddle()) { _delay_ms(500); currentAction = Action::AFTER_PARKING; }
            break;

        case Action::AFTER_PARKING:
            if (lineSensor.robotBumpLine()) currentAction = Action::FIRST_TURN;
            break;

        case Action::FIRST_TURN:
            if (confirmTurn()) currentAction = Action::FIRST_CORRIDOR;
            break;

        case Action::FIRST_CORRIDOR:
            if (lineSensor.robotBumpLine()) {
                lineSensor.setPreviousDamageState(false);
                led.off();
                currentAction = Action::SECOND_TURN;
            }
            break;

        case Action::SECOND_TURN:
            if (confirmTurn()) currentAction = Action::SECOND_CORRIDOR;
            break;

        case Action::SECOND_CORRIDOR:
            if      (roomCount == 0 || roomCount == 3) currentAction = Action::PEOPLE_ROOM;
            else if (roomCount == 1 || roomCount == 2) currentAction = Action::OBJECT_ROOM;
            else if (roomCount == 4) {
                while (!lineSensor.robotBumpLine()) followPath(Alignment::DEFAULT, Speed::SLOW);
                currentAction = Action::THIRD_TURN;
            }
            break;

        case Action::PEOPLE_ROOM:
            if (lineSensor.robotMiddle()) { roomCount++; currentAction = Action::SECOND_CORRIDOR; }
            break;

        case Action::OBJECT_ROOM:
            if (lineSensor.robotMiddle()) { roomCount++; currentAction = Action::SECOND_CORRIDOR; }
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
            if (direction == 0 ? lineSensor.isOnRightLine() : lineSensor.isOnLeftLine()) {
                parkingCount++;
                if (parkingCount == 2) { currentAction = Action::COMPLETE_PARKING; break; }
                led.red();
                timer.startTimer();
                while (ticks < 40) followPath(Alignment::RIGHT, Speed::DEFAULT);
                led.off();
                timer.stopTimer();
                ticks = 0;
            }
            break;

        case Action::COMPLETE_PARKING:
            currentAction = Action::END;
            break;

        case Action::END:
            while (true) {}
            break;
    }
}

// ── Point d'entrée du mode EXECUTION ─────────────────────────────────────────

void Robot::runProject() {
    timer.setModeCTC(Timer::PRESCALE_64);
    timer.setOCRA(OCR1A_10MS);
    sei();

    _delay_ms(500);

    // Réinitialise l'état de la machine à états à chaque exécution
    stepCount    = 0;
    firstTime    = true;
    roomCount    = 0;
    parkingCount = 0;

    Action currentAction  = Action::FIRST_CORRIDOR;
    Action previousAction = static_cast<Action>(-1);

    while (true) {
        movementLogic(currentAction, previousAction);
        switchLogic(currentAction, previousAction);
    }
}
