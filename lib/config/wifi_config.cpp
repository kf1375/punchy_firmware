#include "wifi_config.h"
#include "logging.h"

WifiConfig::WifiConfig(JsonObject json)
{
  m_ssid = json["ssid"].as<String>();
  m_pass = json["pass"].as<String>();
  LOG_INFO(m_ssid);
  LOG_INFO(m_pass);
  if (m_ssid.length() > 0) {
    m_stored = true;
  }
}

/**
 * @brief Sets the SSID for the WiFi configuration.
 *
 * This function updates the SSID value and marks the configuration as changed.
 * It also sets the stored flag to true.
 *
 * @param ssid The new SSID value to be set.
 */
void WifiConfig::setSSID(String ssid)
{
  m_changed = changeStringConfig(m_ssid, ssid);
  m_stored = true;
}

/**
 * @brief Sets the password for the WiFi configuration.
 *
 * This function updates the password value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param password The new password
 */
void WifiConfig::setPassword(String pass)
{
  m_changed = changeStringConfig(m_pass, pass);
}

/**
 * Gets the ssid for the WiFi configuration
 *
 * @return the ssid
 */
String WifiConfig::getSSID() const
{
  return m_ssid;
}

/**
 * Gets the password for the WiFi configuration
 *
 * @return the password
 */
String WifiConfig::getPassword() const
{
  return m_pass;
}

/**
 * Fill a json object with the WiFi configuration
 *
 * @param json the json to be filled
 */
void WifiConfig::asJson(JsonObject &wifi_json)
{
  wifi_json["ssid"] = getSSID();
  wifi_json["pass"] = getPassword();
}

/**
 * Reset the current wifi connection information
 *
 * @param restart if true, the device will clear flash and restart after the
 * reset
 */
void WifiConfig::reset(bool restart)
{
  m_stored = false;
  // Do not write this change to flash, to avoid loosing credentials on single
  // wifi outage. (restart will try again)
  if (restart) {
    setSSID("");
    setPassword("");
  }
}