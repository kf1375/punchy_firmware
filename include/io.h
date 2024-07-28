// This file contains all the definitions for the IO pins, including the ISR and
// PinModes

#ifndef IO_H
#define IO_H

#include <Arduino.h>

// Stepper Motor Pins
#define STEP_PIN 4
#define DIR_PIN 5
#define ENA_PIN 22

// Breathing Sensor
#define BREATHING_SENSOR_PIN 34 // ADC for breathing sensor

// For Interrupt, to enable the ISR to call the functions
extern void FrontStop();
extern void BackStop();
extern void FrontEnable();
extern void BackEnable();

/**
 * @brief Setup the IO pins
 */
void io_setup();

#endif