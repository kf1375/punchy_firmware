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
 * @brief Sets the version for the firmware configuration.
 *
 * This function updates the version value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param newVersion The new version
 */
void FirmwareConfig::setVersion(String newVersion)
{
  m_changed = changeStringConfig(m_version, newVersion);
}

/**
 * @brief Sets update available flag
 *
 * @param updateAvailable The new update available flag
 */
void FirmwareConfig::setUpdateAvailable(bool updateAvailable)
{
  m_updateAvailable = updateAvailable;
}

/**
 * @brief Sets start update flag
 *
 * @param startUpdate The new start update flag
 */
void FirmwareConfig::setStartUpdate(bool startUpdate)
{
  m_startUpdate = startUpdate;
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