#include "motor_controller.h"

// Constructor
MotorController::MotorController(uint8_t stepPin, uint8_t dirPin)
    : stepPin(stepPin), dirPin(dirPin), stepper(nullptr) {}

// Initialize the motor
void MotorController::begin() 
{
    engine.init(); // Initialize the engine

    // Connect stepper to the specified pin
    stepper = engine.stepperConnectToPin(stepPin);
    if (stepper) {
        stepper->setDirectionPin(dirPin);
        stepper->setEnablePin(-1); // No enable pin by default
        stepper->setAutoEnable(false);
    }
}

int32_t MotorController::currentPosition()
{
    return stepper->getCurrentPosition();
}

// Set speed (steps per second)
void MotorController::setSpeed(uint32_t speed) 
{
    if (stepper) {
        stepper->setSpeedInHz(speed);
    }
}

// Set acceleration (steps per second^2)
void MotorController::setAcceleration(uint32_t acceleration) 
{
    if (stepper) {
        stepper->setAcceleration(acceleration);
    }
}

// Move a specific number of pulse
void MotorController::move(int32_t move)
{
    if (stepper) {
        stepper->move(move, true);
    }
}

// Move to a specific position
void MotorController::moveTo(int32_t position) 
{
    if (stepper) {
        stepper->moveTo(position);
    }
}

// Enable the motor
void MotorController::enableMotor() 
{
    if (stepper) {
        stepper->setAutoEnable(true);
    }
}

// Disable the motor
void MotorController::disableMotor() 
{
    if (stepper) {
        stepper->forceStop();
        stepper->setAutoEnable(false);
    }
}

// Check if the motor is running
bool MotorController::isRunning() 
{
    return stepper && stepper->isRunning();
}
