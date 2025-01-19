#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include <mongoose.h>

class Util {

public:
    static String getMacAddress();
    static void printMgStr(const mg_str &str);

    // Function to save WiFi credentials
    static bool saveCredentials(const String& ssid, const String& password);
    // Function to read WiFi credentials
    static bool readCredentials(String& ssid, String& password);
};

#endif // UTIL_H