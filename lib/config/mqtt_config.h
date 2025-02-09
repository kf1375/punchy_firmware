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

  String getAddress() const { return m_address; }
  String getUser() const { return m_username; }
  String getPass() const { return m_password; }

  void setUser(String username) { changeStringConfig(m_username, username); }
  void setPass(String password) { changeStringConfig(m_password, password); }
  void setAddress(String address) { changeStringConfig(m_address, address); }

  bool credentialsStored() { return (m_username != "" && m_username != "null"); }

  void asJson(JsonObject &json);

private:
  String m_address = "mqtt://mq.dorsi-dynamics.com:1883";
  String m_username;
  String m_password;
};

#endif
