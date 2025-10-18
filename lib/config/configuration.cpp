#include "configuration.h"

#include "logging.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

/**
 * @brief Flash storage setuo
 *
 * Initializes the flash file system, log all present files and read config to
 * RAM
 */
void Configuration::begin()
{
  LOG_INFO("Setting up config from filesystem...");
  // Setting up littlefs File System, required for HTML and dataloggin
  if (!LittleFS
           .begin()) { // Mounts the littlefs file system and handle littlefs
    // Errors:
    LOG_INFO("An Error has occurred while mounting SPIFFS");
    return;
  }

  // List all files in littlefs (for debugging purposes)
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file) {
    LOG_INFO(file.name());
    file = root.openNextFile();
  }

  deserializeConfig();
}

/**
 * @brief Flash storage loop
 *
 * Config Loop to check if any configuration changed and store the flash if true
 */
void Configuration::loop()
{
  if (wifi.changed() || mqtt.changed() || hardware.changed() ||
      firmware.changed()) {
    serializeConfig();
    if (wifi.ssid().length() > 0) {
      wifi.setStored(true);
    }
  }
}

/**
 * @brief Deserializes the config
 *
 * Deserializes the configuration from the flash storage and store
 * the configuration values into the respective variables in ram
 */
void Configuration::deserializeConfig()
{
  File file = LittleFS.open(path, FILE_READ);
  String fileContent = file.readString();
  JsonDocument doc;
  if (!file) {
    LOG_INFO("Failed to open file for reading");
    return;
  }
  DeserializationError error = deserializeJson(doc, fileContent);
  if (error) {
    LOG_INFO("Failed to read file " + String(error.code()));
  }
  file.close();

  JsonObject wifiJson = doc["wifi"].as<JsonObject>();
  wifi = WifiConfig(wifiJson);

  JsonObject mqttJson = doc["mqtt"].as<JsonObject>();
  mqtt = MqttConfig(mqttJson);

  JsonObject hardwareJson = doc["hardware"].as<JsonObject>();
  hardware = HardwareConfig(hardwareJson);

  JsonObject firmwareJson = doc["firmware"].as<JsonObject>();
  firmware = FirmwareConfig(firmwareJson);

  LOG_INFO(fileContent);
  LOG_INFO("Deserialization done!");
}

/**
 * @brief Serialization of stored config
 *
 * Serializes the configuration and stores it on the flash storage
 */
void Configuration::serializeConfig()
{
  File file = LittleFS.open(path, FILE_WRITE);
  if (!file) {
    LOG_INFO("- failed to open file for writing");
    return;
  }

  JsonDocument doc;

  JsonObject wifiJson = doc["wifi"].to<JsonObject>();
  wifi.asJson(wifiJson);

  JsonObject mqttJson = doc["mqtt"].to<JsonObject>();
  mqtt.asJson(mqttJson);

  JsonObject hardwareJson = doc["hardware"].to<JsonObject>();
  hardware.asJson(hardwareJson);

  JsonObject firmwareJson = doc["firmware"].to<JsonObject>();
  firmware.asJson(firmwareJson);

  if (serializeJsonPretty(doc, file) == 0) {
    LOG_INFO("Failed to write to file");
  }
  file.close();

  // LOG_INFO("Stored new configuration");
}
