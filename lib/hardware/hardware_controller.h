#ifndef HARDWARE_CONTROLLER_H
#define HARDWARE_CONTROLLER_H

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
