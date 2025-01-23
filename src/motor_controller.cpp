#include "motor_controller.h"

// Constructor
MotorController::MotorController(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin)
    : m_stepPin(stepPin), m_dirPin(dirPin), m_enablePin(enablePin), m_stepper(nullptr) {}

// Initialize the motor
void MotorController::begin() 
{
    m_engine.init(); // Initialize the engine

    // Connect stepper to the specified pin
    m_stepper = m_engine.stepperConnectToPin(m_stepPin);
    if (m_stepper) {
        m_stepper->setDirectionPin(m_dirPin);
        m_stepper->setEnablePin(m_enablePin); // No enable pin by default
        m_stepper->setAutoEnable(true);
    }
}

int32_t MotorController::currentPosition()
{
    return m_stepper->getCurrentPosition();
}

// Set speed (rpm)
void MotorController::setSpeed(uint32_t speed) 
{
    if (m_stepper) {
        uint32_t speedInHz = (speed * STEPS_PER_REVOLUTION) / 60; // Convert rpm to Hz
        m_stepper->setSpeedInHz(speedInHz);
    }
}

// Set acceleration (steps per second^2)
void MotorController::setAcceleration(uint32_t acceleration) 
{
    if (m_stepper) {
        m_stepper->setAcceleration(acceleration);
    }
}

// Move a specific number of pulse
void MotorController::move(int32_t move)
{
    if (m_stepper) {
        m_stepper->move(move, true);
    }
}

// Move to a specific position
void MotorController::moveTo(int32_t position) 
{
    if (m_stepper) {
        m_stepper->moveTo(position);
    }
}

// run continuously in one direction
void MotorController::runForward()
{
    if (m_stepper) {
        m_stepper->runForward();
    }
}

void MotorController::runBackward()
{
    if (m_stepper) {
        m_stepper->runBackward();
    }
}

// Enable the motor
void MotorController::enableMotor() 
{
    // if (m_stepper) {
    //     m_stepper->setAutoEnable(true);
    // }
}

// Disable the motor
void MotorController::disableMotor() 
{
    if (m_stepper) {
        m_stepper->forceStop();
        // m_stepper->setAutoEnable(false);
    }
}

// Check if the motor is running
bool MotorController::isRunning() 
{
    return m_stepper && m_stepper->isRunning();
}
