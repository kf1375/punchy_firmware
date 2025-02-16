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

  String ssid() const;
  String password() const;
  bool stored() { return m_stored; }

  void setSSID(String ssid);
  void setPassword(String pass);
  void setFailed(bool failed) { m_connectionFailed = failed; }
  void setStored(bool stored) { m_stored = stored; }

  void reset(bool restart = false);
  bool failed() { return m_connectionFailed; }

  void asJson(JsonObject &json);

private:
  String m_ssid;
  String m_pass;
  bool m_stored = false; // true if credentials are stored
  bool m_connectionFailed = false;
};

#endif
