#include "mqtt_config.h"
#include "logging.h"

/**
 * @brief Construct a new MqttConfig object from a JSON object.
 *
 * This constructor initializes the MqttConfig object using data from the
 * provided JSON object.
 *
 * @param json JSON object containing the MQTT configuration parameters.
 */
MqttConfig::MqttConfig(JsonObject json)
{
  m_username = json["username"].as<String>();
  m_password = json["password"].as<String>();
  m_address = json["address"].as<String>();

  LOG_INFO("MQTT configuration loaded.");
}

/**
 * @brief Sets the User for the MQTT configuration.
 *
 * This function updates the User value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param username The new username
 */
void MqttConfig::setUser(String username)
{
  m_changed = changeStringConfig(m_username, username);
}

/**
 * @brief Sets the Password for the MQTT configuration.
 *
 * This function updates the password value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param password The new password
 */
void MqttConfig::setPass(String password)
{
  m_changed = changeStringConfig(m_password, password);
}

/**
 * @brief Sets the Address for the MQTT configuration.
 *
 * This function updates the Address value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param address The new address
 */
void MqttConfig::setAddress(String address)
{
  m_changed = changeStringConfig(m_address, address);
}

/**
 * @brief Fill a given JSON object with the MQTT configuration data.
 *
 * This method serializes the configuration data into the provided JSON object.
 *
 * @param json JSON object to populate with the MQTT configuration parameters.
 */
void MqttConfig::asJson(JsonObject &json)
{
  json["username"] = m_username;
  json["password"] = m_password;
  json["address"] = m_address;
}