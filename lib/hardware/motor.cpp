#include "motor.h"

// Constructor
Motor::Motor() : m_state(State::Start), m_stepper(StepsPerRevolution, STEPDIR)
{
}

// Initialize the motor
void Motor::begin()
{
  m_stepper.attach(m_stepPin, m_dirPin);
  m_stepper.attachEnable(m_enPin, 5, 1);
  m_stepper.setMaxSpeed(MaxSpeedInRPM * 10);
}

int32_t Motor::currentPosition()
{
  return m_stepper.currentPosition();
}

// Get speed (rpm)
uint32_t Motor::speed()
{
  return ((m_stepper.getSpeedSteps() * 6.0) / StepsPerRevolution);
}

void Motor::setZero(long zeroPoint)
{
  m_stepper.setZero(zeroPoint);
}

// Set speed (rpm)
void Motor::setSpeed(uint32_t speed)
{
  if (speed == this->speed()) {
    return;
  }
  m_stepper.setSpeed(speed * 10);
}

// steps
void Motor::setRampLen(uint32_t ramp_len)
{
  m_stepper.setRampLen(ramp_len);
}

// Move a specific number of pulse
void Motor::move(int32_t move)
{
  m_stepper.doSteps(move);
}

// Move to a specific position
void Motor::moveTo(int32_t position)
{
  m_stepper.moveTo(position);
}

// run continuously in one direction
void Motor::runForward()
{
  m_stepper.rotate(1);
}

void Motor::runBackward()
{
  m_stepper.rotate(-1);
}

// Enable the motor
void Motor::enableMotor()
{
  // if (m_stepper) {
  //     m_stepper.setAutoEnable(true);
  // }
}

// Disable the motor
void Motor::disableMotor()
{
  m_stepper.stop();
}

// Check if the motor is running
bool Motor::isRunning()
{
  return (bool) m_stepper.distanceToGo();
}
