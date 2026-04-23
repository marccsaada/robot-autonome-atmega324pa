/*
Auteurs : Jérémie Anglaret-Guirguis, Anis Benabdallah, Marc Abou-Saada, Yanis Ben Boudaoud
Travail : TP9
Section # : 05
Équipe # : 107111
Correcteur : Abdul-wahab Chaarani

Description : Regroupe les inclusions de toutes les bibliothèques statiques du projet.
*/

/**
 * @file libstatique.hpp
 * @brief Convenience header that includes every static library in the project.
 *
 * Include this single header in application code to pull in all subsystem
 * headers (LED, UART, Button, Motor, Sound, Timer, Wheel, Sensors, etc.).
 */

#include <LED.hpp>
#include <UART.hpp>
#include <button.hpp>
#include <debug.hpp>
#include <interpreter.hpp>
#include <memoryReader.hpp>
#include <robot.hpp>
#include <motor.hpp>
#include <sound.hpp>
#include <timer.hpp>
#include <wheel.hpp>
#include <sound.hpp> 
#include <memoryReader.hpp>  
#include <interpreter.hpp> 
#include <lineSensor.hpp>
#include <distanceSensor.hpp>
#include <eepromAdresses.hpp>