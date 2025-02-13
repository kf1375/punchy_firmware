#include "hardware_config.h"

#include "logging.h"

/**
 * @brief Construct a new SpeedConfig object from a JSON object.
 *
 * Initializes the SpeedConfig object using data from the provided JSON object.
 *
 * @param json JSON object containing the safety configuration parameters.
 */
HardwareConfig::HardwareConfig(JsonObject json)
{
  m_turnType = turnTypeFromString(json["turn_type"].as<String>());
  m_frontPos = json["front_position"].as<int>();
  m_singleSpeed = json["single_speed"].as<int>();
  m_infiniteSpeed = json["infinite_speed"].as<int>();
  m_maxHalfSpeed = json["max_half_speed"].as<int>();
  m_maxFullSpeed = json["max_full_speed"].as<int>();
  LOG_INFO("Hardware configuration loaded.");
}

/**
 * @brief Fill a given JSON object with the SpeedConfig data.
 *
 * This method serializes the safety configuration parameters into the provided
 * JSON object.
 *
 * @param json JSON object to populate with the safety configuration parameters.
 */

void HardwareConfig::asJson(JsonObject &json)
{
  json["turn_type"] = turnTypeToString(m_turnType);
  json["front_position"] = m_frontPos;
  json["single_speed"] = m_singleSpeed;
  json["infinite_speed"] = m_infiniteSpeed;
  json["max_half_speed"] = m_maxHalfSpeed;
  json["max_full_speed"] = m_maxFullSpeed;
}

/**
 * @brief Convert a string to a TurnType enum.
 *
 * This method converts a string to a TurnType enum.
 *
 * @param turnTypeStr String representation of the TurnType.
 * @return HardwareController::TurnType The TurnType enum.
 */
HardwareConfig::TurnType
HardwareConfig::turnTypeFromString(const String &turnTypeStr)
{
  if (turnTypeStr == "HALF_TURN") {
    return TurnType::HalfTurn;
  } else if (turnTypeStr == "FULL_TURN") {
    return TurnType::FullTurn;
  } else {
    return TurnType::HalfTurn;
  }
}

/**
 * @brief Convert a TurnType enum to a string.
 *
 * This method converts a TurnType enum to a string.
 *
 * @param turnType The TurnType enum.
 * @return String The string representation of the TurnType.
 */
String HardwareConfig::turnTypeToString(TurnType turnType) const
{
  if (turnType == TurnType::HalfTurn) {
    return "HALF_TURN";
  } else if (turnType == TurnType::FullTurn) {
    return "FULL_TURN";
  } else {
    return "HALF_TURN";
  }
}