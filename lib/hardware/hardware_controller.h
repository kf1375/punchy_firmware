#ifndef HARDWARE_CONTROLLER_H
#define HARDWARE_CONTROLLER_H

#include "MobaTools.h"
#include "configuration.h"
#include "motor.h"

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

  enum class ManualCommand { Forward = 0, Backward };

  HardwareController(Configuration &config);
  ~HardwareController();

  void begin();
  void loop();

  void setNextState(HardwareController::State state);
  void setFrontPos()
  {
    if (m_state == State::Idle) {
      m_config.hardware.setFrontPosition(m_motor.currentPosition());
    }
  };
  void setRearPos()
  {
    if (m_state == State::Idle) {
      m_motor.setZero();
    }
  };
  void setManualCommand(ManualCommand command) { m_manualCommand = command; };

private:
  Configuration &m_config;
  Motor m_motor;

  State m_state;
  State m_nextState;
  ManualCommand m_manualCommand;
  bool m_turnFinished;

  long m_startPauseForwardMillis;
  long m_startPauseBackMillis;

  void handleSingleTurnState();
  void handleInfiniteTurnState();
  void handleManualTurnState();
  void handleStopState();
};

#endif // HARDWARE_CONTROLLER_H
