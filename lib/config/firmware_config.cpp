#include "firmware_config.h"

#include "logging.h"

/**
 * @brief Construct a new FirmwareConfig object from a JSON object.
 *
 * Initializes the FirmwareConfig object using data from the provided JSON
 * object.
 *
 * @param json JSON object containing the firmware configuration parameters.
 */
FirmwareConfig::FirmwareConfig(JsonObject json)
{
  m_version = json["version"].as<String>();
  LOG_INFO("Firmware configuration loaded.");
}

/**
 * @brief Fill a given JSON object with the FirmwareConfig data.
 *
 * This method serializes the firmware configuration parameters into the
 * provided JSON object.
 *
 * @param json JSON object to populate with the firmware configuration
 * parameters.
 */
void FirmwareConfig::asJson(JsonObject &json)
{
  json["version"] = m_version;
}