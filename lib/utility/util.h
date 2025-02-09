#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include <mongoose.h>

class Util
{
public:
  static String getMacAddress();
  // Function to get number of mongoose connections
  static int getNumberOfConnections(struct mg_mgr *mgr);

  static bool timeElapsed(float seconds, long startT);
};

#endif // UTIL_H