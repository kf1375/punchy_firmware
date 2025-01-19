#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include <Arduino.h>
#include "mongoose.h"

enum class CommandType {
    STOP = 0,
    START_SIGNLE,
    START_INFINITE,
    SETTING_TURN_TYPE,
    SETTING_SET_REAR,
    SETTING_SET_FRONT,
    SETTING_MAX_HALF_SPEED,
    SETTING_MAX_FULL_SPEED,
    COMMAND_UP,
    COMMAND_DOWN
};

struct Command {
    CommandType type;
    int32_t value; 
};

class CommandQueue {
public:
    CommandQueue();
    ~CommandQueue();

    bool addCommand(const Command &cmd);
    bool getNextCommand(Command &cmd);
    void removeCommand();

private:
    struct mg_queue m_queue;
    static const size_t MAX_QUEUE_SIZE = 100; // Adjust the queue size as needed
};

#endif // COMMAND_QUEUE_H
