
#include "custom_functions.h"

#include <iostream>
#include <random>
#include <FS.h>
#include <LittleFS.h>

//---------------------------------------------------------------------------
double generateRandomValueGaus(double lowerRange, double upperRange,
                               double centerValue, double sigma) {
  // Random engine and normal distribution
  static std::mt19937 generator(std::random_device{}());
  std::normal_distribution<double> distribution(centerValue, sigma);

  // Generate value and clamp it within the range
  double value;
  do {
    value = distribution(generator);
  } while (value < lowerRange || value > upperRange);

  return value;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// void runAutomaticMode() {
//  // This function is called when the automatic mode is enabled
//  // It is called in the main loop and should be used to control the device
//  // based on the current state of the system
//  // The function should be non-blocking and return as fast as possible
//
//  // Example: Generate a random value with a normal distribution
//  double value = generateRandomValueGaus(0, 100, 50, 10);
//
//  // Print the generated value
//  std::cout << "Generated value: " << value << std::endl;
//}

// Function to save WiFi credentials
bool saveCredentials(const String& ssid, const String& password) {
  File file = LittleFS.open("/wifi.txt", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }
  file.println(ssid);
  file.println(password);
  file.close();
  return true;
}

// Function to read WiFi credentials
bool readCredentials(String& ssid, String& password) {
  File file = LittleFS.open("/wifi.txt", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return false;
  }
  ssid = file.readStringUntil('\n');
  ssid.trim(); // Remove any whitespace
  password = file.readStringUntil('\n');
  password.trim(); // Remove any whitespace
  file.close();
  return true;
}
