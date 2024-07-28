
#ifndef CUSTOM_FUNCTIONS_H
#define CUSTOM_FUNCTIONS_H
// functions to control the device that are seperate from the stepper motor
#include <Arduino.h>

/**
 * @brief function to calcualte a random values. The values offer normal
 * distribution arround a definded value.
 * @param lowerRange the lowest value the function can return
 * @param upperRange the highest value the function can return
 * @param centerValue the value the gaus curve is centered arround
 * @param sigma the sigma value of the curve
 * @return random value
 * @
 */
double generateRandomValueGaus(double lowerRange, double upperRange,
                               double centerValue, double sigma);

// Function to save WiFi credentials
bool saveCredentials(const String& ssid, const String& password);
// Function to read WiFi credentials
bool readCredentials(String& ssid, String& password);

#endif // CUSTOM_FUNCTIONS_H