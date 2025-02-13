#include "motor.h"

// Constructor
Motor::Motor() : m_state(State::Start) {}

// Initialize the motor
void Motor::begin()
{
  m_stepper = new MoToStepper(STEPS_PER_REVOLUTION, STEPDIR);
  m_stepper->attach(m_stepPin, m_dirPin);
  m_stepper->attachEnable(m_enPin, 5, 1);
  m_stepper->setMaxSpeed(MAX_SPEED_IN_RPM * 10);
  setZero();
}

int32_t Motor::currentPosition()
{
  return m_stepper->currentPosition();
}

// Get speed (rpm)
uint32_t Motor::speed()
{
  if (!m_stepper) {
    return 0;
  }

  return ((m_stepper->getSpeedSteps() * 6.0) / STEPS_PER_REVOLUTION);
}

void Motor::setZero()
{
  if (!m_stepper) {
    return;
  }

  m_stepper->setZero();
}

// Set speed (rpm)
void Motor::setSpeed(uint32_t speed)
{
  if (m_stepper) {
    if (speed == this->speed()) {
      return;
    }
    m_stepper->setSpeed(speed * 10);
  }
}

// steps
void Motor::setRampLen(uint32_t ramp_len)
{
  if (m_stepper) {
    m_stepper->setRampLen(ramp_len);
  }
}

// Move a specific number of pulse
void Motor::move(int32_t move)
{
  if (m_stepper) {
    m_stepper->doSteps(move);
  }
}

// Move to a specific position
void Motor::moveTo(int32_t position)
{
  if (m_stepper) {
    m_stepper->moveTo(position);
  }
}

// run continuously in one direction
void Motor::runForward()
{
  if (m_stepper) {
    m_stepper->rotate(1);
  }
}

void Motor::runBackward()
{
  if (m_stepper) {
    m_stepper->rotate(-1);
  }
}

// Enable the motor
void Motor::enableMotor()
{
  // if (m_stepper) {
  //     m_stepper->setAutoEnable(true);
  // }
}

// Disable the motor
void Motor::disableMotor()
{
  if (m_stepper) {
    m_stepper->stop();
    // m_stepper->setAutoEnable(false);
  }
}

// Check if the motor is running
bool Motor::isRunning()
{
  return (bool) m_stepper->moving();
}
