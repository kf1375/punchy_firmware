#ifndef HARDWARE_CONTROLLER_H
#define HARDWARE_CONTROLLER_H

#include "MobaTools.h"
#include "configuration.h"
#include "motor.h"
#include "rgb_led.h"

// #define SIMULATION_MODE

class HardwareController
{
public:
  enum class State {
    Idle,
    Stop,
    SingleTurn,
    InfiniteTurn,
    ManualTurn,
  };

  enum class ManualCommand { Left = 0, Right };

  enum class LEDState { Orange = 0, Green };

  HardwareController(Configuration &config);
  ~HardwareController();

  void begin();
  void loop();

  State state() const { return m_state; };

  void setNextState(HardwareController::State state);
  void setHitPos()
  {
    m_config.hardware.setHitPosition(m_motor.currentPosition());
  };
  void setRestPos()
  {
    m_config.hardware.setRestPosition(m_motor.currentPosition());
  };
  void setManualCommand(ManualCommand command) { m_manualCommand = command; };
  void setLEDState(LEDState led_state);

private:
  Configuration &m_config;
  Motor m_motor;
  RGBLed m_rgbLed;

  State m_state;
  State m_nextState;
  ManualCommand m_manualCommand;
  bool m_turnFinished;

  long m_startPauseMillis;

  void handleSingleTurnState();
  void handleInfiniteTurnState();
  void handleManualTurnState();
  void handleStopState();

  void sendPositionToSerial();
};

#endif // HARDWARE_CONTROLLER_H
