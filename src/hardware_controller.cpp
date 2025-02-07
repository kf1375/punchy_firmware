#include "hardware_controller.h"
#include "logging.h"

HardwareController::HardwareController(uint8_t motorStepPin, uint8_t motorDirPin, uint8_t motorEnPin) : 
    m_motorController(motorStepPin, motorDirPin, motorEnPin), m_currentMode(Mode::STOP), m_nextMode(Mode::STOP), m_motorState(MotorState::START),
    m_turnFinished(true), m_frontPosDefined(false), m_rearPosDefined(false), m_frontPos(0), m_turnType(TurnType::HALF_TURN),
    m_singleSpeed(100), m_infiniteSpeed(200), m_maxHalfSpeed(1000), m_maxFullSpeed(1000)
{

}

HardwareController::~HardwareController() 
{

}

void HardwareController::init() 
{
    LOG_INFO("Initializing hardware controller.");
    m_motorController.begin();
    m_motorController.setRampLen(0);
    m_motorController.move(15);
    LOG_INFO("Hardware controller initialized.");
}

void HardwareController::poll() 
{    
    if (m_commandQueue) {
        processCommand();
    }
    spin();
}

void HardwareController::processCommand() 
{
    Command cmd;
    if (m_commandQueue->getNextCommand(cmd)) {
        switch (cmd.type) {
            case CommandType::STOP:
                LOG_INFO("STOP command received.");
                m_nextMode = Mode::STOP;
                break;
            case CommandType::START_SIGNLE:
                LOG_INFO("START_SINGLE command received.");
                if (m_frontPosDefined && m_rearPosDefined) {
                    m_singleSpeed = cmd.value;
                    m_nextMode = Mode::SINGLE;
                } else {
                    LOG_INFO("Define positions first!");
                }
                break;
            case CommandType::START_INFINITE:
                LOG_INFO("START_INFINITE command received.");
                if (m_frontPosDefined && m_rearPosDefined) {
                    m_infiniteSpeed = cmd.value;
                    m_nextMode = Mode::INFINITE;
                } else {
                    LOG_INFO("Define positions first!");
                }
                break;
            case CommandType::SETTING_TURN_TYPE:
                LOG_INFO("SETTING_TURN_TYPE command received.");
                if ((TurnType) cmd.value == TurnType::FULL_TURN) {
                    m_turnType = TurnType::FULL_TURN;
                } else if ((TurnType) cmd.value == TurnType::HALF_TURN) {
                    m_turnType = TurnType::HALF_TURN;
                } else {
                    LOG_INFO("Invalid Turn Type!");
                }
                break;
            case CommandType::SETTING_SET_REAR:
                LOG_INFO("SETTING_SET_REAR command received.");
                if (m_frontPosDefined) {
                    m_frontPos = m_frontPos - m_motorController.currentPosition();
                }
                m_motorController.setZero();
                m_rearPosDefined = true;
                break;
            case CommandType::SETTING_SET_FRONT:
                LOG_INFO("SETTING_SET_FRONT command received.");
                Serial.print("Current Position: ");
                LOG_INFO(m_motorController.currentPosition());
                m_frontPos = m_motorController.currentPosition();
                m_frontPosDefined = true;
                break;
            case CommandType::SETTING_MAX_HALF_SPEED:
                LOG_INFO("SETTING_MAX_HALF_SPEED command received.");
                m_maxHalfSpeed = cmd.value;
                break;
            case CommandType::SETTING_MAX_FULL_SPEED:
                LOG_INFO("SETTING_MAX_FULL_SPEED command received.");
                m_maxFullSpeed = cmd.value;
                break;
            case CommandType::COMMAND_UP:
                LOG_INFO("COMMAND_UP command received.");
                if (m_currentMode == Mode::STOP) {
                    m_nextMode = Mode::MANUAL;
                    m_manualCommand = ManualCommand::Forward;
                }
                break;
            case CommandType::COMMAND_DOWN:
                LOG_INFO("COMMAND_DOWN command received.");
                if (m_currentMode == Mode::STOP) {
                    m_nextMode = Mode::MANUAL;
                    m_manualCommand = ManualCommand::Backward;
                }
                break;
            default:
                LOG_INFO("Unknown command type.");
                break;
        }
        m_commandQueue->removeCommand();
    }
}

void HardwareController::spin()
{
    if (m_nextMode != m_currentMode) {
        if (m_turnFinished) {
            m_currentMode = m_nextMode;
        }
    }
    switch (m_currentMode) {
    case Mode::SINGLE:
        handleSingleMode();
        break;
    case Mode::INFINITE:
        handleInfiniteMode();
        break;
    case Mode::MANUAL:
        handleManualMode();
        break;
    case Mode::STOP:
        handleStopMode();
        break;
    default:
        LOG_INFO("Unknown turn mode.");
    }
}

void HardwareController::handleSingleMode() 
{
    switch (m_motorState) {
        case MotorState::START:
            m_turnFinished = false;
            m_motorController.setSpeed(m_singleSpeed);
            if (m_turnType == TurnType::HALF_TURN) {
                m_motorController.moveTo(m_frontPos);
            } else if (m_turnType == TurnType::FULL_TURN) {
                m_motorController.moveTo(STEPS_PER_REVOLUTION);
            }
            m_motorState = MotorState::ROTATE_FORWARD;
            LOG_INFO("Single mode started. Speed: " + String(m_singleSpeed));
            break;
        case MotorState::ROTATE_FORWARD:
            if (!m_motorController.isRunning()) {
                m_motorState = MotorState::PAUSE_FORWARD;
                m_startPauseForwardMillis = millis();
            }
            break;
        case MotorState::PAUSE_FORWARD:
            if ((millis() - m_startPauseForwardMillis) > (m_turnType == TurnType::HALF_TURN ? 1000 : 300)) {
                m_motorController.setRampLen(10);
                m_motorController.setSpeed(90);
                if (m_turnType == TurnType::HALF_TURN) {
                    m_motorController.moveTo(0);
                    m_motorState = MotorState::ROTATE_BACK;
                } else if (m_turnType == TurnType::FULL_TURN) {
                    m_motorController.setZero();
                    m_motorState = MotorState::START;
                    m_nextMode = Mode::STOP;
                    m_turnFinished = true;
                    m_motorController.setRampLen(0);
                    LOG_INFO("Single mode FULL_TURN finished.");
                }
            }
            break;
        case MotorState::ROTATE_BACK:
            if (!m_motorController.isRunning()) {
                m_motorState = MotorState::START;
                m_nextMode = Mode::STOP;
                m_turnFinished = true;
                m_motorController.setRampLen(0);
                LOG_INFO("Single mode HALF_TURN finished.");
            }      
            break;
        default:
            LOG_INFO("Unknown motor state in SINGLE mode.");
    }

}

void HardwareController::handleInfiniteMode() 
{
    switch (m_motorState) {
        case MotorState::START:
            m_turnFinished = false;
            m_motorController.setSpeed(m_infiniteSpeed);
            if (m_turnType == TurnType::HALF_TURN) {
                m_motorController.moveTo(m_frontPos);
            }
            m_motorState = MotorState::ROTATE_FORWARD;
            LOG_INFO("Infinite mode started. Speed: " + String(m_infiniteSpeed));
            break;
        case MotorState::ROTATE_FORWARD:
            if (m_turnType == TurnType::HALF_TURN) {
                if (!m_motorController.isRunning()) {
                    m_motorState = MotorState::PAUSE_FORWARD;
                    m_startPauseForwardMillis = millis();
                }
            } else if (m_turnType == TurnType::FULL_TURN) {
                m_turnFinished = false;
                m_motorController.runForward();
                if (m_motorController.currentPosition() >= STEPS_PER_REVOLUTION) {
                    m_motorController.setZero();
                    m_turnFinished = true;
                }
                if (m_turnFinished && m_nextMode == Mode::STOP) {
                    m_motorState = MotorState::START;
                    m_currentMode = Mode::STOP;
                    LOG_INFO("Infinite mode FULL_TURN finished.");
                }
            }
            break;
        case MotorState::PAUSE_FORWARD:
            if (millis() - m_startPauseForwardMillis >= 1000) {
                m_motorController.setRampLen(10);
                m_motorController.setSpeed(90);
                m_motorController.moveTo(0);
                m_motorState = MotorState::ROTATE_BACK;
            }
            break;
        case MotorState::ROTATE_BACK:
            if (!m_motorController.isRunning()) {
                m_motorState = MotorState::PAUSE_BACK;
                m_startPauseBackMillis = millis();
            }
            break;
        case MotorState::PAUSE_BACK:
            if (millis() - m_startPauseBackMillis >= 1000) {
                m_motorState = MotorState::START;
                m_turnFinished = true;
                m_motorController.setRampLen(0);
                LOG_INFO("Infinite mode HALF_TURN finished.");
            }
            break;
        default:
            LOG_INFO("Unknown motor state in INFINITE mode.");
            break;
    }
}

void HardwareController::handleManualMode()
{
    switch (m_motorState) {
        case MotorState::START:
            m_turnFinished = false;
            m_motorController.setRampLen(3);
            m_motorController.setSpeed(30);
            if (m_manualCommand == ManualCommand::Forward) {
                m_motorController.move(15);
                m_motorState = MotorState::ROTATE_FORWARD;
                LOG_INFO("Manual move forward started. Speed: 30");
            } else if (m_manualCommand == ManualCommand::Backward) {
                m_motorController.move(-15);
                m_motorState = MotorState::ROTATE_BACK;
                LOG_INFO("Manual move backward started. Speed: 30");
            }
            break;
        case MotorState::ROTATE_FORWARD:
            if (!m_motorController.isRunning()) {
                Serial.print("Manual move forward finished. Current Position: ");
                LOG_INFO(m_motorController.currentPosition());
                m_motorState = MotorState::START;
                m_nextMode = Mode::STOP;
                m_turnFinished = true;
                m_motorController.setRampLen(0);
            }
            break;
        case MotorState::ROTATE_BACK:
            if (!m_motorController.isRunning()) {
                Serial.print("Manual move back finished. Current Position: ");
                LOG_INFO(m_motorController.currentPosition());
                m_motorState = MotorState::START;
                m_nextMode = Mode::STOP;
                m_turnFinished = true;
                m_motorController.setRampLen(0);
            }
            break;
        default:
            LOG_INFO("Unknown motor state in MANUAL mode.");
    }
}

void HardwareController::handleStopMode()
{
    m_motorController.disableMotor();
    m_motorState = MotorState::START;
}