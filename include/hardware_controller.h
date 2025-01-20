#ifndef HARDWARE_CONTROLLER_H
#define HARDWARE_CONTROLLER_H

#include "command_queue.h"
#include "motor_controller.h"

enum class Mode {
    STOP = 0,
    SINGLE,
    INFINITE,
    MANUAL,
};

enum class TurnType {
    FULL_TURN = 0,
    HALF_TURN
};

enum class MotorState {
    START = 0,
    ROTATE_FORWARD,
    PAUSE_FORWARD,
    ROTATE_BACK,
    PAUSE_BACK
};

enum class ManualCommand {
    Forward = 0,
    Backward
};

class HardwareController {
public:
    HardwareController(uint8_t motorStepPin, uint8_t motorDirPin, uint8_t motorEnPin);
    ~HardwareController();
    
    void init();
    void poll();
    void processCommand();

    void setCommandQueue(CommandQueue *queue) { m_commandQueue = queue; };

private:
    CommandQueue *m_commandQueue;
    MotorController m_motorController;

    Mode m_mode;
    TurnType m_turnType;
    MotorState m_motorState;
    ManualCommand m_manualCommand;
    bool m_turnFinished;
    bool m_frontPosDefined;
    bool m_rearPosDefined;
    int32_t m_frontPos;
    int32_t m_rearPos;
    int m_singleSpeed;
    int m_infiniteSpeed;
    
    int m_maxHalfSpeed;
    int m_maxFullSpeed;

    long m_startPauseForwardMillis;
    long m_startPauseBackMillis;

    void handleSingleMode();
    void handleInfiniteMode();
    void handleManualMode();
    void handleStopMode();
    void spin();
};

#endif // HARDWARE_CONTROLLER_H
