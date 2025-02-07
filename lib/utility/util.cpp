#include "util.h"

#include <FS.h>
#include <LittleFS.h>

String Util::getMacAddress()
{
    uint8_t device_mac_address[6];
    esp_efuse_mac_get_default(device_mac_address);

    String device_id = "";
    for (uint8_t i = 0; i < 6; i++) {
        device_id += String(device_mac_address[i], HEX);
    }
    device_id.toUpperCase();
    return device_id;
}

void Util::printMgStr(const mg_str &str)
{
    for (size_t i = 0; i < str.len; i++) {
        Serial.print(str.buf[i]);
    }
    Serial.print("\n");
}

// Function to save WiFi credentials
bool Util::saveCredentials(const String& ssid, const String& password)
{
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
bool Util::readCredentials(String& ssid, String& password)
{
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
