#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include <mongoose.h>

class Util {

public:
    static String getMacAddress();
    static void printMgStr(const mg_str &str);
};

#endif // UTIL_H