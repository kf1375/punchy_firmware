#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include "Arduino.h"
#include "ArduinoJson.h"
#include "abstract_config.h"

//************************************************************************************************
// Config Class for mqtt relevant variables
// Provides getters and setters for those
// ***********************************************************************************************
class MqttConfig : public AbstractConfig
{
public:
  MqttConfig() {}
  MqttConfig(JsonObject json);
  ~MqttConfig() {}

  String address() const { return m_address; }
  String user() const { return m_username; }
  String pass() const { return m_password; }

  void setUser(String username);
  void setPass(String password);
  void setAddress(String address);

  bool credentialsStored()
  {
    return (m_username != "" && m_username != "null");
  }

  void asJson(JsonObject &json);

private:
  String m_address;
  String m_username;
  String m_password;
};

#endif
