#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "logging.h"
#include <ArduinoJson.h>
#include <vector>

#include "firmware_config.h"
#include "hardware_config.h"
#include "mqtt_config.h"
#include "wifi_config.h"

//************************************************************************************************
// Main Configuration class to combine all other configuration classes
// Handles serialization in a loop and deserialization on startup
//************************************************************************************************
class Configuration
{
public:
  Configuration() {}
  ~Configuration() {};
  void begin();
  void loop();

  WifiConfig wifi;
  FirmwareConfig firmware;
  MqttConfig mqtt;
  HardwareConfig hardware;

  void scheduleRestart(int seconds)
  {
    m_restart_at = millis() + seconds * 1000;
  };
  void serializeConfig();

private:
  const char *path = "/configuration.json";

  void deserializeConfig();

  bool m_changed;
  unsigned long m_restart_at = 0;
};

#endif // CONFIGURATION_H