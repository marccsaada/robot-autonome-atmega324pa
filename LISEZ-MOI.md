# TP Final – INF1900

**Auteurs :** Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
**Équipe :** 107111 | **Section :** 05

---

## Compilation

Un seul `make` suffit, lancé depuis le dossier `app/` :

```sh
cd inf1900-107111/app/
make
```

Le Makefile de `app/` appelle automatiquement le Makefile de `lib/` (cible `lib:`) avant de lier le programme final. Il n'est donc pas nécessaire de faire un `make` séparé dans `lib/`.

Pour flasher le microcontrôleur (ATmega324PA via usbasp) :

```sh
make install
```

---

## Structure du projet

```
inf1900-107111/
├── lib/                        Librairie statique (libstatique.a)
│   ├── Makefile                    Compile tous les .cpp en libstatique.a (avr-ar)
│   ├── robot.hpp/cpp               Agrège tous les sous-systèmes ; contient la logique
│   │                               de déplacement et les trois handlers de mode
│   │                               (runInstruction, runExecution, runRapport)
│   ├── interpreter.hpp/cpp         Interpréteur du bytecode stocké en EEPROM externe
│   ├── motor.hpp/cpp               Contrôle des moteurs (PWM)
│   ├── LED.hpp/cpp                 Gestion de la DEL bicolore
│   ├── sound.hpp/cpp               Génération de sons (buzzer)
│   ├── button.hpp/cpp              Lecture du bouton-poussoir par interruption (INT0)
│   ├── timer.hpp/cpp               Abstraction du Timer1 (mode CTC)
│   ├── lineSensor.hpp/cpp          Capteur de ligne (ADC)
│   ├── distanceSensor.hpp/cpp      Capteur de distance infrarouge (ADC)
│   ├── memoryReader.hpp/cpp        Lecture séquentielle de la mémoire EEPROM externe
│   ├── memoire_24.h/cpp            Pilote bas niveau I²C pour la mémoire 24CXXX
│   ├── UART.hpp/cpp                Communication série
│   ├── can.h/cpp                   Convertisseur analogique-numérique (CAN)
│   ├── debug.hpp/cpp               Utilitaires de débogage (UART)
│   ├── eepromAdresses.hpp          Adresses nommées dans l'EEPROM externe
│   ├── libstatique.hpp             En-tête global qui inclut tous les modules de la lib
│   └── project.hpp                 Constantes propres au projet final
│
└── app/                        Application (point d'entrée)
    ├── Makefile                    Compile main.cpp et lie libstatique.a
    ├── main.cpp                    Point d'entrée (int main())
    │                               Sélection du mode par séquence DEL + bouton,
    │                               entièrement pilotée par interruptions (Timer1 CTC +
    │                               INT0, mode IDLE). Dispatche vers runInstruction(),
    │                               runProject() ou runRapport() selon le choix.
    └── programme.txt               Bytecode exemple pour le mode instruction
```

---

## Fichiers supplémentaires (référence uniquement)

**`selectMode/main.cpp`** — Ancienne version du point d'entrée, entièrement commentée. Conservée comme référence historique ; elle n'est pas compilée.

---

## Notes sur l'architecture

- Toute la logique des modes (déplacement, machine à états, capteurs) est encapsulée dans la classe `Robot` (`lib/robot.hpp`). Le `main()` est minimal.

- L'interpréteur de bytecode (`Interpreter`) lit des instructions depuis l'EEPROM externe via `MemoryReader` et les exécute sur le robot ; il est utilisé uniquement en mode `INSTRUCTION`.

- Aucune scrutation (busy-wait) dans la boucle principale : le robot dort en `SLEEP_MODE_IDLE` et se réveille uniquement sur interruption Timer1 ou INT0.
