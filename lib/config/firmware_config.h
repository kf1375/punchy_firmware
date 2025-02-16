#ifndef FIRMWARE_CONFIG_H
#define FIRMWARE_CONFIG_H

#include "Arduino.h"
#include "ArduinoJson.h"

#include "abstract_config.h"

//************************************************************************************************
// Config Class for various firmware relevant variables
// Provides getters and setters for those
// ***********************************************************************************************
class FirmwareConfig : public AbstractConfig
{
public:
  FirmwareConfig() {};
  FirmwareConfig(JsonObject json);
  ~FirmwareConfig() {};

  String version() { return m_version; };
  bool updateAvailable() { return m_updateAvailable; };
  bool startUpdate() { return m_startUpdate; };

  void setVersion(String newVersion);
  void setUpdateAvailable(bool updateAvailable);
  void setStartUpdate(bool startUpdate);

  void asJson(JsonObject &json);

private:
  String m_version;
  bool m_updateAvailable = false;
  bool m_startUpdate = false;
};

#endif
