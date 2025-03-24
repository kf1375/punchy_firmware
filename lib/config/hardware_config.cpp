#include "hardware_config.h"

#include "logging.h"

/**
 * @brief Construct a new HardwareConfig object from a JSON object.
 *
 * Initializes the HardwareConfig object using data from the provided JSON
 * object.
 *
 * @param json JSON object containing the hardware configuration parameters.
 */
HardwareConfig::HardwareConfig(JsonObject json)
{
  m_turnType = turnTypeFromString(json["turn_type"].as<String>());
  m_frontPos = json["front_position"].as<int>();
  m_rearPos = json["rear_position"].as<int>();
  m_singleSpeed = json["single_speed"].as<int>();
  m_infiniteSpeed = json["infinite_speed"].as<int>();
  m_maxHalfSpeed = json["max_half_speed"].as<int>();
  m_maxFullSpeed = json["max_full_speed"].as<int>();
  LOG_INFO("Hardware configuration loaded.");
}

/**
 * @brief Sets the turn type for the hardware configuration.
 *
 * This function updates the turn type value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param turnType The new turn type
 */
void HardwareConfig::setTurnType(TurnType turnType)
{
  if (m_turnType == turnType)
    m_changed = false;

  m_turnType = turnType;
  m_changed = true;
}

/**
 * @brief Sets the frontPos for the hardware configuration.
 *
 * This function updates the frontPos value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param frontPos The new frontPos
 */
void HardwareConfig::setFrontPosition(int frontPos)
{
  m_changed = changeIntConfig(m_frontPos, frontPos);
}

/**
 * @brief Sets the rearPos for the hardware configuration.
 *
 * This function updates the rearPos value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param rearPos The new rearPos
 */
void HardwareConfig::setRearPosition(int rearPos)
{
  m_changed = changeIntConfig(m_rearPos, rearPos);
}

/**
 * @brief Sets the single speed for the hardware configuration.
 *
 * This function updates the single speed value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param speed The new single speed
 */
void HardwareConfig::setSingleSpeed(int speed)
{
  m_changed = changeIntConfig(m_singleSpeed, speed);
}

/**
 * @brief Sets the infinite speed for the hardware configuration.
 *
 * This function updates the infinite speed value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param speed The new infinite speed
 */
void HardwareConfig::setInfiniteSpeed(int speed)
{
  m_changed = changeIntConfig(m_infiniteSpeed, speed);
}

/**
 * @brief Sets the max half speed for the hardware configuration.
 *
 * This function updates the max half speed value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param value The new max half speed
 */
void HardwareConfig::setMaxHalfSpeed(int value)
{
  m_changed = changeIntConfig(m_maxHalfSpeed, value);
}

/**
 * @brief Sets the max full speed for the hardware configuration.
 *
 * This function updates the max full speed value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param value The new max full speed
 */
void HardwareConfig::setMaxFullSpeed(int value)
{
  m_changed = changeIntConfig(m_maxHalfSpeed, value);
}

/**
 * @brief Fill a given JSON object with the HardwareConfig data.
 *
 * This method serializes the safety configuration parameters into the provided
 * JSON object.
 *
 * @param json JSON object to populate with the hardware configuration
 * parameters.
 */
void HardwareConfig::asJson(JsonObject &json)
{
  json["turn_type"] = turnTypeToString(m_turnType);
  json["front_position"] = m_frontPos;
  json["rear_position"] = m_rearPos;
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