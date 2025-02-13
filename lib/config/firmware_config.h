#ifndef FIRMWARE_CONFIG_H
#define FIRMWARE_CONFIG_H

#include "Arduino.h"
#include "ArduinoJson.h"
#include "abstract_config.h"

#ifndef DEVICE_VERSION
#define DEVICE_VERSION "0.0.1"
#endif

//************************************************************************************************
// Config Class for various driver relevant variables
// Provides getters and setters for those
// ***********************************************************************************************
class FirmwareConfig : public AbstractConfig
{
public:
  FirmwareConfig() {}
  ~FirmwareConfig() {}

  String getVersion() { return version; }
  bool getUpdateAvailable() { return update_available; }
  bool getStartUpdate() { return start_update; }
  void setVersion(String new_version)
  {
    changeStringConfig(version, new_version);
  }
  void setUpdateAvailable(bool new_update_available)
  {
    update_available = new_update_available;
  }
  void setStartUpdate(bool new_start_update)
  {
    start_update = new_start_update;
  }

  void asJson(JsonObject &json)
  {
    json["version"] = version;
    json["update_available"] = update_available;
  }

private:
  String version = DEVICE_VERSION;

  bool update_available = false;
  bool start_update = false;
};

#endif
