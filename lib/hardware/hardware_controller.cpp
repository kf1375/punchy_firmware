#include "hardware_controller.h"

#include "logging.h"

/**
 * @brief Construct a new Hardware Controller:: Hardware Controller object
 *
 * @param config
 */
HardwareController::HardwareController(Configuration &config) : m_config(config), m_motor(), m_state(State::Idle), m_turnFinished(true) {}

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

// void HardwareController::processCommand()
// {
//     Command cmd;
//     if (m_commandQueue->getNextCommand(cmd)) {
//         switch (cmd.type) {
//             case CommandType::STOP:
//                 LOG_INFO("STOP command received.");
//                 m_nextMode = Mode::STOP;
//                 break;
//             case CommandType::START_SIGNLE:
//                 LOG_INFO("START_SINGLE command received.");
//                 if (m_frontPosDefined && m_rearPosDefined) {
//                     m_singleSpeed = cmd.value;
//                     m_nextMode = Mode::SINGLE;
//                 } else {
//                     LOG_INFO("Define positions first!");
//                 }
//                 break;
//             case CommandType::START_INFINITE:
//                 LOG_INFO("START_INFINITE command received.");
//                 if (m_frontPosDefined && m_rearPosDefined) {
//                     m_infiniteSpeed = cmd.value;
//                     m_nextMode = Mode::INFINITE;
//                 } else {
//                     LOG_INFO("Define positions first!");
//                 }
//                 break;
//             case CommandType::SETTING_TURN_TYPE:
//                 LOG_INFO("SETTING_TURN_TYPE command received.");
//                 if ((TurnType) cmd.value == HardwareConfig::TurnType::FULL_TURN) {
//                     m_turnType = HardwareConfig::TurnType::FULL_TURN;
//                 } else if ((TurnType) cmd.value == HardwareConfig::TurnType::HALF_TURN) {
//                     m_turnType = HardwareConfig::TurnType::HALF_TURN;
//                 } else {
//                     LOG_INFO("Invalid Turn Type!");
//                 }
//                 break;
//             case CommandType::SETTING_SET_REAR:
//                 LOG_INFO("SETTING_SET_REAR command received.");
//                 if (m_frontPosDefined) {
//                     m_frontPos = m_frontPos - m_motorController.currentPosition();
//                 }
//                 m_motorController.setZero();
//                 m_rearPosDefined = true;
//                 break;
//             case CommandType::SETTING_SET_FRONT:
//                 LOG_INFO("SETTING_SET_FRONT command received.");
//                 Serial.print("Current Position: ");
//                 LOG_INFO(m_motorController.currentPosition());
//                 m_frontPos = m_motorController.currentPosition();
//                 m_frontPosDefined = true;
//                 break;
//             case CommandType::SETTING_MAX_HALF_SPEED:
//                 LOG_INFO("SETTING_MAX_HALF_SPEED command received.");
//                 m_maxHalfSpeed = cmd.value;
//                 break;
//             case CommandType::SETTING_MAX_FULL_SPEED:
//                 LOG_INFO("SETTING_MAX_FULL_SPEED command received.");
//                 m_maxFullSpeed = cmd.value;
//                 break;
//             case CommandType::COMMAND_UP:
//                 LOG_INFO("COMMAND_UP command received.");
//                 if (m_currentMode == Mode::STOP) {
//                     m_nextMode = Mode::MANUAL;
//                     m_manualCommand = ManualCommand::Forward;
//                 }
//                 break;
//             case CommandType::COMMAND_DOWN:
//                 LOG_INFO("COMMAND_DOWN command received.");
//                 if (m_currentMode == Mode::STOP) {
//                     m_nextMode = Mode::MANUAL;
//                     m_manualCommand = ManualCommand::Backward;
//                 }
//                 break;
//             default:
//                 LOG_INFO("Unknown command type.");
//                 break;
//         }
//         m_commandQueue->removeCommand();
//     }
// }

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
    if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
      m_motor.moveTo(m_config.hardware.frontPosition());
    } else if (m_config.hardware.turnType() == HardwareConfig::TurnType::FullTurn) {
      m_motor.moveTo(STEPS_PER_REVOLUTION);
    }
    m_motor.setState(Motor::State::RotateForward);
    LOG_INFO("Single mode started. Speed: " + String(m_config.hardware.singleSpeed()));
    break;
  case Motor::State::RotateForward:
    if (!m_motor.isRunning()) {
      m_motor.setState(Motor::State::PauseForward);
      m_startPauseForwardMillis = millis();
    }
    break;
  case Motor::State::PauseForward:
    if ((millis() - m_startPauseForwardMillis) > (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn ? 1000 : 300)) {
      m_motor.setRampLen(10);
      m_motor.setSpeed(90);
      if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
        m_motor.moveTo(0);
        m_motor.setState(Motor::State::RotateBack);
      } else if (m_config.hardware.turnType() == HardwareConfig::TurnType::FullTurn) {
        m_motor.setZero();
        m_motor.setState(Motor::State::Start);
        m_nextState = State::Stop;
        m_turnFinished = true;
        m_motor.setRampLen(0);
        LOG_INFO("Single mode FULL_TURN finished.");
      }
    }
    break;
  case Motor::State::RotateBack:
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

void HardwareController::handleInfiniteTurnState()
{
  switch (m_motor.state()) {
  case Motor::State::Start:
    m_turnFinished = false;
    m_motor.setSpeed(m_config.hardware.infiniteSpeed());
    if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
      m_motor.moveTo(m_config.hardware.frontPosition());
    }
    m_motor.setState(Motor::State::RotateForward);
    LOG_INFO("Infinite mode started. Speed: " + String(m_config.hardware.infiniteSpeed()));
    break;
  case Motor::State::RotateForward:
    if (m_config.hardware.turnType() == HardwareConfig::TurnType::HalfTurn) {
      if (!m_motor.isRunning()) {
        m_motor.setState(Motor::State::PauseForward);
        m_startPauseForwardMillis = millis();
      }
    } else if (m_config.hardware.turnType() == HardwareConfig::TurnType::FullTurn) {
      m_turnFinished = false;
      m_motor.runForward();
      if (m_motor.currentPosition() >= STEPS_PER_REVOLUTION) {
        m_motor.setZero();
        m_turnFinished = true;
      }
      if (m_turnFinished && m_nextState == State::Stop) {
        m_motor.setState(Motor::State::Start);
        m_state = State::Stop;
        LOG_INFO("Infinite mode FULL_TURN finished.");
      }
    }
    break;
  case Motor::State::PauseForward:
    if (millis() - m_startPauseForwardMillis >= 1000) {
      m_motor.setRampLen(10);
      m_motor.setSpeed(90);
      m_motor.moveTo(0);
      m_motor.setState(Motor::State::RotateBack);
    }
    break;
  case Motor::State::RotateBack:
    if (!m_motor.isRunning()) {
      m_motor.setState(Motor::State::PauseBack);
      m_startPauseBackMillis = millis();
    }
    break;
  case Motor::State::PauseBack:
    if (millis() - m_startPauseBackMillis >= 1000) {
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

void HardwareController::handleManualTurnState()
{
  switch (m_motor.state()) {
  case Motor::State::Start:
    m_turnFinished = false;
    m_motor.setRampLen(3);
    m_motor.setSpeed(30);
    if (m_manualCommand == ManualCommand::Forward) {
      m_motor.move(15);
      m_motor.setState(Motor::State::RotateForward);
      LOG_INFO("Manual move forward started. Speed: 30");
    } else if (m_manualCommand == ManualCommand::Backward) {
      m_motor.move(-15);
      m_motor.setState(Motor::State::RotateBack);
      LOG_INFO("Manual move backward started. Speed: 30");
    }
    break;
  case Motor::State::RotateForward:
    if (!m_motor.isRunning()) {
      Serial.print("Manual move forward finished. Current Position: ");
      LOG_INFO(m_motor.currentPosition());
      m_motor.setState(Motor::State::Start);
      m_nextState = State::Stop;
      m_turnFinished = true;
      m_motor.setRampLen(0);
    }
    break;
  case Motor::State::RotateBack:
    if (!m_motor.isRunning()) {
      Serial.print("Manual move back finished. Current Position: ");
      LOG_INFO(m_motor.currentPosition());
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

void HardwareController::handleStopState()
{
  m_motor.disableMotor();
  m_motor.setState(Motor::State::Start);
  m_state = State::Idle;
}