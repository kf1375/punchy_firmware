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