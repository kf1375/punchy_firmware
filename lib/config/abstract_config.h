#ifndef ABSTRACT_CONFIG_H
#define ABSTRACT_CONFIG_H

#include "Arduino.h"
#include "ArduinoJson.h"
#include "logging.h"

//************************************************************************************************
// Child Class for every config class
// Defines functions to reliably change config variables
// Provide a flag to indicate a changed variable to trigger flash storage
//************************************************************************************************

class AbstractConfig
{
public:
  AbstractConfig() : m_changed(false) {}
  ~AbstractConfig() {}

  bool changeFloatConfig(float &old_config, float new_value);
  bool changeBoolConfig(bool &old_config, bool new_value);
  bool changeStringConfig(String &old_config, String new_value);

  bool changed();
  virtual void asJson(JsonObject &result) = 0;

protected:
  bool m_changed;
};

#endif //
