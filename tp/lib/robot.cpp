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

// ── Méthodes existantes ───────────────────────────────────────────────────────

void Robot::wait(uint16_t multiplicator) {
    for (uint16_t i = 0; i < multiplicator; ++i)
        _delay_ms(25);
}

void Robot::playEepromNotes() {
    Memory   mem;
    uint16_t size  = mem.getSize();
    uint8_t  found = 0;

    for (uint16_t i = 0; i < size; i += 2) {
        uint8_t instr   = mem.readInstruction();
        uint8_t operand = mem.readOperand();
        
        if (instr == SGO && found < 3){
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

    while(mem.getAddress() < size) {
        uint8_t instr   = mem.readInstruction();
        uint8_t operand = mem.readOperand();
        if (instr == TRD) direction       = 1;
        if (instr == TRG) direction       = 0;
        if (instr == ATT){ 
            if (direction == 0) { parkingOperand = operand; }
            else if (direction == 1){ parkingOperand = 5 - operand;}
        }
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
    static const uint16_t EEPROM_ADDR_BASE = 100;

    const char* noms[NB_ZONES] = {
        "Local A      ", "Local B      ", "Local C      ",
        "Local D      ", "Couloir OUEST", "Couloir EST  ",
    };
    const char* types[NB_ZONES] = {
        " personne(s)", " objet(s)", " objet(s)",
        " personne(s)", " zone(s) endommagee(s)", " zone(s) endommagee(s)"
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
        uint8_t leftWheelSpeed  = LEFT_SPEED + 9;
        uint8_t rightWheelSpeed = RIGHT_SPEED;
    }
    else { 
        uint8_t leftWheelSpeed  = LEFT_SPEED;
        uint8_t rightWheelSpeed = RIGHT_SPEED;
    }
    

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



void Robot::followLeftLine() {

    uint8_t leftWheelSpeed;
    uint8_t rightWheelSpeed;
                
    bool s1 = lineSensor.getSensor(0);
    bool s2 = lineSensor.getSensor(1);
    bool s3 = lineSensor.getSensor(2);
    bool s4 = lineSensor.getSensor(3);


    if      (s1 && s2) { leftWheelSpeed = LEFT_SPEED+33;      rightWheelSpeed = RIGHT_SPEED; }
    else if (s2 && s3) {leftWheelSpeed = LEFT_SPEED+33;      rightWheelSpeed = RIGHT_SPEED;}
    else if (s1)       { leftWheelSpeed = LEFT_SPEED-25; rightWheelSpeed = RIGHT_SPEED;      }
    else if (s2 || s3 || s4) {leftWheelSpeed = LEFT_SPEED+25;      rightWheelSpeed = RIGHT_SPEED;}
    
    else {leftWheelSpeed = LEFT_SPEED-23; rightWheelSpeed = RIGHT_SPEED;}

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followRightLine() {
    uint8_t leftWheelSpeed;
    uint8_t rightWheelSpeed;

    bool s2 = lineSensor.getSensor(1);
    bool s3 = lineSensor.getSensor(2);
    bool s4 = lineSensor.getSensor(3);
    bool s5 = lineSensor.getSensor(4);

    if      (s4 && s5) { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED+31; }
    else if (s3 && s4) { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED+31; }
    else if (s5)       { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED-25; }
    else if (s2 || s3 || s4) { leftWheelSpeed = LEFT_SPEED; rightWheelSpeed = RIGHT_SPEED+25; }
    else               { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED-23; }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}

void Robot::followRightWall(Speed speed) {



    uint8_t leftWheelSpeed= LEFT_SPEED;
    uint8_t rightWheelSpeed= RIGHT_SPEED;

    if      (speed == Speed::SLOW)    { leftWheelSpeed = LEFT_SPEED  - 35; rightWheelSpeed = RIGHT_SPEED - 35; }
    else if (speed == Speed::DEFAULT) { leftWheelSpeed = LEFT_SPEED - 20;       rightWheelSpeed = RIGHT_SPEED - 20;      }
    else if (speed == Speed::FAST)    { leftWheelSpeed = LEFT_SPEED  - 10; rightWheelSpeed = RIGHT_SPEED - 10; }


    bool s3 = lineSensor.getSensor(2);
    bool s4 = lineSensor.getSensor(3);
    bool s5 = lineSensor.getSensor(4);



    if      (s4 && s5) {  leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED-5;  }
    else if (s5)        { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED + 50; }
    else if (s3 && s4)  { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED + 50; }
    else                { leftWheelSpeed = LEFT_SPEED; rightWheelSpeed = RIGHT_SPEED - 55;      }

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}
void Robot::followLeftWall(Speed speed) {

    uint8_t leftWheelSpeed = LEFT_SPEED;
    uint8_t rightWheelSpeed = RIGHT_SPEED;

    if      (speed == Speed::SLOW)    { leftWheelSpeed = LEFT_SPEED  - 35; rightWheelSpeed = RIGHT_SPEED - 35; }
    else if (speed == Speed::DEFAULT) { leftWheelSpeed = LEFT_SPEED - 20;       rightWheelSpeed = RIGHT_SPEED - 20;      }
    else if (speed == Speed::FAST)    { leftWheelSpeed = LEFT_SPEED  - 10; rightWheelSpeed = RIGHT_SPEED - 10; }

    bool s1 = lineSensor.getSensor(0);
    bool s2 = lineSensor.getSensor(1);
    bool s3 = lineSensor.getSensor(2);

    if      (s1 && s2) { leftWheelSpeed = LEFT_SPEED;      rightWheelSpeed = RIGHT_SPEED; }
    else if (s1)        { leftWheelSpeed = LEFT_SPEED+55;      rightWheelSpeed = RIGHT_SPEED; }
    else if (s2 && s3)  { leftWheelSpeed = LEFT_SPEED+55;      rightWheelSpeed = RIGHT_SPEED; }
    else                { leftWheelSpeed = LEFT_SPEED-30; rightWheelSpeed = RIGHT_SPEED;      } 

    motor.goForward(leftWheelSpeed, rightWheelSpeed);
}
void Robot::followWall(Speed speed) {
    if (direction == 0) followRightWall(speed);
    else                followLeftWall(speed);
}

void Robot::alignToTurn() {
    motor.stop();
    _delay_ms(300);

    if (direction == 0) motor.goForward(0, RIGHT_SPEED - 10);
    else                motor.goForward(LEFT_SPEED - 10, 0);
    _delay_ms(1750);
}

bool Robot::confirmTurn() {
    if ((direction == 0 ? lineSensor.offTrackRight() : lineSensor.offTrackLeft()) && lineSensor.offTrackAmount() <= 2) return false;
    else return true;
}

void Robot::detectObject(EEPROMAddress addr) {
    _delay_ms(500); 
    lineSensor.resetObjectCount();
    // ticks = 0;
    timer.startTimer();

    motor.goForward(255, 255);
    _delay_ms(80);
    while (ticks < 190) {
        motor.goForward(LEFT_SPEED,RIGHT_SPEED);
        lineSensor.findObject(addr);
        if (lineSensor.offTrackLeft()){
            motor.goForward(LEFT_SPEED,RIGHT_SPEED + lineSensor.offTrackAmount());
        }
        else if (lineSensor.offTrackRight()){
            motor.goForward(LEFT_SPEED,RIGHT_SPEED);
        }
    led.off();
    }
    timer.stopTimer();
    ticks = 0; 
    
}

void Robot::findRoom3() {
    while (!(direction == 0 ? lineSensor.getSensor(0) : lineSensor.getSensor(4))) { 
        if (direction == 0) motor.goForward(0, RIGHT_SPEED - 20);
        else                motor.goForward(LEFT_SPEED - 20, 0);
    }
    if (direction == 0) {
        while (lineSensor.getSensor(0) || lineSensor.getSensor(1) || lineSensor.getSensor(2))
            followLeftLine();
    } else {
        while (lineSensor.getSensor(2) || lineSensor.getSensor(3) || lineSensor.getSensor(4))
            followRightLine();
    }
    timer.startTimer();
    if (direction == 0) {
        motor.goForward(LEFT_SPEED + 3, RIGHT_SPEED);
    } else {
        motor.goForward(LEFT_SPEED + 5, RIGHT_SPEED);
    }

    while (lineSensor.robotMiddle() && ticks < 185) {}  // OLD 185
    timer.stopTimer();
    ticks = 0;

    _delay_ms(250); // NOUVEAU
    motor.stop();
    _delay_ms(250);

    if (direction == 0) {
        timer.startTimer();
        while (ticks < 60) { 
            if (lineSensor.offTrackLeft()  && lineSensor.offTrackAmount() < 2) break;
            motor.spinLeftSpeed(85);
        }
        timer.stopTimer();
        ticks = 0;
        while (!(lineSensor.offTrackLeft()  && lineSensor.offTrackAmount() < 2)) {
            motor.spinRightSpeed(85);
        }
    } else { 
        timer.startTimer();
        while (ticks < 60) { 
            if (lineSensor.offTrackRight()  && lineSensor.offTrackAmount() < 2) break;
            motor.spinRightSpeed(85);
        }
        timer.stopTimer();
        ticks = 0;
        while (!(lineSensor.offTrackRight() && lineSensor.offTrackAmount() < 2)) {
            motor.spinLeftSpeed(85);
        }
    }

    motor.stop();
    _delay_ms(250);
}

// ── Machine à états ───────────────────────────────────────────────────────────

void Robot::movementLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {

        case Action::PARKING:
        // readEepromOperands();
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

            if (direction == 0) // Sort vers la droite, puis tourne à gauche
                motor.goForward(0, RIGHT_SPEED - 10);
            
            else motor.goForward(LEFT_SPEED - 10, 0); // Sort vers la gauche, puis tourne à droite
            _delay_ms(1800);
            while (true) {
                if ((direction == 0 ? lineSensor.offTrackRight() : lineSensor.offTrackLeft()) && lineSensor.offTrackAmount() < 3) {
                    while (!lineSensor.robotMiddle()) {}

                    _delay_ms(50);
                    while (direction == 0 ? !lineSensor.isOnRightLine() : !lineSensor.isOnLeftLine()) {
                        if (direction == 0 ) motor.goForward(LEFT_SPEED + 18, RIGHT_SPEED);
                        else                 motor.goForward(LEFT_SPEED, RIGHT_SPEED + 10);
                        led.red();
                    }
                    led.off();
                    
                    while (!lineSensor.robotMiddle()) {
                        if (direction == 0) motor.goForward(72, RIGHT_SPEED);
                        else                motor.goForward(LEFT_SPEED, 72);
                    }
                    if (direction == 0) motor.goForward(LEFT_SPEED, RIGHT_SPEED - 10);
                    else                motor.goForward(LEFT_SPEED - 10, RIGHT_SPEED);
                    break;
                }
            }
            
            break;


        case Action::AFTER_PARKING:
            timer.startTimer();
            while (ticks < 350 && !lineSensor.robotBumpLine()) {
                // led.red();
                followWall(Speed::DEFAULT);
                if (direction == 0 ? lineSensor.isOnRightLine() : lineSensor.isOnLeftLine()) {
                    ticks = 0;
                }
            }
            // led.off();
            timer.stopTimer();
            ticks = 0;

            while (!lineSensor.robotBumpLine()) {
                if (direction == 0) followPath(Alignment::RIGHT, Speed::SLOW);
                else                followPath(Alignment::LEFT, Speed::SLOW);
                // led.green();
            }
            // led.off();

            break;


        case Action::FIRST_TURN:
            alignToTurn();
            while (!lineSensor.robotMiddle())
                turn();
            
            motor.stop();
            _delay_ms(500);
            break;

        case Action::FIRST_CORRIDOR:
            lineSensor.resetDamage();
            timer.startTimer();

            while (ticks < 150) followPath(Alignment::DEFAULT, Speed::DEFAULT);
            ticks = 0;

            while (ticks < 450 && !lineSensor.robotBumpLine()) {
                if (lineSensor.findDamage(DAMAGE_EST)) followLine();
                else                                   followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;

            while (!lineSensor.robotBumpLine()) { 
                if (direction == 0) followPath(Alignment::RIGHT, Speed::SLOW);
                else                followPath(Alignment::LEFT, Speed::SLOW);
            }

            motor.stop();
            _delay_ms(500);
            
            if (direction == 0) motor.goForward(0, RIGHT_SPEED);
            else                motor.goForward(LEFT_SPEED, 0);
            _delay_ms(75);
            motor.stop();

            break;

        case Action::SECOND_TURN:


            // alignToTurn();
            // while (direction == 0 ? !(lineSensor.getSensor(4) && !lineSensor.getSensor(3)) : !(lineSensor.getSensor(0) && !lineSensor.getSensor(1)))
            //     turn();
            

            // while (!lineSensor.robotMiddle()) {
            //     turn();
            //     motor.goForward(LEFT_SPEED, 0);
            // }
            // motor.stop();
            // _delay_ms(500);

            // _delay_ms(500);

            motor.stop();
            _delay_ms(500);
            while (!lineSensor.robotBumpLine())
                motor.goBackward(LEFT_SPEED + 20, RIGHT_SPEED + 20);
            
            // motor.goBackward(LEFT_SPEED, RIGHT_SPEED);
            // // _delay_ms(3000);

                if (direction == 0) motor.goForward(0, RIGHT_SPEED);
                else                motor.goForward(LEFT_SPEED, 0);
                _delay_ms(1000);
                while (direction == 0 ? (lineSensor.getSensor(0) || lineSensor.getSensor(1)) : (lineSensor.getSensor(3) || lineSensor.getSensor(4))) {
                    if (direction == 0) motor.goForward(0, RIGHT_SPEED);
                    else                motor.goForward(LEFT_SPEED, 0);
                }

            while (direction == 0 ? !lineSensor.getSensor(0) : !lineSensor.getSensor(4)) {
                if (direction == 0) motor.goForward(0, RIGHT_SPEED);
                else                motor.goForward(LEFT_SPEED, 0);
            }
            motor.stop();
            // _delay_ms(500);
            break;

        case Action::SECOND_CORRIDOR:
            if (roomCount == 0) {
                timer.startTimer();
                while (direction == 0 ? ticks < 300 : ticks < 380) {
                    followPath(Alignment::DEFAULT, Speed::SLOW);
                }
                timer.stopTimer();
                ticks = 0;
                // ticks = 0;

                // findRoom3();

                motor.stop();
                _delay_ms(500);

                if (direction == 0) motor.spinLeft(115);
                else                motor.spinRight(115);
                motor.stop();
                _delay_ms(800);

            } else if ((roomCount == 1) || (roomCount == 2)) {

                timer.startTimer();

                while (ticks < 200) {
                    if (direction == 0) followLeftLine();
                    else                followRightLine();
                }

                timer.stopTimer();
                ticks = 0;

                findRoom3();

                if (direction == 0) motor.spinLeft(90);
                else                motor.spinRight(90);
            
            } else if (roomCount == 3) {
                timer.startTimer();

                while (ticks < 200) {
                    if (direction == 0) followLeftLine();
                    else                followRightLine();
                }
                timer.stopTimer();

                ticks = 0;   

                
                findRoom3();
                timer.startTimer();
                while(ticks < 40) {
                    motor.goForward(LEFT_SPEED, RIGHT_SPEED);
                }
                timer.stopTimer();
                ticks = 0;
                if (direction == 0) motor.spinLeft(105);
                else                motor.spinRight(105);
                motor.stop();
                _delay_ms(800);
            }
            break;

        case Action::PEOPLE_ROOM:
            if      (roomCount == 0)
                distanceSensor.scanRoom(*this, direction == 0 ? PERSON_D : PERSON_A, direction == 0 ? LEFT : RIGHT);
            else if (roomCount == 3)
                distanceSensor.scanRoom(*this, direction == 0 ? PERSON_A : PERSON_D, direction == 0 ? LEFT : RIGHT);

            if (direction == 0) motor.spinRightSpeed(90);
            else                motor.spinLeftSpeed(90);
            if (direction == 0) { while (!(lineSensor.offTrackLeft()  && lineSensor.offTrackAmount() < 3)) {} }
            else                { while (!(lineSensor.offTrackRight() && lineSensor.offTrackAmount() < 3)) {} }
            while (!(lineSensor.robotMiddle())) {}

    

            while (!lineSensor.robotMiddle())
                followPath(Alignment::DEFAULT, Speed::DEFAULT);

            break;

        case Action::OBJECT_ROOM:
            if      (roomCount == 1) detectObject(direction == 0 ? OBJECT_C : OBJECT_B);
            else if (roomCount == 2) detectObject(direction == 0 ? OBJECT_B : OBJECT_C);

            while (!lineSensor.robotBumpLine())
                motor.goForward(110, 110);
            motor.stop();
                _delay_ms(500);


            if (direction == 0) motor.spinRight(140);
            else                motor.spinLeft(140);

            motor.stop();
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
            // alignToTurn();
            // while (!lineSensor.robotMiddle())
            //     turn();

            // motor.stop();
            // _delay_ms(500);
            motor.stop();
            _delay_ms(500);
            while (!lineSensor.robotBumpLine()) { 
                motor.goBackward(LEFT_SPEED, RIGHT_SPEED);
            }
            motor.stop();
            _delay_ms(500);

            while (!lineSensor.robotBumpLine())
                motor.goBackward(LEFT_SPEED + 20, RIGHT_SPEED + 20);
            
            // motor.goBackward(LEFT_SPEED, RIGHT_SPEED);
            // // _delay_ms(3000);

            while (direction == 0 ? (lineSensor.getSensor(0) || lineSensor.getSensor(1)) : (lineSensor.getSensor(3) || lineSensor.getSensor(4))) {
                if (direction == 0) motor.goForward(0, RIGHT_SPEED);
                else                motor.goForward(LEFT_SPEED, 0);
            }

            while (direction == 0 ? !lineSensor.getSensor(0) : !lineSensor.getSensor(4)) {
                if (direction == 0) motor.goForward(0, RIGHT_SPEED - 20);
                else                motor.goForward(LEFT_SPEED - 20, 0);
            }
            motor.stop();
            _delay_ms(500);

            if (direction == 0) motor.goForward(0, RIGHT_SPEED);
            else                motor.goForward(LEFT_SPEED, 0);
            _delay_ms(75);
            motor.stop();
            
            
            break;

        case Action::THIRD_CORRIDOR:
            lineSensor.resetDamage();
            timer.startTimer();
            while (ticks < 150) followPath(Alignment::DEFAULT, Speed::DEFAULT);
            ticks = 0;
            while (ticks < 400 && !lineSensor.robotBumpLine()) {
                if (lineSensor.findDamage(DAMAGE_OUEST)) followLine();
                else   followPath(Alignment::DEFAULT, Speed::DEFAULT);
            }
            timer.stopTimer();
            ticks = 0;
            

            
            while (!lineSensor.robotBumpLine())
                followPath(direction == 0 ? Alignment::RIGHT : Alignment::LEFT, Speed::SLOW);
            break;

        case Action::FOURTH_TURN:
            alignToTurn();
            while (!lineSensor.robotMiddle())
                turn();
            
            motor.stop();
            _delay_ms(500);
            break;

        case Action::ENTER_PARKING:
            break;

        case Action::COMPLETE_PARKING:
            timer.startTimer();
            if (direction == 0) while (ticks < 220) motor.goForward(95,  160);
            else                while (ticks < 240) motor.goForward(160, 95);
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
            while (true) {}
            break;
    }
}

void Robot::switchLogic(Action& currentAction, Action& previousAction) {
    switch (currentAction) {
        case Action::PARKING:
            if (lineSensor.robotBumpLine()) {_delay_ms(500); currentAction = Action::LEAVE_PARKING; }
            break;

        case Action::LEAVE_PARKING:
            if (lineSensor.robotMiddle()) { _delay_ms(500); currentAction = Action::AFTER_PARKING; }
            break;

        case Action::AFTER_PARKING:
            if (lineSensor.robotBumpLine()) currentAction = Action::FIRST_TURN;
            led.green();
            break;

        case Action::FIRST_TURN:
            if (confirmTurn()) currentAction = Action::FIRST_CORRIDOR;
            break;

        case Action::FIRST_CORRIDOR:
            // if (lineSensor.robotBumpLine()) {
                lineSensor.setPreviousDamageState(false);
                led.off();
                currentAction = Action::SECOND_TURN;
            // }
            break;

        case Action::SECOND_TURN:
            currentAction = Action::SECOND_CORRIDOR;
            break;

        case Action::SECOND_CORRIDOR:
            if      (roomCount == 0 || roomCount == 3) currentAction = Action::PEOPLE_ROOM;
            else if (roomCount == 1 || roomCount == 2) currentAction = Action::OBJECT_ROOM;
            else if (roomCount == 4) {
                while (!lineSensor.robotBumpLine()) followPath(direction == 0 ? Alignment::RIGHT : Alignment::LEFT, Speed::SLOW);
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
            
            timer.startTimer();
            while (ticks < 250) {
                followWall(Speed::SLOW);
            }
            timer.stopTimer();
            ticks = 0;
    
            while (parkingCount != parkingOperand) { 
                followWall(Speed::DEFAULT);
                if (direction == 0 ? lineSensor.isOnRightLine() : lineSensor.isOnLeftLine()) {
                    parkingCount++;

                    timer.startTimer();
                    while (ticks < 40) followWall(Speed::DEFAULT);
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

    readEepromOperands();

    Action currentAction  = Action::PARKING;
    Action previousAction = static_cast<Action>(-1);

    while (true) {
        movementLogic(currentAction, previousAction);
        switchLogic(currentAction, previousAction);
    }
}
