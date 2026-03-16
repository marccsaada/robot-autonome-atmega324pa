#pragma once
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/**
 * @file button.hpp
 * @brief Hardware button management with interrupt support.
 */

/**
 * @brief Represents a physical button that can be located on different pins.
 */
class Button {
public:
    /**
     * @brief Possible locations for the button.
     */
    enum Location {
        MOTHERBOARD,
        PD3_BUTTON,
        PB2_BUTTON
    };

    /**
     * @brief Possible interrupt modes for the button.
     */
    enum Mode {
        RISING = (1 << ISC01) | (1 << ISC00),
        FALLING = (1 << ISC01),
        ANY = (1 << ISC00)
    };

    /**
     * @brief Constructs a button and configures its state detection.
     *
     * @param mode Detection mode (rising/falling/any).
     * @param location Physical location of the button.
     */
    Button(Mode mode, Location location_ = MOTHERBOARD);
    ~Button();
    /**
     * @brief Checks if the button is pressed (reads the logical state).
     *
     * @return true if the button is pressed, false otherwise.
     */
    bool isPressed();

    /**
     * @brief Initializes the pin and enables the button interrupt.
     */
    void init();

    /** @brief Enables the button interrupt. */
    void enableInterupt();

    /** @brief Disables the button interrupt. */
    void disableInterupt();

private:
    Mode mode_;
    Location location_;
};