#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include "Arduino.h"
#include "abstract_config.h"

//************************************************************************************************
// Config Class for a wifi network interface
// Provides getters and setters for credentials
// ***********************************************************************************************
class WifiConfig : public AbstractConfig
{
public:
  WifiConfig() {}
  WifiConfig(JsonObject json);
  ~WifiConfig() {}

  void setSSID(String ssid);
  void setPassword(String pass);
  String getSSID() const;
  String getPassword() const;
  void setFailed(bool failed) { m_connection_failed = failed; }

  bool stored() { return m_stored; }
  void stored(bool stored) { m_stored = stored; }
  void asJson(JsonObject &json);
  void reset(bool restart = false);
  bool failed() { return m_connection_failed; }

private:
  String m_ssid;
  String m_pass;
  bool m_stored = false; // true if credentials are stored
  bool m_connection_failed = false;
};

#endif
