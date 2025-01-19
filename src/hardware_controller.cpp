#include "hardware_controller.h"

HardwareController::HardwareController() {}

HardwareController::~HardwareController() {}

void HardwareController::init() {
    // Initialize hardware components like motors and sensors
    Serial.println("Hardware controller initialized.");
}

void HardwareController::poll() {
    // Poll for commands and process them
    Command cmd;
    if (m_commandQueue->getNextCommand(cmd)) {
        processCommand(cmd);
        m_commandQueue->removeCommand();
    }
}

void HardwareController::processCommand(const Command &cmd) {
    // Process the command
    switch (cmd.type) {
        case STOP:
            Serial.println("STOP command received.");
            // Handle STOP command
            break;
        case START_SIGNLE:
            Serial.println("START_SINGLE command received.");
            // Handle START_SINGLE command
            break;
        case START_INFINITE:
            Serial.println("START_INFINITE command received.");
            // Handle START_INFINITE command
            break;
        case SETTING_TURN_TYPE:
            Serial.println("SETTING_TURN_TYPE command received.");
            // Handle SETTING_TURN_TYPE command
            break;
        case SETTING_SET_REAR:
            Serial.println("SETTING_SET_REAR command received.");
            // Handle SETTING_SET_REAR command
            break;
        case SETTING_SET_FRONT:
            Serial.println("SETTING_SET_FRONT command received.");
            // Handle SETTING_SET_FRONT command
            break;
        case SETTING_MAX_HALF_SPEED:
            Serial.println("SETTING_MAX_HALF_SPEED command received.");
            // Handle SETTING_MAX_HALF_SPEED command
            break;
        case SETTING_MAX_FULL_SPEED:
            Serial.println("SETTING_MAX_FULL_SPEED command received.");
            // Handle SETTING_MAX_FULL_SPEED command
            break;
        case COMMAND_UP:
            Serial.println("COMMAND_UP command received.");
            // Handle COMMAND_UP command
            break;
        case COMMAND_DOWN:
            Serial.println("COMMAND_DOWN command received.");
            // Handle COMMAND_DOWN command
            break;
        default:
            Serial.println("Unknown command type.");
            break;
    }
}
