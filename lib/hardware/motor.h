#ifndef MOTOR_H
#define MOTOR_H

#include <MobaTools.h>

#define STEPS_PER_REVOLUTION 200
#define MAX_SPEED_IN_RPM 500

class Motor
{
public:
  enum class State {
    Start,
    RotateForward,
    PauseForward,
    RotateBack,
    PauseBack
  };

  Motor();

  void begin();
  int32_t currentPosition();
  // Get Speed (RPM)
  uint32_t speed();

  void setZero();
  // Set speed (RPM)
  void setSpeed(uint32_t speed);
  void setRampLen(uint32_t ramp_len);
  void move(int32_t move);
  void moveTo(int32_t position);
  void runForward();
  void runBackward();
  void enableMotor();
  void disableMotor();
  bool isRunning();

  void setState(Motor::State state) { m_state = state; };
  Motor::State state() { return m_state; };

private:
  MoToStepper *m_stepper; // Stepper instance
  State m_state;

  const uint8_t m_stepPin = 4;
  const uint8_t m_dirPin = 5;
  const uint8_t m_enPin = 22;
};

#endif // MOTOR_H
