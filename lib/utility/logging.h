#ifndef LOGGING_H
#define LOGGING_H

#include "Arduino.h"

#define LOG_INFO(log)                                                          \
  Serial.println("[INFO] (" + String(xPortGetCoreID()) + ") " +                \
                 String(__FILE__) + ":" + String(__LINE__) + " : " + log);

#endif // LOGGING_H