#ifndef HARDWARE_CONTROLLER_H
#define HARDWARE_CONTROLLER_H

#include "command_queue.h"

class HardwareController {
public:
    HardwareController();
    ~HardwareController();
    
    void init();
    void poll();
    void processCommand(const Command &cmd);

    void setCommandQueue(CommandQueue *queue) { m_commandQueue = queue; };

private:
    CommandQueue *m_commandQueue;
};

#endif // HARDWARE_CONTROLLER_H
