#ifndef MOTOR_H
#define MOTOR_H

#include <MobaTools.h>

class Motor
{
public:
  enum class State {
    Start,
    Prepare,
    PausePrepare,
    ToHit,
    PauseHit,
    ToRest,
    PauseRest,
    RotateCW,
    RotateCCW
  };

  static const int StepsPerRevolution = 200;
  static const int MaxSpeedInRPM = 500;

  Motor();

  void begin();
  int32_t currentPosition();
  // Get Speed (RPM)
  uint32_t speed();

  void setZero(long zeroPoint);
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
  MoToStepper m_stepper; // Stepper instance
  State m_state;

  const uint8_t m_stepPin = 4;
  const uint8_t m_dirPin = 5;
  const uint8_t m_enPin = 22;
};

#endif // MOTOR_H
