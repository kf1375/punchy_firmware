#include "web_updater.h"

#include "util.h"

/**
 * @brief Initializes the WebUpdater class and sets up OTA update configurations.
 *
 * - Retrieves the current firmware version from `_config.firmware`.
 * - Initializes the `esp32FOTA` instance with device details.
 * - Sets the manifest URL for OTA updates.
 * - Registers a callback function to handle update completion.
 */
void WebUpdater::setup()
{
  // Get the current firmware version from the configuration
  m_deviceVersion = m_config.firmware.getVersion();

  // Initialize the esp32FOTA object with device name and firmware version
  m_esp32FOTA = new esp32FOTA(m_deviceName, m_deviceVersion);

  // Set the URL for the OTA manifest
  m_esp32FOTA->setManifestURL(m_manifestUrl);

  // Bind and register the callback function to handle actions after update finishes
  m_esp32FOTA->setUpdateFinishedCb(std::bind(&WebUpdater::updateFinishedCallback, this, std::placeholders::_1, std::placeholders::_2));
}

/**
 * @brief Continuously checks for OTA updates and initiates updates if required.
 *
 * - Executes periodically when Wi-Fi is connected.
 * - Checks for available updates every 10 seconds.
 * - Initiates OTA update if an update is available and flagged to start.
 */
void WebUpdater::loop()
{
  // Check if the device is connected to Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    static unsigned long last_run = 0;

    // Perform the update check every 10 seconds
    if (Util::timeElapsed(10, last_run)) {
      // Check for available updates and update the configuration state
      m_config.firmware.setUpdateAvailable(m_esp32FOTA->execHTTPcheck());
      last_run = millis();
    }

    // If an update is available and the start flag is set, initiate OTA
    if (m_config.firmware.getUpdateAvailable() && m_config.firmware.getStartUpdate()) {
      m_esp32FOTA->execOTA();
    }
  }
}

/**
 * @brief Callback function triggered after an OTA update finishes.
 *
 * @param partition Indicates which partition was updated (e.g., SPIFFS or firmware).
 * @param restart_after Flag indicating whether the device should restart after the update.
 *
 * - Logs the completion of the update.
 * - If SPIFFS was updated, saves the current configuration to disk.
 * - Restarts the device if the `restart_after` flag is true.
 */
void WebUpdater::updateFinishedCallback(int partition, bool restart_after)
{
  // Log which partition was updated (SPIFFS or firmware)
  LOG_INFO("Update of " + String(partition == U_SPIFFS ? "spiffs" : "firmware") + " partition finished\n");

  // If the SPIFFS partition was updated, save the running configuration
  if (partition == U_SPIFFS) {
    LOG_INFO("Saving running configuration to disk...");
    m_config.serializeConfig();
  }

  // Restart the device if the restart flag is true
  if (restart_after) {
    ESP.restart();
  }
}
