#include "hardware_controller.h"

HardwareController::HardwareController(uint8_t motorStepPin, uint8_t motorDirPin, uint8_t motorEnPin) : 
    m_motorController(motorStepPin, motorDirPin, motorEnPin), m_mode(Mode::STOP), m_motorState(MotorState::START),
    m_turnFinished(false), m_frontPosDefined(false), m_rearPosDefined(false), m_frontPos(0), m_rearPos(0),
    m_singleSpeed(100), m_infiniteSpeed(200), m_maxHalfSpeed(1000), m_maxFullSpeed(1000)
{

}

HardwareController::~HardwareController() 
{

}

void HardwareController::init() 
{
    Serial.println("\nInitializing hardware controller...");
    m_motorController.begin();
    m_motorController.setAcceleration(100000);
    Serial.println("\nHardware controller initialized.");
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
                Serial.println("STOP command received.");
                m_mode = Mode::STOP;
                break;
            case CommandType::START_SIGNLE:
                Serial.println("START_SINGLE command received.");
                if (m_frontPosDefined && m_rearPosDefined) {
                    m_singleSpeed = cmd.value;
                    m_mode = Mode::SINGLE;
                } else {
                    Serial.println("Define positions first!");
                    m_mode =  Mode::STOP;
                }
                break;
            case CommandType::START_INFINITE:
                Serial.println("START_INFINITE command received.");
                if (m_frontPosDefined && m_rearPosDefined) {
                    m_infiniteSpeed = cmd.value;
                    m_mode = Mode::INFINITE;
                } else {
                    Serial.println("Define positions first!");
                    m_mode = Mode::STOP;
                }
                break;
            case CommandType::SETTING_TURN_TYPE:
                Serial.println("SETTING_TURN_TYPE command received.");
                if ((TurnType) cmd.value == TurnType::FULL_TURN) {
                    m_turnType = TurnType::FULL_TURN;
                } else if ((TurnType) cmd.value == TurnType::HALF_TURN) {
                    m_turnType = TurnType::HALF_TURN;
                } else {
                    Serial.println("Invalid Turn Type!");
                }
                break;
            case CommandType::SETTING_SET_REAR:
                Serial.println("SETTING_SET_REAR command received.");
                m_rearPos = m_motorController.currentPosition();
                Serial.print("Rear Position: ");
                Serial.println(m_rearPos);
                m_rearPosDefined = true;
                break;
            case CommandType::SETTING_SET_FRONT:
                Serial.println("SETTING_SET_FRONT command received.");
                m_frontPos = m_motorController.currentPosition();
                Serial.print("Front Position: ");
                Serial.println(m_frontPos);
                m_frontPosDefined = true;
                break;
            case CommandType::SETTING_MAX_HALF_SPEED:
                Serial.println("SETTING_MAX_HALF_SPEED command received.");
                m_maxHalfSpeed = cmd.value;
                break;
            case CommandType::SETTING_MAX_FULL_SPEED:
                Serial.println("SETTING_MAX_FULL_SPEED command received.");
                m_maxFullSpeed = cmd.value;
                break;
            case CommandType::COMMAND_UP:
                Serial.println("COMMAND_UP command received.");
                if (m_mode == Mode::STOP) {
                    m_mode = Mode::MANUAL;
                    m_manualCommand = ManualCommand::Forward;
                }
                break;
            case CommandType::COMMAND_DOWN:
                Serial.println("COMMAND_DOWN command received.");
                if (m_mode == Mode::STOP) {
                    m_mode = Mode::MANUAL;
                    m_manualCommand = ManualCommand::Backward;
                }
                break;
            default:
                Serial.println("Unknown command type.");
                break;
        }
        m_commandQueue->removeCommand();
    }
}

void HardwareController::spin()
{
    switch (m_mode) {
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
        Serial.println("Unknown turn mode.");
        break;
    }
}

void HardwareController::handleSingleMode() 
{
    switch (m_motorState) {
        case MotorState::START:
            m_motorController.setSpeed(m_singleSpeed * 100);
            m_motorState = MotorState::ROTATE_FORWARD;
            Serial.println("Single mode started. Speed: " + String(m_singleSpeed));
            break;
        case MotorState::ROTATE_FORWARD:
            m_motorController.moveTo(m_turnType == TurnType::HALF_TURN ? m_frontPos : 200);
            if (!m_motorController.isRunning()) {
                m_motorState = MotorState::PAUSE_FORWARD;
                m_startPauseForwardMillis = millis();
            }
            break;
        case MotorState::PAUSE_FORWARD:
            if ((millis() - m_startPauseForwardMillis) > (m_turnType == TurnType::HALF_TURN ? 1000 : 300)) {
                m_motorController.setSpeed(25 * 100);
                m_motorState = MotorState::ROTATE_BACK;
            }
            break;
        case MotorState::ROTATE_BACK:
            m_motorController.moveTo(m_turnType == TurnType::HALF_TURN ? m_rearPos : 0);
            if (!m_motorController.isRunning()) {
                m_motorState = MotorState::START;
                m_mode = Mode::STOP;
                m_turnFinished = true;
                Serial.println("Single mode finished.");
            }
            break;
        default:
            Serial.println("Unknown motor state in SINGLE mode.");
            break;
    }

}

void HardwareController::handleInfiniteMode() 
{
    switch (m_motorState) {
        case MotorState::START:
            m_motorController.setSpeed(m_infiniteSpeed * 100);
            m_motorState = MotorState::ROTATE_FORWARD;
            Serial.println("Infinite mode started. Speed: " + String(m_infiniteSpeed));
            break;
        case MotorState::ROTATE_FORWARD:
            if (m_turnType == TurnType::HALF_TURN) {
                m_motorController.moveTo(m_frontPos);
                if (!m_motorController.isRunning()) {
                    m_motorState = MotorState::PAUSE_FORWARD;
                    m_startPauseForwardMillis = millis();
                }
            } else if (m_turnType == TurnType::FULL_TURN) {
                m_motorController.runForward();
            }
            break;
        case MotorState::PAUSE_FORWARD:
            if (millis() - m_startPauseForwardMillis >= 1000) {
                m_motorController.setSpeed(25 * 100);
                m_motorState = MotorState::ROTATE_BACK;
            }
            break;
        case MotorState::ROTATE_BACK:
            m_motorController.moveTo(m_rearPos);
            if (!m_motorController.isRunning()) {
                m_motorState = MotorState::PAUSE_BACK;
                m_startPauseBackMillis = millis();
            }
            break;
        case MotorState::PAUSE_BACK:
            if (millis() - m_startPauseBackMillis >= 1000) {
                m_motorState = MotorState::START;
                m_turnFinished = true;
            }
            break;
        default:
            Serial.println("Unknown motor state in INFINITE mode.");
            break;
    }
}

void HardwareController::handleManualMode()
{
    switch (m_motorState) {
        case MotorState::START:
            m_motorController.setSpeed(100 * 100);
            if (m_manualCommand == ManualCommand::Forward) {
                m_motorState = MotorState::ROTATE_FORWARD;
                Serial.println("Manual move forward started. Speed: " + String(m_singleSpeed));
            } else {
                m_motorState = MotorState::ROTATE_BACK;
                Serial.println("Manual move backward started. Speed: " + String(m_singleSpeed));
            }
            break;
        case MotorState::ROTATE_FORWARD:
            m_motorController.move(10);
            if (!m_motorController.isRunning()) {
                Serial.print("Manual move forward finished. Current Position: ");
                Serial.print("Current Position: ");
                Serial.println(m_motorController.currentPosition());
                m_motorState = MotorState::START;
                m_mode = Mode::STOP;
                m_turnFinished = true;
            }
            break;
        case MotorState::ROTATE_BACK:
            m_motorController.move(-10);
            if (!m_motorController.isRunning()) {
                Serial.print("Manual move back finished. Current Position: ");
                Serial.print("Current Position: ");
                Serial.println(m_motorController.currentPosition());
                m_motorState = MotorState::START;
                m_mode = Mode::STOP;
                m_turnFinished = true;
            }
            break;
        default:
            Serial.println("Unknown motor state in MANUAL mode.");
            break;
    }
}

void HardwareController::handleStopMode()
{
    if (m_motorController.isRunning()) {
        m_motorController.disableMotor();
    }
}