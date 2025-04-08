#include "hardware_controller.h"

#include "logging.h"

/**
 * @brief Construct a new Hardware Controller:: Hardware Controller object
 *
 * @param config
 */
HardwareController::HardwareController(Configuration &config)
    : m_config(config), m_motor(), m_state(State::Idle), m_turnFinished(true)
{
}

/**
 * @brief Destroy the Hardware Controller:: Hardware Controller object
 *
 */
HardwareController::~HardwareController() {}

/**
 * @brief Initialize the hardware controller
 *
 */
void HardwareController::begin()
{
  LOG_INFO("Initializing hardware controller.");
#ifdef SIMULATION_MODE
  LOG_INFO("Initializing simulation mode.");
  Serial1.begin(115200, SERIAL_8N1, 16, 17);
#endif
  m_motor.begin();
  m_motor.setRampLen(0);
  LOG_INFO("Hardware controller initialized.");
}

/**
 * @brief Set the next state object
 *
 * @param state
 */
void HardwareController::setNextState(HardwareController::State state)
{
  if (m_nextState == state)
    return;

  m_nextState = state;
}

/**
 * @brief Loop function for the hardware controller
 *
 */
void HardwareController::loop()
{
  if (m_nextState != m_state) {
    if (m_turnFinished) {
      m_state = m_nextState;
    }
  }

#ifdef SIMULATION_MODE
  // Send motor position periodically
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 100) { // Update position every 100ms
    lastUpdate = millis();
    sendPositionToSerial();
  }
#endif

  switch (m_state) {
  case State::SingleTurn:
    handleSingleTurnState();
    break;
  case State::InfiniteTurn:
    handleInfiniteTurnState();
    break;
  case State::ManualTurn:
    handleManualTurnState();
    break;
  case State::Stop:
    handleStopState();
    break;
  case State::Idle:
    break;
  default:
    LOG_INFO("Unknown turn mode.");
  }
}

/**
 * @brief Handle the single turn state
 *
 */
void HardwareController::handleSingleTurnState()
{
  switch (m_motor.state()) {
  case Motor::State::Start:
    m_turnFinished = false;
    m_motor.setSpeed(m_config.hardware.singleSpeed());
    if (m_motor.currentPosition() != m_config.hardware.restPosition()) {
      m_motor.moveTo(m_config.hardware.restPosition());
      m_motor.setState(Motor::State::Prepare);
    } else {
      if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
        // ----------------- S
        if (m_config.hardware.hitDirection() ==
            HardwareConfig::HitDirection::Left) {
          if (m_config.hardware.hitPosition() <=
              m_config.hardware.restPosition()) {
            m_motor.moveTo(m_config.hardware.hitPosition());
          } else {
            m_motor.moveTo(m_config.hardware.hitPosition() -
                           Motor::StepsPerRevolution);
          }
        } else {
          if (m_config.hardware.hitPosition() <=
              m_config.hardware.restPosition()) {
            m_motor.moveTo(m_config.hardware.hitPosition() +
                           Motor::StepsPerRevolution);
          } else {
            m_motor.moveTo(m_config.hardware.hitPosition());
          }
        }
        // ----------------- E
      } else if (m_config.hardware.turnType() ==
                 HardwareConfig::TurnType::FullTurn) {
        if (m_config.hardware.hitDirection() ==
            HardwareConfig::HitDirection::Left) {
          m_motor.move(-Motor::StepsPerRevolution);
        } else {
          m_motor.move(Motor::StepsPerRevolution);
        }
      }
      m_motor.setState(Motor::State::ToHit);
    }
    LOG_INFO("Single mode started. Speed: " +
             String(m_config.hardware.singleSpeed()));
    break;
  case Motor::State::Prepare:
    if (!m_motor.isRunning()) {
      m_motor.setState(Motor::State::PausePrepare);
      m_startPauseMillis = millis();
    }
    break;
  case Motor::State::PausePrepare:
    if (millis() - m_startPauseMillis > 1000) {
      if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
        // ----------------- S
        if (m_config.hardware.hitDirection() ==
            HardwareConfig::HitDirection::Left) {
          if (m_config.hardware.hitPosition() <=
              m_config.hardware.restPosition()) {
            m_motor.moveTo(m_config.hardware.hitPosition());
          } else {
            m_motor.moveTo(m_config.hardware.hitPosition() -
                           Motor::StepsPerRevolution);
          }
        } else {
          if (m_config.hardware.hitPosition() <=
              m_config.hardware.restPosition()) {
            m_motor.moveTo(m_config.hardware.hitPosition() +
                           Motor::StepsPerRevolution);
          } else {
            m_motor.moveTo(m_config.hardware.hitPosition());
          }
        }
        // ----------------- E
      } else if (m_config.hardware.turnType() ==
                 HardwareConfig::TurnType::FullTurn) {
        if (m_config.hardware.hitDirection() ==
            HardwareConfig::HitDirection::Left) {
          m_motor.move(-Motor::StepsPerRevolution);
        } else {
          m_motor.move(Motor::StepsPerRevolution);
        }
      }
      m_motor.setState(Motor::State::ToHit);
    }
    break;
  case Motor::State::ToHit:
    if (!m_motor.isRunning()) {
      m_motor.setState(Motor::State::PauseHit);
      m_startPauseMillis = millis();
    }
    break;
  case Motor::State::PauseHit:
    if ((millis() - m_startPauseMillis) >
        (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn
             ? 1000
             : 300)) {
      m_motor.setRampLen(10);
      m_motor.setSpeed(90);
      if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
        m_motor.moveTo(m_config.hardware.restPosition());
        m_motor.setState(Motor::State::ToRest);
      } else if (m_config.hardware.turnType() ==
                 HardwareConfig::TurnType::FullTurn) {
        m_motor.setState(Motor::State::Start);
        if (m_config.hardware.hitDirection() ==
            HardwareConfig::HitDirection::Left) {
          m_config.hardware.setHitPosition(m_config.hardware.hitPosition() -
                                           Motor::StepsPerRevolution);
          m_config.hardware.setRestPosition(m_config.hardware.restPosition() -
                                            Motor::StepsPerRevolution);
        } else {
          m_config.hardware.setHitPosition(m_config.hardware.hitPosition() +
                                           Motor::StepsPerRevolution);
          m_config.hardware.setRestPosition(m_config.hardware.restPosition() +
                                            Motor::StepsPerRevolution);
        }
        m_nextState = State::Stop;
        m_turnFinished = true;
        LOG_INFO("Single mode FULL_TURN finished. " +
                 m_config.hardware.restPosition() + " " +
                 m_config.hardware.hitPosition());
      }
    }
    break;
  case Motor::State::ToRest:
    if (!m_motor.isRunning()) {
      m_motor.setState(Motor::State::Start);
      m_nextState = State::Stop;
      m_turnFinished = true;
      m_motor.setRampLen(0);
      LOG_INFO("Single mode HALF_TURN finished.");
    }
    break;
  default:
    LOG_INFO("Unknown motor state in SINGLE mode.");
  }
}

/**
 * @brief Handle the infinite turn state
 *
 */
void HardwareController::handleInfiniteTurnState()
{
  switch (m_motor.state()) {
  case Motor::State::Start:
    m_turnFinished = false;
    m_motor.setSpeed(m_config.hardware.infiniteSpeed());
    if (m_motor.currentPosition() != m_config.hardware.restPosition()) {
      m_motor.moveTo(m_config.hardware.restPosition());
      m_motor.setState(Motor::State::Prepare);
    } else {
      if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
        // ----------------- S
        if (m_config.hardware.hitDirection() ==
            HardwareConfig::HitDirection::Left) {
          if (m_config.hardware.hitPosition() <=
              m_config.hardware.restPosition()) {
            m_motor.moveTo(m_config.hardware.hitPosition());
          } else {
            m_motor.moveTo(m_config.hardware.hitPosition() -
                           Motor::StepsPerRevolution);
          }
        } else {
          if (m_config.hardware.hitPosition() <=
              m_config.hardware.restPosition()) {
            m_motor.moveTo(m_config.hardware.hitPosition() +
                           Motor::StepsPerRevolution);
          } else {
            m_motor.moveTo(m_config.hardware.hitPosition());
          }
        }
        // ----------------- E
      }
      m_motor.setState(Motor::State::ToHit);
    }
    LOG_INFO("Infinite mode started. Speed: " +
             String(m_config.hardware.infiniteSpeed()));
    break;
  case Motor::State::Prepare:
    if (!m_motor.isRunning()) {
      m_motor.setState(Motor::State::PausePrepare);
      m_startPauseMillis = millis();
    }
    break;
  case Motor::State::PausePrepare:
    if (millis() - m_startPauseMillis > 1000) {
      if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
        // ----------------- S
        if (m_config.hardware.hitDirection() ==
            HardwareConfig::HitDirection::Left) {
          if (m_config.hardware.hitPosition() <=
              m_config.hardware.restPosition()) {
            m_motor.moveTo(m_config.hardware.hitPosition());
          } else {
            m_motor.moveTo(m_config.hardware.hitPosition() -
                           Motor::StepsPerRevolution);
          }
        } else {
          if (m_config.hardware.hitPosition() <=
              m_config.hardware.restPosition()) {
            m_motor.moveTo(m_config.hardware.hitPosition() +
                           Motor::StepsPerRevolution);
          } else {
            m_motor.moveTo(m_config.hardware.hitPosition());
          }
        }
        // ----------------- E
      }
      m_motor.setState(Motor::State::ToHit);
    }
    break;
  case Motor::State::ToHit:
    if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
      if (!m_motor.isRunning()) {
        m_motor.setState(Motor::State::PauseHit);
        m_startPauseMillis = millis();
      }
    } else if (m_config.hardware.turnType() ==
               HardwareConfig::TurnType::FullTurn) {
      m_turnFinished = false;
      if (m_config.hardware.hitDirection() ==
          HardwareConfig::HitDirection::Left) {
        m_motor.runBackward();
        if (m_motor.currentPosition() <=
            m_config.hardware.restPosition() - Motor::StepsPerRevolution) {
          m_config.hardware.setHitPosition(m_motor.currentPosition() +
                                           (m_config.hardware.hitPosition() -
                                            m_config.hardware.restPosition()));
          m_config.hardware.setRestPosition(m_motor.currentPosition());
          m_turnFinished = true;
        }
      } else {
        m_motor.runForward();
        if (m_motor.currentPosition() >=
            m_config.hardware.restPosition() + Motor::StepsPerRevolution) {
          m_config.hardware.setHitPosition(m_motor.currentPosition() +
                                           (m_config.hardware.hitPosition() -
                                            m_config.hardware.restPosition()));
          m_config.hardware.setRestPosition(m_motor.currentPosition());
          m_turnFinished = true;
        }
      }
      if (m_turnFinished && m_nextState == State::Stop) {
        m_motor.setState(Motor::State::Start);
        m_state = State::Stop;
        LOG_INFO("Infinite mode FULL_TURN finished.");
      }
    }
    break;
  case Motor::State::PauseHit:
    if (millis() - m_startPauseMillis >= 1000) {
      m_motor.setRampLen(10);
      m_motor.setSpeed(90);
      m_motor.moveTo(m_config.hardware.restPosition());
      m_motor.setState(Motor::State::ToRest);
    }
    break;
  case Motor::State::ToRest:
    if (!m_motor.isRunning()) {
      m_motor.setState(Motor::State::PauseRest);
      m_startPauseMillis = millis();
    }
    break;
  case Motor::State::PauseRest:
    if (millis() - m_startPauseMillis >= 1000) {
      m_motor.setState(Motor::State::Start);
      m_turnFinished = true;
      m_motor.setRampLen(0);
      LOG_INFO("Infinite mode HALF_TURN finished.");
    }
    break;
  default:
    LOG_INFO("Unknown motor state in INFINITE mode.");
    break;
  }
}

/**
 * @brief Handle the manual turn state
 *
 */
void HardwareController::handleManualTurnState()
{
  switch (m_motor.state()) {
  case Motor::State::Start:
    m_turnFinished = false;
    m_motor.setRampLen(3);
    m_motor.setSpeed(30);
    if (m_manualCommand == ManualCommand::Left) {
      m_motor.move(-15);
      m_motor.setState(Motor::State::RotateCCW);
      LOG_INFO("Manual move forward started. Speed: 30");
    } else if (m_manualCommand == ManualCommand::Right) {
      m_motor.move(15);
      m_motor.setState(Motor::State::RotateCW);
      LOG_INFO("Manual move backward started. Speed: 30");
    }
    break;
  case Motor::State::RotateCW:
    if (!m_motor.isRunning()) {
      LOG_INFO("Manual move forward finished. Current Position: " +
               m_motor.currentPosition());
      m_motor.setState(Motor::State::Start);
      m_nextState = State::Stop;
      m_turnFinished = true;
      m_motor.setRampLen(0);
    }
    break;
  case Motor::State::RotateCCW:
    if (!m_motor.isRunning()) {
      LOG_INFO("Manual move back finished. Current Position: " +
               m_motor.currentPosition());
      m_motor.setState(Motor::State::Start);
      m_nextState = State::Stop;
      m_turnFinished = true;
      m_motor.setRampLen(0);
    }
    break;
  default:
    LOG_INFO("Unknown motor state in MANUAL mode.");
  }
}

/**
 * @brief Handle the stop turn state
 *
 */
void HardwareController::handleStopState()
{
  m_motor.disableMotor();
  m_motor.setState(Motor::State::Start);
  m_state = State::Idle;
}

void HardwareController::sendPositionToSerial()
{
  static int32_t lastPosition = 0;
  if (lastPosition == m_motor.currentPosition())
    return;

  Serial1.print("POSITION:");
  Serial1.println(m_motor.currentPosition());
  lastPosition = m_motor.currentPosition();
}