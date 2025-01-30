#include "motor_controller.h"

// Constructor
MotorController::MotorController(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin)
    : m_stepPin(stepPin), m_dirPin(dirPin), m_enablePin(enablePin), m_stepper(nullptr) {}

// Initialize the motor
void MotorController::begin() 
{
    m_stepper = new MoToStepper(STEPS_PER_REVOLUTION, STEPDIR);
    m_stepper->attach(m_stepPin, m_dirPin);
    m_stepper->setMaxSpeed(MAX_SPEED_IN_RPM * 10);
    setZero();
}

int32_t MotorController::currentPosition()
{
    return m_stepper->currentPosition();
}

// Get speed (rpm)
uint32_t MotorController::speed()
{
    if (!m_stepper) {
        return 0;
    }

    return ((m_stepper->getSpeedSteps() * 6.0) / STEPS_PER_REVOLUTION);
}

void MotorController::setZero()
{
    if (!m_stepper) {
        return;
    }

    m_stepper->setZero();
}

// Set speed (rpm)
void MotorController::setSpeed(uint32_t speed) 
{
    if (m_stepper) {
        if (speed == this->speed()) {
            return;
        }
        m_stepper->setSpeed(speed * 10);
    }
}

// steps
void MotorController::setRampLen(uint32_t ramp_len) 
{
    if (m_stepper) {
        m_stepper->setRampLen(ramp_len);
    }
}

// Move a specific number of pulse
void MotorController::move(int32_t move)
{
    if (m_stepper) {
        m_stepper->move(move);
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
        m_stepper->rotate(1);
    }
}

void MotorController::runBackward()
{
    if (m_stepper) {
        m_stepper->rotate(-1);
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
        m_stepper->stop();
        // m_stepper->setAutoEnable(false);
    }
}

// Check if the motor is running
bool MotorController::isRunning() 
{
    return m_stepper && m_stepper->stepsToDo();
}
