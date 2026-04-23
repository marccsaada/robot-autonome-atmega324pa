/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis
Ben Boudaoud Travail : TP Final Section # : 05 Équipe # : 107111

Description : Implémentation de l'interface unifiée du robot : mouvement, LED,
son, et logique complète d'exécution du projet (machine à états).
*/

#include <libstatique.hpp>
#include <robot.hpp>

extern volatile uint16_t ticks;

// ── Timing constants (ms) ─────────────────────────────────────────────────────
static const uint16_t WAIT_UNIT_MS                  = 25;
static const uint16_t STARTUP_BLINK_MS              = 125;
static const uint8_t  EEPROM_WRITE_DELAY_MS         = 5;
static const uint16_t ALIGN_STOP_DELAY_MS           = 300;
static const uint16_t ALIGN_TURN_DURATION_MS        = 1750;
static const uint16_t DETECT_INITIAL_DELAY_MS       = 500;
static const uint16_t BURST_DURATION_MS             = 80;
static const uint16_t ROOM3_STOP_DELAY_MS           = 250;
static const uint16_t CORNER_FORWARD_MS             = 700;
static const uint16_t STOP_DELAY_MS                 = 500;
static const uint16_t PARKING_BACKWARD_MS           = 2380;
static const uint16_t PARKING_FORWARD_MS            = 1380;
static const uint16_t LEAVE_PARKING_FORWARD_MS      = 1800;
static const uint16_t LEAVE_PARKING_STABILIZE_MS    = 50;
static const uint16_t COMPLETE_PARKING_DELAY_MS     = 900;
static const uint16_t END_SOUND_DURATION_MS         = 1000;
static const uint16_t BACKUP_DURATION_MS            = 100;
static const uint16_t ROOM_ENTRY_DELAY_MS           = 800;

// ── Count / shift constants ───────────────────────────────────────────────────
static const uint8_t  STARTUP_BLINK_COUNT           = 8;
static const uint8_t  EEPROM_ZONE_COUNT             = 6;
static const uint8_t  MSB_SHIFT                     = 8;
static const uint8_t  MAX_ATT_OPERAND               = 5;
static const uint8_t  END_SOUND_NOTE                = 46;

// ── Speed / PWM constants ─────────────────────────────────────────────────────
static const uint8_t  TURN_SPEED_BOOST              = 18;
static const uint8_t  TURN_LOW_SPEED               = 95;
static const uint8_t  TURN_MED_SPEED               = 100;
static const uint8_t  STRAIGHT_SPEED_TRIM           = 9;
static const uint8_t  SLOW_SPEED_REDUCTION          = 20;
static const uint8_t  FAST_SPEED_BOOST              = 30;
static const uint8_t  ALIGN_WHEEL_TRIM              = 10;
static const uint8_t  ALIGN_BOOST                   = 7;
static const uint8_t  OFFTRACK_BOOST                = 30;
static const uint8_t  OFFTRACK_REDUCTION            = 5;
static const float    FOLLOW_LINE_RATIO             = 1.3f;
static const uint8_t  LEFT_LINE_STRONG_BOOST        = 33;
static const uint8_t  LINE_ADJUST                   = 25;
static const uint8_t  LINE_FALLBACK                 = 23;
static const uint8_t  RIGHT_LINE_BOOST              = 31;
static const uint8_t  WALL_SLOW_REDUCTION           = 35;
static const uint8_t  WALL_DEFAULT_REDUCTION        = 20;
static const uint8_t  WALL_FAST_REDUCTION           = 10;
static const uint8_t  WALL_NEAR_REDUCTION           = 5;
static const uint8_t  WALL_BOOST                    = 50;
static const uint8_t  WALL_FAR_REDUCTION            = 55;
static const uint8_t  LEFT_WALL_BOOST               = 55;
static const uint8_t  LEFT_WALL_REDUCTION           = 30;
static const uint8_t  BURST_SPEED                   = 255;
static const uint8_t  PARTIAL_SPEED                 = 72;
static const uint8_t  OBJECT_ROOM_SPIN_RIGHT        = 150;
static const uint8_t  OBJECT_ROOM_SPIN_LEFT         = 145;
static const uint8_t  BACKUP_SPEED                  = 200;
static const uint8_t  COMPLETE_PARKING_SPEED_A      = 95;
static const uint8_t  COMPLETE_PARKING_SPEED_B      = 160;
static const uint8_t  COMPLETE_PARKING_SPEED_C      = 170;
static const uint8_t  COMPLETE_PARKING_REDUCTION    = 40;
static const uint8_t  COMPLETE_PARKING_FINE_TRIM    = 10;
static const uint8_t  ROOM3_SPIN_SPEED              = 85;
static const uint8_t  LEAVE_PARKING_SPEED_TRIM      = 10;

// ── Tick / threshold constants ────────────────────────────────────────────────
static const uint8_t  CONFIRM_OFFTRACK_MAX          = 2;
static const uint16_t DETECT_TICKS_DIR0             = 198;
static const uint16_t DETECT_TICKS_DIR1             = 196;
static const uint16_t ROOM3_TICKS_DEFAULT           = 175;
static const uint16_t ROOM3_TICKS_2                 = 180;
static const uint8_t  ROOM3_SPIN_TIMEOUT_TICKS      = 90;
static const uint8_t  ROOM3_OFFTRACK_THRESHOLD      = 2;
static const uint8_t  SECOND_TURN_TICKS             = 130;
static const uint8_t  SECOND_CORRIDOR_TICKS_DIR0    = 135;
static const uint8_t  SECOND_CORRIDOR_TICKS_DIR1    = 140;
static const uint8_t  ROOM_ENTRY_SPIN_ANGLE         = 115;
static const uint8_t  CORRIDOR_SPIN_ANGLE           = 90;
static const uint8_t  PEOPLE_ROOM_OFFTRACK_THRESHOLD = 3;
static const uint16_t CORRIDOR_INITIAL_TICKS        = 150;
static const uint16_t CORRIDOR_MAX_TICKS            = 400;
static const uint16_t WALL_ENTRY_WAIT_TICKS         = 350;
static const uint8_t  PARKING_OFFTRACK_THRESHOLD    = 4;
static const uint8_t  LEAVE_PARKING_OFFTRACK_MAX    = 3;
static const uint16_t COMPLETE_PARKING_TICKS        = 240;
static const uint8_t  ENTER_PARKING_TICKS_DIR0      = 50;
static const uint8_t  ENTER_PARKING_TICKS_DIR1      = 20;
static const uint8_t  ROOM3_FORWARD_TICKS           = 40;
static const uint8_t  ROOM3_SPIN_ANGLE              = 105;

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
    for (uint16_t i = 0; i < multiplicator; ++i) _delay_ms(WAIT_UNIT_MS);
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
                parkingOperand = MAX_ATT_OPERAND - operand;
            }
        }
    }
}

void Robot::runRapport() {
    for (uint8_t i = 0; i < STARTUP_BLINK_COUNT; i++) {
        led.red();
        _delay_ms(STARTUP_BLINK_MS);
        led.off();
        _delay_ms(STARTUP_BLINK_MS);
    }

    UART uart;
    Memoire24CXXX eeprom;

    const char* noms[EEPROM_ZONE_COUNT] = {
        "Local A      ", "Local B      ", "Local C      ",
        "Local D      ", "Couloir OUEST", "Couloir EST  ",
    };
    const char* types[EEPROM_ZONE_COUNT] = {" personne(s)",
                                   " objet(s)",
                                   " objet(s)",
                                   " personne(s)",
                                   " zone(s) endommagee(s)",
                                   " zone(s) endommagee(s)"};
    uint8_t valeurs[EEPROM_ZONE_COUNT] = {0};

    for (uint8_t i = 0; i < EEPROM_ZONE_COUNT; i++)
        eeprom.lecture(PERSON_A + i, &valeurs[i]);

    uart.UART_Transmission("Rapport de conformite\r\n\r\n");
    uart.UART_Transmission("Emplacement     conformite     detail\r\n");
    uart.UART_Transmission(
        "--------------------------------------------------\r\n");
    for (uint8_t i = 0; i < EEPROM_ZONE_COUNT; i++)
        printLine(uart, noms[i], valeurs[i], types[i]);
}

void Robot::runInstruction() {
    for (uint8_t i = 0; i < STARTUP_BLINK_COUNT; i++) {
        led.green();
        _delay_ms(STARTUP_BLINK_MS);
        led.off();
        _delay_ms(STARTUP_BLINK_MS);
    }

    UART uart;
    Memoire24CXXX memoire;
    led.red();

    uint8_t msb = uart.UART_Reception();
    uint8_t lsb = uart.UART_Reception();
    uint16_t taille = ((uint16_t)msb << MSB_SHIFT) | lsb;

    memoire.ecriture(0, msb);
    _delay_ms(EEPROM_WRITE_DELAY_MS);
    memoire.ecriture(1, lsb);
    _delay_ms(EEPROM_WRITE_DELAY_MS);

    for (uint16_t i = 2; i < taille; i++) {
        memoire.ecriture(i, uart.UART_Reception());
        _delay_ms(EEPROM_WRITE_DELAY_MS);
    }

    Interpreter interpreter(*this);
    interpreter.confirm();
}

void Robot::turnLeft() {
    uint8_t       count      = lineSensor.offTrackAmount();
    uint8_t       leftSpeed;
    const uint8_t rightSpeed = RIGHT_SPEED + TURN_SPEED_BOOST;

    if      (count == 5 || count == 0) leftSpeed = 0;
    else if (count == 4) leftSpeed = 0;
    else if (count == 3) leftSpeed = 0;
    else if (count == 2) leftSpeed = TURN_LOW_SPEED;
    else if (count == 1) leftSpeed = TURN_MED_SPEED;
    else                 leftSpeed = LEFT_SPEED;

    motor.goForward(leftSpeed, rightSpeed);
}

void Robot::turnRight() {
    uint8_t       count      = lineSensor.offTrackAmount();
    uint8_t       rightSpeed;
    const uint8_t leftSpeed  = LEFT_SPEED + TURN_SPEED_BOOST;

    if      (count == 5 || count == 0) rightSpeed = 0;
    else if (count == 4) rightSpeed = 0;
    else if (count == 3) rightSpeed = 0;
    else if (count == 2) rightSpeed = TURN_LOW_SPEED;
    else if (count == 1) rightSpeed = TURN_MED_SPEED;
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
        leftWheelSpeed = LEFT_SPEED + STRAIGHT_SPEED_TRIM;
        rightWheelSpeed = RIGHT_SPEED;
    } else {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED;
    }

    if (speed == Speed::SLOW) {
        leftWheelSpeed = LEFT_SPEED - SLOW_SPEED_REDUCTION;
        rightWheelSpeed = RIGHT_SPEED - SLOW_SPEED_REDUCTION;
    } else if (speed == Speed::DEFAULT) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (speed == Speed::FAST) {
        leftWheelSpeed = LEFT_SPEED + FAST_SPEED_BOOST;
        rightWheelSpeed = RIGHT_SPEED + FAST_SPEED_BOOST;
    }

    if (alignment == Alignment::LEFT) {
        leftWheelSpeed = LEFT_SPEED - ALIGN_WHEEL_TRIM;
        rightWheelSpeed = RIGHT_SPEED + ALIGN_BOOST;
    } else if (alignment == Alignment::RIGHT) {
        leftWheelSpeed = LEFT_SPEED + ALIGN_BOOST;
        rightWheelSpeed = RIGHT_SPEED - ALIGN_WHEEL_TRIM;
    }

    if (lineSensor.offTrackLeft()) {
        leftWheelSpeed += lineSensor.offTrackAmount() * OFFTRACK_BOOST;
        rightWheelSpeed -= lineSensor.offTrackAmount() * OFFTRACK_REDUCTION;
    } else if (lineSensor.offTrackRight()) {
        rightWheelSpeed += lineSensor.offTrackAmount() * OFFTRACK_BOOST;
        leftWheelSpeed -= lineSensor.offTrackAmount() * OFFTRACK_REDUCTION;
    }
    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followLine() {
    if (lineSensor.getSensor(1))
        motor.goForward(LEFT_SPEED / FOLLOW_LINE_RATIO, RIGHT_SPEED);
    else if (lineSensor.getSensor(3))
        motor.goForward(LEFT_SPEED, RIGHT_SPEED / FOLLOW_LINE_RATIO);
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
        leftWheelSpeed = LEFT_SPEED + LEFT_LINE_STRONG_BOOST;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s2 && s3) {
        leftWheelSpeed = LEFT_SPEED + LEFT_LINE_STRONG_BOOST;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s1) {
        leftWheelSpeed = LEFT_SPEED - LINE_ADJUST;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s2 || s3 || s4) {
        leftWheelSpeed = LEFT_SPEED + LINE_ADJUST;
        rightWheelSpeed = RIGHT_SPEED;
    } else {
        leftWheelSpeed = LEFT_SPEED - LINE_FALLBACK;
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
        rightWheelSpeed = RIGHT_SPEED + RIGHT_LINE_BOOST;
    } else if (s3 && s4) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + RIGHT_LINE_BOOST;
    } else if (s5) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED - LINE_ADJUST;
    } else if (s2 || s3 || s4) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + LINE_ADJUST;
    } else {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED - LINE_FALLBACK;
    }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followRightWall(Speed speed) {
    uint8_t leftWheelSpeed = LEFT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_SPEED;

    if (speed == Speed::SLOW) {
        leftWheelSpeed = LEFT_SPEED - WALL_SLOW_REDUCTION;
        rightWheelSpeed = RIGHT_SPEED - WALL_SLOW_REDUCTION;
    } else if (speed == Speed::DEFAULT) {
        leftWheelSpeed = LEFT_SPEED - WALL_DEFAULT_REDUCTION;
        rightWheelSpeed = RIGHT_SPEED - WALL_DEFAULT_REDUCTION;
    } else if (speed == Speed::FAST) {
        leftWheelSpeed = LEFT_SPEED - WALL_FAST_REDUCTION;
        rightWheelSpeed = RIGHT_SPEED - WALL_FAST_REDUCTION;
    }

    bool s3 = lineSensor.getSensor(2);
    bool s4 = lineSensor.getSensor(3);
    bool s5 = lineSensor.getSensor(4);

    if (s4 && s5) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED - WALL_NEAR_REDUCTION;
    } else if (s5) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + WALL_BOOST;
    } else if (s3 && s4) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED + WALL_BOOST;
    } else {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED - WALL_FAR_REDUCTION;
    }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}
void Robot::followLeftWall(Speed speed) {
    uint8_t leftWheelSpeed = LEFT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_SPEED;

    if (speed == Speed::SLOW) {
        leftWheelSpeed = LEFT_SPEED - WALL_SLOW_REDUCTION;
        rightWheelSpeed = RIGHT_SPEED - WALL_SLOW_REDUCTION;
    } else if (speed == Speed::DEFAULT) {
        leftWheelSpeed = LEFT_SPEED - WALL_DEFAULT_REDUCTION;
        rightWheelSpeed = RIGHT_SPEED - WALL_DEFAULT_REDUCTION;
    } else if (speed == Speed::FAST) {
        leftWheelSpeed = LEFT_SPEED - WALL_FAST_REDUCTION;
        rightWheelSpeed = RIGHT_SPEED - WALL_FAST_REDUCTION;
    }

    bool s1 = lineSensor.getSensor(0);
    bool s2 = lineSensor.getSensor(1);
    bool s3 = lineSensor.getSensor(2);

    if (s1 && s2) {
        leftWheelSpeed = LEFT_SPEED;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s1) {
        leftWheelSpeed = LEFT_SPEED + LEFT_WALL_BOOST;
        rightWheelSpeed = RIGHT_SPEED;
    } else if (s2 && s3) {
        leftWheelSpeed = LEFT_SPEED + LEFT_WALL_BOOST;
        rightWheelSpeed = RIGHT_SPEED;
    } else {
        leftWheelSpeed = LEFT_SPEED - LEFT_WALL_REDUCTION;
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
    _delay_ms(ALIGN_STOP_DELAY_MS);

    if (direction == 0)
        motor.goForward(0, RIGHT_SPEED - ALIGN_WHEEL_TRIM);
    else
        motor.goForward(LEFT_SPEED - ALIGN_WHEEL_TRIM, 0);
    _delay_ms(ALIGN_TURN_DURATION_MS);
}

bool Robot::confirmTurn() {
    if ((direction == 0 ? lineSensor.offTrackRight()
                        : lineSensor.offTrackLeft()) &&
        lineSensor.offTrackAmount() <= CONFIRM_OFFTRACK_MAX)
        return false;
    else
        return true;
}

void Robot::detectObject(EEPROMAddress addr) {
    _delay_ms(DETECT_INITIAL_DELAY_MS);
    lineSensor.resetObjectCount();

    timer.startTimer();
    motor.goForward(BURST_SPEED, BURST_SPEED);
    _delay_ms(BURST_DURATION_MS);
    while (direction == 0 ? ticks < DETECT_TICKS_DIR0 : ticks < DETECT_TICKS_DIR1) {
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
               ticks < ROOM3_TICKS_DEFAULT) {
            followPath(direction == 0 ? Alignment::RIGHT : Alignment::DEFAULT, Speed::DEFAULT);
        }
    } else if (roomCount == 2) {
        while ((lineSensor.robotMiddle() ||
                (direction == 0 ? lineSensor.getSensor(4)
                                : lineSensor.getSensor(0))) &&
               ticks < ROOM3_TICKS_2) {
            followPath(direction == 0 ? Alignment::RIGHT : Alignment::DEFAULT, Speed::DEFAULT);
        }
    } else {
        while ((lineSensor.robotMiddle() ||
                (direction == 0 ? lineSensor.getSensor(4)
                                : lineSensor.getSensor(0))) &&
               ticks < ROOM3_TICKS_DEFAULT) {
            followPath(direction == 0 ? Alignment::RIGHT : Alignment::DEFAULT, Speed::DEFAULT);
        }
    }

    timer.stopTimer();
    ticks = 0;

    _delay_ms(ROOM3_STOP_DELAY_MS);
    motor.stop();
    _delay_ms(ROOM3_STOP_DELAY_MS);

    if (direction == 0) {
        timer.startTimer();
        while (ticks < ROOM3_SPIN_TIMEOUT_TICKS) {
            if (lineSensor.offTrackLeft() && lineSensor.offTrackAmount() < ROOM3_OFFTRACK_THRESHOLD)
                break;
            motor.spinLeftSpeed(ROOM3_SPIN_SPEED);
        }

        timer.stopTimer();
        ticks = 0;
        while (
            !(lineSensor.offTrackLeft() && lineSensor.offTrackAmount() < ROOM3_OFFTRACK_THRESHOLD)) {
            motor.spinRightSpeed(ROOM3_SPIN_SPEED);
        }
    } else {
        timer.startTimer();
        while (ticks < ROOM3_SPIN_TIMEOUT_TICKS) {
            if (lineSensor.offTrackRight() && lineSensor.offTrackAmount() < ROOM3_OFFTRACK_THRESHOLD)
                break;
            motor.spinRightSpeed(ROOM3_SPIN_SPEED);
        }

        timer.stopTimer();
        ticks = 0;

        while (
            !(lineSensor.offTrackRight() && lineSensor.offTrackAmount() < ROOM3_OFFTRACK_THRESHOLD)) {
            motor.spinLeftSpeed(ROOM3_SPIN_SPEED);
        }
    }

    motor.stop();
    _delay_ms(ROOM3_STOP_DELAY_MS);
}

void Robot::turnCorner() {
    motor.goForward(LEFT_SPEED, RIGHT_SPEED);
    _delay_ms(CORNER_FORWARD_MS);

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
    _delay_ms(STOP_DELAY_MS);
}

void Robot::movementLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {
        case Action::PARKING:
            if (stepCount == 0) {
                if (direction == 0)
                    motor.goBackward(LEFT_SPEED, 0);
                else
                    motor.goBackward(0, RIGHT_SPEED);
                _delay_ms(PARKING_BACKWARD_MS);
                motor.stop();
                stepCount++;
            }
            if (stepCount == 1) {
                if (direction == 0)
                    motor.goForward(0, RIGHT_SPEED);
                else
                    motor.goForward(LEFT_SPEED, 0);
                _delay_ms(PARKING_FORWARD_MS);
                while (lineSensor.offTrackAmount() < PARKING_OFFTRACK_THRESHOLD) {
                }
                motor.stop();
                stepCount++;
            }
            break;

        case Action::LEAVE_PARKING:

            if (direction == 0)
                motor.goForward(0, RIGHT_SPEED - LEAVE_PARKING_SPEED_TRIM);
            else
                motor.goForward(LEFT_SPEED - LEAVE_PARKING_SPEED_TRIM, 0);
            _delay_ms(LEAVE_PARKING_FORWARD_MS);
            while (true) {
                if ((direction == 0 ? lineSensor.offTrackRight()
                                    : lineSensor.offTrackLeft()) &&
                    lineSensor.offTrackAmount() < LEAVE_PARKING_OFFTRACK_MAX) {
                    while (!lineSensor.robotMiddle()) {
                    }

                    _delay_ms(LEAVE_PARKING_STABILIZE_MS);
                    while (direction == 0 ? !lineSensor.isOnRightLine()
                                          : !lineSensor.isOnLeftLine()) {
                        if (direction == 0)
                            motor.goForward(LEFT_SPEED + TURN_SPEED_BOOST, RIGHT_SPEED);
                        else
                            motor.goForward(LEFT_SPEED, RIGHT_SPEED + LEAVE_PARKING_SPEED_TRIM);
                    }

                    while (!lineSensor.robotMiddle()) {
                        if (direction == 0)
                            motor.goForward(PARTIAL_SPEED, RIGHT_SPEED);
                        else
                            motor.goForward(LEFT_SPEED, PARTIAL_SPEED);
                    }
                    if (direction == 0)
                        motor.goForward(LEFT_SPEED, RIGHT_SPEED - LEAVE_PARKING_SPEED_TRIM);
                    else
                        motor.goForward(LEFT_SPEED - LEAVE_PARKING_SPEED_TRIM, RIGHT_SPEED);
                    break;
                }
            }
            break;

        case Action::AFTER_PARKING:
            timer.startTimer();
            while (ticks < WALL_ENTRY_WAIT_TICKS && !lineSensor.robotBumpLine()) {
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
            _delay_ms(STOP_DELAY_MS);
            turnCorner();
            break;

        case Action::FIRST_CORRIDOR:
            lineSensor.resetDamage();
            timer.startTimer();

            while (ticks < CORRIDOR_INITIAL_TICKS) followPath(Alignment::DEFAULT, Speed::DEFAULT);
            ticks = 0;

            while (ticks < CORRIDOR_MAX_TICKS && !lineSensor.robotBumpLine()) {
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
            _delay_ms(STOP_DELAY_MS);
            turnCorner();

            timer.startTimer();
            while (ticks < SECOND_TURN_TICKS)
                followPath(direction == 0 ? Alignment::RIGHT : Alignment::LEFT,
                           Speed::DEFAULT);
            timer.stopTimer();
            ticks = 0;
            motor.stop();
            break;

        case Action::SECOND_CORRIDOR: {
            if (roomCount == 0) {
                timer.startTimer();
                while (direction == 0 ? ticks < SECOND_CORRIDOR_TICKS_DIR0 : ticks < SECOND_CORRIDOR_TICKS_DIR1) {
                    followPath(direction == 0 ? Alignment::LEFT : Alignment::RIGHT, Speed::SLOW);
                }
                timer.stopTimer();
                ticks = 0;

                motor.stop();
                _delay_ms(STOP_DELAY_MS);

                if (direction == 0)
                    motor.spinLeft(ROOM_ENTRY_SPIN_ANGLE);
                else
                    motor.spinRight(ROOM_ENTRY_SPIN_ANGLE);
                motor.stop();
                _delay_ms(ROOM_ENTRY_DELAY_MS);

            } else if ((roomCount == 1) || (roomCount == 2)) {
                motor.stop();
                _delay_ms(STOP_DELAY_MS);

                while (!(direction == 0 ? (lineSensor.getSensor(0) || lineSensor.getSensor(1) || lineSensor.getSensor(2)) : 
                                          (lineSensor.getSensor(2) || lineSensor.getSensor(3) || lineSensor.getSensor(4)))) {
                    if (direction == 0)
                        followLeftLine();
                    else
                        followRightLine();
                }

                findRoom3();

                if (direction == 0)
                    motor.spinLeft(CORRIDOR_SPIN_ANGLE);
                else
                    motor.spinRight(CORRIDOR_SPIN_ANGLE);

            } else if (roomCount == 3) {
                motor.stop();
                _delay_ms(STOP_DELAY_MS);

                while (!(direction == 0 ? lineSensor.getSensor(0) || lineSensor.getSensor(1) || lineSensor.getSensor(2) :
                                          lineSensor.getSensor(2) || lineSensor.getSensor(3) || lineSensor.getSensor(4))) {
                    if (direction == 0)
                        followLeftLine();
                    else
                        followRightLine();
                }

                findRoom3();
                timer.startTimer();
                while (ticks < ROOM3_FORWARD_TICKS) {
                    motor.goForward(LEFT_SPEED, RIGHT_SPEED);
                }
                timer.stopTimer();
                ticks = 0;
                if (direction == 0)
                    motor.spinLeft(ROOM3_SPIN_ANGLE);
                else
                    motor.spinRight(ROOM3_SPIN_ANGLE);
                motor.stop();
                _delay_ms(ROOM_ENTRY_DELAY_MS);
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
                motor.spinRightSpeed(CORRIDOR_SPIN_ANGLE);
            else
                motor.spinLeftSpeed(CORRIDOR_SPIN_ANGLE);
            if (direction == 0) {
                while (!(lineSensor.offTrackLeft() &&
                         lineSensor.offTrackAmount() < PEOPLE_ROOM_OFFTRACK_THRESHOLD)) {
                }
            } else {
                while (!(lineSensor.offTrackRight() &&
                         lineSensor.offTrackAmount() < PEOPLE_ROOM_OFFTRACK_THRESHOLD)) {
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
            _delay_ms(STOP_DELAY_MS);

            if (direction == 0)
                motor.spinRight(OBJECT_ROOM_SPIN_RIGHT);
            else
                motor.spinLeft(OBJECT_ROOM_SPIN_LEFT);

            motor.stop();

            _delay_ms(ROOM_ENTRY_DELAY_MS);

            timer.stopTimer();
            ticks = 0;

            motor.goForward(BURST_SPEED, BURST_SPEED);
            _delay_ms(BURST_DURATION_MS);
            while (!lineSensor.robotBumpLine())
                motor.goForward(LEFT_SPEED, RIGHT_SPEED);
            motor.stop();
            _delay_ms(STOP_DELAY_MS);

            if (!lineSensor.robotBumpLine()) {
                motor.goBackward(BACKUP_SPEED, BACKUP_SPEED);
                _delay_ms(BACKUP_DURATION_MS);
                while (!lineSensor.robotBumpLine())
                    motor.goBackward(LEFT_SPEED, RIGHT_SPEED);
                motor.stop();
            }


            if (direction == 0)
                motor.goForward(0, RIGHT_SPEED);
            else
                motor.goForward(LEFT_SPEED, 0);
            _delay_ms(CORNER_FORWARD_MS);

            if (direction == 0)
                while (!(lineSensor.offTrackAmount() < ROOM3_OFFTRACK_THRESHOLD &&
                         lineSensor.offTrackRight())) {
                }
            else
                while (!(lineSensor.offTrackAmount() < ROOM3_OFFTRACK_THRESHOLD &&
                         lineSensor.offTrackLeft())) {
                }

            while (!lineSensor.robotMiddle()) turn();
            previousAction = Action::OBJECT_ROOM;
            break;

        case Action::THIRD_TURN:
            motor.stop();
            _delay_ms(STOP_DELAY_MS);

            turnCorner();

            break;

        case Action::THIRD_CORRIDOR:
            lineSensor.resetDamage();
            timer.startTimer();
            while (ticks < CORRIDOR_INITIAL_TICKS) followPath(Alignment::DEFAULT, Speed::DEFAULT);
            ticks = 0;
            while (ticks < CORRIDOR_MAX_TICKS && !lineSensor.robotBumpLine()) {
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
            _delay_ms(STOP_DELAY_MS);
            turnCorner();
            break;

        case Action::ENTER_PARKING:
            break;

        case Action::COMPLETE_PARKING:
            timer.startTimer();
            if (direction == 0) while (ticks < COMPLETE_PARKING_TICKS) motor.goForward(COMPLETE_PARKING_SPEED_A, COMPLETE_PARKING_SPEED_B);
            else                while (ticks < COMPLETE_PARKING_TICKS) motor.goForward(COMPLETE_PARKING_SPEED_C, COMPLETE_PARKING_SPEED_A);
            timer.stopTimer();
            ticks = 0;
            while (lineSensor.robotMiddle()) {
                motor.goForward(LEFT_SPEED - COMPLETE_PARKING_REDUCTION, RIGHT_SPEED - COMPLETE_PARKING_REDUCTION);
            }
            if (lineSensor.offTrackRight()){
                motor.goBackward(LEFT_SPEED - COMPLETE_PARKING_REDUCTION, RIGHT_SPEED - COMPLETE_PARKING_FINE_TRIM);
            }
            else if (lineSensor.offTrackLeft()){
                motor.goBackward(LEFT_SPEED - COMPLETE_PARKING_FINE_TRIM, RIGHT_SPEED - COMPLETE_PARKING_REDUCTION);
            } else if (lineSensor.robotBumpLine()) {
                motor.goBackward(LEFT_SPEED - COMPLETE_PARKING_REDUCTION, RIGHT_SPEED - COMPLETE_PARKING_REDUCTION);
            }
            _delay_ms(COMPLETE_PARKING_DELAY_MS);

            break;

        case Action::END:
            motor.stop();
            sound.playSound(END_SOUND_NOTE);
            _delay_ms(END_SOUND_DURATION_MS);
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
                _delay_ms(STOP_DELAY_MS);
                currentAction = Action::LEAVE_PARKING;
            }
            break;

        case Action::LEAVE_PARKING:
            if (lineSensor.robotMiddle()) {
                _delay_ms(STOP_DELAY_MS);
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
            while (ticks < WALL_ENTRY_WAIT_TICKS) {
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
                        while (ticks < ENTER_PARKING_TICKS_DIR0) followWall(Speed::DEFAULT);
                    } else {
                        while (ticks < ENTER_PARKING_TICKS_DIR1) followWall(Speed::DEFAULT);
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

    _delay_ms(STOP_DELAY_MS);

    stepCount = 0;
    firstTime = true;
    roomCount = 0;
    parkingCount = 0;

    for (uint8_t i = 0; i < EEPROM_ZONE_COUNT; i++) {
        eeprom.ecriture(PERSON_A + i, 0);
        _delay_ms(EEPROM_WRITE_DELAY_MS);
    }

    readEepromOperands();

    Action currentAction = Action::PARKING;
    Action previousAction = static_cast<Action>(-1);

    while (true) {
        movementLogic(currentAction, previousAction);
        switchLogic(currentAction, previousAction);
    }
}
