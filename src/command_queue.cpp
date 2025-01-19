#include "command_queue.h"

CommandQueue::CommandQueue() 
{
    // Initialize the mg_queue with a buffer size
    static char buf[MAX_QUEUE_SIZE];
    mg_queue_init(&m_queue, buf, sizeof(buf));
}

CommandQueue::~CommandQueue() {}

bool CommandQueue::addCommand(const Command &cmd) 
{
    char *ptr;
    size_t len = sizeof(cmd);  // Size of the Command struct

    if (mg_queue_book(&m_queue, &ptr, len) < len) {
        return false; // Not enough space to add command
    }
    
    memcpy(ptr, &cmd, len);  // Copy the command to the queue
    mg_queue_add(&m_queue, len); // Add the command to the queue
    return true;
}

bool CommandQueue::getNextCommand(Command &cmd) 
{
    char *ptr;
    size_t len = mg_queue_next(&m_queue, &ptr);
    
    if (len == 0) {
        return false; // No messages in the queue
    }

    memcpy(&cmd, ptr, len);  // Copy the command from the queue
    return true;
}

void CommandQueue::removeCommand() 
{
    char *ptr;
    size_t len = mg_queue_next(&m_queue, &ptr);
    if (len > 0) {
        mg_queue_del(&m_queue, len);  // Remove the command from the queue
    }
}
